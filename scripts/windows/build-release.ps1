[CmdletBinding()]
param(
    [string]$QtRoot = 'C:\Qt\6.9.3\msvc2022_64',
    [string]$VsDevCmd = 'C:\BuildTools\Common7\Tools\VsDevCmd.bat',
    [string]$CMake = 'C:\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe',
    [string]$Ninja = 'C:\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe'
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$buildDirectory = Join-Path $repositoryRoot 'build\windows-release'

foreach ($required in @($VsDevCmd, $CMake, $Ninja, (Join-Path $QtRoot 'bin\windeployqt.exe'))) {
    if (-not (Test-Path -LiteralPath $required -PathType Leaf)) {
        throw "Required build tool is missing: $required"
    }
}

$environmentLines = & $env:ComSpec /d /s /c "`"$VsDevCmd`" -no_logo -arch=amd64 -host_arch=amd64 >nul && set"
if ($LASTEXITCODE -ne 0) { throw "VsDevCmd failed with exit code $LASTEXITCODE" }
foreach ($line in $environmentLines) {
    if ($line -match '^([^=]+)=(.*)$') {
        Set-Item -Path "Env:$($Matches[1])" -Value $Matches[2]
    }
}
$env:PATH = "$($QtRoot)\bin;$env:PATH"

& $CMake -S $repositoryRoot -B $buildDirectory -G Ninja -DCMAKE_BUILD_TYPE=Release "-DCMAKE_MAKE_PROGRAM=$Ninja" "-DCMAKE_PREFIX_PATH=$QtRoot"
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed with exit code $LASTEXITCODE" }

& $CMake --build $buildDirectory --clean-first --parallel $env:NUMBER_OF_PROCESSORS
if ($LASTEXITCODE -ne 0) { throw "CMake build failed with exit code $LASTEXITCODE" }

& $CMake -E env "PATH=$env:PATH" ctest --test-dir $buildDirectory --output-on-failure --verbose
if ($LASTEXITCODE -ne 0) { throw "CTest failed with exit code $LASTEXITCODE" }

$executableCandidates = @(
    (Join-Path $buildDirectory 'ab-compare.exe'),
    (Join-Path $buildDirectory 'app\ab-compare.exe')
)
$executable = $executableCandidates | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1
if (-not $executable) { throw "Built executable was not found in: $($executableCandidates -join ', ')" }
$smokeStdout = Join-Path $buildDirectory 'smoke-test.stdout.txt'
$smokeStderr = Join-Path $buildDirectory 'smoke-test.stderr.txt'
$smokeProcess = Start-Process -FilePath $executable -ArgumentList '--smoke-test' -Wait -PassThru `
    -RedirectStandardOutput $smokeStdout -RedirectStandardError $smokeStderr
$smokeOutput = (Get-Content -LiteralPath $smokeStdout -Raw -ErrorAction SilentlyContinue) +
    (Get-Content -LiteralPath $smokeStderr -Raw -ErrorAction SilentlyContinue)
if ($smokeProcess.ExitCode -ne 0 -or $smokeOutput -notmatch 'SMOKE_VERSION=0\.3\.0-beta\.3') {
    throw "QML smoke test failed with exit code $($smokeProcess.ExitCode):`n$smokeOutput"
}
Write-Host $smokeOutput
Remove-Item -LiteralPath $smokeStdout, $smokeStderr -Force -ErrorAction SilentlyContinue

& (Join-Path $PSScriptRoot 'package-release.ps1') -BuildDirectory 'build\windows-release' -QtRoot $QtRoot
if ($LASTEXITCODE -ne 0) { throw "Packaging failed with exit code $LASTEXITCODE" }
