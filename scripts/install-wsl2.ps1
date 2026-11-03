# install-wsl2.ps1 - enable WSL2 + VirtualMachinePlatform, install WSL kernel, download Ubuntu 24.04
# ASCII only on purpose: Windows PowerShell 5.1 reads BOM-less .ps1 files as ANSI (GBK here),
# and non-ASCII characters break string parsing.
# Log: wsl-install.log (same folder)

$ErrorActionPreference = 'Continue'
$root = $PSScriptRoot
$log  = Join-Path $root 'wsl-install.log'

function Log([string]$msg) {
    $line = "$(Get-Date -Format 'HH:mm:ss')  $msg"
    Write-Host $line
    $line | Out-File -FilePath $log -Append -Encoding utf8
}

Log '==================== WSL2 install start ===================='

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

Log '--- 1/4 enable Microsoft-Windows-Subsystem-Linux ---'
$out = & dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart 2>&1
Log (($out | Out-String).Trim())
Log "dism exit code: $LASTEXITCODE"

Log '--- 2/4 enable VirtualMachinePlatform ---'
$out = & dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart 2>&1
Log (($out | Out-String).Trim())
Log "dism exit code: $LASTEXITCODE"

Log '--- 3/4 wsl --update ---'
$out = & wsl.exe --update 2>&1
Log (($out | Out-String).Trim())
Log "wsl --update exit code: $LASTEXITCODE"

Log '--- 4/4 default version 2 + Ubuntu 24.04 ---'
$out = & wsl.exe --set-default-version 2 2>&1
Log (($out | Out-String).Trim())
Log "wsl --set-default-version exit code: $LASTEXITCODE"

$out = & wsl.exe --install -d Ubuntu-24.04 --no-launch 2>&1
Log (($out | Out-String).Trim())
Log "wsl --install exit code: $LASTEXITCODE"

if ($LASTEXITCODE -ne 0) {
    Log 'Retry with --web-download ...'
    $out = & wsl.exe --install -d Ubuntu-24.04 --web-download 2>&1
    Log (($out | Out-String).Trim())
    Log "retry exit code: $LASTEXITCODE"
}

Log '==================== done ===================='
Log 'If a reboot is required: reboot the PC, then open Ubuntu from the Start menu once.'
Read-Host 'Press Enter to close this window'
