//-----------------------------------------------------------------------------
// F3xx_USB0_InterruptServiceRoutine.c
//-----------------------------------------------------------------------------
// Copyright 2014
//
// Program Description:
//
// Source file for USB firmware. Includes top level ISR with SETUP,
// and Endpoint data handlers.  Also includes routine for USB suspend,
// reset, and procedural stall.
//
//
// FID:            3XX000005
// Target:         C8051F32x
// Tool chain:     Keil C51 7.50 / Keil EVAL C51
//                 Silicon Laboratories IDE version 2.6
// Project Name:   generic
//
//
// Release 1.0
//    -140401 Initial Revision (Tushar Mazumder)
//
// Other than bug fixes or implementation changes, this file should not need to be updated.


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "c8051f3xx.h"
#include "F3xx_USB0_Register.h"
#include "F3xx_USB0_InterruptServiceRoutine.h"
#include "F3xx_USB0_Descriptor.h"
#include "F3xx_USB0_ReportHandler.h"


//-----------------------------------------------------------------------------
// Global Variable Definitions
//-----------------------------------------------------------------------------
uint8_t USB0_STATE;              // Holds the current USB State
                                       // def. in F3xx_USB0_InterruptServiceRoutine.h

setup_buffer SETUP;                    // Buffer for current device
                                       // request information

uint16_t DATASIZE;                 // Size of data to return
uint16_t DATASENT;                 // Amount of data sent so far
uint8_t* DATAPTR;                // Pointer to data to return

uint8_t EP_STATUS[3] = {EP_IDLE, EP_HALT, EP_HALT};
                                       // Holds the status for each endpoint

//-----------------------------------------------------------------------------
// Local Function Definitions
//-----------------------------------------------------------------------------
void Usb_Resume (void);                // Resumes USB operation
void Usb_Reset (void);                 // Called after USB bus reset
void Handle_Control (void);            // Handle SETUP packet on EP 0
#if USB_writeReadHID_EP1OUT_EP1IN
   // VERIFY: OS24EVK-66 Support USB_writeReadHID_EP3OUT_EP3IN use EP3IN/EP3OUT for writeReadHID() commands
   // # pragma message("writeReadHID() EP1OUT 64-byte buffer")
   // # pragma message("writeReadHID() EP1IN  64-byte buffer")
void Handle_In1 (void);                // Handle EP1IN in packet on EP 1
void Handle_Out1 (void);               // Handle EP1OUT out packet on EP 1
#elif USB_writeReadHID_EP2OUT_EP2IN
   // # pragma message("writeReadHID() EP2OUT 128-byte buffer")
   // # pragma message("writeReadHID() EP2IN  128-byte buffer")
void Handle_In2 (void);                // Handle EP2IN in packet on EP 2
void Handle_Out2 (void);               // Handle EP2OUT out packet on EP 2
#elif USB_writeReadHID_EP3OUT_EP3IN
   // # pragma message("writeReadHID() EP3OUT 256-byte buffer")
   // # pragma message("writeReadHID() EP3IN  256-byte buffer")
void Handle_In3 (void);                // Handle EP3IN in packet on EP 3
void Handle_Out3 (void);               // Handle EP3OUT out packet on EP 3
#endif // USB_writeReadHID_EP1OUT_EP1IN

void Usb_Suspend (void);               // This routine called when
                                       // Suspend signalling on bus
void Fifo_Read (uint8_t, uint16_t, uint8_t *);
                                       // Used for multiple byte reads
                                       // of Endpoint fifos
void Fifo_Write_Foreground (uint8_t, uint16_t, uint8_t *);
                                       // Used for multiple byte writes
                                       // of Endpoint fifos in foreground
void Fifo_Write_InterruptServiceRoutine (uint8_t, uint16_t,
										 uint8_t *);
                                       // Used for multiple byte
                                       // writes of Endpoint fifos


//-----------------------------------------------------------------------------
// USB Interface Interrupt Service Routine (ISR)
//-----------------------------------------------------------------------------
// Called after any USB type interrupt, this handler determines which type
// of interrupt occurred, and calls the specific routine to handle it.
//
void Usb_ISR (void) interrupt INTERRUPT_USB0 /* interrupt 8 */  // Top-level USB ISR
{
   uint8_t bCommon, bIn, bOut;
   POLL_READ_BYTE (CMINT, bCommon);    // Read all interrupt registers
   POLL_READ_BYTE (IN1INT, bIn);       // this read also clears the register (EP0IN EP1IN EP2IN EP3IN)
   POLL_READ_BYTE (OUT1INT, bOut);     // (EP1OUT EP2OUT EP3OUT)
   {
      if (bCommon & rbRSUINT)          // Handle Resume interrupt
      {
         Usb_Resume ();
      }
      if (bCommon & rbRSTINT)          // Handle Reset interrupt
      {
         Usb_Reset ();
      }
      if (bIn & rbEP0)                 // Handle SETUP packet received
      {                                // or packet transmitted if Endpoint 0
         Handle_Control();             // is in transmit mode
      }
#if USB_writeReadHID_EP1OUT_EP1IN
      // VERIFY: OS24EVK-66 Support USB_writeReadHID_EP3OUT_EP3IN use EP3IN/EP3OUT for writeReadHID() commands
      // # pragma message("writeReadHID() EP1OUT 64-byte buffer")
      // # pragma message("writeReadHID() EP1IN  64-byte buffer")
      if (bIn & rbIN1)                 // Handle In Packet sent, put new data
      {                                // on endpoint 1 fifo
         Handle_In1 ();    // EP1IN send data to host
      }
      if (bOut & rbOUT1)               // Handle Out packet received, take
      {                                // data off endpoint 2 fifo - should say endpoint 1 
                                       //   [mku]: <WTF> USB has unambiguous way to refer to endpoints.
                                       //   Why are they calling EP1OUT "endpoint 2"? </WTF>
         Handle_Out1 ();   // EP1OUT receive data from host
      }
#elif USB_writeReadHID_EP2OUT_EP2IN
      // # pragma message("writeReadHID() EP2OUT 128-byte buffer")
      // # pragma message("writeReadHID() EP2IN  128-byte buffer")
      if (bIn & rbIN2)                 // Handle In Packet sent, put new data into EP2IN FIFO
      {
         Handle_In2 ();    // EP2IN send data to host
      }
      if (bOut & rbOUT2)               // Handle Out packet received, take data from EP2OUT FIFO
      {                                
         Handle_Out2 ();   // EP2OUT receive data from host
      }
#elif USB_writeReadHID_EP3OUT_EP3IN
      // # pragma message("writeReadHID() EP3OUT 256-byte buffer")
      // # pragma message("writeReadHID() EP3IN  256-byte buffer")
      if (bIn & rbIN3)                 // Handle In Packet sent, put new data into EP3IN FIFO
      {
         Handle_In3 ();    // EP3IN send data to host
      }
      if (bOut & rbOUT3)               // Handle Out packet received, take data from EP3OUT FIFO
      {                                
         Handle_Out3 ();   // EP3OUT receive data from host
      }
#endif // USB_writeReadHID_EP1OUT_EP1IN
      if (bCommon & rbSUSINT)          // Handle Suspend interrupt
      {
         Usb_Suspend ();
      }
   }
}

//-----------------------------------------------------------------------------
// Support Routines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Usb_Reset
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// - Set state to default
// - Clear Usb Inhibit bit
//
//-----------------------------------------------------------------------------

void Usb_Reset (void)
{
   USB0_STATE = DEV_DEFAULT;           // Set device state to default

   POLL_WRITE_BYTE (POWER, 0x01);      // Clear usb inhibit bit to enable USB
                                       // suspend detection

   EP_STATUS[0] = EP_IDLE;             // Set default Endpoint Status
   EP_STATUS[1] = EP_HALT;
   EP_STATUS[2] = EP_HALT;
}



//-----------------------------------------------------------------------------
// Usb_Resume
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Resume normal USB operation
//
//-----------------------------------------------------------------------------

void Usb_Resume(void)
{
   volatile int16_t k;

   k++;

   // Add code for resume
}


//-----------------------------------------------------------------------------
// Handle_Control
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// - Decode Incoming SETUP requests
// - Load data packets on fifo while in transmit mode
//
//-----------------------------------------------------------------------------

void Handle_Control (void)
{
   uint8_t ControlReg;           // Temporary storage for EP control
                                       // register

   POLL_WRITE_BYTE (INDEX, 0);         // Set Index to Endpoint Zero
   POLL_READ_BYTE (E0CSR, ControlReg); // Read control register

   if (EP_STATUS[0] == EP_ADDRESS)     // Handle Status Phase of Set Address
                                       // command
   {
      POLL_WRITE_BYTE (FADDR, SETUP.wValue.c[LSB]);
      EP_STATUS[0] = EP_IDLE;
   }

   if (ControlReg & rbSTSTL)           // If last packet was a sent stall,
   {                                   // reset STSTL bit and return EP0
                                       // to idle state
      POLL_WRITE_BYTE (E0CSR, 0);
      EP_STATUS[0] = EP_IDLE;
      return;
   }

   if (ControlReg & rbSUEND)           // If last SETUP transaction was
   {                                   // ended prematurely then set
      POLL_WRITE_BYTE (E0CSR, rbDATAEND);
      // Serviced SETUP End bit and return EP0
      POLL_WRITE_BYTE (E0CSR, rbSSUEND);
      EP_STATUS[0] = EP_IDLE;          // To idle state
   }

   if (EP_STATUS[0] == EP_IDLE)        // If Endpoint 0 is in idle mode
   {
      if (ControlReg & rbOPRDY)        // Make sure that EP 0 has an Out Packet
      {                                // ready from host although if EP0
                                       // is idle, this should always be the
                                       // case
         Fifo_Read (FIFO_EP0, 8, (uint8_t *)&SETUP);
                                       // Get SETUP Packet off of Fifo,
                                       // it is currently Big-Endian

                                       // Compiler Specific - these next three
                                       // statements swap the bytes of the
                                       // SETUP packet words to Big Endian so
                                       // they can be compared to other 16-bit
                                       // values elsewhere properly
         SETUP.wValue.i = SETUP.wValue.c[MSB] + 256*SETUP.wValue.c[LSB];
         SETUP.wIndex.i = SETUP.wIndex.c[MSB] + 256*SETUP.wIndex.c[LSB];
         SETUP.wLength.i = SETUP.wLength.c[MSB] + 256*SETUP.wLength.c[LSB];

		// Intercept HID class-specific requests
		if( (SETUP.bmRequestType & ~0x80) == DSC_HID) {
			switch (SETUP.bRequest) {
				case GET_REPORT:
					Get_Report ();
					break;
				case SET_REPORT:
					Set_Report ();
					break;
				case GET_IDLE:
					Get_Idle ();
					break;
				case SET_IDLE:
					Set_Idle ();
					break;
				case GET_PROTOCOL:
					Get_Protocol ();
					break;
				case SET_PROTOCOL:
					Set_Protocol ();
					break;
	            default:
                  Force_Stall ();      // Send stall to host if invalid
	            break;                  // request
			}
		} else

         switch (SETUP.bRequest)       // Call correct subroutine to handle
         {                             // each kind of standard request
            case GET_STATUS:
               Get_Status ();
               break;
            case CLEAR_FEATURE:
               Clear_Feature ();
               break;
            case SET_FEATURE:
               Set_Feature ();
               break;
            case SET_ADDRESS:
               Set_Address ();
               break;
            case GET_DESCRIPTOR:
               Get_Descriptor ();
               break;
            case GET_CONFIGURATION:
               Get_Configuration ();
               break;
            case SET_CONFIGURATION:
               Set_Configuration ();
               break;
            case GET_INTERFACE:
               Get_Interface ();
               break;
            case SET_INTERFACE:
               Set_Interface ();
               break;
            default:
               Force_Stall ();         // Send stall to host if invalid request
               break;
         }
      }
   }

   if (EP_STATUS[0] == EP_TX)          // See if endpoint should transmit
   {
      if (!(ControlReg & rbINPRDY) )   // Don't overwrite last packet
      {
         // Read control register
         POLL_READ_BYTE (E0CSR, ControlReg);

         // Check to see if SETUP End or Out Packet received, if so do not put
         // any new data on FIFO
         if ((!(ControlReg & rbSUEND)) || (!(ControlReg & rbOPRDY)))
         {
            // Add In Packet ready flag to E0CSR bitmask
            ControlReg = rbINPRDY;
            if (DATASIZE >= EP0_PACKET_SIZE)
            {
               // Break Data into multiple packets if larger than Max Packet
               Fifo_Write_InterruptServiceRoutine (FIFO_EP0, EP0_PACKET_SIZE,
               									  (uint8_t*)DATAPTR);
               // Advance data pointer
               DATAPTR  += EP0_PACKET_SIZE;
               // Decrement data size
               DATASIZE -= EP0_PACKET_SIZE;
               // Increment data sent counter
               DATASENT += EP0_PACKET_SIZE;
            }
            else
            {
               // If data is less than Max Packet size or zero
               Fifo_Write_InterruptServiceRoutine (FIFO_EP0, DATASIZE,
               									  (uint8_t*)DATAPTR);
               ControlReg |= rbDATAEND;// Add Data End bit to bitmask
               EP_STATUS[0] = EP_IDLE; // Return EP 0 to idle state
            }
            if (DATASENT == SETUP.wLength.i)
            {
               // This case exists when the host requests an even multiple of
               // your endpoint zero max packet size, and you need to exit
               // transmit mode without sending a zero length packet
               ControlReg |= rbDATAEND;// Add Data End bit to mask
               EP_STATUS[0] = EP_IDLE; // Return EP 0 to idle state
            }
            // Write mask to E0CSR
            POLL_WRITE_BYTE(E0CSR, ControlReg);
         }
      }
   }

   if (EP_STATUS[0] == EP_RX)          // See if endpoint should transmit
   {
      // Read control register
      POLL_READ_BYTE (E0CSR, ControlReg);
      if (ControlReg & rbOPRDY)        // Verify packet was received
      {
         ControlReg = rbSOPRDY;
   		if (DATASIZE >= EP0_PACKET_SIZE)
   		{
            Fifo_Read(FIFO_EP0, EP0_PACKET_SIZE, (uint8_t*)DATAPTR);
            // Advance data pointer
            DATAPTR  += EP0_PACKET_SIZE;
            // Decrement data size
            DATASIZE -= EP0_PACKET_SIZE;
            // Increment data sent counter
            DATASENT += EP0_PACKET_SIZE;
   		}
   		else
   		{
            // read bytes from FIFO
            Fifo_Read (FIFO_EP0, DATASIZE, (uint8_t*) DATAPTR);

            ControlReg |= rbDATAEND;   // Signal end of data
            EP_STATUS[0] = EP_IDLE;    // set Endpoint to IDLE
   		}
   		if (DATASENT == SETUP.wLength.i)
   		{
   			ControlReg |= rbDATAEND;
   			EP_STATUS[0] = EP_IDLE;
   		}
         // If EP_RX mode was entered through a SET_REPORT request,
         // call the ReportHandler_OUT function and pass the Report
         // ID, which is the first by the of DATAPTR's buffer
         if ( (EP_STATUS[0] == EP_IDLE) && (SETUP.bRequest == SET_REPORT) )
         {
            OUT_REPORT_HANDLER ( /* internalCall */ 1); // internal request
         }

         if (EP_STATUS[0] != EP_STALL) POLL_WRITE_BYTE (E0CSR, ControlReg);
      }
   }

}

//-----------------------------------------------------------------------------
// Handle_In1
//-----------------------------------------------------------------------------
//
// Handler will be entered after the endpoint's buffer has been
// transmitted to the host.  In1_StateMachine is set to Idle, which
// signals the foreground routine SendPacket that the Endpoint
// is ready to transmit another packet.
//-----------------------------------------------------------------------------
#if USB_writeReadHID_EP1OUT_EP1IN
   // VERIFY: OS24EVK-66 Support USB_writeReadHID_EP3OUT_EP3IN use EP3IN/EP3OUT for writeReadHID() commands
# pragma message("Handle_In1() EP1IN  64-byte buffer")
void Handle_In1 ()
{
	EP_STATUS[1] = EP_IDLE;
}
#elif USB_writeReadHID_EP2OUT_EP2IN
# pragma message("Handle_In2() EP2IN  128-byte buffer")
void Handle_In2 ()
{
	EP_STATUS[2] = EP_IDLE;
}
#elif USB_writeReadHID_EP3OUT_EP3IN
# pragma message("Handle_In3() EP3IN  256-byte buffer")
void Handle_In3 ()
{
	EP_STATUS[3] = EP_IDLE;
}
#endif // USB_writeReadHID_EP1OUT_EP1IN

//-----------------------------------------------------------------------------
// Handle_Out1
//-----------------------------------------------------------------------------
// Take the received packet from the host off the fifo and put it into
// the IO_PACKET array.
//
//-----------------------------------------------------------------------------
#if USB_writeReadHID_EP1OUT_EP1IN
   // VERIFY: OS24EVK-66 Support USB_writeReadHID_EP3OUT_EP3IN use EP3IN/EP3OUT for writeReadHID() commands
# pragma message("Handle_Out1() EP1OUT 64-byte buffer")
void Handle_Out1 ()
{

   uint8_t Count = 0;
   uint8_t ControlReg;

   POLL_WRITE_BYTE (INDEX, 1);         // Set index to endpoint 2 registers; should be enpoint 1
   POLL_READ_BYTE (EOUTCSR1, ControlReg);

   if (EP_STATUS[1] == EP_HALT)        // If endpoint is halted, send a stall
   {
      POLL_WRITE_BYTE (EOUTCSR1, rbOutSDSTL);
   }

   else                                // Otherwise read received packet
                                       // from host
   {
      if (ControlReg & rbOutSTSTL)     // Clear sent stall bit if last
                                       // packet was a stall
      {
         POLL_WRITE_BYTE (EOUTCSR1, rbOutCLRDT);
      }

      Setup_IO_BUFFER ();             // Configure buffer to save
                                       // received data
      Fifo_Read(FIFO_EP1, IO_BUFFER.Length, IO_BUFFER.Ptr);
	  

      // Process data according to received Report ID.
      // In systems with Report Descriptors that do not define report IDs,
      // the host will still format OUT packets with a prefix byte
      // of '0x00'.

      OUT_REPORT_HANDLER ( /* internalCall */ 0); // external request

      POLL_WRITE_BYTE (EOUTCSR1, 0);   // Clear Out Packet ready bit
   }
}
#elif USB_writeReadHID_EP2OUT_EP2IN
# pragma message("Handle_Out2() EP2OUT 128-byte buffer")
// VERIFY: OS24EVK-66 implement void Handle_Out2() for USB_writeReadHID_EP2OUT_EP2IN=1
void Handle_Out2 ()
{

   uint8_t Count = 0;
   uint8_t ControlReg;

   POLL_WRITE_BYTE (INDEX, 2);         // Set index to endpoint 2 registers; should be enpoint 1
   POLL_READ_BYTE (EOUTCSR1, ControlReg);

   if (EP_STATUS[2] == EP_HALT)        // If endpoint is halted, send a stall
   {
      POLL_WRITE_BYTE (EOUTCSR1, rbOutSDSTL);
   }

   else                                // Otherwise read received packet
                                       // from host
   {
      if (ControlReg & rbOutSTSTL)     // Clear sent stall bit if last
                                       // packet was a stall
      {
         POLL_WRITE_BYTE (EOUTCSR1, rbOutCLRDT);
      }

      Setup_IO_BUFFER ();             // Configure buffer to save
                                       // received data
      Fifo_Read(FIFO_EP2, IO_BUFFER.Length, IO_BUFFER.Ptr);
	  

      // Process data according to received Report ID.
      // In systems with Report Descriptors that do not define report IDs,
      // the host will still format OUT packets with a prefix byte
      // of '0x00'.

      OUT_REPORT_HANDLER ( /* internalCall */ 0); // external request

      POLL_WRITE_BYTE (EOUTCSR1, 0);   // Clear Out Packet ready bit
   }
}
#elif USB_writeReadHID_EP3OUT_EP3IN
# pragma message("Handle_Out3() EP3OUT 256-byte buffer")
// VERIFY: OS24EVK-66 implement void Handle_Out3() for USB_writeReadHID_EP3OUT_EP3IN=1
void Handle_Out3 ()
{

   uint8_t Count = 0;
   uint8_t ControlReg;

   POLL_WRITE_BYTE (INDEX, 3);         // Set index to endpoint 2 registers; should be enpoint 1
   POLL_READ_BYTE (EOUTCSR1, ControlReg);

   if (EP_STATUS[3] == EP_HALT)        // If endpoint is halted, send a stall
   {
      POLL_WRITE_BYTE (EOUTCSR1, rbOutSDSTL);
   }

   else                                // Otherwise read received packet
                                       // from host
   {
      if (ControlReg & rbOutSTSTL)     // Clear sent stall bit if last
                                       // packet was a stall
      {
         POLL_WRITE_BYTE (EOUTCSR1, rbOutCLRDT);
      }

      Setup_IO_BUFFER ();             // Configure buffer to save
                                       // received data
      Fifo_Read(FIFO_EP3, IO_BUFFER.Length, IO_BUFFER.Ptr);
	  

      // Process data according to received Report ID.
      // In systems with Report Descriptors that do not define report IDs,
      // the host will still format OUT packets with a prefix byte
      // of '0x00'.

      OUT_REPORT_HANDLER ( /* internalCall */ 0); // external request

      POLL_WRITE_BYTE (EOUTCSR1, 0);   // Clear Out Packet ready bit
   }
}
#endif // USB_writeReadHID_EP1OUT_EP1IN

//-----------------------------------------------------------------------------
// Usb_Suspend
//-----------------------------------------------------------------------------
// Enter suspend mode after suspend signalling is present on the bus
//
void Usb_Suspend (void)
{
   volatile int16_t k;
   k++;
}

//-----------------------------------------------------------------------------
// Fifo_Read
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) BYTE addr : target address
//                2) uint16_t uNumBytes : number of bytes to unload
//                3) BYTE * pData : read data destination
//
// Read from the selected endpoint FIFO
//
//-----------------------------------------------------------------------------
void Fifo_Read (uint8_t addr, uint16_t uNumBytes,
               uint8_t * pData)
{
   int16_t i;

   if (uNumBytes)                      // Check if >0 bytes requested,
   {
      USB0ADR = (addr);                // Set address
      USB0ADR |= 0xC0;                 // Set auto-read and initiate
                                       // first read

      // Unload <NumBytes> from the selected FIFO
      for(i=0;i< (uNumBytes);i++)
      {
         while (USB0ADR & 0x80);       // Wait for BUSY->'0' (data ready)
         pData[i] = USB0DAT;           // Copy data byte
      }

      //while(USB0ADR & 0x80);         // Wait for BUSY->'0' (data ready)
      USB0ADR = 0;                     // Clear auto-read
   }
}

//-----------------------------------------------------------------------------
// Fifo_Write
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//                1) BYTE addr : target address
//                2) uint16_t uNumBytes : number of bytes to unload
//                3) BYTE * pData : location of source data
//
// Write to the selected endpoint FIFO
//
// Fifo_Write_Foreground is used for function calls made in the foreground routines,
// and Fifo_Write_InterruptServiceRoutine is used for calls made in an ISR.

//-----------------------------------------------------------------------------

void Fifo_Write_Foreground (uint8_t addr, uint16_t uNumBytes,
                    uint8_t * pData)
{
   int16_t i;

   // If >0 bytes requested,
   if (uNumBytes)
   {
      while (USB0ADR & 0x80);          // Wait for BUSY->'0'
                                       // (register available)
      USB0ADR = (addr);                // Set address (mask out bits7-6)

      // Write <NumBytes> to the selected FIFO
      for(i=0;i<uNumBytes;i++)
      {
         USB0DAT = pData[i];
         while (USB0ADR & 0x80);       // Wait for BUSY->'0' (data ready)
      }
   }
}

void Fifo_Write_InterruptServiceRoutine (uint8_t addr,
                                         uint16_t uNumBytes,
                                         uint8_t * pData)
{
   int16_t i;

   // If >0 bytes requested,
   if (uNumBytes)
   {
      while (USB0ADR & 0x80);          // Wait for BUSY->'0'
                                       // (register available)
      USB0ADR = (addr);                // Set address (mask out bits7-6)

      // Write <NumBytes> to the selected FIFO
      for (i=0; i<uNumBytes; i++)
      {
         USB0DAT = pData[i];
         while (USB0ADR & 0x80);       // Wait for BUSY->'0' (data ready)
      }
   }
}

//-----------------------------------------------------------------------------
// Force_Stall
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Force a procedural stall to be sent to the host
//
//-----------------------------------------------------------------------------

void Force_Stall (void)
{
   POLL_WRITE_BYTE (INDEX, 0);
   POLL_WRITE_BYTE (E0CSR, rbSDSTL);   // Set the send stall bit
   EP_STATUS[0] = EP_STALL;            // Put the endpoint in stall status
}


//-----------------------------------------------------------------------------
// SendPacket
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - Report ID that's used to call the appropriate IN handler
//
// This function can be called by other routines to force an IN packet
// transmit.  It takes as an input the Report ID of the packet to be
// transmitted.
//-----------------------------------------------------------------------------
void SendPacket (void)
{
   bit EAState;
   uint8_t ControlReg;

   EAState = EA;
   EA = 0;

   POLL_WRITE_BYTE (INDEX, USB_EP_INDEX); // index 1 for EP1IN to host, 2 for EP2IN to host, 3 for EP3IN to host

   // Read contol register for EP <INDEX>
   POLL_READ_BYTE (EINCSR1, ControlReg);
   
   // If endpoint is currently halted, send a stall
   if (EP_STATUS[USB_EP_INDEX] == EP_HALT)
   {
      POLL_WRITE_BYTE (EINCSR1, rbInSDSTL);
   }
   else if(EP_STATUS[USB_EP_INDEX] == EP_IDLE)
   {
      // the state will be updated inside the ISR handler
      EP_STATUS[USB_EP_INDEX] = EP_TX;

      // Clear sent stall if last packet returned a stall
      if (ControlReg & rbInSTSTL)
      {
         POLL_WRITE_BYTE (EINCSR1, rbInCLRDT);
      }

      // Clear underrun bit if it was set
      if (ControlReg & rbInUNDRUN)
      {
         POLL_WRITE_BYTE (EINCSR1, 0x00);
      }

      //ReportHandler_IN_Foreground (ReportID);
      // Put new data on Fifo
#if USB_writeReadHID_EP1OUT_EP1IN
   // VERIFY: OS24EVK-66 Support USB_writeReadHID_EP3OUT_EP3IN use EP3IN/EP3OUT for writeReadHID() commands
   // # pragma message("writeReadHID() EP1OUT 64-byte buffer")
   // # pragma message("writeReadHID() EP1IN  64-byte buffer")
      Fifo_Write_Foreground (FIFO_EP1, IO_BUFFER.Length,
                    (uint8_t *)IO_BUFFER.Ptr);
#elif USB_writeReadHID_EP2OUT_EP2IN
   // # pragma message("writeReadHID() EP2OUT 128-byte buffer")
   // # pragma message("writeReadHID() EP2IN  128-byte buffer")
      Fifo_Write_Foreground (FIFO_EP2, IO_BUFFER.Length,
                    (uint8_t *)IO_BUFFER.Ptr);
#elif USB_writeReadHID_EP3OUT_EP3IN
   // # pragma message("writeReadHID() EP3OUT 256-byte buffer")
   // # pragma message("writeReadHID() EP3IN  256-byte buffer")
      Fifo_Write_Foreground (FIFO_EP3, IO_BUFFER.Length,
                    (uint8_t *)IO_BUFFER.Ptr);
#endif // USB_writeReadHID_EP1OUT_EP1IN
      
      // Set In Packet ready bit,
      // indicating fresh data on FIFO 1
      POLL_WRITE_BYTE (EINCSR1, rbInINPRDY);
   }

   EA = EAState;
}

// VERIFY: OS24EVK-66 USB_SUPPORT_SendPacketWaitEP1INready=1 SendPacketWaitEP1INready() as SendPacket() with polling for INPRTY=0.
#if USB_SUPPORT_SendPacketWaitEP1INready
void SendPacketWaitEP1INready (void)
{
   bit EAState;
   uint8_t ControlReg;

   EAState = EA;
   EA = 0;

   POLL_WRITE_BYTE (INDEX, USB_EP_INDEX); // index 1 for EP1IN to host, 2 for EP2IN to host, 3 for EP3IN to host

   // Read contol register for EP <INDEX>
   POLL_READ_BYTE (EINCSR1, ControlReg);
   
#if 1
   // VERIFY: OS24EVK-66 SendPacketWaitEP1INready() POLL_READ_BYTE EINCSR1 test for rbInINPRDY=0 (IN packet ready) status clear before start of SendPacket()? (Any issue with first packet?)
   while ((ControlReg & rbInINPRDY) != 0)
   {
      POLL_READ_BYTE (EINCSR1, ControlReg);
   }
#endif   
   
   // If endpoint is currently halted, send a stall
   if (EP_STATUS[USB_EP_INDEX] == EP_HALT)
   {
      POLL_WRITE_BYTE (EINCSR1, rbInSDSTL);
   }
   else if(EP_STATUS[USB_EP_INDEX] == EP_IDLE)
   {
      // the state will be updated inside the ISR handler
      EP_STATUS[USB_EP_INDEX] = EP_TX;

      // Clear sent stall if last packet returned a stall
      if (ControlReg & rbInSTSTL)
      {
         POLL_WRITE_BYTE (EINCSR1, rbInCLRDT);
      }

      // Clear underrun bit if it was set
      if (ControlReg & rbInUNDRUN)
      {
         POLL_WRITE_BYTE (EINCSR1, 0x00);
      }

      //ReportHandler_IN_Foreground (ReportID);
      // Put new data on Fifo
#if USB_writeReadHID_EP1OUT_EP1IN
   // VERIFY: OS24EVK-66 Support USB_writeReadHID_EP3OUT_EP3IN use EP3IN/EP3OUT for writeReadHID() commands
   // # pragma message("writeReadHID() EP1OUT 64-byte buffer")
   // # pragma message("writeReadHID() EP1IN  64-byte buffer")
      Fifo_Write_Foreground (FIFO_EP1, IO_BUFFER.Length,
                    (uint8_t *)IO_BUFFER.Ptr);
#elif USB_writeReadHID_EP2OUT_EP2IN
   // # pragma message("writeReadHID() EP2OUT 128-byte buffer")
   // # pragma message("writeReadHID() EP2IN  128-byte buffer")
      Fifo_Write_Foreground (FIFO_EP2, IO_BUFFER.Length,
                    (uint8_t *)IO_BUFFER.Ptr);
#elif USB_writeReadHID_EP3OUT_EP3IN
   // # pragma message("writeReadHID() EP3OUT 256-byte buffer")
   // # pragma message("writeReadHID() EP3IN  256-byte buffer")
      Fifo_Write_Foreground (FIFO_EP3, IO_BUFFER.Length,
                    (uint8_t *)IO_BUFFER.Ptr);
#endif // USB_writeReadHID_EP1OUT_EP1IN
      
      // Set In Packet ready bit,
      // indicating fresh data on FIFO 1
      POLL_WRITE_BYTE (EINCSR1, rbInINPRDY);
   }

   EA = EAState;
}
#endif

