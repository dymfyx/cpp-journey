param(
  [string]$Action = 'click',
  [int]$X = 0, [int]$Y = 0,
  [int]$X2 = 0, [int]$Y2 = 0,
  [string]$Keys = ''
)
$ErrorActionPreference = 'Stop'
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public class Mouse {
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint f, int dx, int dy, uint d, int e);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool GetCursorPos(out POINT p);
  [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
  public const uint LEFTDOWN = 0x0002, LEFTUP = 0x0004, RIGHTDOWN = 0x0008, RIGHTUP = 0x0010, WHEEL = 0x0800;
  public static void Click(int x, int y) {
    SetCursorPos(x, y); System.Threading.Thread.Sleep(120);
    mouse_event(LEFTDOWN, 0, 0, 0, 0); System.Threading.Thread.Sleep(80);
    mouse_event(LEFTUP, 0, 0, 0, 0);
  }
  public static void Drag(int x1, int y1, int x2, int y2) {
    SetCursorPos(x1, y1); System.Threading.Thread.Sleep(150);
    mouse_event(LEFTDOWN, 0, 0, 0, 0); System.Threading.Thread.Sleep(100);
    int steps = 24;
    for (int i = 1; i <= steps; i++) {
      SetCursorPos(x1 + (x2 - x1) * i / steps, y1 + (y2 - y1) * i / steps);
      System.Threading.Thread.Sleep(20);
    }
    mouse_event(LEFTUP, 0, 0, 0, 0);
  }
}
'@
[void][Mouse]::SetProcessDPIAware()
switch ($Action) {
  'click' { [Mouse]::Click($X, $Y); Write-Output "clicked $X,$Y" }
  'rightclick' {
    [void][Mouse]::SetCursorPos($X, $Y); Start-Sleep -Milliseconds 150
    [Mouse]::mouse_event(0x0008, 0, 0, 0, 0); Start-Sleep -Milliseconds 80
    [Mouse]::mouse_event(0x0010, 0, 0, 0, 0); Write-Output "right-clicked $X,$Y"
  }
  'drag' { [Mouse]::Drag($X, $Y, $X2, $Y2); Write-Output "dragged $X,$Y -> $X2,$Y2" }
  'pos' { $p = New-Object Mouse+POINT; [void][Mouse]::GetCursorPos([ref]$p); Write-Output "cursor $($p.X),$($p.Y)" }
  'move' { [void][Mouse]::SetCursorPos($X, $Y); Write-Output "moved $X,$Y" }
  'keys' {
    Add-Type -AssemblyName System.Windows.Forms
    $parts = $Keys -split '\+'
    $fx = @()
    $mods = @()
    foreach ($p in $parts[0..($parts.Count - 2)]) {
      switch ($p) {
        'Ctrl' { $mods += 'ControlKey' }
        'Shift' { $mods += 'ShiftKey' }
        'Alt' { $mods += 'Menu' }
        'Win' { $mods += 'LWin' }
      }
    }
    $main = $parts[-1]
    foreach ($m in $mods) { [System.Windows.Forms.SendKeys]::SendWait("{$m}") ; $fx += $m }
    # send main key
    $map = @{ 'Enter'='{ENTER}'; 'Esc'='{ESC}'; 'Tab'='{TAB}'; 'Backspace'='{BACKSPACE}'; 'Delete'='{DEL}'; 'Space'=' '; 'F1'='{F1}'; 'F2'='{F2}'; 'F3'='{F3}'; 'F4'='{F4}' }
    if ($map.ContainsKey($main)) { [System.Windows.Forms.SendKeys]::SendWait($map[$main]) } else { [System.Windows.Forms.SendKeys]::SendWait($main.ToLower()) }
    foreach ($m in $mods) { [System.Windows.Forms.SendKeys]::SendWait("{" + $m + " up}") }
    Write-Output "keys sent: $Keys"
  }
  default { throw "unknown action $Action" }
}
