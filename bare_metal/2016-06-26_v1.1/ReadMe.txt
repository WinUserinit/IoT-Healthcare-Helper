-------------------------------------------------------------------------------
 Readme.txt
-------------------------------------------------------------------------------

Copyright 2014

Program Description:
-------------------

HID firmware readme


Target and Tool Chain Information:
---------------------------------

FID:            3XX000015
Target:         C8051F320/1, C8051F326/7, C8051F340
Tool chain:     Keil C51 7.50 / Keil EVAL C51
                Silicon Laboratories IDE version 2.6
Project Name:   Generic HID Firmware


Command Line Options:
--------------------

Assembler : Default
Compiler  : Default
Linker    : Default 


File List:
---------

c8051f3xx.h									Original SiLabs file.
F3xx_Initialize.h							Other than bug fixes or implementation changes, this file should not need to be updated.
F3xx_USB0_Descriptor.h						Other than bug fixes or implementation changes, this file should not need to be updated.
F3xx_USB0_InterruptServiceRoutine.h			Other than bug fixes or implementation changes, this file should not need to be updated.
F3xx_USB0_Register.h						Original SiLabs file.
F3xx_USB0_ReportHandler.h					Other than bug fixes or implementation changes, this file should not need to be updated.
SMBus.h										Other than bug fixes or implementation changes, this file should not need to be updated.
F3xx_Initialize.c							Other than bug fixes or implementation changes, this file should not need to be updated. (But, configure port pins in this file if using this file as a base for other projects.)
F3xx_USB0_Descriptor.c						Update VID, PID and product string in this file.
F3xx_USB0_InterruptServiceRoutine.c			Other than bug fixes or implementation changes, this file should not need to be updated.
F3xx_USB0_USB0_Main.c						Update main program in this file.
F3xx_USB0_ReportHandler.c					OUT_REPORT_HANDLER may be modified to new features.
F3xx_USB0_Standard_Requests.c				Other than bug fixes or implementation changes, this file should not need to be updated.
SMBus.c										Other than bug fixes or implementation changes, this file should not need to be updated.
SPI.c										Other than bug fixes or implementation changes, this file should not need to be updated.

ReadMe.txt (this file)	


Release Information:
-------------------
	        
Version 1.0
	Initial release (Tushar Mazumder).
        1 APR 2014

-------------------------------------------------------------------------------
 End Of File
-------------------------------------------------------------------------------