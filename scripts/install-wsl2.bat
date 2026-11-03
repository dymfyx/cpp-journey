@echo off
chcp 65001 >nul
echo Starting WSL2 installer (a UAC prompt will appear, click YES)...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0install-wsl2.ps1"
echo.
echo Log file: %~dp0wsl-install.log
pause
