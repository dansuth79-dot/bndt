@echo off
setlocal
cd /d "%~dp0"
echo.
echo   bndt by Switchkicker - Windows build
echo   ------------------------------------
echo.

where cmake >nul 2>nul
if errorlevel 1 (
    echo CMake was not found. Install it from https://cmake.org or with Visual Studio's C++ CMake tools.
    pause
    exit /b 1
)

rem Use a local JUCE if JUCE_DIR is set, otherwise CMake downloads JUCE 8.0.4 the first time.
set JUCE_ARG=
if defined JUCE_DIR set JUCE_ARG=-DJUCE_DIR="%JUCE_DIR%"

cmake -S . -B build -G "Visual Studio 17 2022" -A x64 %JUCE_ARG%
if errorlevel 1 goto fail

cmake --build build --config Release --parallel
if errorlevel 1 goto fail

if not exist dist mkdir dist
if exist dist\bndt.vst3 rmdir /s /q dist\bndt.vst3
xcopy /E /I /Y /Q "build\bndt_artefacts\Release\VST3\bndt.vst3" "dist\bndt.vst3" >nul
copy /Y "build\bndt_artefacts\Release\Standalone\bndt.exe" "dist\bndt.exe" >nul

echo.
echo Built:  dist\bndt.vst3  and  dist\bndt.exe
echo.
echo Installing the VST3 to "%CommonProgramFiles%\VST3" ...
xcopy /E /I /Y /Q "dist\bndt.vst3" "%CommonProgramFiles%\VST3\bndt.vst3" >nul 2>nul
if errorlevel 1 (
    echo   Couldn't copy there - run build.bat as administrator, or copy dist\bndt.vst3 across by hand.
) else (
    echo   Done. Rescan plugins in your DAW.
)
echo.
pause
exit /b 0

:fail
echo.
echo Build failed - scroll up for the first error.
pause
exit /b 1
