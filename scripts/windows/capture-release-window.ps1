[CmdletBinding()]
param(
    [string]$Executable,
    [string]$Output
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if (-not $Executable) {
    $Executable = Join-Path $repositoryRoot 'dist\release\AudioABComparator-0.2.1-beta.2-windows-x86_64\ab-compare.exe'
}
if (-not $Output) {
    $Output = Join-Path $repositoryRoot 'dist\media\audio-ab-comparator.png'
}

Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class AudioABWindowCapture {
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT { public int Left; public int Top; public int Right; public int Bottom; }
    [DllImport("user32.dll")]
    public static extern bool GetWindowRect(IntPtr handle, out RECT rectangle);
}
'@

$process = Start-Process -FilePath $Executable -PassThru
try {
    for ($attempt = 0; $attempt -lt 100 -and $process.MainWindowHandle -eq 0; $attempt++) {
        Start-Sleep -Milliseconds 100
        $process.Refresh()
    }
    if ($process.MainWindowHandle -eq 0) { throw 'The application window did not appear.' }
    Start-Sleep -Milliseconds 800

    $rectangle = New-Object AudioABWindowCapture+RECT
    if (-not [AudioABWindowCapture]::GetWindowRect($process.MainWindowHandle, [ref]$rectangle)) {
        throw 'GetWindowRect failed.'
    }
    $width = $rectangle.Right - $rectangle.Left
    $height = $rectangle.Bottom - $rectangle.Top
    if ($width -le 0 -or $height -le 0) { throw "Invalid window size: ${width}x${height}" }

    $outputDirectory = Split-Path -Parent $Output
    New-Item -ItemType Directory -Path $outputDirectory -Force | Out-Null
    $bitmap = New-Object System.Drawing.Bitmap $width, $height
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    try {
        $graphics.CopyFromScreen($rectangle.Left, $rectangle.Top, 0, 0, $bitmap.Size)
        $bitmap.Save($Output, [System.Drawing.Imaging.ImageFormat]::Png)
    } finally {
        $graphics.Dispose()
        $bitmap.Dispose()
    }
} finally {
    if (-not $process.HasExited) {
        $null = $process.CloseMainWindow()
        if (-not $process.WaitForExit(3000)) { Stop-Process -Id $process.Id }
    }
}
