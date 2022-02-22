// Project-level preprocessor #defines effective across all *.c files
//
// Normally these would live in a project definition file,
// but I don't see how to do that with this Keil IDE.
//

#ifndef _PROJECT_DEFINES_H
#define _PROJECT_DEFINES_H

// OS24EVK-66 stdint.h exact width integers
typedef unsigned char uint8_t;  // Keil C51: 1-byte scalar
typedef   signed char  int8_t;  // Keil C51: 1-byte scalar
typedef unsigned int  uint16_t; // Keil C51: 2-byte scalar, big-endian
typedef   signed int   int16_t; // Keil C51: 2-byte scalar, big-endian
typedef unsigned long uint32_t; // Keil C51: 4-byte scalar, big-endian
typedef   signed long  int32_t; // Keil C51: 4-byte scalar, big-endian

//------------------------------------------------------------
// Support MAX30100 optical sensor
//
#ifndef SUPPORT_MAX30100
#define SUPPORT_MAX30100 0
#endif

//------------------------------------------------------------
// Support MAX30101 optical sensor
//
#ifndef SUPPORT_MAX30101
#define SUPPORT_MAX30101 1
#endif

//------------------------------------------------------------
// Support MAX30101+Accelerometer LIS2DH
//
#ifndef SUPPORT_LIS2DH
#define SUPPORT_LIS2DH 1
#endif
//
// lis2dh_configure_FIFO argument fifoModeFM10 for streaming data
//
#ifndef LIS2DH_DEFAULT_FIFOMODE
// In bypass mode, the FIFO is not operational and for this reason it remains empty. As
// described in the next figure, for each channel only the first address is used. The remaining
// FIFO slots are empty.
// lis2dh_configure_FIFO(/* fifoModeFM10 0:Bypass */ 0, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
//#define LIS2DH_DEFAULT_FIFOMODE 0
//
// In FIFO mode, data from X, Y and Z channels are stored into the FIFO. A watermark
// interrupt can be enabled (FIFO_WTMK_EN bit into FIFO_CTRL_REG (2E) in order to be
// raised when the FIFO is filled to the level specified into the FIFO_WTMK_LEVEL bits of
// FIFO_CTRL_REG (2E). The FIFO continues filling until it is full (32 slots of data for X, Y and
// Z). When full, the FIFO stops collecting data from the input channels.
// lis2dh_configure_FIFO(/* fifoModeFM10 1:FIFO mode */ 1, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
//#define LIS2DH_DEFAULT_FIFOMODE 1
//
// In the stream mode, data from X, Y and Z measurement are stored into the FIFO. A
// watermark interrupt can be enabled and set as in the FIFO mode.The FIFO continues filling
// until it’s full (32 slots of data for X, Y and Z). When full, the FIFO discards the older data as
// the new arrive.
// lis2dh_configure_FIFO(/* fifoModeFM10 2:Stream mode */ 2, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
#define LIS2DH_DEFAULT_FIFOMODE 2
//
// In Stream-to_FIFO mode, data from X, Y and Z measurement are stored into the FIFO. A
// watermark interrupt can be enabled (FIFO_WTMK_EN bit into FIFO_CTRL_REG) in order
// to be raised when the FIFO is filled to the level specified into the FIFO_WTMK_LEVEL bits
// of FIFO_CTRL_REG. The FIFO continues filling until it’s full (32 slots of 10 bit for for X, Y
// and Z). When full, the FIFO discards the older data as the new arrive. Once trigger event
// occurs, the FIFO starts operating in FIFO mode.
// lis2dh_configure_FIFO(/* fifoModeFM10 3:Trigger mode */ 3, /* fifoTR */ 0, /* fifoWatermarkLevelFTH40 */ 0);
//#define LIS2DH_DEFAULT_FIFOMODE 3
//
#endif
// FIFO can store up to 32 samples; HID report can deliver 10 samples
// LIS2DH_DEFAULT_FIFOWATERMARKLEVEL minimum 0
// LIS2DH_DEFAULT_FIFOWATERMARKLEVEL maximum 31
#ifndef LIS2DH_DEFAULT_FIFOWATERMARKLEVEL
#define LIS2DH_DEFAULT_FIFOWATERMARKLEVEL 26
#endif

//------------------------------------------------------------
// Support MAX7311 GPIO chip (for I2C diagnostic)
// cost: xdata=4, code=289
//
#ifndef SUPPORT_MAX7311
#define SUPPORT_MAX7311 0
#endif


//------------------------------------------------------------
// I2C low level function i2c_smbus_read_byte_data
#if SUPPORT_MAX30100
#define SUPPORT_i2c_smbus_read_byte_data 1
#endif
#if SUPPORT_MAX30101
#define SUPPORT_i2c_smbus_read_byte_data 1
#endif
#if SUPPORT_LIS2DH
#define SUPPORT_i2c_smbus_read_byte_data 1
#endif
#if SUPPORT_MAX7311
#define SUPPORT_i2c_smbus_read_byte_data 1
#endif
#ifndef SUPPORT_i2c_smbus_read_byte_data
#define SUPPORT_i2c_smbus_read_byte_data 0
#endif

//------------------------------------------------------------
// i2c_smbus_read_bytes_data() for reading multiple consecutive bytes
//
// TODO1: use i2c_smbus_read_bytes_data(uint8_t deviceAddress, uint8_t regAddress, uint8_t count, uint8_t *pRegValue)
// TODO1: OS24EVK-66 SUPPORT_i2c_smbus_read_bytes_data=1 breaks the data streaming. (2014-04-09)
#ifndef SUPPORT_i2c_smbus_read_bytes_data
#define SUPPORT_i2c_smbus_read_bytes_data 0
#endif


//------------------------------------------------------------
// USB Manufacturer String
//
#ifndef USB_MFGR_STRING_MAXIMINTEGRATED_COM
#define USB_MFGR_STRING_MAXIMINTEGRATED_COM 1
#endif

//------------------------------------------------------------
// USB Product ID String
//
#ifndef USB_PRODUCT_STRING_MAX30101
#define USB_PRODUCT_STRING_MAX30101 1
#endif

//------------------------------------------------------------
// USB Use Endpoint 1 OUT
// TODONT: OS24EVK-66 #define USB_SUPPORT_EP1OUT 0 : can we omit EP1OUT INTERRUPT endpoint? This would conserve USB FIFO memory and allow double buffering.
// 2015-04-09 the host program really gets stuck without EP1OUT. Why?
// Then again, do I really need an OUT endpoint other than EP0?
// Maybe I do need EP1OUT, to support the writeReadHID() commands.
//#ifndef USB_SUPPORT_EP1OUT
//#define USB_SUPPORT_EP1OUT 1
//// turning this off maybe isn't supported...
//#endif

//------------------------------------------------------------
// USB Endpoint Selector for writeReadHID() commands
//   EP1 has fixed size 64-byte buffer
//   EP2 has fixed size 128-byte buffer
//   EP3 has fixed size 256-byte buffer
//
#ifndef USB_writeReadHID_EP1OUT_EP1IN
#define USB_writeReadHID_EP1OUT_EP1IN 0
#endif
#ifndef USB_writeReadHID_EP2OUT_EP2IN
#define USB_writeReadHID_EP2OUT_EP2IN 0
#endif
#ifndef USB_writeReadHID_EP3OUT_EP3IN
#define USB_writeReadHID_EP3OUT_EP3IN 1
#endif

#if USB_writeReadHID_EP1OUT_EP1IN
# pragma message("writeReadHID() EP1OUT 64-byte buffer")
# pragma message("writeReadHID() EP1IN  64-byte buffer")
# pragma message("USB_EP_INDEX 1 for EP_STATUS[USB_EP_INDEX] and POLL_WRITE_BYTE (INDEX, USB_EP_INDEX);")
# define USB_EP_INDEX 1
#elif USB_writeReadHID_EP2OUT_EP2IN
# pragma message("writeReadHID() EP2OUT 128-byte buffer")
# pragma message("writeReadHID() EP2IN  128-byte buffer")
# pragma message("USB_EP_INDEX 2 for EP_STATUS[USB_EP_INDEX] and POLL_WRITE_BYTE (INDEX, USB_EP_INDEX);")
# define USB_EP_INDEX 2
#elif USB_writeReadHID_EP3OUT_EP3IN
# pragma message("writeReadHID() EP3OUT 256-byte buffer")
# pragma message("writeReadHID() EP3IN  256-byte buffer")
# pragma message("USB_EP_INDEX 3 for EP_STATUS[USB_EP_INDEX] and POLL_WRITE_BYTE (INDEX, USB_EP_INDEX);")
# define USB_EP_INDEX 3
#endif // USB_writeReadHID_EP1OUT_EP1IN

#if USB_writeReadHID_EP1OUT_EP1IN
   // TODO: OS24EVK-66 Support USB_writeReadHID_EP3OUT_EP3IN use EP3IN/EP3OUT for writeReadHID() commands
   // # pragma message("writeReadHID() EP1OUT 64-byte buffer")
   // # pragma message("writeReadHID() EP1IN  64-byte buffer")
#elif USB_writeReadHID_EP2OUT_EP2IN
   // # pragma message("writeReadHID() EP2OUT 128-byte buffer")
   // # pragma message("writeReadHID() EP2IN  128-byte buffer")
#elif USB_writeReadHID_EP3OUT_EP3IN
   // # pragma message("writeReadHID() EP3OUT 256-byte buffer")
   // # pragma message("writeReadHID() EP3IN  256-byte buffer")
#endif // USB_writeReadHID_EP1OUT_EP1IN


#if USB_writeReadHID_EP1OUT_EP1IN
#endif // USB_writeReadHID_EP1OUT_EP1IN

#if USB_writeReadHID_EP2OUT_EP2IN
#endif // USB_writeReadHID_EP2OUT_EP2IN

#if USB_writeReadHID_EP3OUT_EP3IN
#endif // USB_writeReadHID_EP3OUT_EP3IN

//------------------------------------------------------------
// SendPacketWaitEP1INready() as SendPacket() with polling for INPRTY=0.
// Could be endpoint EP1, EP2, or EP3, depending on USB_EP_INDEX
//
#ifndef USB_SUPPORT_SendPacketWaitEP1INready
#define USB_SUPPORT_SendPacketWaitEP1INready 1
#endif


//------------------------------------------------------------
//   P2.1 = diagnostic LED driver DS2 (1=light,0=dark)
//   P2.2 = diagnostic LED driver DS1 (1=light,0=dark)
//
#ifndef SUPPORT_LED1_LED2_DIAGNOSTIC
#define SUPPORT_LED1_LED2_DIAGNOSTIC 1
#endif

//------------------------------------------------------------
//   P2.0 = diagnostic pushbutton on C8051F321 daughtercard TOOLSTICK321DC (0=pressed; 1=not pressed)
// cost: xdata=2, code=50
//
#ifndef SUPPORT_P20BTN_DIAGNOSTIC
#define SUPPORT_P20BTN_DIAGNOSTIC 0
#endif


//------------------------------------------------------------
// Support SPI interface bus
// cost: xdata=21, code=730
//
#ifndef SUPPORT_SPI
#define SUPPORT_SPI 0
#endif


#endif // _PROJECT_DEFINES_H
