:: Build using Keil C51 compiler (8-bit C8051F321 target)
::
:: Enable delayed expansion of variables
:: http://stackoverflow.com/a/245442/2427089
setlocal enableextensions enabledelayedexpansion
::
set SRCFILES=*.c
set HEXFILE=os24
::
:: Keil C51 directive DB = DEBUG = generate debug information in object file
:: Keil C51 directive OE = OBJECTEXTEND = include type definition information in object file
:: Keil C51 directive LA = Large = large memory model: variables and local data in xdata
:: Keil C51 directive CD = CODE =  append assembly mnemonics listing to LST listing file
:: Keil C51 directive DF = DEFINE = preprocessor symbol; example DF (X1='1+5',iofunc='getkey ()')
set C51FLAGS=DB OE Large CD
::
:: Keil BL51 linker directives (in cyglink.txt)
:: Keil BL51 linker directive TO "C:\FeatureBranch\OS24EVK-66\os24" = ???
:: Keil BL51 linker directive RS(256) = RAMSIZE(256) = number of bytes of DATA and IDATA in target 8051 device (min 64, default 128, max 256)
:: Keil BL51 linker directive PL(68) = PAGELENGTH(68) = number of lines printed per page in the linker map file (min 10)
:: Keil BL51 linker directive PW(78) = PAGEWIDTH(78) = number of character per line that may be printed to the linker map file (min 72, max 132)
:: Keil BL51 linker directive NOOL = NOOVERLAY  = linker does not overlay memory used by local variables and function arguments.
:: Keil BL51 linker directive IX = IXREF = generate a cross-reference report in the linker map 
::
@title Searching for Keil C51
set KEILPATH=c:\keil\C51\BIN
::
@if not exist %KEILPATH%\C51.exe goto :errorMissingCompiler
@if not exist %KEILPATH%\bl51.exe goto :errorMissingLinker
@if not exist %KEILPATH%\oh51.exe goto :errorMissingHexConverter
::
echo Rebuilding... >output.txt
del %HEXFILE%.hex
del *.OBJ
del *.LST
@title Compiling
:: Keil C51 C compiler
::    %KEILPATH%\C51.exe "F3xx_USB0_InterruptServiceRoutine.c" %C51FLAGS%
::    %KEILPATH%\C51.exe "SPI.c" %C51FLAGS%
@for %%f in (%SRCFILES%) do @(
   @title Compiling %%f
   @echo. >>output.txt 2>&1
   @echo. Compiling %%f >>output.txt 2>&1
   @echo %KEILPATH%\C51.exe "%%f" %C51FLAGS% >>output.txt 2>&1
   %KEILPATH%\C51.exe "%%f" %C51FLAGS% >>output.txt 2>&1
)
::
@title Linking
:: Keil C51 linker
@echo %KEILPATH%\bl51.exe @".\cyglink.txt" >>output.txt 2>&1
%KEILPATH%\bl51.exe @".\cyglink.txt" >>output.txt 2>&1
::
@title Converting to Hex
:: Object to Hex file converter
@echo %KEILPATH%\oh51.exe ".\%HEXFILE%" >>output.txt 2>&1
%KEILPATH%\oh51.exe ".\%HEXFILE%" >>output.txt 2>&1
if errorlevel 1 goto :errorBuildError
::
if exist C:\Python32\pythonw.exe ( 
   @echo C:\Python32\pythonw -u "fixup_os24_m51.py"
   C:\Python32\pythonw -u "fixup_os24_m51.py" 
   )
::
:: this isn't a valid existence test;
:: in case of failure both os24 and os24.hex exist as zero-length files.
if not exist %HEXFILE%.hex goto :errorBuildError
::
:success
@title Success
type output.txt
@find "*** ERROR" output.txt
goto :done
:errorMissingCompiler
@echo --- Failure --- Compiler Not Installed.
@echo .
@echo       expected %KEILPATH%\C51.exe
@echo .
pause
goto done
:errorMissingLinker
@echo --- Failure --- Compiler Not Installed.
@echo .
@echo       expected %KEILPATH%\bl51.exe
@echo .
pause
goto done
:errorMissingHexConverter
@echo --- Failure --- Compiler Not Installed.
@echo .
@echo       expected %KEILPATH%\oh51.exe
@echo .
pause
goto done
:errorBuildError
type output.txt
@echo --- Failure --- Build error
@echo .
@echo       unable to build %HEXFILE%.hex
@echo .
@find "*** ERROR" output.txt
pause
goto done
:done
@echo Done.
endlocal
