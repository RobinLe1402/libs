@ECHO OFF
REM This script tries to every FONT resource of every .fon file in a directory

IF NOT EXIST ".\extractFNT.exe" GOTO :NOEXE

ECHO Input folder:
SET /P "sSRC="
SET sSRC=%sSRC:"=%

ECHO.

ECHO Output folder:
SET /P "sDEST="
SET sDEST=%sDEST:"=%

FOR /R "%sSRC%" %%f IN (*.fon) DO (
ECHO.
ECHO.
ECHO.
ECHO File: "%%~nxf"
extractFNT "%%f" * "%sDEST%"
)

PAUSE
EXIT


:NOEXE
ECHO "extractFNT.exe" not found.
PAUSE
EXIT