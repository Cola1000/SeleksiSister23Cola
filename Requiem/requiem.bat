@REM Ini yang furina (1000 decimal digits)

@REM @echo off

@REM set SOURCE_FILE=.\src\furina_no_requiem.c
@REM set OUTPUT_NAME=".\bin\Se mettre sur son trente-et-un.exe"

@REM ECHO Compiling...

@REM gcc -o %OUTPUT_NAME% %SOURCE_FILE%

@REM if errorlevel 1 (
@REM   ECHO Compile failed.
@REM ) else (
@REM   ECHO Compile successful! Furina has successfully sung her requiem.
@REM )

@REM PAUSE


@REM Ini yang focalor (1e6 decimal digits)

@echo off

set SOURCE_FILE=.\src\focalor_no_requiem.c
set OUTPUT_NAME=".\bin\Sinner's-Finale.exe"

ECHO Compiling...

gcc -o %OUTPUT_NAME% %SOURCE_FILE%

if errorlevel 1 (
  ECHO Compile failed.
) else (
  ECHO Compile successful! Foclaor has successfully sung her requiem.
)

PAUSE