# 下载 JSONTestSuite 的 test_parsing 到 tests/jsonsuite
#
# 为什么用 Windows PowerShell：WSL 里连不上 github.com:443（公司/校园网环境常见），
# 而 Windows 侧可以正常访问。文件下载和解压完只放进工作区，和 git 无关。
#
# 用法（在仓库根目录）：
#   pwsh -File scripts/fetch-jsontestsuite.ps1

param(
    [string]$OutDir = (Join-Path $PSScriptRoot '..\projects\p1-jq-lite\tests\jsonsuite')
)

$ErrorActionPreference = 'Stop'

$zipPath = Join-Path $PSScriptRoot 'downloads\JSONTestSuite-master.zip'
New-Item -ItemType Directory -Force -Path (Split-Path $zipPath) | Out-Null

if (-not (Test-Path $zipPath)) {
    Write-Host "下载 JSONTestSuite master.zip ..."
    Invoke-WebRequest -Uri 'https://github.com/nst/JSONTestSuite/archive/refs/heads/master.zip' `
        -OutFile $zipPath -UseBasicParsing -TimeoutSec 300
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
$archive = [System.IO.Compression.ZipFile]::OpenRead((Resolve-Path $zipPath))
try {
    $prefix = 'JSONTestSuite-master/test_parsing/'
    New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
    $count = 0
    foreach ($entry in $archive.Entries) {
        if ($entry.FullName -notlike "$prefix*" -or $entry.Name -eq '') { continue }
        [System.IO.Compression.ZipFileExtensions]::ExtractToFile(
            $entry, (Join-Path $OutDir $entry.Name), $true)
        $count++
    }
}
finally {
    $archive.Dispose()
}

Write-Host "解压完成: $count 个文件 -> $OutDir"
Write-Host ("y_ = {0}  n_ = {1}  i_ = {2}" -f
    (Get-ChildItem $OutDir -Filter 'y_*.json').Count,
    (Get-ChildItem $OutDir -Filter 'n_*.json').Count,
    (Get-ChildItem $OutDir -Filter 'i_*.json').Count)
