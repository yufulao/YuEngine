// 模块: Tools UiWebEditorWeb
// 文件: Tools/UiWebEditorWeb/App.js

(function StartUiWebEditorWeb() {
    const model = window.UiWebEditorModel;
    const STORAGE_THEME = "YuEngine.UiWebEditor.Theme";
    const STORAGE_LAYOUT = "YuEngine.UiWebEditor.Layout";
    const STORAGE_GROUPS = "YuEngine.UiWebEditor.InspectorGroups";
    const STORAGE_LIVE_DRAFT = "YuEngine.UiWebEditor.LiveDraft";
    const LIVE_CHANNEL = "YuEngine.UiWebEditor.LiveChannel";
    const instance_id = String(Date.now()) + "-" + Math.random().toString(16).slice(2);
    const state = {
        document: model.CreateDefaultDocument(),
        filter: "",
        drag: null,
        hierarchyDrag: null,
        layoutDrag: null,
        showRuntimeData: false,
        theme: "light",
        layout: {
            leftWidth: 300,
            rightWidth: 340,
            runtimeHeight: 240
        },
        collapsedGroups: {},
        liveDraftStamp: 0,
        liveSourceUrl: "",
        liveSourceText: "",
        liveFetchPending: false,
        liveChannel: null,
        suppressLivePublish: false
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
            const value = window.localStorage.getItem(key);
            if (!value) {
                return "";
            }
            return value;
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
        const text = JSON.stringify(value);
        return WriteStorage(key, text);
    }

    function ReadUrlParam(name) {
        const params = new URLSearchParams(window.location.search);
        return params.get(name) || "";
    }

    function MarkDirty() {
        state.document.editor.dirty = true;
        RenderSummary();
        RenderJsonPreview();
        PublishLiveDraft();
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

    function ApplyTheme(theme) {
        let next_theme = "light";
        if (theme === "dark") {
            next_theme = "dark";
        }
        state.theme = next_theme;
        document.body.dataset.theme = next_theme;
        WriteStorage(STORAGE_THEME, next_theme);
        const button = GetElement("theme-toggle-button");
        if (!button) {
            return;
        }
        button.textContent = next_theme === "dark" ? "Light" : "Dark";
        button.setAttribute("aria-pressed", next_theme === "dark" ? "true" : "false");
    }

    function ToggleTheme() {
        if (state.theme === "dark") {
            ApplyTheme("light");
            return;
        }
        ApplyTheme("dark");
    }

    function ApplyLayout() {
        const root = document.documentElement;
        root.style.setProperty("--left-pane-width", String(state.layout.leftWidth) + "px");
        root.style.setProperty("--right-pane-width", String(state.layout.rightWidth) + "px");
        root.style.setProperty("--runtime-pane-height", String(state.layout.runtimeHeight) + "px");
        WriteJsonStorage(STORAGE_LAYOUT, state.layout);
    }

    function LoadLayout() {
        const stored = ReadJsonStorage(STORAGE_LAYOUT, {});
        state.layout.leftWidth = ClampNumber(Number(stored.leftWidth || state.layout.leftWidth), 220, 560);
        state.layout.rightWidth = ClampNumber(Number(stored.rightWidth || state.layout.rightWidth), 260, 620);
        state.layout.runtimeHeight = ClampNumber(Number(stored.runtimeHeight || state.layout.runtimeHeight), 140, 460);
        ApplyLayout();
    }

    function LoadInspectorGroups() {
        const stored = ReadJsonStorage(STORAGE_GROUPS, {});
        if (!stored || typeof stored !== "object") {
            state.collapsedGroups = {};
            return;
        }
        state.collapsedGroups = stored;
    }

    function SaveInspectorGroups() {
        WriteJsonStorage(STORAGE_GROUPS, state.collapsedGroups);
    }

    function CreateInspectorGroup(container, key, title, default_open) {
        const details = CreateElement("details", "inspector-group", "");
        const summary = CreateElement("summary", "inspector-group-title", title);
        const body = CreateElement("div", "inspector-group-body", "");
        const has_saved_state = Object.prototype.hasOwnProperty.call(state.collapsedGroups, key);
        details.open = has_saved_state ? Boolean(state.collapsedGroups[key]) : default_open;
        details.dataset.groupKey = key;
        details.appendChild(summary);
        details.appendChild(body);
        details.addEventListener("toggle", function OnToggleEvent() {
            state.collapsedGroups[key] = details.open;
            SaveInspectorGroups();
        });
        container.appendChild(details);
        return body;
    }

    function GetLiveSourceUrl() {
        const source = ReadUrlParam("src");
        if (source) {
            return source;
        }
        return ReadUrlParam("document");
    }

    function PublishLiveDraft() {
        if (state.suppressLivePublish) {
            return;
        }
        const updated_at = Date.now();
        const record = {
            instanceId: instance_id,
            updatedAt: updated_at,
            document: model.NormalizeDocument(state.document)
        };
        state.liveDraftStamp = updated_at;
        WriteJsonStorage(STORAGE_LIVE_DRAFT, record);
        if (!state.liveChannel) {
            return;
        }
        state.liveChannel.postMessage({ updatedAt: updated_at });
    }

    function ApplyExternalDocument(document_value, status_text) {
        state.suppressLivePublish = true;
        state.document = model.NormalizeDocument(document_value);
        state.document.editor.dirty = false;
        Render();
        state.suppressLivePublish = false;
        if (status_text) {
            SetText("status-text", status_text);
        }
        return model.BuildRuntimeDocument(state.document);
    }

    function TryApplyStoredLiveDraft() {
        const record = ReadJsonStorage(STORAGE_LIVE_DRAFT, null);
        if (!record || !record.document) {
            return false;
        }
        if (record.instanceId === instance_id) {
            return false;
        }
        if (Number(record.updatedAt || 0) <= state.liveDraftStamp) {
            return false;
        }
        state.liveDraftStamp = Number(record.updatedAt || Date.now());
        ApplyExternalDocument(record.document, "Live data refreshed");
        return true;
    }

    function FetchLiveSource() {
        if (!state.liveSourceUrl) {
            return;
        }
        if (state.liveFetchPending) {
            return;
        }
        state.liveFetchPending = true;
        fetch(state.liveSourceUrl, { cache: "no-store" }).then(function OnResponse(response) {
            if (!response.ok) {
                return "";
            }
            return response.text();
        }).then(function OnText(text) {
            state.liveFetchPending = false;
            if (!text || text === state.liveSourceText) {
                return;
            }
            state.liveSourceText = text;
            const parsed = JSON.parse(text);
            ApplyExternalDocument(parsed, "Live source refreshed");
        }).catch(function OnError() {
            state.liveFetchPending = false;
        });
    }

    function RefreshLiveData() {
        const applied = TryApplyStoredLiveDraft();
        if (applied) {
            return;
        }
        FetchLiveSource();
    }

    function RefreshStyleSheets() {
        const links = document.querySelectorAll('link[rel="stylesheet"][href]');
        const stamp = String(Date.now());
        links.forEach(function RefreshLink(link) {
            const url = new URL(link.getAttribute("href"), window.location.href);
            url.searchParams.set("v", stamp);
            link.href = url.href;
        });
    }

    function RefreshEditorShell() {
        RefreshStyleSheets();
        RefreshLiveData();
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

    function GetSelectedNode() {
        const inspector = model.BuildInspector(state.document);
        if (!inspector) {
            return null;
        }
        return inspector.node;
    }

    function FindNodeRecord(document_value, node_id) {
        return document_value.nodes.find(function MatchNode(node) {
            return node.nodeId === node_id;
        }) || null;
    }

    function CompareNodeOrder(left, right) {
        if (left.siblingOrder !== right.siblingOrder) {
            return left.siblingOrder - right.siblingOrder;
        }
        return left.nodeId - right.nodeId;
    }

    function SelectNode(node_id) {
        state.document.editor.selectedNodeId = node_id;
        Render();
    }

    function UpdateSelectedNode(patch) {
        const selected = GetSelectedNode();
        if (!selected) {
            return;
        }
        state.document = model.UpdateNode(state.document, selected.nodeId, patch);
        Render();
    }

    function UpdateSelectedRectTransform(patch) {
        UpdateSelectedNode({ rectTransform: patch });
    }

    function UpdateSelectedComponent(component_key, patch) {
        const selected = GetSelectedNode();
        if (!selected) {
            return;
        }
        const components = Object.assign({}, selected.components || {});
        const current = Object.assign({}, components[component_key] || {});
        components[component_key] = Object.assign(current, patch);
        UpdateSelectedNode({ components: components });
    }

    function UpdateSelectedComponentNested(component_key, field_name, patch) {
        const selected = GetSelectedNode();
        if (!selected) {
            return;
        }
        const components = Object.assign({}, selected.components || {});
        const current = Object.assign({}, components[component_key] || {});
        const nested = Object.assign({}, current[field_name] || {}, patch);
        current[field_name] = nested;
        components[component_key] = current;
        UpdateSelectedNode({ components: components });
    }

    function UpdateNodeRecord(collection_name, node_id, default_record, patch) {
        const document_value = model.NormalizeDocument(state.document);
        const records = (document_value[collection_name] || []).slice();
        let record_index = records.findIndex(function MatchNode(record) {
            return Number(record.nodeId) === Number(node_id);
        });
        const current = record_index >= 0 ? records[record_index] : default_record;
        const next_record = Object.assign({}, current, patch);
        next_record.nodeId = node_id;
        if (record_index < 0) {
            record_index = records.length;
        }
        records[record_index] = next_record;
        document_value[collection_name] = records;
        document_value.editor.dirty = true;
        state.document = document_value;
        Render();
    }

    function UpdateStyleRef(node_id, patch) {
        UpdateNodeRecord("styleRefs", node_id, {
            nodeId: node_id,
            styleKey: 0,
            themeKey: 0,
            tokenKey: 0,
            valueKind: "ColorRgba8",
            value: "#ffffff",
            overridesTheme: false
        }, patch);
    }

    function UpdateResourceRef(node_id, patch) {
        UpdateNodeRecord("resourceRefs", node_id, {
            nodeId: node_id,
            kind: "Custom",
            resourceKey: 0,
            label: ""
        }, patch);
    }

    function UpdateEventBinding(node_id, patch) {
        UpdateNodeRecord("eventBindings", node_id, {
            nodeId: node_id,
            bindingKey: 0,
            eventKey: 0,
            command: ""
        }, patch);
    }

    function CreateCanvasRectFromDrag() {
        const width = Math.max(8, state.drag.canvasRect.width);
        const height = Math.max(8, state.drag.canvasRect.height);
        return {
            left: state.drag.canvasRect.left,
            top: state.drag.canvasRect.top,
            width: width,
            height: height
        };
    }

    function StartCanvasDrag(event, item, mode) {
        event.preventDefault();
        event.stopPropagation();
        state.drag = {
            mode: mode,
            nodeId: item.nodeId,
            startX: event.clientX,
            startY: event.clientY,
            canvasRect: {
                left: item.canvasRect.left,
                top: item.canvasRect.top,
                width: item.canvasRect.width,
                height: item.canvasRect.height
            }
        };
        SelectNode(item.nodeId);
    }

    function StartCanvasMove(event, item) {
        StartCanvasDrag(event, item, "Move");
    }

    function StartCanvasResize(event, item) {
        StartCanvasDrag(event, item, "Resize");
    }

    function UpdateCanvasDrag(event) {
        if (!state.drag) {
            return;
        }
        const delta_x = event.clientX - state.drag.startX;
        const delta_y = event.clientY - state.drag.startY;
        const next_rect = CreateCanvasRectFromDrag();
        if (state.drag.mode === "Move") {
            next_rect.left += delta_x;
            next_rect.top += delta_y;
            state.document = model.UpdateNodeFromCanvasRect(state.document, state.drag.nodeId, next_rect);
            Render();
            return;
        }
        if (state.drag.mode === "Resize") {
            next_rect.width = Math.max(8, next_rect.width + delta_x);
            next_rect.height = Math.max(8, next_rect.height + delta_y);
            state.document = model.UpdateNodeFromCanvasRect(state.document, state.drag.nodeId, next_rect);
            Render();
            return;
        }
    }

    function FinishCanvasDrag() {
        state.drag = null;
    }

    function StartLayoutDrag(event, mode) {
        event.preventDefault();
        state.layoutDrag = {
            mode: mode,
            startX: event.clientX,
            startY: event.clientY,
            leftWidth: state.layout.leftWidth,
            rightWidth: state.layout.rightWidth,
            runtimeHeight: state.layout.runtimeHeight
        };
        document.body.dataset.resizing = "true";
    }

    function UpdateLayoutDrag(event) {
        if (!state.layoutDrag) {
            return;
        }
        const delta_x = event.clientX - state.layoutDrag.startX;
        const delta_y = event.clientY - state.layoutDrag.startY;
        if (state.layoutDrag.mode === "Left") {
            state.layout.leftWidth = ClampNumber(state.layoutDrag.leftWidth + delta_x, 220, 560);
            ApplyLayout();
            return;
        }
        if (state.layoutDrag.mode === "Right") {
            state.layout.rightWidth = ClampNumber(state.layoutDrag.rightWidth - delta_x, 260, 620);
            ApplyLayout();
            return;
        }
        if (state.layoutDrag.mode === "Runtime") {
            state.layout.runtimeHeight = ClampNumber(state.layoutDrag.runtimeHeight - delta_y, 140, 460);
            ApplyLayout();
            return;
        }
    }

    function FinishLayoutDrag() {
        state.layoutDrag = null;
        document.body.dataset.resizing = "false";
    }

    function AdjustLayoutByKeyboard(mode, delta) {
        if (mode === "Left") {
            state.layout.leftWidth = ClampNumber(state.layout.leftWidth + delta, 220, 560);
            ApplyLayout();
            return;
        }
        if (mode === "Right") {
            state.layout.rightWidth = ClampNumber(state.layout.rightWidth - delta, 260, 620);
            ApplyLayout();
            return;
        }
        if (mode === "Runtime") {
            state.layout.runtimeHeight = ClampNumber(state.layout.runtimeHeight + delta, 140, 460);
            ApplyLayout();
            return;
        }
    }

    function HandleResizerKey(event, mode) {
        if (event.key !== "ArrowLeft" && event.key !== "ArrowRight" && event.key !== "ArrowUp" && event.key !== "ArrowDown") {
            return;
        }
        event.preventDefault();
        let delta = 16;
        if (event.key === "ArrowLeft" || event.key === "ArrowUp") {
            delta = -16;
        }
        AdjustLayoutByKeyboard(mode, delta);
    }

    function AppendInput(container, label_text, value, on_input) {
        const group = CreateElement("label", "field");
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

    function AppendColorInput(container, label_text, value, on_input) {
        const group = CreateElement("label", "field");
        const label = CreateElement("span", "", label_text);
        const input = CreateElement("input", "", "");
        input.type = "color";
        input.value = value;
        input.addEventListener("input", function OnInputEvent() {
            on_input(input.value);
        });
        group.appendChild(label);
        group.appendChild(input);
        container.appendChild(group);
    }

    function AppendNumberInput(container, label_text, value, on_input) {
        const group = CreateElement("label", "field");
        const label = CreateElement("span", "", label_text);
        const input = CreateElement("input", "", "");
        input.type = "number";
        input.value = String(value);
        input.addEventListener("input", function OnInputEvent() {
            const next_value = Number(input.value);
            on_input(next_value);
        });
        group.appendChild(label);
        group.appendChild(input);
        container.appendChild(group);
    }

    function AppendVector2Inputs(container, title, value, on_input) {
        const group = CreateElement("div", "field-grid");
        AppendNumberInput(group, title + " X", value.x, function UpdateX(next_value) {
            on_input({ x: next_value });
        });
        AppendNumberInput(group, title + " Y", value.y, function UpdateY(next_value) {
            on_input({ y: next_value });
        });
        container.appendChild(group);
    }

    function AppendThicknessInputs(container, title, value, on_input) {
        const group = CreateElement("div", "field-grid");
        AppendNumberInput(group, title + " L", value.left, function UpdateLeft(next_value) {
            on_input({ left: next_value });
        });
        AppendNumberInput(group, title + " T", value.top, function UpdateTop(next_value) {
            on_input({ top: next_value });
        });
        AppendNumberInput(group, title + " R", value.right, function UpdateRight(next_value) {
            on_input({ right: next_value });
        });
        AppendNumberInput(group, title + " B", value.bottom, function UpdateBottom(next_value) {
            on_input({ bottom: next_value });
        });
        container.appendChild(group);
    }

    function AppendCheckbox(container, label_text, checked, on_input) {
        const group = CreateElement("label", "field checkbox-field");
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

    function AppendSelect(container, label_text, value, values, on_input) {
        const group = CreateElement("label", "field");
        const label = CreateElement("span", "", label_text);
        const select = CreateElement("select", "", "");
        values.forEach(function AddOption(option_value) {
            const option = CreateElement("option", "", option_value);
            option.value = option_value;
            option.selected = option_value === value;
            select.appendChild(option);
        });
        select.addEventListener("change", function OnChangeEvent() {
            on_input(select.value);
        });
        group.appendChild(label);
        group.appendChild(select);
        container.appendChild(group);
    }

    function AppendSelectOptions(container, label_text, value, options, on_input) {
        const group = CreateElement("label", "field");
        const label = CreateElement("span", "", label_text);
        const select = CreateElement("select", "", "");
        options.forEach(function AddOption(option_record) {
            const option = CreateElement("option", "", option_record.label);
            option.value = String(option_record.value);
            option.selected = String(option_record.value) === String(value);
            select.appendChild(option);
        });
        select.addEventListener("change", function OnChangeEvent() {
            on_input(Number(select.value));
        });
        group.appendChild(label);
        group.appendChild(select);
        container.appendChild(group);
    }

    function RenderSummary() {
        const result = model.ValidateDocument(state.document);
        const dirty_text = state.document.editor.dirty ? "Dirty" : "Saved";
        SetText("status-text", result.status + " / " + dirty_text);
        SetText("node-count", String(result.summary.nodeCount));
        SetText("issue-count", String(result.summary.issueCount));
        SetText("layout-id", String(state.document.schema.layoutId));
    }

    function RenderHierarchy() {
        const list = GetElement("hierarchy-list");
        Clear(list);
        const items = model.BuildHierarchy(state.document);
        items.forEach(function RenderItem(item) {
            const row = CreateElement("div", "tree-row", "");
            row.tabIndex = 0;
            row.draggable = item.canDrag;
            row.setAttribute("role", "button");
            row.dataset.nodeId = String(item.nodeId);
            row.style.paddingLeft = String(12 + item.depth * 16) + "px";
            row.dataset.selected = item.selected ? "true" : "false";
            row.addEventListener("click", function OnClickEvent() {
                SelectNode(item.nodeId);
            });
            row.addEventListener("keydown", function OnKeyDownEvent(event) {
                if (event.key !== "Enter" && event.key !== " ") {
                    return;
                }
                event.preventDefault();
                SelectNode(item.nodeId);
            });
            row.addEventListener("dragstart", function OnDragStartEvent(event) {
                StartHierarchyDrag(event, item);
            });
            row.addEventListener("dragover", function OnDragOverEvent(event) {
                UpdateHierarchyDragOver(event, row);
            });
            row.addEventListener("dragleave", function OnDragLeaveEvent() {
                ClearHierarchyDropHint(row);
            });
            row.addEventListener("drop", function OnDropEvent(event) {
                DropHierarchyNode(event, item, row);
            });
            row.addEventListener("dragend", function OnDragEndEvent() {
                FinishHierarchyDrag();
            });
            const marker = CreateElement("span", "tree-marker", item.component.slice(0, 1).toUpperCase());
            const text = CreateElement("span", "tree-label", item.name);
            const meta = CreateElement("span", "tree-meta", "#" + item.nodeId);
            row.appendChild(marker);
            row.appendChild(text);
            row.appendChild(meta);
            list.appendChild(row);
        });
    }

    function GetHierarchyDropMode(event, row) {
        const rect = row.getBoundingClientRect();
        const local_y = event.clientY - rect.top;
        const third_height = rect.height / 3;
        if (local_y < third_height) {
            return "Before";
        }
        if (local_y > rect.height - third_height) {
            return "After";
        }
        return "Inside";
    }

    function ClearHierarchyDropHint(row) {
        row.dataset.dropMode = "";
    }

    function ClearHierarchyDropHints() {
        const rows = document.querySelectorAll(".tree-row");
        rows.forEach(function ClearRow(row) {
            ClearHierarchyDropHint(row);
        });
    }

    function StartHierarchyDrag(event, item) {
        if (!item.canDrag) {
            event.preventDefault();
            return;
        }
        state.hierarchyDrag = {
            nodeId: item.nodeId
        };
        event.dataTransfer.effectAllowed = "move";
        event.dataTransfer.setData("text/plain", String(item.nodeId));
    }

    function UpdateHierarchyDragOver(event, row) {
        if (!state.hierarchyDrag) {
            return;
        }
        event.preventDefault();
        row.dataset.dropMode = GetHierarchyDropMode(event, row);
        event.dataTransfer.dropEffect = "move";
    }

    function FinishHierarchyDrag() {
        state.hierarchyDrag = null;
        ClearHierarchyDropHints();
    }

    function ReadDraggedNodeId(event) {
        const raw_value = event.dataTransfer.getData("text/plain");
        const node_id = Number(raw_value);
        if (Number.isFinite(node_id)) {
            return node_id;
        }
        if (state.hierarchyDrag) {
            return state.hierarchyDrag.nodeId;
        }
        return 0;
    }

    function GetSiblingDropIndex(document_value, moving_id, target_id, insert_after) {
        const target = FindNodeRecord(document_value, target_id);
        if (!target) {
            return 0;
        }
        const siblings = document_value.nodes.filter(function MatchSibling(node) {
            if (node.nodeId === moving_id) {
                return false;
            }
            return node.parentId === target.parentId;
        }).sort(CompareNodeOrder);
        const target_index = siblings.findIndex(function MatchTarget(node) {
            return node.nodeId === target.nodeId;
        });
        if (target_index < 0) {
            return siblings.length;
        }
        if (insert_after) {
            return target_index + 1;
        }
        return target_index;
    }

    function ApplyHierarchyDrop(moving_id, target_id, mode) {
        if (moving_id === target_id) {
            return;
        }
        const document_value = model.NormalizeDocument(state.document);
        const target = FindNodeRecord(document_value, target_id);
        if (!target) {
            return;
        }
        let parent_id = target.nodeId;
        let sibling_index = document_value.nodes.filter(function MatchChild(node) {
            return node.parentId === target.nodeId;
        }).length;
        if (mode === "Before") {
            parent_id = target.parentId;
            sibling_index = GetSiblingDropIndex(document_value, moving_id, target_id, false);
        }
        if (mode === "After") {
            parent_id = target.parentId;
            sibling_index = GetSiblingDropIndex(document_value, moving_id, target_id, true);
        }
        if (parent_id <= 0) {
            return;
        }
        if (!model.CanMoveNode(document_value, moving_id, parent_id)) {
            return;
        }
        state.document = model.MoveNode(document_value, moving_id, parent_id, sibling_index);
        Render();
    }

    function DropHierarchyNode(event, item, row) {
        if (!state.hierarchyDrag) {
            return;
        }
        event.preventDefault();
        const moving_id = ReadDraggedNodeId(event);
        const mode = GetHierarchyDropMode(event, row);
        ApplyHierarchyDrop(moving_id, item.nodeId, mode);
        FinishHierarchyDrag();
    }

    function RenderCanvas() {
        const canvas = GetElement("canvas-surface");
        Clear(canvas);
        const viewport = state.document.editor.viewport;
        const width = viewport.runtimeRect.width * viewport.scale;
        const height = viewport.runtimeRect.height * viewport.scale;
        const grid_size = Math.max(8, 24 * viewport.scale);
        canvas.style.width = String(width) + "px";
        canvas.style.height = String(height) + "px";
        canvas.style.backgroundSize = String(grid_size) + "px " + String(grid_size) + "px";
        SetText("canvas-view-label", String(Math.round(viewport.scale * 100)) + "%");
        const items = model.BuildCanvasItems(state.document);
        items.forEach(function RenderCanvasItem(item) {
            const node = CreateElement("button", "canvas-node", "");
            const rect = item.canvasRect;
            node.type = "button";
            node.dataset.selected = item.selected ? "true" : "false";
            node.dataset.enabled = item.enabled ? "true" : "false";
            node.style.left = String(rect.left) + "px";
            node.style.top = String(rect.top) + "px";
            node.style.width = String(rect.width) + "px";
            node.style.height = String(rect.height) + "px";
            node.addEventListener("click", function OnClickEvent(event) {
                event.stopPropagation();
                SelectNode(item.nodeId);
            });
            node.addEventListener("pointerdown", function OnPointerDownEvent(event) {
                StartCanvasMove(event, item);
            });
            node.appendChild(CreateElement("span", "canvas-node-title", item.name));
            node.appendChild(CreateElement("span", "canvas-node-kind", item.component));
            if (item.previewTint) {
                node.style.color = item.previewTint;
            }
            node.appendChild(CreateElement("span", "canvas-node-preview", item.previewText));
            const handle = CreateElement("span", "resize-handle", "");
            handle.addEventListener("pointerdown", function OnPointerDownEvent(event) {
                StartCanvasResize(event, item);
            });
            node.appendChild(handle);
            canvas.appendChild(node);
        });
    }

    function RenderParentInspector(panel, node, parent_options) {
        if (!parent_options || parent_options.length === 0) {
            return;
        }
        const options = parent_options.map(function MapParentOption(option) {
            const indent = "  ".repeat(option.depth);
            return {
                value: option.nodeId,
                label: indent + option.name + " #" + option.nodeId
            };
        });
        AppendSelectOptions(panel, "Parent", node.parentId, options, function UpdateParent(value) {
            const document_value = model.NormalizeDocument(state.document);
            const siblings = document_value.nodes.filter(function MatchChild(child) {
                return child.parentId === value;
            });
            state.document = model.MoveNode(document_value, node.nodeId, value, siblings.length);
            Render();
        });
    }

    function RenderRectTransformInspector(panel, rect_transform) {
        const group = CreateInspectorGroup(panel, "rect-transform", "RectTransform", true);
        AppendVector2Inputs(group, "Anchor Min", rect_transform.anchorMin, function UpdateAnchorMin(value) {
            UpdateSelectedRectTransform({ anchorMin: value });
        });
        AppendVector2Inputs(group, "Anchor Max", rect_transform.anchorMax, function UpdateAnchorMax(value) {
            UpdateSelectedRectTransform({ anchorMax: value });
        });
        AppendVector2Inputs(group, "Pivot", rect_transform.pivot, function UpdatePivot(value) {
            UpdateSelectedRectTransform({ pivot: value });
        });
        AppendVector2Inputs(group, "Offset Min", rect_transform.offsetMin, function UpdateOffsetMin(value) {
            UpdateSelectedRectTransform({ offsetMin: value });
        });
        AppendVector2Inputs(group, "Offset Max", rect_transform.offsetMax, function UpdateOffsetMax(value) {
            UpdateSelectedRectTransform({ offsetMax: value });
        });
        AppendThicknessInputs(group, "Margin", rect_transform.margin, function UpdateMargin(value) {
            UpdateSelectedRectTransform({ margin: value });
        });
        AppendThicknessInputs(group, "Padding", rect_transform.padding, function UpdatePadding(value) {
            UpdateSelectedRectTransform({ padding: value });
        });
        AppendNumberInput(group, "DPI Scale", rect_transform.dpiScale, function UpdateDpiScale(value) {
            UpdateSelectedRectTransform({ dpiScale: value });
        });
    }

    function RenderCommonComponentInspector(panel, common) {
        const group = CreateInspectorGroup(panel, "component-common", "Common Runtime", true);
        AppendSelect(group, "Layout Type", common.layoutType, ["Absolute", "Stack", "Grid", "Overlay", "ScrollViewport"], function UpdateLayoutType(value) {
            UpdateSelectedComponent("common", { layoutType: value });
        });
        AppendNumberInput(group, "Style Key", common.styleKey, function UpdateStyleKey(value) {
            UpdateSelectedComponent("common", { styleKey: value });
        });
        AppendNumberInput(group, "Theme Key", common.themeKey, function UpdateThemeKey(value) {
            UpdateSelectedComponent("common", { themeKey: value });
        });
        AppendNumberInput(group, "Token Key", common.tokenKey, function UpdateTokenKey(value) {
            UpdateSelectedComponent("common", { tokenKey: value });
        });
        AppendNumberInput(group, "Resource Key", common.resourceKey, function UpdateResourceKey(value) {
            UpdateSelectedComponent("common", { resourceKey: value });
        });
        AppendNumberInput(group, "Event Key", common.eventKey, function UpdateEventKey(value) {
            UpdateSelectedComponent("common", { eventKey: value });
        });
        AppendSelect(group, "Hit Test Route", common.hitTestRoute, ["None", "Self", "Children"], function UpdateHitTestRoute(value) {
            UpdateSelectedComponent("common", { hitTestRoute: value });
        });
        AppendCheckbox(group, "Clip Children", common.clipChildren, function UpdateClipChildren(value) {
            UpdateSelectedComponent("common", { clipChildren: value });
        });
    }

    function RenderContainerComponentInspector(panel, container) {
        const group = CreateInspectorGroup(panel, "component-container", "Container", true);
        AppendSelect(group, "Container Layout", container.layoutType, ["Absolute", "Stack", "Grid", "Overlay", "ScrollViewport"], function UpdateLayoutType(value) {
            UpdateSelectedComponent("container", { layoutType: value });
        });
        AppendNumberInput(group, "Style Key", container.styleKey, function UpdateStyleKey(value) {
            UpdateSelectedComponent("container", { styleKey: value });
        });
        AppendCheckbox(group, "Scissor", container.scissor, function UpdateScissor(value) {
            UpdateSelectedComponent("container", { scissor: value });
        });
        AppendCheckbox(group, "Raycast Target", container.raycastTarget, function UpdateRaycastTarget(value) {
            UpdateSelectedComponent("container", { raycastTarget: value });
        });
    }

    function RenderTextComponentInspector(panel, text) {
        const group = CreateInspectorGroup(panel, "component-text", "Text", true);
        AppendSelect(group, "Content Mode", text.contentMode, ["Plain", "LocalizationKey"], function UpdateContentMode(value) {
            UpdateSelectedComponent("text", { contentMode: value });
        });
        AppendInput(group, "Content", text.content, function UpdateContent(value) {
            UpdateSelectedComponent("text", { content: value });
        });
        AppendInput(group, "Localization Key", text.localizationKey, function UpdateLocalizationKey(value) {
            UpdateSelectedComponent("text", { localizationKey: value });
        });
        AppendNumberInput(group, "Font Resource", text.fontResourceKey, function UpdateFontResource(value) {
            UpdateSelectedComponent("text", { fontResourceKey: value });
        });
        AppendNumberInput(group, "Fallback Font", text.fallbackFontResourceKey, function UpdateFallbackFont(value) {
            UpdateSelectedComponent("text", { fallbackFontResourceKey: value });
        });
        AppendNumberInput(group, "Font Size", text.fontSize, function UpdateFontSize(value) {
            UpdateSelectedComponent("text", { fontSize: value });
        });
        AppendSelect(group, "Font Style", text.fontStyle, ["Normal", "Bold", "Italic", "BoldItalic"], function UpdateFontStyle(value) {
            UpdateSelectedComponent("text", { fontStyle: value });
        });
        AppendSelect(group, "Horizontal Align", text.horizontalAlignment, ["Left", "Center", "Right"], function UpdateHorizontalAlignment(value) {
            UpdateSelectedComponent("text", { horizontalAlignment: value });
        });
        AppendSelect(group, "Vertical Align", text.verticalAlignment, ["Top", "Middle", "Bottom"], function UpdateVerticalAlignment(value) {
            UpdateSelectedComponent("text", { verticalAlignment: value });
        });
        AppendSelect(group, "Wrap", text.wrap, ["None", "Character"], function UpdateWrap(value) {
            UpdateSelectedComponent("text", { wrap: value });
        });
        AppendSelect(group, "Overflow", text.overflow, ["Clip"], function UpdateOverflow(value) {
            UpdateSelectedComponent("text", { overflow: value });
        });
        AppendNumberInput(group, "Line Height", text.lineHeight, function UpdateLineHeight(value) {
            UpdateSelectedComponent("text", { lineHeight: value });
        });
        AppendColorInput(group, "Tint", text.tint, function UpdateTint(value) {
            UpdateSelectedComponent("text", { tint: value });
        });
        AppendCheckbox(group, "Outline", text.outline.enabled, function UpdateOutline(value) {
            UpdateSelectedComponentNested("text", "outline", { enabled: value });
        });
        AppendColorInput(group, "Outline Color", text.outline.color, function UpdateOutlineColor(value) {
            UpdateSelectedComponentNested("text", "outline", { color: value });
        });
        AppendNumberInput(group, "Outline Width", text.outline.width, function UpdateOutlineWidth(value) {
            UpdateSelectedComponentNested("text", "outline", { width: value });
        });
        AppendCheckbox(group, "Shadow", text.shadow.enabled, function UpdateShadow(value) {
            UpdateSelectedComponentNested("text", "shadow", { enabled: value });
        });
        AppendColorInput(group, "Shadow Color", text.shadow.color, function UpdateShadowColor(value) {
            UpdateSelectedComponentNested("text", "shadow", { color: value });
        });
        AppendNumberInput(group, "Shadow X", text.shadow.offset.x, function UpdateShadowX(value) {
            const offset = Object.assign({}, text.shadow.offset, { x: value });
            UpdateSelectedComponentNested("text", "shadow", { offset: offset });
        });
        AppendNumberInput(group, "Shadow Y", text.shadow.offset.y, function UpdateShadowY(value) {
            const offset = Object.assign({}, text.shadow.offset, { y: value });
            UpdateSelectedComponentNested("text", "shadow", { offset: offset });
        });
        AppendNumberInput(group, "Material Key", text.materialKey, function UpdateMaterialKey(value) {
            UpdateSelectedComponent("text", { materialKey: value });
        });
        AppendNumberInput(group, "Style Key", text.styleKey, function UpdateStyleKey(value) {
            UpdateSelectedComponent("text", { styleKey: value });
        });
        AppendCheckbox(group, "Scissor", text.scissor, function UpdateScissor(value) {
            UpdateSelectedComponent("text", { scissor: value });
        });
        AppendCheckbox(group, "Raycast Target", text.raycastTarget, function UpdateRaycastTarget(value) {
            UpdateSelectedComponent("text", { raycastTarget: value });
        });
    }

    function RenderImageComponentInspector(panel, image) {
        const group = CreateInspectorGroup(panel, "component-image", "Image", true);
        AppendNumberInput(group, "Sprite Resource", image.spriteResourceKey, function UpdateSpriteResource(value) {
            UpdateSelectedComponent("image", { spriteResourceKey: value });
        });
        AppendNumberInput(group, "Atlas Key", image.atlasKey, function UpdateAtlasKey(value) {
            UpdateSelectedComponent("image", { atlasKey: value });
        });
        AppendNumberInput(group, "Material Key", image.materialKey, function UpdateMaterialKey(value) {
            UpdateSelectedComponent("image", { materialKey: value });
        });
        AppendNumberInput(group, "Style Key", image.styleKey, function UpdateStyleKey(value) {
            UpdateSelectedComponent("image", { styleKey: value });
        });
        AppendSelect(group, "Image Type", image.imageType, ["Simple", "Sliced"], function UpdateImageType(value) {
            UpdateSelectedComponent("image", { imageType: value });
        });
        AppendColorInput(group, "Tint", image.tint, function UpdateTint(value) {
            UpdateSelectedComponent("image", { tint: value });
        });
        AppendCheckbox(group, "Preserve Aspect", image.preserveAspect, function UpdatePreserveAspect(value) {
            UpdateSelectedComponent("image", { preserveAspect: value });
        });
        AppendThicknessInputs(group, "Nine Slice", image.nineSlice, function UpdateNineSlice(value) {
            const nine_slice = Object.assign({}, image.nineSlice, value);
            UpdateSelectedComponent("image", { nineSlice: nine_slice });
        });
        AppendCheckbox(group, "Scissor", image.scissor, function UpdateScissor(value) {
            UpdateSelectedComponent("image", { scissor: value });
        });
        AppendCheckbox(group, "Raycast Target", image.raycastTarget, function UpdateRaycastTarget(value) {
            UpdateSelectedComponent("image", { raycastTarget: value });
        });
    }

    function RenderButtonComponentInspector(panel, button) {
        const group = CreateInspectorGroup(panel, "component-button", "Button", true);
        AppendInput(group, "Label", button.label, function UpdateLabel(value) {
            UpdateSelectedComponent("button", { label: value });
        });
        AppendNumberInput(group, "Text Style", button.textStyleKey, function UpdateTextStyle(value) {
            UpdateSelectedComponent("button", { textStyleKey: value });
        });
        AppendNumberInput(group, "Image Style", button.imageStyleKey, function UpdateImageStyle(value) {
            UpdateSelectedComponent("button", { imageStyleKey: value });
        });
        AppendNumberInput(group, "Normal Style", button.normalStyleKey, function UpdateNormalStyle(value) {
            UpdateSelectedComponent("button", { normalStyleKey: value });
        });
        AppendNumberInput(group, "Hover Style", button.hoverStyleKey, function UpdateHoverStyle(value) {
            UpdateSelectedComponent("button", { hoverStyleKey: value });
        });
        AppendNumberInput(group, "Pressed Style", button.pressedStyleKey, function UpdatePressedStyle(value) {
            UpdateSelectedComponent("button", { pressedStyleKey: value });
        });
        AppendNumberInput(group, "Disabled Style", button.disabledStyleKey, function UpdateDisabledStyle(value) {
            UpdateSelectedComponent("button", { disabledStyleKey: value });
        });
        AppendNumberInput(group, "Selected Style", button.selectedStyleKey, function UpdateSelectedStyle(value) {
            UpdateSelectedComponent("button", { selectedStyleKey: value });
        });
        AppendNumberInput(group, "Normal Sprite", button.normalSpriteKey, function UpdateNormalSprite(value) {
            UpdateSelectedComponent("button", { normalSpriteKey: value });
        });
        AppendNumberInput(group, "Pressed Sprite", button.pressedSpriteKey, function UpdatePressedSprite(value) {
            UpdateSelectedComponent("button", { pressedSpriteKey: value });
        });
        AppendNumberInput(group, "Submit Event", button.submitEventKey, function UpdateSubmitEvent(value) {
            UpdateSelectedComponent("button", { submitEventKey: value });
        });
        AppendNumberInput(group, "Cancel Event", button.cancelEventKey, function UpdateCancelEvent(value) {
            UpdateSelectedComponent("button", { cancelEventKey: value });
        });
        AppendNumberInput(group, "Sound Resource", button.soundResourceKey, function UpdateSoundResource(value) {
            UpdateSelectedComponent("button", { soundResourceKey: value });
        });
        AppendInput(group, "Action Key", button.actionKey, function UpdateActionKey(value) {
            UpdateSelectedComponent("button", { actionKey: value });
        });
        AppendCheckbox(group, "Pointer Activation", button.pointerActivation, function UpdatePointerActivation(value) {
            UpdateSelectedComponent("button", { pointerActivation: value });
        });
        AppendCheckbox(group, "Keyboard Activation", button.keyboardActivation, function UpdateKeyboardActivation(value) {
            UpdateSelectedComponent("button", { keyboardActivation: value });
        });
        AppendCheckbox(group, "Gamepad Activation", button.gamepadActivation, function UpdateGamepadActivation(value) {
            UpdateSelectedComponent("button", { gamepadActivation: value });
        });
    }

    function RenderSliderComponentInspector(panel, slider) {
        const group = CreateInspectorGroup(panel, "component-slider", "Slider", true);
        AppendNumberInput(group, "Min", slider.minValue, function UpdateMinValue(value) {
            UpdateSelectedComponent("slider", { minValue: value });
        });
        AppendNumberInput(group, "Max", slider.maxValue, function UpdateMaxValue(value) {
            UpdateSelectedComponent("slider", { maxValue: value });
        });
        AppendNumberInput(group, "Value", slider.value, function UpdateValue(value) {
            UpdateSelectedComponent("slider", { value: value });
        });
        AppendNumberInput(group, "Step", slider.step, function UpdateStep(value) {
            UpdateSelectedComponent("slider", { step: value });
        });
        AppendSelect(group, "Axis", slider.axis, ["Horizontal", "Vertical"], function UpdateAxis(value) {
            UpdateSelectedComponent("slider", { axis: value });
        });
        AppendNumberInput(group, "Track Style", slider.trackStyleKey, function UpdateTrackStyle(value) {
            UpdateSelectedComponent("slider", { trackStyleKey: value });
        });
        AppendNumberInput(group, "Fill Style", slider.fillStyleKey, function UpdateFillStyle(value) {
            UpdateSelectedComponent("slider", { fillStyleKey: value });
        });
        AppendNumberInput(group, "Handle Style", slider.handleStyleKey, function UpdateHandleStyle(value) {
            UpdateSelectedComponent("slider", { handleStyleKey: value });
        });
        AppendNumberInput(group, "Track Sprite", slider.trackSpriteKey, function UpdateTrackSprite(value) {
            UpdateSelectedComponent("slider", { trackSpriteKey: value });
        });
        AppendNumberInput(group, "Fill Sprite", slider.fillSpriteKey, function UpdateFillSprite(value) {
            UpdateSelectedComponent("slider", { fillSpriteKey: value });
        });
        AppendNumberInput(group, "Handle Sprite", slider.handleSpriteKey, function UpdateHandleSprite(value) {
            UpdateSelectedComponent("slider", { handleSpriteKey: value });
        });
        AppendNumberInput(group, "Change Event", slider.changeEventKey, function UpdateChangeEvent(value) {
            UpdateSelectedComponent("slider", { changeEventKey: value });
        });
        AppendInput(group, "Increase Action", slider.increaseActionKey, function UpdateIncreaseAction(value) {
            UpdateSelectedComponent("slider", { increaseActionKey: value });
        });
        AppendInput(group, "Decrease Action", slider.decreaseActionKey, function UpdateDecreaseAction(value) {
            UpdateSelectedComponent("slider", { decreaseActionKey: value });
        });
        AppendCheckbox(group, "Pointer Drag", slider.pointerDrag, function UpdatePointerDrag(value) {
            UpdateSelectedComponent("slider", { pointerDrag: value });
        });
        AppendCheckbox(group, "Keyboard Adjust", slider.keyboardAdjust, function UpdateKeyboardAdjust(value) {
            UpdateSelectedComponent("slider", { keyboardAdjust: value });
        });
        AppendCheckbox(group, "Gamepad Adjust", slider.gamepadAdjust, function UpdateGamepadAdjust(value) {
            UpdateSelectedComponent("slider", { gamepadAdjust: value });
        });
    }

    function RenderComponentInspector(panel, node) {
        RenderCommonComponentInspector(panel, node.components.common);
        if (node.component === "Container") {
            RenderContainerComponentInspector(panel, node.components.container);
            return;
        }
        if (node.component === "Text") {
            RenderTextComponentInspector(panel, node.components.text);
            return;
        }
        if (node.component === "Image") {
            RenderImageComponentInspector(panel, node.components.image);
            return;
        }
        if (node.component === "Button") {
            RenderButtonComponentInspector(panel, node.components.button);
            return;
        }
        if (node.component === "Slider") {
            RenderSliderComponentInspector(panel, node.components.slider);
            return;
        }
        panel.appendChild(CreateElement("p", "empty-text", "Component requires native runtime support"));
    }

    function GetFirstRecord(records, fallback) {
        if (records.length > 0) {
            return records[0];
        }
        return fallback;
    }

    function RenderReferenceInspector(panel, node, inspector) {
        const style_ref = GetFirstRecord(inspector.styleRefs, {
            nodeId: node.nodeId,
            styleKey: 0,
            themeKey: 0,
            tokenKey: 0,
            valueKind: "ColorRgba8",
            value: "#ffffff",
            overridesTheme: false
        });
        const resource_ref = GetFirstRecord(inspector.resourceRefs, {
            nodeId: node.nodeId,
            kind: "Custom",
            resourceKey: 0,
            label: ""
        });
        const event_binding = GetFirstRecord(inspector.eventBindings, {
            nodeId: node.nodeId,
            bindingKey: 0,
            eventKey: 0,
            command: ""
        });
        const group = CreateInspectorGroup(panel, "references", "References", false);
        AppendNumberInput(group, "Style Ref Key", style_ref.styleKey, function UpdateStyleKey(value) {
            UpdateStyleRef(node.nodeId, { styleKey: value });
        });
        AppendNumberInput(group, "Style Theme", style_ref.themeKey, function UpdateThemeKey(value) {
            UpdateStyleRef(node.nodeId, { themeKey: value });
        });
        AppendNumberInput(group, "Style Token", style_ref.tokenKey, function UpdateTokenKey(value) {
            UpdateStyleRef(node.nodeId, { tokenKey: value });
        });
        AppendInput(group, "Style Value", style_ref.value, function UpdateStyleValue(value) {
            UpdateStyleRef(node.nodeId, { value: value });
        });
        AppendSelect(group, "Resource Kind", resource_ref.kind, ["Sprite", "Font", "Localization", "Audio", "Custom"], function UpdateKind(value) {
            UpdateResourceRef(node.nodeId, { kind: value });
        });
        AppendNumberInput(group, "Resource Key", resource_ref.resourceKey, function UpdateResourceKey(value) {
            UpdateResourceRef(node.nodeId, { resourceKey: value });
        });
        AppendInput(group, "Resource Label", resource_ref.label || "", function UpdateResourceLabel(value) {
            UpdateResourceRef(node.nodeId, { label: value });
        });
        AppendNumberInput(group, "Binding Key", event_binding.bindingKey, function UpdateBindingKey(value) {
            UpdateEventBinding(node.nodeId, { bindingKey: value });
        });
        AppendNumberInput(group, "Event Key", event_binding.eventKey, function UpdateEventKey(value) {
            UpdateEventBinding(node.nodeId, { eventKey: value });
        });
        AppendInput(group, "Command", event_binding.command || "", function UpdateCommand(value) {
            UpdateEventBinding(node.nodeId, { command: value });
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

        const schema_group = CreateInspectorGroup(panel, "schema", "Schema", false);
        AppendNumberInput(schema_group, "Layout ID", state.document.schema.layoutId, function UpdateLayoutId(value) {
            state.document.schema.layoutId = value;
            MarkDirty();
        });
        AppendNumberInput(schema_group, "Root Node ID", state.document.schema.rootNodeId, function UpdateRootNodeId(value) {
            state.document.schema.rootNodeId = value;
            MarkDirty();
        });

        const node = inspector.node;
        const node_group = CreateInspectorGroup(panel, "node", "Node", true);
        AppendInput(node_group, "Name", node.name, function UpdateName(value) {
            UpdateSelectedNode({ name: value });
        });
        AppendSelect(node_group, "Component", node.component, inspector.componentTypes, function UpdateComponent(value) {
            UpdateSelectedNode({ component: value });
        });
        RenderParentInspector(node_group, node, inspector.parentOptions);
        RenderRectTransformInspector(panel, node.rectTransform);

        AppendNumberInput(node_group, "Layer", node.layer, function UpdateLayer(value) {
            UpdateSelectedNode({ layer: value });
        });
        AppendNumberInput(node_group, "Order", node.siblingOrder, function UpdateOrder(value) {
            UpdateSelectedNode({ siblingOrder: value });
        });
        AppendCheckbox(node_group, "Visible", node.visible, function UpdateVisible(value) {
            UpdateSelectedNode({ visible: value });
        });
        AppendCheckbox(node_group, "Enabled", node.enabled, function UpdateEnabled(value) {
            UpdateSelectedNode({ enabled: value });
        });
        AppendCheckbox(node_group, "Hit Test", node.hitTestable, function UpdateHitTest(value) {
            UpdateSelectedNode({ hitTestable: value });
        });

        RenderComponentInspector(panel, node);
        RenderReferenceInspector(panel, node, inspector);

        const counts = CreateElement("div", "record-counts", "");
        counts.appendChild(CreateElement("span", "", "Style " + inspector.styleRefs.length));
        counts.appendChild(CreateElement("span", "", "Resource " + inspector.resourceRefs.length));
        counts.appendChild(CreateElement("span", "", "Event " + inspector.eventBindings.length));
        node_group.appendChild(counts);
    }

    function RenderTheme() {
        const panel = GetElement("theme-panel");
        Clear(panel);
        const tokens = state.document.theme.tokens || [];
        tokens.forEach(function RenderToken(token, index) {
            const row = CreateElement("div", "token-row", "");
            const name = CreateElement("input", "", "");
            const value = CreateElement("input", "", "");
            name.value = token.name;
            value.value = token.value;
            value.type = token.role === "Color" ? "color" : "text";
            name.addEventListener("input", function OnInputEvent() {
                token.name = name.value;
                state.document.theme.tokens[index] = token;
                MarkDirty();
            });
            value.addEventListener("input", function OnInputEvent() {
                token.value = value.value;
                state.document.theme.tokens[index] = token;
                MarkDirty();
            });
            row.appendChild(name);
            row.appendChild(value);
            panel.appendChild(row);
        });
    }

    function RenderResourcePanel() {
        const panel = GetElement("resource-panel");
        Clear(panel);
        const resources = state.document.resourceRefs || [];
        if (resources.length === 0) {
            panel.appendChild(CreateElement("p", "empty-text", "No resources"));
            return;
        }
        resources.forEach(function RenderResource(resource) {
            const row = CreateElement("button", "resource-row", "");
            row.type = "button";
            row.addEventListener("click", function OnClickEvent() {
                SelectNode(Number(resource.nodeId));
            });
            row.appendChild(CreateElement("span", "", String(resource.kind)));
            row.appendChild(CreateElement("span", "", String(resource.resourceKey)));
            row.appendChild(CreateElement("span", "", String(resource.label || "")));
            panel.appendChild(row);
        });
    }

    function RenderStatePreview() {
        const panel = GetElement("state-panel");
        Clear(panel);
        state.document.statePreview.forEach(function RenderStateInput(input, index) {
            const row = CreateElement("div", "state-row", "");
            const label = CreateElement("span", "", input.name);
            const value = CreateElement("input", "", "");
            value.value = String(input.value);
            value.addEventListener("input", function OnInputEvent() {
                input.value = value.value;
                state.document.statePreview[index] = input;
                MarkDirty();
            });
            row.appendChild(label);
            row.appendChild(value);
            panel.appendChild(row);
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
            row.appendChild(CreateElement("code", "", "node " + issue.nodeId));
            panel.appendChild(row);
        });
    }

    function RenderJsonPreview() {
        const center = GetElement("center-pane");
        const panel = GetElement("runtime-data-panel");
        const toggle = GetElement("runtime-json-toggle-button");
        center.dataset.runtimeOpen = state.showRuntimeData ? "true" : "false";
        panel.dataset.open = state.showRuntimeData ? "true" : "false";
        toggle.setAttribute("aria-expanded", state.showRuntimeData ? "true" : "false");
        if (!state.showRuntimeData) {
            GetElement("json-preview").textContent = "";
            return;
        }
        const runtime_document = model.BuildRuntimeDocument(state.document);
        const text = model.FormatJson(runtime_document);
        GetElement("json-preview").textContent = text;
    }

    function UpdateViewportScale(scale) {
        const viewport = state.document.editor.viewport;
        viewport.scale = ClampNumber(scale, 0.25, 2.5);
        viewport.panX = 0;
        viewport.panY = 0;
        state.document.editor.dirty = true;
        Render();
    }

    function FitCanvasToFrame() {
        const workbench = GetElement("canvas-workbench");
        if (!workbench) {
            return;
        }
        const viewport = state.document.editor.viewport;
        const width_scale = Math.max(1, workbench.clientWidth - 48) / viewport.runtimeRect.width;
        const height_scale = Math.max(1, workbench.clientHeight - 48) / viewport.runtimeRect.height;
        const next_scale = Math.min(width_scale, height_scale);
        UpdateViewportScale(next_scale);
    }

    function ToggleRuntimeData(open) {
        state.showRuntimeData = open;
        RenderJsonPreview();
    }

    function Render() {
        state.document = model.NormalizeDocument(state.document);
        RenderSummary();
        RenderHierarchy();
        RenderCanvas();
        RenderInspector();
        RenderTheme();
        RenderResourcePanel();
        RenderStatePreview();
        RenderValidation();
        RenderJsonPreview();
        PublishLiveDraft();
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
            DownloadJson("YuUiEditorDraft.json", state.document);
            Render();
        });
        GetElement("export-runtime-button").addEventListener("click", function OnClickEvent() {
            const runtime_document = model.BuildRuntimeDocument(state.document);
            DownloadJson("YuUiRuntimeData.json", runtime_document);
        });
        GetElement("validate-button").addEventListener("click", function OnClickEvent() {
            RenderValidation();
            RenderSummary();
        });
        GetElement("theme-toggle-button").addEventListener("click", function OnClickEvent() {
            ToggleTheme();
        });
        GetElement("live-refresh-button").addEventListener("click", function OnClickEvent() {
            RefreshEditorShell();
        });
        GetElement("runtime-json-toggle-button").addEventListener("click", function OnClickEvent() {
            ToggleRuntimeData(!state.showRuntimeData);
        });
        GetElement("runtime-json-close-button").addEventListener("click", function OnClickEvent() {
            ToggleRuntimeData(false);
        });
        GetElement("canvas-fit-button").addEventListener("click", function OnClickEvent() {
            FitCanvasToFrame();
        });
        GetElement("canvas-reset-view-button").addEventListener("click", function OnClickEvent() {
            UpdateViewportScale(1);
        });
        GetElement("canvas-zoom-out-button").addEventListener("click", function OnClickEvent() {
            const scale = state.document.editor.viewport.scale - 0.1;
            UpdateViewportScale(scale);
        });
        GetElement("canvas-zoom-in-button").addEventListener("click", function OnClickEvent() {
            const scale = state.document.editor.viewport.scale + 0.1;
            UpdateViewportScale(scale);
        });
        GetElement("add-container-button").addEventListener("click", function OnClickEvent() {
            state.document = model.AddNode(state.document, "Container");
            Render();
        });
        GetElement("add-text-button").addEventListener("click", function OnClickEvent() {
            state.document = model.AddNode(state.document, "Text");
            Render();
        });
        GetElement("add-image-button").addEventListener("click", function OnClickEvent() {
            state.document = model.AddNode(state.document, "Image");
            Render();
        });
        GetElement("add-button-button").addEventListener("click", function OnClickEvent() {
            state.document = model.AddNode(state.document, "Button");
            Render();
        });
        GetElement("add-slider-button").addEventListener("click", function OnClickEvent() {
            state.document = model.AddNode(state.document, "Slider");
            Render();
        });
        GetElement("remove-node-button").addEventListener("click", function OnClickEvent() {
            const selected = GetSelectedNode();
            if (!selected) {
                return;
            }
            state.document = model.RemoveNode(state.document, selected.nodeId);
            Render();
        });
        GetElement("canvas-surface").addEventListener("click", function OnClickEvent() {
            SelectNode(state.document.schema.rootNodeId);
        });
        GetElement("left-pane-resizer").addEventListener("pointerdown", function OnPointerDownEvent(event) {
            StartLayoutDrag(event, "Left");
        });
        GetElement("right-pane-resizer").addEventListener("pointerdown", function OnPointerDownEvent(event) {
            StartLayoutDrag(event, "Right");
        });
        GetElement("runtime-pane-resizer").addEventListener("pointerdown", function OnPointerDownEvent(event) {
            StartLayoutDrag(event, "Runtime");
        });
        GetElement("left-pane-resizer").addEventListener("keydown", function OnKeyDownEvent(event) {
            HandleResizerKey(event, "Left");
        });
        GetElement("right-pane-resizer").addEventListener("keydown", function OnKeyDownEvent(event) {
            HandleResizerKey(event, "Right");
        });
        GetElement("runtime-pane-resizer").addEventListener("keydown", function OnKeyDownEvent(event) {
            HandleResizerKey(event, "Runtime");
        });
        window.addEventListener("pointermove", UpdateCanvasDrag);
        window.addEventListener("pointermove", UpdateLayoutDrag);
        window.addEventListener("pointerup", FinishCanvasDrag);
        window.addEventListener("pointerup", FinishLayoutDrag);
        window.addEventListener("pointercancel", FinishCanvasDrag);
        window.addEventListener("pointercancel", FinishLayoutDrag);
    }

    function BindLiveRefresh() {
        state.liveSourceUrl = GetLiveSourceUrl();
        window.addEventListener("storage", function OnStorageEvent(event) {
            if (event.key !== STORAGE_LIVE_DRAFT) {
                return;
            }
            RefreshLiveData();
        });
        window.addEventListener("focus", function OnFocusEvent() {
            RefreshEditorShell();
        });
        document.addEventListener("visibilitychange", function OnVisibilityChangeEvent() {
            if (document.visibilityState !== "visible") {
                return;
            }
            RefreshEditorShell();
        });
        if ("BroadcastChannel" in window) {
            state.liveChannel = new BroadcastChannel(LIVE_CHANNEL);
            state.liveChannel.addEventListener("message", function OnMessageEvent() {
                RefreshLiveData();
            });
        }
        window.setInterval(function OnPollEvent() {
            if (!document.hasFocus()) {
                return;
            }
            RefreshLiveData();
        }, 1500);
    }

    function ExposeExternalApi() {
        window.YuUiWebEditorApplyDocument = function ApplyDocument(document_value) {
            return ApplyExternalDocument(document_value, "External data applied");
        };
        window.YuUiWebEditor = {
            ApplyDocument: window.YuUiWebEditorApplyDocument,
            GetDocument: function GetDocument() {
                return model.NormalizeDocument(state.document);
            },
            GetRuntimeDocument: function GetRuntimeDocument() {
                return model.BuildRuntimeDocument(state.document);
            },
            RefreshLiveData: RefreshLiveData,
            RefreshEditorShell: RefreshEditorShell,
            SetTheme: ApplyTheme
        };
    }

    document.addEventListener("DOMContentLoaded", function OnDomReadyEvent() {
        const stored_theme = ReadStorage(STORAGE_THEME);
        ApplyTheme(stored_theme);
        LoadLayout();
        LoadInspectorGroups();
        BindLiveRefresh();
        ExposeExternalApi();
        BindToolbar();
        RefreshLiveData();
        Render();
    });
})();
