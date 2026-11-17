# 生成复盘手册 PDF：合并 docs/handbook/*.md -> HTML -> Edge 无头打印 PDF
# 用法：powershell -ExecutionPolicy Bypass -File scripts\build-handbook.ps1 [-Version 1.0]
param(
    [string]$Version = "1.0"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$handbookDir = Join-Path $root "docs\handbook"
$combined = Join-Path $handbookDir "_combined.md"
$html = Join-Path $handbookDir "_combined.html"
$date = Get-Date -Format "yyyyMMdd"
$pdf = Join-Path $root "docs\复盘手册-v$Version-$date.pdf"

# 1. 按文件名排序合并
$files = Get-ChildItem $handbookDir -Filter "*.md" | Where-Object { $_.Name -notlike "_*" } | Sort-Object Name
if ($files.Count -eq 0) { throw "docs/handbook 下没有章节文件" }
$sb = [System.Text.StringBuilder]::new()
foreach ($f in $files) {
    [void]$sb.AppendLine((Get-Content $f.FullName -Raw -Encoding UTF8))
    [void]$sb.AppendLine("`n`n")
}
Set-Content -Path $combined -Value $sb.ToString() -Encoding UTF8
Write-Host "合并 $($files.Count) 个章节 -> $combined"

# 2. Markdown -> HTML
$py = (Get-Command python).Source
& $py (Join-Path $PSScriptRoot "md2html.py") $combined $html
if ($LASTEXITCODE -ne 0) { throw "markdown 转换失败" }

# 3. Edge 无头打印 PDF
$edge = "C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"
if (-not (Test-Path $edge)) { $edge = "C:\Program Files\Microsoft\Edge\Application\msedge.exe" }
if (-not (Test-Path $edge)) { throw "找不到 Edge，无法打印 PDF" }

$profile = Join-Path $env:TEMP ("kb-print-" + [guid]::NewGuid().ToString("N").Substring(0, 8))
$uri = ([uri]$html).AbsoluteUri
& $edge --headless=new --disable-gpu --no-first-run --no-pdf-header-footer `
    "--user-data-dir=$profile" "--print-to-pdf=$pdf" $uri 2>&1 | Out-Null
Start-Sleep -Seconds 2
Remove-Item $profile -Recurse -Force -ErrorAction SilentlyContinue

if (-not (Test-Path $pdf)) { throw "PDF 生成失败：$pdf" }
$size = [math]::Round((Get-Item $pdf).Length / 1KB, 1)
Write-Host "PDF 生成成功：$pdf ($size KB)"
