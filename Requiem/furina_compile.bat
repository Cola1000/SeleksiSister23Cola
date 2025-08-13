@echo off

set SOURCE_FILE=.\src\furina_no_requiem.c
set OUTPUT_NAME=".\bin\Se mettre sur son trente-et-un.exe"

ECHO Compiling...

gcc -o %OUTPUT_NAME% %SOURCE_FILE%

if errorlevel 1 (
  ECHO Compile failed.
) else (
  ECHO Compile successful! Furina has successfully sung her requiem.
)

PAUSE