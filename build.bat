@echo off
echo ========================================
echo KompleteReaper Native - Build Script
echo ========================================
echo.

set "VSPATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
if exist "%VSPATH%" (
    call "%VSPATH%"
    goto :build
)

set "VSPATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
if exist "%VSPATH%" (
    call "%VSPATH%"
    goto :build
)

set "VSPATH=C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if exist "%VSPATH%" (
    call "%VSPATH%"
    goto :build
)

echo ERROR: Visual Studio not found
pause
exit /b 1

:build
echo Visual Studio initialized.

REM Clean build
if exist build rd /s /q build
mkdir build
cd build

echo Running CMake...
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% NEQ 0 goto :error

echo Building...
nmake
if %ERRORLEVEL% NEQ 0 goto :error

echo.
if exist reaper_kompletereaper.dll (
    echo Build successful! Copying to REAPER...
    copy /y reaper_kompletereaper.dll "%APPDATA%\REAPER\UserPlugins\"
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo ========================================
        echo INSTALLED SUCCESSFULLY!
        echo Restart REAPER to use the new driver.
        echo ========================================
    ) else (
        echo ERROR: Could not copy DLL. Is REAPER running?
    )
) else (
    echo ERROR: Compiled DLL not found in build folder.
)

cd ..
pause
exit /b 0

:error
echo.
echo ERROR: Build failed
cd ..
pause
exit /b 1
