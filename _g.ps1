param(
  [long]$Hwnd = 460294,
  [string]$Steps = 'hotkey;sleep:900;shot:now.png',
  [string]$Dir = 'C:\Users\dym54\Desktop\c++项目'
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public class G {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L,T,R,B; }
  [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X,Y; }
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool GetCursorPos(out POINT p);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr h);
  [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr h, IntPtr after, int x, int y, int cx, int cy, uint flags);
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [StructLayout(LayoutKind.Sequential)] public struct MOUSEINPUT { public int dx; public int dy; public uint mouseData; public uint dwFlags; public uint time; public IntPtr dwExtraInfo; }
  [StructLayout(LayoutKind.Sequential)] public struct KEYBDINPUT { public ushort wVk; public ushort wScan; public uint dwFlags; public uint time; public IntPtr dwExtraInfo; }
  [StructLayout(LayoutKind.Explicit)] public struct INPUTUNION { [FieldOffset(0)] public MOUSEINPUT mi; [FieldOffset(0)] public KEYBDINPUT ki; }
  [StructLayout(LayoutKind.Sequential)] public struct INPUT { public uint type; public INPUTUNION u; }
  [DllImport("user32.dll", SetLastError=true)] public static extern uint SendInput(uint n, INPUT[] inputs, int size);
  public static INPUT Key(ushort vk, bool up) {
    INPUT i = new INPUT(); i.type = 1;
    i.u.ki.wVk = vk; i.u.ki.wScan = 0; i.u.ki.dwFlags = up ? 2u : 0u;
    return i;
  }
  public static INPUT MouseAbs(int x, int y, uint flags, int sw, int sh) {
    INPUT i = new INPUT(); i.type = 0;
    i.u.mi.dx = (int)((double)x * 65535.0 / (sw - 1));
    i.u.mi.dy = (int)((double)y * 65535.0 / (sh - 1));
    i.u.mi.mouseData = 0;
    i.u.mi.dwFlags = flags | 0x8000u | 0x0001u;
    return i;
  }
  public static void Send(INPUT[] a) { SendInput((uint)a.Length, a, Marshal.SizeOf(typeof(INPUT))); }
  public static void ClickAbs(int x, int y, int sw, int sh) {
    INPUT[] a = new INPUT[] { MouseAbs(x,y,0,sw,sh), MouseAbs(x,y,0x0002,sw,sh), MouseAbs(x,y,0x0004,sw,sh) };
    Send(a);
  }
}
'@
[void][G]::SetProcessDPIAware()
$h = [IntPtr]$Hwnd
$sw = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds.Width
$sh = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds.Height
$RECT = [G+RECT]
function GetRect { $r = New-Object G+RECT; [void][G]::GetWindowRect($h, [ref]$r); return $r }
function Shot([string]$name) {
  $r = GetRect
  $w = $r.R - $r.L; $ht = $r.B - $r.T
  if ($w -lt 200 -or $ht -lt 200) { Write-Output ("SHOT-ABORT window too small ${w}x${ht} rect=$($r.L),$($r.T)"); return }
  $path = Join-Path $Dir $name
  $bmp = New-Object System.Drawing.Bitmap $w, $ht
  $g = [System.Drawing.Graphics]::FromImage($bmp)
  $g.CopyFromScreen($r.L, $r.T, 0, 0, (New-Object System.Drawing.Size $w, $ht))
  $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
  $g.Dispose(); $bmp.Dispose()
  Write-Output ("shot {0} {1}x{2} rect={3},{4}" -f $path, $w, $ht, $r.L, $r.T)
}
function ShowWin {
  # global hotkey ctrl+alt+h
  $a = @([G]::Key(0x11,$false),[G]::Key(0x12,$false),[G]::Key(0x48,$false),[G]::Key(0x48,$true),[G]::Key(0x12,$true),[G]::Key(0x11,$true))
  [G]::Send($a)
  Write-Output 'sent ctrl+alt+h'
}
function PressKey([string]$combo) {
  $map = @{ 'esc'=0x1B; 'enter'=0x0D; 'tab'=0x09; 'back'=0x08; 'del'=0x2E; 'space'=0x20;
            'pageup'=0x21; 'pagedown'=0x22; 'home'=0x24; 'end'=0x23; 'left'=0x25; 'up'=0x26; 'right'=0x27; 'down'=0x28 }
  $parts = $combo.ToLower().Split('+')
  $mods = @(); $main = $parts[-1]
  foreach ($p in $parts[0..($parts.Count-2)]) {
    switch ($p) { 'ctrl' { $mods += 0x11 } 'shift' { $mods += 0x10 } 'alt' { $mods += 0x12 } 'win' { $mods += 0x5B } }
  }
  $vk = 0
  if ($map.ContainsKey($main)) { $vk = $map[$main] }
  elseif ($main.Length -eq 1) { $vk = [int][char]([char]::ToUpper($main)) }
  else { throw "bad key $main" }
  $list = New-Object System.Collections.ArrayList
  foreach ($m in $mods) { [void]$list.Add([G]::Key([byte]$m, $false)) }
  [void]$list.Add([G]::Key([byte]$vk, $false))
  [void]$list.Add([G]::Key([byte]$vk, $true))
  $modsRev = $mods.Clone(); [array]::Reverse($modsRev)
  foreach ($m in $modsRev) { [void]$list.Add([G]::Key([byte]$m, $true)) }
  [G]::Send([G+INPUT[]]$list.ToArray())
  Write-Output ("pressed " + $combo)
}
foreach ($step in ($Steps -split ';')) {
  $step = $step.Trim(); if (-not $step) { continue }
  $kv = $step.Split(':', 2)
  $cmd = $kv[0]; $arg = if ($kv.Count -gt 1) { $kv[1] } else { '' }
  switch ($cmd) {
    'hotkey' { ShowWin }
    'sleep' { Start-Sleep -Milliseconds ([int]$arg) }
    'shot' { Shot $arg }
    'state' {
      $r = GetRect
      Write-Output ("state rect={0},{1} {2}x{3} visible={4} iconic={5} fg={6} cursor={7}" -f $r.L,$r.T,($r.R-$r.L),($r.B-$r.T),[G]::IsWindowVisible($h),[G]::IsIconic($h),[G]::GetForegroundWindow(),((New-Object G+POINT) | ForEach-Object { [void][G]::GetCursorPos([ref]$_); "$($_.X),$($_.Y)" }))
    }
    'click' {
      $p = $arg.Split(','); $rx = [int]$p[0]; $ry = [int]$p[1]
      $r = GetRect
      if (($r.R - $r.L) -lt 200) { Write-Output "CLICK-ABORT window minimized"; break }
      [G]::ClickAbs($r.L + $rx, $r.T + $ry, $sw, $sh)
      Write-Output ("clicked rel {0},{1} abs {2},{3}" -f $rx, $ry, ($r.L + $rx), ($r.T + $ry))
      Start-Sleep -Milliseconds 350
    }
    'keys' { PressKey $arg; Start-Sleep -Milliseconds 250 }
    'info' {
      $r = GetRect
      Write-Output ("info rect={0},{1},{2},{3} visible={4} iconic={5}" -f $r.L,$r.T,$r.R,$r.B,[G]::IsWindowVisible($h),[G]::IsIconic($h))
    }
    default { Write-Output "unknown step $step" }
  }
}
