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
#if SUPPORT_MAX30100
#include "max30100.h"
#endif // SUPPORT_MAX30100


#if SUPPORT_MAX30100
uint8_t gMAX30100deviceAddress = 0x00; //!< (default 0xAE) = MAX30100 I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.
code uint8_t searchMAX30100deviceAddressList[] = {
   0xAE, //!< MAX30100 I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.
   0 //<! end of list
}; //!< MAX30100 I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.
#else // SUPPORT_MAX30100
// Omit the "else" clause from MAX30101EVKIT firmware
#endif // SUPPORT_MAX30100


// OS24EVK-66 MAX30100 Present?
// @post gMAX30100deviceAddress = 0 if MAX30100 not connected,
//       gMAX30100deviceAddress = MAX30100 device address if present
#if SUPPORT_MAX30100
void max30100_open()
{
   // OS24EVK-66 Init: max30100_open() Search MAX30100 device address 0x30, 0x32 gMAX30100deviceAddress
   //
   int8_t i2cAddressIndex;
   int8_t DeviceID_regAddr;
   int8_t DeviceID_expect;
   int8_t DeviceID_actual;
   //
   // C not C++, so all local variables must be declared before any statements
   //
   // loop through list of deviceAddress to search
   // static uint8_t searchMAX30100deviceAddressList[] = {0xAE, 0};
   //
   // // example: max30100_RegRead NACK status
   // gMAX30100deviceAddress = 0x44; // MAX30100 0xAE
   //    DeviceID_regAddr = 0xFF; // MAX30100 register DeviceId (constant 0x15) Device identification register.
   //    DeviceID_expect = 0x15;
   //    DeviceID_actual = max30100_RegRead(DeviceID_regAddr, 0); // MAX30100
   //
   // // example: max30100_RegRead NACK status
   // gMAX30100deviceAddress = 0xAE; // MAX30100 0xAE
   //    DeviceID_regAddr = 0xFF; // MAX30100 register DeviceId (constant 0x15) Device identification register.
   //    DeviceID_expect = 0x15;
   //    DeviceID_actual = max30100_RegRead(DeviceID_regAddr, 0); // MAX30100
   //
   // // example: max30100_RegRead NACK status
   // gMAX30100deviceAddress = 0x32; // MAX30100
   //    DeviceID_regAddr = 0x0F; // MAX30100 register WHO_AM_I (constant 0x33) Device identification register.
   //    DeviceID_expect = 0x33;
   //    DeviceID_actual = max30100_RegRead(DeviceID_regAddr, 0); // MAX30100
   //
   // Search the i2c address list for possible devices
   for (i2cAddressIndex = 0; searchMAX30100deviceAddressList[i2cAddressIndex] != 0; i2cAddressIndex++)
   {
      gMAX30100deviceAddress = searchMAX30100deviceAddressList[i2cAddressIndex];
      //
      //~ blockArr[5] = DeviceID_regAddr;
      //~ i2c_read(gMAX30100deviceAddress & 0xFE, blockArr, 1, 1);	
      //	blockArr[2+0] = value read from Device Register
      //
      DeviceID_regAddr = MAX30100_DeviceID; // (constant 0x11) Device identification register.
      DeviceID_expect = 0x11;
      // DeviceID_actual = max30100_RegRead(DeviceID_regAddr, 0);
      if (max30100_RegRead(DeviceID_regAddr, &DeviceID_actual) < 0) {
         // typically for NACK, errno=5 and return value = -1
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_East();
         continue;
      } else if ((DeviceID_actual & 0xFF) != (DeviceID_expect & 0xFF)) {
         //~ printf("\n""max30100_open(): i2c address 0x%2.2X (8-bit 0x%2.2X) ACK"
         //~ "\n""                but reg 0x%2.2X %s=0x%2.2X not 0x%2.2X so wrong device",
         //~ global_max30100_device_data.i2cDeviceAddress7bits,
         //~ global_max30100_device_data.i2cDeviceAddress7bits * 2,
         //~ (DeviceID_regAddr & 0xFF),
         //~ max30100_RegisterAddressName((uint8_t)DeviceID_regAddr, 0, 0),
         //~ (DeviceID_actual & 0xFF),
         //~ (DeviceID_expect & 0xFF)
         //~ );
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_East();
         continue;
      } else {
         //~ printf("\n""max30100_open(): i2c address 0x%2.2X (8-bit 0x%2.2X) ACK; %s=0x%2.2X OK",
         //~ global_max30100_device_data.i2cDeviceAddress7bits,
         //~ global_max30100_device_data.i2cDeviceAddress7bits * 2,
         //~ max30100_RegisterAddressName((uint8_t)DeviceID_regAddr, 0, 0),
         //~ (DeviceID_actual & 0xFF)
         //~ );
         //
         // TODO: OS24EVK-66 Init: if MAX30100 present, initialize 
         // TODO: OS24EVK-66 Init: if MAX30100 present, initialize INT1 Active-Low by writing 0x25 CTRL_REG6 bit 0x02 H_LACTIVE=1
         //max30100_RegWrite(max30100_CTRL_REG6, 0x02);
         //max30100_configure_outputDataRateHz(100);
         //max30100_configure_resolutionBits(12);
         //~ max30100_configure_selfTest(0);
         // TODO: OS24EVK-66 Init: if MAX30100 present, initialize FIFO overflow (watermark)
         //
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_East();
         return; //  1; // success
      }
   }
   gMAX30100deviceAddress = 0; // MAX30100 not found
#if 0
   // TODO1: OS24EVK-66 if MAX30100 absent but (gMockHIDFIFOChannels > 0), still send HID report 1
   // This still isn't meaningful; streaming start fails because of other device regster writes.
   gMockHIDFIFOChannels = 3;
#endif
}
#endif // SUPPORT_MAX30100

// OS24EVK-66 MAX30100 Register Write
#if SUPPORT_MAX30100
int8_t max30100_RegRead(uint8_t regAddress, uint8_t *pRegValue)
{
   return i2c_smbus_read_byte_data(gMAX30100deviceAddress, regAddress, pRegValue);
}
#endif // SUPPORT_MAX30100

#if SUPPORT_MAX30100
// OS24EVK-66 MAX30100 Register Write
int8_t max30100_RegWrite(uint8_t regAddress, uint8_t regValue)
{
   return i2c_smbus_write_byte_data(gMAX30100deviceAddress, regAddress, regValue);
}
#endif // SUPPORT_MAX30100

