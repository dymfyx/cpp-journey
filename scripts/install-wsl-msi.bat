@echo off
rem install-wsl-msi.bat - install the downloaded WSL 3.0.3 MSI with elevation.
rem Run AFTER scripts\downloads\wsl.3.0.3.0.x64.msi exists.
setlocal
pushd "%~dp0"
if not exist "%~dp0downloads\wsl.3.0.3.0.x64.msi" (
  echo ERROR: MSI not found at downloads\wsl.3.0.3.0.x64.msi
  pause
  exit /b 1
)
echo Installing WSL 3.0.3 from MSI ... (click YES in the UAC popup)
powershell.exe -NoProfile -ExecutionPolicy Bypass -Command "Start-Process msiexec.exe -Verb RunAs -ArgumentList '/i','\"%~dp0downloads\wsl.3.0.3.0.x64.msi\"','/passive','/norestart','/l*v','\"%~dp0wsl-msi-install.log\"'"
popd
echo.
echo Installer launched. Wait for it to finish, then run: wsl --version
pause
