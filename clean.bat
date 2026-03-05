@echo off
setlocal
title CH32V003 - Clean

set PROJECT_ROOT=%~dp0
set PROJECT_ROOT=%PROJECT_ROOT:~0,-1%
set OBJ_DIR=%PROJECT_ROOT%\obj
set OUT_DIR=%PROJECT_ROOT%\output

echo.
echo =============================================================
echo   CH32V003 - Clean
echo =============================================================
echo.

if exist "%OBJ_DIR%" (
    echo Removing: obj\
    rmdir /s /q "%OBJ_DIR%"
    echo [OK] obj\ removed
) else (
    echo [OK] obj\ already clean
)

if exist "%OUT_DIR%" (
    echo Removing: output\
    rmdir /s /q "%OUT_DIR%"
    echo [OK] output\ removed
) else (
    echo [OK] output\ already clean
)

echo.
echo =============================================================
echo   [OK] Done
echo =============================================================
echo.
