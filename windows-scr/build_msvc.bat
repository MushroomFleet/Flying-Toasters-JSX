@echo off
REM Flying Toasters Screensaver - MSVC Build Script
REM 
REM Run this from a Visual Studio Developer Command Prompt
REM or after running vcvarsall.bat
REM
REM Usage:
REM   build_msvc.bat          - Build the screensaver
REM   build_msvc.bat clean    - Remove build artifacts
REM   build_msvc.bat install  - Copy to Windows directory (admin required)

setlocal

set TARGET=flying_toasters.scr
set SOURCE=flying_toasters.c
set RESOURCE=flying_toasters.rc

if "%1"=="clean" goto clean
if "%1"=="install" goto install

REM Check for cl.exe
where cl >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo Error: cl.exe not found.
    echo Please run this from a Visual Studio Developer Command Prompt
    echo or run vcvarsall.bat first.
    exit /b 1
)

echo Building Flying Toasters Screensaver...

REM Compile resources
echo Compiling resources...
rc /nologo /fo flying_toasters.res %RESOURCE%
if %ERRORLEVEL% neq 0 (
    echo Resource compilation failed.
    exit /b 1
)

REM Compile and link
echo Compiling and linking...
cl /nologo /O2 /W3 /DUNICODE /D_UNICODE %SOURCE% flying_toasters.res ^
   /link /OUT:%TARGET% ^
   user32.lib gdi32.lib advapi32.lib comctl32.lib scrnsavw.lib

if %ERRORLEVEL% neq 0 (
    echo Compilation failed.
    exit /b 1
)

echo.
echo Build successful: %TARGET%
echo.
echo To test: %TARGET% /s
echo To configure: %TARGET% /c
echo To install: Run this script with 'install' argument as Administrator
goto end

:clean
echo Cleaning build artifacts...
del /q *.obj *.res %TARGET% 2>nul
echo Done.
goto end

:install
echo Installing screensaver...
if not exist %TARGET% (
    echo Error: %TARGET% not found. Build first.
    exit /b 1
)
copy /y %TARGET% "%SYSTEMROOT%\System32\%TARGET%"
if %ERRORLEVEL% neq 0 (
    echo Installation failed. Run as Administrator.
    exit /b 1
)
echo.
echo Installed successfully!
echo Open Settings ^> Personalization ^> Lock screen ^> Screen saver settings
echo and select "Flying Toasters - SVGA Wireframe"
goto end

:end
endlocal
