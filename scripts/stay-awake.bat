@echo off
chcp 65001 >nul
echo Setting power options (no sleep, screen off after 1 min). A UAC prompt will appear, click YES...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0stay-awake.ps1"
echo.
echo Log file: %~dp0stay-awake.log
pause
