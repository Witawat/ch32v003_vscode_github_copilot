@echo off
title CH32V003 - Rebuild

set PROJECT_ROOT=%~dp0..
for %%I in ("%PROJECT_ROOT%") do set "PROJECT_ROOT=%%~fI"

echo.
echo =============================================================
echo   CH32V003 - Rebuild (Clean + Build)
echo =============================================================
echo.

:: Clean ก่อน
call "%PROJECT_ROOT%\scripts\clean.bat"

:: แล้ว Build
call "%PROJECT_ROOT%\scripts\build.bat"
