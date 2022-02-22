//-----------------------------------------------------------------------------
// F3xx_USB0_ReportHandler.h
//-----------------------------------------------------------------------------
// Copyright 2014
//
// Program Description:
//
// Header file to handle input and output reports.
//
//
// FID             3XX000014
// Target:         C8051F32x/C8051F340
// Tool chain:     Keil C51 7.50 / Keil EVAL C51
//                 Silicon Laboratories IDE version 2.6
// Project Name:   Generic HID Firmware
//
//
// Release 1.0
//    -140401 Initial Revision (Tushar Mazumder)
//
// Other than bug fixes or implementation changes, this file should not need to be updated.

#ifndef  _USB_REPORTHANDLER_H_
#define  _USB_REPORTHANDLER_H_

// Project-level preprocessor #defines effective across all *.c files
#include "ProjectDefines.h"

typedef struct{
	uint8_t Length;
	uint8_t* Ptr;
} BufferStructure;

typedef struct
{
	uint8_t major;		// major version
	uint8_t minor;		// minor version
	uint16_t year;			// this will need to be split into two bytes when transferred to GUI
	uint8_t month;
	uint8_t day;
} VersionStructure;

extern void Setup_IO_BUFFER(void);
extern void OUT_REPORT_HANDLER(int16_t internalCall);

extern BufferStructure IO_BUFFER;

extern VersionStructure versionInfo;

void readOS21(void);
void testOS21IntTiming(void);

#if 1 // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service
void readLIS2DH(void);
#endif // TODO1: OS24EVK-83 (firmware 2015-04-29) separate trigger for LIS2DH-only interrupt service

#if SUPPORT_LED1_LED2_DIAGNOSTIC
// OS24EVK-66 flashing diagnostic LEDs
void flash_Led1L_Led2L_100ms();
void flash_Led1L_Led2H_100ms();
void flash_Led1H_Led2L_100ms();
void flash_Led1H_Led2H_100ms();
void flash_Led1Led2_East();
void flash_Led1Led2_Blink();
#endif // SUPPORT_LED1_LED2_DIAGNOSTIC

#if SUPPORT_P20BTN_DIAGNOSTIC
int16_t P20Btn_Pressed();
#endif // SUPPORT_P20BTN_DIAGNOSTIC

int8_t i2c_smbus_read_byte_data(uint8_t deviceAddress, uint8_t regAddress, uint8_t *pRegValue); // @return -1 on NACK, or 1 on success
int8_t i2c_smbus_read_bytes_data(uint8_t deviceAddress, uint8_t regAddress, uint8_t count, uint8_t *pRegValue);
int8_t i2c_smbus_write_byte_data(uint8_t deviceAddress, uint8_t regAddress, uint8_t regValue); // @return -1 on NACK, or 1 on success


#define TIMER0_INIT_DELAY_HALFMS	0
#define TIMER0_INIT_DELAY_US 		1

#endif