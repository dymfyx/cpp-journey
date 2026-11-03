# restore-power.ps1 - restore normal power settings after the WSL download finishes.
# Restores: screen off after 5 min (AC) / 3 min (DC), sleep after 15 min (AC) / 10 min (DC),
#           lid close -> hibernate.
# To use a different lid action, change $LIDVALUE: 2 = hibernate, 1 = sleep, 0 = do nothing.
$ErrorActionPreference = 'Continue'
$root = $PSScriptRoot
$log = Join-Path $root 'restore-power.log'
function Log([string]$m) {
    $line = "[{0}] {1}" -f (Get-Date -Format 'yyyy-MM-dd HH:mm:ss'), $m
    Add-Content -Path $log -Value $line -Encoding UTF8
}
Log '==== restore-power start ===='
try {
    $p = [Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()
    if (-not $p.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
        Log 'not elevated -> relaunch via UAC'
        Start-Process powershell.exe -Verb RunAs -ArgumentList @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', ('"' + $PSCommandPath + '"'))
        Start-Sleep -Seconds 4
        exit
    }
} catch {
    Log ('elevation error: ' + $_.Exception.Message)
}
Log 'elevated OK'
$LIDVALUE = '2'
Log ('lid action target = ' + $LIDVALUE + '  (2=hibernate, 1=sleep, 0=nothing)')
powercfg /change monitor-timeout-ac 5
powercfg /change monitor-timeout-dc 3
powercfg /change standby-timeout-ac 15
powercfg /change standby-timeout-dc 10
powercfg /setacvalueindex SCHEME_CURRENT SUB_BUTTONS LIDACTION $LIDVALUE
powercfg /setdcvalueindex SCHEME_CURRENT SUB_BUTTONS LIDACTION $LIDVALUE
powercfg /setactive SCHEME_CURRENT
Log 'applied: monitor 5/3 min, sleep 15/10 min, lid=hibernate'
Log '---- verify (setting index lines) ----'
$q1 = powercfg /query SCHEME_CURRENT SUB_SLEEP STANDBYIDLE 2>&1 | Select-String '0x'
$q2 = powercfg /query SCHEME_CURRENT SUB_VIDEO VIDEOIDLE 2>&1 | Select-String '0x'
$q3 = powercfg /query SCHEME_CURRENT SUB_BUTTONS LIDACTION 2>&1 | Select-String '0x'
foreach ($l in $q1) { Log ('sleep: ' + $l.Line) }
foreach ($l in $q2) { Log ('screen: ' + $l.Line) }
foreach ($l in $q3) { Log ('lid: ' + $l.Line) }
Log '==== done ===='
Read-Host 'Done. Press Enter to close'
