YuAssetSmokeDemo

This folder is an isolated runtime smoke demo. It does not depend on YuEngine build outputs.

Run:
    powershell -ExecutionPolicy Bypass -File .\RunAssetSmokeDemo.ps1

Expected output:
    YuAssetSmokeDemo PASS mesh_vertices=36 texture=128x128 bgm=XAudio2/Vorbis seconds=8

The demo renders a textured mesh with D3D11 and loops FormalBgm.ogg through libogg/libvorbisfile decoding into XAudio2.
