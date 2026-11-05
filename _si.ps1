param(
  [string]$Do = 'hotkey',
  [int]$X = 0, [int]$Y = 0,
  [string]$Out = "$env:TEMP\si.png"
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
public class SI {
  [StructLayout(LayoutKind.Sequential)] public struct MOUSEINPUT { public int dx; public int dy; public uint mouseData; public uint dwFlags; public uint time; public IntPtr dwExtraInfo; }
  [StructLayout(LayoutKind.Sequential)] public struct KEYBDINPUT { public ushort wVk; public ushort wScan; public uint dwFlags; public uint time; public IntPtr dwExtraInfo; }
  [StructLayout(LayoutKind.Explicit)] public struct INPUTUNION { [FieldOffset(0)] public MOUSEINPUT mi; [FieldOffset(0)] public KEYBDINPUT ki; }
  [StructLayout(LayoutKind.Sequential)] public struct INPUT { public uint type; public INPUTUNION u; }
  [DllImport("user32.dll", SetLastError=true)] public static extern uint SendInput(uint n, INPUT[] inputs, int size);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern bool GetCursorPos(out POINT p);
  [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }

  public static INPUT Key(ushort vk, bool up) {
    INPUT i = new INPUT(); i.type = 1;
    i.u.ki.wVk = vk; i.u.ki.wScan = 0;
    i.u.ki.dwFlags = up ? 2u : 0u;
    i.u.ki.time = 0; i.u.ki.dwExtraInfo = IntPtr.Zero;
    return i;
  }
  public static INPUT MouseAbs(int x, int y, uint flags, int screenW, int screenH) {
    INPUT i = new INPUT(); i.type = 0;
    i.u.mi.dx = (int)((double)x * 65535.0 / (screenW - 1));
    i.u.mi.dy = (int)((double)y * 65535.0 / (screenH - 1));
    i.u.mi.mouseData = 0;
    i.u.mi.dwFlags = flags | 0x8000u | 0x0001u; // ABSOLUTE|MOVE
    i.u.mi.time = 0; i.u.mi.dwExtraInfo = IntPtr.Zero;
    return i;
  }
  public static void Send(INPUT[] arr) { SendInput((uint)arr.Length, arr, Marshal.SizeOf(typeof(INPUT))); }
  public static string Cursor() { POINT p; GetCursorPos(out p); return p.X + "," + p.Y; }
}
'@
[void][SI]::SetProcessDPIAware()
$sw = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds.Width
$sh = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds.Height
function ShotFull([string]$path) {
  $bmp = New-Object System.Drawing.Bitmap $sw, $sh
  $g = [System.Drawing.Graphics]::FromImage($bmp)
  $g.CopyFromScreen(0, 0, 0, 0, (New-Object System.Drawing.Size $sw, $sh))
  $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
  $g.Dispose(); $bmp.Dispose()
  Write-Output ("saved {0} {1}x{2}" -f $path, $sw, $sh)
}
switch ($Do) {
  'hotkey' {
    # Ctrl+Alt+H
    $arr = @(
      [SI]::Key(0x11,$false), [SI]::Key(0x12,$false), [SI]::Key(0x48,$false),
      [SI]::Key(0x48,$true),  [SI]::Key(0x12,$true),  [SI]::Key(0x11,$true)
    )
    [SI]::Send($arr)
    Write-Output "sent ctrl+alt+h"
  }
  'click' {
    $arr = @(
      [SI]::MouseAbs($X,$Y,0,$sw,$sh),
      [SI]::MouseAbs($X,$Y,0x0002,$sw,$sh),
      [SI]::MouseAbs($X,$Y,0x0004,$sw,$sh)
    )
    [SI]::Send($arr)
    Write-Output ("clicked {0},{1} cursor={2}" -f $X,$Y,[SI]::Cursor())
  }
  'move' {
    $arr = @([SI]::MouseAbs($X,$Y,0,$sw,$sh))
    [SI]::Send($arr)
    Write-Output ("moved {0},{1} cursor={2}" -f $X,$Y,[SI]::Cursor())
  }
  'shot' { ShotFull $Out }
  'cursor' { Write-Output ("cursor=" + [SI]::Cursor()) }
  default { throw "unknown Do=$Do" }
}
