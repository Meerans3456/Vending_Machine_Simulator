@echo off
rem build.bat - assemble one program with JWasm into a DOS .exe
rem Usage:  build vending        (also: build t_utils, t_input, t_math)
cd /d "%~dp0"
if "%~1"=="" (
    echo Usage: build ^<name^>   e.g. build vending
    exit /b 1
)
set "SRC="
if exist "src\%~1.asm" set "SRC=src\%~1.asm"
if exist "tests\%~1.asm" set "SRC=tests\%~1.asm"
if not defined SRC (
    echo Cannot find %~1.asm in src\ or tests\
    exit /b 1
)
"%~dp0tools\JWasm\JWasm.exe" -0 -mz -Iinc -Fo %~1.exe %SRC%