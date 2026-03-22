@echo off
setlocal EnableExtensions DisableDelayedExpansion

rem Generate .vscode/tasks.json, .vscode/launch.json and .vscode/c_cpp_properties.json
rem Usage:
rem   gen_vscode_files.bat [target_project_dir] [elf_name] [mcu_name] [mounriver_root]
rem Example:
rem   gen_vscode_files.bat "D:\my_project" "my_fw.elf" "CH32V003" "C:/MounRiver/MounRiver_Studio2"

set "TARGET_DIR=%~1"
if "%TARGET_DIR%"=="" set "TARGET_DIR=%CD%"

if not exist "%TARGET_DIR%" (
  echo [ERROR] Target directory not found: "%TARGET_DIR%"
  exit /b 1
)

set "MCU_NAME=%~3"
if "%MCU_NAME%"=="" set "MCU_NAME=CH32V003"

set "ELF_NAME=%~2"
if "%ELF_NAME%"=="" set "ELF_NAME=%MCU_NAME%.elf"

set "MOUNRIVER_ROOT=%~4"
if "%MOUNRIVER_ROOT%"=="" set "MOUNRIVER_ROOT=C:/MounRiver/MounRiver_Studio2"

set "TOOLCHAIN_BIN=%MOUNRIVER_ROOT%/resources/app/resources/win32/components/WCH/Toolchain/RISC-V Embedded GCC/bin"
set "VSCODE_DIR=%TARGET_DIR%\.vscode"

if not exist "%VSCODE_DIR%" (
  mkdir "%VSCODE_DIR%" || (
    echo [ERROR] Cannot create: "%VSCODE_DIR%"
    exit /b 1
  )
)

call :write_tasks > "%VSCODE_DIR%\tasks.json" || goto :write_failed
call :write_launch > "%VSCODE_DIR%\launch.json" || goto :write_failed
call :write_cpp_properties > "%VSCODE_DIR%\c_cpp_properties.json" || goto :write_failed

echo [OK] Generated VS Code config files:
echo      %VSCODE_DIR%\tasks.json
echo      %VSCODE_DIR%\launch.json
echo      %VSCODE_DIR%\c_cpp_properties.json
echo [INFO] MCU  : %MCU_NAME%
echo [INFO] ELF  : %ELF_NAME%
echo [INFO] Tool : %TOOLCHAIN_BIN%
exit /b 0

:write_failed
echo [ERROR] Failed to write one or more VS Code files.
exit /b 1

:write_tasks
echo {
echo   "version": "2.0.0",
echo   "tasks": [
echo     {
echo       "label": "Build %MCU_NAME% (MounRiver)",
echo       "type": "shell",
echo       "command": "cmd",
echo       "args": [
echo         "/c",
echo         "${workspaceFolder}\\build.bat"
echo       ],
echo       "problemMatcher": {
echo         "owner": "cpp",
echo         "fileLocation": ["relative", "${workspaceFolder}"],
echo         "pattern": {
echo           "regexp": "^(.*):(\\d+):(\\d+): (warning|error): (.*)$",
echo           "file": 1,
echo           "line": 2,
echo           "column": 3,
echo           "severity": 4,
echo           "message": 5
echo         }
echo       },
echo       "group": {
echo         "kind": "build",
echo         "isDefault": true
echo       },
echo       "presentation": {
echo         "echo": true,
echo         "reveal": "always",
echo         "panel": "new"
echo       }
echo     },
echo     {
echo       "label": "Clean %MCU_NAME%",
echo       "type": "shell",
echo       "command": "cmd",
echo       "args": [
echo         "/c",
echo         "${workspaceFolder}\\clean.bat"
echo       ],
echo       "presentation": {
echo         "echo": true,
echo         "reveal": "always",
echo         "panel": "same"
echo       }
echo     },
echo     {
echo       "label": "Rebuild %MCU_NAME%",
echo       "type": "shell",
echo       "command": "cmd",
echo       "args": [
echo         "/c",
echo         "${workspaceFolder}\\rebuild.bat"
echo       ],
echo       "presentation": {
echo         "echo": true,
echo         "reveal": "always",
echo         "panel": "same"
echo       }
echo     },
echo     {
echo       "label": "Upload %MCU_NAME% (WCH-Link)",
echo       "type": "shell",
echo       "command": "cmd",
echo       "args": [
echo         "/c",
echo         "${workspaceFolder}\\upload.bat"
echo       ],
echo       "presentation": {
echo         "echo": true,
echo         "reveal": "always",
echo         "panel": "new"
echo       }
echo     }
echo   ]
echo }
exit /b 0

:write_launch
echo {
echo   "version": "0.2.0",
echo   "configurations": [
echo     {
echo       "name": "%MCU_NAME% Debug",
echo       "type": "cppdbg",
echo       "request": "launch",
echo       "program": "${workspaceFolder}/obj/%ELF_NAME%",
echo       "args": [],
echo       "stopAtEntry": false,
echo       "cwd": "${workspaceFolder}",
echo       "environment": [
echo         {
echo           "name": "PATH",
echo           "value": "%TOOLCHAIN_BIN%;${env:PATH}"
echo         }
echo       ],
echo       "externalConsole": true,
echo       "MIMode": "gdb",
echo       "miDebuggerPath": "%TOOLCHAIN_BIN%/riscv-none-embed-gdb.exe",
echo       "setupCommands": [
echo         {
echo           "description": "Set architecture",
echo           "text": "set architecture riscv:rv32",
echo           "ignoreFailures": false
echo         },
echo         {
echo           "description": "Configure debugging",
echo           "text": "set mem inaccessible-by-default off",
echo           "ignoreFailures": false
echo         },
echo         {
echo           "description": "Set disassembler options",
echo           "text": "set disassembler-options xw",
echo           "ignoreFailures": false
echo         }
echo       ],
echo       "preLaunchTask": "Build %MCU_NAME% (MounRiver)",
echo       "presentation": {
echo         "hidden": false,
echo         "group": "",
echo         "order": 1
echo       }
echo     }
echo   ]
echo }
exit /b 0

:write_cpp_properties
setlocal EnableDelayedExpansion
echo {
echo   "configurations": [
echo     {
echo       "name": "MounRiver Studio Build",
echo       "includePath": [
set "FIRST_INCLUDE=1"
for %%R in ("Debug" "Core" "User" "Peripheral\inc" "Peripheral\src" "Hardware\inc" "Hardware\src" "Lib" "Startup") do (
  if exist "%TARGET_DIR%\%%~R" (
    set "INC_PATH=%%~R"
    set "INC_PATH=!INC_PATH:\=/!"
    if "!FIRST_INCLUDE!"=="1" (
      echo         "${workspaceFolder}/!INC_PATH!"
      set "FIRST_INCLUDE=0"
    ) else (
      echo         ,"${workspaceFolder}/!INC_PATH!"
    )
  )
)
if "!FIRST_INCLUDE!"=="1" (
  echo         "${workspaceFolder}"
)
echo       ],
echo       "defines": [
echo         "_DEBUG",
echo         "UNICODE",
echo         "_UNICODE"
echo       ],
echo       "intelliSenseMode": "linux-gcc-arm",
echo       "compilerPath": "%TOOLCHAIN_BIN%/riscv-none-embed-gcc.exe",
echo       "cStandard": "gnu99",
echo       "cppStandard": "gnu++11"
echo     }
echo   ],
echo   "version": 4
echo }
endlocal
exit /b 0