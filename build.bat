@echo off
setlocal enabledelayedexpansion
title CH32V003 - Build

:: ============================================================
::  Select Toolchain
::    1 = RISC-V Embedded GCC   (GCC 8.2.0)  riscv-none-embed-
::    2 = RISC-V Embedded GCC12 (GCC 12.2.0) riscv-wch-elf-  <-- Recommended for CH32V003
:: ============================================================
set TOOLCHAIN_CHOICE=2

:: ============================================================
set PROJECT_ROOT=%~dp0
set PROJECT_ROOT=%PROJECT_ROOT:~0,-1%
set OBJ_DIR=%PROJECT_ROOT%\obj
set OUT_DIR=%PROJECT_ROOT%\output
set TOOLCHAIN_BASE=C:\MounRiver\MounRiver_Studio2\resources\app\resources\win32\components\WCH\Toolchain

if "%TOOLCHAIN_CHOICE%"=="2" (
    set TC_NAME=RISC-V Embedded GCC12
    set GCC_PREFIX=riscv-wch-elf-
) else (
    set TC_NAME=RISC-V Embedded GCC
    set GCC_PREFIX=riscv-none-embed-
)

set TC_BIN=%TOOLCHAIN_BASE%\%TC_NAME%\bin
set GCC="%TC_BIN%\%GCC_PREFIX%gcc.exe"
set OBJCOPY="%TC_BIN%\%GCC_PREFIX%objcopy.exe"
set SIZE="%TC_BIN%\%GCC_PREFIX%size.exe"

set ARCH=-march=rv32ecxw -mabi=ilp32e
set CFLAGS=%ARCH% -msmall-data-limit=0 -msave-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g
set INCLUDES=-I"%PROJECT_ROOT%\Debug" -I"%PROJECT_ROOT%\Core" -I"%PROJECT_ROOT%\User" -I"%PROJECT_ROOT%\Peripheral\inc" -I"%PROJECT_ROOT%\User\SimpleHAL"
set LDFLAGS=%ARCH% -T"%PROJECT_ROOT%\Ld\Link.ld" -nostartfiles -Xlinker --gc-sections -Wl,-Map="%OUT_DIR%\CH32V003.map" --specs=nano.specs --specs=nosys.specs -lprintf

echo.
echo =============================================================
echo   CH32V003 - Build  [%GCC_PREFIX%gcc]
echo =============================================================
echo.

:: ตรวจ compiler
if not exist "%TC_BIN%\%GCC_PREFIX%gcc.exe" (
    echo [ERROR] Compiler not found: %TC_BIN%\%GCC_PREFIX%gcc.exe
    echo.
    echo  Check TOOLCHAIN_CHOICE in build.bat
    goto :BUILD_FAIL
)
echo [OK] Compiler: %TC_BIN%\%GCC_PREFIX%gcc.exe
echo.

:: Create obj and output directories
if not exist "%OBJ_DIR%" mkdir "%OBJ_DIR%"
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

:: Create empty response file (use forward slashes for GCC)
echo.> "%OBJ_DIR%\objects.rsp"
set "OBJ_FWD=%OBJ_DIR:\=/%"

set ERR=0

echo --- Compiling C files ---

:: Core
call :CC "%PROJECT_ROOT%\Core\core_riscv.c"                  "core_riscv.o"

:: Debug
call :CC "%PROJECT_ROOT%\Debug\debug.c"                       "debug.o"

:: Peripheral/src
for %%F in ("%PROJECT_ROOT%\Peripheral\src\*.c") do (
    call :CC "%%F" "%%~nF.o"
)

:: User
call :CC "%PROJECT_ROOT%\User\main.c"                         "main.o"
call :CC "%PROJECT_ROOT%\User\system_ch32v00x.c"              "system_ch32v00x.o"
call :CC "%PROJECT_ROOT%\User\ch32v00x_it.c"                  "ch32v00x_it.o"

:: User/SimpleHAL
for %%F in ("%PROJECT_ROOT%\User\SimpleHAL\*.c") do (
    call :CC "%%F" "%%~nF.o"
)

:: User/Lib (recursive)
for /r "%PROJECT_ROOT%\User\Lib" %%F in (*.c) do (
    call :CC "%%F" "lib_%%~nF.o"
)

:: Assembly
echo   [ASM] Startup\startup_ch32v00x.S
%GCC% %CFLAGS% -c "%PROJECT_ROOT%\Startup\startup_ch32v00x.S" -o "%OBJ_DIR%\startup_ch32v00x.o" 2>&1
if errorlevel 1 (
    echo   [ERROR] Startup assembly failed
    set ERR=1
) else (
    echo "%OBJ_FWD%/startup_ch32v00x.o">> "%OBJ_DIR%\objects.rsp"
)

if %ERR%==1 goto :BUILD_FAIL

echo.
echo --- Linking ---
%GCC% @"%OBJ_DIR%\objects.rsp" %LDFLAGS% -o "%OUT_DIR%\CH32V003.elf" 2>&1
if errorlevel 1 (
    echo [ERROR] Linking failed
    goto :BUILD_FAIL
)
echo [OK] output\CH32V003.elf

echo.
echo --- Creating HEX ---
%OBJCOPY% -O ihex "%OUT_DIR%\CH32V003.elf" "%OUT_DIR%\CH32V003.hex" 2>&1
if errorlevel 1 (
    echo [WARNING] HEX creation failed
) else (
    echo [OK] output\CH32V003.hex
)

echo.
echo --- Firmware Size ---
%SIZE% --format=berkeley "%OUT_DIR%\CH32V003.elf"

echo.
echo --- Memory Usage (CH32V003: Flash=16K  RAM=2K) ---
%SIZE% --format=berkeley "%OUT_DIR%\CH32V003.elf" > "%OBJ_DIR%\size.tmp"
for /f "skip=1 tokens=1,2,3" %%a in (%OBJ_DIR%\size.tmp) do (
    set /a TEXT=%%a
    set /a DATA=%%b
    set /a BSS=%%c
    set /a FLASH_USED=%%a+%%b
    set /a RAM_USED=%%b+%%c
)
set /a FLASH_TOTAL=16384
set /a RAM_TOTAL=2048
set /a FLASH_FREE=FLASH_TOTAL-FLASH_USED
set /a RAM_FREE=RAM_TOTAL-RAM_USED
set /a FLASH_PCT=FLASH_USED*100/FLASH_TOTAL
set /a RAM_PCT=RAM_USED*100/RAM_TOTAL
del "%OBJ_DIR%\size.tmp" 2>nul

echo.
echo   Flash:  %FLASH_USED% / %FLASH_TOTAL% bytes  (%FLASH_PCT%%%)  [Free: %FLASH_FREE% bytes]
echo   RAM:    %RAM_USED% / %RAM_TOTAL% bytes  (%RAM_PCT%%%)  [Free: %RAM_FREE% bytes]
echo.

echo.
echo =============================================================
echo   [OK] Build successful!
echo.
echo   Intermediate : obj\  (*.o)
echo   Final output : output\CH32V003.elf
echo                  output\CH32V003.hex
echo                  output\CH32V003.map
echo =============================================================
echo.
goto :EOF


:CC
    echo   [C] %~nx1
    %GCC% %CFLAGS% %INCLUDES% -c %1 -o "%OBJ_DIR%\%~2" 2>&1
    if errorlevel 1 (
        echo   [ERROR] Compile failed: %~nx1
        set ERR=1
        exit /b 1
    )
    echo "%OBJ_FWD%/%~2">> "%OBJ_DIR%\objects.rsp"
    exit /b 0


:BUILD_FAIL
    echo.
    echo =============================================================
    echo   [ERROR] Build failed!
    echo =============================================================
    echo.
    exit /b 1
