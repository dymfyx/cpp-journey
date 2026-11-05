param([long]$Hwnd = 460294, [string]$Out = "$env:TEMP\pw.png", [int]$Flag = 2)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public class PW {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L,T,R,B; }
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
}
'@
[void][PW]::SetProcessDPIAware()
$h = [IntPtr]$Hwnd
$r = New-Object PW+RECT
[void][PW]::GetWindowRect($h, [ref]$r)
$w = $r.R - $r.L; $hh = $r.B - $r.T
Write-Output ("rect {0}x{1} at {2},{3} visible={4}" -f $w, $hh, $r.L, $r.T, [PW]::IsWindowVisible($h))
$bmp = New-Object System.Drawing.Bitmap $w, $hh
$g = [System.Drawing.Graphics]::FromImage($bmp)
$hdc = $g.GetHdc()
$ok = [PW]::PrintWindow($h, $hdc, [uint32]$Flag)
$g.ReleaseHdc($hdc)
Write-Output "PrintWindow flag=$Flag ok=$ok"
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose(); $bmp.Dispose()
Write-Output "saved $Out"
