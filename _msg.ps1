param(
  [string]$Do = 'shot',
  [long]$Hwnd = 460294,
  [int]$X = 0, [int]$Y = 0,
  [string]$Out = "$env:TEMP\msg.png"
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public class MG {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L,T,R,B; }
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int cmd);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr h, IntPtr after, int x, int y, int cx, int cy, uint flags);
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint msg, IntPtr w, IntPtr l);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  public static IntPtr LP(int x, int y) { return (IntPtr)((y << 16) | (x & 0xFFFF)); }
  public static void Click(IntPtr h, int x, int y) {
    PostMessage(h, 0x0200, IntPtr.Zero, LP(x, y));   // WM_MOUSEMOVE
    System.Threading.Thread.Sleep(80);
    PostMessage(h, 0x0201, (IntPtr)1, LP(x, y));     // WM_LBUTTONDOWN
    System.Threading.Thread.Sleep(90);
    PostMessage(h, 0x0202, IntPtr.Zero, LP(x, y));   // WM_LBUTTONUP
  }
  public static void Move(IntPtr h, int x, int y) { PostMessage(h, 0x0200, IntPtr.Zero, LP(x, y)); }
}
'@
[void][MG]::SetProcessDPIAware()
$h = [IntPtr]$Hwnd
function Rect([IntPtr]$hh) {
  $r = New-Object MG+RECT; [void][MG]::GetWindowRect($hh, [ref]$r)
  return ("{0}x{1} at {2},{3}" -f ($r.R - $r.L), ($r.B - $r.T), $r.L, $r.T)
}
function ClientSize([IntPtr]$hh) {
  $c = New-Object MG+RECT; [void][MG]::GetClientRect($hh, [ref]$c)
  return ("client {0}x{1}" -f ($c.R - $c.L), ($c.B - $c.T))
}
function Shot([IntPtr]$hh, [string]$path) {
  $r = New-Object MG+RECT; [void][MG]::GetWindowRect($hh, [ref]$r)
  $w = $r.R - $r.L; $ht = $r.B - $r.T
  if ($w -le 0 -or $ht -le 0) { throw "bad rect ${w}x${ht}" }
  $bmp = New-Object System.Drawing.Bitmap $w, $ht
  $g = [System.Drawing.Graphics]::FromImage($bmp)
  $hdc = $g.GetHdc()
  $ok = [MG]::PrintWindow($hh, $hdc, 2)
  $g.ReleaseHdc($hdc); $g.Dispose()
  $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Png); $bmp.Dispose()
  Write-Output ("shot {0} ok={1} {2}x{3} -> {4}" -f $Hwnd, $ok, $w, $ht, $path)
}
switch ($Do) {
  'info' { Write-Output (Rect $h); Write-Output (ClientSize $h); Write-Output ("visible=" + [MG]::IsWindowVisible($h)) }
  'shot' { Shot $h $Out }
  'showbottom' {
    [void][MG]::ShowWindow($h, 4)  # SW_SHOWNOACTIVATE
    Start-Sleep -Milliseconds 200
    [void][MG]::SetWindowPos($h, [IntPtr]1, 0, 0, 0, 0, 0x0013)  # HWND_BOTTOM, NOMOVE|NOSIZE|NOACTIVATE
    Start-Sleep -Milliseconds 300
    Write-Output (Rect $h); Write-Output (ClientSize $h); Write-Output ("visible=" + [MG]::IsWindowVisible($h))
  }
  'restore' {
    [void][MG]::ShowWindow($h, 9)  # SW_RESTORE
    Start-Sleep -Milliseconds 500
    Write-Output (Rect $h); Write-Output ("visible=" + [MG]::IsWindowVisible($h))
  }
  'show' { [void][MG]::ShowWindow($h, 5); Start-Sleep -Milliseconds 400; Write-Output (Rect $h); Write-Output ("visible=" + [MG]::IsWindowVisible($h)) }
  'hide' { [void][MG]::ShowWindow($h, 0); Write-Output ("hidden, visible=" + [MG]::IsWindowVisible($h)) }
  'click' { [MG]::Click($h, $X, $Y); Start-Sleep -Milliseconds 500; Write-Output "msg-clicked $X,$Y -> $Hwnd" }
  'clickshot' { [MG]::Click($h, $X, $Y); Start-Sleep -Milliseconds 600; Shot $h $Out }
  'move' { [MG]::Move($h, $X, $Y); Write-Output "msg-move $X,$Y" }
  default { throw "unknown Do=$Do" }
}
