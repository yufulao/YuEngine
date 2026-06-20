# YuEngine UI Web Editor

Module path: `Tools/UiWebEditorWeb`

This directory contains the first Web/HTML UI data editor foundation. It is a
static browser tool for editing generic UI data files. It does not require a
C++ compile or relink cycle for editor UI iteration.

## Reference Used

Reference used: `C:\Users\yufulao\Desktop\oa\openagents\workspace`

Borrowed behavior:
- Fixed shell with left navigation, central work surface, and right detail pane.
- Dense toolbar commands and small panel headers.
- Clear separation between frontend state and backend/service boundaries.

Not copied:
- Next/React application structure.
- OpenAgents workspace API, agent model, or assets.
- Any server-owned collaborative behavior.

YuEngine-specific acceptance:
- The frontend edits generic UI data and exports runtime data.
- Engine runtime consumes exported data and has no dependency on this frontend.
- Existing `Tools/UiWebEditor*` C++ modules remain data/schema/protocol support.

## Data Boundary

| Layer | Owns | File or API shape |
| --- | --- | --- |
| Web frontend | hierarchy, inspector, canvas overlay, resource list, style/theme editing, state preview inputs | `Index.html`, `App.js`, `EditorModel.js`, `Style.css` |
| Editor draft | frontend selection and state preview inputs | `YuUiEditorDraft.json` export |
| Runtime data | schema, nodes with RectTransform fields, layouts, style refs, resource refs, event bindings, theme tokens | `YuUiRuntimeData.json` export |
| Local editor service | optional file IO and validation service boundary | existing `Tools/UiWebEditorService` |
| Engine runtime | data consumption and interpretation | no dependency on this directory |

## Layout Source

The runtime layout source of truth is the YuEngine RectTransform data shape:
`anchorMin`, `anchorMax`, `pivot`, `offsetMin`, `offsetMax`, `margin`,
`padding`, and `dpiScale`. Browser `left`, `top`, `width`, and `height`
values are adapter output for the editor canvas only.

The Web adapter resolves RectTransform data with the same arithmetic as
`UiRectMath`, then converts engine bottom-left runtime coordinates into
browser top-left canvas coordinates. Drag and resize operations run the inverse
adapter path and write RectTransform offsets back into the document.

Runtime export includes RectTransform data and strips editor-only viewport,
selection, state preview, canvas overlay, and browser preview state.

## Run

Direct browser mode:

```text
Tools/UiWebEditorWeb/Index.html
```

Optional static server:

```powershell
python -m http.server 8080 --directory Tools/UiWebEditorWeb
```

Then open:

```text
http://127.0.0.1:8080/Index.html
```

## Data Workflow

1. Open `Index.html`.
2. Edit hierarchy, node fields, theme tokens, resources, or state preview data.
3. Use `Save Draft` for editor draft data.
4. Use `Export Runtime` for runtime-facing data.
5. Import any compatible JSON through `Open`.

Sample input:

```text
Tools/UiWebEditorWeb/Samples/GenericUiLayout.json
```

## Validation

Run the focused frontend model tests:

```powershell
node Tests/UiWebEditorWeb/UiWebEditorWebTests.js
```

The tests validate RectTransform normalization, adapter forward conversion,
inverse canvas edit updates, runtime export boundaries, clone independence,
legacy rect migration, and the sample fixture.
