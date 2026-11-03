@echo off
chcp 65001 >nul
echo Finishing WSL2 setup after reboot (a UAC prompt will appear, click YES)...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0finish-wsl2.ps1"
echo.
echo Log file: %~dp0wsl-install.log
pause
