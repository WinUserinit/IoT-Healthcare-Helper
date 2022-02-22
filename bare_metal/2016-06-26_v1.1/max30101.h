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

#ifndef __MAX30101_H__
#define __MAX30101_H__

// OS24EVK-58 Software Toolkit - bare metal device driver

extern uint8_t gMAX30101deviceAddress;

// TODO: 2015-05-02 cache value of MAX30101_ModeConfiguration in gMAX30101_ModeConfiguration_value
extern uint8_t gMAX30101_ModeConfiguration_value;

void max30101_open();
int8_t max30101_RegRead(uint8_t regAddress, uint8_t *pRegValue);
int8_t max30101_RegWrite(uint8_t regAddress, uint8_t regValue);

/// I2C register addresses within the device
typedef enum max30101_regAddr_enum_t {

    /// MAX30101 register 0x00: r/o Interrupt Status 1
    MAX30101_InterruptStatus1 = 0x00, 

    /// MAX30101 register 0x01: r/o Interrupt Status 2
    MAX30101_InterruptStatus2 = 0x01, 

    /// MAX30101 register 0x02: r/w Interrupt Enable 1
    MAX30101_InterruptEnable1 = 0x02, 

    /// MAX30101 register 0x03: r/w Interrupt Enable 2
    MAX30101_InterruptEnable2 = 0x03, 

    /// MAX30101 register 0x04: r/w FIFO Write Pointer
    MAX30101_FIFOWritePointer = 0x04, 

    /// MAX30101 register 0x05: r/w Over Flow Counter
    MAX30101_OverFlowCounter = 0x05, 

    /// MAX30101 register 0x06: r/w FIFO Read Pointer
    MAX30101_FIFOReadPointer = 0x06, 

    /// MAX30101 register 0x07: r/w FIFO Data Register
    MAX30101_FIFOData = 0x07, 

    /// MAX30101 register 0x08: r/w FIFO Configuration
    MAX30101_FIFOConfiguration = 0x08, 

    /// MAX30101 register 0x09: r/w Mode Configuration (0x80=SHDN, 0x40=RESET, 0x07=MODE[2:0])
    MAX30101_ModeConfiguration = 0x09, 

    /// MAX30101 register 0x0A: r/w SPO2 Configuration
    MAX30101_SPO2Configuration = 0x0A, 

    /// MAX30101 register 0x0C: r/w LED1 Pulse Amplitude (Red)
    MAX30101_LED1RedPulseAmplitude = 0x0C, 

    /// MAX30101 register 0x0D: r/w LED2 Pulse Amplitude (IR)
    MAX30101_LED2IrPulseAmplitude = 0x0D, 

    /// MAX30101 register 0x0E: r/w LED3 Pulse Amplitude (Green) (RevisionID value > 3)
    MAX30101_LED3GreenPulseAmplitude = 0x0E, 

    /// MAX30101 register 0x0F: r/w LED4 Pulse Amplitude (Green)
    MAX30101_LED4GreenPulseAmplitude = 0x0F, 

    /// MAX30101 register 0x10: r/w Proximity Mode LED Pulse Amplitude (PILOT_PA[7:0])
    MAX30101_ProximityModePulseAmplitude = 0x10, 

    /// MAX30101 register 0x11: r/w Multi-LED Mode Control Register timeslot 1 and 2
    MAX30101_MultiLEDModeControlTime2Time1 = 0x11, 

    /// MAX30101 register 0x12: r/w Multi-LED Mode Control Register timeslot 3 and 4
    MAX30101_MultiLEDModeControlTime4Time3 = 0x12, 

    /// MAX30101 register 0x1F: r/o Die Temperature integer
    MAX30101_DieTemperatureInteger = 0x1F, 

    /// MAX30101 register 0x20: r/o Die Temperature fraction
    MAX30101_DieTemperatureFraction = 0x20, 

    /// MAX30101 register 0x21: r/o? Die Temperature Config (write 0x01 to trigger a conversion)
    MAX30101_DieTemperatureConfig = 0x21, 

    /// MAX30101 register 0x30: r/w Proximity Interrupt Threshold
    MAX30101_ProximityInterruptThreshold = 0x30, 

    /// MAX30101 register 0xFE: r/o Revision ID
    MAX30101_RevisionID = 0xFE, 

    /// MAX30101 register 0xFF: r/o Part ID (constant 0x15)
    MAX30101_DeviceID = 0xFF, 

} max30101_regAddr_enum;

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

#endif /* __MAX30101_H__ */
