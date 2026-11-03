# finish-wsl2.ps1 - run AFTER the reboot (features are enabled by then)
# Installs the WSL kernel (if missing) and downloads/registers Ubuntu 24.04.
# ASCII only: Windows PowerShell 5.1 reads BOM-less .ps1 as ANSI.
# Log: wsl-install.log (same folder)

$ErrorActionPreference = 'Continue'
$root = $PSScriptRoot
$log  = Join-Path $root 'wsl-install.log'

function Log([string]$msg) {
    $line = "$(Get-Date -Format 'HH:mm:ss')  $msg"
    Write-Host $line
    $line | Out-File -FilePath $log -Append -Encoding utf8
}

Log '==================== WSL2 finish (post-reboot) ===================='

$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
    [Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Log 'Not elevated. Requesting UAC elevation... (click YES in the popup)'
    try {
        Start-Process powershell -Verb RunAs -ArgumentList @(
            '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$PSCommandPath`""
        ) | Out-Null
        Log 'UAC request sent. A NEW window will open and do the real work; keep it open.'
    } catch {
        Log "Elevation failed: $($_.Exception.Message)"
    }
    Start-Sleep -Seconds 5
    exit
}

Log "Elevated OK. User: $env:USERNAME"

Log '--- 1/3 wsl --set-default-version 2 ---'
$out = & wsl.exe --set-default-version 2 2>&1
Log (($out | Out-String).Trim())
Log "exit code: $LASTEXITCODE"

Log '--- 2/3 wsl --install -d Ubuntu-24.04 --no-launch ---'
$out = & wsl.exe --install -d Ubuntu-24.04 --no-launch 2>&1
Log (($out | Out-String).Trim())
Log "exit code: $LASTEXITCODE"

if ($LASTEXITCODE -ne 0) {
    Log 'Retry with --web-download (bypass Microsoft Store) ...'
    $out = & wsl.exe --install -d Ubuntu-24.04 --web-download 2>&1
    Log (($out | Out-String).Trim())
    Log "retry exit code: $LASTEXITCODE"
}

Log '--- 3/3 wsl -l -v ---'
$out = & wsl.exe -l -v 2>&1
Log (($out | Out-String).Trim())

Log '==================== done ===================='
Log 'Next: open Ubuntu from the Start menu once, create your UNIX username, then run setup-ubuntu.sh.'
Read-Host 'Press Enter to close this window'
