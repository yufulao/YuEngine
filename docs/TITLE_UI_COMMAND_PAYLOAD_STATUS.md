# L13 Title UI Command Payload Status

L13 converts original title-menu render bytecode into service-owned UI command payloads. It is
not a hand-written menu and it is not a final renderer backend.

## Contract

The runtime executes:

```text
project.json
-> startup baseline scripts
-> script/menu/titlemenu.b64.sqasm setupProc
-> one title main frame
-> renderProc
-> UI And 2D Render Service command state
```

The recovered path keeps title UI behavior attached to original bytecode:

- `th.makeLangPath` resolves language-specific title textures;
- `gMenu.loadGraphHandle`, `gMenu.drawGraph`, `gMenu.drawString`, and `gMenu.drawVersion`
  populate service state;
- `GraphString`, `GetStringSize`, `ColorFloat`, `DrawGraph`, and related helpers preserve
  text/layout/color/draw payloads;
- `ScrollWindow.drawList` materializes per-row `MenuObject` frame objects and invokes the
  recovered callback, instead of replacing the menu with a fake loop.

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe title-ui samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_title_ui_command_payload_contract --output-on-failure
```

Current metric:

```text
ok=true
title_setup_found=true
title_setup_executed=true
title_render_executed=true
entry=setupProc
created_objects=26
command_count=55
draw_commands=9
graph_string_commands=5
string_size_queries=5
text_draw_commands=6
graph_draw_commands=3
color_commands=11
localized_menu_text_commands=10
draw_list_item_commands=5
background_resource_bound=true
logo_resource_bound=true
unresolved_calls=0
truncated=false
```

Full regression gate:

```powershell
cmake -S . -B build\cmake-bt143
cmake --build build\cmake-bt143 --config Debug --clean-first -- -j1
ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure
python -m unittest discover -s tests
```

## Boundary

L13 proves title setup/render command payload recovery. It does not prove:

- full Squirrel VM semantics;
- interactive save/load/options branches;
- renderer backend submission;
- real font, shader, blend, texture upload, or D3D device behavior;
- original-game oracle frame parity.

Do not regress from this by writing a replacement title menu. Future work must consume this
payload.

## Next Edges

- L14 has since executed Continue, New Game, Load, Option, and Exit title branches through
  original bytecode and service inputs.
- L15 has since joined title/scene/actor/camera/input/event/audio/save service state into a
  gameplay-frame update contract.
- L16: build a backend-facing renderer command buffer that consumes both title UI payloads and
  scene mesh/material/texture handles.
