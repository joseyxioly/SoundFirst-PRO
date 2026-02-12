@echo off
echo ========================================
echo FX Mapper - Building Standalone EXE
echo ========================================
echo.

REM Check if PyInstaller is installed
python -m pip show pyinstaller >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo PyInstaller not found. Installing...
    python -m pip install pyinstaller
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: Failed to install PyInstaller
        pause
        exit /b 1
    )
)

echo Building FX Mapper executable...
pyinstaller --onefile --windowed --name "SoundFirst_PRO_Mapper" --icon=logo.png fx_mapper_gen.py

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build successful!
echo Executable: dist\SoundFirst_PRO_Mapper.exe
echo ========================================
echo.
echo You can now distribute dist\SoundFirst_PRO_Mapper.exe as a standalone tool.
pause
