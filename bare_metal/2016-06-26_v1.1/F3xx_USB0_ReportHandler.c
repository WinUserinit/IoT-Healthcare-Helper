//-----------------------------------------------------------------------------
// F3xx_USB0_ReportHandler.c
//-----------------------------------------------------------------------------
// Copyright 2014
//
// Program Description:
//
// Contains functions and variables dealing with Input and Output HID reports.
//
//
// FID:            3XX000007
// Target:         C8051F340
// Tool chain:     Keil C51 7.50 / Keil EVAL C51
//                 Silicon Laboratories IDE version 2.6
// Project Name:   Generic HID Firmware
//
//
// Release 1.0
//   	-140401 Initial Revision (Tushar Mazumder)
//
// OUT_REPORT_HANDLER may be modified to handle new features.

// ----------------------------------------------------------------------------
// Header files
// ----------------------------------------------------------------------------

// Project-level preprocessor #defines effective across all *.c files
#include "ProjectDefines.h"

// OS24EVK-58 Software Toolkit - bare metal device driver
#if SUPPORT_MAX30100
#include "max30100.h"
#endif // SUPPORT_MAX30100

// OS24EVK-58 Software Toolkit - bare metal device driver
#if SUPPORT_MAX30101
#include "max30101.h"
#endif // SUPPORT_MAX30101

// OS24EVK-58 Software Toolkit - bare metal device driver
#if SUPPORT_LIS2DH
#include "lis2dh.h"
#endif // SUPPORT_LIS2DH

// OS24EVK-58 Software Toolkit - bare metal device driver
#if SUPPORT_MAX7311
#include "max7311.h"
#endif // SUPPORT_MAX7311

#include "c8051f3xx.h" // not in SiLabs example for this file
#include "F3xx_USB0_ReportHandler.h"
#include "F3xx_USB0_InterruptServiceRoutine.h"
#include "F3xx_USB0_Descriptor.h"
#include "F3xx_Initialize.h"


// ----------------------------------------------------------------------------
// Local Function Prototypes
// ----------------------------------------------------------------------------

void OUT_REPORT_HANDLER(int16_t internalCall);

// ----------------------------------------------------------------------------
// Global Variable Declaration
// ----------------------------------------------------------------------------

//#define DEBUGOS21	//verify the write/read ptr locations

BufferStructure IO_BUFFER;

sbit P20Btn = P2^0;   	//   P2.0 = diagnostic pushbutton on C8051F321 daughtercard TOOLSTICK321DC (0=pressed; 1=not pressed)
sbit Led2 = P2^1;     	//   P2.1 = diagnostic LED driver DS2 (1=light,0=dark)
sbit Led1 = P2^2;     	//   P2.2 = diagnostic LED driver DS1 (1=light,0=dark)
sbit P23spare = P2^3; 	//   P2.3 = spare, unconnected pin

extern float gSMBusClkFreq;
extern float gTimer1ClkFreq;
extern float gSysClkFreq;
extern uint8_t gI2Cflags;
#ifdef EXPLICIT_REPORT_ID
// OS24EVK-66 Firmware v1.1 2015-03-26 initial gOffset=1 EXPLICIT_REPORT_ID
# if 0 // regression test
uint8_t gOffset = 0; // EXPLICIT_REPORT_ID is defined
# else
uint8_t gOffset = 1; // EXPLICIT_REPORT_ID is defined
# endif
#else
// OS24EVK-66 Firmware v1.1 2015-03-24 initial gOffset=0, but changed to 1 in OUT_REPORT_HANDLER() EXPLICIT_REPORT_ID
uint8_t gOffset = 0; // EXPLICIT_REPORT_ID is not defined
#endif


//------------------------------
//
// MAX30101 optical sensor device driver
//
//------------------------------

uint8_t blockArr[192];
uint8_t gOS21ready = 0; // TODO1: OS24EVK-66 could gOS21ready be a bit instead of uint8_t ? conserve xdata space
uint8_t gOS21numIntServiced = 0;
#if SUPPORT_LIS2DH
uint8_t gLIS2DHnumIntServiced = 0; // VERIFY: OS24EVK-66 global counter for lis2dh numReportsSent
#endif // SUPPORT_LIS2DH

// https://jira.maxim-ic.com/browse/OS24EVK-24 Refactor: HID report enum and wrappers
// Add INT0Enable() firmware diagnostic to generate Mock HID reports:
uint8_t gMockHIDFIFOChannels = 0;
uint8_t gMockHIDFIFOChannel = 0;
uint16_t gMockHIDFIFOCh1 = 0x100;
uint16_t gMockHIDFIFOCh2 = 0x200;
uint16_t gMockHIDFIFOCh3 = 0x300;
uint16_t gMockHIDFIFOCh4 = 0x400;
#if SUPPORT_LIS2DH
uint8_t gMockHIDreport2 = 0; // VERIFY: OS24EVK-66 firmware diagnostic Mock HID REPORT_ID(2)
uint16_t gMockHID2X = 0x1020;
uint16_t gMockHID2Y = 0x3040;
uint16_t gMockHID2Z = 0x5060;
#endif // SUPPORT_LIS2DH

#if 1 // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service
uint8_t gLIS2DHready = 0;
#endif // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service


// ----------------------------------------------------------------------------
// External Function Prototypes
// ----------------------------------------------------------------------------

void i2c_write(uint8_t slaveAddress, uint8_t *pData, uint8_t lenData, uint8_t lenStartReg);
void i2c_read(uint8_t slaveAddress, uint8_t *pData, uint8_t lenData, uint8_t lenStartReg);
int16_t clearSDA(void);
void Timer0_Init(uint8_t delayUnitsAreMicroseconds);
void Timer1_Init(void);
void Timer2_Init(void);
void T0_Wait(uint16_t delayCount); // number of half-milliseconds or microseconds to wait, depending on previous Timer0_Init(mode)
#if SUPPORT_SPI
void SPI_mode0(uint8_t, uint8_t, uint8_t, uint8_t*);
void SPI_mode1(uint8_t, uint8_t*);
void SPI_mode2(uint8_t, uint8_t*);
#endif // SUPPORT_SPI

// ----------------------------------------------------------------------------
// OUT_REPORT_HANDLER()
// ----------------------------------------------------------------------------
// This handler processes the output report sent from the host.
//-----------------------------------------------------------------------------
void OUT_REPORT_HANDLER(int16_t internalCall) 
{
	uint8_t temp;

// VERIFY: OS24EVK-66 shouldn't gOffset initial value be 1 because of EXPLICIT_REPORT_ID ?
// gOffset is never changed anywhere else; this should be the initial value.
// OS24EVK-66 Firmware v1.1 2015-03-24 initial gOffset=0, but changed to 1 in OUT_REPORT_HANDLER() EXPLICIT_REPORT_ID
#ifdef EXPLICIT_REPORT_ID
	gOffset = 1;
#endif
   // IO_BUFFER.Ptr[0] must be HID_REPORT_ID_1 OUT from host?
   
	switch (IO_BUFFER.Ptr[0+gOffset])
	{
		case (0): //Firmware version  -- HRMonitorForm1.cs string HIDversion()
			// read -- myHID.FirmwareVersion(out bytVerMajor, out VerMinor, out verYearHundreds, out verYear, out verMonth, out verDay)
			IO_BUFFER.Ptr[0] = SHORT_REPORT_ID; // HID_REPORT_ID_1
			IO_BUFFER.Ptr[0+gOffset] = 0; // error status
			IO_BUFFER.Ptr[1+gOffset] = versionInfo.major;
			IO_BUFFER.Ptr[2+gOffset] = versionInfo.minor;
			IO_BUFFER.Ptr[3+gOffset] = (versionInfo.year & 0xFF00) >> 8;	// year is an int, but we can only send bytes, so need to split it
			IO_BUFFER.Ptr[4+gOffset] = versionInfo.year & 0xFF;
			IO_BUFFER.Ptr[5+gOffset] = versionInfo.month;
			IO_BUFFER.Ptr[6+gOffset] = versionInfo.day;
			SendPacket(); // no need to check for internal call since only the GUI will request a read through this function
			break;
		case (1): //LED (C51F321 P2.2=red, P2.1=green)
			switch (IO_BUFFER.Ptr[1+gOffset]) 
			{
				case (0):	// write -- myHID.LEDSet(xxxxxxP22P21)
					Led2 = IO_BUFFER.Ptr[2+gOffset] & 1; // sbit Led2 = P2^1;
					Led1 = (IO_BUFFER.Ptr[2+gOffset] & 2) >> 1; // sbit Led1 = P2^2;
					break;
				case (1):	// read -- myHID.GPIOP0Get(out xxxxxxP06P07) Firmware 2015-04-28: GPIOP0Get(P0byte) for INT_AC testing
					Timer0_Init(TIMER0_INIT_DELAY_HALFMS);
					T0_Wait(IO_BUFFER.Ptr[2+gOffset] * 256 + IO_BUFFER.Ptr[3+gOffset]);	// variable delay to check asynchronous read
					IO_BUFFER.Ptr[0] = SHORT_REPORT_ID; // HID_REPORT_ID_1
					IO_BUFFER.Ptr[0+gOffset] = 0; // error status
					//~ IO_BUFFER.Ptr[1+gOffset] = ((P0 & 0x80) >> 7) + ((P0 & 0x40) >> 5);
					IO_BUFFER.Ptr[1+gOffset] = P0; // Why all the fuss about scrambling P0 bits? Can I see the P0.0 and P0.1 interrupt pin states?
					SendPacket(); // no need to check for internal call since only the GUI will request a read through this function
					break;
			}
			break;
		case (2): //GPIO configuration (C51F321 P1 and P2)
			switch (IO_BUFFER.Ptr[1+gOffset]) 
			{
				case (0):	// write -- myHID.GPIOP1P2ConfigSet(P1MDOUT, P2MDOUT, weakPullupDisable)
					P1MDOUT = IO_BUFFER.Ptr[2+gOffset];	// P1 push-pull (1) or open-collector (0)
					P2MDOUT = IO_BUFFER.Ptr[3+gOffset];	// P2 push-pull (1) or open-collector (0)
					(IO_BUFFER.Ptr[4+gOffset] & 1) ? (XBR1 |= 0x80) : (XBR1 &= 0x7F);	// weak pull up disable (open collector only); 1 disabled, 0 enabled
					break;
				case (1):	// read -- myHID.GPIOP1P2ConfigGet(out P1MDOUT, out P2MDOUT, out weakPullupDisable)
					IO_BUFFER.Ptr[0] = SHORT_REPORT_ID; // HID_REPORT_ID_1
					IO_BUFFER.Ptr[0+gOffset] = 0; // error status					
					IO_BUFFER.Ptr[1+gOffset] = P1MDOUT;	// P1 push-pull (1) or open-collector (0)
					IO_BUFFER.Ptr[2+gOffset] = P2MDOUT;	// P2 push-pull (1) or open-collector (0)
					IO_BUFFER.Ptr[3+gOffset] = (XBR1 & 0x80) >> 7; // weakPullupDisable
					SendPacket(); // no need to check for internal call since only the GUI will request a read through this function
					break;
			}
			break;
		case (3): //GPIO value (C51F321 P1 and P2)
			switch (IO_BUFFER.Ptr[1+gOffset]) 
			{
				case (0):	// write -- myHID.GPIOP1P2Out(P1, P2)
					P1 = IO_BUFFER.Ptr[2+gOffset];		// P1 HI (1) or LO (0); set P1==1 and P1MDOUT==1 for HI-Z
					P2 = IO_BUFFER.Ptr[3+gOffset];		// P2 HI (1) or LO (0); set P2==1 and P1MDOUT==2 for HI-Z
					break;
				case (1):	// read -- myHID.GPIOP1P2In(out P1, out P2)
					temp = XBR1;
					if (IO_BUFFER.Ptr[2+gOffset] & 1)	// enable weak pullups in case GP pins are open-collector and not connected to anything (which would falsely give '0')
					{
						XBR1 &= 0x7F;
						Timer0_Init(TIMER0_INIT_DELAY_HALFMS);
						T0_Wait(2);
					}
					IO_BUFFER.Ptr[0] = SHORT_REPORT_ID; // HID_REPORT_ID_1
					IO_BUFFER.Ptr[0+gOffset] = 0; 		// error status
					IO_BUFFER.Ptr[1+gOffset] = P1;		// P1 HI (1) or LO (0)
					IO_BUFFER.Ptr[2+gOffset] = P2;		// P2 HI (1) or LO (0)
					XBR1 = temp;
					SendPacket(); // no need to check for internal call since only the GUI will request a read through this function
					break;
			}
			break;
		case (4): //I2C configuration
			switch (IO_BUFFER.Ptr[1+gOffset])
			{
				case 0:	// write -- myHID.I2CConfigSet(gI2Cflags, EXTHOLD, ClearSDAbyTogglingSCL)
					gI2Cflags = IO_BUFFER.Ptr[2+gOffset];
					// bit0 == 1: repeated start (versus stop/start) after write before read (applies to random read only)
					// bit1 == 1: start random read with a write, but end the write right away without sending reg address (emulate Jungo dongle for debug purposes)
					// bit2 == 1: repeat transaction if slave NACKs (suggest not to use this)
					// all flags are OR'd
					(IO_BUFFER.Ptr[3+gOffset] & 1) ? (SMB0CF |= 0x10) : (SMB0CF &= 0xEF);	// set EXTHOLD bit (SMBus setup / hold time extension)
					if (IO_BUFFER.Ptr[4+gOffset] & 1)	// clear SDA by toggling SCL
					{
						IO_BUFFER.Ptr[0] = SHORT_REPORT_ID; // HID_REPORT_ID_1
						IO_BUFFER.Ptr[0+gOffset] = 0; 	// transaction error status
						IO_BUFFER.Ptr[1+gOffset] = clearSDA();	// clearSDA error status
						SendPacket();			// send status of clearing SDA to host
					}
					break;
				case 1:	//read -- myHID.I2CConfigGet(out config)
					IO_BUFFER.Ptr[0] = SHORT_REPORT_ID; // HID_REPORT_ID_1
					IO_BUFFER.Ptr[0+gOffset] = 0; // error status					
					IO_BUFFER.Ptr[1+gOffset] = gI2Cflags;
					IO_BUFFER.Ptr[2+gOffset] = (SMB0CF & 0x10) >> 4; // EXTHOLD
					SendPacket(); // no need to check for internal call since only the GUI will request a read through this function
					break;
			}
			break;
		case (5): //I2C clock rate (number of counts to overflow)
			switch (IO_BUFFER.Ptr[1+gOffset])  
			{
				case (0):	// write -- myHID.I2CClockSet(ReloadTH1)
					gSMBusClkFreq = gTimer1ClkFreq / 3.0 / IO_BUFFER.Ptr[2+gOffset];	//GUI sends the number of counts to overflow; HID must calculate the desired SMBus clock frequency
					TR1 = 0;
					Timer1_Init();
					break;
				case (1): 	// read -- myHID.I2CClockGet(out TimerClockMHz, out ReloadTH1)
					IO_BUFFER.Ptr[0] = SHORT_REPORT_ID; // HID_REPORT_ID_1
					IO_BUFFER.Ptr[0+gOffset] = 0; // error status
					IO_BUFFER.Ptr[1+gOffset] = gTimer1ClkFreq / 1000000 / 3;	//return the SMBus timer's clock frequency (in MHz)
					IO_BUFFER.Ptr[2+gOffset] = 256-TH1;						//and return the SMBus timer's count value in order to calculate the SMBus clock frequency; TH1 is the reload value that gets loaded into TL0 upon overflow; the reload value is 256-TH1 since (0)-TH1 gives the proper number of counts to overflow
					SendPacket(); // no need to check for internal call since only the GUI will request a read through this function
					break;
			}
			break;
		case (6):	// I2C transaction -- HID.cs void writeI2C() readI2C()
			switch (IO_BUFFER.Ptr[1+gOffset] & 1)
			{
				case (0):	// write -- myHID.I2CWrite(deviceAddress, numDataBytes, numRegBytes, data[], reg[], ignoreNACK)
					i2c_write(/* slaveAddress */ IO_BUFFER.Ptr[1+gOffset] & 0xFE, /* uint8_t *pData */ IO_BUFFER.Ptr, /* lenData */ IO_BUFFER.Ptr[2+gOffset], /* lenStartReg */ IO_BUFFER.Ptr[3+gOffset]); 
					// note that start register address & data start at IO_BUFFER.Ptr[4]
					break;
				case (1):	// read -- myHID.I2CRead(deviceAddress, numDataBytes, numRegBytes, readData[], reg[], ignoreNACK)
					i2c_read(/* slaveAddress */ IO_BUFFER.Ptr[1+gOffset] & 0xFE, /* uint8_t *pData */ IO_BUFFER.Ptr, /* lenData */ IO_BUFFER.Ptr[2+gOffset], /* lenStartReg */ IO_BUFFER.Ptr[3+gOffset]);
					// note that start register address starts at IO_BUFFER.Ptr[4]
					break;
			}
			if (!internalCall)
				SendPacket(); // don't send status to host if startup or switch is causing the I2C write
			break;
		case (7):	// SPI config
			switch (IO_BUFFER.Ptr[1+gOffset]) //check if read (1) or write (0) 
			{
				case (0):	// write -- myHID.SPIConfigSet(config)
#if SUPPORT_SPI
					SPI0CN &= 0xFE;	// Disable SPI before setting clock phase and polarity
					(IO_BUFFER.Ptr[2+gOffset] & 0x1) ? (SPI0CFG |= 0x10) : (SPI0CFG &= 0xEF);	// set CKPOL; 0 means idle LO
					((IO_BUFFER.Ptr[2+gOffset] & 0x2) >> 1) ? (SPI0CFG |= 0x20) : (SPI0CFG &= 0xDF);	// set CKPHA; 0 means latch data when clock goes to the opposite polarity of idle
					((IO_BUFFER.Ptr[2+gOffset] & 0x4) >> 2)	? (SPI0CN |= 0x04) : (SPI0CN &= 0xFB);	// set NSSMD0
					SPI0CN |= 0x01;
#endif // SUPPORT_SPI
					break;
				case (1):	// read -- myHID.SPIConfigGet(out config)
#if SUPPORT_SPI
					IO_BUFFER.Ptr[0] = SHORT_REPORT_ID; // HID_REPORT_ID_1
					IO_BUFFER.Ptr[0+gOffset] = 0; // error status
					IO_BUFFER.Ptr[1+gOffset] = ((SPI0CFG & 0x30) >> 4) + (SPI0CN & 4) ;	// return NSSMD0, CKPHA, CKPOL in bits 2, 1, 0								
#else // SUPPORT_SPI
					IO_BUFFER.Ptr[0] = SHORT_REPORT_ID; // HID_REPORT_ID_1
					IO_BUFFER.Ptr[0+gOffset] = 0xFA; // error status -- failure, SPI not supported
#endif // SUPPORT_SPI
					SendPacket(); // no need to check for internal call since only the GUI will request a read through this function
					break;
			}
			break;
		case (8): 	// SPI clock rate
			switch (IO_BUFFER.Ptr[1+gOffset]) //check if read (1) or write (0) 
			{
				case (0):	// write -- myHID.SPIClockSet(SPI0CKR)
#if SUPPORT_SPI
					SPI0CKR = IO_BUFFER.Ptr[2+gOffset];
#endif // SUPPORT_SPI
					break;
				case (1): 	// read -- myHID.SPIClockGet(out SPI0CKR)
#if SUPPORT_SPI
					IO_BUFFER.Ptr[0] = SHORT_REPORT_ID; // HID_REPORT_ID_1
					IO_BUFFER.Ptr[0+gOffset] = 0; // error status
					IO_BUFFER.Ptr[1+gOffset] = gSysClkFreq / 1000000;					// return system clock freq (in MHz)
					IO_BUFFER.Ptr[2+gOffset] = SPI0CKR;	// return SPI0CKR
#else // SUPPORT_SPI
					IO_BUFFER.Ptr[0] = SHORT_REPORT_ID; // HID_REPORT_ID_1
					IO_BUFFER.Ptr[0+gOffset] = 0xFA; // error status -- failure, SPI not supported
					IO_BUFFER.Ptr[1+gOffset] = gSysClkFreq / 1000000;					// return system clock freq (in MHz)
					IO_BUFFER.Ptr[2+gOffset] = SPI0CKR;	// return SPI0CKR
#endif // SUPPORT_SPI
					SendPacket(); // no need to check for internal call since only the GUI will request a read through this function		
					break;
			}
			break;
		case (9):	// SPI transaction
			temp = IO_BUFFER.Ptr[2+gOffset];
			switch (IO_BUFFER.Ptr[1+gOffset])	// SPI type 
			{
#if SUPPORT_SPI
				case (0):
					// void SPI_mode0(uint8_t phase_change, uint8_t phase_change_byte, uint8_t num_bytes, uint8_t *dataPtr)
					// IO_BUFFER.Ptr[1+gOffset] -- 0 for SPI_mode0: multi-byte SPI transfer.
					// IO_BUFFER.Ptr[2+gOffset] -- not used
					// IO_BUFFER.Ptr[3+gOffset] -- phase_change // !=0 enable changing the clock phase. some slaves change phase between write/read
					// IO_BUFFER.Ptr[4+gOffset] -- phase_change_byte // byte index where phase change should happen
					// IO_BUFFER.Ptr[5+gOffset] -- num_bytes
					// IO_BUFFER.Ptr[6+gOffset] -- not used
					// IO_BUFFER.Ptr[7+gOffset] -- first byte of data starts on the (7 + offset) byte
					SPI_mode0(IO_BUFFER.Ptr[3+gOffset], IO_BUFFER.Ptr[4+gOffset], IO_BUFFER.Ptr[5+gOffset], IO_BUFFER.Ptr);
					break;
				case (1):
					// IO_BUFFER.Ptr[1+gOffset] -- 1 for SPI_mode1: two-byte SPI transfer. read flag: enable changing the clock phase on first byte and changing phase between first and second byte
					// IO_BUFFER.Ptr[2+gOffset] -- read flag
					// IO_BUFFER.Ptr[3+gOffset] -- not used
					// IO_BUFFER.Ptr[4+gOffset] -- not used
					// IO_BUFFER.Ptr[5+gOffset] -- not used; num_bytes = 2
					// IO_BUFFER.Ptr[6+gOffset] -- not used
					// IO_BUFFER.Ptr[7+gOffset] -- first byte of data starts on the (7 + offset) byte
					SPI_mode1(temp, IO_BUFFER.Ptr);
					break;
				case (2):
					// IO_BUFFER.Ptr[1+gOffset] --2 for SPI_mode2: two-byte SPI transfer. read flag: enable changing the clock phase on first byte and sampling the LSb after the second byte read
					// IO_BUFFER.Ptr[2+gOffset] -- read flag
					// IO_BUFFER.Ptr[3+gOffset] -- not used
					// IO_BUFFER.Ptr[4+gOffset] -- not used
					// IO_BUFFER.Ptr[5+gOffset] -- not used; num_bytes = 2
					// IO_BUFFER.Ptr[6+gOffset] -- not used
					// IO_BUFFER.Ptr[7+gOffset] -- first byte of data starts on the (7 + offset) byte
					SPI_mode2(temp, IO_BUFFER.Ptr);
					break;
#endif // SUPPORT_SPI
			}
			if (!internalCall && temp)
				SendPacket();
			break;
		case (23): //enable INT0 -- HRMonitorForm1.cs WriteOS24StartConfig()
			// https://jira.maxim-ic.com/browse/OS24EVK-24 Refactor: HID report enum and wrappers
			// Add INT0Enable() firmware diagnostic to generate Mock HID reports
			gMockHIDFIFOChannel = 0;
			gMockHIDFIFOCh1 = 0x100;
			gMockHIDFIFOCh2 = 0x200;
			gMockHIDFIFOCh3 = 0x300;
			gMockHIDFIFOCh4 = 0x400;
			gOS21numIntServiced = 0;
#if SUPPORT_LIS2DH
         gLIS2DHnumIntServiced = 0; // VERIFY: OS24EVK-66 global counter for lis2dh numReportsSent
         gMockHID2X = 0x1020;
         gMockHID2Y = 0x3040;
         gMockHID2Z = 0x5060;
#endif // SUPPORT_LIS2DH
			switch(IO_BUFFER.Ptr[1+gOffset])
			{
			case 0: //  -- myHID.INT0Enable(0) disabled (EX0=0)
			case 1: //  -- myHID.INT0Enable(1) real hardware (EX0=1)
			default:
				gMockHIDFIFOChannels = 0;
#if SUPPORT_LIS2DH
				gMockHIDreport2 = 0; // VERIFY: OS24EVK-66 firmware diagnostic Mock HID REPORT_ID(2)
#endif // SUPPORT_LIS2DH
				EX0 = IO_BUFFER.Ptr[1+gOffset];	// Enable INT0 interrupt
#if SUPPORT_LIS2DH 
            // TODO1: OS24EVK-66 INT0Enable() handler call lis2dh_configure_FIFO()
            if (IO_BUFFER.Ptr[1+gOffset] == 0) {
               //  -- myHID.INT0Enable(0) disabled (EX0=0)
               lis2dh_configure_FIFO(/* fifoModeFM10 0:Bypass */ 0, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
#if 1 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
               // TODO1: OS24EVK-66 OS24EVK-83 enable LIS2DH interrupt INT1: EX1 = 1
               EX1 = 0;
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
            }
            else {
               //  -- myHID.INT0Enable(1) real hardware (EX0=1)
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
               // TODO: configure lis2dh_CTRL_REG1 without modifying ODR field
               lis2dh_RegWrite(lis2dh_CTRL_REG1, 0x57); // magic? ODR=100Hz LPen=0 Zen=1 Yen=1 Xen=1
               lis2dh_RegWrite(lis2dh_CTRL_REG2, 0x01); // magic? HPIS1=1
               lis2dh_RegWrite(lis2dh_CTRL_REG3, 0x06); // magic? I1_WTM=1 I1_OVERRUN=1
               lis2dh_RegWrite(lis2dh_CTRL_REG4, 0x88); // magic? BDU=1 FS=2G HR=1
               lis2dh_RegWrite(lis2dh_CTRL_REG5, 0x01); // magic? FIFO_EN=0 D4D_INT2=1
               //~ lis2dh_RegWrite(lis2dh_CTRL_REG6, 0x22); // magic? I2_INT2=1 H_LACTIVE=1
               lis2dh_RegWrite(lis2dh_INT1_CFG, 0x7F); // magic? 6 direction movement recognition
               lis2dh_RegWrite(lis2dh_INT1_THS, 0x04); // magic? threshold
               lis2dh_RegWrite(lis2dh_INT1_DURATION, 0x00); // magic? duration
               lis2dh_RegWrite(lis2dh_INT2_CFG, 0x7F); // magic? 6 direction movement recognition
               lis2dh_RegWrite(lis2dh_INT2_THS, 0x04); // magic? threshold
               lis2dh_RegWrite(lis2dh_INT2_DURATION, 0x00); // magic? duration
               // TODO1: (2015-04-27) apparently reading INT1_SOURCE disrupts the accelerometer OUT_Y data stream.
               lis2dh_RegRead(lis2dh_INT1_SOURCE, 0 /* discard reg read value */ ); //!< clear INT by reading INT1_SRC register (read-only) 0 IA ZH ZL YH YL XH XL
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
#if 1 // TODO: lis2dh_configure_FIFO(LIS2DH_DEFAULT_FIFOMODE) when max30101_RegWrite(RegAddressEnum.ModeConfiguration,...)
# ifdef LIS2DH_DEFAULT_FIFOMODE
               lis2dh_configure_FIFO(/* fifoModeFM10 1:FIFO mode */ LIS2DH_DEFAULT_FIFOMODE, /* fifoTR */ 0, LIS2DH_DEFAULT_FIFOWATERMARKLEVEL);
# else
               lis2dh_configure_FIFO(/* fifoModeFM10 1:FIFO mode */ 1, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
# endif
#endif // TODO: lis2dh_configure_FIFO(LIS2DH_DEFAULT_FIFOMODE) when max30101_RegWrite(RegAddressEnum.ModeConfiguration,...)
//
// (Sometimes)
// (X) Missed first USB HID2 report. _lastHID1ReportCount=0, _lastHID2ReportCount=-0,
// This report's numInterruptsServiced=1
//
// INT_AC duty cycle is low(active) most of the time. 
// Should be briefly active while interrupt pending, then inactive(high) while FIFO is buffering.
//
// Why INT_AC stuck active(low) starting 150ms before first INT_OS pulse?
// INT_AC goes low after write 0x2e FIFO_CTRL_REG with 0x8A 2:Stream Mode, watermark 10 samples
// How long does it take before FIFO overrun?
// With 0x2E=0x8A FIFO watermark level 10 and ODR 100Hz, overflow after 110ms 
// With 0x2E=0x9F FIFO watermark level 32 and ODR 100Hz, overflow after 320ms 
//
//   LIS2DH.RegRead(0x20 CTRL_REG1, 0x57);
//      ODR = 0x05(100 Hz), LPen = 0, Zen = 1, Yen = 1, Xen = 1
//   LIS2DH.RegRead(0x21 CTRL_REG2, 0x00);
//      HPM = 0 Normal, HPCF = 0, FDS = 0, HPCLICK = 0, HPIS2 = 0, HPIS1 = 0
//   LIS2DH.RegRead(0x22 CTRL_REG3, 0x06);
//      I1_CLICK = 0, I1_AOI1 = 0, I1_AOI2 = 0,
//      I1_DRDY1 = 0, I1_DRDY2 = 0, I1_WTM = 1, I1_OVERRUN = 1
//   LIS2DH.RegRead(0x23 CTRL_REG4, 0x88);
//      BDU = 1, BLE = 0, FS = 0(±2G)
//      HR = 1, ST1 = 0, ST0 = 0, SIM = 0
//   LIS2DH.RegRead(0x24 CTRL_REG5, 0x48);
//      BOOT = 0, FIFO_EN = 1
//      LIR_INT1 = 1, D4D_INT1 = 0, LIR_INT2 = 0, D4D_INT2 = 0
//   LIS2DH.RegRead(0x25 CTRL_REG6, 0x02);
//      I2_CLICKen = 0, I2_INT1 = 0, I2_INT2 = 0, BOOT_I2 = 0
//      P2_ACT = 0, H_LACTIVE = 1
//   LIS2DH.RegRead(0x2e FIFO_CTRL_REG, 0x8a);
//      FM = 2(Stream mode), TR = 0, FTH = 0x0a(level 10)
//   LIS2DH.RegRead(0x2f FIFO_SRC_REG, 0xdf);
//      WTM = 1, OVRN_FIFO = 1, EMPTY = 0, FSS = 0x1f(level 31)
//   LIS2DH.RegRead(0x31 INT1_SOURCE, 0x00);
//   LIS2DH.RegRead(0x35 INT2_SOURCE, 0x00);
//
#if 1 // TODO: lis2dh_configure_FIFO(LIS2DH_DEFAULT_FIFOMODE) when max30101_RegWrite(RegAddressEnum.ModeConfiguration,...)
      // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
               // TODO1: OS24EVK-66 OS24EVK-83 enable LIS2DH interrupt INT1: EX1 = 1
               EX1 = 1;
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
            }
#endif
				break;
			case 2: //  -- myHID.INT0Enable(2) Mock HID x1 channel    101, 102, 103, ...
				gMockHIDFIFOChannels = 1;
				EX0 = 1; // Enable INT0 interrupt
				break;
			case 3: //  -- myHID.INT0Enable(3) Mock HID x2 channels   101, 201, 102, 202, 103, 203, ...
				gMockHIDFIFOChannels = 2;
				EX0 = 1; // Enable INT0 interrupt
				break;
			case 4: //  -- myHID.INT0Enable(4) Mock HID x3 channels   101, 201, 301, 102, 202, 302, 103, 203, 303, ...
				gMockHIDFIFOChannels = 3;
				EX0 = 1; // Enable INT0 interrupt
				break;
			case 5: //  -- myHID.INT0Enable(5) Mock HID x4 channels   101, 201, 301, 401, 102, 202, 302, 402, 103, 203, 303, 403, ...
				gMockHIDFIFOChannels = 4;
				EX0 = 1; // Enable INT0 interrupt
				break;
#if SUPPORT_LIS2DH
			case 6: //  -- myHID.INT0Enable(6) Mock HID REPORT_ID(2) x3 channels (X,Y,Z) 101, 201, 301, 102, 202, 302, 103, 203, 303, ...
				gMockHIDreport2 = 1; // VERIFY: OS24EVK-66 firmware diagnostic Mock HID REPORT_ID(2)
				EX0 = 1; // Enable INT0 interrupt
				break;
#endif // SUPPORT_LIS2DH
			}
			break;
	}
}

void Setup_IO_BUFFER(void)
{
   IO_BUFFER.Ptr = IO_PACKET;
#ifdef EXPLICIT_REPORT_ID
	IO_BUFFER.Length = 64;
#else	
	IO_BUFFER.Length = SHORT_REPORT_COUNT;
#endif
}

//-----------------------------------------------------------------------------
// External Interrupt 0 Interrupt Service Routine (ISR)
//-----------------------------------------------------------------------------
// MAX30101 Optical Sensor interrupt
//   P0.1 = MAX30101 INT (INT0 interrupt source)
//
void INT0_ISR (void) interrupt INTERRUPT_INT0 /* interrupt 0 */       // INT0 ISR
{
	gOS21ready = 1;
#if 1 // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service
   // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service
   gLIS2DHready = 1;
#endif // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service
}

//-----------------------------------------------------------------------------
// External Interrupt 1 Interrupt Service Routine (ISR)
//-----------------------------------------------------------------------------
// Accelerometer interrupt
//   P0.0 = Accelerometer INT (optional INT1 interrupt source)
//
#if SUPPORT_LIS2DH
void INT1_ISR (void) interrupt INTERRUPT_INT1 /* interrupt 2 */
{
   // OS24EVK-66 stub ISR for INT2 (C8051F321 P0.0 = STMicro LIS2DH INT1)
   
   // Either MAX30101 or Accelerometer interrupt,
   // will both trigger the same interrupt service routine
   // which reads the FIFO contents of both devices.
#if 1 // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service
   // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service
   gLIS2DHready = 1;
#else // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service
	gOS21ready = 1;
#endif // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service

}
#endif // SUPPORT_LIS2DH

void testOS21IntTiming()
{
#if SUPPORT_LIS2DH
	// uint8_t inner; // warning: unreferenced local variable
#else
	uint8_t inner; // warning: unreferenced local variable
#endif
	uint8_t outer;
	gOS21ready = 0;
	// clear interrupts (not checking brownout)
	blockArr[5] = 0;	// int
	i2c_read(gMAX30101deviceAddress & 0xFE, blockArr, 1, 1);
	blockArr[5] = 5;
	for (outer=0; outer<60; outer++) {
		i2c_read(gMAX30101deviceAddress & 0xFE, blockArr, 1, 1);
	}
	SendPacket();
}

//-----------------------------------------------------------------------------
// Either MAX30101 or Accelerometer interrupt,
// will both trigger the same interrupt service routine
// which reads the FIFO contents of both devices.
//
// LIS2DH and MAX30101 internal clocks are not synchronous, so could have different FIFO levels.
// Algorithms will low-pass filter etc. so don't care if output data rates are not exactly equal/synchronous.
//
void readOS21()
{
   // MAX30101: each sample is 3 bytes per channel
#define MAX30101_Bytes_Per_Channel 3
	uint8_t inner;
	uint8_t outer;
	uint8_t ptrDelta;
   // VERIFY: OS24EVK-72 HID report 1 byte 3 = MAX30101_InterruptStatus1 (not MAX30101_OverFlowCounter)
	// uint8_t ovf;
   uint8_t MAX30101_InterruptStatus1_value;
	uint8_t bytesRemaining;
	bit singleSample = 0;
	bit singleReport = 1;	// only return 60 bytes (10x 3-byte red and IR samples)
#if SUPPORT_MAX30101
   // VERIFY: OS24EVK-66 Replace int with uint8_t where applicable; conserve xdata space
   uint8_t numBytesToRequestFromOpticalFIFO;
#else
   #define numBytesToRequestFromOpticalFIFO \
   (singleSample                                            \
      ? (2 * MAX30101_Bytes_Per_Channel)                    \
      : (singleReport                                       \
         ? (2 * MAX30101_Bytes_Per_Channel * 10)            \
         : (ptrDelta * 2 * MAX30101_Bytes_Per_Channel)      \
         )                                                  \
      )

#endif // SUPPORT_MAX30101
#if 0 // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service
#if 1 // OS24EVK-66 ISR Poll for 0x27 STATUS_REG2 bit 0x08 ZYXDA
   // TODO1: OS24EVK-66 ISR Poll for 0x27 STATUS_REG2 bit 0x08 ZYXDA new data available when reading X,Y,Z
   uint8_t lis2dh_STATUS_REG2_value = 0; //!< (read-only) ZYXOR ZOR YOR XOR ZYXDA ZDA YDA XDA
#if 1 // OS24EVK-66 Firmware regresssion 2015-04-16_v1.1 (svn r=15232) vs 2015-04-15_v1.1 (svn r=15222): crashes GUI and Console program on connect (API ReadFile error).
   // 2015-04-22 Yikes! This declaration alone is enough to break it.
   // Are we overloading some internal resource?
   // 2015-04-23 restored lis2dh_FIFO_SRC_REG_value, after compacting xdata by 6 bytes...
   //   2015-04-15_v1.1 (svn r=15222) Program Size: data=10.5 xdata=484 code=12705
   //   2015-04-16_v1.1 (svn r=15306) Program Size: data=10.5 xdata=478 code=12687
   // So that could mean the dragon still lurks, 5 bytes away...
   uint8_t lis2dh_FIFO_SRC_REG_value /* = 0 */ ; //!< (read-only) WTM OVRN_FIFO EMPTY FSS4 FSS3 FSS2 FSS1 FSS0
#endif // OS24EVK-66 Firmware regresssion 2015-04-16_v1.1 (svn r=15232) vs 2015-04-15_v1.1 (svn r=15222): crashes GUI and Console program on connect (API ReadFile error).
#endif // OS24EVK-66 ISR Poll for 0x27 STATUS_REG2 bit 0x08 ZYXDA
#endif // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service
   //
   // C not C++, so all local variables must be declared before any statements
   //
	if (singleSample) {
		singleReport = 1;	//singleSample must be in a singleReport
	}
	gOS21ready = 0;
	// clear interrupts (not checking brownout)
   
#if SUPPORT_MAX30101
   numBytesToRequestFromOpticalFIFO = (singleSample 
      ? (2 * MAX30101_Bytes_Per_Channel) 
      : (singleReport 
         ? (2 * MAX30101_Bytes_Per_Channel * 10) 
         : (ptrDelta * 2 * MAX30101_Bytes_Per_Channel)
         )
      );
#endif // SUPPORT_MAX30101
	
#if SUPPORT_MAX30101
   // read MAX30101 FIFO into blockArr[2..]
   if (gMAX30101deviceAddress != 0) {
# if 0
// TODO1: OS24EVK-66 i2c_smbus_read_bytes_data not working?
      // I2C Burst Read starting from Device Register 0, length 2
      //	Device Register 0 MAX30101_InterruptStatus1
      //	Device Register 1 MAX30101_InterruptStatus2
      i2c_smbus_read_bytes_data(
         /* uint8_t deviceAddress */ gMAX30101deviceAddress, 
         /* uint8_t regAddress */ MAX30101_InterruptStatus1, 
         /* uint8_t count */ 2, 
         /* uint8_t *pRegValue */ 0);
      
      // I2C Burst Read starting from Device Register 4, length 3
      //	Device Register 4 MAX30101_FIFOWritePointer
      //	Device Register 5 MAX30101_OverFlowCounter
      //	Device Register 6 MAX30101_FIFOReadPointer
      i2c_smbus_read_bytes_data(
         /* uint8_t deviceAddress */ gMAX30101deviceAddress, 
         /* uint8_t regAddress */ MAX30101_FIFOWritePointer, 
         /* uint8_t count */ 3, 
         /* uint8_t *pRegValue */ 0);
# else
      // I2C Burst Read starting from Device Register 0, length 1
      //	Device Register 0 Interrupt Status 1
      blockArr[5] = MAX30101_InterruptStatus1;	// int MAX30101_InterruptStatus1
      i2c_read(gMAX30101deviceAddress & 0xFE, blockArr, 2, 1);	
      // VERIFY: OS24EVK-72 HID report 1 byte 3 = MAX30101_InterruptStatus1 (not MAX30101_OverFlowCounter)
      MAX30101_InterruptStatus1_value = blockArr[2+0];
	
      // I2C Burst Read starting from Device Register 4, length 3
      //	Device Register 4 FIFO Write Pointer
      //	Device Register 5 Overflow Counter
      //	Device Register 6 FIFO Read Pointer
      blockArr[5] = 4;	// write ptr, ovf, read ptr
      i2c_read(gMAX30101deviceAddress & 0xFE, blockArr, 3, 1);	
# endif
      // reg0x08 sets the number of empty FIFO slots (in 32 sample FIFO) until interrupt. 
      // If reg0x08=0, then on the 1st iteration, the writeptr will wrap back to 0, which is where the readptr is. ptrdelta=0.
      // If reg0x08=1, then on the 1st iteration, the writeptr will be 31 and readptr=0. ptrdelta=31.
      // 
      //	blockArr[2+0] = value read from Device Register 4 FIFO Write Pointer
      //	blockArr[2+1] = value read from Device Register 5 Overflow Counter
      //	blockArr[2+2] = value read from Device Register 6 FIFO Read Pointer
      ptrDelta = (blockArr[2+0] - blockArr[2+2])&0x1F;	// FIFO ptrs are 5 bits, but HID can only read 10 samples per report
      // VERIFY: OS24EVK-72 HID report 1 byte 3 = MAX30101_InterruptStatus1 (not MAX30101_OverFlowCounter)
      //ovf = blockArr[2+1]; // value read from Device Register 5 MAX30101_OverFlowCounter
      
      // I2C Burst Read starting from Device Register 7, length (singleSample ? (6) : (singleReport ? (60) : (ptrDelta * 6)))
      // 	Device Register 7 FIFO Data Register
      //	This register streams out FIFO data, without auto-incrementing i2c register address
      blockArr[5] = 7; // Device Register 7 FIFO Data Register
      //FIXME must use block read on FIFO as follows otherwise data is corrupted
      i2c_read(
         /* slaveAddress */ (gMAX30101deviceAddress & 0xFE), 
         /* uint8_t *pData */ blockArr, 
         /* lenData */ numBytesToRequestFromOpticalFIFO, 
         /* lenStartReg */ 1);
   }
#else // SUPPORT_MAX30101
      // I2C Burst Read starting from Device Register 0, length 1
      //	Device Register 0 Interrupt Status 1
      blockArr[5] = 0;	// int
      i2c_read(gMAX30101deviceAddress & 0xFE, blockArr, 2, 1);	
	
      // I2C Burst Read starting from Device Register 4, length 3
      //	Device Register 4 FIFO Write Pointer
      //	Device Register 5 Overflow Counter
      //	Device Register 6 FIFO Read Pointer
      blockArr[5] = 4;	// write ptr, ovf, read ptr
      i2c_read(gMAX30101deviceAddress & 0xFE, blockArr, 3, 1);	
      // reg0x08 sets the number of empty FIFO slots (in 32 sample FIFO) until interrupt. 
      // If reg0x08=0, then on the 1st iteration, the writeptr will wrap back to 0, which is where the readptr is. ptrdelta=0.
      // If reg0x08=1, then on the 1st iteration, the writeptr will be 31 and readptr=0. ptrdelta=31.
      // 
      //	blockArr[2+0] = value read from Device Register 4 FIFO Write Pointer
      //	blockArr[2+1] = value read from Device Register 5 Overflow Counter
      //	blockArr[2+2] = value read from Device Register 6 FIFO Read Pointer
      ptrDelta = (blockArr[2+0] - blockArr[2+2])&0x1F;	// FIFO ptrs are 5 bits, but HID can only read 10 samples per report
      ovf = blockArr[2+1]; // value read from Device Register 5 MAX30101_OverFlowCounter
      
      // I2C Burst Read starting from Device Register 7, length (singleSample ? (6) : (singleReport ? (60) : (ptrDelta * 6)))
      // 	Device Register 7 FIFO Data Register
      //	This register streams out FIFO data, without auto-incrementing i2c register address
      blockArr[5] = 7; // Device Register 7 FIFO Data Register
      //FIXME must use block read on FIFO as follows otherwise data is corrupted
      i2c_read(
         /* slaveAddress */ (gMAX30101deviceAddress & 0xFE), 
         /* uint8_t *pData */ blockArr, 
         /* lenData */ numBytesToRequestFromOpticalFIFO, 
         /* lenStartReg */ 1);
#endif // SUPPORT_MAX30101
	// firmware diagnostic to generate Mock HID REPORT_ID(1) data
	if (gMockHIDFIFOChannels > 0) {
		// Add INT0Enable() firmware diagnostic to generate Mock HID reports
		// Overwrite blockArr[2+inner...] with specified Mock data for testing
		// INT0Enable(2) Mock HID x1 channel    101, 102, 103, ...
		// INT0Enable(3) Mock HID x2 channels   101, 201, 102, 202, 103, 203, ...
		// INT0Enable(4) Mock HID x3 channels   101, 201, 301, 102, 202, 302, 103, 203, 303, ...
		//
		// OS24EVK-29 Erroneous FIFO data readout in LED mode (Green)
		// Based on what the chip actually does, it appears that in LED 3-channel mode
		// where the HID report can only hold 6 and 2/3 samples, we end up
		// overlapping (resending) the 2/3 partial sample in the next report.
		// Based on the preliminary data sheet, I expected the partial sample readout
		// would have incremented the FIFO read pointer, but apparently not. That's good!
		// So change the mock HID diagnostic to match what the part actually does.
		gMockHIDFIFOChannel  = 0;
		//
		for (inner = 0; inner < numBytesToRequestFromOpticalFIFO; inner += MAX30101_Bytes_Per_Channel) {
			switch(gMockHIDFIFOChannel) {
				case 0:
					gMockHIDFIFOCh1++;
					blockArr[2+inner] = 0; // (gMockHIDFIFOCh1 >>16) & 0xFF; // High byte
					blockArr[3+inner] = (gMockHIDFIFOCh1 >> 8) & 0xFF; // Mid byte
					blockArr[4+inner] = (gMockHIDFIFOCh1 >> 0) & 0xFF; // Low byte
					break;
				case 1:
					gMockHIDFIFOCh2++;
					blockArr[2+inner] = 0; // (gMockHIDFIFOCh2 >>16) & 0xFF; // High byte
					blockArr[3+inner] = (gMockHIDFIFOCh2 >> 8) & 0xFF; // Mid byte
					blockArr[4+inner] = (gMockHIDFIFOCh2 >> 0) & 0xFF; // Low byte
					break;
				case 2:
					gMockHIDFIFOCh3++;
					blockArr[2+inner] = 0; // (gMockHIDFIFOCh3 >>16) & 0xFF; // High byte
					blockArr[3+inner] = (gMockHIDFIFOCh3 >> 8) & 0xFF; // Mid byte
					blockArr[4+inner] = (gMockHIDFIFOCh3 >> 0) & 0xFF; // Low byte
					break;
				case 3:
					gMockHIDFIFOCh4++;
					blockArr[2+inner] = 0; // (gMockHIDFIFOCh4>>16) & 0xFF; // High byte
					blockArr[3+inner] = (gMockHIDFIFOCh4 >> 8) & 0xFF; // Mid byte
					blockArr[4+inner] = (gMockHIDFIFOCh4 >> 0) & 0xFF; // Low byte
					break;
			}
			// gMockHIDFIFOChannel = (gMockHIDFIFOChannel + 1) % gMockHIDFIFOChannels;
			gMockHIDFIFOChannel = gMockHIDFIFOChannel + 1;
			if (gMockHIDFIFOChannel >= gMockHIDFIFOChannels) {
				gMockHIDFIFOChannel  = 0;
			}
		}
	}
	//
   // Send HID REPORT_ID(1) from blockArr[2..]
#if SUPPORT_MAX30101
   // TODO1: OS24EVK-66 if MAX30101 absent but (gMockHIDFIFOChannels > 0), still send HID report 1
   if (gMAX30101deviceAddress == 0) 
   {
      ptrDelta = 26; // mock value number of samples
      gMockHIDFIFOChannels = 3;
   }
   if ((gMAX30101deviceAddress != 0) || (gMockHIDFIFOChannels > 0)) 
#else
   // if (gMAX30101deviceAddress != 0) 
   if (1) 
#endif
   {
      // warning: if using ptrdelta to specify how many samples to read, do not read more than len(blockArr) bytes, otherwise memory access violation will occur
      //i2c_read(gMAX30101deviceAddress & 0xFE, blockArr, ptrDelta * 6, 1);
      IO_BUFFER.Ptr[0] = SHORT_REPORT_ID; // HID_REPORT_ID_1
      IO_BUFFER.Ptr[1] = gOS21numIntServiced++;
      IO_BUFFER.Ptr[2] = ptrDelta;
      // VERIFY: OS24EVK-72 HID report 1 byte 3 = MAX30101_InterruptStatus1 (not MAX30101_OverFlowCounter)
      IO_BUFFER.Ptr[3] = MAX30101_InterruptStatus1_value; // instead of ovf value read from Device Register 5 MAX30101_OverFlowCounter
      for (inner=0; inner<(singleSample ? 6 : 60); inner++) {
         IO_BUFFER.Ptr[4+inner] = blockArr[2+inner];
      }
// VERIFY: OS24EVK-66 USB_SUPPORT_SendPacketWaitEP1INready=1 SendPacketWaitEP1INready() as SendPacket() with polling for INPRTY=0.
#if USB_SUPPORT_SendPacketWaitEP1INready
      SendPacketWaitEP1INready();
#else // USB_SUPPORT_SendPacketWaitEP1INready
      SendPacket();
#endif // USB_SUPPORT_SendPacketWaitEP1INready
      if (!singleReport)
      {
         if (ptrDelta == 0) {
            ptrDelta = 32;
         }
         for (outer=0; outer<ptrDelta/10; outer++)	// integer division
         {
            bytesRemaining = ptrDelta*6-(outer+1)*60;	// each full loop sends 60 bytes: (outer+1)*60
            for (inner=0; inner<(bytesRemaining <= 60 ? bytesRemaining : 60); inner++) {
               IO_BUFFER.Ptr[4+inner] = blockArr[2+inner+(outer+1)*60];
            }
// VERIFY: OS24EVK-66 USB_SUPPORT_SendPacketWaitEP1INready=1 SendPacketWaitEP1INready() as SendPacket() with polling for INPRTY=0.
#if USB_SUPPORT_SendPacketWaitEP1INready
            SendPacketWaitEP1INready();
#else // USB_SUPPORT_SendPacketWaitEP1INready
            SendPacket();
#endif // USB_SUPPORT_SendPacketWaitEP1INready
         }
      }
   }
   //
#if 0 // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service
   // moved to readLIS2DH()
   //
   // VERIFY: OS24EVK-66 update *F3xx_USB0_ReportHandler.c* Interrupt Service Routine readOS21() to acquire accelerometer data and send a second HID report. 
   //
#if SUPPORT_LIS2DH
   // read LIS2DH FIFO into blockArr[2..]
#define LIS2DH_Bytes_Per_Channel 2
   if (gLIS2DHdeviceAddress != 0) {
      // TODO: OS24EVK-66 read LIS2DH FIFO into blockArr[2..]
#if 1
      
      // TODO: OS24EVK-66 HID2 LIS2DH ptrDelta = 0x2F FIFO_SRC_REG bits FFS4:0
      lis2dh_RegRead(lis2dh_FIFO_SRC_REG, &lis2dh_FIFO_SRC_REG_value); //!< (read-only) WTM OVRN_FIFO EMPTY FSS4 FSS3 FSS2 FSS1 FSS0
      // ptrDelta = (lis2dh_FIFO_SRC_REG_value & 0x1F);
      ptrDelta = lis2dh_FIFO_SRC_REG_value; // diagnostic: give me all the 0x2F FIFO_SRC_REG bits 
      
#if 1 // OS24EVK-66 ISR Poll for 0x27 STATUS_REG2 bit 0x08 ZYXDA      
      // TODO1: OS24EVK-66 ISR Poll for 0x27 STATUS_REG2 bit 0x08 ZYXDA new data available when reading X,Y,Z
      do {
         lis2dh_RegRead(lis2dh_STATUS_REG2, &lis2dh_STATUS_REG2_value);
      } while ((lis2dh_STATUS_REG2_value & 0x08) != 0x08);
#endif // OS24EVK-66 ISR Poll for 0x27 STATUS_REG2 bit 0x08 ZYXDA
      
      // TODO1: OS24EVK-66 Read LIS2DH XYZ FIFO using a single 60-byte I2C read operation in readOS21() // read LIS2DH FIFO into blockArr[2..]
      #define numBytesToRequestFromLIS2DHFIFO 60
      //	This register streams out FIFO data, without auto-incrementing i2c register address
      blockArr[5] = lis2dh_OUT_X_L | 0x80; // start of FIFO register 
      // 2015-04-16 OS24EVK-66 OS24EVK-57 LIS2DH multiple-byte read requires (regAddress | 0x80)
      // LIS2DH data sheet pg 25 5.1.1 I2C operation (near last paragraph)
      // In order to read multiple bytes, it is necessary to assert the most significant bit of the subaddress
      // field. In other words, SUB(7) must be equal to 1 while SUB(6-0) represents the
      // address of first register to be read.
      //
      // Note this raw XYZ data is LITTLE-ENDIAN (least significant byte first)
      //
      //FIXME must use block read on FIFO as follows otherwise data is corrupted
      i2c_read(
         /* slaveAddress */ (gLIS2DHdeviceAddress & 0xFE), 
         /* uint8_t *pData */ blockArr, 
         /* lenData */ numBytesToRequestFromLIS2DHFIFO, 
         /* lenStartReg */ 1);
#else
      // mockup: read real X,Y,Z vector even if not from the FIFO
      // VERIFY: OS24EVK-66 Restore the HID2 mock data diagnostic, verify no missing HID2 reports
      lis2dh_Read_XYZ(&gMockHID2X, &gMockHID2Y, &gMockHID2Z);
		for (inner=0; inner<(singleSample ? (6) : (singleReport ? (60) : (ptrDelta * 6))); inner += LIS2DH_Bytes_Per_Channel) {
			switch(gMockHIDFIFOChannel) {
				case 0: // mock LIS2DH lis2dh_OUT_X_H,lis2dh_OUT_X_L
					blockArr[2+inner] = (gMockHID2X >> 8) & 0xFF; // Mid byte
					blockArr[3+inner] = (gMockHID2X >> 0) & 0xFF; // Low byte
					break;
				case 1: // mock LIS2DH lis2dh_OUT_Y_H,lis2dh_OUT_Y_L
					blockArr[2+inner] = (gMockHID2Y >> 8) & 0xFF; // Mid byte
					blockArr[3+inner] = (gMockHID2Y >> 0) & 0xFF; // Low byte
					break;
				case 2: // mock LIS2DH lis2dh_OUT_Z_H,lis2dh_OUT_Z_L
					blockArr[2+inner] = (gMockHID2Z >> 8) & 0xFF; // Mid byte
					blockArr[3+inner] = (gMockHID2Z >> 0) & 0xFF; // Low byte
					break;
			}
			gMockHIDFIFOChannel = gMockHIDFIFOChannel + 1;
			if (gMockHIDFIFOChannel >= 3) {
				gMockHIDFIFOChannel  = 0;
            lis2dh_Read_XYZ(&gMockHID2X, &gMockHID2Y, &gMockHID2Z);
			}
      }
#endif

      // TODO1: Reset LIS2DH INT_ACC interrupt by reading INT1_SOURCE register
      // (2015-04-26) apparently reading INT1_SOURCE into lis2dh_FIFO_SRC_REG_value breaks the accelerometer data stream
      //~ lis2dh_RegRead(lis2dh_INT1_SOURCE, &lis2dh_FIFO_SRC_REG_value); //!< clear INT by reading INT1_SRC register (read-only) 0 IA ZH ZL YH YL XH XL
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
      // TODO1: (2015-04-27) apparently reading INT1_SOURCE disrupts the accelerometer OUT_Y data stream.
      lis2dh_RegRead(lis2dh_INT1_SOURCE, 0 /* discard reg read value */ ); //!< clear INT by reading INT1_SRC register (read-only) 0 IA ZH ZL YH YL XH XL
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH

   }
	// firmware diagnostic to generate Mock HID REPORT_ID(2) data
	if (gMockHIDreport2 > 0) {
      // TODO1: OS24EVK-66 firmware diagnostic Mock HID REPORT_ID(2) 
		// Add INT0Enable() firmware diagnostic to generate Mock HID reports
		// Overwrite blockArr[2+inner...] with specified Mock data for testing
		// INT0Enable(6) Mock HID REPORT_ID(2) x3 channels (X,Y,Z) 101, 201, 301, 102, 202, 302, 103, 203, 303, ...
		//
		gMockHIDFIFOChannel  = 0;
		//
		// LIS2DH: each sample is 2 bytes per channel
		for (inner=0; inner<(singleSample ? (6) : (singleReport ? (60) : (ptrDelta * 6))); inner += LIS2DH_Bytes_Per_Channel) {
			switch(gMockHIDFIFOChannel) {
				case 0: // mock LIS2DH lis2dh_OUT_X_H,lis2dh_OUT_X_L
					gMockHID2X++;
					blockArr[2+inner] = (gMockHID2X >> 8) & 0xFF; // Mid byte
					blockArr[3+inner] = (gMockHID2X >> 0) & 0xFF; // Low byte
					break;
				case 1: // mock LIS2DH lis2dh_OUT_Y_H,lis2dh_OUT_Y_L
					gMockHID2Y++;
					blockArr[2+inner] = (gMockHID2Y >> 8) & 0xFF; // Mid byte
					blockArr[3+inner] = (gMockHID2Y >> 0) & 0xFF; // Low byte
					break;
				case 2: // mock LIS2DH lis2dh_OUT_Z_H,lis2dh_OUT_Z_L
					gMockHID2Z++;
					blockArr[2+inner] = (gMockHID2Z >> 8) & 0xFF; // Mid byte
					blockArr[3+inner] = (gMockHID2Z >> 0) & 0xFF; // Low byte
					break;
			}
			gMockHIDFIFOChannel = gMockHIDFIFOChannel + 1;
			if (gMockHIDFIFOChannel >= 3) {
				gMockHIDFIFOChannel  = 0;
			}
      }
   }
	//
   // Send HID REPORT_ID(2) from blockArr[2..]
   if (gLIS2DHdeviceAddress != 0) {
      //
      // VERIFY: OS24EVK-66 Send HID REPORT_ID(2)
      // warning: if using ptrdelta to specify how many samples to read, do not read more than len(blockArr) bytes, otherwise memory access violation will occur
      //i2c_read(gMAX30101deviceAddress & 0xFE, blockArr, ptrDelta * 6, 1);
      IO_BUFFER.Ptr[0] = HID_REPORT_ID_2;       // VERIFY: OS24EVK-66 need HID REPORT_ID(2)
      IO_BUFFER.Ptr[1] = gLIS2DHnumIntServiced++; // VERIFY: OS24EVK-66 global counter for lis2dh numReportsSent
      IO_BUFFER.Ptr[2] = lis2dh_FIFO_SRC_REG_value; // VERIFY: OS24EVK-66 HID2 LIS2DH ptrDelta = 0x2F FIFO_SRC_REG bits FFS4:0 
      IO_BUFFER.Ptr[3] = '2'; // DIAGNOSTIC lis2dh_reserved_HID2_03 this is really HID#2        ovf; // OverFlowCounter
      for (inner=0; inner<(singleSample ? 6 : 60); inner++) {
         IO_BUFFER.Ptr[4+inner] = blockArr[2+inner];
      }
// VERIFY: OS24EVK-66 USB_SUPPORT_SendPacketWaitEP1INready=1 SendPacketWaitEP1INready() as SendPacket() with polling for INPRTY=0.
#if USB_SUPPORT_SendPacketWaitEP1INready
      SendPacketWaitEP1INready();
#else // USB_SUPPORT_SendPacketWaitEP1INready
      SendPacket();
#endif // USB_SUPPORT_SendPacketWaitEP1INready
      if (!singleReport)
      {
         if (ptrDelta == 0) {
            ptrDelta = 32;
         }
         for (outer=0; outer<ptrDelta/10; outer++)	// integer division
         {
            bytesRemaining = ptrDelta*6-(outer+1)*60;	// each full loop sends 60 bytes: (outer+1)*60
            for (inner=0; inner<(bytesRemaining <= 60 ? bytesRemaining : 60); inner++) {
               IO_BUFFER.Ptr[4+inner] = blockArr[2+inner+(outer+1)*60];
            }
// VERIFY: OS24EVK-66 USB_SUPPORT_SendPacketWaitEP1INready=1 SendPacketWaitEP1INready() as SendPacket() with polling for INPRTY=0.
#if USB_SUPPORT_SendPacketWaitEP1INready
            SendPacketWaitEP1INready();
#else // USB_SUPPORT_SendPacketWaitEP1INready
            SendPacket();
#endif // USB_SUPPORT_SendPacketWaitEP1INready
         }
      }
      //
#if 1
      // FAILED: (2015-04-13_v1.1) OS24EVK-66 ISR explicitly re-enable FIFO mode by lis2dh_configure_FIFO(/* fifoModeFM10 1:FIFO mode */ 1, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
#if 1 // OS24EVK-66 Firmware regresssion 2015-04-16_v1.1 (svn r=15232) vs 2015-04-15_v1.1 (svn r=15222): crashes GUI and Console program on connect (API ReadFile error).
      lis2dh_RegRead(lis2dh_FIFO_SRC_REG, &lis2dh_FIFO_SRC_REG_value); //!< (read-only) WTM OVRN_FIFO EMPTY FSS4 FSS3 FSS2 FSS1 FSS0
      if ((lis2dh_FIFO_SRC_REG_value & 0x40) != 0) {
         // lis2dh_FIFO_SRC_REG bit OVRN_FIFO
         lis2dh_configure_FIFO(/* fifoModeFM10 0:Bypass mode */ 0, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
         lis2dh_Read_XYZ(&gMockHID2X, &gMockHID2Y, &gMockHID2Z);
# ifdef LIS2DH_DEFAULT_FIFOMODE
         lis2dh_configure_FIFO(/* fifoModeFM10 1:FIFO mode */ LIS2DH_DEFAULT_FIFOMODE, /* fifoTR */ 0, LIS2DH_DEFAULT_FIFOWATERMARKLEVEL);
# else
         lis2dh_configure_FIFO(/* fifoModeFM10 1:FIFO mode */ 1, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
# endif
      }
#else // OS24EVK-66 Firmware regresssion 2015-04-16_v1.1 (svn r=15232) vs 2015-04-15_v1.1 (svn r=15222): crashes GUI and Console program on connect (API ReadFile error).
      lis2dh_RegRead(lis2dh_FIFO_SRC_REG, &lis2dh_STATUS_REG2_value);
      if ((lis2dh_STATUS_REG2_value & 0x40) != 0) {
         // lis2dh_FIFO_SRC_REG bit OVRN_FIFO
         lis2dh_configure_FIFO(/* fifoModeFM10 0:Bypass mode */ 0, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
         lis2dh_Read_XYZ(&gMockHID2X, &gMockHID2Y, &gMockHID2Z);
# ifdef LIS2DH_DEFAULT_FIFOMODE
         lis2dh_configure_FIFO(/* fifoModeFM10 1:FIFO mode */ LIS2DH_DEFAULT_FIFOMODE, /* fifoTR */ 0, LIS2DH_DEFAULT_FIFOWATERMARKLEVEL);
# else
         lis2dh_configure_FIFO(/* fifoModeFM10 1:FIFO mode */ 1, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
# endif
      }
#endif // OS24EVK-66 Firmware regresssion 2015-04-16_v1.1 (svn r=15232) vs 2015-04-15_v1.1 (svn r=15222): crashes GUI and Console program on connect (API ReadFile error).
#endif
      //
   }
#endif // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service

#endif // SUPPORT_LIS2DH
} // readOS21() MAX30101 + Accelerometer FIFO readout interrupt 


#if 1 // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service
//-----------------------------------------------------------------------------
// Accelerometer interrupt,
// trigger the same interrupt service routine
// which reads the FIFO contents of LIS2DH.
//
// LIS2DH and MAX30101 internal clocks are not synchronous, so could have different FIFO levels.
// Algorithms will low-pass filter etc. so don't care if output data rates are not exactly equal/synchronous.
//
// when gLIS2DHready
void readLIS2DH()
{
   // MAX30101: each sample is 3 bytes per channel
#define MAX30101_Bytes_Per_Channel 3
	uint8_t inner;
	uint8_t outer;
	uint8_t ptrDelta;
   // VERIFY: OS24EVK-72 HID report 1 byte 3 = MAX30101_InterruptStatus1 (not MAX30101_OverFlowCounter)
	// uint8_t ovf;
   //~ uint8_t MAX30101_InterruptStatus1_value;
	uint8_t bytesRemaining;
	bit singleSample = 0;
	bit singleReport = 1;	// only return 60 bytes (10x 3-byte red and IR samples)
//#if SUPPORT_MAX30101
//   // VERIFY: OS24EVK-66 Replace int with uint8_t where applicable; conserve xdata space
//   //~ uint8_t numBytesToRequestFromOpticalFIFO;
//#else
//   #define numBytesToRequestFromOpticalFIFO \
//   (singleSample                                            \
//      ? (2 * MAX30101_Bytes_Per_Channel)                    \
//      : (singleReport                                       \
//         ? (2 * MAX30101_Bytes_Per_Channel * 10)            \
//         : (ptrDelta * 2 * MAX30101_Bytes_Per_Channel)      \
//         )                                                  \
//      )
//
//#endif // SUPPORT_MAX30101
#if 1 // OS24EVK-66 ISR Poll for 0x27 STATUS_REG2 bit 0x08 ZYXDA
   // TODO1: OS24EVK-66 ISR Poll for 0x27 STATUS_REG2 bit 0x08 ZYXDA new data available when reading X,Y,Z
   uint8_t lis2dh_STATUS_REG2_value = 0; //!< (read-only) ZYXOR ZOR YOR XOR ZYXDA ZDA YDA XDA
#if 1 // OS24EVK-66 Firmware regresssion 2015-04-16_v1.1 (svn r=15232) vs 2015-04-15_v1.1 (svn r=15222): crashes GUI and Console program on connect (API ReadFile error).
   // 2015-04-22 Yikes! This declaration alone is enough to break it.
   // Are we overloading some internal resource?
   // 2015-04-23 restored lis2dh_FIFO_SRC_REG_value, after compacting xdata by 6 bytes...
   //   2015-04-15_v1.1 (svn r=15222) Program Size: data=10.5 xdata=484 code=12705
   //   2015-04-16_v1.1 (svn r=15306) Program Size: data=10.5 xdata=478 code=12687
   // So that could mean the dragon still lurks, 5 bytes away...
   uint8_t lis2dh_FIFO_SRC_REG_value /* = 0 */ ; //!< (read-only) WTM OVRN_FIFO EMPTY FSS4 FSS3 FSS2 FSS1 FSS0
#endif // OS24EVK-66 Firmware regresssion 2015-04-16_v1.1 (svn r=15232) vs 2015-04-15_v1.1 (svn r=15222): crashes GUI and Console program on connect (API ReadFile error).
#endif // OS24EVK-66 ISR Poll for 0x27 STATUS_REG2 bit 0x08 ZYXDA
   //
   // C not C++, so all local variables must be declared before any statements
   //
	if (singleSample) {
		singleReport = 1;	//singleSample must be in a singleReport
	}
   // TODO: 2015-05-02 singleReport = 1 unless HR mode, in readLIS2DH()
   // TODO: 2015-05-02 cache value of MAX30101_ModeConfiguration in gMAX30101_ModeConfiguration_value
   if (gMAX30101_ModeConfiguration_value == 0x02) {
      // "ModeConfiguration",0x09,0x02,"r/w Mode Configuration (0x80=SHDN, 0x40=RESET, 0x07=MODE[2:0]): HR mode"
      singleReport = 0;
   } else {
      // "ModeConfiguration",0x09,0x03,"r/w Mode Configuration (0x80=SHDN, 0x40=RESET, 0x07=MODE[2:0]): SPO2 mode"
      // "ModeConfiguration",0x09,0x07,"r/w Mode Configuration (0x80=SHDN, 0x40=RESET, 0x07=MODE[2:0]): LED mode"
   }
	gLIS2DHready = 0;
	// clear interrupts (not checking brownout)
   
   //
   // VERIFY: OS24EVK-66 update *F3xx_USB0_ReportHandler.c* Interrupt Service Routine readOS21() to acquire accelerometer data and send a second HID report. 
   //
#if SUPPORT_LIS2DH
   // read LIS2DH FIFO into blockArr[2..]
#define LIS2DH_Bytes_Per_Channel 2
   if (gLIS2DHdeviceAddress != 0) {
      // TODO: OS24EVK-66 read LIS2DH FIFO into blockArr[2..]
#if 1
      
      // TODO: OS24EVK-66 HID2 LIS2DH ptrDelta = 0x2F FIFO_SRC_REG bits FFS4:0
      lis2dh_RegRead(lis2dh_FIFO_SRC_REG, &lis2dh_FIFO_SRC_REG_value); //!< (read-only) WTM OVRN_FIFO EMPTY FSS4 FSS3 FSS2 FSS1 FSS0
      // ptrDelta = (lis2dh_FIFO_SRC_REG_value & 0x1F);
      ptrDelta = lis2dh_FIFO_SRC_REG_value; // diagnostic: give me all the 0x2F FIFO_SRC_REG bits 
      if ((lis2dh_FIFO_SRC_REG_value & 0x40) != 0) {
         // lis2dh_FIFO_SRC_REG bit OVRN_FIFO
#if 1 // turn on the Red LED to indicate FIFO overflow happened
         // VERIFY: OS24EVK-83 2015-05-01 turn on the Red LED to indicate FIFO overflow happened
         //   P2.2 = diagnostic LED driver DS1 (1=light,0=dark)
         Led1 = 1; // [RED]
         // Confirmed: in HR+XYZ mode, overflow does indeed happen.
#endif // turn on the Red LED to indicate FIFO overflow happened
      }
      
#if 0 // OS24EVK-66 ISR Poll for 0x27 STATUS_REG2 bit 0x08 ZYXDA      
      // TODO1: OS24EVK-66 ISR Poll for 0x27 STATUS_REG2 bit 0x08 ZYXDA new data available when reading X,Y,Z
      do {
         lis2dh_RegRead(lis2dh_STATUS_REG2, &lis2dh_STATUS_REG2_value);
      } while ((lis2dh_STATUS_REG2_value & 0x08) != 0x08);
#endif // OS24EVK-66 ISR Poll for 0x27 STATUS_REG2 bit 0x08 ZYXDA
      
      // TODO1: OS24EVK-66 Read LIS2DH XYZ FIFO using a single 60-byte I2C read operation in readOS21() // read LIS2DH FIFO into blockArr[2..]
      #define numBytesToRequestFromLIS2DHFIFO 60
      //	This register streams out FIFO data, without auto-incrementing i2c register address
      blockArr[5] = lis2dh_OUT_X_L | 0x80; // start of FIFO register 
      // 2015-04-16 OS24EVK-66 OS24EVK-57 LIS2DH multiple-byte read requires (regAddress | 0x80)
      // LIS2DH data sheet pg 25 5.1.1 I2C operation (near last paragraph)
      // In order to read multiple bytes, it is necessary to assert the most significant bit of the subaddress
      // field. In other words, SUB(7) must be equal to 1 while SUB(6-0) represents the
      // address of first register to be read.
      //
      // Note this raw XYZ data is LITTLE-ENDIAN (least significant byte first)
      //
      //FIXME must use block read on FIFO as follows otherwise data is corrupted
      i2c_read(
         /* slaveAddress */ (gLIS2DHdeviceAddress & 0xFE), 
         /* uint8_t *pData */ blockArr, 
         /* lenData */ numBytesToRequestFromLIS2DHFIFO, 
         /* lenStartReg */ 1);
#else
      // mockup: read real X,Y,Z vector even if not from the FIFO
      // VERIFY: OS24EVK-66 Restore the HID2 mock data diagnostic, verify no missing HID2 reports
      lis2dh_Read_XYZ(&gMockHID2X, &gMockHID2Y, &gMockHID2Z);
		for (inner=0; inner<(singleSample ? (6) : (singleReport ? (60) : (ptrDelta * 6))); inner += LIS2DH_Bytes_Per_Channel) {
			switch(gMockHIDFIFOChannel) {
				case 0: // mock LIS2DH lis2dh_OUT_X_H,lis2dh_OUT_X_L
					blockArr[2+inner] = (gMockHID2X >> 8) & 0xFF; // Mid byte
					blockArr[3+inner] = (gMockHID2X >> 0) & 0xFF; // Low byte
					break;
				case 1: // mock LIS2DH lis2dh_OUT_Y_H,lis2dh_OUT_Y_L
					blockArr[2+inner] = (gMockHID2Y >> 8) & 0xFF; // Mid byte
					blockArr[3+inner] = (gMockHID2Y >> 0) & 0xFF; // Low byte
					break;
				case 2: // mock LIS2DH lis2dh_OUT_Z_H,lis2dh_OUT_Z_L
					blockArr[2+inner] = (gMockHID2Z >> 8) & 0xFF; // Mid byte
					blockArr[3+inner] = (gMockHID2Z >> 0) & 0xFF; // Low byte
					break;
			}
			gMockHIDFIFOChannel = gMockHIDFIFOChannel + 1;
			if (gMockHIDFIFOChannel >= 3) {
				gMockHIDFIFOChannel  = 0;
            lis2dh_Read_XYZ(&gMockHID2X, &gMockHID2Y, &gMockHID2Z);
			}
      }
#endif

      // TODO1: Reset LIS2DH INT_ACC interrupt by reading INT1_SOURCE register
      // (2015-04-26) apparently reading INT1_SOURCE into lis2dh_FIFO_SRC_REG_value breaks the accelerometer data stream
      //~ lis2dh_RegRead(lis2dh_INT1_SOURCE, &lis2dh_FIFO_SRC_REG_value); //!< clear INT by reading INT1_SRC register (read-only) 0 IA ZH ZL YH YL XH XL
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
      // TODO1: (2015-04-27) apparently reading INT1_SOURCE disrupts the accelerometer OUT_Y data stream.
      lis2dh_RegRead(lis2dh_INT1_SOURCE, 0 /* discard reg read value */ ); //!< clear INT by reading INT1_SRC register (read-only) 0 IA ZH ZL YH YL XH XL
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH

   }
	// firmware diagnostic to generate Mock HID REPORT_ID(2) data
	if (gMockHIDreport2 > 0) {
      // TODO1: OS24EVK-66 firmware diagnostic Mock HID REPORT_ID(2) 
		// Add INT0Enable() firmware diagnostic to generate Mock HID reports
		// Overwrite blockArr[2+inner...] with specified Mock data for testing
		// INT0Enable(6) Mock HID REPORT_ID(2) x3 channels (X,Y,Z) 101, 201, 301, 102, 202, 302, 103, 203, 303, ...
		//
		gMockHIDFIFOChannel  = 0;
		//
		// LIS2DH: each sample is 2 bytes per channel
		for (inner=0; inner<(singleSample ? (6) : (singleReport ? (60) : (ptrDelta * 6))); inner += LIS2DH_Bytes_Per_Channel) {
			switch(gMockHIDFIFOChannel) {
				case 0: // mock LIS2DH lis2dh_OUT_X_H,lis2dh_OUT_X_L
					gMockHID2X++;
					blockArr[2+inner] = (gMockHID2X >> 8) & 0xFF; // Mid byte
					blockArr[3+inner] = (gMockHID2X >> 0) & 0xFF; // Low byte
					break;
				case 1: // mock LIS2DH lis2dh_OUT_Y_H,lis2dh_OUT_Y_L
					gMockHID2Y++;
					blockArr[2+inner] = (gMockHID2Y >> 8) & 0xFF; // Mid byte
					blockArr[3+inner] = (gMockHID2Y >> 0) & 0xFF; // Low byte
					break;
				case 2: // mock LIS2DH lis2dh_OUT_Z_H,lis2dh_OUT_Z_L
					gMockHID2Z++;
					blockArr[2+inner] = (gMockHID2Z >> 8) & 0xFF; // Mid byte
					blockArr[3+inner] = (gMockHID2Z >> 0) & 0xFF; // Low byte
					break;
			}
			gMockHIDFIFOChannel = gMockHIDFIFOChannel + 1;
			if (gMockHIDFIFOChannel >= 3) {
				gMockHIDFIFOChannel  = 0;
			}
      }
   }
	//
   // Send HID REPORT_ID(2) from blockArr[2..]
   if (gLIS2DHdeviceAddress != 0) {
      //
      // VERIFY: OS24EVK-66 Send HID REPORT_ID(2)
      // warning: if using ptrdelta to specify how many samples to read, do not read more than len(blockArr) bytes, otherwise memory access violation will occur
      //i2c_read(gMAX30101deviceAddress & 0xFE, blockArr, ptrDelta * 6, 1);
      IO_BUFFER.Ptr[0] = HID_REPORT_ID_2;       // VERIFY: OS24EVK-66 need HID REPORT_ID(2)
      IO_BUFFER.Ptr[1] = gLIS2DHnumIntServiced++; // VERIFY: OS24EVK-66 global counter for lis2dh numReportsSent
      IO_BUFFER.Ptr[2] = lis2dh_FIFO_SRC_REG_value; // VERIFY: OS24EVK-66 HID2 LIS2DH ptrDelta = 0x2F FIFO_SRC_REG bits FFS4:0 
      IO_BUFFER.Ptr[3] = '2'; // DIAGNOSTIC lis2dh_reserved_HID2_03 this is really HID#2        ovf; // OverFlowCounter
      for (inner=0; inner<(singleSample ? 6 : 60); inner++) {
         IO_BUFFER.Ptr[4+inner] = blockArr[2+inner];
      }
// VERIFY: OS24EVK-66 USB_SUPPORT_SendPacketWaitEP1INready=1 SendPacketWaitEP1INready() as SendPacket() with polling for INPRTY=0.
#if USB_SUPPORT_SendPacketWaitEP1INready
      SendPacketWaitEP1INready();
#else // USB_SUPPORT_SendPacketWaitEP1INready
      SendPacket();
#endif // USB_SUPPORT_SendPacketWaitEP1INready
      if (!singleReport)
      {
         if (ptrDelta == 0) {
            ptrDelta = 32;
         }
         for (outer=0; outer<ptrDelta/10; outer++)	// integer division
         {
            bytesRemaining = ptrDelta*6-(outer+1)*60;	// each full loop sends 60 bytes: (outer+1)*60
            for (inner=0; inner<(bytesRemaining <= 60 ? bytesRemaining : 60); inner++) {
               IO_BUFFER.Ptr[4+inner] = blockArr[2+inner+(outer+1)*60];
            }
// VERIFY: OS24EVK-66 USB_SUPPORT_SendPacketWaitEP1INready=1 SendPacketWaitEP1INready() as SendPacket() with polling for INPRTY=0.
#if USB_SUPPORT_SendPacketWaitEP1INready
            SendPacketWaitEP1INready();
#else // USB_SUPPORT_SendPacketWaitEP1INready
            SendPacket();
#endif // USB_SUPPORT_SendPacketWaitEP1INready
         }
      }
      //
#if 1
      // FAILED: (2015-04-13_v1.1) OS24EVK-66 ISR explicitly re-enable FIFO mode by lis2dh_configure_FIFO(/* fifoModeFM10 1:FIFO mode */ 1, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
#if 1 // OS24EVK-66 Firmware regresssion 2015-04-16_v1.1 (svn r=15232) vs 2015-04-15_v1.1 (svn r=15222): crashes GUI and Console program on connect (API ReadFile error).
      lis2dh_RegRead(lis2dh_FIFO_SRC_REG, &lis2dh_FIFO_SRC_REG_value); //!< (read-only) WTM OVRN_FIFO EMPTY FSS4 FSS3 FSS2 FSS1 FSS0
      if ((lis2dh_FIFO_SRC_REG_value & 0x40) != 0) {
         // lis2dh_FIFO_SRC_REG bit OVRN_FIFO
         lis2dh_configure_FIFO(/* fifoModeFM10 0:Bypass mode */ 0, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
         lis2dh_Read_XYZ(&gMockHID2X, &gMockHID2Y, &gMockHID2Z);
# ifdef LIS2DH_DEFAULT_FIFOMODE
         lis2dh_configure_FIFO(/* fifoModeFM10 1:FIFO mode */ LIS2DH_DEFAULT_FIFOMODE, /* fifoTR */ 0, LIS2DH_DEFAULT_FIFOWATERMARKLEVEL);
# else
         lis2dh_configure_FIFO(/* fifoModeFM10 1:FIFO mode */ 1, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
# endif
#if 1 // turn on the Red LED to indicate FIFO overflow happened
         // VERIFY: OS24EVK-83 2015-05-01 turn on the Red LED to indicate FIFO overflow happened
         //   P2.2 = diagnostic LED driver DS1 (1=light,0=dark)
         Led1 = 1; // [RED]
         // Confirmed: in HR+XYZ mode, overflow does indeed happen.
#endif // turn on the Red LED to indicate FIFO overflow happened
      }
#else // OS24EVK-66 Firmware regresssion 2015-04-16_v1.1 (svn r=15232) vs 2015-04-15_v1.1 (svn r=15222): crashes GUI and Console program on connect (API ReadFile error).
      lis2dh_RegRead(lis2dh_FIFO_SRC_REG, &lis2dh_STATUS_REG2_value);
      if ((lis2dh_STATUS_REG2_value & 0x40) != 0) {
         // lis2dh_FIFO_SRC_REG bit OVRN_FIFO
         lis2dh_configure_FIFO(/* fifoModeFM10 0:Bypass mode */ 0, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
         lis2dh_Read_XYZ(&gMockHID2X, &gMockHID2Y, &gMockHID2Z);
# ifdef LIS2DH_DEFAULT_FIFOMODE
         lis2dh_configure_FIFO(/* fifoModeFM10 1:FIFO mode */ LIS2DH_DEFAULT_FIFOMODE, /* fifoTR */ 0, LIS2DH_DEFAULT_FIFOWATERMARKLEVEL);
# else
         lis2dh_configure_FIFO(/* fifoModeFM10 1:FIFO mode */ 1, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
# endif
      }
#endif // OS24EVK-66 Firmware regresssion 2015-04-16_v1.1 (svn r=15232) vs 2015-04-15_v1.1 (svn r=15222): crashes GUI and Console program on connect (API ReadFile error).
#endif
      //
   }
}
#endif // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service

// OS24EVK-66 SMBusReadByte protocol I2C 8-bit register read
//
// @param[in] deviceAddress = I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.
// @param[in] regAddress = 8-bit register address
// @param[out] pRegValue = optional buffer to get the received data
// @post global blockarr is used
// @pre global gOffset = 0 or 1 depending on whether EXPLICIT_REPORT_ID
//
// @return -1 on NACK, or 1 on success
//
#if SUPPORT_i2c_smbus_read_byte_data
int8_t i2c_smbus_read_byte_data(uint8_t deviceAddress, uint8_t regAddress, uint8_t *pRegValue)
{
   uint8_t regValue;
   uint8_t status;
   //
   // C not C++, so all local variables must be declared before any statements
   //
   blockArr[4+gOffset] = regAddress;
   //  SMBusReadByte is i2c_read(deviceAddress, blockArray, 2, 1)
#if 1
   // OS24EVK-66 Firmware v1.1 2015-03-29 scope check; i2c_smbus_read_byte_data i2c_read with numDataBytes = 1 instead of 2.
   // VERIFY: OS24EVK-66 i2c_smbus_read_byte_data i2c_read with numDataBytes = 1 instead of 2, avoid unnecessary extra byte readout?
   // Looks like this doesn't work -- firmware doesn't find the devices?
   // Yet HID I2C commands do access the devices ok.
   // But then streaming data doesn't seem to generate HID reports,
   // despite being able to turn on the optical device LEDs.
   i2c_read(/* deviceAddress = I2C_ADDRESS */ (deviceAddress & 0xFE), 
   		blockArr, 
		/* lenData:numDataBytes = 2 */    1, // TODO1: OS24EVK-66 i2c_smbus_read_byte_data i2c_read with numDataBytes = 1 instead of 2, avoid unnecessary extra byte readout?
		/* lenStartReg:numRegBytes = 1 */ 1); 
   // after first call: blockArr[] = { 0x01, 0x00 status, 0x15 regValue, 0x01, 0x20, 0xFF, 0x02, 0x20, ... }
   status = blockArr[0+gOffset];
   regValue = blockArr[1+gOffset]; // readData[i] = IOBuf[i + 1];
#else
   // Initial address search works, but I don't understand why lenData:numDataBytes = 2 instead of 1.
   i2c_read(/* deviceAddress = I2C_ADDRESS */ (deviceAddress & 0xFE), 
   		blockArr, 
		/* lenData:numDataBytes = 2 */    2, 
		/* lenStartReg:numRegBytes = 1 */ 1); 
   // after first call: blockArr[] = { 0x01, 0x00 status, 0x15 regValue, 0x15 regValue, 0x20, 0xFF, 0x02, 0x20, ... }
   status = blockArr[0+gOffset];
   regValue = blockArr[2+gOffset]; // readData[i] = IOBuf[i + 2];
   // looks like thie 2+gOffset isn't really right for Byte Read.
#endif
   //	blockArr[2+0] = value read from Device Register
   //
   if (status != 0) {
      return -1; // NACK
   }
   //
   if (pRegValue != 0) {
      *(pRegValue) = regValue;
   }
   return 1; // ACK
}
#endif // SUPPORT_i2c_smbus_read_byte_data

#if SUPPORT_i2c_smbus_read_bytes_data
// OS24EVK-66 SMBusReadByte protocol I2C 8-bit register read, multiple consecutive bytes
//
// @param[in] deviceAddress = I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.
// @param[in] regAddress = 8-bit register address of first byte
// @param[in] count = number of consecutive register addresses to read
// @param[out] pRegValue = optional buffer to get the received data
// @post global blockarr is used
// @pre global gOffset = 0 or 1 depending on whether EXPLICIT_REPORT_ID
//
// @return -1 on NACK, or 1 on success
//
int8_t i2c_smbus_read_bytes_data(uint8_t deviceAddress, uint8_t regAddress, uint8_t count, uint8_t *pRegValue)
{
   uint8_t regValue;
   uint8_t status;
   //
   // C not C++, so all local variables must be declared before any statements
   //
   blockArr[4+gOffset] = regAddress;
   //  SMBusReadByte is i2c_read(deviceAddress, blockArray, 2, 1)
   i2c_read(/* deviceAddress = I2C_ADDRESS */ (deviceAddress & 0xFE), 
   		blockArr, 
		/* lenData:numDataBytes = 2 */    count,
		/* lenStartReg:numRegBytes = 1 */ 1); 
   status = blockArr[0+gOffset];
   regValue = blockArr[2+gOffset]; // readData[i] = IOBuf[i + 2];
   //	blockArr[2+0] = value read from Device Register
   //
   if (status != 0) {
      return -1; // NACK
   }
   //
   if (pRegValue != 0) {
      while (count > 0) {
         *(pRegValue+count) = blockArr[2+gOffset+count];
      }
   }
   return 1; // ACK
}
#endif // SUPPORT_i2c_smbus_read_bytes_data

// OS24EVK-66 SMBusWriteByte protocol I2C 8-bit register read
//
// @param[in] deviceAddress = I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.
// @param[in] regAddress = 8-bit register address
// @param[in] regValue = 8-bit register value
// @post global blockarr is used
// @pre global gOffset = 0 or 1 depending on whether EXPLICIT_REPORT_ID
//
// @return -1 on NACK, or 1 on success
//
#if SUPPORT_i2c_smbus_read_byte_data
int8_t i2c_smbus_write_byte_data(uint8_t deviceAddress, uint8_t regAddress, uint8_t regValue)
{
   uint8_t status;
   //
   // C not C++, so all local variables must be declared before any statements
   //
   blockArr[4+gOffset] = regAddress;
   blockArr[5+gOffset] = regValue;
   i2c_write(/* deviceAddress = I2C_ADDRESS */ (deviceAddress & 0xFE), 
   		blockArr, 
		/* lenData:numDataBytes = 1 */    1, 
		/* lenStartReg:numRegBytes = 1 */ 1); 
      // note that start register address & data start at IO_BUFFER.Ptr[4]
   status = blockArr[0+gOffset];
   //
   if (status != 0) {
      return -1; // NACK
   }
   return 1; // ACK
}
#endif // SUPPORT_i2c_smbus_read_byte_data


#if SUPPORT_LED1_LED2_DIAGNOSTIC
// OS24EVK-66 flashing diagnostic LEDs
void flash_Led1L_Led2L_100ms()
{
   Led1 = 0;
   Led2 = 0;
   Timer0_Init(TIMER0_INIT_DELAY_HALFMS);
   T0_Wait(200);
}
#endif // SUPPORT_LED1_LED2_DIAGNOSTIC

#if SUPPORT_LED1_LED2_DIAGNOSTIC
// OS24EVK-66 flashing diagnostic LEDs
void flash_Led1L_Led2H_100ms()
{
   Led1 = 0;
   Led2 = 1;
   Timer0_Init(TIMER0_INIT_DELAY_HALFMS);
   T0_Wait(200);
}
#endif // SUPPORT_LED1_LED2_DIAGNOSTIC

#if SUPPORT_LED1_LED2_DIAGNOSTIC
// OS24EVK-66 flashing diagnostic LEDs
void flash_Led1H_Led2L_100ms()
{
   Led1 = 1;
   Led2 = 0;
   Timer0_Init(TIMER0_INIT_DELAY_HALFMS);
   T0_Wait(200);
}
#endif // SUPPORT_LED1_LED2_DIAGNOSTIC

#if SUPPORT_LED1_LED2_DIAGNOSTIC
// OS24EVK-66 flashing diagnostic LEDs
void flash_Led1H_Led2H_100ms()
{
   Led1 = 1;
   Led2 = 1;
   Timer0_Init(TIMER0_INIT_DELAY_HALFMS);
   T0_Wait(200);
}
#endif // SUPPORT_LED1_LED2_DIAGNOSTIC

#if SUPPORT_LED1_LED2_DIAGNOSTIC
void flash_Led1Led2_East()
{
   flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
   flash_Led1L_Led2H_100ms(); // [   ] [GREEN]
   flash_Led1L_Led2L_100ms(); // [   ] [     ]
   flash_Led1H_Led2L_100ms(); // [RED] [     ]
   flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
}
#endif // SUPPORT_LED1_LED2_DIAGNOSTIC

#if SUPPORT_LED1_LED2_DIAGNOSTIC
void flash_Led1Led2_Blink()
{
   flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
   flash_Led1L_Led2L_100ms(); // [   ] [     ]
   flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
}
#endif // SUPPORT_LED1_LED2_DIAGNOSTIC

#if SUPPORT_P20BTN_DIAGNOSTIC
//   P2.0 = diagnostic pushbutton on C8051F321 daughtercard TOOLSTICK321DC (0=pressed; 1=not pressed)
int16_t P20Btn_Pressed()
{
   return !P20Btn;
}
#endif // SUPPORT_P20BTN_DIAGNOSTIC


