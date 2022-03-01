@ECHO OFF
REM Thie script tries to generate a preview bitmap image for each .fon file in a directory

IF NOT EXIST ".\previewFON.exe" GOTO :NOEXE

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
previewFON "%%f" ~ "%sDEST%\%%~nxf.bmp"
)

PAUSE
EXIT


:NOEXE
ECHO "previewFON.exe" not found.
PAUSE
EXIT