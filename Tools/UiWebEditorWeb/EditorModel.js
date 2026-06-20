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

    function CreateRect(x, y, width, height) {
        return {
            x: x,
            y: y,
            width: width,
            height: height,
            anchorMin: { x: 0, y: 0 },
            anchorMax: { x: 0, y: 0 },
            pivot: { x: 0.5, y: 0.5 },
            dpiScale: 1
        };
    }

    function CreateDefaultDocument() {
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
                    rect: CreateRect(0, 0, 960, 540),
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
                    rect: CreateRect(32, 28, 320, 48),
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
                    rect: CreateRect(32, 108, 220, 48),
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
                dirty: false
            }
        };
    }

    function NormalizeNode(node, index) {
        const node_id = ToNumber(node.nodeId, index + 1);
        const parent_id = ToNumber(node.parentId, 0);
        const rect = node.rect || {};
        return {
            nodeId: node_id,
            parentId: parent_id,
            name: String(node.name || "Node" + node_id),
            component: String(node.component || "Container"),
            text: String(node.text || ""),
            rect: {
                x: ToNumber(rect.x, 0),
                y: ToNumber(rect.y, 0),
                width: Math.max(1, ToNumber(rect.width, 120)),
                height: Math.max(1, ToNumber(rect.height, 48)),
                anchorMin: rect.anchorMin || { x: 0, y: 0 },
                anchorMax: rect.anchorMax || { x: 0, y: 0 },
                pivot: rect.pivot || { x: 0.5, y: 0.5 },
                dpiScale: ToNumber(rect.dpiScale, 1)
            },
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

        document.nodes.forEach(function CheckNode(node, index) {
            const duplicate_count = node_counts.get(node.nodeId) || 0;
            if (duplicate_count > 1) {
                PushIssue(issues, "DuplicateNodeId", "Node id is duplicated", node.nodeId);
            }
            if (node.nodeId <= 0) {
                PushIssue(issues, "InvalidNodeRecord", "Node id must be positive", node.nodeId);
            }
            if (node.parentId > 0 && !FindNode(document, node.parentId)) {
                PushIssue(issues, "MissingParentNode", "Parent node does not exist", node.nodeId);
            }
            if (node.rect.width <= 0 || node.rect.height <= 0) {
                PushIssue(issues, "InvalidNodeRecord", "Node rect must have positive size", node.nodeId);
            }
            if (node.siblingOrder !== index && node.parentId === 0) {
                return;
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

    function BuildHierarchy(document) {
        const normalized = NormalizeDocument(document);
        function GetDepth(node) {
            let depth = 0;
            let parent_id = node.parentId;
            while (parent_id > 0) {
                const parent = FindNode(normalized, parent_id);
                if (!parent) {
                    return depth;
                }
                depth += 1;
                parent_id = parent.parentId;
            }
            return depth;
        }
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
                depth: GetDepth(node),
                selected: normalized.editor.selectedNodeId === node.nodeId,
                visible: node.visible,
                enabled: node.enabled
            };
        });
    }

    function BuildCanvasItems(document) {
        const normalized = NormalizeDocument(document);
        return normalized.nodes.filter(function KeepVisible(node) {
            return node.visible;
        }).sort(function CompareCanvas(left, right) {
            if (left.layer !== right.layer) {
                return left.layer - right.layer;
            }
            return left.siblingOrder - right.siblingOrder;
        }).map(function MapCanvas(node) {
            return {
                nodeId: node.nodeId,
                name: node.name,
                component: node.component,
                rect: Clone(node.rect),
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

    function UpdateNode(source, node_id, patch) {
        const document = NormalizeDocument(source);
        document.nodes = document.nodes.map(function MapNode(node) {
            if (node.nodeId !== node_id) {
                return node;
            }
            const next_node = Object.assign({}, node, patch);
            if (patch.rect) {
                next_node.rect = Object.assign({}, node.rect, patch.rect);
            }
            return next_node;
        });
        document.editor.selectedNodeId = node_id;
        document.editor.dirty = true;
        return document;
    }

    function AddNode(source, component) {
        const document = NormalizeDocument(source);
        const selected = FindNode(document, document.editor.selectedNodeId);
        const parent_id = selected ? selected.nodeId : document.schema.rootNodeId;
        const node_id = GetNextNodeId(document);
        const sibling_order = document.nodes.filter(function MatchParent(node) {
            return node.parentId === parent_id;
        }).length;
        const new_node = {
            nodeId: node_id,
            parentId: parent_id,
            name: String(component || "Container") + node_id,
            component: String(component || "Container"),
            text: String(component || "Container"),
            rect: CreateRect(48 + sibling_order * 24, 64 + sibling_order * 52, 180, 44),
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
        const runtime_document = {
            schema: document.schema,
            nodes: document.nodes,
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
        CreateDefaultDocument: CreateDefaultDocument,
        NormalizeDocument: NormalizeDocument,
        ValidateDocument: ValidateDocument,
        BuildHierarchy: BuildHierarchy,
        BuildCanvasItems: BuildCanvasItems,
        BuildInspector: BuildInspector,
        UpdateNode: UpdateNode,
        AddNode: AddNode,
        RemoveNode: RemoveNode,
        BuildRuntimeDocument: BuildRuntimeDocument,
        FormatJson: FormatJson
    };
});
