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

// ----------------------------------------------------------------------------
// Header files
// ----------------------------------------------------------------------------

// Project-level preprocessor #defines effective across all *.c files
#include "ProjectDefines.h"

// OS24EVK-58 Software Toolkit - bare metal device driver
#if SUPPORT_MAX30101
#include "max30101.h"
#endif // SUPPORT_MAX30101

#if SUPPORT_LIS2DH
#include "lis2dh.h"
#endif // SUPPORT_LIS2DH

#if SUPPORT_MAX7311
#include "max7311.h"
#endif // SUPPORT_MAX7311

// OS24EVK-66 Global LIS2DH device address gLIS2DHdeviceAddress
#if SUPPORT_LIS2DH
uint8_t gLIS2DHdeviceAddress = 0x00; //!< (default 0x32) LIS2DH I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.
code uint8_t searchLIS2DHdeviceAddressList[] = {
   0x30, //!< 0x30 is optional STMicro LIS2DH with SDO/SA0 pin low
   0x32, //!< 0x32 is optional STMicro LIS2DH with SDO/SA0 pin high
   0 //<! end of list
}; //!< LIS2DH I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.
#endif // SUPPORT_LIS2DH


// OS24EVK-66 LIS2DH Accelerometer Register Read
#if SUPPORT_LIS2DH
int8_t lis2dh_RegRead(uint8_t regAddress, uint8_t *pRegValue)
{
   return i2c_smbus_read_byte_data(gLIS2DHdeviceAddress, regAddress, pRegValue);
}
#endif // SUPPORT_LIS2DH

// OS24EVK-66 LIS2DH Accelerometer Register Write
#if SUPPORT_LIS2DH
int8_t lis2dh_RegWrite(uint8_t regAddress, uint8_t regValue)
{
   return i2c_smbus_write_byte_data(gLIS2DHdeviceAddress, regAddress, regValue);
}
#endif // SUPPORT_LIS2DH

// read lis2dh 16-bit pair of registers
#if SUPPORT_LIS2DH
int16_t lis2dh_Read_16Bit(uint8_t regAddress_L, uint8_t regAddress_H, int16_t* resultPtr)
{
   // TODO: OS24EVK-66 LIS2DH Accelerometer Register Read 16-bit pair
   uint8_t regValue_L = 0;
   uint8_t regValue_H = 0;
   uint16_t regValue_HL = 0;
   //
   // C not C++, so all local variables must be declared before any statements
   //
   if (!gLIS2DHdeviceAddress) {
      return 0; // device is not available
   }
   //
#if SUPPORT_i2c_smbus_read_bytes_data
   // TODO1: use i2c_smbus_read_bytes_data(uint8_t deviceAddress, uint8_t regAddress, uint8_t count, uint8_t *pRegValue)
   // assuming address OUT_X_H = 1 + address OUT_X_L
   i2c_smbus_read_bytes_data(gLIS2DHdeviceAddress, regAddress_L, 2, &regValue_HL);
   regValue_HL = SWAP_HIGH_AND_LOW_BYTE(regValue_HL);
#else // SUPPORT_i2c_smbus_read_bytes_data
   lis2dh_RegRead(regAddress_H, &regValue_H);
   lis2dh_RegRead(regAddress_L, &regValue_L);
   regValue_HL = (((int16_t)regValue_H << 8) & 0xFF00) | (regValue_L & 0x00FF);
#endif // SUPPORT_i2c_smbus_read_bytes_data
   //
   if (resultPtr != 0) {
      *(resultPtr) = regValue_HL;
   }
   return regValue_HL;
}
#endif // SUPPORT_LIS2DH

#if SUPPORT_LIS2DH
// read li2sdh X Y Z acceleration vector
void lis2dh_Read_XYZ(int16_t* XPtr, int16_t* YPtr, int16_t* ZPtr)
{
   // TODO: OS24EVK-66 LIS2DH Accelerometer Read XYZ vector
   //
   // C not C++, so all local variables must be declared before any statements
   //
   if (!gLIS2DHdeviceAddress) {
      return; // device is not available
   }
   //
// #if SUPPORT_i2c_smbus_read_bytes_data
// TODO1: OS24EVK-66 Read LIS2DH XYZ using a single 6-byte I2C read operation in lis2dh_Read_XYZ()
// // TODO1: use i2c_smbus_read_bytes_data(uint8_t deviceAddress, uint8_t regAddress, uint8_t count, uint8_t *pRegValue)
   // 2015-04-16 OS24EVK-66 OS24EVK-57 LIS2DH multiple-byte read requires (regAddress | 0x80)
   // LIS2DH data sheet pg 25 5.1.1 I2C operation (near last paragraph)
   // In order to read multiple bytes, it is necessary to assert the most significant bit of the subaddress
   // field. In other words, SUB(7) must be equal to 1 while SUB(6-0) represents the
   // address of first register to be read.
// #else // SUPPORT_i2c_smbus_read_bytes_data
   lis2dh_Read_16Bit(lis2dh_OUT_X_L, lis2dh_OUT_X_H, XPtr);
   lis2dh_Read_16Bit(lis2dh_OUT_Y_L, lis2dh_OUT_Y_H, YPtr);
   lis2dh_Read_16Bit(lis2dh_OUT_Z_L, lis2dh_OUT_Z_H, ZPtr);
// #endif // SUPPORT_i2c_smbus_read_bytes_data
}
#endif // SUPPORT_LIS2DH

#if SUPPORT_LIS2DH
uint8_t lis2dh_configure_outputDataRateHz(uint16_t outputDataRateHz)
{
    uint8_t lis2dh_CTRL_REG1_value = 0; //!< ODR3 ODR2 ODR1 ODR0 LPen Zen Yen Xen (ODR3:0=0000:PowerDown; 0101:100Hz)
    //~ uint8_t lis2dh_CTRL_REG2_value = 0; //!< HPM1 HPM0 HPCF2 HPCF1 FDS HPCLICK HPIS2 HPIS1
    //~ uint8_t lis2dh_CTRL_REG3_value = 0; //!< I1_CLICK I1_AOI1 I1_AOI2 I1_DRDY1 I1_DRDY2 I1_WTM I1_OVERRUN --
    //~ uint8_t lis2dh_CTRL_REG4_value = 0; //!< BDU BLE FS1 FS0 HR ST1 ST0 SIM (ST1:ST0 self test mode 10 or 01)
    //~ uint8_t lis2dh_CTRL_REG5_value = 0; //!< BOOT FIFO_EN -- -- LIR_INT1 D4D_INT1 LIR_INT2 D4D_INT2
    //~ uint8_t lis2dh_CTRL_REG6_value = 0; //!< I2_CLICKen I2_INT1 I2_INT2 BOOT_I2 P2_ACT -- -- H_LACTIVE --
    //
    // C not C++, so all local variables must be declared before any statements
    //
   if (!gLIS2DHdeviceAddress) {
      return 0; // device is not available
   }
    //
    lis2dh_RegRead(lis2dh_CTRL_REG1, &lis2dh_CTRL_REG1_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG2, &lis2dh_CTRL_REG2_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG3, &lis2dh_CTRL_REG3_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG4, &lis2dh_CTRL_REG4_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG5, &lis2dh_CTRL_REG5_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG6, &lis2dh_CTRL_REG6_value);
    //
    switch(outputDataRateHz)
    {
    case 0:
        // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0: 0 0 0 0 Power down mode
        lis2dh_CTRL_REG1_value &=~ (0x0F << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        lis2dh_CTRL_REG1_value |=  (0x00 << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        break;
    case 1:
        // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0: 0 0 0 1 HR / normal / Low power mode (1 Hz)
        lis2dh_CTRL_REG1_value &=~ (0x0F << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        lis2dh_CTRL_REG1_value |=  (0x01 << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        break;
    case 10:
        // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0: 0 0 1 0 HR / normal / Low power mode (10 Hz)
        lis2dh_CTRL_REG1_value &=~ (0x0F << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        lis2dh_CTRL_REG1_value |=  (0x02 << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        break;
    case 25:
        // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0: 0 0 1 1 HR / normal / Low power mode (25 Hz)
        lis2dh_CTRL_REG1_value &=~ (0x0F << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        lis2dh_CTRL_REG1_value |=  (0x03 << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        break;
    case 50:
        // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0: 0 1 0 0 HR / normal / Low power mode (50 Hz)
        lis2dh_CTRL_REG1_value &=~ (0x0F << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        lis2dh_CTRL_REG1_value |=  (0x04 << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        break;
    case 100:
        // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0: 0 1 0 1 HR / normal / Low power mode (100 Hz)
        lis2dh_CTRL_REG1_value &=~ (0x0F << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        lis2dh_CTRL_REG1_value |=  (0x05 << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        break;
    case 200:
        // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0: 0 1 1 0 HR / normal / Low power mode (200 Hz)
        lis2dh_CTRL_REG1_value &=~ (0x0F << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        lis2dh_CTRL_REG1_value |=  (0x06 << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        break;
    case 400:
        // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0: 0 1 1 1 HR/ normal / Low power mode (400 Hz)
        lis2dh_CTRL_REG1_value &=~ (0x0F << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        lis2dh_CTRL_REG1_value |=  (0x07 << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        break;
    case 1620:
        // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0: 1 0 0 0 Low power mode (1.620 kHz)
        lis2dh_CTRL_REG1_value &=~ (0x0F << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        lis2dh_CTRL_REG1_value |=  (0x08 << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        break;
    case 1344:
        // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0: 1 0 0 1 HR/ normal (1.344 kHz); Low power mode (5.376 kHz)
        lis2dh_CTRL_REG1_value &=~ (0x0F << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        lis2dh_CTRL_REG1_value |=  (0x09 << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        break;
    case 5376:
        // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0: 1 0 0 1 HR/ normal (1.344 kHz); Low power mode (5.376 kHz)
        lis2dh_CTRL_REG1_value &=~ (0x0F << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        lis2dh_CTRL_REG1_value |=  (0x09 << 4); // lis2dh_CTRL_REG1 ODR3 ODR2 ODR1 ODR0
        break;
    default:
        return 0; // failure; invalid configuration
    }
    //
    lis2dh_RegWrite(lis2dh_CTRL_REG1, lis2dh_CTRL_REG1_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG2, lis2dh_CTRL_REG2_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG3, lis2dh_CTRL_REG3_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG4, lis2dh_CTRL_REG4_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG5, lis2dh_CTRL_REG5_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG6, lis2dh_CTRL_REG6_value);
    //
    return 1; // success
}
#endif // SUPPORT_LIS2DH

#if SUPPORT_LIS2DH
uint8_t lis2dh_configure_resolutionBits(uint8_t resolutionBits)
{
    uint8_t lis2dh_CTRL_REG1_value = 0; //!< ODR3 ODR2 ODR1 ODR0 LPen Zen Yen Xen (ODR3:0=0000:PowerDown; 0101:100Hz)
    //~ uint8_t lis2dh_CTRL_REG2_value = 0; //!< HPM1 HPM0 HPCF2 HPCF1 FDS HPCLICK HPIS2 HPIS1
    //~ uint8_t lis2dh_CTRL_REG3_value = 0; //!< I1_CLICK I1_AOI1 I1_AOI2 I1_DRDY1 I1_DRDY2 I1_WTM I1_OVERRUN --
    uint8_t lis2dh_CTRL_REG4_value = 0; //!< BDU BLE FS1 FS0 HR ST1 ST0 SIM (ST1:ST0 self test mode 10 or 01)
    //~ uint8_t lis2dh_CTRL_REG5_value = 0; //!< BOOT FIFO_EN -- -- LIR_INT1 D4D_INT1 LIR_INT2 D4D_INT2
    //~ uint8_t lis2dh_CTRL_REG6_value = 0; //!< I2_CLICKen I2_INT1 I2_INT2 BOOT_I2 P2_ACT -- -- H_LACTIVE --
    //
    // C not C++, so all local variables must be declared before any statements
    //
   if (!gLIS2DHdeviceAddress) {
      return 0; // device is not available
   }
    //
    lis2dh_RegRead(lis2dh_CTRL_REG1, &lis2dh_CTRL_REG1_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG2, &lis2dh_CTRL_REG2_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG3, &lis2dh_CTRL_REG3_value);
    lis2dh_RegRead(lis2dh_CTRL_REG4, &lis2dh_CTRL_REG4_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG5, &lis2dh_CTRL_REG5_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG6, &lis2dh_CTRL_REG6_value);
    //
    switch(resolutionBits)
    {
    case 12:
        // 12-bit mode: lis2dh_CTRL_REG1 bit 0x08 LPen = 0, lis2dh_CTRL_REG4 bit 0x08 HR = 1
        lis2dh_CTRL_REG1_value &=~ (0x01 << 3); // lis2dh_CTRL_REG1 LPen
        lis2dh_CTRL_REG4_value |=  (0x01 << 3); // lis2dh_CTRL_REG4 HR
        break;
    case 10:
        // 10-bit mode: lis2dh_CTRL_REG1 bit 0x08 LPen = 0, lis2dh_CTRL_REG4 bit 0x08 HR = 0
        lis2dh_CTRL_REG1_value &=~ (0x01 << 3); // lis2dh_CTRL_REG1 LPen
        lis2dh_CTRL_REG4_value &=~ (0x01 << 3); // lis2dh_CTRL_REG4 HR
        break;
    case 8:
        //  8-bit mode: lis2dh_CTRL_REG1 bit 0x08 LPen = 1, lis2dh_CTRL_REG4 bit 0x08 HR = 0
        lis2dh_CTRL_REG1_value |=  (0x01 << 3); // lis2dh_CTRL_REG1 LPen
        lis2dh_CTRL_REG4_value &=~ (0x01 << 3); // lis2dh_CTRL_REG4 HR
        break;
    default:
        return 0; // failure; invalid configuration
    }
    //
    lis2dh_RegWrite(lis2dh_CTRL_REG1, lis2dh_CTRL_REG1_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG2, lis2dh_CTRL_REG2_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG3, lis2dh_CTRL_REG3_value);
    lis2dh_RegWrite(lis2dh_CTRL_REG4, lis2dh_CTRL_REG4_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG5, lis2dh_CTRL_REG5_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG6, lis2dh_CTRL_REG6_value);
    //
    return 1; // success
}
#endif // SUPPORT_LIS2DH

#if 0 // SUPPORT_LIS2DH
uint8_t lis2dh_configure_selfTest(uint8_t selfTest)
{
    //~ uint8_t lis2dh_CTRL_REG1_value = 0; //!< ODR3 ODR2 ODR1 ODR0 LPen Zen Yen Xen (ODR3:0=0000:PowerDown; 0101:100Hz)
    //~ uint8_t lis2dh_CTRL_REG2_value = 0; //!< HPM1 HPM0 HPCF2 HPCF1 FDS HPCLICK HPIS2 HPIS1
    //~ uint8_t lis2dh_CTRL_REG3_value = 0; //!< I1_CLICK I1_AOI1 I1_AOI2 I1_DRDY1 I1_DRDY2 I1_WTM I1_OVERRUN --
    uint8_t lis2dh_CTRL_REG4_value = 0; //!< BDU BLE FS1 FS0 HR ST1 ST0 SIM (ST1:ST0 self test mode 10 or 01)
    //~ uint8_t lis2dh_CTRL_REG5_value = 0; //!< BOOT FIFO_EN -- -- LIR_INT1 D4D_INT1 LIR_INT2 D4D_INT2
    //~ uint8_t lis2dh_CTRL_REG6_value = 0; //!< I2_CLICKen I2_INT1 I2_INT2 BOOT_I2 P2_ACT -- -- H_LACTIVE --
    //
    // C not C++, so all local variables must be declared before any statements
    //
   if (!gLIS2DHdeviceAddress) {
      return 0; // device is not available
   }
    //
    //~ lis2dh_RegRead(lis2dh_CTRL_REG1, &lis2dh_CTRL_REG1_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG2, &lis2dh_CTRL_REG2_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG3, &lis2dh_CTRL_REG3_value);
    lis2dh_RegRead(lis2dh_CTRL_REG4, &lis2dh_CTRL_REG4_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG5, &lis2dh_CTRL_REG5_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG6, &lis2dh_CTRL_REG6_value);
    //
    switch(selfTest)
    {
    case 0:
        // Normal mode: lis2dh_CTRL_REG4 ST1:ST0 = 00
        lis2dh_CTRL_REG4_value &=~ (0x03 << 1); // lis2dh_CTRL_REG4 ST1:ST0
        lis2dh_CTRL_REG4_value |=  (0x00 << 1); // lis2dh_CTRL_REG4 ST1:ST0
        break;
    case 1:
        // Self test 0: lis2dh_CTRL_REG4 ST1:ST0 = 01
        lis2dh_CTRL_REG4_value &=~ (0x03 << 1); // lis2dh_CTRL_REG4 ST1:ST0
        lis2dh_CTRL_REG4_value |=  (0x01 << 1); // lis2dh_CTRL_REG4 ST1:ST0
        break;
    case 2:
        // Self test 1: lis2dh_CTRL_REG4 ST1:ST0 = 10
        lis2dh_CTRL_REG4_value &=~ (0x03 << 1); // lis2dh_CTRL_REG4 ST1:ST0
        lis2dh_CTRL_REG4_value |=  (0x02 << 1); // lis2dh_CTRL_REG4 ST1:ST0
        break;
    default:
        return 0; // failure; invalid configuration
    }
    //
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG1, lis2dh_CTRL_REG1_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG2, lis2dh_CTRL_REG2_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG3, lis2dh_CTRL_REG3_value);
    lis2dh_RegWrite(lis2dh_CTRL_REG4, lis2dh_CTRL_REG4_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG5, lis2dh_CTRL_REG5_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG6, lis2dh_CTRL_REG6_value);
    //
    return 1; // success
}
#endif // SUPPORT_LIS2DH

#if SUPPORT_LIS2DH
uint8_t lis2dh_configure_BDU(uint8_t bduBlockDataUpdate)
{
    //~ uint8_t lis2dh_CTRL_REG1_value = 0; //!< ODR3 ODR2 ODR1 ODR0 LPen Zen Yen Xen (ODR3:0=0000:PowerDown; 0101:100Hz)
    //~ uint8_t lis2dh_CTRL_REG2_value = 0; //!< HPM1 HPM0 HPCF2 HPCF1 FDS HPCLICK HPIS2 HPIS1
    //~ uint8_t lis2dh_CTRL_REG3_value = 0; //!< I1_CLICK I1_AOI1 I1_AOI2 I1_DRDY1 I1_DRDY2 I1_WTM I1_OVERRUN --
    uint8_t lis2dh_CTRL_REG4_value = 0; //!< BDU BLE FS1 FS0 HR ST1 ST0 SIM (ST1:ST0 self test mode 10 or 01)
    //~ uint8_t lis2dh_CTRL_REG5_value = 0; //!< BOOT FIFO_EN -- -- LIR_INT1 D4D_INT1 LIR_INT2 D4D_INT2
    //~ uint8_t lis2dh_CTRL_REG6_value = 0; //!< I2_CLICKen I2_INT1 I2_INT2 BOOT_I2 P2_ACT -- -- H_LACTIVE --
    //
    // C not C++, so all local variables must be declared before any statements
    //
   if (!gLIS2DHdeviceAddress) {
      return 0; // device is not available
   }
    //
    //~ lis2dh_RegRead(lis2dh_CTRL_REG1, &lis2dh_CTRL_REG1_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG2, &lis2dh_CTRL_REG2_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG3, &lis2dh_CTRL_REG3_value);
    lis2dh_RegRead(lis2dh_CTRL_REG4, &lis2dh_CTRL_REG4_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG5, &lis2dh_CTRL_REG5_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG6, &lis2dh_CTRL_REG6_value);
    //
    switch(bduBlockDataUpdate)
    {
    case 0:
        // Normal mode: lis2dh_CTRL_REG4 BDU = 0
        lis2dh_CTRL_REG4_value &=~ (0x01 << 7); // lis2dh_CTRL_REG4 BDU
        lis2dh_CTRL_REG4_value |=  (0x00 << 7); // lis2dh_CTRL_REG4 BDU
        break;
    case 1:
        // Self test 0: lis2dh_CTRL_REG4 BDU = 1
        lis2dh_CTRL_REG4_value &=~ (0x01 << 7); // lis2dh_CTRL_REG4 BDU
        lis2dh_CTRL_REG4_value |=  (0x01 << 7); // lis2dh_CTRL_REG4 BDU
        break;
    default:
        return 0; // failure; invalid configuration
    }
    //
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG1, lis2dh_CTRL_REG1_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG2, lis2dh_CTRL_REG2_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG3, lis2dh_CTRL_REG3_value);
    lis2dh_RegWrite(lis2dh_CTRL_REG4, lis2dh_CTRL_REG4_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG5, lis2dh_CTRL_REG5_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG6, lis2dh_CTRL_REG6_value);
    //
    return 1; // success
}
#endif // SUPPORT_LIS2DH

#if SUPPORT_LIS2DH
uint8_t lis2dh_configure_FIFO(uint8_t fifoModeFM10, uint8_t fifoTR, uint8_t fifoWatermarkLevelFTH40)
{
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    xdata uint8_t lis2dh_CTRL_REG1_value = 0; //!< ODR3 ODR2 ODR1 ODR0 LPen Zen Yen Xen (ODR3:0=0000:PowerDown; 0101:100Hz)
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    xdata uint8_t lis2dh_CTRL_REG2_value = 0; //!< HPM1 HPM0 HPCF2 HPCF1 FDS HPCLICK HPIS2 HPIS1
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    xdata uint8_t lis2dh_CTRL_REG3_value = 0; //!< I1_CLICK I1_AOI1 I1_AOI2 I1_DRDY1 I1_DRDY2 I1_WTM I1_OVERRUN --
    //~ uint8_t lis2dh_CTRL_REG4_value = 0; //!< BDU BLE FS1 FS0 HR ST1 ST0 SIM (ST1:ST0 self test mode 10 or 01)
    xdata uint8_t lis2dh_CTRL_REG5_value = 0; //!< BOOT FIFO_EN -- -- LIR_INT1 D4D_INT1 LIR_INT2 D4D_INT2
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    //~ xdata uint8_t lis2dh_CTRL_REG6_value = 0; //!< I2_CLICKen I2_INT1 I2_INT2 BOOT_I2 P2_ACT -- -- H_LACTIVE --
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    xdata uint8_t lis2dh_FIFO_CTRL_REG_value = 0; //!< FM1 FM0 TR FTH4 FTH3 FTH2 FTH1 FTH0
    //~ uint8_t lis2dh_FIFO_SRC_REG_value = 0;  //!< (read-only) WTM OVRN_FIFO EMPTY FSS4 FSS3 FSS2 FSS1 FSS0
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    xdata uint8_t lis2dh_INT1_CFG_value = 0; //!< AOI 6D ZHIE/ZUPE ZLIE/ZDOWNE YHIE/YUPE YLIE/YDOWNE XHIE/XUPE XLIE/XDOWNE
    //~ uint8_t lis2dh_INT1_SOURCE_value = 0; //!< (read-only) 0 IA ZH ZL YH YL XH XL
    xdata uint8_t lis2dh_INT1_THS_value = 0; //!< Interrupt 1 threshold. 1LSb = 16mg FS=2g; 1LSb = 32 mg FS=4g; 1LSb = 62 mg FS=8g; 1LSb = 186 mg FS=16g
    xdata uint8_t lis2dh_INT1_DURATION_value = 0; //!< D6 - D0 bits set the minimum duration of the Interrupt 2 event to be recognized. Duration steps and maximum values depend on the ODR chosen.
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    //
    // C not C++, so all local variables must be declared before any statements
    //
   if (!gLIS2DHdeviceAddress) {
      return 0; // device is not available
   }
   
   //
   // OS24EVK-66 lis2dh_configure_FIFO GOTCHA! need lis2dh_CTRL_REG5 bit 0x40 FIFO_EN, and maybe lis2dh_CTRL_REG3 bit 0x04 I1_WTM
   //
   
    //
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    lis2dh_RegRead(lis2dh_CTRL_REG1, &lis2dh_CTRL_REG1_value);
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    lis2dh_RegRead(lis2dh_CTRL_REG2, &lis2dh_CTRL_REG2_value);
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    lis2dh_RegRead(lis2dh_CTRL_REG3, &lis2dh_CTRL_REG3_value);
    //~ lis2dh_RegRead(lis2dh_CTRL_REG4, &lis2dh_CTRL_REG4_value);
    lis2dh_RegRead(lis2dh_CTRL_REG5, &lis2dh_CTRL_REG5_value);
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    //~ lis2dh_RegRead(lis2dh_CTRL_REG6, &lis2dh_CTRL_REG6_value);
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    lis2dh_RegRead(lis2dh_FIFO_CTRL_REG, &lis2dh_FIFO_CTRL_REG_value);
    //~ lis2dh_RegRead(lis2dh_FIFO_SRC_REG,  &lis2dh_FIFO_SRC_REG_value);
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    lis2dh_RegRead(lis2dh_INT1_CFG, &lis2dh_INT1_CFG_value);
    //~ lis2dh_RegRead(lis2dh_INT1_SOURCE, &lis2dh_INT1_SOURCE_value);
    lis2dh_RegRead(lis2dh_INT1_THS, &lis2dh_INT1_THS_value);
    lis2dh_RegRead(lis2dh_INT1_DURATION, &lis2dh_INT1_DURATION_value);
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    //
    switch(fifoModeFM10)
    {
    case 0:
        // Bypass Mode: lis2dh_FIFO_CTRL_REG FM1:FM0 = 00
        lis2dh_FIFO_CTRL_REG_value &=~ (0x03 << 6); // lis2dh_CTRL_REG4 FM1:FM0
        lis2dh_FIFO_CTRL_REG_value |=  (0x00 << 6); // lis2dh_CTRL_REG4 FM1:FM0 = 00
        lis2dh_CTRL_REG5_value &=~  (0x01 << 6); // lis2dh_CTRL_REG5 bit 0x40 FIFO_EN = 0
        break;
    case 1:
        // "FIFO Mode": lis2dh_FIFO_CTRL_REG FM1:FM0 = 01
        lis2dh_FIFO_CTRL_REG_value &=~ (0x03 << 6); // lis2dh_CTRL_REG4 FM1:FM0
        lis2dh_FIFO_CTRL_REG_value |=  (0x01 << 6); // lis2dh_CTRL_REG4 FM1:FM0 = 01
        lis2dh_CTRL_REG5_value |=  (0x01 << 6); // lis2dh_CTRL_REG5 bit 0x40 FIFO_EN = 1
        break;
    case 2:
        // Stream Mode: lis2dh_FIFO_CTRL_REG FM1:FM0 = 10
        lis2dh_FIFO_CTRL_REG_value &=~ (0x03 << 6); // lis2dh_CTRL_REG4 FM1:FM0
        lis2dh_FIFO_CTRL_REG_value |=  (0x02 << 6); // lis2dh_CTRL_REG4 FM1:FM0 = 10
        lis2dh_CTRL_REG5_value |=  (0x01 << 6); // lis2dh_CTRL_REG5 bit 0x40 FIFO_EN = 1
        //
        // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 enable LIS2DH interrupt INT1: EX1 = 1
        // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH interrupt source
        //
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH interrupt source
        lis2dh_CTRL_REG1_value |=  (0x07 << 0); // lis2dh_CTRL_REG1 Zen = 1, Yen = 1, Xen = 1
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
        lis2dh_CTRL_REG2_value |=  (0x01 << 0); // lis2dh_CTRL_REG2 HPIS1 = 1
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
        lis2dh_CTRL_REG3_value |=  (0x01 << 2); // lis2dh_CTRL_REG3 bit 0x04 I1_WTM should be 1 to enable FIFO Watermark interrupt on INT1 pin.
        lis2dh_CTRL_REG3_value |=  (0x01 << 1); // lis2dh_CTRL_REG3 bit 0x02 I1_OVERRUN should be 1 to enable FIFO Overrun interrupt on INT1 pin.
        //
        lis2dh_CTRL_REG5_value |=  (0x01 << 3); // lis2dh_CTRL_REG5 bit 0x08 LIR_INT1 Latch Interrupt; clear INT by reading INT1_SRC register
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
        lis2dh_CTRL_REG5_value |=  (0x01 << 0); // lis2dh_CTRL_REG5 D4D_INT2
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
        //
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
        //~ lis2dh_CTRL_REG6_value |=  (0x01 << 1); // lis2dh_CTRL_REG6 bit 0x02 H_LACTIVE interrupt active low
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
        //~ lis2dh_CTRL_REG6_value |=  (0x01 << 5); // lis2dh_CTRL_REG6 I2_INT2
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
        lis2dh_INT1_CFG_value = 0x7f; // 6 direction movement recognition
        lis2dh_INT1_THS_value = 0x04;
        lis2dh_INT1_DURATION_value = 0x00;
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
        //
        break;
    case 3:
        // Trigger Mode: lis2dh_FIFO_CTRL_REG FM1:FM0 = 10
        lis2dh_FIFO_CTRL_REG_value &=~ (0x03 << 6); // lis2dh_CTRL_REG4 FM1:FM0
        lis2dh_FIFO_CTRL_REG_value |=  (0x03 << 6); // lis2dh_CTRL_REG4 FM1:FM0 = 11
        lis2dh_CTRL_REG5_value |=  (0x01 << 6); // lis2dh_CTRL_REG5 bit 0x40 FIFO_EN = 1
        break;
    default:
        return 0; // failure; invalid configuration
    }
    //
    // FIFO TR bit (??)
    lis2dh_FIFO_CTRL_REG_value &=~ (  0x01 << 5); // lis2dh_CTRL_REG4 TR
    lis2dh_FIFO_CTRL_REG_value |=  ( (fifoTR & 0x01) << 5); // lis2dh_CTRL_REG4 TR
    //
    // FIFO watermark level (valid in fifoModeFM10 1 and 3)
    lis2dh_FIFO_CTRL_REG_value &=~ ( 0x1F << 0); // lis2dh_CTRL_REG4 FTH4:FTH0
    lis2dh_FIFO_CTRL_REG_value |=  ( (fifoWatermarkLevelFTH40 & 0x1F) << 0); // lis2dh_CTRL_REG4 FTH4:FTH0
    //
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    lis2dh_RegWrite(lis2dh_CTRL_REG1, lis2dh_CTRL_REG1_value); // lis2dh_CTRL_REG1 Zen = 1, Yen = 1, Xen = 1
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    lis2dh_RegWrite(lis2dh_CTRL_REG2, lis2dh_CTRL_REG2_value); // lis2dh_CTRL_REG2 HPIS1 = 1
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    lis2dh_RegWrite(lis2dh_CTRL_REG3, lis2dh_CTRL_REG3_value);
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG4, lis2dh_CTRL_REG4_value);
    lis2dh_RegWrite(lis2dh_CTRL_REG5, lis2dh_CTRL_REG5_value);
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    //~ lis2dh_RegWrite(lis2dh_CTRL_REG6, lis2dh_CTRL_REG6_value); // lis2dh_CTRL_REG6 I2_INT2 H_LACTIVE
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    lis2dh_RegWrite(lis2dh_FIFO_CTRL_REG, lis2dh_FIFO_CTRL_REG_value);
#if 0 // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    lis2dh_RegWrite(lis2dh_INT1_CFG, lis2dh_INT1_CFG_value); // 6 direction movement recognition
    //~ lis2dh_RegWrite(lis2dh_INT1_SOURCE, lis2dh_INT1_SOURCE_value);
    lis2dh_RegWrite(lis2dh_INT1_THS, lis2dh_INT1_THS_value);
    lis2dh_RegWrite(lis2dh_INT1_DURATION, lis2dh_INT1_DURATION_value);
#endif // TODO1: (Firmware 2015-04-26) OS24EVK-66 OS24EVK-83 configure LIS2DH
    //
    return 1; // success
}
#endif // SUPPORT_LIS2DH

// OS24EVK-66 LIS2DH Accelerometer Present?
// @post gLIS2DHdeviceAddress = 0 if Accelerometer not connected,
//       gLIS2DHdeviceAddress = Accelerometer device address if present
#if SUPPORT_LIS2DH
void lis2dh_open()
{
   // OS24EVK-66 Init: lis2dh_open() Search LIS2DH device address 0x30, 0x32 gLIS2DHdeviceAddress
   //
   int8_t i2cAddressIndex;
   int8_t DeviceID_regAddr;
   int8_t DeviceID_expect;
   int8_t DeviceID_actual;
   //
   // C not C++, so all local variables must be declared before any statements
   //
   // loop through list of deviceAddress to search
   // static uint8_t searchMAX30101deviceAddressList[] = {0xAE, 0};
   //
   // // example: lis2dh_RegRead NACK status
   // gLIS2DHdeviceAddress = 0x44; // MAX30101 0xAE
   //    DeviceID_regAddr = 0xFF; // MAX30101 register DeviceId (constant 0x15) Device identification register.
   //    DeviceID_expect = 0x15;
   //    DeviceID_actual = lis2dh_RegRead(DeviceID_regAddr, 0); // MAX30101
   //
   // // example: lis2dh_RegRead NACK status
   // gLIS2DHdeviceAddress = 0xAE; // MAX30101 0xAE
   //    DeviceID_regAddr = 0xFF; // MAX30101 register DeviceId (constant 0x15) Device identification register.
   //    DeviceID_expect = 0x15;
   //    DeviceID_actual = lis2dh_RegRead(DeviceID_regAddr, 0); // MAX30101
   //
   // // example: lis2dh_RegRead NACK status
   // gLIS2DHdeviceAddress = 0x32; // LIS2DH
   //    DeviceID_regAddr = 0x0F; // LIS2DH register WHO_AM_I (constant 0x33) Device identification register.
   //    DeviceID_expect = 0x33;
   //    DeviceID_actual = lis2dh_RegRead(DeviceID_regAddr, 0); // MAX30101
   //
   // Search the i2c address list for possible devices
   for (i2cAddressIndex = 0; searchLIS2DHdeviceAddressList[i2cAddressIndex] != 0; i2cAddressIndex++)
   {
      gLIS2DHdeviceAddress = searchLIS2DHdeviceAddressList[i2cAddressIndex];
      //
      //~ blockArr[5] = DeviceID_regAddr;
      //~ i2c_read(gLIS2DHdeviceAddress & 0xFE, blockArr, 1, 1);	
      //	blockArr[2+0] = value read from Device Register
      //
      DeviceID_regAddr = lis2dh_WHO_AM_I; // 0x0F; // LIS2DH register WHO_AM_I (constant 0x33) Device identification register.
      DeviceID_expect = 0x33;
      // DeviceID_actual = lis2dh_RegRead(DeviceID_regAddr, 0);
      if (lis2dh_RegRead(DeviceID_regAddr, &DeviceID_actual) < 0) {
         // typically for NACK, errno=5 and return value = -1
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_East();
         continue;
      } else if ((DeviceID_actual & 0xFF) != (DeviceID_expect & 0xFF)) {
         //~ printf("\n""lis2dh_open(): i2c address 0x%2.2X (8-bit 0x%2.2X) ACK"
         //~ "\n""                but reg 0x%2.2X %s=0x%2.2X not 0x%2.2X so wrong device",
         //~ global_lis2dh_device_data.i2cDeviceAddress7bits,
         //~ global_lis2dh_device_data.i2cDeviceAddress7bits * 2,
         //~ (DeviceID_regAddr & 0xFF),
         //~ lis2dh_RegisterAddressName((uint8_t)DeviceID_regAddr, 0, 0),
         //~ (DeviceID_actual & 0xFF),
         //~ (DeviceID_expect & 0xFF)
         //~ );
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_East();
         continue;
      } else {
         //~ printf("\n""lis2dh_open(): i2c address 0x%2.2X (8-bit 0x%2.2X) ACK; %s=0x%2.2X OK",
         //~ global_lis2dh_device_data.i2cDeviceAddress7bits,
         //~ global_lis2dh_device_data.i2cDeviceAddress7bits * 2,
         //~ lis2dh_RegisterAddressName((uint8_t)DeviceID_regAddr, 0, 0),
         //~ (DeviceID_actual & 0xFF)
         //~ );
         //
         // OS24EVK-66 Init: if LIS2DH present, initialize INT1 Active-Low by writing 0x25 CTRL_REG6 bit 0x02 H_LACTIVE=1
         lis2dh_RegWrite(lis2dh_CTRL_REG6, 0x02); //!< I2_CLICKen I2_INT1 I2_INT2 BOOT_I2 P2_ACT -- -- H_LACTIVE --
         // lis2dh_open() is the only place we write lis2dh_CTRL_REG6.
         //
         lis2dh_configure_outputDataRateHz(100);
         lis2dh_configure_resolutionBits(12);
         //~ lis2dh_configure_selfTest(0);
         //
         // TODO1: OS24EVK-66 Configure LIS2DH to enable FIFO buffer (get out of "bypass mode")
         //
         // Block data update. Default value: 0
         // (0: continuos update; 1: output registers not updated until MSB and LSB have
         // been read)
         lis2dh_configure_BDU( /* int16_t bduBlockDataUpdate */ 1);
         //
         // In bypass mode, the FIFO is not operational and for this reason it remains empty. As
         // described in the next figure, for each channel only the first address is used. The remaining
         // FIFO slots are empty.
         lis2dh_configure_FIFO(/* fifoModeFM10 0:Bypass */ 0, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
         //
         // In FIFO mode, data from X, Y and Z channels are stored into the FIFO. A watermark
         // interrupt can be enabled (FIFO_WTMK_EN bit into FIFO_CTRL_REG (2E) in order to be
         // raised when the FIFO is filled to the level specified into the FIFO_WTMK_LEVEL bits of
         // FIFO_CTRL_REG (2E). The FIFO continues filling until it is full (32 slots of data for X, Y and
         // Z). When full, the FIFO stops collecting data from the input channels.
#if 0 // OS24EVK-66 ISR Poll for 0x27 STATUS_REG2 bit 0x08 ZYXDA      
         lis2dh_configure_FIFO(/* fifoModeFM10 1:FIFO mode */ 1, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
#endif // OS24EVK-66 ISR Poll for 0x27 STATUS_REG2 bit 0x08 ZYXDA      
         //
         // In the stream mode, data from X, Y and Z measurement are stored into the FIFO. A
         // watermark interrupt can be enabled and set as in the FIFO mode.The FIFO continues filling
         // until it’s full (32 slots of data for X, Y and Z). When full, the FIFO discards the older data as
         // the new arrive.
         //~ lis2dh_configure_FIFO(/* fifoModeFM10 2:Stream mode */ 2, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
         //
         // In Stream-to_FIFO mode, data from X, Y and Z measurement are stored into the FIFO. A
         // watermark interrupt can be enabled (FIFO_WTMK_EN bit into FIFO_CTRL_REG) in order
         // to be raised when the FIFO is filled to the level specified into the FIFO_WTMK_LEVEL bits
         // of FIFO_CTRL_REG. The FIFO continues filling until it’s full (32 slots of 10 bit for for X, Y
         // and Z). When full, the FIFO discards the older data as the new arrive. Once trigger event
         // occurs, the FIFO starts operating in FIFO mode.
         //~ lis2dh_configure_FIFO(/* fifoModeFM10 3:Trigger mode */ 3, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
         //
         // TODO1: OS24EVK-66 Configure LIS2DH to issue FIFO watermark interrupt
         //
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_East();
         return; //  1; // success
      }
   }
   gLIS2DHdeviceAddress = 0; // LIS2DH not found
}
#endif // SUPPORT_LIS2DH
