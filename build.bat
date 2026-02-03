@echo off

REM Setup Visual Studio environment
REM Adjust the path below to match your Visual Studio installation
call "C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars64.bat" x64

REM Create build directory if it doesn't exist
if not exist build mkdir build

REM Clean previous build artifacts
del /Q build\* 2>nul

REM Change to build directory
pushd build

echo Compiling main.cpp...
echo.

REM Compile with output to build folder
REM -Zi: Generate debug info
REM -FC: Full path in diagnostics
REM -nologo: Suppress compiler banner
REM -Fe: Specify executable name
cl -Zi -FC -DUNICODE -D_UNICODE -Fe:main.exe ..\main.cpp user32.lib gdi32.lib

echo.

REM Check if compilation succeeded
if exist main.exe (
    echo Build succeeded!
    echo.
    echo Files created:
    dir /B
) else (
    echo Build failed! Check errors above.
)

REM Return to original directory
popd

pause