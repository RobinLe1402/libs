@ECHO OFF

TITLE RobinLe Environment Variable Setup

REM Call parameters
SET ParamInstall=INSTALL
SET ParamUninstall=UNINST

SET "EnvVar_GitHub_Lib=GitHub_rl_libs"


IF "%1" EQU "%ParamInstall%" GOTO :Install
IF "%1" EQU "%ParamUninstall%" GOTO :Uninstall

REM Neither INSTALL nor UNINST as parameter --> invalid call
:InvalidCall
ECHO Invalid call. Please use %ParamInstall% or %ParamUninstall% as call parameter.
ECHO %1
PAUSE
EXIT



:Install
CD ..\..
ECHO Setting up environment variable "%EnvVar_GitHub_Lib%" as "%CD%"
SETX %EnvVar_GitHub_Lib% %CD%\
ECHO.

PAUSE
EXIT



:Uninstall
ECHO Deleting environment variable "%%%EnvVar_GitHub_Lib%%%"
REG DELETE HKCU\ENVIRONMENT /F /V %EnvVar_GitHub_Lib%
ECHO.

PAUSE
EXIT