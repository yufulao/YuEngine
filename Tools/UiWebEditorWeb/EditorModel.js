// 模块: Tools UiWebEditorWeb
// 文件: Tools/UiWebEditorWeb/EditorModel.js

(function RegisterUiWebEditorModel(root, factory) {
    const api = factory();
    if (typeof module !== "undefined" && module.exports) {
        module.exports = api;
    }
    if (root) {
        root.UiWebEditorModel = api;
    }
})(typeof globalThis !== "undefined" ? globalThis : this, function CreateUiWebEditorModel() {
    const SCHEMA_ID = "YUFS";
    const SCHEMA_VERSION = 1;
    const RESOURCE_KINDS = ["Sprite", "Font", "Localization", "Audio", "Custom"];
    const DEFAULT_VIEWPORT_RECT = { x: 0, y: 0, width: 960, height: 540 };

    function Clone(value) {
        return JSON.parse(JSON.stringify(value));
    }

    function ToNumber(value, fallback) {
        const number_value = Number(value);
        if (Number.isFinite(number_value)) {
            return number_value;
        }
        return fallback;
    }

    function ToBool(value, fallback) {
        if (typeof value === "boolean") {
            return value;
        }
        return fallback;
    }

    function GetArray(value) {
        if (Array.isArray(value)) {
            return value;
        }
        return [];
    }

    function CreateVector2(x, y) {
        return {
            x: x,
            y: y
        };
    }

    function CreateThickness(left, top, right, bottom) {
        return {
            left: left,
            top: top,
            right: right,
            bottom: bottom
        };
    }

    function CreateRect(x, y, width, height) {
        return {
            x: x,
            y: y,
            width: width,
            height: height
        };
    }

    function CreateFullStretchRectTransform() {
        return {
            anchorMin: CreateVector2(0, 0),
            anchorMax: CreateVector2(1, 1),
            pivot: CreateVector2(0.5, 0.5),
            offsetMin: CreateVector2(0, 0),
            offsetMax: CreateVector2(0, 0),
            margin: CreateThickness(0, 0, 0, 0),
            padding: CreateThickness(0, 0, 0, 0),
            dpiScale: 1
        };
    }

    function NormalizeVector2(value, fallback) {
        const source = value || fallback || {};
        return {
            x: ToNumber(source.x, 0),
            y: ToNumber(source.y, 0)
        };
    }

    function NormalizeThickness(value, fallback) {
        const source = value || fallback || {};
        return {
            left: ToNumber(source.left, 0),
            top: ToNumber(source.top, 0),
            right: ToNumber(source.right, 0),
            bottom: ToNumber(source.bottom, 0)
        };
    }

    function NormalizeRect(value, fallback) {
        const source = value || fallback || DEFAULT_VIEWPORT_RECT;
        return {
            x: ToNumber(source.x, DEFAULT_VIEWPORT_RECT.x),
            y: ToNumber(source.y, DEFAULT_VIEWPORT_RECT.y),
            width: Math.max(1, ToNumber(source.width, DEFAULT_VIEWPORT_RECT.width)),
            height: Math.max(1, ToNumber(source.height, DEFAULT_VIEWPORT_RECT.height))
        };
    }

    function NormalizeRectTransform(value) {
        const full_stretch = CreateFullStretchRectTransform();
        const source = value || full_stretch;
        return {
            anchorMin: NormalizeVector2(source.anchorMin, full_stretch.anchorMin),
            anchorMax: NormalizeVector2(source.anchorMax, full_stretch.anchorMax),
            pivot: NormalizeVector2(source.pivot, full_stretch.pivot),
            offsetMin: NormalizeVector2(source.offsetMin, full_stretch.offsetMin),
            offsetMax: NormalizeVector2(source.offsetMax, full_stretch.offsetMax),
            margin: NormalizeThickness(source.margin, full_stretch.margin),
            padding: NormalizeThickness(source.padding, full_stretch.padding),
            dpiScale: Math.max(0.001, ToNumber(source.dpiScale, full_stretch.dpiScale))
        };
    }

    function ResolveRectTransform(parent_rect, transform) {
        const parent = NormalizeRect(parent_rect, DEFAULT_VIEWPORT_RECT);
        const rect_transform = NormalizeRectTransform(transform);
        const dpi_scale = rect_transform.dpiScale;
        const margin = rect_transform.margin;
        const padding = rect_transform.padding;
        const left_anchor = parent.x + parent.width * rect_transform.anchorMin.x;
        const bottom_anchor = parent.y + parent.height * rect_transform.anchorMin.y;
        const right_anchor = parent.x + parent.width * rect_transform.anchorMax.x;
        const top_anchor = parent.y + parent.height * rect_transform.anchorMax.y;
        const left = left_anchor + rect_transform.offsetMin.x * dpi_scale + margin.left * dpi_scale;
        const bottom = bottom_anchor + rect_transform.offsetMin.y * dpi_scale + margin.bottom * dpi_scale;
        const right = right_anchor + rect_transform.offsetMax.x * dpi_scale - margin.right * dpi_scale;
        const top = top_anchor + rect_transform.offsetMax.y * dpi_scale - margin.top * dpi_scale;
        const width = right - left;
        const height = top - bottom;
        const content_left = left + padding.left * dpi_scale;
        const content_bottom = bottom + padding.bottom * dpi_scale;
        const content_width = width - (padding.left + padding.right) * dpi_scale;
        const content_height = height - (padding.top + padding.bottom) * dpi_scale;
        return {
            status: width >= 0 && height >= 0 && content_width >= 0 && content_height >= 0 ? "Success" : "InvalidOutputRect",
            rect: CreateRect(left, bottom, width, height),
            contentRect: CreateRect(content_left, content_bottom, content_width, content_height),
            pivotPoint: CreateVector2(left + width * rect_transform.pivot.x, bottom + height * rect_transform.pivot.y)
        };
    }

    function EngineRectToCanvasRect(engine_rect, viewport) {
        const view = NormalizeEditorViewport(viewport);
        const rect = NormalizeRect(engine_rect, DEFAULT_VIEWPORT_RECT);
        const view_top = view.runtimeRect.y + view.runtimeRect.height;
        return {
            left: (rect.x - view.runtimeRect.x) * view.scale + view.panX,
            top: (view_top - rect.y - rect.height) * view.scale + view.panY,
            width: rect.width * view.scale,
            height: rect.height * view.scale
        };
    }

    function CanvasRectToEngineRect(canvas_rect, viewport) {
        const view = NormalizeEditorViewport(viewport);
        const rect = canvas_rect || {};
        const left = ToNumber(rect.left, 0);
        const top = ToNumber(rect.top, 0);
        const width = Math.max(1, ToNumber(rect.width, 1));
        const height = Math.max(1, ToNumber(rect.height, 1));
        const engine_width = width / view.scale;
        const engine_height = height / view.scale;
        const view_top = view.runtimeRect.y + view.runtimeRect.height;
        return {
            x: view.runtimeRect.x + (left - view.panX) / view.scale,
            y: view_top - ((top - view.panY) / view.scale) - engine_height,
            width: engine_width,
            height: engine_height
        };
    }

    function ApplyEngineRectToRectTransform(parent_rect, transform, engine_rect) {
        const parent = NormalizeRect(parent_rect, DEFAULT_VIEWPORT_RECT);
        const rect_transform = NormalizeRectTransform(transform);
        const rect = NormalizeRect(engine_rect, DEFAULT_VIEWPORT_RECT);
        const dpi_scale = rect_transform.dpiScale;
        const margin = rect_transform.margin;
        const left_anchor = parent.x + parent.width * rect_transform.anchorMin.x;
        const bottom_anchor = parent.y + parent.height * rect_transform.anchorMin.y;
        const right_anchor = parent.x + parent.width * rect_transform.anchorMax.x;
        const top_anchor = parent.y + parent.height * rect_transform.anchorMax.y;
        const right = rect.x + rect.width;
        const top = rect.y + rect.height;
        rect_transform.offsetMin = {
            x: (rect.x - left_anchor - margin.left * dpi_scale) / dpi_scale,
            y: (rect.y - bottom_anchor - margin.bottom * dpi_scale) / dpi_scale
        };
        rect_transform.offsetMax = {
            x: (right - right_anchor + margin.right * dpi_scale) / dpi_scale,
            y: (top - top_anchor + margin.top * dpi_scale) / dpi_scale
        };
        return rect_transform;
    }

    function CreateFixedRectTransformFromCanvas(canvas_rect, viewport) {
        const engine_rect = CanvasRectToEngineRect(canvas_rect, viewport);
        const parent_rect = NormalizeEditorViewport(viewport).runtimeRect;
        return ApplyEngineRectToRectTransform(parent_rect, CreateFullStretchRectTransform(), engine_rect);
    }

    function NormalizeEditorViewport(value) {
        const source = value || {};
        return {
            runtimeRect: NormalizeRect(source.runtimeRect, DEFAULT_VIEWPORT_RECT),
            scale: Math.max(0.001, ToNumber(source.scale, 1)),
            panX: ToNumber(source.panX, 0),
            panY: ToNumber(source.panY, 0)
        };
    }

    function CreateDefaultDocument() {
        const viewport = NormalizeEditorViewport({});
        return {
            schema: {
                schemaId: SCHEMA_ID,
                schemaVersion: SCHEMA_VERSION,
                layoutId: 1001,
                rootNodeId: 1
            },
            nodes: [
                {
                    nodeId: 1,
                    parentId: 0,
                    name: "Root",
                    component: "Container",
                    rectTransform: CreateFullStretchRectTransform(),
                    siblingOrder: 0,
                    layer: 0,
                    visible: true,
                    enabled: true,
                    hitTestable: false
                },
                {
                    nodeId: 2,
                    parentId: 1,
                    name: "Header",
                    component: "Text",
                    text: "Title",
                    rectTransform: CreateFixedRectTransformFromCanvas(
                        { left: 32, top: 28, width: 320, height: 48 },
                        viewport),
                    siblingOrder: 0,
                    layer: 1,
                    visible: true,
                    enabled: true,
                    hitTestable: false
                },
                {
                    nodeId: 3,
                    parentId: 1,
                    name: "PreviewButton",
                    component: "Button",
                    text: "Action",
                    rectTransform: CreateFixedRectTransformFromCanvas(
                        { left: 32, top: 108, width: 220, height: 48 },
                        viewport),
                    siblingOrder: 1,
                    layer: 1,
                    visible: true,
                    enabled: true,
                    hitTestable: true
                }
            ],
            layouts: [
                {
                    containerNodeId: 1,
                    type: "Stack",
                    direction: "Vertical",
                    gridColumnCount: 1,
                    spacing: 12,
                    padding: 24
                }
            ],
            styleRefs: [
                {
                    nodeId: 1,
                    styleKey: 1101,
                    themeKey: 1,
                    tokenKey: 1,
                    valueKind: "ColorRgba8",
                    value: "#f8fafc",
                    overridesTheme: true
                },
                {
                    nodeId: 2,
                    styleKey: 1102,
                    themeKey: 1,
                    tokenKey: 2,
                    valueKind: "ColorRgba8",
                    value: "#111827",
                    overridesTheme: true
                },
                {
                    nodeId: 3,
                    styleKey: 1103,
                    themeKey: 1,
                    tokenKey: 3,
                    valueKind: "ColorRgba8",
                    value: "#2563eb",
                    overridesTheme: true
                }
            ],
            resourceRefs: [
                {
                    nodeId: 2,
                    kind: "Font",
                    resourceKey: 2101,
                    label: "DefaultSans"
                }
            ],
            eventBindings: [
                {
                    nodeId: 3,
                    bindingKey: 3101,
                    eventKey: 4101,
                    command: "Submit"
                }
            ],
            theme: {
                tokens: [
                    { tokenKey: 1, name: "Surface", role: "Color", valueKind: "ColorRgba8", value: "#f8fafc" },
                    { tokenKey: 2, name: "TextPrimary", role: "Color", valueKind: "ColorRgba8", value: "#111827" },
                    { tokenKey: 3, name: "Accent", role: "Color", valueKind: "ColorRgba8", value: "#2563eb" },
                    { tokenKey: 4, name: "Gap", role: "Number", valueKind: "Number1000", value: 12000 }
                ],
                themes: [
                    { themeKey: 1, name: "Default", tokenKeys: [1, 2, 3, 4], default: true }
                ]
            },
            statePreview: [
                {
                    inputKey: 5101,
                    nodeId: 3,
                    name: "Pressed",
                    valueKind: "Bool",
                    value: false,
                    affectsVisibility: false,
                    affectsEnabled: true,
                    affectsResource: false
                }
            ],
            editor: {
                selectedNodeId: 1,
                dirty: false,
                viewport: viewport
            }
        };
    }

    function MigrateLegacyRect(rect) {
        const viewport = NormalizeEditorViewport({});
        const canvas_rect = {
            left: ToNumber(rect.x, 0),
            top: ToNumber(rect.y, 0),
            width: Math.max(1, ToNumber(rect.width, 120)),
            height: Math.max(1, ToNumber(rect.height, 48))
        };
        return CreateFixedRectTransformFromCanvas(canvas_rect, viewport);
    }

    function NormalizeNode(node, index) {
        const node_id = ToNumber(node.nodeId, index + 1);
        const rect_transform = node.rectTransform ? node.rectTransform : MigrateLegacyRect(node.rect || {});
        return {
            nodeId: node_id,
            parentId: ToNumber(node.parentId, 0),
            name: String(node.name || "Node" + node_id),
            component: String(node.component || "Container"),
            text: String(node.text || ""),
            rectTransform: NormalizeRectTransform(rect_transform),
            siblingOrder: ToNumber(node.siblingOrder, index),
            layer: ToNumber(node.layer, 0),
            visible: ToBool(node.visible, true),
            enabled: ToBool(node.enabled, true),
            hitTestable: ToBool(node.hitTestable, true)
        };
    }

    function NormalizeDocument(source) {
        const base = source && typeof source === "object" ? source : CreateDefaultDocument();
        const document = Clone(base);
        const schema = document.schema || {};
        document.schema = {
            schemaId: schema.schemaId || SCHEMA_ID,
            schemaVersion: ToNumber(schema.schemaVersion, SCHEMA_VERSION),
            layoutId: ToNumber(schema.layoutId, 0),
            rootNodeId: ToNumber(schema.rootNodeId, 0)
        };
        document.nodes = GetArray(document.nodes).map(NormalizeNode);
        document.layouts = GetArray(document.layouts);
        document.styleRefs = GetArray(document.styleRefs);
        document.resourceRefs = GetArray(document.resourceRefs);
        document.eventBindings = GetArray(document.eventBindings);
        document.statePreview = GetArray(document.statePreview);
        document.theme = document.theme || { tokens: [], themes: [] };
        document.theme.tokens = GetArray(document.theme.tokens);
        document.theme.themes = GetArray(document.theme.themes);
        document.editor = document.editor || {};
        document.editor.selectedNodeId = ToNumber(document.editor.selectedNodeId, document.schema.rootNodeId);
        document.editor.dirty = ToBool(document.editor.dirty, false);
        document.editor.viewport = NormalizeEditorViewport(document.editor.viewport);
        return document;
    }

    function FindNode(document, node_id) {
        return GetArray(document.nodes).find(function MatchNode(node) {
            return node.nodeId === node_id;
        }) || null;
    }

    function GetNextNodeId(document) {
        let next_id = 1;
        GetArray(document.nodes).forEach(function VisitNode(node) {
            if (node.nodeId >= next_id) {
                next_id = node.nodeId + 1;
            }
        });
        return next_id;
    }

    function PushIssue(issues, kind, message, node_id) {
        issues.push({
            kind: kind,
            message: message,
            nodeId: node_id || 0
        });
    }

    function IsUnitRange(value) {
        if (value < 0) {
            return false;
        }
        return value <= 1;
    }

    function IsNonNegative(value) {
        return value >= 0;
    }

    function ValidateRectTransform(transform) {
        const rect_transform = NormalizeRectTransform(transform);
        if (!IsUnitRange(rect_transform.anchorMin.x) || !IsUnitRange(rect_transform.anchorMin.y)) {
            return "InvalidAnchor";
        }
        if (!IsUnitRange(rect_transform.anchorMax.x) || !IsUnitRange(rect_transform.anchorMax.y)) {
            return "InvalidAnchor";
        }
        if (rect_transform.anchorMin.x > rect_transform.anchorMax.x || rect_transform.anchorMin.y > rect_transform.anchorMax.y) {
            return "InvalidAnchor";
        }
        if (!IsUnitRange(rect_transform.pivot.x) || !IsUnitRange(rect_transform.pivot.y)) {
            return "InvalidPivot";
        }
        if (rect_transform.dpiScale <= 0) {
            return "InvalidDpiScale";
        }
        const margin = rect_transform.margin;
        if (!IsNonNegative(margin.left) || !IsNonNegative(margin.top) ||
            !IsNonNegative(margin.right) || !IsNonNegative(margin.bottom)) {
            return "InvalidMargin";
        }
        const padding = rect_transform.padding;
        if (!IsNonNegative(padding.left) || !IsNonNegative(padding.top) ||
            !IsNonNegative(padding.right) || !IsNonNegative(padding.bottom)) {
            return "InvalidPadding";
        }
        return "Success";
    }

    function ResolveDocumentRects(source) {
        const document = NormalizeDocument(source);
        const viewport = document.editor.viewport;
        const resolved = new Map();
        const ordered_nodes = document.nodes.slice().sort(function CompareNodes(left, right) {
            if (left.parentId !== right.parentId) {
                return left.parentId - right.parentId;
            }
            return left.siblingOrder - right.siblingOrder;
        });
        ordered_nodes.forEach(function ResolveNode(node) {
            const parent_result = resolved.get(node.parentId);
            const parent_rect = parent_result ? parent_result.rect : viewport.runtimeRect;
            const result = ResolveRectTransform(parent_rect, node.rectTransform);
            resolved.set(node.nodeId, result);
        });
        return resolved;
    }

    function ValidateDocument(source) {
        const document = NormalizeDocument(source);
        const issues = [];
        if (document.schema.schemaId !== SCHEMA_ID) {
            PushIssue(issues, "InvalidHeader", "Schema id must be YUFS", 0);
        }
        if (document.schema.schemaVersion !== SCHEMA_VERSION) {
            PushIssue(issues, "InvalidHeader", "Schema version must be 1", 0);
        }
        if (document.nodes.length === 0) {
            PushIssue(issues, "MissingRootNode", "Document must contain nodes", 0);
        }

        const node_counts = new Map();
        document.nodes.forEach(function CountNode(node) {
            const count = node_counts.get(node.nodeId) || 0;
            node_counts.set(node.nodeId, count + 1);
        });

        const resolved = ResolveDocumentRects(document);
        document.nodes.forEach(function CheckNode(node) {
            const duplicate_count = node_counts.get(node.nodeId) || 0;
            const transform_status = ValidateRectTransform(node.rectTransform);
            const rect_result = resolved.get(node.nodeId);
            if (duplicate_count > 1) {
                PushIssue(issues, "DuplicateNodeId", "Node id is duplicated", node.nodeId);
            }
            if (node.nodeId <= 0) {
                PushIssue(issues, "InvalidNodeRecord", "Node id must be positive", node.nodeId);
            }
            if (node.parentId > 0 && !FindNode(document, node.parentId)) {
                PushIssue(issues, "MissingParentNode", "Parent node does not exist", node.nodeId);
            }
            if (transform_status !== "Success") {
                PushIssue(issues, transform_status, "RectTransform field is invalid", node.nodeId);
            }
            if (rect_result && rect_result.status !== "Success") {
                PushIssue(issues, rect_result.status, "RectTransform cannot resolve to a valid runtime rect", node.nodeId);
            }
        });

        const root = FindNode(document, document.schema.rootNodeId);
        if (!root) {
            PushIssue(issues, "MissingRootNode", "Root node does not exist", document.schema.rootNodeId);
        }

        document.layouts.forEach(function CheckLayout(layout) {
            if (!FindNode(document, ToNumber(layout.containerNodeId, 0))) {
                PushIssue(issues, "MissingLayoutContainerNode", "Layout container node does not exist", 0);
            }
        });

        document.styleRefs.forEach(function CheckStyle(style_ref) {
            if (!FindNode(document, ToNumber(style_ref.nodeId, 0))) {
                PushIssue(issues, "MissingStyleRefNode", "Style ref node does not exist", 0);
            }
        });

        document.resourceRefs.forEach(function CheckResource(resource_ref) {
            const kind = String(resource_ref.kind || "Invalid");
            if (!FindNode(document, ToNumber(resource_ref.nodeId, 0))) {
                PushIssue(issues, "MissingResourceRefNode", "Resource ref node does not exist", 0);
            }
            if (!RESOURCE_KINDS.includes(kind)) {
                PushIssue(issues, "InvalidResourceRef", "Resource kind is not supported", ToNumber(resource_ref.nodeId, 0));
            }
        });

        document.eventBindings.forEach(function CheckBinding(binding) {
            if (!FindNode(document, ToNumber(binding.nodeId, 0))) {
                PushIssue(issues, "MissingEventBindingNode", "Event binding node does not exist", 0);
            }
            if (ToNumber(binding.bindingKey, 0) === 0 || ToNumber(binding.eventKey, 0) === 0) {
                PushIssue(issues, "MissingEventBindingKey", "Event binding requires stable keys", ToNumber(binding.nodeId, 0));
            }
        });

        const status = issues.length === 0 ? "Success" : "IssuesFound";
        return {
            status: status,
            issues: issues,
            summary: {
                nodeCount: document.nodes.length,
                layoutCount: document.layouts.length,
                styleRefCount: document.styleRefs.length,
                resourceRefCount: document.resourceRefs.length,
                eventBindingCount: document.eventBindings.length,
                issueCount: issues.length
            }
        };
    }

    function GetNodeDepth(document, node) {
        let depth = 0;
        let parent_id = node.parentId;
        while (parent_id > 0) {
            const parent = FindNode(document, parent_id);
            if (!parent) {
                return depth;
            }
            depth += 1;
            parent_id = parent.parentId;
        }
        return depth;
    }

    function BuildHierarchy(document) {
        const normalized = NormalizeDocument(document);
        return normalized.nodes.slice().sort(function CompareNodes(left, right) {
            if (left.layer !== right.layer) {
                return left.layer - right.layer;
            }
            return left.siblingOrder - right.siblingOrder;
        }).map(function MapNode(node) {
            return {
                nodeId: node.nodeId,
                parentId: node.parentId,
                name: node.name,
                component: node.component,
                depth: GetNodeDepth(normalized, node),
                selected: normalized.editor.selectedNodeId === node.nodeId,
                visible: node.visible,
                enabled: node.enabled
            };
        });
    }

    function BuildCanvasItems(document) {
        const normalized = NormalizeDocument(document);
        const resolved = ResolveDocumentRects(normalized);
        return normalized.nodes.filter(function KeepVisible(node) {
            return node.visible;
        }).sort(function CompareCanvas(left, right) {
            if (left.layer !== right.layer) {
                return left.layer - right.layer;
            }
            return left.siblingOrder - right.siblingOrder;
        }).map(function MapCanvas(node) {
            const rect_result = resolved.get(node.nodeId);
            const runtime_rect = rect_result ? rect_result.rect : CreateRect(0, 0, 1, 1);
            return {
                nodeId: node.nodeId,
                name: node.name,
                component: node.component,
                runtimeRect: Clone(runtime_rect),
                canvasRect: EngineRectToCanvasRect(runtime_rect, normalized.editor.viewport),
                selected: normalized.editor.selectedNodeId === node.nodeId,
                enabled: node.enabled,
                hitTestable: node.hitTestable
            };
        });
    }

    function BuildInspector(document) {
        const normalized = NormalizeDocument(document);
        const selected_node = FindNode(normalized, normalized.editor.selectedNodeId);
        if (!selected_node) {
            return null;
        }
        return {
            node: Clone(selected_node),
            styleRefs: normalized.styleRefs.filter(function MatchStyle(style_ref) {
                return ToNumber(style_ref.nodeId, 0) === selected_node.nodeId;
            }),
            resourceRefs: normalized.resourceRefs.filter(function MatchResource(resource_ref) {
                return ToNumber(resource_ref.nodeId, 0) === selected_node.nodeId;
            }),
            eventBindings: normalized.eventBindings.filter(function MatchBinding(binding) {
                return ToNumber(binding.nodeId, 0) === selected_node.nodeId;
            })
        };
    }

    function MergeRectTransform(current, patch) {
        const base = NormalizeRectTransform(current);
        const update = patch || {};
        return NormalizeRectTransform({
            anchorMin: Object.assign({}, base.anchorMin, update.anchorMin || {}),
            anchorMax: Object.assign({}, base.anchorMax, update.anchorMax || {}),
            pivot: Object.assign({}, base.pivot, update.pivot || {}),
            offsetMin: Object.assign({}, base.offsetMin, update.offsetMin || {}),
            offsetMax: Object.assign({}, base.offsetMax, update.offsetMax || {}),
            margin: Object.assign({}, base.margin, update.margin || {}),
            padding: Object.assign({}, base.padding, update.padding || {}),
            dpiScale: ToNumber(update.dpiScale, base.dpiScale)
        });
    }

    function UpdateNode(source, node_id, patch) {
        const document = NormalizeDocument(source);
        document.nodes = document.nodes.map(function MapNode(node) {
            if (node.nodeId !== node_id) {
                return node;
            }
            const next_node = Object.assign({}, node, patch);
            if (patch.rectTransform) {
                next_node.rectTransform = MergeRectTransform(node.rectTransform, patch.rectTransform);
            }
            return next_node;
        });
        document.editor.selectedNodeId = node_id;
        document.editor.dirty = true;
        return document;
    }

    function GetParentRect(document, node_id) {
        const node = FindNode(document, node_id);
        if (!node) {
            return document.editor.viewport.runtimeRect;
        }
        if (node.parentId === 0) {
            return document.editor.viewport.runtimeRect;
        }
        const resolved = ResolveDocumentRects(document);
        const parent_result = resolved.get(node.parentId);
        if (!parent_result) {
            return document.editor.viewport.runtimeRect;
        }
        return parent_result.rect;
    }

    function UpdateNodeFromCanvasRect(source, node_id, canvas_rect) {
        const document = NormalizeDocument(source);
        const node = FindNode(document, node_id);
        if (!node) {
            return document;
        }
        const engine_rect = CanvasRectToEngineRect(canvas_rect, document.editor.viewport);
        const parent_rect = GetParentRect(document, node_id);
        const rect_transform = ApplyEngineRectToRectTransform(parent_rect, node.rectTransform, engine_rect);
        return UpdateNode(document, node_id, { rectTransform: rect_transform });
    }

    function AddNode(source, component) {
        const document = NormalizeDocument(source);
        const selected = FindNode(document, document.editor.selectedNodeId);
        const parent_id = selected ? selected.nodeId : document.schema.rootNodeId;
        const parent_rect = selected ? ResolveDocumentRects(document).get(selected.nodeId).rect : document.editor.viewport.runtimeRect;
        const node_id = GetNextNodeId(document);
        const sibling_order = document.nodes.filter(function MatchParent(node) {
            return node.parentId === parent_id;
        }).length;
        const engine_rect = {
            x: parent_rect.x + 48 + sibling_order * 24,
            y: parent_rect.y + parent_rect.height - 108 - sibling_order * 52,
            width: 180,
            height: 44
        };
        const rect_transform = ApplyEngineRectToRectTransform(parent_rect, CreateFullStretchRectTransform(), engine_rect);
        const new_node = {
            nodeId: node_id,
            parentId: parent_id,
            name: String(component || "Container") + node_id,
            component: String(component || "Container"),
            text: String(component || "Container"),
            rectTransform: rect_transform,
            siblingOrder: sibling_order,
            layer: selected ? selected.layer + 1 : 1,
            visible: true,
            enabled: true,
            hitTestable: true
        };
        document.nodes.push(new_node);
        document.editor.selectedNodeId = node_id;
        document.editor.dirty = true;
        return document;
    }

    function RemoveNode(source, node_id) {
        const document = NormalizeDocument(source);
        if (node_id === document.schema.rootNodeId) {
            return document;
        }
        const removed_ids = new Set([node_id]);
        let changed = true;
        while (changed) {
            changed = false;
            document.nodes.forEach(function VisitNode(node) {
                if (removed_ids.has(node.parentId) && !removed_ids.has(node.nodeId)) {
                    removed_ids.add(node.nodeId);
                    changed = true;
                }
            });
        }
        document.nodes = document.nodes.filter(function KeepNode(node) {
            return !removed_ids.has(node.nodeId);
        });
        document.layouts = document.layouts.filter(function KeepLayout(layout) {
            return !removed_ids.has(ToNumber(layout.containerNodeId, 0));
        });
        document.styleRefs = document.styleRefs.filter(function KeepStyle(style_ref) {
            return !removed_ids.has(ToNumber(style_ref.nodeId, 0));
        });
        document.resourceRefs = document.resourceRefs.filter(function KeepResource(resource_ref) {
            return !removed_ids.has(ToNumber(resource_ref.nodeId, 0));
        });
        document.eventBindings = document.eventBindings.filter(function KeepBinding(binding) {
            return !removed_ids.has(ToNumber(binding.nodeId, 0));
        });
        document.editor.selectedNodeId = document.schema.rootNodeId;
        document.editor.dirty = true;
        return document;
    }

    function BuildRuntimeDocument(source) {
        const document = NormalizeDocument(source);
        const runtime_nodes = document.nodes.map(function MapRuntimeNode(node) {
            return {
                nodeId: node.nodeId,
                parentId: node.parentId,
                name: node.name,
                component: node.component,
                text: node.text,
                rectTransform: Clone(node.rectTransform),
                siblingOrder: node.siblingOrder,
                layer: node.layer,
                visible: node.visible,
                enabled: node.enabled,
                hitTestable: node.hitTestable
            };
        });
        const runtime_document = {
            schema: document.schema,
            nodes: runtime_nodes,
            layouts: document.layouts,
            styleRefs: document.styleRefs,
            resourceRefs: document.resourceRefs,
            eventBindings: document.eventBindings,
            theme: document.theme
        };
        return Clone(runtime_document);
    }

    function FormatJson(value) {
        return JSON.stringify(value, null, 4);
    }

    return {
        SCHEMA_ID: SCHEMA_ID,
        SCHEMA_VERSION: SCHEMA_VERSION,
        DEFAULT_VIEWPORT_RECT: DEFAULT_VIEWPORT_RECT,
        CreateDefaultDocument: CreateDefaultDocument,
        NormalizeDocument: NormalizeDocument,
        NormalizeRectTransform: NormalizeRectTransform,
        ResolveRectTransform: ResolveRectTransform,
        ResolveDocumentRects: ResolveDocumentRects,
        EngineRectToCanvasRect: EngineRectToCanvasRect,
        CanvasRectToEngineRect: CanvasRectToEngineRect,
        ApplyEngineRectToRectTransform: ApplyEngineRectToRectTransform,
        ValidateDocument: ValidateDocument,
        BuildHierarchy: BuildHierarchy,
        BuildCanvasItems: BuildCanvasItems,
        BuildInspector: BuildInspector,
        UpdateNode: UpdateNode,
        UpdateNodeFromCanvasRect: UpdateNodeFromCanvasRect,
        AddNode: AddNode,
        RemoveNode: RemoveNode,
        BuildRuntimeDocument: BuildRuntimeDocument,
        FormatJson: FormatJson
    };
});
