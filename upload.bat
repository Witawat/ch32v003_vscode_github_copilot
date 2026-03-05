@echo off
setlocal EnableDelayedExpansion

:: ============================================================
::  upload.bat - Flash CH32V003 via WCH-Link using OpenOCD
:: ============================================================

set OPENOCD_DIR=C:\MounRiver\MounRiver_Studio2\resources\app\resources\win32\components\WCH\OpenOCD\OpenOCD\bin
set OPENOCD=%OPENOCD_DIR%\openocd.exe
set OPENOCD_CFG=%OPENOCD_DIR%\wch-riscv.cfg

set WORKSPACE=%~dp0
set ELF_FILE=%WORKSPACE%output\CH32V003.elf

echo ============================================================
echo  CH32V003 Upload via WCH-Link
echo ============================================================

:: Check that OpenOCD exists
if not exist "%OPENOCD%" (
    echo [ERROR] OpenOCD not found: %OPENOCD%
    exit /b 1
)

:: Check that firmware exists
if not exist "%ELF_FILE%" (
    echo [ERROR] Firmware not found: %ELF_FILE%
    echo         Run build.bat first.
    exit /b 1
)

echo OpenOCD : %OPENOCD%
echo Config  : %OPENOCD_CFG%
echo Firmware: %ELF_FILE%
echo.
echo Connecting to WCH-Link...
echo.

:: Convert backslashes to forward slashes for OpenOCD
set "ELF_FWD=%ELF_FILE:\=/%"

"%OPENOCD%" -f "%OPENOCD_CFG%" -c "program {%ELF_FWD%} verify reset exit"

if %ERRORLEVEL% equ 0 (
    echo.
    echo ============================================================
    echo  Upload successful! Device is running.
    echo ============================================================
) else (
    echo.
    echo ============================================================
    echo  [ERROR] Upload failed! (Exit code: %ERRORLEVEL%)
    echo  - Check that WCH-Link is connected
    echo  - Check that the device is powered
    echo ============================================================
    exit /b 1
)

endlocal
