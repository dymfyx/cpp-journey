param([string]$Out = "$env:TEMP\fullscreen.png", [int]$X = 0, [int]$Y = 0, [int]$W = 0, [int]$H = 0)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public class Cap2 {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L,T,R,B; }
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool GetCursorPos(out POINT p);
  [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
}
'@
[void][Cap2]::SetProcessDPIAware()
Add-Type -AssemblyName System.Windows.Forms
$bounds = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
if ($W -le 0) { $W = $bounds.Width; $X = $bounds.X }
if ($H -le 0) { $H = $bounds.Height; $Y = $bounds.Y }
$bmp = New-Object System.Drawing.Bitmap $W, $H
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($X, $Y, 0, 0, (New-Object System.Drawing.Size $W, $H))
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose(); $bmp.Dispose()
$p = New-Object Cap2+POINT
[void][Cap2]::GetCursorPos([ref]$p)
Write-Output "saved $Out ${W}x${H} from ($X,$Y); cursor at $($p.X),$($p.Y)"
