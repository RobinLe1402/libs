@ECHO OFF
REM Thie script analyzes each .fnt file in a directory and writes the output to a file
CHCP 1252 > NUL

IF NOT EXIST ".\analyzeFNT.exe" GOTO :NOEXE

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
FOR /R "%sSRC%" %%f IN (*.fnt) DO (
analyzeFNT "%%f">>"%sDEST%"
ECHO.>>"%sDEST%"
ECHO.>>"%sDEST%"
ECHO.>>"%sDEST%"
)

ECHO Done.
PAUSE
EXIT


:NOEXE
ECHO "analyzeFNT.exe" not found.
PAUSE
EXIT