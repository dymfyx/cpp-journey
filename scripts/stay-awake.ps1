# stay-awake.ps1 - keep the PC awake (download-friendly) while the screen can turn off.
# ASCII only: Windows PowerShell 5.1 reads BOM-less .ps1 as ANSI.
# Log: stay-awake.log (same folder)

$ErrorActionPreference = 'Continue'
$root = $PSScriptRoot
$log  = Join-Path $root 'stay-awake.log'

function Log([string]$msg) {
    $line = "$(Get-Date -Format 'HH:mm:ss')  $msg"
    Write-Host $line
    $line | Out-File -FilePath $log -Append -Encoding utf8
}

$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
    [Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Start-Process powershell -Verb RunAs -ArgumentList @(
        '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$PSCommandPath`""
    ) | Out-Null
    Start-Sleep -Seconds 4
    exit
}

Log '=== keep-awake start ==='

Log 'never sleep on AC / battery:'
powercfg /change standby-timeout-ac 0 2>&1 | ForEach-Object { Log $_ }
powercfg /change hibernate-timeout-ac 0 2>&1 | ForEach-Object { Log $_ }
powercfg /change standby-timeout-dc 0 2>&1 | ForEach-Object { Log $_ }
powercfg /change hibernate-timeout-dc 0 2>&1 | ForEach-Object { Log $_ }

Log 'screen off after 1 min (AC / battery):'
powercfg /change monitor-timeout-ac 1 2>&1 | ForEach-Object { Log $_ }
powercfg /change monitor-timeout-dc 1 2>&1 | ForEach-Object { Log $_ }

Log 'lid close = do nothing (AC / battery):'
powercfg /setacvalueindex SCHEME_CURRENT SUB_BUTTONS LIDACTION 0 2>&1 | ForEach-Object { Log $_ }
powercfg /setdcvalueindex SCHEME_CURRENT SUB_BUTTONS LIDACTION 0 2>&1 | ForEach-Object { Log $_ }
powercfg /setactive SCHEME_CURRENT 2>&1 | ForEach-Object { Log $_ }

Log 'read back (SUB_SLEEP STANDBYIDLE):'
powercfg /query SCHEME_CURRENT SUB_SLEEP STANDBYIDLE 2>&1 | ForEach-Object { Log $_ }
Log 'read back (SUB_VIDEO VIDEOIDLE):'
powercfg /query SCHEME_CURRENT SUB_VIDEO VIDEOIDLE 2>&1 | ForEach-Object { Log $_ }

Log '=== done: PC will not sleep. Screen turns off after 1 min. ==='
Read-Host 'Press Enter to close this window'
