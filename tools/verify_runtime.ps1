param(
    [ValidateSet("full", "edge", "fast")]
    [string]$Mode = "fast",

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

function Invoke-CheckedCapture {
    param(
        [string]$Name,
        [string]$Exe,
        [string[]]$ArgumentList
    )

    Write-Host "==> $Name"
    $output = & $Exe @ArgumentList 2>&1
    $exitCode = $LASTEXITCODE
    if ($exitCode -ne 0) {
        $output | ForEach-Object { Write-Host $_ }
        throw "$Name failed with exit code $exitCode"
    }

    $metrics = $output | Select-String -Pattern '"metrics":' | Select-Object -First 1
    if ($null -ne $metrics) {
        Write-Host $metrics.Line.Trim()
    }
    $suite = $output | Select-String -Pattern '^contract-suite ' | Select-Object -First 1
    if ($null -ne $suite) {
        Write-Host $suite.Line.Trim()
    }
}

$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
Push-Location $repoRoot
try {
    if ($Mode -eq "edge" -and [string]::IsNullOrWhiteSpace($Filter)) {
        throw "Mode=edge requires -Filter, for example -Filter yuengine_backend_upload_binding_contract"
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

    $cliPath = Join-Path $BuildDir "yuengine_cli.exe"
    if (-not (Test-Path $cliPath)) {
        $cliPath = Join-Path (Join-Path $BuildDir $Config) "yuengine_cli.exe"
    }

    if ($Mode -eq "fast") {
        Invoke-CheckedCapture "runtime fast contract" $cliPath @(
            "backend-upload-bind",
            "samples\touhou_new_world\project.json",
            "--repo-root",
            "."
        )
    } elseif ($Mode -eq "edge") {
        Invoke-CheckedCapture "runtime contract suite edge" $cliPath @(
            "runtime-contract-suite",
            "samples\touhou_new_world\project.json",
            "--repo-root",
            ".",
            "--filter",
            $Filter
        )
    } else {
        $ctestArgs = @(
            "--test-dir",
            $BuildDir,
            "-C",
            $Config,
            "--output-on-failure",
            "--parallel",
            "$Jobs",
            "-R",
            "yuengine_runtime_contract_suite"
        )
        Invoke-Checked "ctest $Mode" "ctest" $ctestArgs
    }

    if (-not $SkipDiffCheck) {
        Invoke-Checked "git diff --check" "git" @("diff", "--check")
    }
} finally {
    Pop-Location
}
