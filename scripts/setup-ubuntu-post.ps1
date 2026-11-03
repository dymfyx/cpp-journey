# setup-ubuntu-post.ps1 - finish the Ubuntu 24.04 setup after import.
# Run in a NORMAL terminal (no admin). ASCII only.
# Fixes: rootfs has no ca-certificates, so https mirrors fail -> use plain http mirrors.
$ErrorActionPreference = 'Continue'
$root = $PSScriptRoot
$distro = 'Ubuntu-24.04'
$log = Join-Path $root 'setup-ubuntu-post.log'

function Log([string]$m) {
    $line = "$(Get-Date -Format 'HH:mm:ss')  $m"
    Write-Host $line
    $line | Out-File -FilePath $log -Append -Encoding utf8
}

Log '==================== setup-ubuntu-post start ===================='

$bash = @'
export DEBIAN_FRONTEND=noninteractive
KEYRING=/usr/share/keyrings/ubuntu-archive-keyring.gpg
SRC=/etc/apt/sources.list.d/ubuntu.sources

write_src() {
  local base="$1"
  local signed=""
  [ -f "$KEYRING" ] && signed="Signed-By: $KEYRING"
  mkdir -p /etc/apt/sources.list.d
  cat > "$SRC" <<EOF
Types: deb
URIs: $base
Suites: noble noble-updates noble-backports
Components: main restricted universe multiverse
$signed
EOF
  rm -f /etc/apt/sources.list
  echo "== sources -> $base =="
  cat "$SRC"
}

try_update() {
  echo "== apt-get update =="
  apt-get update 2>&1 | tail -n 20
  return ${PIPESTATUS[0]}
}

ok=0
for base in "http://mirrors.tuna.tsinghua.edu.cn/ubuntu" \
            "http://mirrors.aliyun.com/ubuntu" \
            "http://mirrors.ustc.edu.cn/ubuntu" \
            "http://archive.ubuntu.com/ubuntu"; do
  write_src "$base"
  if try_update; then ok=1; break; fi
  echo "!! mirror failed: $base"
done
if [ "$ok" != "1" ]; then
  echo "FATAL: no working apt mirror"
  exit 11
fi

echo "== install base + toolchain =="
apt-get install -y --no-install-recommends \
  sudo ca-certificates build-essential cmake ninja-build clang clangd lldb gdb \
  git curl zip unzip pkg-config file less vim nano 2>&1 | tail -n 30
if [ "${PIPESTATUS[0]}" != "0" ]; then echo "FATAL: apt install failed"; exit 12; fi

echo "== create user dym =="
if ! id -u dym >/dev/null 2>&1; then
  useradd -m -s /bin/bash -G sudo dym
fi
echo "dym ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/90-dym
chmod 440 /etc/sudoers.d/90-dym
printf '[user]\ndefault=dym\n' > /etc/wsl.conf
echo "== user ready =="
id dym

echo "== versions =="
g++ --version | head -n1
clang++ --version | head -n1
cmake --version | head -n1
ninja --version
git --version
echo "== setup done =="
'@

$bash.Replace("`r`n", "`n") | & wsl.exe -d $distro -u root bash -s 2>&1 | ForEach-Object { Log ($_ -replace "`0", '') }
Log "exit code: $LASTEXITCODE"

Log '--- restart WSL and verify ---'
& wsl.exe --shutdown 2>&1 | Out-Null
Start-Sleep -Seconds 4
$who = @(& wsl.exe -d $distro whoami 2>&1) | ForEach-Object { ($_ -replace "`0", '').Trim() }
Log ("whoami: " + ($who -join ' '))
$os = @(& wsl.exe -d $distro bash -lc 'head -n 3 /etc/os-release' 2>&1) | ForEach-Object { ($_ -replace "`0", '').Trim() }
foreach ($l in $os) { Log $l }
$gcc = @(& wsl.exe -d $distro bash -lc 'g++ --version | head -n1' 2>&1) | ForEach-Object { ($_ -replace "`0", '').Trim() }
Log ("g++: " + ($gcc -join ' '))

Log '==================== setup-ubuntu-post done ===================='
Read-Host 'Press Enter to close'
