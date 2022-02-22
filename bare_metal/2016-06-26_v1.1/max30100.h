/*******************************************************************************
* Copyright (C) 2015 Maxim Integrated Products, Inc., All rights Reserved.
* * This software is protected by copyright laws of the United States and
* of foreign countries. This material may also be protected by patent laws
* and technology transfer regulations of the United States and of foreign
* countries. This software is furnished under a license agreement and/or a
* nondisclosure agreement and may only be used or reproduced in accordance
* with the terms of those agreements. Dissemination of this information to
* any party or parties not specified in the license agreement and/or
* nondisclosure agreement is expressly prohibited.
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

//*******************************************************************************
//
// This source code file is under the same NDA terms as the IC data sheet.
//
//*******************************************************************************

#ifndef __MAX30100_H__
#define __MAX30100_H__

// OS24EVK-58 Software Toolkit - bare metal device driver

extern uint8_t gMAX30100deviceAddress;

void max30100_open();
int8_t max30100_RegRead(uint8_t regAddress, uint8_t *pRegValue);
int8_t max30100_RegWrite(uint8_t regAddress, uint8_t regValue);

/// I2C register addresses within the device
typedef enum max30100_regAddr_enum_t {

    // MAX30100 register 0x00: r/o Interrupt Status
    MAX30100_InterruptStatus = 0x00,

    // MAX30100 register 0x01: r/w Interrupt Enable
    MAX30100_InterruptEnable = 0x01,

    // MAX30100 register 0x02: r/w FIFO Write Pointer
    MAX30100_FIFOWritePointer = 0x02,

    // MAX30100 register 0x03: r/w Over Flow Counter
    MAX30100_OverFlowCounter = 0x03,

    // MAX30100 register 0x04: r/w FIFO Read Pointer
    MAX30100_FIFOReadPointer = 0x04,

    // MAX30100 register 0x05: r/w FIFO Data Register
    MAX30100_FIFOData = 0x05,

    // MAX30100 register 0x06: r/w Mode Configuration (0x80=SHDN, 0x40=RESET, 0x08=TEMP_EN, 0x07=MODE[2:0])
    MAX30100_ModeConfiguration = 0x06,

    // MAX30100 register 0x07: r/w SPO2 Configuration (0x40=SPO2_HI_RES_EN, 0x20=reserved_0, 0x1C=SPO2_SR[2:0], 0x03=LED_PW[1:0])
    // chkHiRes.Checked = reserved constant 1 RegAddressEnum.SPO2Configuration bit 0x40=SPO2_HI_RES_EN
    // chkADCrange.Checked = reserved constant 0 RegAddressEnum.SPO2Configuration bit 0x20=reserved_0
    MAX30100_SPO2Configuration = 0x07,

    // MAX30100 register 0x09: r/w LED Pulse Amplitude (0xF0=RED_PA[3:0], 0x0F=IR_PA[3:0])
    MAX30100_LEDRedIRPulseAmplitude = 0x09,

    // MAX30100 register 0x16: r/o Die Temperature integer
    MAX30100_DieTemperatureInteger = 0x16,

    // MAX30100 register 0x17: r/o Die Temperature fraction
    MAX30100_DieTemperatureFraction = 0x17,

    // MAX30100 register 0xFE: r/o Revision ID
    MAX30100_RevisionID = 0xFE,

    // MAX30100 register 0xFF: r/o Part ID (constant 0x11)
    MAX30100_DeviceID = 0xFF,

} max30100_regAddr_enum;

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

#endif /* __MAX30100_H__ */
