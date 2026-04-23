@echo off
REM ============================================================================
REM Ultimate CrackMe - Build Script
REM Requires: Visual Studio Developer Command Prompt
REM ============================================================================

echo.
echo  ██╗   ██╗██╗  ████████╗██╗███╗   ███╗ █████╗ ████████╗███████╗
echo  ██║   ██║██║  ╚══██╔══╝██║████╗ ████║██╔══██╗╚══██╔══╝██╔════╝
echo  ██║   ██║██║     ██║   ██║██╔████╔██║███████║   ██║   █████╗  
echo  ██║   ██║██║     ██║   ██║██║╚██╔╝██║██╔══██║   ██║   ██╔══╝  
echo  ╚██████╔╝███████╗██║   ██║██║ ╚═╝ ██║██║  ██║   ██║   ███████╗
echo   ╚═════╝ ╚══════╝╚═╝   ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝   ╚═╝   ╚══════╝
echo              ██████╗██████╗  █████╗  ██████╗██╗  ██╗███╗   ███╗███████╗
echo             ██╔════╝██╔══██╗██╔══██╗██╔════╝██║ ██╔╝████╗ ████║██╔════╝
echo             ██║     ██████╔╝███████║██║     █████╔╝ ██╔████╔██║█████╗  
echo             ██║     ██╔══██╗██╔══██║██║     ██╔═██╗ ██║╚██╔╝██║██╔══╝  
echo             ╚██████╗██║  ██║██║  ██║╚██████╗██║  ██╗██║ ╚═╝ ██║███████╗
echo              ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝╚═╝     ╚═╝╚══════╝
echo.
echo ============================================================================
echo                         Building Ultimate CrackMe...
echo ============================================================================
echo.

REM Check for cl.exe
where cl >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] cl.exe not found!
    echo Please run this script from Visual Studio Developer Command Prompt.
    echo.
    echo To open Developer Command Prompt:
    echo   1. Search for "Developer Command Prompt" in Start menu
    echo   2. Or run: "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    echo.
    pause
    exit /b 1
)

REM Create output directory
if not exist "bin" mkdir bin

echo [1/3] Compiling protection.cpp...
cl /c /EHsc /O2 /MT /W4 /GS- /D_CRT_SECURE_NO_WARNINGS /Fo"bin\protection.obj" src\protection.cpp
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile protection.cpp
    pause
    exit /b 1
)

echo [2/3] Compiling crypto.cpp...
cl /c /EHsc /O2 /MT /W4 /GS- /D_CRT_SECURE_NO_WARNINGS /Fo"bin\crypto.obj" src\crypto.cpp
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile crypto.cpp
    pause
    exit /b 1
)

echo [3/3] Compiling main.cpp and linking...
cl /EHsc /O2 /MT /W4 /GS- /D_CRT_SECURE_NO_WARNINGS ^
   /Fe"bin\UltimateCrackMe.exe" ^
   src\main.cpp bin\protection.obj bin\crypto.obj ^
   user32.lib gdi32.lib comctl32.lib advapi32.lib iphlpapi.lib ^
   /link /SUBSYSTEM:WINDOWS /DYNAMICBASE:NO

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Failed to compile and link
    pause
    exit /b 1
)

echo.
echo ============================================================================
echo                              BUILD SUCCESSFUL!
echo ============================================================================
echo.
echo Output: bin\UltimateCrackMe.exe
echo.
echo Protection Features:
echo   [+] Anti-Debug (8 techniques)
echo   [+] Anti-VM (4 checks)  
echo   [+] String Encryption
echo   [+] Opaque Predicates
echo   [+] Junk Code Insertion
echo   [+] CRC32 Integrity Check
echo   [+] 5-Layer Serial Validation
echo.
echo Valid Serial Format: XXXX-XXXX-XXXX-XXXX
echo Example: ULTM-CR4K-M3X1-2024
echo.
echo ============================================================================
echo.

REM Cleanup obj files
del bin\*.obj 2>nul

pause
