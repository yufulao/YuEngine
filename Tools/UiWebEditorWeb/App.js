// 模块: Tools UiWebEditorWeb
// 文件: Tools/UiWebEditorWeb/App.js

(function StartUiWebEditorWeb() {
    const model = window.UiWebEditorModel;
    const state = {
        document: model.CreateDefaultDocument(),
        filter: "",
        drag: null,
        hierarchyDrag: null,
        showRuntimeData: false
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

    function MarkDirty() {
        state.document.editor.dirty = true;
        RenderSummary();
        RenderJsonPreview();
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
        panel.appendChild(CreateElement("h3", "inspector-section-title", "RectTransform"));
        AppendVector2Inputs(panel, "Anchor Min", rect_transform.anchorMin, function UpdateAnchorMin(value) {
            UpdateSelectedRectTransform({ anchorMin: value });
        });
        AppendVector2Inputs(panel, "Anchor Max", rect_transform.anchorMax, function UpdateAnchorMax(value) {
            UpdateSelectedRectTransform({ anchorMax: value });
        });
        AppendVector2Inputs(panel, "Pivot", rect_transform.pivot, function UpdatePivot(value) {
            UpdateSelectedRectTransform({ pivot: value });
        });
        AppendVector2Inputs(panel, "Offset Min", rect_transform.offsetMin, function UpdateOffsetMin(value) {
            UpdateSelectedRectTransform({ offsetMin: value });
        });
        AppendVector2Inputs(panel, "Offset Max", rect_transform.offsetMax, function UpdateOffsetMax(value) {
            UpdateSelectedRectTransform({ offsetMax: value });
        });
        AppendThicknessInputs(panel, "Margin", rect_transform.margin, function UpdateMargin(value) {
            UpdateSelectedRectTransform({ margin: value });
        });
        AppendThicknessInputs(panel, "Padding", rect_transform.padding, function UpdatePadding(value) {
            UpdateSelectedRectTransform({ padding: value });
        });
        AppendNumberInput(panel, "DPI Scale", rect_transform.dpiScale, function UpdateDpiScale(value) {
            UpdateSelectedRectTransform({ dpiScale: value });
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

        panel.appendChild(CreateElement("h3", "inspector-section-title", "Schema"));
        AppendNumberInput(panel, "Layout ID", state.document.schema.layoutId, function UpdateLayoutId(value) {
            state.document.schema.layoutId = value;
            MarkDirty();
        });
        AppendNumberInput(panel, "Root Node ID", state.document.schema.rootNodeId, function UpdateRootNodeId(value) {
            state.document.schema.rootNodeId = value;
            MarkDirty();
        });

        const node = inspector.node;
        panel.appendChild(CreateElement("h3", "inspector-section-title", "Node"));
        AppendInput(panel, "Name", node.name, function UpdateName(value) {
            UpdateSelectedNode({ name: value });
        });
        AppendSelect(panel, "Component", node.component, ["Container", "Text", "Image", "Button", "Slider", "Toggle"], function UpdateComponent(value) {
            UpdateSelectedNode({ component: value });
        });
        RenderParentInspector(panel, node, inspector.parentOptions);
        AppendInput(panel, "Text", node.text || "", function UpdateText(value) {
            UpdateSelectedNode({ text: value });
        });
        RenderRectTransformInspector(panel, node.rectTransform);

        AppendNumberInput(panel, "Layer", node.layer, function UpdateLayer(value) {
            UpdateSelectedNode({ layer: value });
        });
        AppendNumberInput(panel, "Order", node.siblingOrder, function UpdateOrder(value) {
            UpdateSelectedNode({ siblingOrder: value });
        });
        AppendCheckbox(panel, "Visible", node.visible, function UpdateVisible(value) {
            UpdateSelectedNode({ visible: value });
        });
        AppendCheckbox(panel, "Enabled", node.enabled, function UpdateEnabled(value) {
            UpdateSelectedNode({ enabled: value });
        });
        AppendCheckbox(panel, "Hit Test", node.hitTestable, function UpdateHitTest(value) {
            UpdateSelectedNode({ hitTestable: value });
        });

        const counts = CreateElement("div", "record-counts", "");
        counts.appendChild(CreateElement("span", "", "Style " + inspector.styleRefs.length));
        counts.appendChild(CreateElement("span", "", "Resource " + inspector.resourceRefs.length));
        counts.appendChild(CreateElement("span", "", "Event " + inspector.eventBindings.length));
        panel.appendChild(counts);
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
        GetElement("add-button-button").addEventListener("click", function OnClickEvent() {
            state.document = model.AddNode(state.document, "Button");
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
        window.addEventListener("pointermove", UpdateCanvasDrag);
        window.addEventListener("pointerup", FinishCanvasDrag);
        window.addEventListener("pointercancel", FinishCanvasDrag);
    }

    document.addEventListener("DOMContentLoaded", function OnDomReadyEvent() {
        BindToolbar();
        Render();
    });
})();
