@echo off
rem build.bat - assemble one program with JWasm into a DOS .exe
rem Usage:  build <name>        e.g. build vending, build t_math
rem Looks for <name>.asm in src\ and then in tests\; writes <name>.exe to this folder.
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
if not exist "tools\JWasm\JWasm.exe" (
    echo JWasm.exe not found at tools\JWasm\JWasm.exe - see Setup.md
    exit /b 1
)
"tools\JWasm\JWasm.exe" -0 -mz -Iinc -Fo %~1.exe %SRC%
