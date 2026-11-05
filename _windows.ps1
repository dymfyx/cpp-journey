param([string]$Filter = '')
$ErrorActionPreference = 'Stop'
Add-Type -TypeDefinition @'
using System;
using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;
public class Win {
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L,T,R,B; }
  public delegate bool EnumProc(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr l);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetWindowTextLength(IntPtr h);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  public static List<string> List() {
    var res = new List<string>();
    EnumWindows(delegate(IntPtr h, IntPtr l) {
      uint pid; GetWindowThreadProcessId(h, out pid);
      var t = new StringBuilder(512); GetWindowText(h, t, 512);
      var c = new StringBuilder(256); GetClassName(h, c, 256);
      RECT r; GetWindowRect(h, out r);
      res.Add(string.Format("{0}\t{1}\t{2}\t{3},{4},{5},{6}\t{7}\t{8}", h, pid, IsWindowVisible(h) ? "vis" : "hid", r.L, r.T, r.R, r.B, c.ToString(), t.ToString()));
      return true;
    }, IntPtr.Zero);
    return res;
  }
}
'@
[void][Win]::SetProcessDPIAware()
$all = [Win]::List()
if ($Filter) { $all = $all | Where-Object { $_ -match $Filter } }
$all | ForEach-Object { Write-Output $_ }
Write-Output "-- total $(($all | Measure-Object).Count)"
