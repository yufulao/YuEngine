# VFS Resource Status

Lane: `project-runtime`

This document tracks L2 VFS And Pack Manifest Depth.

## Implemented

- Pack-manifest resource entries keep normalized path, pack id, size, relative offset, and absolute offset.
- Loose resources keep normalized path, physical path, and size.
- Stem resolution returns stable sorted results across pack and loose mounts.
- `yuengine_cli resources <project.json>` emits resource diagnostics for:
  - manifest `resources.required`;
  - oracle title script resource refs;
  - oracle first-mission script resource refs.
- `.sqasm` resource ref extraction rejects root paths, parent paths, whitespace/control literals, and unresolved no-extension script keys.
- Windows path canonicalization uses error-code fallback so inaccessible system files do not fail project resource diagnostics.

## Verified On 2026-06-09

- Original sample resource report:
  - `ok`: true
  - loose mounts: 2
  - pack-manifest resources: 13,028
  - required resources: 5
  - required missing: 0
  - title background stem: 8 results, pack plus merged loose, including `menu/title/title_back_sc.dds`
  - title logo stem: 8 results, pack plus merged loose, including `menu/title/logo_sc.dds`
  - title DLC stem from script: 8 results, including `menu/title/dlc_sc.dds`
  - title script resource refs: 8, unresolved: 0
  - first mission script resource refs: 3, unresolved: 0
  - first mission stage: `map/doujou/doujou.sge`
  - first mission rail camera: `map/doujou/doujou.rcm`
- Empty sample resource report:
  - `ok`: true
  - required resources: 0
  - script modules: 0

## Verification Commands

Use `--clean-first` with the current Ninja/MSVC build on this machine. CMake's localized
`/showIncludes` dependency prefix is mis-encoded, so header changes can otherwise leave stale
object files.

```powershell
cmd.exe /d /s /c '"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 -vcvars_ver=14.43 && "D:\app\CMake\bin\cmake.exe" -S . -B build\cmake-bt143 -G Ninja -DCMAKE_MAKE_PROGRAM="D:/opt/Python311/Scripts/ninja.exe" && "D:\app\CMake\bin\cmake.exe" --build build\cmake-bt143 --clean-first && "D:\app\CMake\bin\ctest.exe" --test-dir build\cmake-bt143 --output-on-failure'
build\cmake-bt143\yuengine_cli.exe resources samples\touhou_new_world\project.json
build\cmake-bt143\yuengine_cli.exe resources samples\empty_project\project.json
python -m unittest discover -s tests
```

## Boundary

This is dependency diagnostics, not asset loading or rendering.

The report proves that title and first-mission resource references resolve through the project VFS.
It does not parse DDS/SGE/RCM payloads or execute native resource APIs yet.
