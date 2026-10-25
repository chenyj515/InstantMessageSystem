@echo off
chcp 65001 >nul
setlocal
set "IMS_EXE="
if exist "%~dp0out\build\x64-Debug\Debug\InstantMessageSystem.exe" set "IMS_EXE=%~dp0out\build\x64-Debug\Debug\InstantMessageSystem.exe"
if exist "%~dp0out\build\x64-Debug\InstantMessageSystem.exe" set "IMS_EXE=%~dp0out\build\x64-Debug\InstantMessageSystem.exe"
if exist "%~dp0build\Debug\InstantMessageSystem.exe" set "IMS_EXE=%~dp0build\Debug\InstantMessageSystem.exe"
if exist "%~dp0build\Release\InstantMessageSystem.exe" set "IMS_EXE=%~dp0build\Release\InstantMessageSystem.exe"
if exist "%~dp0build\InstantMessageSystem.exe" set "IMS_EXE=%~dp0build\InstantMessageSystem.exe"
if "%IMS_EXE%"=="" (
  echo Executable not found. Please build with CMake/VS first.
  echo Example: cmake -S . -B out\build\x64-Debug ^&^& cmake --build out\build\x64-Debug
  pause
  exit /b 1
)
"%IMS_EXE%" "%~dp0data"
set "IMS_RESULT=%ERRORLEVEL%"
pause
exit /b %IMS_RESULT%
