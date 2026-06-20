// 模块: Tools SceneWebEditorWeb
// 文件: Tools/SceneWebEditorWeb/SceneEditorModel.js

(function RegisterSceneWebEditorModel(root, factory) {
    const api = factory();
    if (typeof module !== "undefined" && module.exports) {
        module.exports = api;
    }
    if (root) {
        root.SceneWebEditorModel = api;
    }
})(typeof globalThis !== "undefined" ? globalThis : this, function CreateSceneWebEditorModel() {
    const SCHEMA_ID = "YUSC";
    const SCHEMA_VERSION = 1;
    const RESOURCE_SLOTS = ["Mesh", "Material", "Audio", "Custom"];
    const DEFAULT_VIEWPORT = {
        panX: 0,
        panY: 0,
        zoom: 40,
        gridSize: 1
    };

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

    function ToString(value, fallback) {
        if (typeof value === "string") {
            return value;
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

    function CreateVector3(x, y, z) {
        return {
            x: x,
            y: y,
            z: z
        };
    }

    function NormalizeId(value, fallback) {
        const id = Math.trunc(ToNumber(value, fallback));
        if (id > 0) {
            return id;
        }
        return fallback;
    }

    function NormalizeOrder(value, fallback) {
        const order = Math.trunc(ToNumber(value, fallback));
        if (order >= 0) {
            return order;
        }
        return fallback;
    }

    function NormalizeVector3(value, fallback) {
        const source = value || fallback || CreateVector3(0, 0, 0);
        return {
            x: ToNumber(source.x, fallback ? fallback.x : 0),
            y: ToNumber(source.y, fallback ? fallback.y : 0),
            z: ToNumber(source.z, fallback ? fallback.z : 0)
        };
    }

    function NormalizeScale(value) {
        const source = NormalizeVector3(value, CreateVector3(1, 1, 1));
        return {
            x: source.x,
            y: source.y,
            z: source.z
        };
    }

    function NormalizeObject(record, index) {
        const object_id = NormalizeId(record.objectId, index + 1);
        return {
            objectId: object_id,
            objectKey: ToString(record.objectKey, "SceneObject" + object_id),
            displayName: ToString(record.displayName, "Scene Object " + object_id),
            order: NormalizeOrder(record.order, index),
            enabled: ToBool(record.enabled, true)
        };
    }

    function NormalizeTransform(record) {
        return {
            objectId: NormalizeId(record.objectId, 0),
            position: NormalizeVector3(record.position, CreateVector3(0, 0, 0)),
            rotation: NormalizeVector3(record.rotation, CreateVector3(0, 0, 0)),
            scale: NormalizeScale(record.scale)
        };
    }

    function NormalizeResourceRef(record) {
        const slot = ToString(record.slot, "Custom");
        return {
            objectId: NormalizeId(record.objectId, 0),
            slot: slot,
            resourceKey: ToString(record.resourceKey, "")
        };
    }

    function NormalizeEditorSidecar(value, root_object_id) {
        const source = value || {};
        const viewport = source.viewport || {};
        const grid = source.grid || {};
        const snap = source.snap || {};
        return {
            selectedObjectId: NormalizeId(source.selectedObjectId, root_object_id),
            dirty: ToBool(source.dirty, false),
            viewport: {
                panX: ToNumber(viewport.panX, DEFAULT_VIEWPORT.panX),
                panY: ToNumber(viewport.panY, DEFAULT_VIEWPORT.panY),
                zoom: Math.max(4, ToNumber(viewport.zoom, DEFAULT_VIEWPORT.zoom)),
                gridSize: Math.max(0.25, ToNumber(viewport.gridSize, DEFAULT_VIEWPORT.gridSize))
            },
            grid: {
                enabled: ToBool(grid.enabled, true),
                majorEvery: Math.max(1, Math.trunc(ToNumber(grid.majorEvery, 5)))
            },
            snap: {
                enabled: ToBool(snap.enabled, false),
                translateStep: Math.max(0.01, ToNumber(snap.translateStep, 0.25)),
                rotateStep: Math.max(0.01, ToNumber(snap.rotateStep, 15)),
                scaleStep: Math.max(0.01, ToNumber(snap.scaleStep, 0.1))
            },
            gizmo: ToString(source.gizmo, "Translate"),
            foldouts: source.foldouts || {},
            paneState: source.paneState || {}
        };
    }

    function CompareObjectOrder(left, right) {
        if (left.order !== right.order) {
            return left.order - right.order;
        }
        return left.objectId - right.objectId;
    }

    function CompareTransformOrder(left, right) {
        return left.objectId - right.objectId;
    }

    function CompareResourceRefOrder(left, right) {
        if (left.objectId !== right.objectId) {
            return left.objectId - right.objectId;
        }
        if (left.slot !== right.slot) {
            return left.slot < right.slot ? -1 : 1;
        }
        return left.resourceKey < right.resourceKey ? -1 : 1;
    }

    function NormalizeObjectOrders(document) {
        document.objects.slice().sort(CompareObjectOrder).forEach(function AssignOrder(record, index) {
            record.order = index;
        });
    }

    function CreateDefaultDocument() {
        return {
            schema: {
                schemaId: SCHEMA_ID,
                schemaVersion: SCHEMA_VERSION,
                documentId: "SceneDocument001",
                sceneKey: "SampleScene",
                documentHash: ""
            },
            objects: [
                {
                    objectId: 1,
                    objectKey: "SceneRoot",
                    displayName: "Scene Root",
                    order: 0,
                    enabled: true
                },
                {
                    objectId: 2,
                    objectKey: "MarkerA",
                    displayName: "Marker A",
                    order: 1,
                    enabled: true
                },
                {
                    objectId: 3,
                    objectKey: "MarkerB",
                    displayName: "Marker B",
                    order: 2,
                    enabled: true
                }
            ],
            transforms: [
                {
                    objectId: 1,
                    position: CreateVector3(0, 0, 0),
                    rotation: CreateVector3(0, 0, 0),
                    scale: CreateVector3(1, 1, 1)
                },
                {
                    objectId: 2,
                    position: CreateVector3(-2, 0, 1),
                    rotation: CreateVector3(0, 20, 0),
                    scale: CreateVector3(1, 1, 1)
                },
                {
                    objectId: 3,
                    position: CreateVector3(2, 0, -1),
                    rotation: CreateVector3(0, -20, 0),
                    scale: CreateVector3(1, 1, 1)
                }
            ],
            resourceRefs: [
                {
                    objectId: 2,
                    slot: "Mesh",
                    resourceKey: "pkg://scene/marker-a"
                },
                {
                    objectId: 3,
                    slot: "Material",
                    resourceKey: "pkg://scene/marker-material"
                }
            ],
            editor: {
                selectedObjectId: 1,
                dirty: false,
                viewport: Clone(DEFAULT_VIEWPORT),
                grid: {
                    enabled: true,
                    majorEvery: 5
                },
                snap: {
                    enabled: false,
                    translateStep: 0.25,
                    rotateStep: 15,
                    scaleStep: 0.1
                },
                gizmo: "Translate",
                foldouts: {
                    scene: true,
                    object: true,
                    transform: true,
                    resources: true,
                    editor: false
                },
                paneState: {}
            }
        };
    }

    function NormalizeDocument(source) {
        const base = source && typeof source === "object" ? source : CreateDefaultDocument();
        const document = Clone(base);
        const schema = document.schema || {};
        document.schema = {
            schemaId: ToString(schema.schemaId, SCHEMA_ID),
            schemaVersion: Math.trunc(ToNumber(schema.schemaVersion, SCHEMA_VERSION)),
            documentId: ToString(schema.documentId, "SceneDocument001"),
            sceneKey: ToString(schema.sceneKey, "Scene"),
            documentHash: ToString(schema.documentHash, "")
        };
        document.objects = GetArray(document.objects).map(NormalizeObject);
        document.transforms = GetArray(document.transforms).map(NormalizeTransform);
        document.resourceRefs = GetArray(document.resourceRefs).map(NormalizeResourceRef);
        NormalizeObjectOrders(document);
        const first_object = document.objects[0] || { objectId: 0 };
        document.editor = NormalizeEditorSidecar(document.editor, first_object.objectId);
        return document;
    }

    function FindObject(document, object_id) {
        return document.objects.find(function MatchObject(record) {
            return record.objectId === object_id;
        }) || null;
    }

    function FindTransform(document, object_id) {
        return document.transforms.find(function MatchTransform(record) {
            return record.objectId === object_id;
        }) || null;
    }

    function GetNextObjectId(document) {
        let next_id = 1;
        document.objects.forEach(function VisitObject(record) {
            if (record.objectId >= next_id) {
                next_id = record.objectId + 1;
            }
        });
        return next_id;
    }

    function PushIssue(issues, kind, message, object_id) {
        issues.push({
            kind: kind,
            message: message,
            objectId: object_id || 0
        });
    }

    function IsFiniteVector3(value) {
        return Number.isFinite(value.x) && Number.isFinite(value.y) && Number.isFinite(value.z);
    }

    function ValidateDocument(source) {
        const document = NormalizeDocument(source);
        const issues = [];
        if (document.schema.schemaId !== SCHEMA_ID) {
            PushIssue(issues, "InvalidHeader", "Schema id must be YUSC", 0);
        }
        if (document.schema.schemaVersion !== SCHEMA_VERSION) {
            PushIssue(issues, "InvalidHeader", "Schema version must be 1", 0);
        }
        if (!document.schema.documentId) {
            PushIssue(issues, "InvalidHeader", "Document id is required", 0);
        }
        if (!document.schema.sceneKey) {
            PushIssue(issues, "InvalidHeader", "Scene key is required", 0);
        }
        if (document.objects.length === 0) {
            PushIssue(issues, "MissingObjectRecord", "Scene requires at least one object record", 0);
        }

        const object_ids = new Map();
        const object_keys = new Map();
        document.objects.forEach(function CountObject(record) {
            object_ids.set(record.objectId, (object_ids.get(record.objectId) || 0) + 1);
            object_keys.set(record.objectKey, (object_keys.get(record.objectKey) || 0) + 1);
            if (record.objectId <= 0) {
                PushIssue(issues, "InvalidObjectId", "Object id must be positive", record.objectId);
            }
            if (!record.objectKey) {
                PushIssue(issues, "InvalidObjectKey", "Object key is required", record.objectId);
            }
        });
        object_ids.forEach(function CheckObjectId(count, object_id) {
            if (count > 1) {
                PushIssue(issues, "DuplicateObjectId", "Object id is duplicated", object_id);
            }
        });
        object_keys.forEach(function CheckObjectKey(count, object_key) {
            if (count > 1) {
                PushIssue(issues, "DuplicateObjectKey", "Object key is duplicated", 0);
            }
        });

        const transform_ids = new Map();
        document.transforms.forEach(function CheckTransform(record) {
            transform_ids.set(record.objectId, (transform_ids.get(record.objectId) || 0) + 1);
            if (!FindObject(document, record.objectId)) {
                PushIssue(issues, "TransformWithoutObject", "Transform references a missing object", record.objectId);
            }
            if (!IsFiniteVector3(record.position) || !IsFiniteVector3(record.rotation) || !IsFiniteVector3(record.scale)) {
                PushIssue(issues, "InvalidTransform", "Transform values must be finite", record.objectId);
            }
            if (record.scale.x <= 0 || record.scale.y <= 0 || record.scale.z <= 0) {
                PushIssue(issues, "InvalidTransform", "Transform scale must be positive", record.objectId);
            }
        });
        transform_ids.forEach(function CheckTransformId(count, object_id) {
            if (count > 1) {
                PushIssue(issues, "DuplicateTransform", "Transform record is duplicated", object_id);
            }
        });
        document.objects.forEach(function CheckMissingTransform(record) {
            if (!FindTransform(document, record.objectId)) {
                PushIssue(issues, "MissingTransform", "Object requires a transform record", record.objectId);
            }
        });

        const resource_tuples = new Map();
        document.resourceRefs.forEach(function CheckResourceRef(record) {
            const tuple_key = String(record.objectId) + "|" + record.slot + "|" + record.resourceKey;
            resource_tuples.set(tuple_key, (resource_tuples.get(tuple_key) || 0) + 1);
            if (!FindObject(document, record.objectId)) {
                PushIssue(issues, "ResourceWithoutObject", "Resource ref references a missing object", record.objectId);
            }
            if (!RESOURCE_SLOTS.includes(record.slot)) {
                PushIssue(issues, "InvalidResourceSlot", "Resource slot is not supported", record.objectId);
            }
            if (!record.resourceKey) {
                PushIssue(issues, "InvalidResourceKey", "Resource key is required", record.objectId);
            }
        });
        resource_tuples.forEach(function CheckResourceTuple(count, tuple_key) {
            if (count > 1) {
                PushIssue(issues, "DuplicateResourceTuple", "Resource ref tuple is duplicated", 0);
            }
        });

        const status = issues.length === 0 ? "Success" : "IssuesFound";
        return {
            status: status,
            issues: issues,
            summary: {
                objectCount: document.objects.length,
                transformCount: document.transforms.length,
                resourceRefCount: document.resourceRefs.length,
                issueCount: issues.length
            }
        };
    }

    function ComputeDocumentHash(value) {
        const text = JSON.stringify(value);
        let hash = 2166136261;
        for (let index = 0; index < text.length; ++index) {
            hash ^= text.charCodeAt(index);
            hash = Math.imul(hash, 16777619);
        }
        return "0x" + (hash >>> 0).toString(16).padStart(8, "0").toUpperCase();
    }

    function BuildRuntimeDocument(source) {
        const document = NormalizeDocument(source);
        const runtime_document = {
            schema: {
                schemaId: document.schema.schemaId,
                schemaVersion: document.schema.schemaVersion,
                documentId: document.schema.documentId,
                sceneKey: document.schema.sceneKey,
                documentHash: ""
            },
            objects: document.objects.slice().sort(CompareObjectOrder).map(function MapObject(record) {
                return Clone(record);
            }),
            transforms: document.transforms.slice().sort(CompareTransformOrder).map(function MapTransform(record) {
                return Clone(record);
            }),
            resourceRefs: document.resourceRefs.slice().sort(CompareResourceRefOrder).map(function MapResource(record) {
                return Clone(record);
            })
        };
        runtime_document.schema.documentHash = ComputeDocumentHash(runtime_document);
        return Clone(runtime_document);
    }

    function BuildObjectList(source) {
        const document = NormalizeDocument(source);
        return document.objects.slice().sort(CompareObjectOrder).map(function MapObject(record) {
            return {
                objectId: record.objectId,
                objectKey: record.objectKey,
                displayName: record.displayName,
                order: record.order,
                enabled: record.enabled,
                selected: record.objectId === document.editor.selectedObjectId
            };
        });
    }

    function BuildViewportItems(source) {
        const document = NormalizeDocument(source);
        const viewport = document.editor.viewport;
        return document.objects.slice().sort(CompareObjectOrder).map(function MapViewportItem(record) {
            const transform = FindTransform(document, record.objectId) || {
                position: CreateVector3(0, 0, 0),
                rotation: CreateVector3(0, 0, 0),
                scale: CreateVector3(1, 1, 1)
            };
            return {
                objectId: record.objectId,
                displayName: record.displayName,
                enabled: record.enabled,
                selected: record.objectId === document.editor.selectedObjectId,
                left: 360 + viewport.panX + transform.position.x * viewport.zoom,
                top: 260 + viewport.panY - transform.position.z * viewport.zoom,
                radius: Math.max(8, transform.scale.x * 12),
                position: Clone(transform.position)
            };
        });
    }

    function BuildInspector(source) {
        const document = NormalizeDocument(source);
        const selected = FindObject(document, document.editor.selectedObjectId) || document.objects[0] || null;
        if (!selected) {
            return null;
        }
        const selected_transform = FindTransform(document, selected.objectId) || NormalizeTransform({
            objectId: selected.objectId
        });
        return {
            schema: Clone(document.schema),
            object: Clone(selected),
            transform: Clone(selected_transform),
            resourceRefs: document.resourceRefs.filter(function MatchResource(record) {
                return record.objectId === selected.objectId;
            }).map(function CloneResource(record) {
                return Clone(record);
            }),
            editor: Clone(document.editor)
        };
    }

    function UpdateSchema(source, patch) {
        const document = NormalizeDocument(source);
        document.schema = Object.assign({}, document.schema, patch || {});
        document.editor.dirty = true;
        return document;
    }

    function UpdateObject(source, object_id, patch) {
        const document = NormalizeDocument(source);
        document.objects = document.objects.map(function MapObject(record) {
            if (record.objectId !== object_id) {
                return record;
            }
            return NormalizeObject(Object.assign({}, record, patch || {}), record.order);
        });
        NormalizeObjectOrders(document);
        document.editor.selectedObjectId = object_id;
        document.editor.dirty = true;
        return document;
    }

    function UpdateTransform(source, object_id, patch) {
        const document = NormalizeDocument(source);
        document.transforms = document.transforms.map(function MapTransform(record) {
            if (record.objectId !== object_id) {
                return record;
            }
            return NormalizeTransform(Object.assign({}, record, patch || {}));
        });
        document.editor.selectedObjectId = object_id;
        document.editor.dirty = true;
        return document;
    }

    function SetResourceRef(source, object_id, slot, resource_key) {
        const document = NormalizeDocument(source);
        const normalized_slot = RESOURCE_SLOTS.includes(slot) ? slot : "Custom";
        const existing = document.resourceRefs.find(function MatchResource(record) {
            return record.objectId === object_id && record.slot === normalized_slot;
        });
        if (existing) {
            existing.resourceKey = ToString(resource_key, "");
            document.editor.selectedObjectId = object_id;
            document.editor.dirty = true;
            return document;
        }
        document.resourceRefs.push({
            objectId: object_id,
            slot: normalized_slot,
            resourceKey: ToString(resource_key, "")
        });
        document.editor.selectedObjectId = object_id;
        document.editor.dirty = true;
        return document;
    }

    function SelectObject(source, object_id) {
        const document = NormalizeDocument(source);
        if (!FindObject(document, object_id)) {
            return document;
        }
        document.editor.selectedObjectId = object_id;
        return document;
    }

    function AddObject(source) {
        const document = NormalizeDocument(source);
        const object_id = GetNextObjectId(document);
        const order = document.objects.length;
        document.objects.push({
            objectId: object_id,
            objectKey: "SceneObject" + object_id,
            displayName: "Scene Object " + object_id,
            order: order,
            enabled: true
        });
        document.transforms.push({
            objectId: object_id,
            position: CreateVector3(order * 1.5, 0, 0),
            rotation: CreateVector3(0, 0, 0),
            scale: CreateVector3(1, 1, 1)
        });
        document.editor.selectedObjectId = object_id;
        document.editor.dirty = true;
        return document;
    }

    function DuplicateObject(source, object_id) {
        const document = NormalizeDocument(source);
        const current = FindObject(document, object_id);
        const transform = FindTransform(document, object_id);
        if (!current || !transform) {
            return document;
        }
        const next_id = GetNextObjectId(document);
        const duplicate = Clone(current);
        duplicate.objectId = next_id;
        duplicate.objectKey = current.objectKey + "Copy" + next_id;
        duplicate.displayName = current.displayName + " Copy";
        duplicate.order = document.objects.length;
        document.objects.push(duplicate);
        const next_transform = Clone(transform);
        next_transform.objectId = next_id;
        next_transform.position.x += 1;
        next_transform.position.z += 1;
        document.transforms.push(next_transform);
        document.resourceRefs.filter(function MatchSourceResource(record) {
            return record.objectId === object_id;
        }).forEach(function CopyResource(record) {
            const next_resource = Clone(record);
            next_resource.objectId = next_id;
            document.resourceRefs.push(next_resource);
        });
        document.editor.selectedObjectId = next_id;
        document.editor.dirty = true;
        return document;
    }

    function RemoveObject(source, object_id) {
        const document = NormalizeDocument(source);
        if (document.objects.length <= 1) {
            return document;
        }
        document.objects = document.objects.filter(function KeepObject(record) {
            return record.objectId !== object_id;
        });
        document.transforms = document.transforms.filter(function KeepTransform(record) {
            return record.objectId !== object_id;
        });
        document.resourceRefs = document.resourceRefs.filter(function KeepResource(record) {
            return record.objectId !== object_id;
        });
        NormalizeObjectOrders(document);
        const first_object = document.objects[0] || { objectId: 0 };
        document.editor.selectedObjectId = first_object.objectId;
        document.editor.dirty = true;
        return document;
    }

    function UpdateEditor(source, patch) {
        const document = NormalizeDocument(source);
        document.editor = NormalizeEditorSidecar(Object.assign({}, document.editor, patch || {}), document.editor.selectedObjectId);
        document.editor.dirty = true;
        return document;
    }

    function FormatJson(value) {
        return JSON.stringify(value, null, 4);
    }

    return {
        SCHEMA_ID: SCHEMA_ID,
        SCHEMA_VERSION: SCHEMA_VERSION,
        RESOURCE_SLOTS: RESOURCE_SLOTS,
        CreateDefaultDocument: CreateDefaultDocument,
        NormalizeDocument: NormalizeDocument,
        ValidateDocument: ValidateDocument,
        BuildRuntimeDocument: BuildRuntimeDocument,
        BuildObjectList: BuildObjectList,
        BuildViewportItems: BuildViewportItems,
        BuildInspector: BuildInspector,
        UpdateSchema: UpdateSchema,
        UpdateObject: UpdateObject,
        UpdateTransform: UpdateTransform,
        SetResourceRef: SetResourceRef,
        SelectObject: SelectObject,
        AddObject: AddObject,
        DuplicateObject: DuplicateObject,
        RemoveObject: RemoveObject,
        UpdateEditor: UpdateEditor,
        FormatJson: FormatJson
    };
});
