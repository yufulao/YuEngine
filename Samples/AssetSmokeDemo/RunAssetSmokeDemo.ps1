param(
    [string]$Configuration = 'Debug',
    [string]$BuildDir = 'Build',
    [string]$UEEngineRoot = $env:UE_ENGINE_ROOT,
    [string]$EngineRoot = '',
    [int]$Seconds = 8
)

$ErrorActionPreference = 'Stop'

# Resolve the sample root from the script location.
$root = $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($root)) {
    $root = Split-Path -Parent $PSCommandPath
}
if ([string]::IsNullOrWhiteSpace($root)) {
    Write-Error "Missing script root."
    exit 1
}

$build = Join-Path $root $BuildDir
$configureArgs = @('-S', $root, '-B', $build)
if (-not [string]::IsNullOrWhiteSpace($UEEngineRoot)) {
    $configureArgs += "-DYU_ASSET_SMOKE_UE_ENGINE_ROOT=$UEEngineRoot"
}
if (-not [string]::IsNullOrWhiteSpace($EngineRoot)) {
    $configureArgs += "-DYU_ASSET_SMOKE_ENGINE_ROOT=$EngineRoot"
}

# Configure and build the sample target from source.
& cmake @configureArgs
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

& cmake --build $build --config $Configuration --target YuAssetSmokeDemo
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$exe = Join-Path $build "$Configuration\YuAssetSmokeDemo.exe"
if (-not (Test-Path -LiteralPath $exe)) {
    Write-Error "Missing demo executable: $exe"
    exit 1
}

# Run sample assets and emit one ignored capture output.
$assets = Join-Path $root 'Assets'
$music = Join-Path $assets 'Music\FormalBgm.ogg'
$capture = Join-Path $root 'FormalOggCapture.bmp'

& $exe --assets $assets --music $music --seconds $Seconds --capture $capture
exit $LASTEXITCODE
