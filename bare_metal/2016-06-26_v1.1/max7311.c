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
#if SUPPORT_MAX7311
uint8_t gMAX7311deviceAddress = 0x00; //!< (default 0x40) MAX7311 I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.
code uint8_t searchmax7311deviceAddressList[] = {
#if 0
   // short list of common addresses
    0x40, //!< 8-bit i2c address 0x40: AD2=GND, AD1=GND, AD0=GND
    0x4E, //!< 8-bit i2c address 0x4E: AD2=V+ , AD1=V+ , AD0=V+ 
    0xB0, //!< 8-bit i2c address 0xB0: AD2=SCL, AD1=SCL, AD0=SCL
    0xBE, //!< 8-bit i2c address 0xBE: AD2=SDA, AD1=SDA, AD0=SDA
#else
   // long list of all available MAX7311 addresses
   // may be too much table?
    0x20, //!< 8-bit i2c address 0x20: AD2=GND, AD1=SCL, AD0=GND
    0x22, //!< 8-bit i2c address 0x22: AD2=GND, AD1=SCL, AD0=V+ 
    0x24, //!< 8-bit i2c address 0x24: AD2=GND, AD1=SDA, AD0=GND
    0x26, //!< 8-bit i2c address 0x26: AD2=GND, AD1=SDA, AD0=V+ 
    0x28, //!< 8-bit i2c address 0x28: AD2=V+ , AD1=SCL, AD0=GND
    0x2A, //!< 8-bit i2c address 0x2A: AD2=V+ , AD1=SCL, AD0=V+ 
    0x2C, //!< 8-bit i2c address 0x2C: AD2=V+ , AD1=SDA, AD0=GND
    0x2E, //!< 8-bit i2c address 0x2E: AD2=V+ , AD1=SDA, AD0=V+ 
    
#if 0
   // address conflict with LIS2DH; remove from MAX7311 search
    0x30, //!< 8-bit i2c address 0x30: AD2=GND, AD1=SCL, AD0=SCL
    0x32, //!< 8-bit i2c address 0x32: AD2=GND, AD1=SCL, AD0=SDA
#endif
    0x34, //!< 8-bit i2c address 0x34: AD2=GND, AD1=SDA, AD0=SCL
    0x36, //!< 8-bit i2c address 0x36: AD2=GND, AD1=SDA, AD0=SDA
    0x38, //!< 8-bit i2c address 0x38: AD2=V+ , AD1=SCL, AD0=SCL
    0x3A, //!< 8-bit i2c address 0x3A: AD2=V+ , AD1=SCL, AD0=SDA
    0x3C, //!< 8-bit i2c address 0x3C: AD2=V+ , AD1=SDA, AD0=SCL
    0x3E, //!< 8-bit i2c address 0x3E: AD2=V+ , AD1=SDA, AD0=SDA
    
    0x40, //!< 8-bit i2c address 0x40: AD2=GND, AD1=GND, AD0=GND (all GND)
    0x42, //!< 8-bit i2c address 0x42: AD2=GND, AD1=GND, AD0=V+ 
    0x44, //!< 8-bit i2c address 0x44: AD2=GND, AD1=V+ , AD0=GND
    0x46, //!< 8-bit i2c address 0x46: AD2=GND, AD1=V+ , AD0=V+ 
    0x48, //!< 8-bit i2c address 0x48: AD2=V+ , AD1=GND, AD0=GND
    0x4A, //!< 8-bit i2c address 0x4A: AD2=V+ , AD1=GND, AD0=V+ 
    0x4C, //!< 8-bit i2c address 0x4C: AD2=V+ , AD1=V+ , AD0=GND
    0x4E, //!< 8-bit i2c address 0x4E: AD2=V+ , AD1=V+ , AD0=V+  (all V+)
    
    0x50, //!< 8-bit i2c address 0x50: AD2=GND, AD1=GND, AD0=SCL
    0x52, //!< 8-bit i2c address 0x52: AD2=GND, AD1=GND, AD0=SDA
    0x54, //!< 8-bit i2c address 0x54: AD2=GND, AD1=V+ , AD0=SCL
    0x56, //!< 8-bit i2c address 0x56: AD2=GND, AD1=V+ , AD0=SDA
    0x58, //!< 8-bit i2c address 0x58: AD2=V+ , AD1=GND, AD0=SCL
    0x5A, //!< 8-bit i2c address 0x5A: AD2=V+ , AD1=GND, AD0=SDA
    0x5C, //!< 8-bit i2c address 0x5C: AD2=V+ , AD1=V+ , AD0=SCL
    0x5E, //!< 8-bit i2c address 0x5E: AD2=V+ , AD1=V+ , AD0=SDA
    
    0xA0, //!< 8-bit i2c address 0xA0: AD2=SCL, AD1=SCL, AD0=GND
    0xA2, //!< 8-bit i2c address 0xA2: AD2=SCL, AD1=SCL, AD0=V+ 
    0xA4, //!< 8-bit i2c address 0xA4: AD2=SCL, AD1=SDA, AD0=GND
    0xA6, //!< 8-bit i2c address 0xA6: AD2=SCL, AD1=SDA, AD0=V+ 
    0xA8, //!< 8-bit i2c address 0xA8: AD2=SDA, AD1=SCL, AD0=GND
    0xAA, //!< 8-bit i2c address 0xAA: AD2=SDA, AD1=SCL, AD0=V+ 
    0xAC, //!< 8-bit i2c address 0xAC: AD2=SDA, AD1=SDA, AD0=GND
    0xAE, //!< 8-bit i2c address 0xAE: AD2=SDA, AD1=SDA, AD0=V+ 
    
    0xB0, //!< 8-bit i2c address 0xB0: AD2=SCL, AD1=SCL, AD0=SCL (all SCL)
    0xB2, //!< 8-bit i2c address 0xB2: AD2=SCL, AD1=SCL, AD0=SDA
    0xB4, //!< 8-bit i2c address 0xB4: AD2=SCL, AD1=SDA, AD0=SCL
    0xB6, //!< 8-bit i2c address 0xB6: AD2=SCL, AD1=SDA, AD0=SDA
    0xB8, //!< 8-bit i2c address 0xB8: AD2=SDA, AD1=SCL, AD0=SCL
    0xBA, //!< 8-bit i2c address 0xBA: AD2=SDA, AD1=SCL, AD0=SDA
    0xBC, //!< 8-bit i2c address 0xBC: AD2=SDA, AD1=SDA, AD0=SCL
    0xBE, //!< 8-bit i2c address 0xBE: AD2=SDA, AD1=SDA, AD0=SDA (all SDA)
    
    0xC0, //!< 8-bit i2c address 0xC0: AD2=SCL, AD1=GND, AD0=GND
    0xC2, //!< 8-bit i2c address 0xC2: AD2=SCL, AD1=GND, AD0=V+ 
    0xC4, //!< 8-bit i2c address 0xC4: AD2=SCL, AD1=V+ , AD0=GND
    0xC6, //!< 8-bit i2c address 0xC6: AD2=SCL, AD1=V+ , AD0=V+ 
    0xC8, //!< 8-bit i2c address 0xC8: AD2=SDA, AD1=GND, AD0=GND
    0xCA, //!< 8-bit i2c address 0xCA: AD2=SDA, AD1=GND, AD0=V+ 
    0xCC, //!< 8-bit i2c address 0xCC: AD2=SDA, AD1=V+ , AD0=GND
    0xCE, //!< 8-bit i2c address 0xCE: AD2=SDA, AD1=V+ , AD0=V+ 
    
    0xD0, //!< 8-bit i2c address 0xD0: AD2=SCL, AD1=GND, AD0=SCL
    0xD2, //!< 8-bit i2c address 0xD2: AD2=SCL, AD1=GND, AD0=SDA
    0xD4, //!< 8-bit i2c address 0xD4: AD2=SCL, AD1=V+ , AD0=SCL
    0xD6, //!< 8-bit i2c address 0xD6: AD2=SCL, AD1=V+ , AD0=SDA
    0xD8, //!< 8-bit i2c address 0xD8: AD2=SDA, AD1=GND, AD0=SCL
    0xDA, //!< 8-bit i2c address 0xDA: AD2=SDA, AD1=GND, AD0=SDA
    0xDC, //!< 8-bit i2c address 0xDC: AD2=SDA, AD1=V+ , AD0=SCL
    0xDE, //!< 8-bit i2c address 0xDE: AD2=SDA, AD1=V+ , AD0=SDA
#endif
    0 //<! end of list
}; //!< MAX7311 I2C device address (slave address on I2C bus), 8-bits, LEFT-justified.
#endif // SUPPORT_MAX7311


#if SUPPORT_MAX7311
// OS24EVK-66 MAX7311 Register Write
int8_t max7311_RegRead(uint8_t regAddress, uint8_t *pRegValue)
{
   return i2c_smbus_read_byte_data(gMAX7311deviceAddress, regAddress, pRegValue);
}
#endif // SUPPORT_MAX7311

#if SUPPORT_MAX7311
// OS24EVK-66 MAX7311 Register Write
int8_t max7311_RegWrite(uint8_t regAddress, uint8_t regValue)
{
   return i2c_smbus_write_byte_data(gMAX7311deviceAddress, regAddress, regValue);
}
#endif // SUPPORT_MAX7311

#if SUPPORT_MAX7311
void max7311_open()
{
   // OS24EVK-66 Init: max7311_open() Search max7311 device address 0x30, 0x32 gmax7311deviceAddress
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
   // // example: max7311_RegRead NACK status
   // gmax7311deviceAddress = 0x44; // MAX30101 0xAE
   //    DeviceID_regAddr = 0xFF; // MAX30101 register DeviceId (constant 0x15) Device identification register.
   //    DeviceID_expect = 0x15;
   //    DeviceID_actual = max7311_RegRead(DeviceID_regAddr, 0); // MAX30101
   //
   // // example: max7311_RegRead NACK status
   // gmax7311deviceAddress = 0xAE; // MAX30101 0xAE
   //    DeviceID_regAddr = 0xFF; // MAX30101 register DeviceId (constant 0x15) Device identification register.
   //    DeviceID_expect = 0x15;
   //    DeviceID_actual = max7311_RegRead(DeviceID_regAddr, 0); // MAX30101
   //
   // // example: max7311_RegRead NACK status
   // gmax7311deviceAddress = 0x32; // max7311
   //    DeviceID_regAddr = 0x0F; // max7311 register WHO_AM_I (constant 0x33) Device identification register.
   //    DeviceID_expect = 0x33;
   //    DeviceID_actual = max7311_RegRead(DeviceID_regAddr, 0); // MAX30101
   //
   // Search the i2c address list for possible devices
   for (i2cAddressIndex = 0; searchmax7311deviceAddressList[i2cAddressIndex] != 0; i2cAddressIndex++)
   {
      gMAX7311deviceAddress = searchmax7311deviceAddressList[i2cAddressIndex];
      //
      //~ blockArr[5] = DeviceID_regAddr;
      //~ i2c_read(gmax7311deviceAddress & 0xFE, blockArr, 1, 1);	
      //	blockArr[2+0] = value read from Device Register
      //
      DeviceID_regAddr = max7311_Timeout;
      DeviceID_expect = 0x01;
      // DeviceID_actual = max7311_RegRead(DeviceID_regAddr, 0);
      if (max7311_RegRead(DeviceID_regAddr, &DeviceID_actual) < 0) {
         // typically for NACK, errno=5 and return value = -1
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_East();
         continue;
      } else if ((DeviceID_actual & 0xFF) != (DeviceID_expect & 0xFF)) {
         //~ printf("\n""max7311_open(): i2c address 0x%2.2X (8-bit 0x%2.2X) ACK"
         //~ "\n""                but reg 0x%2.2X %s=0x%2.2X not 0x%2.2X so wrong device",
         //~ global_max7311_device_data.i2cDeviceAddress7bits,
         //~ global_max7311_device_data.i2cDeviceAddress7bits * 2,
         //~ (DeviceID_regAddr & 0xFF),
         //~ max7311_RegisterAddressName((uint8_t)DeviceID_regAddr, 0, 0),
         //~ (DeviceID_actual & 0xFF),
         //~ (DeviceID_expect & 0xFF)
         //~ );
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_East();
         continue;
      } else {
         //~ printf("\n""max7311_open(): i2c address 0x%2.2X (8-bit 0x%2.2X) ACK; %s=0x%2.2X OK",
         //~ global_max7311_device_data.i2cDeviceAddress7bits,
         //~ global_max7311_device_data.i2cDeviceAddress7bits * 2,
         //~ max7311_RegisterAddressName((uint8_t)DeviceID_regAddr, 0, 0),
         //~ (DeviceID_actual & 0xFF)
         //~ );
         //
         // VERIFY: OS24EVK-66 Init: if max7311 present, initialize 
         max7311_RegWrite(max7311_Port0700ConfigInput, 0x00); // bitmap where 1=input pin, 0=output pin
         max7311_RegWrite(max7311_Port1508ConfigInput, 0x00); // bitmap where 1=input pin, 0=output pin
         max7311_RegWrite(max7311_Port0700Output,      0x00); // bitmap of output pin drivers
         max7311_RegWrite(max7311_Port1508Output,      0x00); // bitmap of output pin drivers
         max7311_RegWrite(max7311_Port0700Invert,      0x00); // bitmap where 1=inverted (active-low) signal; affects Input Port and Output Port
         max7311_RegWrite(max7311_Port1508Invert,      0x00); // bitmap where 1=inverted (active-low) signal; affects Input Port and Output Port
         //~ max7311_configure_outputDataRateHz(100);
         //~ max7311_configure_resolutionBits(12);
         //~ max7311_configure_selfTest(0);
         //
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_Blink();
         //~ flash_Led1Led2_East();
         return; //  1; // success
      }
   }
   gMAX7311deviceAddress = 0; // max7311 not found
}
#endif // SUPPORT_MAX7311

