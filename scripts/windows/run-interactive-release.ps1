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
    & (Join-Path $PSScriptRoot 'build-release.ps1') *>&1 | Tee-Object -FilePath $logPath
    exit 0
} catch {
    ($_ | Out-String) | Add-Content -LiteralPath $logPath -Encoding utf8
    exit 1
}
