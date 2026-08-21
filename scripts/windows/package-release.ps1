[CmdletBinding()]
param(
    [string]$BuildDirectory = 'build\windows-release',
    [string]$QtRoot = 'C:\Qt\6.9.3\msvc2022_64',
    [string]$OutputDirectory = 'dist\release'
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$releaseVersion = '0.2.1-beta.3'
$packageName = "AudioABComparator-$releaseVersion-windows-x86_64"
$repositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$buildPath = (Resolve-Path (Join-Path $repositoryRoot $BuildDirectory)).Path
$outputPath = Join-Path $repositoryRoot $OutputDirectory
$installPath = Join-Path $outputPath 'install'
$packagePath = Join-Path $outputPath $packageName
$zipPath = Join-Path $outputPath "$packageName.zip"
$executableCandidates = @(
    (Join-Path $buildPath 'ab-compare.exe'),
    (Join-Path $buildPath 'app\ab-compare.exe')
)
$executable = $executableCandidates | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1
$executable = if ($executable) { $executable } else { $executableCandidates[0] }
$windeployqt = Join-Path $QtRoot 'bin\windeployqt.exe'

foreach ($required in @($executable, $windeployqt, (Join-Path $repositoryRoot 'LICENSE'), (Join-Path $repositoryRoot 'THIRD_PARTY_NOTICES.md'))) {
    if (-not (Test-Path -LiteralPath $required -PathType Leaf)) {
        throw "Required file is missing: $required"
    }
}

if (Test-Path -LiteralPath $installPath) { Remove-Item -LiteralPath $installPath -Recurse -Force }
if (Test-Path -LiteralPath $packagePath) { Remove-Item -LiteralPath $packagePath -Recurse -Force }
if (Test-Path -LiteralPath $zipPath) { Remove-Item -LiteralPath $zipPath -Force }

New-Item -ItemType Directory -Path $installPath -Force | Out-Null
New-Item -ItemType Directory -Path $packagePath -Force | Out-Null

& cmake --install $buildPath --prefix $installPath
if ($LASTEXITCODE -ne 0) { throw "cmake --install failed with exit code $LASTEXITCODE" }

$installedExecutable = Join-Path $installPath 'bin\ab-compare.exe'
& $windeployqt --release --no-compiler-runtime --qmldir (Join-Path $repositoryRoot 'app\ui') $installedExecutable
if ($LASTEXITCODE -ne 0) { throw "windeployqt failed with exit code $LASTEXITCODE" }

$redistRoot = $env:VCToolsRedistDir
if (-not $redistRoot) {
    $redistRoot = Get-ChildItem 'C:\BuildTools\VC\Redist\MSVC' -Directory -ErrorAction SilentlyContinue |
        Sort-Object Name -Descending |
        Select-Object -First 1 -ExpandProperty FullName
}
$crtDirectory = if ($redistRoot) { Join-Path $redistRoot 'x64\Microsoft.VC143.CRT' } else { $null }
if (-not $crtDirectory -or -not (Test-Path -LiteralPath $crtDirectory -PathType Container)) {
    throw 'The app-local Microsoft VC143 runtime directory was not found.'
}
Copy-Item -Path (Join-Path $crtDirectory '*.dll') -Destination (Join-Path $installPath 'bin') -Force

Copy-Item -Path (Join-Path $installPath 'bin\*') -Destination $packagePath -Recurse -Force
Copy-Item -LiteralPath (Join-Path $repositoryRoot 'packaging\windows\README.txt') -Destination (Join-Path $packagePath 'README.txt')
Copy-Item -LiteralPath (Join-Path $repositoryRoot 'LICENSE') -Destination (Join-Path $packagePath 'LICENSE')
Copy-Item -LiteralPath (Join-Path $repositoryRoot 'THIRD_PARTY_NOTICES.md') -Destination (Join-Path $packagePath 'THIRD_PARTY_NOTICES.md')
Copy-Item -LiteralPath (Join-Path $repositoryRoot 'licenses') -Destination (Join-Path $packagePath 'licenses') -Recurse

Compress-Archive -LiteralPath $packagePath -DestinationPath $zipPath -CompressionLevel Optimal
$hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $zipPath).Hash.ToLowerInvariant()
Set-Content -LiteralPath (Join-Path $outputPath 'SHA256SUMS') -Encoding ascii -NoNewline -Value "$hash  $packageName.zip`n"

Write-Host "Package: $zipPath"
Write-Host "SHA-256: $hash"
