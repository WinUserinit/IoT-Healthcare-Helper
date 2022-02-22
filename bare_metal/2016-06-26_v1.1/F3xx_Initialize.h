//-----------------------------------------------------------------------------
// F3xx_Initialize.h
//-----------------------------------------------------------------------------
// Copyright 2014
//
// Program Description:
//
// This file includes all of the Report IDs and variables needed by
// USB_ReportHandler.c to process input and output reports,
// as well as initialization routine prototypes.
//
//
// FID:            3XX000010
// Target:         C8051F3xx
// Tool chain:     Keil C51 7.50 / Keil EVAL C51
//                 Silicon Laboratories IDE version 2.6
// Project Name:   Generic HID Firmware
//
// Release 1.0
//    -140401 Initial Revision (Tushar Mazumder)
//
// Other than bug fixes or implementation changes, this file should not need to be updated.


#ifndef  _F3xx_Initialize_H_
#define  _F3xx_Initialize_

// Project-level preprocessor #defines effective across all *.c files
#include "ProjectDefines.h"

extern uint8_t xdata IO_PACKET[];

void System_Init(void);

//-----------------------------------------------------------------------------
// 16-bit SFR Definitions for 'F32x
//-----------------------------------------------------------------------------
sfr16 TMR2RL   = 0xca;                   // Timer2 reload value
sfr16 TMR2     = 0xcc;                   // Timer2 counter     
sfr16 TMR3     = 0x94;                 // Timer3 counter
sfr16 TMR3RL   = 0x92;                 // Timer3 reload

// ----------------------------------------------------------------------------
// Report IDs
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Report Sizes (in bytes)
// ----------------------------------------------------------------------------
#define SHORT_REPORT_COUNT		0x3F


#endif