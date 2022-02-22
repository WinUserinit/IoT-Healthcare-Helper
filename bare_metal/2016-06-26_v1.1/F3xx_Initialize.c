//-----------------------------------------------------------------------------
// F3xx_Initialize.c
//-----------------------------------------------------------------------------
// Copyright 2014
//
// Program Description:
//
// Initializes uC's system clock, timers, USB, SMBus, ports.
//
//
// FID             3XX000001
// Target:         C8051F32x/C8051F340
// Tool chain:     Keil C51 7.50 / Keil EVAL C51
//                 Silicon Laboratories IDE version 2.6
// Project Name:   Generic HID Firmware
//
//
// Release 1.0
//    -140401 Initial Revision (Tushar Mazumder)
//
// Other than bug fixes or implementation changes, this file should not need to be updated. (But, configure port pins in this file if using this file as a base for other projects.)

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------

// Project-level preprocessor #defines effective across all *.c files
#include "ProjectDefines.h"

#include "C8051F320.h"
#include "F3xx_Initialize.h"
#include "F3xx_USB0_InterruptServiceRoutine.h"
#include "F3xx_USB0_Register.h"
#include "SMBus.h"

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

// USB clock selections (SFR CLKSEL)
#define USB_4X_CLOCK       0x00        // Select 4x clock multiplier, for USB
#define USB_INT_OSC_DIV_2  0x10        // Full Speed
#define USB_EXT_OSC        0x20
#define USB_EXT_OSC_DIV_2  0x30
#define USB_EXT_OSC_DIV_3  0x40
#define USB_EXT_OSC_DIV_4  0x50

// System clock selections (SFR CLKSEL)
#define SYS_INT_OSC        0x00        // Select to use internal oscillator
#define SYS_EXT_OSC        0x01        // Select to use an external oscillator
#define SYS_4X_DIV_2       0x02

// ----------------------------------------------------------------------------
// Global Variable Declarations
// ----------------------------------------------------------------------------
float gSysClkFreq;	
float gTimer1ClkFreq;	// global to send back to GUI in order to calculate allowable SMBus clock frequencies
float gSMBusClkFreq;
uint8_t xdata IO_PACKET[64]; // alias IO_BUFFER.Ptr[0..63]
extern bit gSMBbusy; 
// uint16_t gTH0; // VERIFY: OS24EVK-66 remove unused global varaibles; conserve xdata space
// uint16_t gTL0; // VERIFY: OS24EVK-66 remove unused global varaibles; conserve xdata space

// ----------------------------------------------------------------------------
// Local Function Prototypes
// ----------------------------------------------------------------------------
void Sysclk_Init(void);                
void Port_Init(void);                  
void Usb_Init(void);
#if SUPPORT_SPI
void SPI_Init(void);
#endif // SUPPORT_SPI
void SMBus_Init(void);
void Timer0_Init(uint8_t delayUnitsAreMicroseconds);
void Timer1_Init(void);
void Timer3_Init(void);
void T0_Wait(uint16_t delayCount); // number of half-milliseconds or microseconds to wait, depending on previous Timer0_Init(uint8_t delayUnitsAreMicroseconds)
void getTimer0or1ClkFreq(uint8_t, float*);
void getTimer2ClkFreq(float*);
void getTimer3ClkFreq(float*);
void INT0_Init(void);



//-----------------------------------------------------------------------------
// System_Init (void)
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// This top-level initialization routine calls all support routine.
//
// ----------------------------------------------------------------------------
void System_Init(void)
{
	PCA0MD &= ~0x40;                    // Disable Watchdog timer

	Sysclk_Init();                      // Initialize oscillator
	Port_Init();                        // Initialize crossbar and GPIO
	Usb_Init();                         // Initialize USB0
	//Timer0_Init(1);
	Timer1_Init();                      // Initialize timer1 for SMBus clock
	Timer3_Init ();						// Initialize timer3 for SMBus clock low timeout
#if SUPPORT_SPI
	SPI_Init();							// Inititalize SPI
#endif // SUPPORT_SPI
	SMBus_Init();						// Initialize SMBus (I2C)
	INT0_Init();
}

//-----------------------------------------------------------------------------
// Sysclk_Init
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Initialize system clock to maximum frequency.
//
//-----------------------------------------------------------------------------
void Sysclk_Init(void)
{
	uint8_t delay = 100;
											//internal oscillator = 12MHz
	OSCICN = 0x83;                        	//enable internal oscillator (default); SYSCLK = internal oscillator div 1 if CLKSEL.[10] = 00

	CLKMUL = 0x00;                         	//reset 4x clock multiplier; select internal oscillator as input to 4x clock multiplier
	CLKMUL |= 0x80;                        	//enable 4x clock multiplier
	while (delay--);                       	//delay for >5us per page 121 of manual
	CLKMUL |= 0xC0;                        	//initialize the 4x clock multiplier
	while(!(CLKMUL & 0x20));                //wait for 4x clock multiplier to lock

	CLKSEL = 0x00;                			//USB clock is from the 4x clock multiplier = 48MHz
	CLKSEL |= 0x02;							//SYSCLK is from the 4x clock multiplier div 2 = 24MHz (if lowest two bits 0, then would come from OSCICN = 12MHz)
	switch (CLKSEL & 3)
	{
		case (0):
			switch (OSCICN & 3)
			{
				case (0):
					gSysClkFreq = INT_OSC_FREQ / 8.0;
					break;
				case (1):
					gSysClkFreq = INT_OSC_FREQ / 4.0;
					break;
				case (2):
					gSysClkFreq = INT_OSC_FREQ / 2.0;
					break;
				case (3):
					gSysClkFreq = INT_OSC_FREQ / 1.0;
					break;
				default:
					gSysClkFreq = 0.0;
			}
			break;
		case (2):
			gSysClkFreq = INT_OSC_FREQ * 4.0 / 2.0;
			break;
		default:
			gSysClkFreq = 0.0;
	}
}


//-----------------------------------------------------------------------------
// Port_Init
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// - Configure the Crossbar and GPIO ports.
//
//-----------------------------------------------------------------------------
void Port_Init(void)
{  
					// P0.7 LED2 (1 = on)							PP	LO	D	NS
					// P0.6 LED1 (1 = on)							PP	LO	D	NS
					// P0.5 SCL										PP	LO	D	NS will automatically be configured for OC
					// P0.4 SDA										PP	LO	D	NS will automatically be configured for OC
					// P0.3 NSS										PP	LO	D	NS
					// P0.2 MOSI									PP	LO	D	NS
					// P0.1 MISO									PP	LO	D	NS
					// P0.0 SCK										PP	LO	D	NS
	//P0MDOUT = 0xFF;	// PP PP PP PP : PP PP PP PP
	//P0MDIN  = 0xFF;	// D  D  D  D  : D  D  D  D
	//P0SKIP  = 0x00;	// NS NS NS NS : NS NS NS NS
	//P0      = 0x00;	// LO LO LO LO : LO LO LO LO

					// P1.7 GPIO or ADC input
					// P1.6 GPIO or ADC input
					// P1.5 GPIO or ADC input
					// P1.4 GPIO or ADC input
					// P1.3 GPIO or ADC input
					// P1.2 GPIO or ADC input
					// P1.1 GPIO or ADC input
					// P1.0 GPIO or ADC input
	//P1MDOUT = 0x00;	// OC OC OC OC : OC OC OC OC
	//P1MDIN  = 0xFF;	// D  D  D  D  : D  D  D  D
	//P1SKIP  = 0x00;	// NS NS NS NS : NS NS NS NS
	//P1      = 0xFF;	// HI HI HI HI : HI HI HI HI

					// P2.7 NOT USED
					// P2.6 NOT USED
					// P2.5 NOT USED
					// P2.4 NOT USED
					// P2.3 GPIO or ADC input
					// P2.2 GPIO or ADC input
					// P2.1 GPIO or ADC input
					// P2.0 GPIO or ADC input
	//P2MDOUT = 0x00;	// OC OC OC OC : OC OC OC OC
	//P2MDIN  = 0xFF;	// D  D  D  D  : D  D  D  D
	//P2SKIP  = 0x00;	// NS NS NS NS : NS NS NS NS
	//P2      = 0xFF;	// HI HI HI HI : HI HI HI HI

	//---------------------------------------------
	// MAX30101EVKIT pin assignment:
	//   P0.0 = Accelerometer INT (optional INT1 interrupt source)
	//   P0.1 = MAX30101 INT (INT0 interrupt source)
	//   P0.2 = MAX30101 SDA
	//   P0.3 = MAX30101 SCL
	//   P2.1 = diagnostic LED driver DS2
	//   P2.2 = diagnostic LED driver DS1
	P0SKIP  = 0x03; // Port0 Skip Register bitmap: 1=skipped by the crossbar
	//   crossbar skip P0.0 = unassigned
	//   crossbar skip P0.1 = MAX30101 INT
	//   P0.2 = MAX30101 SDA
	//   P0.3 = MAX30101 SCL
	P2MDOUT = 0x06; // Port 2 Output Mode Register bitmap: 1=push-pull, 0=open-drain
	//   push-pull P2.1 = diagnostic LED driver DS2
	//   push-pull P2.2 = diagnostic LED driver DS1
	XBR0    = 0x04;	// SMB0E=1: SMBus (I2C) enable
			// XBR0: CP1AE=0 CP1E=0 CP0AE=0 CP0E=0 SYSCKE=0 SMB0E=1 SPI0E=0 URT0E=0 
	XBR1    = 0x40;	// XBARE=1: XBAR enable
			// XBR1: WEAKPUD=0 XBARE=1 T1E=0 T0E=0 ECIE=0 PCA0ME=000
	//XBR0    = 0x06;	// SPI and SMBus (I2C) enable
	//XBR1    = 0xC0;	// XBAR enable, Pull-Up disable

	//---------------------------------------------
	// SiLabs C8051F321 crossbar assigns pin resources in order (LSB first):
	// UART can only use TX0/P0.4 and RX0/P0.5
	// SPI assigns SCLK, MISO, MOSI, and possibly NSS in 4-wire mode
	// SMBus (I2C) assigns SDA, SCL
	// CP0 comparator outputs (CP0 cannot be on P0.1)
	// CP1 comparator outputs
	// SYSCLK
	// PCA
	// Timers T0, T1
	// GPIO ports P0, P1, P2, P3
	//---------------------------------------------
	// Independently of the crossbar, the external interrupts INT0/INT1 are assigned
	// by the IT01CF register.
	// default INT0 active-low monitoring port P0.1
	// default INT1 active-low monitoring port P0.0
	//---------------------------------------------
	// If both SPI and SMBus(I2C) are both enabled enabled, and we reserve P0.0 and P0.1:
	//   P0.0 = unassigned (INT1 interrupt source)
	//   P0.1 = unassigned (INT0 interrupt source)
	//   P0.2 = SCK
	//   P0.3 = MISO
	//   P0.4 = MOSI
	//   P0.5 = SDA
	//   P0.6 = SCL
	//---------------------------------------------
	// If no SPI but SMBus(I2C) is enabled, and we reserve P0.0 and P0.1:
	//   P0.0 = unassigned (INT1 interrupt source)
	//   P0.1 = unassigned (INT0 interrupt source)
	//   P0.2 = SDA
	//   P0.3 = SCL
	//---------------------------------------------

}

#if SUPPORT_SPI
void SPI_Init(void)
{
   	SPI0CFG = 0x40;	// Enable master mode, CKPOL = 0, CKPHA = 0
    SPI0CN  = 0x0D;	// Single master mode, NSS = 1, SPI0 enabled
    SPI0CKR = 0x0B;	// SPI clock rate divisor; SPI clock rate is SYSCLK / [2 * (SPI0CKR + 1)]
}
#endif // SUPPORT_SPI

void SMBus_Init(void)
{			
    SMB0CF 	= 0xCD;		//Enable SMBus; enable SMBus slave inhibit; enable SCL timeout detection; enable SMBbus free timeout detection; timer1 overflow is SMBus clock source
	EIE1	|= 0x01;	//Enable SMBus interrupts. Make sure to OR this since the register is preset elsewhere.
	EIP1 	|= 0x01;	//set SMBus interrupt to  high priority
}

//-----------------------------------------------------------------------------
// USB0_Init
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// - Initialize USB0
// - Enable USB0 interrupts
// - Enable USB0 transceiver
// - Enable USB0 with suspend detection
//
//-----------------------------------------------------------------------------
void Usb_Init(void)
{
   POLL_WRITE_BYTE(POWER,  0x08);      // Force Asynchronous USB Reset
   POLL_WRITE_BYTE(IN1IE,  0x07);      // Enable Endpoint 0-2 in interrupts
   POLL_WRITE_BYTE(OUT1IE, 0x07);      // Enable Endpoint 0-2 out interrupts
   POLL_WRITE_BYTE(CMIE,   0x07);      // Enable Reset, Resume, and Suspend
   USB0XCN = 0xE0;                     // Enable transceiver; select full speed
   POLL_WRITE_BYTE(CLKREC, 0x80);      // Enable clock recovery, single-step
   EIE1 |= 0x02;                       // Enable USB0 Interrupts
   EA = 1;                             // Global Interrupt enable
                                       // Enable USB0 by clearing the USB
                                       // Inhibit bit
   POLL_WRITE_BYTE(POWER,  0x01);      // and enable suspend detection
}


//-----------------------------------------------------------------------------
// T0_Wait
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//   1) uint16_t ms - number of half-milliseconds or microseconds to wait
//                        range is full range of int: 0 to 65536
//
// Configure Timer0 to wait for <half-ms> half-milliseconds or microseconds using SYSCLK as its time
// base.
//
void Timer0_Init(uint8_t delayUnitsAreMicroseconds)
//delayUnitsAreMicroseconds: 0 half-ms delay; 1 us delay
{
	float timer0ClkFreq;
	TCON &= ~0x30;                      // Stop Timer0; Clear TF0
	TMOD &= ~0x0f;                      // clear T0 settings
	TMOD |=  0x02;						// Timer0 in 8-bit auto-reload mode
	CKCON &= ~0x07;						// clear Timer0 clock settings; WARNING: this clears timer0/1 prescale bits too!
	CKCON |= (delayUnitsAreMicroseconds ? 0x04 : 0x02);		// T0 clock is SYSCLK if 0x04 (us delay); T0 clock is SYSCLK/48 if 0x02 (half-ms delay; cannot do ms delay using 8-bit counter)
	//IP |= 2;							// high priority required in order to preempt USB interrupt
	//ET0 = 1;							// T0 interrupt enable
	getTimer0or1ClkFreq(0, &timer0ClkFreq);
	TH0 = -((uint32_t)timer0ClkFreq/(delayUnitsAreMicroseconds ? 1000000 : 1000));	// overflow every us or half-ms
	// (1/timer0ClkFreq) is the time for one clock in s; (1/timer0ClkFreq)*1000 is the time for one clock in ms (ms/clock)
	// 1 / [(1/timer0ClkFreq)*1000] ms/clock = timer0ClkFreq/1000 clocks/ms is the number of clocks for one ms
}

void T0_Wait(uint16_t delayCount) // number of half-milliseconds or microseconds to wait, depending on previous Timer0_Init(uint8_t delayUnitsAreMicroseconds)
{
	if (delayCount < 5) {
		return;
   }
	TL0 = TH0;							// set inital count before starting counter
	TF0 = 0;                         	// Clear overflow indicator
	TR0 = 1;      	
	while (delayCount - 5)
	{
		while (!TF0);
		TF0 = 0;						// Clear overflow indicator
		delayCount--;
	}
	TR0 = 0;                            
}

void getTimer0or1ClkFreq(uint8_t timer, float *timer0or1ClkFreq)
{
	uint8_t mask;
	uint8_t rightShift;
	switch (timer)			
	{
		case 0:				//timer0 uses bit 2 to determine the clock source; right shift twice to get it into the 0 position for comparison
			mask = 4;
			rightShift = 2;
			break;
		case 1:				//timer1 uses bit 3 to determine the clock source; right shift thrice to get it into the 0 position for comparison
			mask = 8;
			rightShift = 3;
			break;
	}
	switch ((CKCON & mask) >> rightShift)
	{
		case (0):
			switch (CKCON & 3)
			{
				case (0):
					*timer0or1ClkFreq = gSysClkFreq / 12.0;
					break;
				case (1):
					*timer0or1ClkFreq = gSysClkFreq / 4.0;
					break;
				case (2):
					*timer0or1ClkFreq = gSysClkFreq / 48.0;
					break;
				default:
					*timer0or1ClkFreq = 0.0;
			}
			break;
		case (1):
			*timer0or1ClkFreq = gSysClkFreq;
			break;
		default:
			*timer0or1ClkFreq = 0.0;
	}
}

//-----------------------------------------------------------------------------
// Timer1_Init()
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Timer1 is configured as the SMBus clock source as follows:
// - Timer1 in 8-bit auto-reload mode
// - Timer1 overflow rate => 3 * gSMBusClkFreq
// - The maximum SCL clock rate will be ~1/3 the Timer1 overflow rate
// - Timer1 enabled
//
void Timer1_Init (void)
{
// gSMBusClkFreq = timer1ClkFreq / (0xFF - TH1 + 1) / 3 
	//CKCON &= ~0x08;	// clear Timer1 clock settings; don't clear prescale bits
	CKCON |= 0x08; 	// Timer1 clock source = gSysClkFreq
	TMOD &= ~0xf0;	// clear T1 settings
	TMOD |= 0x20; 	// Timer1 in 8-bit auto-reload mode
	getTimer0or1ClkFreq(1, &gTimer1ClkFreq);
	// Timer1 configured to overflow at 1/3 the rate defined by gSMBusClkFreq
	if ((gTimer1ClkFreq/gSMBusClkFreq/3.0) < 255.0)	// SMBus clock = 31.25kHz min, 8MHz max; not contiguous since integer division
		TH1 = -(gTimer1ClkFreq/gSMBusClkFreq/3.0);
	else
		TH1 = 0; 	// gSysClkFreq is too fast to generate the desired gSMBusClkFreq; set to the slowest possible SMB_FREQUENCY
	TL1 = TH1; 		// Init Timer1
	TR1 = 1; 		// Timer1 enabled
}

//-----------------------------------------------------------------------------
// Timer3_Init()
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Timer3 configured for use by the SMBus low timeout detect feature as
// follows:
// - Timer3 in 16-bit auto-reload mode
// - SYSCLK/12 as Timer3 clock source
// - Timer3 reload registers loaded for a 25ms overflow period
// - Timer3 pre-loaded to overflow after 25ms
// - Timer3 enabled
//
void Timer3_Init (void)
{
	float timer3ClkFreq;
	TMR3CN = 0x00;                      // Timer3 configured for 16-bit auto-reload, low-byte interrupt disabled
	CKCON &= ~0x40;                     // Timer3 uses SYSCLK/12
	getTimer3ClkFreq(&timer3ClkFreq);
	TMR3RL = -(timer3ClkFreq/40.0);		// Timer3 configured to overflow after ~25ms (for SMBus low timeout detect)
	TMR3 = TMR3RL;                      // number of clocks for 25ms = 25 / [(1/timer3ClkFreq) * 1000] = timer3ClkFreq/40

	EIE1 |= 0x80;                       // Timer3 interrupt enable
	TMR3CN |= 0x04;                     // Start Timer3
}

void getTimer3ClkFreq(float *timer3ClkFreq)
{
	switch (CKCON & 0x40)
	{
		case (0):
			switch (TMR3CN & 1)
			{
				case (0):
					*timer3ClkFreq = gSysClkFreq / 12.0;
					break;
				default:
					*timer3ClkFreq = 0.0;
			}
			break;
		case (0x40):
			*timer3ClkFreq = gSysClkFreq;
			break;
		default:
			*timer3ClkFreq = 0.0;
	}
}

//-----------------------------------------------------------------------------
// Timer3 Interrupt Service Routine (ISR)
//-----------------------------------------------------------------------------
// A Timer3 interrupt indicates an SMBus SCL low timeout.
// The SMBus is disabled and re-enabled if a timeout occurs.
//
void Timer3_ISR (void) interrupt INTERRUPT_TIMER3 /* interrupt 14 */
{
   SMB0CF &= ~0x80;                    // Disable SMBus
   SMB0CF |= 0x80;                     // Re-enable SMBus
   TMR3CN &= ~0x80;                    // Clear Timer3 interrupt-pending flag
   gSMBbusy = 0;                       // Free bus
}

void INT0_Init(void)
{
	TCON |= 0x05; // TF1=x TR1=x TF0=x TR0=x IE1=x IT1=1 (INT1 edge trigger) IE0=x IT0=1 (INT0 edge trigger)
	//IT01CF   = 0x11;                   // INT0 on P0.1, active low (default)
	EX0 = 1;							// Enable INT0 interrupt	// FIXME this needs to be enabled on startup, otherwise when we enable a bogus report is sent.
	//EX1 = 1;							// Enable INT1 interrupt
}