/*******************************************************************************
* Copyright (C) 2015 Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*******************************************************************************
*/

#ifndef __MAX7311_H__
#define __MAX7311_H__

// OS24EVK-58 Software Toolkit - bare metal device driver

extern uint8_t gMAX7311deviceAddress;

void max7311_open();
int8_t max7311_RegRead(uint8_t regAddress, uint8_t *pRegValue);
int8_t max7311_RegWrite(uint8_t regAddress, uint8_t regValue);

/// I2C register addresses within the device
typedef enum max7311_regAddr_enum_t {
    max7311_Port0700Input       = 0x00, //!< pins I/O0..I/O7 Input Port -- read-only bitmap of logic level at the pin
    max7311_Port0700Output      = 0x02, //!< pins I/O0..I/O7 Output Port -- bitmap of output pin drivers
    max7311_Port0700Invert      = 0x04, //!< pins I/O0..I/O7 Polarity Inversion -- bitmap where 1=inverted (active-low) signal; affects Input Port and Output Port
    max7311_Port0700ConfigInput = 0x06, //!< pins I/O0..I/O7 Configuration -- bitmap where 1=input pin, 0=output pin
    max7311_Port1508Input       = 0x01, //!< pins I/O8..I/O15 Input Port -- read-only bitmap of logic level at the pin
    max7311_Port1508Output      = 0x03, //!< pins I/O8..I/O15 Output Port -- bitmap of output pin drivers
    max7311_Port1508Invert      = 0x05, //!< pins I/O8..I/O15 Polarity Inversion -- bitmap where 1=inverted (active-low) signal; affects Input Port and Output Port
    max7311_Port1508ConfigInput = 0x07, //!< pins I/O8..I/O15 Configuration -- bitmap where 1=input pin, 0=output pin
    max7311_Timeout             = 0x08  //!< Timeout Register
} max7311_regAddr_enum;

//*******************************************************************************
// External functions that are not part of MAX3010 device driver
//*******************************************************************************

// This is the low-level interface to the platform I2C device driver.
// This function is not part of the MAX30101 device driver.
//
// SMBusReadByte protocol I2C 8-bit register read
//
// @param[in] deviceAddress = I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.
// @param[in] regAddress = 8-bit register address
// @param[out] pRegValue = optional buffer to get the received data
// @post global blockarr is used
// @pre global gOffset = 0 or 1 depending on whether EXPLICIT_REPORT_ID
//
// @return -1 on NACK, or 1 on success
//
extern int8_t i2c_smbus_read_byte_data(uint8_t deviceAddress, uint8_t regAddress, uint8_t *pRegValue); 

// This is the low-level interface to the platform I2C device driver.
// This function is not part of the MAX30101 device driver.
//
// SMBusReadByte protocol I2C 8-bit register read, multiple consecutive bytes
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
extern int8_t i2c_smbus_read_bytes_data(uint8_t deviceAddress, uint8_t regAddress, uint8_t count, uint8_t *pRegValue);

// This is the low-level interface to the platform I2C device driver.
// This function is not part of the MAX30101 device driver.
//
// SMBusWriteByte protocol I2C 8-bit register read
//
// @param[in] deviceAddress = I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.
// @param[in] regAddress = 8-bit register address
// @param[in] regValue = 8-bit register value
// @post global blockarr is used
// @pre global gOffset = 0 or 1 depending on whether EXPLICIT_REPORT_ID
//
// @return -1 on NACK, or 1 on success
//
extern int8_t i2c_smbus_write_byte_data(uint8_t deviceAddress, uint8_t regAddress, uint8_t regValue); 

#endif /* __MAX7311_H__ */
