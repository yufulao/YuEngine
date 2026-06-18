$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$exe = Join-Path $root 'Build\Debug\YuAssetSmokeDemo.exe'
$assets = Join-Path $root 'Assets'
$music = Join-Path $assets 'Music\FormalBgm.ogg'
$capture = Join-Path $root 'FormalOggCapture.bmp'

& $exe --assets $assets --music $music --seconds 8 --capture $capture
exit $LASTEXITCODE
