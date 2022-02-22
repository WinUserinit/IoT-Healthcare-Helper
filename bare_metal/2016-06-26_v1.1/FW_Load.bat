:: MAX30101EVKIT Firmware Loader
::
@title Verify MCUProgramSettingsPGS is present
::    @set MCUProgramSettingsPGS=C:\FeatureBranch\OS24EVK-66\MAX30101_OS24EVK-66_ProgramSettings.pgs
::    @if exist %MCUProgramSettingsPGS% goto :foundPGSSettings
::
@set MCUProgramSettingsPGS=C:\svn\MAX30101\firmware\Uxx\ToolstickLoader\MAX30101ProgramSettings.pgs
@if exist %MCUProgramSettingsPGS% goto :foundPGSSettings
::
:: TODO: try other likely paths?
@if not exist %MCUProgramSettingsPGS% goto :errorMissingPGSSettings
:foundPGSSettings
@echo found %MCUProgramSettingsPGS%
::
@title Verify MCUProductionProgrammer is present
@echo searching for MCUProductionProgrammer...
@set PathToTool=C:\svn\MAX30101\firmware\Uxx\ToolstickLoader
@set MCUProductionProgrammer=%PathToTool%\MCUProductionProgrammer.exe
::
@title Verify MCUProductionProgrammer is installed
@if not exist %MCUProductionProgrammer% goto :errorToolNotInstalled
:: TODO: try other likely paths?
@echo found %MCUProductionProgrammer%
::
@title MCUProductionProgrammer
cd %PathToTool%
%MCUProductionProgrammer% %MCUProgramSettingsPGS%
::
@title success
:success
goto :eof
:errorMissingPGSSettings
@echo --- Failure --- Missing Tool Not Installed.
@echo .
@echo       expected %MCUProgramSettingsPGS%
@echo .
pause
goto :eof
:errorToolNotInstalled
@echo --- Failure --- Missing Tool Not Installed.
@echo .
@echo       expected %MCUProductionProgrammer%
@echo .
pause
goto :eof
:eof
