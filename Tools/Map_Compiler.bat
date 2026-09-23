@echo off
setlocal

chcp 65001 >nul
title WRECK.EXE // MAP COMPILER v1.0

@REM Set terminal background to black, text to standard gray/white
chcp 65001 >nul
title WRECK.EXE // MAP COMPILER v1.0

@REM  Set the whole window to dark background with bright red text (Retro Doom Style)
color 0C
cls

echo  ======================================================
echo   ██     ██  ██████  ███████  ██████ ██   ██ 
echo   ██     ██ ██    ██ ██      ██      ██  ██  
echo   ██  █  ██ ██████   █████   ██      █████   
echo   ██ ███ ██ ██    ██ ██      ██      ██  ██  
echo    ███ ███  ██    ██ ███████  ██████ ██   ██ 
echo  ======================================================
echo   [SYSTEM] WRECK ENGINE GEOMETRY COMPILER v1.0
echo   [TARGET] TRENCHBROOM VALVE 220 MAP PIPELINE
echo  ======================================================
echo.

@REM Get the directory where this script lives, and point it to the compiler.exe
set COMPILER=%~dp0..\Build\output\Wreck-Compiler.exe

@REM  Check if the compiler exists
if not exist "%COMPILER%" (
    echo [ERROR] Couldn't find Wreck-Compiler.exe at:
    echo %COMPILER%
    echo Check your relative path in the batch file!
    echo .
    pause
    exit /b 1
)

@REM  Check if a map file was dragged / dropped or passed as an argument
if "%~1"=="" (
    echo [ERROR] No input map file provided!
    echo Drag and drop a .map file onto this script to compile it.
    echo .
    pause
    exit /b 1
) 

echo =======================================================
echo Compiling: %~nx1
echo =======================================================

@REM  Run the compiler 
%COMPILER%  "%~1" "%~dpn1.wdf"

if %ERRORLEVEL% neq 0 (
    echo .
    echo [ERROR] Compilation failed!
    echo .
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo =======================================================
echo Build complete! Output saved as: 
echo %~dpn1.wdf
echo =======================================================
pause