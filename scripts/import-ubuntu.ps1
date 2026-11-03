# import-ubuntu.ps1 (v3) - import Ubuntu 24.04 from the official ubuntu-base rootfs.
# Run in a NORMAL terminal (no admin needed). ASCII only.
# Requires: scripts\downloads\ubuntu-base-24.04.5-base-amd64.tar.gz
# If a previous run left a broken distro, run:  wsl --unregister Ubuntu-24.04
$ErrorActionPreference = 'Continue'
$root = $PSScriptRoot
$gzFile = Join-Path $root 'downloads\ubuntu-base-24.04.5-base-amd64.tar.gz'
$distro = 'Ubuntu-24.04'
$tarFile = Join-Path $env:TEMP 'ubuntu-24.04-rootfs.tar'
$wslRoot = Join-Path $env:LOCALAPPDATA 'WSL'
$installDir = Join-Path $wslRoot 'Ubuntu-24.04'
$log = Join-Path $root 'import-ubuntu.log'

function Log([string]$m) {
    $line = "$(Get-Date -Format 'HH:mm:ss')  $m"
    Write-Host $line
    $line | Out-File -FilePath $log -Append -Encoding utf8
}

Log '==================== import-ubuntu start (v3) ===================='
if (-not (Test-Path $gzFile)) {
    Log "ERROR: rootfs not found: $gzFile"
    Read-Host 'Press Enter to close'
    exit 1
}
Log "rootfs : $gzFile"
Log "distro : $distro"
Log "dir    : $installDir"

# --- make sure the WSL data directory exists (this was missing in v1) ---
New-Item -ItemType Directory -Path $wslRoot -Force | Out-Null
Log ("wslRoot exists: " + (Test-Path $wslRoot))

$existing = @(& wsl.exe -l -q 2>$null) | ForEach-Object { ($_ -replace "`0", '').Trim() }
if ($existing -contains $distro) {
    Log 'distro already registered, skipping import'
} else {
    if (Test-Path $installDir) {
        Log "cleaning leftover install dir: $installDir"
        Remove-Item $installDir -Recurse -Force -ErrorAction SilentlyContinue
    }

    Log '--- decompress rootfs: gzip -> tar (about 30-60 s) ---'
    if (Test-Path $tarFile) { Remove-Item $tarFile -Force }
    try {
        $in = [IO.File]::OpenRead($gzFile)
        $outStream = [IO.File]::Create($tarFile)
        $gz = New-Object System.IO.Compression.GZipStream($in, [System.IO.Compression.CompressionMode]::Decompress)
        $gz.CopyTo($outStream)
        $gz.Close()
        $outStream.Flush()
        $outStream.Close()
        $in.Close()
        Log ('tar size: {0:N0} bytes' -f (Get-Item $tarFile).Length)
    } catch {
        Log ('decompress failed: ' + $_.Exception.Message)
        Read-Host 'Press Enter to close'
        exit 1
    }

    Log '--- wsl --import (attempt 1: location does not exist yet) ---'
    $out = & wsl.exe --import $distro $installDir $tarFile --version 2 2>&1
    Log (($out | Out-String).Trim())
    Log "exit code: $LASTEXITCODE"

    if ($LASTEXITCODE -ne 0) {
        Log '--- attempt 2: create the location directory first, retry ---'
        New-Item -ItemType Directory -Path $installDir -Force | Out-Null
        Start-Sleep -Seconds 2
        $out = & wsl.exe --import $distro $installDir $tarFile --version 2 2>&1
        Log (($out | Out-String).Trim())
        Log "exit code: $LASTEXITCODE"
    }

    if ($LASTEXITCODE -ne 0) {
        Log 'IMPORT FAILED - diagnostics below.'
        Log ('tar exists: ' + (Test-Path $tarFile) + '  size: ' + (Get-Item $tarFile -ErrorAction SilentlyContinue).Length)
        $st = & wsl.exe --status 2>&1
        Log (($st | Out-String).Trim())
        $ver = & wsl.exe --version 2>&1
        Log (($ver | Out-String).Trim())
        Remove-Item $tarFile -Force -ErrorAction SilentlyContinue
        Log 'Try manually:  wsl --unregister Ubuntu-24.04   then rerun this script.'
        Read-Host 'Press Enter to close'
        exit 1
    }
    Remove-Item $tarFile -Force -ErrorAction SilentlyContinue
    Log 'import OK'
}

Log '--- post-install inside Ubuntu (user, tuna mirror, toolchain) ---'
Log '    this can take 5-20 minutes depending on network speed'

$bash = @'
set -e
export DEBIAN_FRONTEND=noninteractive
echo "== switch apt mirror to tsinghua =="
for f in /etc/apt/sources.list /etc/apt/sources.list.d/ubuntu.sources; do
  if [ -f "$f" ]; then
    cp -n "$f" "$f.bak" 2>/dev/null || true
    sed -i 's|http://archive.ubuntu.com/ubuntu|https://mirrors.tuna.tsinghua.edu.cn/ubuntu|g' "$f"
    sed -i 's|http://security.ubuntu.com/ubuntu|https://mirrors.tuna.tsinghua.edu.cn/ubuntu|g' "$f"
    sed -i 's|https://archive.ubuntu.com/ubuntu|https://mirrors.tuna.tsinghua.edu.cn/ubuntu|g' "$f"
    sed -i 's|https://security.ubuntu.com/ubuntu|https://mirrors.tuna.tsinghua.edu.cn/ubuntu|g' "$f"
  fi
done
echo "== apt update =="
apt-get update
echo "== install base + toolchain =="
apt-get install -y --no-install-recommends sudo ca-certificates build-essential cmake ninja-build clang clangd lldb gdb git curl zip unzip pkg-config
echo "== create user dym =="
if ! id -u dym >/dev/null 2>&1; then
  useradd -m -s /bin/bash -G sudo dym
fi
echo "dym ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/90-dym
chmod 440 /etc/sudoers.d/90-dym
printf '[boot]\nsystemd=false\n\n[user]\ndefault=dym\n' > /etc/wsl.conf
apt-get clean
echo "== versions =="
g++ --version | head -n1
clang++ --version | head -n1
cmake --version | head -n1
ninja --version
git --version
echo "== post-install done =="
'@

$bash.Replace("`r`n", "`n") | & wsl.exe -d $distro -u root bash -s 2>&1 | ForEach-Object { Log ($_ -replace "`0", '') }

Log '--- restart WSL and verify ---'
& wsl.exe --shutdown 2>&1 | Out-Null
Start-Sleep -Seconds 4
$who = @(& wsl.exe -d $distro whoami 2>&1) | ForEach-Object { ($_ -replace "`0", '').Trim() }
Log ("whoami: " + ($who -join ' '))
$os = @(& wsl.exe -d $distro bash -lc 'head -n 2 /etc/os-release' 2>&1) | ForEach-Object { ($_ -replace "`0", '').Trim() }
foreach ($l in $os) { Log $l }
Log '==================== import-ubuntu done ===================='
Log 'Next: open a terminal and run  wsl -d Ubuntu-24.04  to enter Ubuntu.'
Read-Host 'Press Enter to close'
