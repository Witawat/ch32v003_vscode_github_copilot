@echo off
title CH32V003 - Rebuild

set PROJECT_ROOT=%~dp0
set PROJECT_ROOT=%PROJECT_ROOT:~0,-1%

echo.
echo =============================================================
echo   CH32V003 - Rebuild (Clean + Build)
echo =============================================================
echo.

:: Clean ก่อน
call "%PROJECT_ROOT%\clean.bat"

:: แล้ว Build
call "%PROJECT_ROOT%\build.bat"
