param(
    [ValidateSet("full", "edge")]
    [string]$Mode = "full",

    [string]$BuildDir = "build\cmake-bt143",
    [string]$Config = "Debug",
    [int]$Jobs = 8,
    [string]$Filter = "",

    [switch]$CleanBuild,
    [switch]$NoBuild,
    [switch]$SkipPython,
    [switch]$SkipDiffCheck
)

$ErrorActionPreference = "Stop"

function Invoke-Checked {
    param(
        [string]$Name,
        [string]$Exe,
        [string[]]$ArgumentList
    )

    Write-Host "==> $Name"
    & $Exe @ArgumentList
    if ($LASTEXITCODE -ne 0) {
        throw "$Name failed with exit code $LASTEXITCODE"
    }
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
Push-Location $repoRoot
try {
    if ($Mode -eq "edge" -and [string]::IsNullOrWhiteSpace($Filter)) {
        throw "Mode=edge requires -Filter, for example -Filter yuengine_backend_device_adapter_contract"
    }

    if (-not $NoBuild) {
        $buildArgs = @("--build", $BuildDir, "--config", $Config, "--target", "yuengine_cli")
        if ($CleanBuild) {
            $buildArgs = @("--build", $BuildDir, "--config", $Config, "--clean-first", "--target", "yuengine_cli")
        }
        $buildArgs += @("--", "-j1")
        Invoke-Checked "cmake build" "cmake" $buildArgs
    }

    if (-not $SkipPython) {
        Invoke-Checked "python unittest" "python" @("-m", "unittest", "discover", "-s", "tests")
    }

    $ctestArgs = @("--test-dir", $BuildDir, "-C", $Config, "--output-on-failure", "--parallel", "$Jobs")
    if (-not [string]::IsNullOrWhiteSpace($Filter)) {
        $ctestArgs += @("-R", $Filter)
    }
    Invoke-Checked "ctest $Mode" "ctest" $ctestArgs

    if (-not $SkipDiffCheck) {
        Invoke-Checked "git diff --check" "git" @("diff", "--check")
    }
} finally {
    Pop-Location
}
