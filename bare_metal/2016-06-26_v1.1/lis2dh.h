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

#ifndef __LIS2DH_H__
#define __LIS2DH_H__

// OS24EVK-58 Software Toolkit - bare metal device driver

// OS24EVK-66 Global LIS2DH device address
extern uint8_t gLIS2DHdeviceAddress;

void lis2dh_open();
uint8_t lis2dh_configure_outputDataRateHz(uint16_t outputDataRateHz);
uint8_t lis2dh_configure_resolutionBits(uint8_t resolutionBits);
uint8_t lis2dh_configure_selfTest(uint8_t selfTest);
uint8_t lis2dh_configure_BDU(uint8_t bduBlockDataUpdate);
uint8_t lis2dh_configure_FIFO(uint8_t fifoModeFM10, uint8_t fifoTR, uint8_t fifoWatermarkLevelFTH40);

int8_t lis2dh_RegRead(uint8_t regAddress, uint8_t *pRegValue);
int8_t lis2dh_RegWrite(uint8_t regAddress, uint8_t regValue);
int16_t lis2dh_Read_16Bit(uint8_t regAddress_L, uint8_t regAddress_H, int16_t* resultPtr);
void lis2dh_Read_XYZ(int16_t* XPtr, int16_t* YPtr, int16_t* ZPtr);

/// I2C register addresses within the device
typedef enum lis2dh_regAddr_enum_t {
    lis2dh_STATUS_REG_AUX  = 0x07, //!< (read-only) -- TOR -- -- -- TDA -- --
    lis2dh_OUT_TEMP_L      = 0x0C, //!< (read-only) Temperature sensor data.
    lis2dh_OUT_TEMP_H      = 0x0D, //!< (read-only) Temperature sensor data.
    lis2dh_INT_COUNTER_REG = 0x0E, //!< (read-only) 
    lis2dh_WHO_AM_I        = 0x0F, //!< (read-only) (constant 0x33) Device identification register.
    lis2dh_TEMP_CFG_REG    = 0x1F, //!< TEMP_EN1 TEMP_EN0 0 0 0 0 0 0 (TEMP_EN1:0=00:disabled;11=enabled)
    lis2dh_CTRL_REG1       = 0x20, //!< ODR3 ODR2 ODR1 ODR0 LPen Zen Yen Xen (ODR3:0=0000:PowerDown; 0101:100Hz)
    lis2dh_CTRL_REG2       = 0x21, //!< HPM1 HPM0 HPCF2 HPCF1 FDS HPCLICK HPIS2 HPIS1
    lis2dh_CTRL_REG3       = 0x22, //!< I1_CLICK I1_AOI1 I1_AOI2 I1_DRDY1 I1_DRDY2 I1_WTM I1_OVERRUN --
    lis2dh_CTRL_REG4       = 0x23, //!< BDU BLE FS1 FS0 HR ST1 ST0 SIM (ST1:ST0 self test mode 10 or 01)
    lis2dh_CTRL_REG5       = 0x24, //!< BOOT FIFO_EN -- -- LIR_INT1 D4D_INT1 LIR_INT2 D4D_INT2
    lis2dh_CTRL_REG6       = 0x25, //!< I2_CLICKen I2_INT1 I2_INT2 BOOT_I2 P2_ACT -- -- H_LACTIVE --
    lis2dh_REFERENCE       = 0x26, //!< Reference value for Interrupt generation.
    lis2dh_STATUS_REG2     = 0x27, //!< (read-only) ZYXOR ZOR YOR XOR ZYXDA ZDA YDA XDA
    lis2dh_OUT_X_L         = 0x28, //!< (read-only) X-axis acceleration data. The value is expressed as two's complement left justified.
    lis2dh_OUT_X_H         = 0x29, //!< (read-only) X-axis acceleration data. The value is expressed as two's complement left justified.
    lis2dh_OUT_Y_L         = 0x2A, //!< (read-only) Y-axis acceleration data. The value is expressed as two's complement left justified.
    lis2dh_OUT_Y_H         = 0x2B, //!< (read-only) Y-axis acceleration data. The value is expressed as two's complement left justified.
    lis2dh_OUT_Z_L         = 0x2C, //!< (read-only) Z-axis acceleration data. The value is expressed as two's complement left justified.
    lis2dh_OUT_Z_H         = 0x2D, //!< (read-only) Z-axis acceleration data. The value is expressed as two's complement left justified.
    lis2dh_FIFO_CTRL_REG   = 0x2E, //!< FM1 FM0 TR FTH4 FTH3 FTH2 FTH1 FTH0
    lis2dh_FIFO_SRC_REG    = 0x2F, //!< (read-only) WTM OVRN_FIFO EMPTY FSS4 FSS3 FSS2 FSS1 FSS0
    lis2dh_INT1_CFG        = 0x30, //!< AOI 6D ZHIE/ZUPE ZLIE/ZDOWNE YHIE/YUPE YLIE/YDOWNE XHIE/XUPE XLIE/XDOWNE
    lis2dh_INT1_SOURCE     = 0x31, //!< (read-only) 0 IA ZH ZL YH YL XH XL
    lis2dh_INT1_THS        = 0x32, //!< Interrupt 1 threshold. 1LSb = 16mg FS=2g; 1LSb = 32 mg FS=4g; 1LSb = 62 mg FS=8g; 1LSb = 186 mg FS=16g
    lis2dh_INT1_DURATION   = 0x33, //!< D6 - D0 bits set the minimum duration of the Interrupt 2 event to be recognized. Duration steps and maximum values depend on the ODR chosen.
    lis2dh_INT2_CFG        = 0x34, //!< AOI 6D ZHIE ZLIE YHIE YLIE XHIE XLIE
    lis2dh_INT2_SOURCE     = 0x35, //!< (read-only) 0 IA ZH ZL YH YL XH XL
    lis2dh_INT2_THS        = 0x36, //!< Interrupt 2 threshold. 1LSb = 16mg FS=2g; 1LSb = 32mg FS=4g; 1LSb = 62mg FS=8g; 1LSb = 186mg FS=16g
    lis2dh_INT2_DURATION   = 0x37, //!< D6 - D0 bits set the minimum duration of the Interrupt 2 event to be recognized. Duration steps and maximum values depend on the ODR chosen.
    lis2dh_CLICK_CFG       = 0x38, //!< -- -- ZD ZS YD YS XD XS
    lis2dh_CLICK_SRC       = 0x39, //!< (read-only) -- IA DClick SClick Sign Z Y X
    lis2dh_CLICK_THS       = 0x3A, //!< -- Ths6 Ths5 Ths4 Ths3 Ths2 Ths1 Ths0
    lis2dh_TIME_LIMIT      = 0x3B, //!< -- TLI6 TLI5 TLI4 TLI3 TLI2 TLI1 TLI0
    lis2dh_TIME_LATENCY    = 0x3C, //!< TLA7 TLA6 TLA5 TLA4 TLA3 TLA2 TLA1 TLA0
    lis2dh_TIME_WINDOW     = 0x3D, //!< TW7 TW6 TW5 TW4 TW3 TW2 TW1 TW0
    lis2dh_Act_THS         = 0x3E, //!< Sleep to wake, return to Sleep activation threshold in Low power mode. 1LSb = 16mg FS=2g; 1LSb = 32 mg FS=4g; 1LSb = 62 mg FS=8g; 1LSb = 186 mg FS=16g
    lis2dh_Act_DUR         = 0x3F, //!< Sleep to Wake, Return to Sleep duration 1LSb = (8*1[LSb]+1)/ODR
} lis2dh_regAddr_enum;

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

extern uint8_t lis2dh_configure_FIFO(uint8_t fifoModeFM10, uint8_t fifoTR, uint8_t fifoWatermarkLevelFTH40);

#endif /* __LIS2DH_H__ */
