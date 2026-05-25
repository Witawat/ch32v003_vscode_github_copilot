@echo off
setlocal enabledelayedexpansion
title CH32V003 - Build All SimpleHAL Examples

:: ============================================================
::  Build ALL SimpleHAL Example Files (compile-check only)
::  BUT every .c has its own main(), so compile-only -c check.
:: ============================================================
set PROJECT_ROOT=%~dp0..\..
for %%I in ("%PROJECT_ROOT%") do set "PROJECT_ROOT=%%~fI"

set TOOLCHAIN_BASE=C:\MounRiver\MounRiver_Studio2\resources\app\resources\win32\components\WCH\Toolchain

:: Select toolchain (2 = riscv-wch-elf- GCC12 recommended for CH32V003)
set TOOLCHAIN_CHOICE=2
if "%TOOLCHAIN_CHOICE%"=="2" (
    set TC_NAME=RISC-V Embedded GCC12
    set GCC_PREFIX=riscv-wch-elf-
) else (
    set TC_NAME=RISC-V Embedded GCC
    set GCC_PREFIX=riscv-none-embed-
)

set TC_BIN=%TOOLCHAIN_BASE%\%TC_NAME%\bin
set GCC="%TC_BIN%\%GCC_PREFIX%gcc.exe"

set ARCH=-march=rv32ecxw -mabi=ilp32e
set CFLAGS=%ARCH% -msmall-data-limit=0 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g
set INCLUDES=-I"%PROJECT_ROOT%\Debug" -I"%PROJECT_ROOT%\Core" -I"%PROJECT_ROOT%\User" -I"%PROJECT_ROOT%\Peripheral\inc" -I"%PROJECT_ROOT%\User\SimpleHAL"

:: Check compiler
if not exist "%TC_BIN%\%GCC_PREFIX%gcc.exe" (
    echo [ERROR] Compiler not found: %TC_BIN%\%GCC_PREFIX%gcc.exe
    echo.
    echo  Check TOOLCHAIN_CHOICE or toolchain path in this script.
    pause
    exit /b 1
)
echo [OK] Compiler: %TC_BIN%\%GCC_PREFIX%gcc.exe
echo.

set EXAMPLES_DIR=%~dp0
set TOTAL=0
set PASS=0
set FAIL=0

echo ============================================================
echo  Building all SimpleHAL examples (compile-check only)
echo  PASS = no syntax/type errors
echo  FAIL = compiler error (check output above)
echo ============================================================
echo.

for /r "%EXAMPLES_DIR%" %%F in (*.c) do (
    set /a TOTAL+=1
    set "FNAME=%%~nxF"
    set "FDIR=%%~dpF"
    for %%D in ("!FDIR:~0,-1!") do set "FDIRNAME=%%~nxD"
    
    echo [!TOTAL!] !FDIRNAME!/!FNAME!
    %GCC% %CFLAGS% %INCLUDES% -c "%%F" -o nul 2>&1
    if !errorlevel! equ 0 (
        echo   [PASS]
        set /a PASS+=1
    ) else (
        echo   [FAIL]
        set /a FAIL+=1
    )
    echo.
)

echo ============================================================
echo  Results: !PASS!/!TOTAL! passed, !FAIL! failed
echo ============================================================
if !FAIL! gtr 0 (
    echo  Some files have build errors - check output above.
) else (
    echo  All files compiled successfully!
)
echo.

endlocal
exit /b %FAIL%
