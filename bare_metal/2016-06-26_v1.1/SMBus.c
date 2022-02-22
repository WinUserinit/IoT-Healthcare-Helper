//-----------------------------------------------------------------------------
// SMBus_ISR.c
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

// Project-level preprocessor #defines effective across all *.c files
#include "ProjectDefines.h"

#include "c8051f3xx.h"
#include "F3xx_USB0_Descriptor.h"
#include "SMBus.h"

sbit SDA = P0^4;
sbit SCL = P0^5;

uint8_t *gpSMBdata;				// Global pointer to SMBus register and data bytes
                                       

uint8_t gSMBnumBytesToTransfer;	// Global holder for number of data bytes to send or receive in the current SMBus transfer.

uint8_t gSMBlenStartRegister;		// length of the starting register address (in bytes) to handle multi-byte addresses; used in random read/write

// gSMBslaveAddress contains the I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.
uint8_t gSMBslaveAddress;			// Target SMBus slave address

bit gSMBbusy = 0;						// Software flag to indicate when the i2c_read() or i2c_write() functions have claimed the SMBus

bit gSMBrw;								// Software flag to indicate the direction of the current transfer

bit gSMBsendStartRegister;				// When set, this flag causes the ISR to send the starting register after sending the slave address. Used in random read/write


bit gSMBrandomRead;						// When set, this flag causes the ISR to send a START signal after sending the word address.
										// A random read (a read from a particular address in memory) starts as a write then
										// changes to a read after the repeated start is sent. The ISR handles this switchover if the <gSMBrandomRead>
										// bit is set.

bit gSMBackPoll;						// When set, this flag causes the ISR to send a repeated START until the slave has acknowledged its address

bit gSMBrepeatedStart;					// When set, this flag allows the ISR to do a repeated start (instead of stop/start) on random reads, 

bit gSMBstoppedWrite;					// When set, this flag allows the ISR to emulate the jungo driver version on psuedo-random reads, 
										// where the slave address is written with the write bit set, immediately proceeeded 
										// by a stop/start (or repeated start) before sending the slave address with the read bit set.

// TODO: OS24EVK-66 2015-05-20: firmware xxxx-xx-xx_v1.1 gSMBrepeatedStart 
// gI2Cflags bit 0x01 gSMBrepeatedStart  Use I2C repeated start between write and read
// gI2Cflags bit 0x02 debug: jungo emulation
// gI2Cflags bit 0x04 gSMBackPoll Enable Acknowledge Polling
uint8_t gI2Cflags = 1; // previously 0

extern uint8_t gOffset;

//-----------------------------------------------------------------------------
// SMBus0 Interface Interrupt Service Routine (ISR)
//-----------------------------------------------------------------------------
// SMBus ISR state machine
// - Master only implementation - no slave or arbitration states defined
// - gSMBslaveAddress contains the I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.
// - All master-receive incoming data is written starting at the global pointer gpSMBdata
// -- gpSMBdata[0+gOffset] contains status
// -- gpSMBdata[1+gOffset] contains first master-receive byte
// -- gpSMBdata[2+gOffset] contains second master-receive byte etc.
// -- gpSMBdata[4+gOffset] contains "start register" i.e. regAddress i.e. register address within the device
// - All master-transmit outgoing data is read from the global pointer gpSMBdata
// -- gpSMBdata[5 + offset + gSMBlenStartRegister] contains first master-transmit byte
//
// - (obsolete) All incoming data is written starting at the global pointer <gpSMBdataIn>
// - (obsolete) All outgoing data is read from the global pointer <gpSMBdataOut>
//
void SMBus_ISR (void) interrupt INTERRUPT_SMBUS0 /* interrupt 7 */
{
	bit FAIL = 0;                       // Used by the ISR to flag failed
                                       // transfers

	static int8_t dataByte;                      // Used by the ISR to count the
                                       // number of data bytes sent or
                                       // received

	static int8_t regByte; 

	static bit SEND_START = 0;          // Send a start. Used when switching to read mode.

	switch (SMB0CN & 0xF0)              // Status vector
	{
		// Master Transmitter/Receiver: START condition transmitted.
		case SMB_MTSTA:
			SMB0DAT = gSMBslaveAddress;             // Load address of the target slave
			SMB0DAT &= 0xFE;              // Clear the LSbit of the address for the
										// R/W bit
			SMB0DAT |= gSMBrw;            // Load R/W bit
			STA = 0;                      // Manually clear START bit
			dataByte = 0;                        // Reset data byte counter
			regByte = 0;
			break;

		// Master Transmitter: Data byte (or Slave Address) transmitted
		case SMB_MTDB:
			if (ACK)                      // Slave Address or Data Byte
			{                             // Acknowledged?
				if (SEND_START)
				{
					if (!gSMBrepeatedStart)
						STO = 1;				// Set STO to terminate previous transfer (write register)
					STA = 1;
					SEND_START = 0;
					break;
				}
				if (gSMBstoppedWrite)			// only can occur when before a read
				{
					gSMBrw = READ;
					gSMBstoppedWrite = 0;
					if (!gSMBrepeatedStart)
						STO = 1;
					STA = 1;
					break;
				}
				if(gSMBsendStartRegister)       // Are we sending the start register?
				{
					SMB0DAT = *(gpSMBdata + 4 + gOffset + regByte++);	// Send start register (regAddress)
										
					if (regByte == gSMBlenStartRegister) 
					{
						gSMBsendStartRegister = 0;   // Clear flag
						if (gSMBrandomRead)
						{
							SEND_START = 1;      // Send a START after the next ACK cycle
							gSMBrw = READ;
						}
					}
					break;
				}

				if (gSMBrw==WRITE)         // Is this transfer a WRITE?
				{
					if (dataByte < gSMBnumBytesToTransfer)   // Is there data to send?
						SMB0DAT = *(gpSMBdata + 4 + gOffset + gSMBlenStartRegister + dataByte++); // master-transmit data starts at (5 + offset + gSMBlenStartRegister) byte
					else
					{
						STO = 1;              // Set STO to terminate transfer
						gSMBbusy = 0;         // Clear software busy flag
						*(gpSMBdata + gOffset) = 0; 			// status = 0, no errors
					}
				} 
			}
			else                          // If slave NACK,
				if(gSMBackPoll)
					STA = 1;                // Restart transfer
				else
					FAIL = 1;               // Indicate failed transfer and handle at end of ISR
			break;

		// Master Receiver: byte received
		case SMB_MRDB:
			if ( dataByte < gSMBnumBytesToTransfer )       // Is there any data remaining?
			{
				*(gpSMBdata + 1 + gOffset + dataByte++) = SMB0DAT;   // Store received byte starting in the (2+offset) position; first position contains status
				if (dataByte < gSMBnumBytesToTransfer)
					ACK = 1;                   // Set ACK bit (may be cleared later in the code)
				else if (dataByte == gSMBnumBytesToTransfer)        // This is the last byte
				{
					gSMBbusy = 0;              // Free SMBus interface
					ACK = 0;                   // Send NACK to indicate last byte of this transfer
					STO = 1;                   // Send STOP to terminate transfer
					*(gpSMBdata + gOffset) = 0;				// error status = 0; should be no errors if we got past the write portion
				}
			}
			break;

		default:
			FAIL = 1;                     // Indicate failed transfer
                                       // and handle at end of ISR
			break;
	}

	if (FAIL)                           // If the transfer failed,
	{
		SMB0CF &= ~0x80;                 // Reset communication
		SMB0CF |= 0x80;
		STA = 0;
		STO = 0;
		ACK = 0;

		gSMBbusy = 0;                    // Free SMBus

		FAIL = 0;
		*(gpSMBdata + gOffset) = 1; 					// status = 1, NACK error
	}

	SI = 0;                             // Clear interrupt flag
}

//-----------------------------------------------------------------------------
// Support Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// i2c_write ()
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//   1) uint8_t slaveAddress - address of the i2c slave
//
//   2) uint8_t* pData - pointer to the buffer containing the register address (optional) and data to be written
//
//   3) uint8_t lenStartReg - starting register byte length (i.e., 2 = 2 bytes long); 0 indicates a non-random write
//
//   4) uint8_t lenData - number of data bytes to be written to the i2c slave
//	 
//
void i2c_write(uint8_t slaveAddress, uint8_t *pData, uint8_t lenData, uint8_t lenStartReg)
{
 	while (gSMBbusy);							// Wait for SMBus to be free.
	gSMBbusy = 1;								// Claim SMBus (set to busy)

	gSMBslaveAddress = slaveAddress;			// Set target slave address
	gSMBrw = WRITE;								// Mark next transfer as a write
	if (lenStartReg == 0)						// no address bytes, so not a random write
		gSMBsendStartRegister = 0;               
	else										// random write; send register address after slave address
		gSMBsendStartRegister = 1;				
	gSMBrandomRead = 0;							// Do not send a START signal after the register address since this is a write
	gSMBackPoll = gI2Cflags & 4;					// Enable Acknowledge Polling (The ISR will automatically restart the transfer if the slave does not acknowledge its address.

	*pData = SHORT_REPORT_ID; // HID_REPORT_ID_1
	gpSMBdata = pData;
	gSMBlenStartRegister = lenStartReg;			// byte length of start register (stop or repeated start occurs after the last byte)
	gSMBnumBytesToTransfer = lenData;			// byte length of data

	gSMBstoppedWrite = 0;						// not used in write

	STA = 1;
	while (gSMBbusy);							// Stall the software here until the I2C transaction is complete
}

//-----------------------------------------------------------------------------
// i2c_read ()
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//   1) uint8_t slaveAddress - address of the i2c slave
//
//   2) uint8_t* pData - pointer to the buffer containing the register address (optional)
//
//   3) uint8_t lenStartReg - starting register byte length (i.e., 2 = 2 bytes long); 0 indicates a non-random read
//
//   4) uint8_t lenData - number of data bytes to be read from the i2c slave
//	 
//
void i2c_read (uint8_t slaveAddress, uint8_t *pData, uint8_t lenData, uint8_t lenStartReg)
{
	while (gSMBbusy);						// Wait for SMBus to be free.
	gSMBbusy = 1;							// Claim SMBus (set to busy)

	gSMBslaveAddress = slaveAddress;		// Set target slave address
  
	if (lenStartReg == 0)					// non-random read
	{
		gSMBrw = READ;
		gSMBsendStartRegister = 0;
	}
	else									// random read; send starting register first
	{
		gSMBrw = WRITE;                     // A random read starts as a write then changes to a read after the repeated start is sent. 
											// The ISR handles this switchover if the <gSMBrandomRead> bit is set.
  		gSMBsendStartRegister = 1;			// Send starting register address after slave address
	}
	if (gI2Cflags & 2)						// pseudo random read; pretend to send the starting register by sending slave address with write bit set, but don't send it (this is to emulate the jungo version for debug)
	{
		gSMBstoppedWrite = 1;
		gSMBsendStartRegister = 0;
	}
	else 									// normal operation
		gSMBstoppedWrite = 0;
			
	gSMBrandomRead = 1;                 	// Send a START after the register address
	gSMBrepeatedStart = gI2Cflags & 1;		// indicate whether or not to send STOP before the START after write (including stopped write, so don't move this to the non-random read section of the above 'if')
	
	gSMBackPoll = gI2Cflags & 4;				// Enable Acknowledge Polling

	*pData = SHORT_REPORT_ID; // HID_REPORT_ID_1
	gpSMBdata = pData;
	gSMBlenStartRegister = lenStartReg;		// byte length of start register (stop or repeated start occurs after the last byte)
	gSMBnumBytesToTransfer = lenData;		// byte length of data

	STA = 1;
	while(gSMBbusy);						// Stall the software here until the I2C transaction is complete
   
   // NACK error is indicated by *(gpSMBdata + gOffset) = 1; // status = 1, NACK error
}


// If slave is holding SDA low because of an improper SMBus reset or error
// Provide clock pulses to allow the slave to advance out
// of its current state. This will allow it to release SDA.
// (This function is recommended by SiLabs, but it doesn't seem to do anything because you can't force SCL if it's configured for SMBus)
int16_t clearSDA()
{
	uint16_t count = 0;
	uint16_t i;

	while(!SDA && (count < 10))
	{
		SCL = 0; // Drive the clock low
		for(i = 0; i < 255; i++); // Hold the clock low
		SCL = 1; // Release the clock
		while(!SCL); // Wait for open-drain clock output to rise
		for(i = 0; i < 10; i++); // Hold the clock high
		count++;
	}
	(count == 10) ? (count = 1) : (count = 0);
	return count;
}