$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$executable = Join-Path $repositoryRoot 'dist\release\AudioABComparator-0.2.1-beta.2-windows-x86_64\ab-compare.exe'
$fixtures = 'C:\Dev\audioab-format-fixtures'
$outputDirectory = Join-Path $repositoryRoot 'dist\media'

Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class AudioABDemoCapture {
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT { public int Left; public int Top; public int Right; public int Bottom; }
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr handle, out RECT rectangle);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr handle);
    [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
    [DllImport("user32.dll")] public static extern void mouse_event(uint flags, uint dx, uint dy, uint data, UIntPtr extraInfo);
}
'@

function Get-ApplicationRectangle([IntPtr]$handle) {
    $rectangle = New-Object AudioABDemoCapture+RECT
    if (-not [AudioABDemoCapture]::GetWindowRect($handle, [ref]$rectangle)) { throw 'GetWindowRect failed.' }
    return $rectangle
}

function Invoke-WindowClick([IntPtr]$handle, [int]$relativeX, [int]$relativeY) {
    $rectangle = Get-ApplicationRectangle $handle
    [AudioABDemoCapture]::SetForegroundWindow($handle) | Out-Null
    Start-Sleep -Milliseconds 250
    [AudioABDemoCapture]::SetCursorPos($rectangle.Left + $relativeX, $rectangle.Top + $relativeY) | Out-Null
    Start-Sleep -Milliseconds 100
    [AudioABDemoCapture]::mouse_event(0x0002, 0, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 50
    [AudioABDemoCapture]::mouse_event(0x0004, 0, 0, 0, [UIntPtr]::Zero)
}

function Invoke-WindowClickRatio([IntPtr]$handle, [double]$xRatio, [double]$yRatio) {
    $rectangle = Get-ApplicationRectangle $handle
    $width = $rectangle.Right - $rectangle.Left
    $height = $rectangle.Bottom - $rectangle.Top
    Invoke-WindowClick $handle ([math]::Round($width * $xRatio)) ([math]::Round($height * $yRatio))
}

function Open-AudioFile([IntPtr]$handle, [double]$xRatio, [string]$path) {
    Invoke-WindowClickRatio $handle $xRatio 0.205
    Start-Sleep -Milliseconds 700
    [System.Windows.Forms.SendKeys]::SendWait('%n')
    Start-Sleep -Milliseconds 200
    [System.Windows.Forms.SendKeys]::SendWait('^a')
    [System.Windows.Forms.SendKeys]::SendWait($path)
    [System.Windows.Forms.SendKeys]::SendWait('{ENTER}')
    Start-Sleep -Seconds 4
}

function Save-ApplicationWindow([IntPtr]$handle, [string]$path) {
    $rectangle = Get-ApplicationRectangle $handle
    $left = $rectangle.Left + 8
    $top = $rectangle.Top
    $width = $rectangle.Right - $rectangle.Left - 16
    $height = $rectangle.Bottom - $rectangle.Top - 8
    $bitmap = New-Object System.Drawing.Bitmap $width, $height
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    try {
        $graphics.CopyFromScreen($left, $top, 0, 0, $bitmap.Size)
        $bitmap.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
    } finally {
        $graphics.Dispose()
        $bitmap.Dispose()
    }
}

New-Item -ItemType Directory -Path $outputDirectory -Force | Out-Null
$process = Start-Process -FilePath $executable -PassThru
try {
    for ($attempt = 0; $attempt -lt 100 -and $process.MainWindowHandle -eq 0; $attempt++) {
        Start-Sleep -Milliseconds 100
        $process.Refresh()
    }
    if ($process.MainWindowHandle -eq 0) { throw 'The application window did not appear.' }
    $handle = $process.MainWindowHandle
    Start-Sleep -Seconds 1

    Open-AudioFile $handle 0.44 (Join-Path $fixtures 'track-a.wav')
    Open-AudioFile $handle 0.925 (Join-Path $fixtures 'track-b.wav')
    Save-ApplicationWindow $handle (Join-Path $outputDirectory 'demo-1-loaded.png')

    Invoke-WindowClickRatio $handle 0.755 0.71
    Start-Sleep -Milliseconds 700
    Invoke-WindowClickRatio $handle 0.865 0.71
    Start-Sleep -Milliseconds 700
    Save-ApplicationWindow $handle (Join-Path $outputDirectory 'demo-2-switch.png')

    Invoke-WindowClickRatio $handle 0.55 0.10
    Start-Sleep -Milliseconds 700
    Save-ApplicationWindow $handle (Join-Path $outputDirectory 'demo-3-blind.png')
} finally {
    if (-not $process.HasExited) {
        $null = $process.CloseMainWindow()
        if (-not $process.WaitForExit(3000)) { Stop-Process -Id $process.Id }
    }
}
