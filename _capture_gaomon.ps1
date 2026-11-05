param([long]$Hwnd = 460294, [string]$Out = "$env:TEMP\gaomon_ui.png")
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public class Cap {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L,T,R,B; }
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int cmd);
  [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr h);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
}
'@
[void][Cap]::SetProcessDPIAware()
$h = [IntPtr]$Hwnd
if ([Cap]::IsIconic($h)) { [void][Cap]::ShowWindow($h, 9) }
[void][Cap]::SetForegroundWindow($h)
Start-Sleep -Milliseconds 600
$r = New-Object Cap+RECT
[void][Cap]::GetWindowRect($h, [ref]$r)
$w = $r.R - $r.L; $hh = $r.B - $r.T
if ($w -le 0 -or $hh -le 0) { throw "bad rect ${w}x${hh}" }
$bmp = New-Object System.Drawing.Bitmap $w, $hh
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($r.L, $r.T, 0, 0, (New-Object System.Drawing.Size $w, $hh))
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose(); $bmp.Dispose()
Write-Output "saved $Out ${w}x${hh} at $($r.L),$($r.T)"
