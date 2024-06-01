@echo off

mkdir bin
cd bin
cmake .. -DGXT2_ENABLE_UNITY_BUILD=ON

for /f "usebackq tokens=*" %%i in (`vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
  set InstallDir=%%i
)

if exist "%InstallDir%\Common7\Tools\vsdevcmd.bat" (
  call "%InstallDir%\Common7\Tools\vsdevcmd.bat"
)

devenv gxt2.sln /rebuild Debug
devenv gxt2.sln /rebuild Release

if %ERRORLEVEL% NEQ 0 (
  pause
)