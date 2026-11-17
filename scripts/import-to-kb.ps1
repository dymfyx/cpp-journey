# 把复盘手册 PDF 导入「知库 KnowBase」。
#
# 用法（在仓库根目录）：
#   powershell -ExecutionPolicy Bypass -File scripts\import-to-kb.ps1
#   powershell -ExecutionPolicy Bypass -File scripts\import-to-kb.ps1 -Pdf "docs\复盘手册-v1.1-20261201.pdf"
#
# 依赖：知库已安装在 C:\Users\dym54\Desktop\个人知识库（可用 -KbRoot 改），
#       且本机有 curl.exe（Windows 10+ 自带）。
param(
    [string]$Pdf = "",
    [string]$KbRoot = "C:\Users\dym54\Desktop\个人知识库",
    [string]$Token = ""
)

$ErrorActionPreference = "Stop"
$repo = Split-Path -Parent $PSScriptRoot

if (-not $Pdf) {
    $latest = Get-ChildItem (Join-Path $repo "docs") -Filter "复盘手册-*.pdf" |
              Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if (-not $latest) { throw "docs/ 下没有复盘手册 PDF，请先运行 scripts\build-handbook.ps1" }
    $Pdf = $latest.FullName
} elseif (-not [System.IO.Path]::IsPathRooted($Pdf)) {
    $Pdf = Join-Path $repo $Pdf
}
if (-not (Test-Path $Pdf)) { throw "找不到 PDF：$Pdf" }

$sessionFile = Join-Path $KbRoot "vault\.kb\session.json"
if (-not (Test-Path $sessionFile)) { throw "找不到知库 session 文件：$sessionFile" }
$session = Get-Content $sessionFile -Raw | ConvertFrom-Json
$port = if ($session.port) { $session.port } else { 8765 }
if (-not $Token) { $Token = $session.token }

$probe = $false
try {
    $tcp = New-Object System.Net.Sockets.TcpClient
    $tcp.Connect("127.0.0.1", $port)
    $probe = $tcp.Connected
    $tcp.Close()
} catch { $probe = $false }

if (-not $probe) {
    Write-Host "知库服务未运行，正在后台启动……"
    $env:KB_NO_BROWSER = "1"
    Start-Process -FilePath (Join-Path $KbRoot ".venv\Scripts\python.exe") `
        -ArgumentList (Join-Path $KbRoot "server\launch.py") -WindowStyle Hidden
    $deadline = (Get-Date).AddSeconds(30)
    while ((Get-Date) -lt $deadline) {
        Start-Sleep -Milliseconds 500
        try {
            $tcp = New-Object System.Net.Sockets.TcpClient
            $tcp.Connect("127.0.0.1", $port)
            if ($tcp.Connected) { $probe = $true; $tcp.Close(); break }
        } catch { }
    }
    if (-not $probe) { throw "知库服务启动超时（端口 $port）" }
    $session = Get-Content $sessionFile -Raw | ConvertFrom-Json
    if ($session.token) { $Token = $session.token }
    $port = if ($session.port) { $session.port } else { $port }
}

Write-Host "导入：$Pdf"
$result = & curl.exe -s -m 120 -X POST "http://127.0.0.1:$port/api/papers" `
    -H "X-KB-Token: $Token" `
    -F "file=@$Pdf;type=application/pdf"

if ($LASTEXITCODE -ne 0) { throw "curl 调用失败（exit $LASTEXITCODE）" }
Write-Host $result
if ($result -match '"id"') {
    Write-Host "✅ 导入成功。打开知库桌面图标即可在论文列表看到它。"
} else {
    throw "导入可能失败，返回内容：$result"
}
