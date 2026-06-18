param(
    [string]$Configuration = 'Debug',
    [string]$BuildDir = 'Build',
    [string]$UEEngineRoot = $env:UE_ENGINE_ROOT,
    [string]$EngineRoot = '',
    [int]$Seconds = 8
)

$ErrorActionPreference = 'Stop'

# 从脚本所在目录定位示例根目录。
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

# 先配置和构建示例目标，避免依赖仓库中预提交的可执行文件或 DLL。
cmake @configureArgs
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

cmake --build $build --config $Configuration --target YuAssetSmokeDemo
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$exe = Join-Path $build "$Configuration\YuAssetSmokeDemo.exe"
if (-not (Test-Path -LiteralPath $exe)) {
    Write-Error "Missing demo executable: $exe"
    exit 1
}

# 运行示例资源并输出一次截图，截图文件由 .gitignore 保护。
$assets = Join-Path $root 'Assets'
$music = Join-Path $assets 'Music\FormalBgm.ogg'
$capture = Join-Path $root 'FormalOggCapture.bmp'

& $exe --assets $assets --music $music --seconds $Seconds --capture $capture
exit $LASTEXITCODE
