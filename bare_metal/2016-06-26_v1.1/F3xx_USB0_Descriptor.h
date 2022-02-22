//-----------------------------------------------------------------------------
// F3xx_USB0_Descriptor.h
//-----------------------------------------------------------------------------
// Copyright 2014
//
// Program Description:
//
// Header file for USB firmware. Includes descriptor data.
//
//
// FID             3XX000011
// Target:         C8051F32x/C8051F340
// Tool chain:     Keil C51 7.50 / Keil EVAL C51
//                 Silicon Laboratories IDE version 2.6
// Project Name:   Generic HID Firmware
//
//
// Release 1.0
//    -140401 Initial Revision (Tushar Mazumder)
//
// Other than bug fixes or implementation changes, this file should not need to be updated.

#ifndef  _USB_DESC_H_
#define  _USB_DESC_H_

// Project-level preprocessor #defines effective across all *.c files
#include "ProjectDefines.h"

// WORD type definition, for KEIL Compiler
#ifndef _WORD_DEF_                     // Compiler Specific, written for
                                       // Little Endian
#define _WORD_DEF_
typedef union {unsigned int i; unsigned char c[2];} WORD;
#define LSB 1                          // All words sent to and received
                                       // from the host are
#define MSB 0                          // little endian, this is switched
                                       // by software when neccessary.
                                       // These sections of code have been
                                       // marked with "Compiler Specific"
                                       // as above for easier modification
#endif   /* _WORD_DEF_ */



// Byte manipulation macros
// [mku] from MINIQUSB CmodComm.h
// https://svn.maxim-ic.com/svn/evkits/MINIQUSB/software/CmodComm_DLL/MinGW_w32/trunk/CMODCOMM.H
///
/// Return the low byte of a 16-bit value.
/// @see HIGHBYTE, SWAP_HIGH_AND_LOW_BYTE, COMBINE_HIGH_AND_LOW_BYTE
///
#define LOWBYTE(data16) ((uint8_t)(data16 & 0xFF))
///
/// Return the high byte of a 16-bit value.
/// @see LOWBYTE, SWAP_HIGH_AND_LOW_BYTE, COMBINE_HIGH_AND_LOW_BYTE
///
#define HIGHBYTE(data16) ((uint8_t)(data16 >> 8))
///
/// Combine two 8-bit values into a 16-bit value.
/// @see HIGHBYTE, LOWBYTE, SWAP_HIGH_AND_LOW_BYTE
///
//~ #define COMBINE_HIGH_AND_LOW_BYTE(highbyte8, lowbyte8) ((uint16_t)(((uint16_t)highbyte8 * 256) + lowbyte8))
#define COMBINE_HIGH_AND_LOW_BYTE(highbyte8, lowbyte8) ((uint16_t)( (((uint16_t)highbyte8 << 8) & 0xFF00) + (lowbyte8 & 0xFF) ))
///
/// Swap the high and low bytes of a 16-bit value.
/// Use this macro to compensate for big-endian versus little-endian data mismatch.
/// @see HIGHBYTE, LOWBYTE, COMBINE_HIGH_AND_LOW_BYTE
///
#define SWAP_HIGH_AND_LOW_BYTE(data16) (COMBINE_HIGH_AND_LOW_BYTE(LOWBYTE(data16), HIGHBYTE(data16)))


//------------------------------------------
// Standard Device Descriptor Type Defintion
//------------------------------------------
typedef /*code*/ struct
{
   uint8_t bLength;              // Size of this Descriptor in Bytes
   uint8_t bDescriptorType;      // Descriptor Type (=1) 1: DeviceDescriptor DSC_DEVICE
   WORD bcdUSB;                        // USB Spec Release Number in BCD
   uint8_t bDeviceClass;         // Device Class Code
   uint8_t bDeviceSubClass;      // Device Subclass Code
   uint8_t bDeviceProtocol;      // Device Protocol Code
   uint8_t bMaxPacketSize0;      // Maximum Packet Size for EP0
   WORD idVendor;                      // Vendor ID
   WORD idProduct;                     // Product ID
   WORD bcdDevice;                     // Device Release Number in BCD
   uint8_t iManufacturer;        // Index of String Desc for Manufacturer
   uint8_t iProduct;             // Index of String Desc for Product
   uint8_t iSerialNumber;        // Index of String Desc for SerNo
   uint8_t bNumConfigurations;   // Number of possible Configurations
} device_descriptor;                   // End of Device Descriptor Type

//--------------------------------------------------
// Standard Configuration Descriptor Type Definition
//--------------------------------------------------
typedef /*code*/ struct
{
   uint8_t bLength;              // Size of this Descriptor in Bytes
   uint8_t bDescriptorType;      // Descriptor Type (=2) 2: ConfigurationDescriptor DSC_CONFIG
   WORD wTotalLength;                  // Total Length of Data for this Conf
   uint8_t bNumInterfaces;       // No of Interfaces supported by this
                                       // Conf
   uint8_t bConfigurationValue;  // Designator Value for *this*
                                       // Configuration
   uint8_t iConfiguration;       // Index of String Desc for this Conf
   uint8_t bmAttributes;         // Configuration Characteristics (see below)
   uint8_t bMaxPower;            // Max. Power Consumption in this
                                       // Conf (*2mA)
} configuration_descriptor;            // End of Configuration Descriptor Type

//----------------------------------------------
// Standard Interface Descriptor Type Definition
//----------------------------------------------
typedef /*code*/ struct
{
   uint8_t bLength;              // Size of this Descriptor in Bytes
   uint8_t bDescriptorType;      // Descriptor Type (=4) 4: InterfaceDescriptor DSC_INTERFACE
   uint8_t bInterfaceNumber;     // Number of *this* Interface (0..)
   uint8_t bAlternateSetting;    // Alternative for this Interface (if any)
   uint8_t bNumEndpoints;        // No of EPs used by this IF (excl. EP0)
   uint8_t bInterfaceClass;      // Interface Class Code
   uint8_t bInterfaceSubClass;   // Interface Subclass Code
   uint8_t bInterfaceProtocol;   // Interface Protocol Code
   uint8_t iInterface;           // Index of String Desc for this Interface
} interface_descriptor;                // End of Interface Descriptor Type

//------------------------------------------
// Standard Class Descriptor Type Definition
//------------------------------------------
typedef /*code */struct
{
   uint8_t bLength;              // Size of this Descriptor in Bytes (=9)
   uint8_t bDescriptorType;      // Descriptor Type (HID=0x21) DSC_HID
   WORD bcdHID;    				         // HID Class Specification
                                       // release number (=1.01)
   uint8_t bCountryCode;         // Localized country code
   uint8_t bNumDescriptors;	   // Number of class descriptors to follow
   uint8_t bReportDescriptorType;// Report descriptor type (HID=0x22)
   uint16_t wItemLength;			   // Total length of report descriptor table
} class_descriptor;                    // End of Class Descriptor Type

//---------------------------------------------
// Standard Endpoint Descriptor Type Definition
//---------------------------------------------
typedef /*code*/ struct
{
   uint8_t bLength;              // Size of this Descriptor in Bytes
   uint8_t bDescriptorType;      // Descriptor Type (=5) 5: EndPointDescriptor DSC_ENDPOINT
   uint8_t bEndpointAddress;     // Endpoint Address (Number + Direction)
   uint8_t bmAttributes;         // Endpoint Attributes (Transfer Type)
   WORD wMaxPacketSize;	               // Max. Endpoint Packet Size
   uint8_t bInterval;            // Polling Interval (Interrupt) ms
} endpoint_descriptor;                 // End of Endpoint Descriptor Type

//---------------------------------------------
// HID Configuration Descriptor Type Definition
//---------------------------------------------
// From "USB Device Class Definition for Human Interface Devices (HID)".
// Section 7.1:
// "When a Get_Descriptor(Configuration) request is issued,
// it returns the Configuration descriptor, all Interface descriptors,
// all Endpoint descriptors, and the HID descriptor for each interface."
typedef code struct {
	configuration_descriptor 	hid_configuration_descriptor;
	interface_descriptor 		hid_interface_descriptor;
	class_descriptor 			hid_descriptor;
	endpoint_descriptor 		hid_endpoint_in_descriptor;
	endpoint_descriptor 		hid_endpoint_out_descriptor;
}
hid_configuration_descriptor;

#if SUPPORT_LIS2DH
//
// VERIFY: OS24EVK-66 update *F3xx_USB0_Descriptor.c* hid_report_descriptor to define HID REPORT_ID(2) format
#define EXPLICIT_REPORT_ID			// comment out if using implicit report ID
#define HID_REPORT_DESCRIPTOR_SIZE    0x0028
// VERIFY: OS24EVK-66 use SWAP_HIGH_AND_LOW_BYTE(data16) macro to convert big-endian to little-endian
#define HID_REPORT_DESCRIPTOR_SIZE_LE 0x2800
#define SHORT_REPORT_ID 1
#define HID_REPORT_ID_1 1
#define HID_REPORT_ID_2 2
//
#else // SUPPORT_LIS2DH
//
#define EXPLICIT_REPORT_ID			// comment out if using implicit report ID
#ifdef EXPLICIT_REPORT_ID
#define HID_REPORT_DESCRIPTOR_SIZE    0x0018
#define HID_REPORT_DESCRIPTOR_SIZE_LE 0x1800
// VERIFY: OS24EVK-66 use SWAP_HIGH_AND_LOW_BYTE(data16) macro to convert big-endian to little-endian
#else
#define HID_REPORT_DESCRIPTOR_SIZE    0x0016
#define HID_REPORT_DESCRIPTOR_SIZE_LE 0x1600
// VERIFY: OS24EVK-66 use SWAP_HIGH_AND_LOW_BYTE(data16) macro to convert big-endian to little-endian
#endif
#define SHORT_REPORT_ID 1
#define HID_REPORT_ID_1 1
//
#endif // SUPPORT_LIS2DH

typedef code uint8_t hid_report_descriptor[HID_REPORT_DESCRIPTOR_SIZE];

//-----------------------------
// SETUP Packet Type Definition
//-----------------------------
typedef struct
{
   uint8_t bmRequestType;        // Request recipient, type, and dir.
   uint8_t bRequest;             // Specific standard request number
   WORD wValue;                        // varies according to request
   WORD wIndex;                        // varies according to request
   WORD wLength;                       // Number of bytes to transfer
} setup_buffer;                        // End of SETUP Packet Type

#endif  /* _USB_DESC_H_ */