//-----------------------------------------------------------------------------
// SMBus.h
//-----------------------------------------------------------------------------
// Copyright 2014
//
// Program Description:
//
// Header file for SMBus.c
//
//
// FID:            
// Target:         C8051F32x/C8051F340
// Tool chain:     Keil C51 7.50 / Keil EVAL C51
//                 Silicon Laboratories IDE version 2.6
// Project Name:   generic
//
//
// Release 1.0
//    -140401 Initial Revision (Tushar Mazumder)
//
// Other than bug fixes or implementation changes, this file should not need to be updated.

//-----------------------------------------------------------------------------
// Global CONSTANTS
//-----------------------------------------------------------------------------

#ifndef  _SMBUS_H_
#define  _SMBUS_H_

// System clock frequency in Hz
#define	INT_OSC_FREQ	12000000		//internal oscillator frequency

#define  WRITE          0x00           // SMBus WRITE command
#define  READ           0x01           // SMBus READ command

// Status vector - top 4 bits only
#define  SMB_MTSTA      0xE0           // (MT) start transmitted
#define  SMB_MTDB       0xC0           // (MT) data byte transmitted
#define  SMB_MRDB       0x80           // (MR) data byte received
// End status vector definition

#endif