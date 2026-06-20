// 模块: Tests SceneWebEditorWeb
// 文件: Tests/SceneWebEditorWeb/SceneWebEditorWebTests.js

const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const model = require("../../Tools/SceneWebEditorWeb/SceneEditorModel.js");

function LoadSampleDocument() {
    const sample_path = path.resolve(__dirname, "../../Tools/SceneWebEditorWeb/Samples/SceneDocument.json");
    const text = fs.readFileSync(sample_path, "utf8");
    return JSON.parse(text);
}

function LoadToolFile(relative_path) {
    const file_path = path.resolve(__dirname, "../../Tools/SceneWebEditorWeb", relative_path);
    return fs.readFileSync(file_path, "utf8");
}

function FindIssue(result, kind) {
    return result.issues.find(function MatchIssue(issue) {
        return issue.kind === kind;
    });
}

function FindObject(document, object_id) {
    return document.objects.find(function MatchObject(record) {
        return record.objectId === object_id;
    });
}

function TestDefaultDocumentValidates() {
    const document = model.CreateDefaultDocument();
    const normalized = model.NormalizeDocument(document);
    const result = model.ValidateDocument(document);
    const runtime_document = model.BuildRuntimeDocument(document);
    assert.equal(result.status, "Success");
    assert.equal(result.summary.objectCount, 3);
    assert.equal(result.summary.transformCount, 3);
    assert.equal(result.summary.resourceRefCount, 2);
    assert.equal(normalized.schema.schemaId, "YUSC");
    assert.match(runtime_document.schema.documentHash, /^0x[0-9A-F]{8}$/);
}

function TestSampleFixtureValidates() {
    const document = LoadSampleDocument();
    const result = model.ValidateDocument(document);
    const runtime_document = model.BuildRuntimeDocument(document);
    assert.equal(result.status, "Success");
    assert.equal(runtime_document.objects.length, 3);
    assert.equal(runtime_document.transforms.length, 3);
    assert.equal(runtime_document.resourceRefs.length, 2);
}

function TestRuntimeExportStripsEditorSidecar() {
    const document = model.CreateDefaultDocument();
    const runtime_a = model.BuildRuntimeDocument(document);
    document.editor.selectedObjectId = 3;
    document.editor.viewport.panX = 77;
    document.editor.grid.enabled = false;
    const runtime_b = model.BuildRuntimeDocument(document);
    assert.equal(runtime_a.schema.documentHash, runtime_b.schema.documentHash);
    assert.equal(runtime_b.editor, undefined);
    assert.equal(runtime_b.objects[0].objectId, 1);
    assert.equal(runtime_b.transforms[0].objectId, 1);
}

function TestNormalizeProducesDeterministicObjectOrder() {
    const document = model.CreateDefaultDocument();
    document.objects[0].order = 30;
    document.objects[1].order = 10;
    document.objects[2].order = 20;
    const normalized = model.NormalizeDocument(document);
    const list = model.BuildObjectList(normalized);
    assert.deepEqual(list.map(function MapId(record) {
        return record.objectId;
    }), [2, 3, 1]);
    assert.deepEqual(list.map(function MapOrder(record) {
        return record.order;
    }), [0, 1, 2]);
}

function TestDuplicateDiagnostics() {
    const document = model.CreateDefaultDocument();
    document.objects.push(Object.assign({}, document.objects[1]));
    document.transforms.push(Object.assign({}, document.transforms[1]));
    document.resourceRefs.push(Object.assign({}, document.resourceRefs[0]));
    const result = model.ValidateDocument(document);
    assert.equal(result.status, "IssuesFound");
    assert.ok(FindIssue(result, "DuplicateObjectId"));
    assert.ok(FindIssue(result, "DuplicateObjectKey"));
    assert.ok(FindIssue(result, "DuplicateTransform"));
    assert.ok(FindIssue(result, "DuplicateResourceTuple"));
}

function TestMissingAndInvalidRecordDiagnostics() {
    const document = model.CreateDefaultDocument();
    document.transforms = [
        {
            objectId: 999,
            position: { x: 0, y: 0, z: 0 },
            rotation: { x: 0, y: 0, z: 0 },
            scale: { x: 1, y: 1, z: 1 }
        }
    ];
    document.resourceRefs = [
        {
            objectId: 999,
            slot: "InvalidSlot",
            resourceKey: ""
        }
    ];
    const result = model.ValidateDocument(document);
    assert.equal(result.status, "IssuesFound");
    assert.ok(FindIssue(result, "TransformWithoutObject"));
    assert.ok(FindIssue(result, "MissingTransform"));
    assert.ok(FindIssue(result, "ResourceWithoutObject"));
    assert.ok(FindIssue(result, "InvalidResourceSlot"));
    assert.ok(FindIssue(result, "InvalidResourceKey"));
}

function TestObjectOperationsUpdateRuntimeRecords() {
    const document = model.CreateDefaultDocument();
    const added = model.AddObject(document);
    const added_id = added.editor.selectedObjectId;
    const duplicated = model.DuplicateObject(added, added_id);
    const duplicate_id = duplicated.editor.selectedObjectId;
    const removed = model.RemoveObject(duplicated, added_id);
    assert.ok(FindObject(added, added_id));
    assert.ok(FindObject(duplicated, duplicate_id));
    assert.equal(FindObject(removed, added_id), undefined);
    assert.ok(removed.transforms.find(function MatchTransform(record) {
        return record.objectId === duplicate_id;
    }));
}

function TestUpdateOperationsKeepRuntimeBoundary() {
    let document = model.CreateDefaultDocument();
    document = model.UpdateSchema(document, { sceneKey: "EditedScene" });
    document = model.UpdateObject(document, 2, { displayName: "Edited Marker", enabled: false });
    document = model.UpdateTransform(document, 2, { position: { x: 5, y: 1, z: -3 } });
    document = model.SetResourceRef(document, 2, "Material", "pkg://scene/edited-material");
    const inspector = model.BuildInspector(document);
    const runtime_document = model.BuildRuntimeDocument(document);
    assert.equal(inspector.schema.sceneKey, "EditedScene");
    assert.equal(inspector.object.displayName, "Edited Marker");
    assert.equal(runtime_document.objects[1].enabled, false);
    assert.equal(runtime_document.transforms[1].position.x, 5);
    assert.equal(runtime_document.resourceRefs.find(function MatchResource(record) {
        return record.objectId === 2 && record.slot === "Material";
    }).resourceKey, "pkg://scene/edited-material");
}

function TestViewportItemsUseEditorSidecarOnly() {
    const document = model.CreateDefaultDocument();
    const items_a = model.BuildViewportItems(document);
    document.editor.viewport.panX = 40;
    document.editor.viewport.zoom = 80;
    const items_b = model.BuildViewportItems(document);
    const runtime_document = model.BuildRuntimeDocument(document);
    assert.notEqual(items_a[1].left, items_b[1].left);
    assert.notEqual(items_a[1].top, items_b[1].top);
    assert.equal(runtime_document.editor, undefined);
    assert.equal(runtime_document.schema.documentHash, model.BuildRuntimeDocument(model.CreateDefaultDocument()).schema.documentHash);
}

function TestHtmlShellReferencesStaticAssets() {
    const html = LoadToolFile("Index.html");
    assert.ok(html.includes("Style.css"));
    assert.ok(html.includes("SceneEditorModel.js"));
    assert.ok(html.includes("App.js"));
    assert.ok(html.includes("runtime-json-toggle-button"));
    assert.ok(html.includes("scene-viewport"));
    assert.ok(html.includes("inspector-panel"));
}

function TestForbiddenScopeTermsStayOutOfToolSurface() {
    const files = [
        "Index.html",
        "App.js",
        "SceneEditorModel.js",
        "Samples/SceneDocument.json"
    ];
    const forbidden_terms = [
        "Game" + "Object",
        "Actor",
        "Prefab",
        "Unity",
        "Unreal",
        "parentId",
        "componentLifecycle"
    ];
    files.forEach(function CheckFile(relative_path) {
        const text = LoadToolFile(relative_path);
        forbidden_terms.forEach(function CheckTerm(term) {
            assert.equal(text.includes(term), false, relative_path + " must not contain " + term);
        });
    });
}

function RunTests() {
    const tests = [
        { name: "SceneWebEditorWeb_DefaultDocumentValidates", run: TestDefaultDocumentValidates },
        { name: "SceneWebEditorWeb_SampleFixtureValidates", run: TestSampleFixtureValidates },
        { name: "SceneWebEditorWeb_RuntimeExportStripsEditorSidecar", run: TestRuntimeExportStripsEditorSidecar },
        { name: "SceneWebEditorWeb_NormalizeProducesDeterministicObjectOrder", run: TestNormalizeProducesDeterministicObjectOrder },
        { name: "SceneWebEditorWeb_DuplicateDiagnostics", run: TestDuplicateDiagnostics },
        { name: "SceneWebEditorWeb_MissingAndInvalidRecordDiagnostics", run: TestMissingAndInvalidRecordDiagnostics },
        { name: "SceneWebEditorWeb_ObjectOperationsUpdateRuntimeRecords", run: TestObjectOperationsUpdateRuntimeRecords },
        { name: "SceneWebEditorWeb_UpdateOperationsKeepRuntimeBoundary", run: TestUpdateOperationsKeepRuntimeBoundary },
        { name: "SceneWebEditorWeb_ViewportItemsUseEditorSidecarOnly", run: TestViewportItemsUseEditorSidecarOnly },
        { name: "SceneWebEditorWeb_HtmlShellReferencesStaticAssets", run: TestHtmlShellReferencesStaticAssets },
        { name: "SceneWebEditorWeb_ForbiddenScopeTermsStayOutOfToolSurface", run: TestForbiddenScopeTermsStayOutOfToolSurface }
    ];

    tests.forEach(function RunTest(test) {
        test.run();
        console.log("PASS " + test.name);
    });
}

RunTests();
