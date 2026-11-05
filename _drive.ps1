param(
  [string]$Do = 'shot',
  [long]$Hwnd = 460294,
  [int]$X = 0, [int]$Y = 0,
  [int]$Vk = 0,
  [string]$Send = '',
  [string]$Out = "$env:TEMP\gaomon_shot.png"
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public class Drv {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L,T,R,B; }
  [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X,Y; }
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int cmd);
  [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint f, int dx, int dy, uint d, int e);
  [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint flags, int extra);
  public static void Click(int x, int y) {
    SetCursorPos(x, y); System.Threading.Thread.Sleep(150);
    mouse_event(0x0002, 0, 0, 0, 0); System.Threading.Thread.Sleep(90);
    mouse_event(0x0004, 0, 0, 0, 0);
  }
  public static void Key(byte vk) {
    keybd_event(vk, 0, 0, 0); System.Threading.Thread.Sleep(60);
    keybd_event(vk, 0, 2, 0);
  }
}
'@
[void][Drv]::SetProcessDPIAware()
$h = [IntPtr]$Hwnd
function Activate { if ([Drv]::IsIconic($h)) { [void][Drv]::ShowWindow($h, 9) }; [void][Drv]::SetForegroundWindow($h); Start-Sleep -Milliseconds 500 }
function Shot([string]$path) {
  $r = New-Object Drv+RECT
  [void][Drv]::GetWindowRect($h, [ref]$r)
  $w = $r.R - $r.L; $hh = $r.B - $r.T
  if ($w -le 0 -or $hh -le 0) { throw "bad rect ${w}x${hh}" }
  $bmp = New-Object System.Drawing.Bitmap $w, $hh
  $g = [System.Drawing.Graphics]::FromImage($bmp)
  $g.CopyFromScreen($r.L, $r.T, 0, 0, (New-Object System.Drawing.Size $w, $hh))
  $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
  $g.Dispose(); $bmp.Dispose()
  Write-Output ("saved {0} {1}x{2} at {3},{4}" -f $path, $w, $hh, $r.L, $r.T)
}
switch ($Do) {
  'show' { [void][Drv]::ShowWindow($h, 5); Start-Sleep -Milliseconds 300; Activate; Shot $Out }
  'hide' { [void][Drv]::ShowWindow($h, 0); Write-Output "hidden $Hwnd" }
  'shot' { Activate; Shot $Out }
  'click' { Activate; [Drv]::Click($X, $Y); Start-Sleep -Milliseconds 400; Shot $Out }
  'clickonly' { Activate; [Drv]::Click($X, $Y); Start-Sleep -Milliseconds 400; Write-Output "clicked $X,$Y" }
  'key' { Activate; [Drv]::Key([byte]$Vk); Start-Sleep -Milliseconds 400; Shot $Out }
  'sendkeys' { Activate; Add-Type -AssemblyName System.Windows.Forms; [System.Windows.Forms.SendKeys]::SendWait($Send); Start-Sleep -Milliseconds 400; Shot $Out }
  'info' {
    $p = New-Object Drv+POINT
    [void][Drv]::SetProcessDPIAware()
    $r = New-Object Drv+RECT
    [void][Drv]::GetWindowRect($h, [ref]$r)
    Write-Output ("win rect {0},{1} - {2},{3} size {4}x{5}" -f $r.L, $r.T, $r.R, $r.B, ($r.R - $r.L), ($r.B - $r.T))
    Write-Output ("foreground hwnd = {0}" -f [Drv]::GetForegroundWindow())
  }
  default { throw "unknown Do=$Do" }
}
