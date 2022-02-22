The MAX30101 Software Toolkit provides source code resources supporting the MAX30101 optical sensor.

MAX30101 Class Library (.NETv4)
●	Status: tested, verified CSharp source code
●	MAX30101 is configured by setting class instance properties. Most properties include a list of valid settings, or a validation function.
●	enumerated type definition provides symbolic names for each of the device registers, based on the data sheet

MAX30101 Console Demo program
●	Status: tested, verified CSharp source code
●	uses MAX30101 class library to configure sensor and acquire data
●	menu-based program
●	configuration is written to a text file
●	acquired data is written to a csv text file (suitable for import to excel)

Matlab proof-of-concept code 
●	Status: tested, verified in Matlab R2014b (MATLAB version 8.4)
●	uses MAX30101 class library to configure the sensor and acquire data for custom algorithm processing
●	event-handler placeholders where custom matlab post-processing algorithm code could be inserted

Bare Metal C code "device driver" - (i.e. no operating system)
●	Status: tested, verified C source code
●	Compiler: Keil C51 
●	Microcontroller: SiLabs C8051F321 
●	Firmware running on the MAX30101EVKIT and future MAX30101ACCEVKIT system.
●	Latest firmware version (2015-04-22_v1.1) is structured with functions supporting several I2C devices.
    ○	MAX30101 sensor
    ○	LIS2DH accelerometer
    ○	MAX7311 GPIO expander

Linux sysfs Device Driver
●	Status: not tested, not verified, “as-is” C source code
●	not verified - "as-is" - for reference only - Unfortunately I currently don't have a way to test this device driver.
●	modified version of an Android driver (for an NDA customer), removing some features
