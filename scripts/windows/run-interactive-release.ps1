$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$repositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$logDirectory = Join-Path $repositoryRoot 'dist'
$logPath = Join-Path $logDirectory 'windows-interactive-release.log'
New-Item -ItemType Directory -Path $logDirectory -Force | Out-Null
$formatFixtureDirectory = 'C:\Dev\audioab-format-fixtures'
if (Test-Path -LiteralPath $formatFixtureDirectory -PathType Container) {
    $env:AB_COMPARE_TEST_FORMAT_DIR = $formatFixtureDirectory
}

try {
    $buildScript = Join-Path $PSScriptRoot 'build-release.ps1'
    $ErrorActionPreference = 'Continue'
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $buildScript *> $logPath
    $buildExitCode = $LASTEXITCODE
    $ErrorActionPreference = 'Stop'
    if ($buildExitCode -ne 0) {
        throw "Interactive release build failed with exit code $buildExitCode"
    }
    exit 0
} catch {
    ($_ | Out-String) | Add-Content -LiteralPath $logPath -Encoding utf8
    exit 1
}
