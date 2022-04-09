@ECHO OFF
REM This script analyzes each .fon file in a directory and writes the output to a file
CHCP 1252 > NUL

IF NOT EXIST ".\analyzeFON.exe" GOTO :NOEXE

ECHO Input folder:
SET /P "sSRC="
SET sSRC=%sSRC:"=%

ECHO.

ECHO Output file:
SET /P "sDEST="
SET sDEST=%sDEST:"=%

IF EXIST "%sDEST%" DEL "%sDEST%"

ECHO.
ECHO Processing...
FOR /R "%sSRC%" %%f IN (*.fon) DO (
analyzeFON "%%f">>"%sDEST%"
ECHO.>>"%sDEST%"
ECHO.>>"%sDEST%"
ECHO.>>"%sDEST%"
ECHO.>>"%sDEST%"
ECHO.>>"%sDEST%"
ECHO.>>"%sDEST%"
ECHO.>>"%sDEST%"
ECHO.>>"%sDEST%"
ECHO.>>"%sDEST%"
)

ECHO Done.
PAUSE
EXIT


:NOEXE
ECHO "analyzeFON.exe" not found.
PAUSE
EXIT