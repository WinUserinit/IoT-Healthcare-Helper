//-----------------------------------------------------------------------------
// F3xx_USB0_Descriptor.c
//-----------------------------------------------------------------------------
// Copyright 2014
//
// Program Description:
//
// Source file for USB HID firmware. Includes descriptor data.
//
//
// FID:            3XX000004
// Target:         C8051F32x/C8051F340
// Tool chain:     Keil C51 7.50 / Keil EVAL C51
//                 Silicon Laboratories IDE version 2.6
// Project Name:   Generic HID Firmware
//
//
// Release 1.0
//    -140401 Initial Revision (Tushar Mazumder)
//
// Update PID and product string in this file.

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

// Project-level preprocessor #defines effective across all *.c files
#include "ProjectDefines.h"

#include "F3xx_USB0_Register.h"
#include "F3xx_USB0_InterruptServiceRoutine.h"
#include "F3xx_USB0_Descriptor.h"
#include "F3xx_Initialize.h"
//-----------------------------------------------------------------------------
// Descriptor Declarations
//-----------------------------------------------------------------------------

// LSB first

code const device_descriptor DEVICEDESC =
{
   18,                                 // bLength must be 18 bytes for DeviceDescriptor DSC_DEVICE
   0x01,                               // bDescriptorType   1: DeviceDescriptor DSC_DEVICE
   0x1001,                             // bcdUSB				USB 1.1 (01.10)
   0x00,                               // bDeviceClass
   0x00,                               // bDeviceSubClass
   0x00,                               // bDeviceProtocol
   EP0_PACKET_SIZE,                    // bMaxPacketSize0
// VERIFY: OS24EVK-66 use SWAP_HIGH_AND_LOW_BYTE(data16) macro to convert big-endian to little-endian
#if SUPPORT_MAX30100
   SWAP_HIGH_AND_LOW_BYTE(0x0B6A),     // idVendor				0x0B6A: Maxim Integrated
   SWAP_HIGH_AND_LOW_BYTE(0x1364),     // idProduct				0x1364: MAX30100
#elif SUPPORT_MAX30101
   SWAP_HIGH_AND_LOW_BYTE(0x0B6A),     // idVendor				0x0B6A: Maxim Integrated
   SWAP_HIGH_AND_LOW_BYTE(0x1365),     // idProduct				0x1365: MAX30101
#else
   // default USB VID/PID
   SWAP_HIGH_AND_LOW_BYTE(0x0B6A),     // idVendor				0x0B6A: Maxim Integrated
   SWAP_HIGH_AND_LOW_BYTE(0x1365),     // idProduct				0x1364: MAX30100
#endif // SUPPORT_MAX30100
   0x0000,                             // bcdDevice         Device release number in binary-coded decimal
   0x01,                               // iManufacturer     String1Desc, // Manufacturer String
   0x02,                               // iProduct          String2Desc  // Product ID String
   0x00,                               // iSerialNumber     String3Desc  // Serial Number String (optional)
   0x01                                // bNumConfigurations   Number of possible configurations
}; //end of DEVICEDESC

// From "USB Device Class Definition for Human Interface Devices (HID)".
// Section 7.1:
// "When a Get_Descriptor(Configuration) request is issued,
// it returns the Configuration descriptor, all Interface descriptors,
// all Endpoint descriptors, and the HID descriptor for each interface."
code const hid_configuration_descriptor HIDCONFIGDESC =
{

{ // configuration_descriptor hid_configuration_descriptor
   0x09,                               // bLength must be 9 bytes for ConfigurationDescriptor DSC_CONFIG
   0x02,                               // bDescriptorType   2: ConfigurationDescriptor DSC_CONFIG
// VERIFY: OS24EVK-66 use SWAP_HIGH_AND_LOW_BYTE(data16) macro to convert big-endian to little-endian
# if 1
//#  if USB_SUPPORT_EP1OUT
   SWAP_HIGH_AND_LOW_BYTE( 9 + 9 + 9 + 7 + 7 ),     // Totallength (= 9+9+9+7+7)	0x0029; this is the sum of the length of all descriptors except the hid_report_descriptor
//#  else // USB_SUPPORT_EP1OUT
//   SWAP_HIGH_AND_LOW_BYTE( 9 + 9 + 9 + 7     ),     // Totallength (= 9+9+9+7+7)	0x0029; this is the sum of the length of all descriptors except the hid_report_descriptor
//#  endif // USB_SUPPORT_EP1OUT
# else
   0x2900,                             // Totallength (= 9+9+9+7+7)	0x0029; this is the sum of the length of all descriptors except the hid_report_descriptor
# endif
   0x01,                               // NumInterfaces
   0x01,                               // bConfigurationValue
   0x00,                               // iConfiguration       String4Desc  // Configuration Name String (optional)
   0x80,                               // bmAttributes 0x80:Reserved1 0x40:SelfPowered 0x20:RemoteWakeup 0x1F:Reserved00000
   0x20                                // MaxPower (in 2mA units)
},

{ // interface_descriptor hid_interface_descriptor
   0x09,                               // bLength must be 9 bytes for InterfaceDescriptor DSC_INTERFACE
   0x04,                               // bDescriptorType   4: InterfaceDescriptor DSC_INTERFACE
   0x00,                               // bInterfaceNumber
   0x00,                               // bAlternateSetting
//# if USB_SUPPORT_EP1OUT
   0x02,                               // bNumEndpoints (not including EP0)
//# else // USB_SUPPORT_EP1OUT
//   0x01,                               // bNumEndpoints (not including EP0)
//# endif // USB_SUPPORT_EP1OUT
   0x03,                               // bInterfaceClass (3 = HID)
   0x00,                               // bInterfaceSubClass
   0x00,                               // bInterfaceProcotol
   0x00                                // iInterface           String5Desc  // Interface Name String (optional)
},

{ // class_descriptor hid_descriptor
   0x09,	                           // bLength
   0x21,	                           // bDescriptorType   0x21: HIDDescriptor DSC_HID
   0x0101,	                           // bcdHID				1.01
   0x00,	                           // bCountryCode
   0x01,	                           // bNumDescriptors
   0x22,                               // bDescriptorType   0x22: HIDReportDescriptor DSC_HID_REPORT
# if 1
// VERIFY: OS24EVK-66 use SWAP_HIGH_AND_LOW_BYTE(data16) macro to convert big-endian to little-endian
   SWAP_HIGH_AND_LOW_BYTE(HID_REPORT_DESCRIPTOR_SIZE)       // wItemLength (tot. len. of hid_report_descriptor; this includes the identifier byte - THIS IS DEPENDENT ON THE NUMBER OF REPORTS!)
# else
   HID_REPORT_DESCRIPTOR_SIZE_LE       // wItemLength (tot. len. of hid_report_descriptor; this includes the identifier byte - THIS IS DEPENDENT ON THE NUMBER OF REPORTS!)
# endif
},

// IN endpoint (mandatory for HID)
{ // endpoint_descriptor hid_endpoint_in_descriptor
   0x07,                               // bLength must be 7 bytes for EndPointDescriptor DSC_ENDPOINT
   0x05,                               // bDescriptorType   5: EndPointDescriptor DSC_ENDPOINT
#if USB_writeReadHID_EP1OUT_EP1IN
   // VERIFY: OS24EVK-66 can we use EP2IN or EP3IN instead of EP1IN INTERRUPT endpoint?
   0x81,                               // bEndpointAddress 0x81:EP1IN 0x01:EP1OUT 0x82:EP2IN 0x83:EP3IN
#elif USB_writeReadHID_EP2OUT_EP2IN
   0x82,                               // bEndpointAddress 0x81:EP1IN 0x01:EP1OUT 0x82:EP2IN 0x83:EP3IN
#elif USB_writeReadHID_EP3OUT_EP3IN
   0x83,                               // bEndpointAddress 0x81:EP1IN 0x01:EP1OUT 0x82:EP2IN 0x83:EP3IN
#endif // USB_writeReadHID_EP1OUT_EP1IN
   0x03,                               // bmAttributes 0:Control 1:Isochronous 2:Bulk 3:Interrupt
// VERIFY: OS24EVK-66 use SWAP_HIGH_AND_LOW_BYTE(data16) macro to convert big-endian to little-endian
//# if 1
   SWAP_HIGH_AND_LOW_BYTE(EP1_PACKET_SIZE),                 // MaxPacketSize (LITTLE ENDIAN)
//# else
//   EP1_PACKET_SIZE_LE,                 // MaxPacketSize (LITTLE ENDIAN)
//# endif
   1                                   // bInterval (Interrupt polling interval in milliseconds)
},

//#if USB_SUPPORT_EP1OUT
// OUT endpoint (optional for HID)
{ // endpoint_descriptor hid_endpoint_out_descriptor
   0x07,                               // bLength must be 7 bytes for EndPointDescriptor DSC_ENDPOINT
   0x05,                               // bDescriptorType   5: EndPointDescriptor  DSC_ENDPOINT
#if USB_writeReadHID_EP1OUT_EP1IN
   // VERIFY: OS24EVK-66 can we use EP2IN or EP3IN instead of EP1IN INTERRUPT endpoint?
   0x01,                               // bEndpointAddress 0x81:EP1IN 0x01:EP1OUT 0x02:EP2OUT 0x03:EP3OUT
#elif USB_writeReadHID_EP2OUT_EP2IN
   0x02,                               // bEndpointAddress 0x81:EP1IN 0x01:EP1OUT 0x02:EP2OUT 0x03:EP3OUT
#elif USB_writeReadHID_EP3OUT_EP3IN
   0x03,                               // bEndpointAddress 0x81:EP1IN 0x01:EP1OUT 0x02:EP2OUT 0x03:EP3OUT
#endif // USB_writeReadHID_EP1OUT_EP1IN
   0x03,                               // bmAttributes 0:Control 1:Isochronous 2:Bulk 3:Interrupt
// VERIFY: OS24EVK-66 use SWAP_HIGH_AND_LOW_BYTE(data16) macro to convert big-endian to little-endian
//# if 1
   SWAP_HIGH_AND_LOW_BYTE(EP1_PACKET_SIZE),                 // MaxPacketSize (LITTLE ENDIAN)
//# else
//   EP1_PACKET_SIZE_LE,                 // MaxPacketSize (LITTLE ENDIAN)
//# endif
   1                                   // bInterval (Interrupt polling interval in milliseconds)
}
//#endif // USB_SUPPORT_EP1OUT

};

// VERIFY: OS24EVK-66 update *F3xx_USB0_Descriptor.c* hid_report_descriptor to define HID REPORT_ID(2) format
// see http://stackoverflow.com/questions/21606991/custom-hid-device-hid-report-descriptor
code const hid_report_descriptor HIDREPORTDESC =
{
    0x06, 0x00, 0xff,                  // (GLOBAL) USAGE_PAGE (Vendor Defined Page 1) 0xFF00 Vendor-defined 
    0x09, 0x01,                        //   (LOCAL)  USAGE (Vendor Usage 1)
    0xa1, 0x01,                        // (MAIN)COLLECTION (Application)
	// 7 bytes

   
#ifdef EXPLICIT_REPORT_ID
    0x85, HID_REPORT_ID_1,    // 0x01  // (GLOBAL) REPORT_ID(1)						1000 01 nn
#endif
    0x95, SHORT_REPORT_COUNT, // 0x3F  // (GLOBAL) REPORT_COUNT(SHORT_REPORT_COUNT)	1001 01 nn
    0x75, 0x08,                        //   (GLOBAL) REPORT_SIZE (8) in bits	0111 01 nn
    0x09, 0x01,                        //   (LOCAL)  USAGE (Vendor Usage 1)
// TODO: OS24EVK-66 HID_Descriptor_Tool Error: LOGICAL MIN MUST be defined 
// TODO: OS24EVK-66 HID_Descriptor_Tool Error: LOGICAL MAX MUST be defined 
//#if USB_SUPPORT_EP1OUT
    0x91, 0x02,                        //   (MAIN)   OUTPUT (Data,Var,Abs)
//#endif // USB_SUPPORT_EP1OUT
    0x75, 0x08,                        //   (GLOBAL) REPORT_SIZE (8)
    0x09, 0x01,                        //   (LOCAL)  USAGE (Vendor Usage 1)
// TODO: OS24EVK-66 HID_Descriptor_Tool Error: LOGICAL MIN MUST be defined 
// TODO: OS24EVK-66 HID_Descriptor_Tool Error: LOGICAL MAX MUST be defined 
    0x81, 0x02,                        //   (MAIN)   INPUT (Data,Var,Abs)

#if SUPPORT_LIS2DH
   // VERIFY: OS24EVK-66 update *F3xx_USB0_Descriptor.c* hid_report_descriptor to define HID REPORT_ID(2) format
    0x85, HID_REPORT_ID_2,    // 0x02  // (GLOBAL) REPORT_ID(2)						1000 01 nn
    0x95, SHORT_REPORT_COUNT, // 0x3F  // (GLOBAL) REPORT_COUNT(SHORT_REPORT_COUNT)	1001 01 nn
    0x75, 0x08,                        //   (GLOBAL) REPORT_SIZE (8) in bits	0111 01 nn
    0x09, 0x01,                        //   (LOCAL)  USAGE (Vendor Usage 1)
// TODO: OS24EVK-66 HID_Descriptor_Tool Error: LOGICAL MIN MUST be defined 
// TODO: OS24EVK-66 HID_Descriptor_Tool Error: LOGICAL MAX MUST be defined 
//#if USB_SUPPORT_EP1OUT
    0x91, 0x02,                        //   (MAIN)   OUTPUT (Data,Var,Abs)
//#endif // USB_SUPPORT_EP1OUT
    0x75, 0x08,                        //   (GLOBAL) REPORT_SIZE (8)
    0x09, 0x01,                        //   (LOCAL)  USAGE (Vendor Usage 1)
// TODO: OS24EVK-66 HID_Descriptor_Tool Error: LOGICAL MIN MUST be defined 
// TODO: OS24EVK-66 HID_Descriptor_Tool Error: LOGICAL MAX MUST be defined 
    0x81, 0x02,                        //   (MAIN)   INPUT (Data,Var,Abs)
#endif // SUPPORT_LIS2DH
   
    0xC0                               // (MAIN)   END_COLLECTION       end Application Collection
	//1 byte
};

// Localization String
#define STR0LEN 4
code const uint8_t String0Desc [STR0LEN] =
{
   STR0LEN,                            // bLength for StringDescriptor
   0x03,                               // bDescriptorType   3: StringDescriptor DSC_STRING
   0x09, 0x04  // US-English
}; // End of String0Desc

// Manufacturer String
#if USB_MFGR_STRING_MAXIMINTEGRATED_COM
#define STR1LEN sizeof ("maximintegrated.com") * 2		// includes NULL terminating character, so don't add 2 bytes
// Unicode UTF-16LE little-endian encoded string, prefixed by length
code const uint8_t String1Desc [STR1LEN] =
{
   STR1LEN,                            // bLength for StringDescriptor
   0x03,                               // bDescriptorType   3: StringDescriptor DSC_STRING
   'm', 0,
   'a', 0,
   'x', 0,
   'i', 0,
   'm', 0,
   'i', 0,
   'n', 0,
   't', 0,
   'e', 0,
   'g', 0,
   'r', 0,
   'a', 0,
   't', 0,
   'e', 0,
   'd', 0,
   '.', 0,
   'c', 0,
   'o', 0,
   'm', 0
}; // End of String1Desc
#else // USB_MFGR_STRING_MAXIMINTEGRATED_COM
#define STR1LEN sizeof ("Maxim-IC,Inc.") * 2		// includes NULL terminating character, so don't add 2 bytes
// Unicode UTF-16LE little-endian encoded string, prefixed by length
code const uint8_t String1Desc [STR1LEN] =
{
   STR1LEN,                            // bLength for StringDescriptor
   0x03,                               // bDescriptorType   3: StringDescriptor DSC_STRING
   'M', 0,
   'a', 0,
   'x', 0,
   'i', 0,
   'm', 0,
   '-', 0,
   'I', 0,
   'C', 0,
   ',', 0,
   'I', 0,
   'n', 0,
   'c', 0,
   '.', 0
}; // End of String1Desc
#endif // USB_MFGR_STRING_MAXIMINTEGRATED_COM

// Product ID String
// Windows: Control Panel: Device Manager:
//   HID: USB Input Device Properties: Details:
//      Bus reported device description
#if SUPPORT_LIS2DH
   #define STR2LEN sizeof ("MAX30101 + LIS2DH") * 2		// includes NULL terminating character, so don't add 2 bytes
   // Unicode UTF-16LE little-endian encoded string, prefixed by length
   code const uint8_t String2Desc [STR2LEN] =
   {
      STR2LEN,                            // bLength for StringDescriptor
      0x03,                               // bDescriptorType   3: StringDescriptor DSC_STRING
      'M', 0,
      'A', 0,
      'X', 0,
      '3', 0,
      '0', 0,
      '1', 0,
      '0', 0,
      '1', 0,
      ' ', 0,
      '+', 0,
      ' ', 0,
      'L', 0,
      'I', 0,
      'S', 0,
      '2', 0,
      'D', 0,
      'H', 0
   }; // End of String2Desc
#else // SUPPORT_LIS2DH
# if USB_PRODUCT_STRING_MAX30101
   #define STR2LEN sizeof ("MAX30101") * 2		// includes NULL terminating character, so don't add 2 bytes
   // Unicode UTF-16LE little-endian encoded string, prefixed by length
   code const uint8_t String2Desc [STR2LEN] =
   {
      STR2LEN,                            // bLength for StringDescriptor
      0x03,                               // bDescriptorType   3: StringDescriptor DSC_STRING
      'M', 0,
      'A', 0,
      'X', 0,
      '3', 0,
      '0', 0,
      '1', 0,
      '0', 0,
      '1', 0
   }; // End of String2Desc
# else
   #define STR2LEN sizeof ("MAX30100") * 2		// includes NULL terminating character, so don't add 2 bytes
   // Unicode UTF-16LE little-endian encoded string, prefixed by length
   code const uint8_t String2Desc [STR2LEN] =
   {
      STR2LEN,                            // bLength for StringDescriptor
      0x03,                               // bDescriptorType   3: StringDescriptor DSC_STRING
      'M', 0,
      'A', 0,
      'X', 0,
      '3', 0,
      '0', 0,
      '1', 0,
      '0', 0,
      '0', 0
   }; // End of String2Desc
# endif // USB_PRODUCT_STRING_MAX30101
#endif // SUPPORT_LIS2DH

// USB String Descriptor Table
uint8_t* const STRINGDESCTABLE [] =
{
   String0Desc, // Localization String
   String1Desc, // Manufacturer String
   String2Desc  // Product ID String
};
