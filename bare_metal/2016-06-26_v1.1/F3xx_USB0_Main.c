//-----------------------------------------------------------------------------
// F3xx_USB_Main.c
//-----------------------------------------------------------------------------
// Copyright 2014
//
// Program Description:
//
// HID firmware for Display Buffer Debug evaluation board.
//
//
// FID:            3XX000006
// Target:         C8051F32x/C8051F340
// Tool chain:     Keil C51 7.50 / Keil EVAL C51
//                 Silicon Laboratories IDE version 2.6
// Project Name:   Generic HID Firmware
//
//
// Release 1.0
//   	-140401 Initial Revision (Tushar Mazumder)
//
// Update main program in this file.


/*
WARNING L15: MULTIPLE CALL TO SEGMENT warnings are due to possible I2C calls in main and in the USB ISR. However, this should never happen. 
Disabling USB interrupts can prevent this from happening, but the warnings still persist.
Disable variable overlaying in linker to prevent these warnings. 
Memory model in compiler may then need to be set to large because of address space overflow.
*/
//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------

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

#include "c8051f3xx.h"
#include "F3xx_USB0_Register.h"
#include "F3xx_Initialize.h"
#include "F3xx_USB0_InterruptServiceRoutine.h"
#include "F3xx_USB0_Descriptor.h"
#include "F3xx_USB0_ReportHandler.h"


// ----------------------------------------------------------------------------
// Global Variable Declarations
// ----------------------------------------------------------------------------

VersionStructure versionInfo;
extern float gSMBusClkFreq;
extern uint8_t gOS21ready; // TODO1: OS24EVK-66 could gOS21ready be a bit instead of uint8_t ? conserve xdata space
#if 1 // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service
extern uint8_t gLIS2DHready;
#endif // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service


// ----------------------------------------------------------------------------
// Local Function Prototypes
// ----------------------------------------------------------------------------

void updateVersion(VersionStructure*);

//-----------------------------------------------------------------------------
// Main Routine
//-----------------------------------------------------------------------------
void main(void)
{
	gSMBusClkFreq = 400000.0;			//must come before system_init, since SMBus clock is set there
	System_Init ();
	updateVersion(&versionInfo);
#if SUPPORT_LED1_LED2_DIAGNOSTIC
   //
   // OS24EVK-66 flashing diagnostic LEDs on C8051F321 init
   flash_Led1Led2_East();
   //~ flash_Led1Led2_East();
   //~ flash_Led1Led2_East();
#endif // SUPPORT_LED1_LED2_DIAGNOSTIC
   //
   // Verify attached devices respond on I2C bus
#if SUPPORT_MAX30100
   max30100_open();
#endif // SUPPORT_MAX30100
#if SUPPORT_MAX30101
   max30101_open();
#endif // SUPPORT_MAX30101
#if SUPPORT_LIS2DH
   // OS24EVK-66 Global LIS2DH device address gLIS2DHdeviceAddress
   lis2dh_open();
#endif // SUPPORT_LIS2DH
#if SUPPORT_MAX7311
   max7311_open();
#endif // SUPPORT_MAX7311
   //
   // VERIFY: Flash diagnostic LEDs to indicate which devices were detected
#if SUPPORT_MAX30100
   if (gMAX30100deviceAddress != 0) {
      // MAX30100 is found, as expected
   } else {
      // MAX30100 is not connected.
      //
#if SUPPORT_LED1_LED2_DIAGNOSTIC
      flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
      flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2H_100ms(); // [   ] [GREEN]
      flash_Led1L_Led2L_100ms(); // [   ] [     ]
      flash_Led1H_Led2L_100ms(); // [RED] [     ]
      flash_Led1L_Led2L_100ms(); // [   ] [     ]
      flash_Led1H_Led2L_100ms(); // [RED] [     ]
      flash_Led1L_Led2L_100ms(); // [   ] [     ]
      flash_Led1H_Led2L_100ms(); // [RED] [     ]
      flash_Led1L_Led2L_100ms(); // [   ] [     ]
      flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
      flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
      
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
#endif // SUPPORT_LED1_LED2_DIAGNOSTIC
   }
#endif // SUPPORT_MAX30100
#if SUPPORT_MAX30101
   if (gMAX30101deviceAddress != 0) {
      // MAX30101 is found, as expected
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
   } else {
      // MAX30101 is not connected.
      // This is only likely if using bare C8051F321 daughterboard,
      // or if MAX30101+ACCEL daughterboard is not connected
      //
#if SUPPORT_LED1_LED2_DIAGNOSTIC
      flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
      flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2H_100ms(); // [   ] [GREEN]
      flash_Led1L_Led2L_100ms(); // [   ] [     ]
      flash_Led1H_Led2L_100ms(); // [RED] [     ]
      flash_Led1L_Led2L_100ms(); // [   ] [     ]
      flash_Led1H_Led2L_100ms(); // [RED] [     ]
      flash_Led1L_Led2L_100ms(); // [   ] [     ]
      flash_Led1H_Led2L_100ms(); // [RED] [     ]
      flash_Led1L_Led2L_100ms(); // [   ] [     ]
      flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
      flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
      
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
#endif // SUPPORT_LED1_LED2_DIAGNOSTIC
   }
#endif // SUPPORT_MAX30101
#if SUPPORT_LIS2DH
   if (gLIS2DHdeviceAddress != 0) {
      // LIS2DH Accelerometer is found
#if SUPPORT_LED1_LED2_DIAGNOSTIC
      flash_Led1Led2_East();
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
      //~ flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
      //~ flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
      //~ flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
      //~ flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
#endif // SUPPORT_LED1_LED2_DIAGNOSTIC
   } else {
      // LIS2DH Accelerometer is not found
      // MAX30101EVKIT classic
      //
#if SUPPORT_LED1_LED2_DIAGNOSTIC
      flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
      flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
      flash_Led1L_Led2L_100ms(); // [   ] [     ]
      flash_Led1L_Led2H_100ms(); // [   ] [GREEN]
      flash_Led1L_Led2L_100ms(); // [   ] [     ]
      flash_Led1L_Led2H_100ms(); // [   ] [GREEN]
      flash_Led1L_Led2L_100ms(); // [   ] [     ]
      flash_Led1L_Led2H_100ms(); // [   ] [GREEN]
      flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
      //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
      flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
      flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
#endif // SUPPORT_LED1_LED2_DIAGNOSTIC
      
   }
#endif // SUPPORT_LIS2DH
#if SUPPORT_MAX7311
   if (gMAX7311deviceAddress != 0) {
      // diagnostic: MAX7311 I/O expander attached to I2C bus
	   max7311_RegWrite(max7311_Port0700Output, 0xAA); // bitmap of output pin drivers
	   max7311_RegWrite(max7311_Port0700Output, 0x55); // bitmap of output pin drivers
	   max7311_RegWrite(max7311_Port0700Output, 0xAA); // bitmap of output pin drivers
	   max7311_RegWrite(max7311_Port0700Output, 0x55); // bitmap of output pin drivers
	   max7311_RegWrite(max7311_Port0700Output, 0xAA); // bitmap of output pin drivers
	   max7311_RegWrite(max7311_Port1508Output, 0xAA); // bitmap of output pin drivers
	   max7311_RegWrite(max7311_Port0700Output, 0x55); // bitmap of output pin drivers
	   max7311_RegWrite(max7311_Port1508Output, 0x55); // bitmap of output pin drivers
   } else {
      // normal: no MAX7311 I/O expander
   }
#endif // SUPPORT_MAX7311
   //
#if SUPPORT_LED1_LED2_DIAGNOSTIC
   flash_Led1Led2_East();
#endif // SUPPORT_LED1_LED2_DIAGNOSTIC
   //
#if 0
   if (gLIS2DHdeviceAddress != 0) {
      flash_Led1Led2_Blink();
      flash_Led1Led2_Blink();
      flash_Led1Led2_Blink();
      flash_Led1Led2_East();
#if SUPPORT_MAX7311
	   max7311_RegWrite(max7311_Port0700Output, 0xAA); // bitmap of output pin drivers
	   max7311_RegWrite(max7311_Port0700Output, 0x55); // bitmap of output pin drivers
	   max7311_RegWrite(max7311_Port0700Output, 0xAA); // bitmap of output pin drivers
	   max7311_RegWrite(max7311_Port0700Output, 0x55); // bitmap of output pin drivers
	   max7311_RegWrite(max7311_Port0700Output, 0xAA); // bitmap of output pin drivers
	   max7311_RegWrite(max7311_Port1508Output, 0xAA); // bitmap of output pin drivers
	   max7311_RegWrite(max7311_Port0700Output, 0x55); // bitmap of output pin drivers
	   max7311_RegWrite(max7311_Port1508Output, 0x55); // bitmap of output pin drivers
#endif // SUPPORT_MAX7311
   } 
   else {
#if SUPPORT_MAX7311
	   max7311_RegWrite(max7311_Port0700Output, 0x81); // bitmap of output pin drivers
	   max7311_RegWrite(max7311_Port1508Output, 0x81); // bitmap of output pin drivers
	   max7311_RegWrite(max7311_Port0700Output, 0x18); // bitmap of output pin drivers
	   max7311_RegWrite(max7311_Port1508Output, 0x18); // bitmap of output pin drivers
#endif // SUPPORT_MAX7311
   }
   //~ flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
   //~ flash_Led1L_Led2H_100ms(); // [   ] [GREEN]
   //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
   //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
   //~ flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
   //~ flash_Led1L_Led2H_100ms(); // [   ] [GREEN]
   //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
   //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
   //~ flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
   //~ flash_Led1L_Led2H_100ms(); // [   ] [GREEN]
   //~ flash_Led1L_Led2L_100ms(); // [   ] [     ]
   //~ flash_Led1H_Led2L_100ms(); // [RED] [     ]
   //~ flash_Led1H_Led2H_100ms(); // [RED] [GREEN]
#endif
   //
   while (1)
   {
      
      if (gOS21ready)
      {
         // read MAX30101 FIFO and (if present) LIS2DH FIFO; HID Report to host
         readOS21();
         //testOS21IntTiming();
#if 1 // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service
         if (gLIS2DHready) {
            // when gLIS2DHready call readLIS2DH()
            readLIS2DH();
         }
      } else if (gLIS2DHready) {
         // when gLIS2DHready call readLIS2DH()
         readLIS2DH();
#endif // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service
      } else {
         // idle
         
#if SUPPORT_P20BTN_DIAGNOSTIC
         // VERIFY: OS24EVK-66 use diagnostic P20Btn to trigger readOS21()
         //   P2.0 = diagnostic pushbutton on C8051F321 daughtercard TOOLSTICK321DC (0=pressed; 1=not pressed)
         if (P20Btn_Pressed()) {
#if 1 // OS24EVK-66 Firmware regresssion 2015-04-16_v1.1 (svn r=15232) vs 2015-04-15_v1.1 (svn r=15222): crashes GUI and Console program on connect (API ReadFile error).
            uint8_t lis2dh_STATUS_REG2_value = 0; //!< (read-only) ZYXOR ZOR YOR XOR ZYXDA ZDA YDA XDA
            uint8_t lis2dh_FIFO_SRC_REG_value = 0; //!< (read-only) WTM OVRN_FIFO EMPTY FSS4 FSS3 FSS2 FSS1 FSS0
            //
            // C not C++, so all local variables must be declared before any statements
            //
            // TODO1: OS24EVK-66 P2.0 button diagnostic: debug what's happening when FIFO seems to be stuck?
            lis2dh_RegRead(lis2dh_STATUS_REG2, &lis2dh_STATUS_REG2_value);
            lis2dh_RegRead(lis2dh_FIFO_SRC_REG, &lis2dh_FIFO_SRC_REG_value);
            //
            // TODO: Set Breakpoint Here
            //
#endif // OS24EVK-66 Firmware regresssion 2015-04-16_v1.1 (svn r=15232) vs 2015-04-15_v1.1 (svn r=15222): crashes GUI and Console program on connect (API ReadFile error).
            readOS21();
         }
#endif // SUPPORT_P20BTN_DIAGNOSTIC
         //
      }
   }
}

void updateVersion(VersionStructure *versionInfo)
{
    // TODO: Update Version Information
#if SUPPORT_MAX30101
# if USB_SUPPORT_SendPacketWaitEP1INready
   versionInfo->major = 1;	
   versionInfo->minor = 1;	
   versionInfo->year = 2015;
   // OS24EVK-66 Firmware regresssion 2015-04-16_v1.1 (svn r=15232) vs 2015-04-15_v1.1 (svn r=15222): crashes GUI and Console program on connect (API ReadFile error).
   // TODO1: OS24EVK-72 (firmware 2015-04-24_v1.1) HID report 1 byte 3 = MAX30101_InterruptStatus1 (not MAX30101_OverFlowCounter)
   // firmware (2015-04-28) to test INT_ACC toggle and OS24EVK-83 enable LIS2DH interrupt 
   // https://svn.maxim-ic.com/svn/evkits/MAX30101/firmware/Uxx/tags/2015-04-28_v1.1_INT_AC_Test
   // 2015-05-01 debugging readLIS2DH()
   // 2015-05-02 singleReport = 1 unless HR mode, in readLIS2DH()
   // 2015-05-03 #define LIS2DH_DEFAULT_FIFOWATERMARKLEVEL 10
   // 2015-05-03 No ISR Poll for 0x27 STATUS_REG2 bit 0x08 ZYXDA
   // Why INT_AC stuck active(low) starting 150ms before first INT_OS pulse?
   // 2015-05-03 #define LIS2DH_DEFAULT_FIFOWATERMARKLEVEL 31
   // Much better, HR mode clearly shows INT_AC 50% duty cycle which matches the 50% data loss(overrun).
   // Now I have to wonder, why INT2 service only quells INT_AC for 100ms?
   // Is the interrupt only reading out a single 10-sample report?
   // And why is there no I2C bus activity in response to INT_AC falling edge?
   //
   // Maybe the issue is that there's a significant delay (>300ms) 
   // between when the firmware first enables LIS2DH FIFO streaming -- by  myHID.INT0Enable(1); --
   // and when the MAX30101 is actually configured for streaming -- by RegWrite(RegAddressEnum.ModeConfiguration, regValue); --?
   //
   // OS24EVK-83 (firmware 2015-05-04) separate trigger for LIS2DH-only interrupt
   // 2015-05-04: move lis2dh_configure_FIFO(LIS2DH_DEFAULT_FIFOMODE) out of myHID.INT0Enable(1)
   // 2015-05-04: lis2dh_configure_FIFO(LIS2DH_DEFAULT_FIFOMODE) when max30101_RegWrite(RegAddressEnum.ModeConfiguration,...)
   // Much improved, INT_AC duty cycle looks more reasonable.
   // Still have some data loss in HR mode, but not as much.
   //
   // OS24EVK-83 (firmware 2015-05-05) separate trigger for LIS2DH-only interrupt
   // 2015-05-05: #define LIS2DH_DEFAULT_FIFOWATERMARKLEVEL 15
   // Good INT_AC duty cycle, looks good.
   //
   // OS24EVK-83 (firmware 2015-05-06) separate trigger for LIS2DH-only interrupt
   // 2015-05-06: #define SUPPORT_MAX7311 0
   // 2015-05-06: #define SUPPORT_SPI 0
   // 2015-05-06: #define SUPPORT_P20BTN_DIAGNOSTIC 0
   // 2015-05-06: remove dead code lis2dh_configure_selfTest
   //
   // OS24EVK-83 (firmware 2015-05-07) separate trigger for LIS2DH-only interrupt
   // 2015-05-07: #define LIS2DH_DEFAULT_FIFOWATERMARKLEVEL 26
   // 2015-06-26: update firmware date to current date
   versionInfo->month = 6;
   versionInfo->day = 26;
# else // USB_SUPPORT_SendPacketWaitEP1INready
   versionInfo->major = 1;	
   versionInfo->minor = 1;	
   versionInfo->year = 2015;
   versionInfo->month = 03;
   versionInfo->day = 31;
# endif // USB_SUPPORT_SendPacketWaitEP1INready
#else
	// regression testing
	versionInfo->major = 1;	
	versionInfo->minor = 0;	
	versionInfo->year = 2014;
	versionInfo->month = 10;
	versionInfo->day = 30;
#endif
}
