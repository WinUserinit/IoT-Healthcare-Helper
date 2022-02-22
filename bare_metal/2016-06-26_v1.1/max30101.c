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

#if 1 // TODO: lis2dh_configure_FIFO(LIS2DH_DEFAULT_FIFOMODE) when max30101_RegWrite(RegAddressEnum.ModeConfiguration,...)
#if SUPPORT_LIS2DH
#include "lis2dh.h"
#endif // SUPPORT_LIS2DH
#endif // TODO: lis2dh_configure_FIFO(LIS2DH_DEFAULT_FIFOMODE) when max30101_RegWrite(RegAddressEnum.ModeConfiguration,...)

#if SUPPORT_MAX30101
uint8_t gMAX30101deviceAddress = 0x00; //!< (default 0xAE) = MAX30101 I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.
code uint8_t searchMAX30101deviceAddressList[] = {
   0xAE, //!< MAX30101 I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.
   0 //<! end of list
}; //!< MAX30101 I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.

// TODO: 2015-05-02 cache value of MAX30101_ModeConfiguration in gMAX30101_ModeConfiguration_value
uint8_t gMAX30101_ModeConfiguration_value = 0;

#else // SUPPORT_MAX30101
// regression test: gI2CreadBurstSlaveAddress was the old name for gMAX30101deviceAddresss
// between gOffset and blockArr
uint8_t gMAX30101deviceAddress = 0xAE; //!< (default 0xAE) = MAX30101 I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.
#endif // SUPPORT_MAX30101

// OS24EVK-66 MAX30101 Present?
// @post gMAX30101deviceAddress = 0 if MAX30101 not connected,
//       gMAX30101deviceAddress = MAX30101 device address if present
#if SUPPORT_MAX30101
void max30101_open()
{
   // OS24EVK-66 Init: max30101_open() Search MAX30101 device address 0x30, 0x32 gMAX30101deviceAddress
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
   // // example: max30101_RegRead NACK status
   // gMAX30101deviceAddress = 0x44; // MAX30101 0xAE
   //    DeviceID_regAddr = 0xFF; // MAX30101 register DeviceId (constant 0x15) Device identification register.
   //    DeviceID_expect = 0x15;
   //    DeviceID_actual = max30101_RegRead(DeviceID_regAddr, 0); // MAX30101
   //
   // // example: max30101_RegRead NACK status
   // gMAX30101deviceAddress = 0xAE; // MAX30101 0xAE
   //    DeviceID_regAddr = 0xFF; // MAX30101 register DeviceId (constant 0x15) Device identification register.
   //    DeviceID_expect = 0x15;
   //    DeviceID_actual = max30101_RegRead(DeviceID_regAddr, 0); // MAX30101
   //
   // // example: max30101_RegRead NACK status
   // gMAX30101deviceAddress = 0x32; // MAX30101
   //    DeviceID_regAddr = 0x0F; // MAX30101 register WHO_AM_I (constant 0x33) Device identification register.
   //    DeviceID_expect = 0x33;
   //    DeviceID_actual = max30101_RegRead(DeviceID_regAddr, 0); // MAX30101
   //
   // Search the i2c address list for possible devices
   for (i2cAddressIndex = 0; searchMAX30101deviceAddressList[i2cAddressIndex] != 0; i2cAddressIndex++)
   {
      gMAX30101deviceAddress = searchMAX30101deviceAddressList[i2cAddressIndex];
      //
      //~ blockArr[5] = DeviceID_regAddr;
      //~ i2c_read(gMAX30101deviceAddress & 0xFE, blockArr, 1, 1);	
      //	blockArr[2+0] = value read from Device Register
      //
      DeviceID_regAddr = MAX30101_DeviceID; // (constant 0x15) Device identification register.
      DeviceID_expect = 0x15;
      // DeviceID_actual = max30101_RegRead(DeviceID_regAddr, 0);
      if (max30101_RegRead(DeviceID_regAddr, &DeviceID_actual) < 0) {
         // typically for NACK, errno=5 and return value = -1
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_East();
         continue;
      } else if ((DeviceID_actual & 0xFF) != (DeviceID_expect & 0xFF)) {
         //~ printf("\n""max30101_open(): i2c address 0x%2.2X (8-bit 0x%2.2X) ACK"
         //~ "\n""                but reg 0x%2.2X %s=0x%2.2X not 0x%2.2X so wrong device",
         //~ global_max30101_device_data.i2cDeviceAddress7bits,
         //~ global_max30101_device_data.i2cDeviceAddress7bits * 2,
         //~ (DeviceID_regAddr & 0xFF),
         //~ max30101_RegisterAddressName((uint8_t)DeviceID_regAddr, 0, 0),
         //~ (DeviceID_actual & 0xFF),
         //~ (DeviceID_expect & 0xFF)
         //~ );
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_East();
         continue;
      } else {
         //~ printf("\n""max30101_open(): i2c address 0x%2.2X (8-bit 0x%2.2X) ACK; %s=0x%2.2X OK",
         //~ global_max30101_device_data.i2cDeviceAddress7bits,
         //~ global_max30101_device_data.i2cDeviceAddress7bits * 2,
         //~ max30101_RegisterAddressName((uint8_t)DeviceID_regAddr, 0, 0),
         //~ (DeviceID_actual & 0xFF)
         //~ );
         //
         // TODO: OS24EVK-66 Init: if MAX30101 present, initialize 
         // TODO: OS24EVK-66 Init: if MAX30101 present, initialize INT1 Active-Low by writing 0x25 CTRL_REG6 bit 0x02 H_LACTIVE=1
         //max30101_RegWrite(max30101_CTRL_REG6, 0x02);
         //max30101_configure_outputDataRateHz(100);
         //max30101_configure_resolutionBits(12);
         //~ max30101_configure_selfTest(0);
         // TODO: OS24EVK-66 Init: if MAX30101 present, initialize FIFO overflow (watermark)
         //
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_East();
         return; //  1; // success
      }
   }
   gMAX30101deviceAddress = 0; // MAX30101 not found
#if 0
   // TODO1: OS24EVK-66 if MAX30101 absent but (gMockHIDFIFOChannels > 0), still send HID report 1
   // This still isn't meaningful; streaming start fails because of other device regster writes.
   gMockHIDFIFOChannels = 3;
#endif
}
#endif // SUPPORT_MAX30101

// OS24EVK-66 MAX30101 Register Write
#if SUPPORT_MAX30101
int8_t max30101_RegRead(uint8_t regAddress, uint8_t *pRegValue)
{
   return i2c_smbus_read_byte_data(gMAX30101deviceAddress, regAddress, pRegValue);
}
#endif // SUPPORT_MAX30101

#if SUPPORT_MAX30101
// OS24EVK-66 MAX30101 Register Write
int8_t max30101_RegWrite(uint8_t regAddress, uint8_t regValue)
{
   // TODO: 2015-05-02 cache value of MAX30101_ModeConfiguration in gMAX30101_ModeConfiguration_value
   if (regAddress == MAX30101_ModeConfiguration) {
      gMAX30101_ModeConfiguration_value = regValue;
#if 0 // TODO: lis2dh_configure_FIFO(LIS2DH_DEFAULT_FIFOMODE) when max30101_RegWrite(RegAddressEnum.ModeConfiguration,...)
# ifdef LIS2DH_DEFAULT_FIFOMODE
      lis2dh_configure_FIFO(/* fifoModeFM10 1:FIFO mode */ LIS2DH_DEFAULT_FIFOMODE, /* fifoTR */ 0, LIS2DH_DEFAULT_FIFOWATERMARKLEVEL);
# endif
#endif // TODO: lis2dh_configure_FIFO(LIS2DH_DEFAULT_FIFOMODE) when max30101_RegWrite(RegAddressEnum.ModeConfiguration,...)
   }
   return i2c_smbus_write_byte_data(gMAX30101deviceAddress, regAddress, regValue);
}
#endif // SUPPORT_MAX30101

