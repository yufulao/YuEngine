// 模块: Tools SceneWebEditorWeb
// 文件: Tools/SceneWebEditorWeb/App.js

(function StartSceneWebEditorWeb() {
    const model = window.SceneWebEditorModel;
    const STORAGE_THEME = "YuEngine.SceneWebEditor.Theme";
    const STORAGE_LAYOUT = "YuEngine.SceneWebEditor.Layout";
    const state = {
        document: model.CreateDefaultDocument(),
        showRuntimeData: false,
        theme: "light",
        objectDrag: null,
        layoutDrag: null,
        layout: {
            leftWidth: 300,
            rightWidth: 360,
            runtimeHeight: 240
        }
    };

    function GetElement(id) {
        return document.getElementById(id);
    }

    function Clear(element) {
        while (element.firstChild) {
            element.removeChild(element.firstChild);
        }
    }

    function SetText(id, value) {
        const element = GetElement(id);
        if (!element) {
            return;
        }
        element.textContent = value;
    }

    function ReadStorage(key) {
        try {
            return window.localStorage.getItem(key) || "";
        } catch (error) {
            return "";
        }
    }

    function WriteStorage(key, value) {
        try {
            window.localStorage.setItem(key, value);
            return true;
        } catch (error) {
            return false;
        }
    }

    function ReadJsonStorage(key, fallback) {
        const text = ReadStorage(key);
        if (!text) {
            return fallback;
        }
        try {
            return JSON.parse(text);
        } catch (error) {
            return fallback;
        }
    }

    function WriteJsonStorage(key, value) {
        return WriteStorage(key, JSON.stringify(value));
    }

    function ClampNumber(value, min_value, max_value) {
        if (value < min_value) {
            return min_value;
        }
        if (value > max_value) {
            return max_value;
        }
        return value;
    }

    function CreateElement(tag_name, class_name, text) {
        const element = document.createElement(tag_name);
        if (class_name) {
            element.className = class_name;
        }
        if (text) {
            element.textContent = text;
        }
        return element;
    }

    function ApplyLayout() {
        const workspace = GetElement("workspace");
        workspace.style.setProperty("--left-pane-width", String(state.layout.leftWidth) + "px");
        workspace.style.setProperty("--right-pane-width", String(state.layout.rightWidth) + "px");
        workspace.style.setProperty("--runtime-panel-height", String(state.layout.runtimeHeight) + "px");
        WriteJsonStorage(STORAGE_LAYOUT, state.layout);
    }

    function ApplyTheme(theme) {
        let next_theme = "light";
        if (theme === "dark") {
            next_theme = "dark";
        }
        state.theme = next_theme;
        document.body.dataset.theme = next_theme;
        WriteStorage(STORAGE_THEME, next_theme);
        SetText("theme-toggle-button", next_theme === "dark" ? "Light" : "Dark");
    }

    function MarkDirty() {
        state.document.editor.dirty = true;
        RenderSummary();
        RenderRuntimeJson();
    }

    function GetSelectedInspector() {
        return model.BuildInspector(state.document);
    }

    function SelectObject(object_id) {
        state.document = model.SelectObject(state.document, object_id);
        Render();
    }

    function UpdateSelectedObject(patch) {
        const inspector = GetSelectedInspector();
        if (!inspector) {
            return;
        }
        state.document = model.UpdateObject(state.document, inspector.object.objectId, patch);
        Render();
    }

    function UpdateSelectedTransform(patch) {
        const inspector = GetSelectedInspector();
        if (!inspector) {
            return;
        }
        state.document = model.UpdateTransform(state.document, inspector.object.objectId, patch);
        Render();
    }

    function UpdateSelectedResource(slot, resource_key) {
        const inspector = GetSelectedInspector();
        if (!inspector) {
            return;
        }
        state.document = model.SetResourceRef(state.document, inspector.object.objectId, slot, resource_key);
        Render();
    }

    function AppendInput(container, label_text, value, on_input) {
        const group = CreateElement("label", "field", "");
        const label = CreateElement("span", "", label_text);
        const input = CreateElement("input", "", "");
        input.value = value;
        input.addEventListener("input", function OnInputEvent() {
            on_input(input.value);
        });
        group.appendChild(label);
        group.appendChild(input);
        container.appendChild(group);
    }

    function AppendNumberInput(container, label_text, value, on_input) {
        const group = CreateElement("label", "field", "");
        const label = CreateElement("span", "", label_text);
        const input = CreateElement("input", "", "");
        input.type = "number";
        input.value = String(value);
        input.addEventListener("input", function OnInputEvent() {
            on_input(Number(input.value));
        });
        group.appendChild(label);
        group.appendChild(input);
        container.appendChild(group);
    }

    function AppendVector3Inputs(container, title, value, on_input) {
        const group = CreateElement("div", "field-grid", "");
        AppendNumberInput(group, title + " X", value.x, function UpdateX(next_value) {
            on_input({ x: next_value });
        });
        AppendNumberInput(group, title + " Y", value.y, function UpdateY(next_value) {
            on_input({ y: next_value });
        });
        AppendNumberInput(group, title + " Z", value.z, function UpdateZ(next_value) {
            on_input({ z: next_value });
        });
        container.appendChild(group);
    }

    function AppendCheckbox(container, label_text, checked, on_input) {
        const group = CreateElement("label", "field checkbox-field", "");
        const input = CreateElement("input", "", "");
        const label = CreateElement("span", "", label_text);
        input.type = "checkbox";
        input.checked = checked;
        input.addEventListener("change", function OnChangeEvent() {
            on_input(input.checked);
        });
        group.appendChild(input);
        group.appendChild(label);
        container.appendChild(group);
    }

    function AppendInspectorGroup(panel, title, group_key, build_body) {
        const group = CreateElement("details", "inspector-group", "");
        const summary = CreateElement("summary", "", title);
        const body = CreateElement("div", "inspector-group-body", "");
        const foldouts = state.document.editor.foldouts || {};
        group.open = foldouts[group_key] !== false;
        group.addEventListener("toggle", function OnToggleEvent() {
            state.document.editor.foldouts[group_key] = group.open;
            MarkDirty();
        });
        build_body(body);
        group.appendChild(summary);
        group.appendChild(body);
        panel.appendChild(group);
    }

    function RenderSummary() {
        const result = model.ValidateDocument(state.document);
        const runtime_document = model.BuildRuntimeDocument(state.document);
        const dirty_text = state.document.editor.dirty ? "Dirty" : "Saved";
        SetText("status-text", result.status + " / " + dirty_text);
        SetText("object-count", String(result.summary.objectCount));
        SetText("issue-count", String(result.summary.issueCount));
        SetText("document-hash", runtime_document.schema.documentHash);
    }

    function RenderObjectList() {
        const panel = GetElement("object-list");
        Clear(panel);
        const objects = model.BuildObjectList(state.document);
        objects.forEach(function RenderObject(record) {
            const row = CreateElement("button", "object-row", "");
            row.type = "button";
            row.dataset.selected = record.selected ? "true" : "false";
            row.addEventListener("click", function OnClickEvent() {
                SelectObject(record.objectId);
            });
            row.appendChild(CreateElement("span", "object-badge", "O"));
            row.appendChild(CreateElement("span", "object-name", record.displayName));
            row.appendChild(CreateElement("span", "object-id", "#" + record.objectId));
            panel.appendChild(row);
        });
    }

    function StartObjectDrag(event, item) {
        event.preventDefault();
        event.stopPropagation();
        state.objectDrag = {
            objectId: item.objectId,
            startX: event.clientX,
            startY: event.clientY,
            position: {
                x: item.position.x,
                y: item.position.y,
                z: item.position.z
            }
        };
        SelectObject(item.objectId);
    }

    function UpdateObjectDrag(event) {
        if (!state.objectDrag) {
            return;
        }
        const viewport = state.document.editor.viewport;
        const delta_x = (event.clientX - state.objectDrag.startX) / viewport.zoom;
        const delta_z = -(event.clientY - state.objectDrag.startY) / viewport.zoom;
        const position = {
            x: state.objectDrag.position.x + delta_x,
            y: state.objectDrag.position.y,
            z: state.objectDrag.position.z + delta_z
        };
        state.document = model.UpdateTransform(state.document, state.objectDrag.objectId, { position: position });
        Render();
    }

    function FinishObjectDrag() {
        state.objectDrag = null;
    }

    function RenderViewport() {
        const viewport = GetElement("scene-viewport");
        Clear(viewport);
        const editor_viewport = state.document.editor.viewport;
        viewport.style.backgroundSize = String(editor_viewport.zoom) + "px " + String(editor_viewport.zoom) + "px";
        SetText("viewport-scale-label", String(Math.round(editor_viewport.zoom)) + " px/u");
        const origin = CreateElement("div", "viewport-origin", "");
        viewport.appendChild(origin);
        const items = model.BuildViewportItems(state.document);
        items.forEach(function RenderViewportItem(item) {
            const node = CreateElement("button", "viewport-object", item.displayName);
            node.type = "button";
            node.dataset.selected = item.selected ? "true" : "false";
            node.dataset.enabled = item.enabled ? "true" : "false";
            node.style.left = String(item.left) + "px";
            node.style.top = String(item.top) + "px";
            node.addEventListener("click", function OnClickEvent(event) {
                event.stopPropagation();
                SelectObject(item.objectId);
            });
            node.addEventListener("pointerdown", function OnPointerDownEvent(event) {
                StartObjectDrag(event, item);
            });
            viewport.appendChild(node);
        });
    }

    function RenderResourceList() {
        const panel = GetElement("resource-list");
        Clear(panel);
        const refs = state.document.resourceRefs || [];
        if (refs.length === 0) {
            panel.appendChild(CreateElement("p", "empty-text", "No resource refs"));
            return;
        }
        refs.forEach(function RenderResourceRef(record) {
            const row = CreateElement("button", "resource-row", "");
            row.type = "button";
            row.addEventListener("click", function OnClickEvent() {
                SelectObject(record.objectId);
            });
            row.appendChild(CreateElement("span", "", record.slot));
            row.appendChild(CreateElement("span", "", record.resourceKey || "empty"));
            panel.appendChild(row);
        });
    }

    function RenderInspector() {
        const panel = GetElement("inspector-panel");
        Clear(panel);
        const inspector = model.BuildInspector(state.document);
        if (!inspector) {
            panel.appendChild(CreateElement("p", "empty-text", "No selection"));
            return;
        }

        AppendInspectorGroup(panel, "Scene Header", "scene", function BuildSceneGroup(body) {
            AppendInput(body, "Document ID", inspector.schema.documentId, function UpdateDocumentId(value) {
                state.document = model.UpdateSchema(state.document, { documentId: value });
                Render();
            });
            AppendInput(body, "Scene Key", inspector.schema.sceneKey, function UpdateSceneKey(value) {
                state.document = model.UpdateSchema(state.document, { sceneKey: value });
                Render();
            });
        });

        AppendInspectorGroup(panel, "Object Identity", "object", function BuildObjectGroup(body) {
            AppendInput(body, "Object Key", inspector.object.objectKey, function UpdateObjectKey(value) {
                UpdateSelectedObject({ objectKey: value });
            });
            AppendInput(body, "Display Name", inspector.object.displayName, function UpdateDisplayName(value) {
                UpdateSelectedObject({ displayName: value });
            });
            AppendNumberInput(body, "Order", inspector.object.order, function UpdateOrder(value) {
                UpdateSelectedObject({ order: value });
            });
            AppendCheckbox(body, "Enabled", inspector.object.enabled, function UpdateEnabled(value) {
                UpdateSelectedObject({ enabled: value });
            });
        });

        AppendInspectorGroup(panel, "Transform", "transform", function BuildTransformGroup(body) {
            AppendVector3Inputs(body, "Position", inspector.transform.position, function UpdatePosition(value) {
                const position = Object.assign({}, inspector.transform.position, value);
                UpdateSelectedTransform({ position: position });
            });
            AppendVector3Inputs(body, "Rotation", inspector.transform.rotation, function UpdateRotation(value) {
                const rotation = Object.assign({}, inspector.transform.rotation, value);
                UpdateSelectedTransform({ rotation: rotation });
            });
            AppendVector3Inputs(body, "Scale", inspector.transform.scale, function UpdateScale(value) {
                const scale = Object.assign({}, inspector.transform.scale, value);
                UpdateSelectedTransform({ scale: scale });
            });
        });

        AppendInspectorGroup(panel, "Resource Refs", "resources", function BuildResourceGroup(body) {
            model.RESOURCE_SLOTS.forEach(function RenderSlot(slot) {
                const current = inspector.resourceRefs.find(function MatchSlot(record) {
                    return record.slot === slot;
                });
                const value = current ? current.resourceKey : "";
                AppendInput(body, slot, value, function UpdateResource(value_text) {
                    UpdateSelectedResource(slot, value_text);
                });
            });
        });

        AppendInspectorGroup(panel, "Editor Sidecar", "editor", function BuildEditorGroup(body) {
            AppendCheckbox(body, "Grid", inspector.editor.grid.enabled, function UpdateGrid(value) {
                const grid = Object.assign({}, inspector.editor.grid, { enabled: value });
                state.document = model.UpdateEditor(state.document, { grid: grid });
                Render();
            });
            AppendCheckbox(body, "Snap", inspector.editor.snap.enabled, function UpdateSnap(value) {
                const snap = Object.assign({}, inspector.editor.snap, { enabled: value });
                state.document = model.UpdateEditor(state.document, { snap: snap });
                Render();
            });
        });
    }

    function RenderValidation() {
        const panel = GetElement("validation-panel");
        Clear(panel);
        const result = model.ValidateDocument(state.document);
        if (result.issues.length === 0) {
            panel.appendChild(CreateElement("p", "success-text", "No validation issues"));
            return;
        }
        result.issues.forEach(function RenderIssue(issue) {
            const row = CreateElement("div", "issue-row", "");
            row.appendChild(CreateElement("strong", "", issue.kind));
            row.appendChild(CreateElement("span", "", issue.message));
            row.appendChild(CreateElement("code", "", "object " + issue.objectId));
            panel.appendChild(row);
        });
    }

    function RenderRuntimeJson() {
        const workspace = GetElement("workspace");
        const panel = GetElement("runtime-data-panel");
        const toggle = GetElement("runtime-json-toggle-button");
        workspace.dataset.runtimeOpen = state.showRuntimeData ? "true" : "false";
        panel.dataset.open = state.showRuntimeData ? "true" : "false";
        toggle.setAttribute("aria-expanded", state.showRuntimeData ? "true" : "false");
        if (!state.showRuntimeData) {
            GetElement("runtime-json-preview").textContent = "";
            return;
        }
        const runtime_document = model.BuildRuntimeDocument(state.document);
        GetElement("runtime-json-preview").textContent = model.FormatJson(runtime_document);
    }

    function Render() {
        state.document = model.NormalizeDocument(state.document);
        RenderSummary();
        RenderObjectList();
        RenderViewport();
        RenderResourceList();
        RenderInspector();
        RenderValidation();
        RenderRuntimeJson();
    }

    function DownloadJson(file_name, payload) {
        const json = model.FormatJson(payload);
        const blob = new Blob([json], { type: "application/json" });
        const url = URL.createObjectURL(blob);
        const anchor = document.createElement("a");
        anchor.href = url;
        anchor.download = file_name;
        document.body.appendChild(anchor);
        anchor.click();
        anchor.remove();
        URL.revokeObjectURL(url);
    }

    function ImportDocument(file) {
        if (!file) {
            return;
        }
        const reader = new FileReader();
        reader.addEventListener("load", function OnLoadEvent() {
            try {
                const parsed = JSON.parse(String(reader.result));
                state.document = model.NormalizeDocument(parsed);
                state.document.editor.dirty = false;
                Render();
            } catch (error) {
                SetText("status-text", "Import failed");
            }
        });
        reader.readAsText(file);
    }

    function UpdateViewportZoom(zoom) {
        const viewport = Object.assign({}, state.document.editor.viewport, {
            zoom: ClampNumber(zoom, 12, 120)
        });
        state.document = model.UpdateEditor(state.document, { viewport: viewport });
        Render();
    }

    function FitViewport() {
        UpdateViewportZoom(40);
    }

    function ToggleRuntimeData(open) {
        state.showRuntimeData = open;
        RenderRuntimeJson();
    }

    function StartPaneResize(event, side) {
        event.preventDefault();
        state.layoutDrag = {
            side: side,
            startX: event.clientX,
            leftWidth: state.layout.leftWidth,
            rightWidth: state.layout.rightWidth
        };
        event.currentTarget.dataset.active = "true";
    }

    function UpdatePaneResize(event) {
        if (!state.layoutDrag) {
            return;
        }
        const delta_x = event.clientX - state.layoutDrag.startX;
        if (state.layoutDrag.side === "Left") {
            state.layout.leftWidth = ClampNumber(state.layoutDrag.leftWidth + delta_x, 240, 520);
        }
        if (state.layoutDrag.side === "Right") {
            state.layout.rightWidth = ClampNumber(state.layoutDrag.rightWidth - delta_x, 300, 560);
        }
        ApplyLayout();
    }

    function FinishPaneResize() {
        state.layoutDrag = null;
        GetElement("left-resizer").dataset.active = "false";
        GetElement("right-resizer").dataset.active = "false";
    }

    function BindToolbar() {
        GetElement("new-button").addEventListener("click", function OnClickEvent() {
            state.document = model.CreateDefaultDocument();
            Render();
        });
        GetElement("open-button").addEventListener("click", function OnClickEvent() {
            GetElement("file-input").click();
        });
        GetElement("file-input").addEventListener("change", function OnChangeEvent(event) {
            const file = event.target.files[0];
            ImportDocument(file);
            event.target.value = "";
        });
        GetElement("save-draft-button").addEventListener("click", function OnClickEvent() {
            state.document.editor.dirty = false;
            DownloadJson("YuSceneEditorDraft.json", state.document);
            Render();
        });
        GetElement("export-runtime-button").addEventListener("click", function OnClickEvent() {
            DownloadJson("YuSceneRuntimeData.json", model.BuildRuntimeDocument(state.document));
        });
        GetElement("runtime-json-toggle-button").addEventListener("click", function OnClickEvent() {
            ToggleRuntimeData(!state.showRuntimeData);
        });
        GetElement("runtime-json-close-button").addEventListener("click", function OnClickEvent() {
            ToggleRuntimeData(false);
        });
        GetElement("theme-toggle-button").addEventListener("click", function OnClickEvent() {
            ApplyTheme(state.theme === "dark" ? "light" : "dark");
        });
        GetElement("validate-button").addEventListener("click", function OnClickEvent() {
            RenderValidation();
            RenderSummary();
        });
        GetElement("add-object-button").addEventListener("click", function OnClickEvent() {
            state.document = model.AddObject(state.document);
            Render();
        });
        GetElement("duplicate-object-button").addEventListener("click", function OnClickEvent() {
            const inspector = GetSelectedInspector();
            if (!inspector) {
                return;
            }
            state.document = model.DuplicateObject(state.document, inspector.object.objectId);
            Render();
        });
        GetElement("remove-object-button").addEventListener("click", function OnClickEvent() {
            const inspector = GetSelectedInspector();
            if (!inspector) {
                return;
            }
            state.document = model.RemoveObject(state.document, inspector.object.objectId);
            Render();
        });
        GetElement("fit-view-button").addEventListener("click", FitViewport);
        GetElement("zoom-out-button").addEventListener("click", function OnClickEvent() {
            UpdateViewportZoom(state.document.editor.viewport.zoom - 8);
        });
        GetElement("zoom-reset-button").addEventListener("click", function OnClickEvent() {
            UpdateViewportZoom(40);
        });
        GetElement("zoom-in-button").addEventListener("click", function OnClickEvent() {
            UpdateViewportZoom(state.document.editor.viewport.zoom + 8);
        });
        GetElement("scene-viewport").addEventListener("click", function OnClickEvent() {
            const first = state.document.objects[0];
            if (!first) {
                return;
            }
            SelectObject(first.objectId);
        });
        GetElement("left-resizer").addEventListener("pointerdown", function OnPointerDownEvent(event) {
            StartPaneResize(event, "Left");
        });
        GetElement("right-resizer").addEventListener("pointerdown", function OnPointerDownEvent(event) {
            StartPaneResize(event, "Right");
        });
        window.addEventListener("pointermove", function OnPointerMoveEvent(event) {
            UpdatePaneResize(event);
            UpdateObjectDrag(event);
        });
        window.addEventListener("pointerup", function OnPointerUpEvent() {
            FinishPaneResize();
            FinishObjectDrag();
        });
        window.addEventListener("pointercancel", function OnPointerCancelEvent() {
            FinishPaneResize();
            FinishObjectDrag();
        });
    }

    function RestoreUserState() {
        state.layout = ReadJsonStorage(STORAGE_LAYOUT, state.layout);
        ApplyLayout();
        ApplyTheme(ReadStorage(STORAGE_THEME) || "light");
    }

    document.addEventListener("DOMContentLoaded", function OnDomReadyEvent() {
        RestoreUserState();
        BindToolbar();
        Render();
    });
})();
