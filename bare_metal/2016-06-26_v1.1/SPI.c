//-----------------------------------------------------------------------------
// SPI.c
//-----------------------------------------------------------------------------
// Copyright 2014
//
// Program Description:
//
// Source file for SMBus Interrupt Service Routine and supporting functions.
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
//		-140401 Initial Revision (Tushar Mazumder)
//
// Other than bug fixes or implementation changes, this file should not need to be updated.

#include "c8051f3xx.h"
#include "F3xx_USB0_Descriptor.h"
#include "F3xx_Initialize.h"

#if SUPPORT_SPI

extern uint8_t gOffset;
sbit MISO = P0^1;

//void T0_Wait_ms (uint16_t);
void SPI_generic(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t*);

void SPI_mode0(uint8_t phase_change, uint8_t phase_change_byte, uint8_t num_bytes, uint8_t *dataPtr)
{
	// const uint8_t phase_change = phase_change; // !=0 enable changing the clock phase. some slaves change phase between write/read
	// const uint8_t phase_change_byte = phase_change_byte; // byte index where phase change should happen
	// const uint8_t toggleClock = 0; // !=0 change phase between write/read, providing an extra clock for slave to send data before the master starts sampling
	// const uint8_t sampleMISO = 0; // !=0 discard the MSb on the second byte read, and then sample the LSb after the read
	// const uint8_t num_bytes = num_bytes;
	// const uint8_t *dataPtr = dataPtr;
	// SPI_generic(phase_change, phase_change_byte, toggleClock, sampleMISO, num_bytes, dataPtr); 
	SPI_generic(phase_change, phase_change_byte, 0, 0, num_bytes, dataPtr);
}

void SPI_mode1(uint8_t read, uint8_t *dataPtr)
{
	// two-byte SPI transfer. read flag: enable changing the clock phase on first byte and changing phase between first and second byte
	// const uint8_t phase_change = read; // !=0 enable changing the clock phase. some slaves change phase between write/read
	// const uint8_t phase_change_byte = 0; // byte index where phase change should happen
	// const uint8_t toggleClock = read; // !=0 change phase between write/read, providing an extra clock for slave to send data before the master starts sampling
	// const uint8_t sampleMISO = 0;
	// const uint8_t num_bytes = 2;
	// const uint8_t *dataPtr = dataPtr;
	// SPI_generic(phase_change, phase_change_byte, toggleClock, sampleMISO, num_bytes, dataPtr);
	SPI_generic(read, 0, read, 0, 2, dataPtr);
}

void SPI_mode2(uint8_t read, uint8_t *dataPtr)
{
	// two-byte SPI transfer. read flag: enable changing the clock phase on first byte and sampling the LSb after the second byte read
	// const uint8_t phase_change = read; // !=0 enable changing the clock phase. some slaves change phase between write/read
	// const uint8_t phase_change_byte = 0; // byte index where phase change should happen
	// const uint8_t toggleClock = 0;
	// const uint8_t sampleMISO = read; // !=0 discard the MSb on the second byte read, and then sample the LSb after the read
	// const uint8_t num_bytes = 2;
	// const uint8_t *dataPtr = dataPtr;
	// SPI_generic(phase_change, phase_change_byte, toggleClock, sampleMISO, num_bytes, dataPtr);
	SPI_generic(read, 0, 0, read, 2, dataPtr);
}

void SPI_generic(uint8_t phase_change, uint8_t phase_change_byte, uint8_t toggleClock, uint8_t sampleMISO, uint8_t num_bytes, uint8_t *dataPtr)
// phase change, toggleClock, sampleMISO will only be '1' if a read
{
	uint8_t i;
	uint8_t temp;

	NSSMD0 = ~NSSMD0;
	for (i=0 ; i<num_bytes ; i++)
	{
		SPI0DAT = *(dataPtr + gOffset + 6 + i);	 // data starts on the (7 + offset) byte
		while (!(SPI0CN & 0x80));	//wait until interrupt flag is cleared to signal end of transfer
		SPI0CN &= 0x7F;				//clear interrupt flag

		// some slaves change phase between write/read but need an extra clock to send data before the master starts sampling
		// toggle CKPOL to create the extra clock
		// this assumes a two byte SPI command
		if (toggleClock && i == 0)	
		{
			(SPI0CFG & 0x10) ? (SPI0CFG &= 0xEF) : (SPI0CFG |= 0x10);
			//T0_Wait_ms(10);
			(SPI0CFG & 0x10) ? (SPI0CFG &= 0xEF) : (SPI0CFG |= 0x10);
		}

		if (phase_change && i == phase_change_byte)
		{
			(SPI0CFG & 0x20) ? (SPI0CFG &= 0xDF) : (SPI0CFG |= 0x20);	// toggle clock phase	
		}

		*(dataPtr + gOffset + 1 + i) = SPI0DAT;	// read SPI0DAT on every write, since we don't know when real data is expected to be read

		// some slaves change phase between write/read and need an extra clock to send data before the master starts sampling, but don't allow the latching edge on the last bit!
		// don't send an extra clock between bytes, but rather discard the MSb on the read, and then sample the LSb after the read
		// this assumes a two byte SPI command
		if (sampleMISO && i == 1)
		{
			temp = ((*(dataPtr + gOffset + 1 + i)) << 1) | MISO;
			*(dataPtr + gOffset + 1 + i) = temp;
		}
	}
	if (phase_change)	// restore original clock phase
	{
		(SPI0CFG & 0x20) ? (SPI0CFG &= 0xDF) : (SPI0CFG |= 0x20);
	}
	NSSMD0 = ~NSSMD0;
	*dataPtr = SHORT_REPORT_ID; // HID_REPORT_ID_1
	*(dataPtr + gOffset) = 0; 	// status
}

#endif // SUPPORT_SPI

