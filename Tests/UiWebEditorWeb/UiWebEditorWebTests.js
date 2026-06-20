// 模块: Tests UiWebEditorWeb
// 文件: Tests/UiWebEditorWeb/UiWebEditorWebTests.js

const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const model = require("../../Tools/UiWebEditorWeb/EditorModel.js");

function LoadSampleDocument() {
    const sample_path = path.resolve(__dirname, "../../Tools/UiWebEditorWeb/Samples/GenericUiLayout.json");
    const text = fs.readFileSync(sample_path, "utf8");
    return JSON.parse(text);
}

function LoadHtmlShell() {
    const html_path = path.resolve(__dirname, "../../Tools/UiWebEditorWeb/Index.html");
    return fs.readFileSync(html_path, "utf8");
}

function AssertClose(actual, expected) {
    const delta = Math.abs(actual - expected);
    assert.ok(delta <= 0.000001, String(actual) + " should equal " + String(expected));
}

function AssertRectClose(actual, expected) {
    AssertClose(actual.x, expected.x);
    AssertClose(actual.y, expected.y);
    AssertClose(actual.width, expected.width);
    AssertClose(actual.height, expected.height);
}

function AssertCanvasRectClose(actual, expected) {
    AssertClose(actual.left, expected.left);
    AssertClose(actual.top, expected.top);
    AssertClose(actual.width, expected.width);
    AssertClose(actual.height, expected.height);
}

function FindCanvasItem(document, node_id) {
    const items = model.BuildCanvasItems(document);
    return items.find(function MatchItem(item) {
        return item.nodeId === node_id;
    });
}

function FindRuntimeNode(document, node_id) {
    return document.nodes.find(function MatchNode(node) {
        return node.nodeId === node_id;
    });
}

function CreateFullRectTransform() {
    return model.NormalizeRectTransform({});
}

function CreateTestNode(node_id, parent_id, name, rect_transform, layer) {
    return {
        nodeId: node_id,
        parentId: parent_id,
        name: name,
        component: "Container",
        text: "",
        rectTransform: rect_transform,
        siblingOrder: 0,
        layer: layer,
        visible: true,
        enabled: true,
        hitTestable: true
    };
}

function CreateResolveOrderDocument() {
    const document = model.CreateDefaultDocument();
    const parent_transform = model.ApplyEngineRectToRectTransform(
        model.DEFAULT_VIEWPORT_RECT,
        CreateFullRectTransform(),
        { x: 100, y: 100, width: 200, height: 150 });
    document.schema.rootNodeId = 100;
    document.nodes = [
        CreateTestNode(1, 2, "Child", CreateFullRectTransform(), 2),
        CreateTestNode(2, 100, "Parent", parent_transform, 1),
        CreateTestNode(100, 0, "Root", CreateFullRectTransform(), 0)
    ];
    document.layouts = [];
    document.styleRefs = [];
    document.resourceRefs = [];
    document.eventBindings = [];
    document.statePreview = [];
    return document;
}

function TestDefaultDocumentValidates() {
    const document = model.CreateDefaultDocument();
    const normalized = model.NormalizeDocument(document);
    const result = model.ValidateDocument(document);
    assert.equal(result.status, "Success");
    assert.equal(result.summary.nodeCount, 3);
    assert.equal(result.summary.issueCount, 0);
    assert.ok(document.nodes[0].rectTransform);
    assert.equal(document.nodes[0].rect, undefined);
    assert.equal(normalized.nodes[1].text, undefined);
    assert.equal(normalized.nodes[1].components.text.content, "Title");
    assert.equal(normalized.nodes[2].components.button.label, "Action");
    assert.equal(result.summary.componentCounts.Text, 1);
    assert.equal(result.summary.componentCounts.Button, 1);
}

function TestComponentMatrixDeclaresImplementedAndBacklog() {
    const matrix = model.GetComponentMatrix();
    const implemented = matrix.implemented.map(function MapComponent(record) {
        return record.component;
    });
    const backlog = matrix.backlog.map(function MapComponent(record) {
        return record.component;
    });
    assert.deepEqual(implemented, ["Container", "Text", "Image", "Button", "Slider"]);
    assert.ok(matrix.records.includes("common"));
    assert.ok(backlog.includes("Toggle"));
    assert.ok(backlog.includes("TextAutoSize"));
    assert.ok(backlog.includes("NativeSchemaValidator"));
}

function TestDuplicateNodeReportsIssue() {
    const document = model.CreateDefaultDocument();
    const duplicate = model.NormalizeDocument(document).nodes[1];
    document.nodes.push(duplicate);
    const result = model.ValidateDocument(document);
    const issue = result.issues.find(function MatchIssue(record) {
        return record.kind === "DuplicateNodeId";
    });
    assert.equal(result.status, "IssuesFound");
    assert.ok(issue);
}

function TestMissingParentReportsIssue() {
    const document = model.CreateDefaultDocument();
    document.nodes[1].parentId = 9999;
    const result = model.ValidateDocument(document);
    const issue = result.issues.find(function MatchIssue(record) {
        return record.kind === "MissingParentNode";
    });
    assert.equal(result.status, "IssuesFound");
    assert.ok(issue);
}

function TestRectTransformResolveIgnoresNodeRecordOrder() {
    const document = CreateResolveOrderDocument();
    const result = model.ValidateDocument(document);
    const resolved = model.ResolveDocumentRects(document);
    const parent_result = resolved.get(2);
    const child_result = resolved.get(1);
    assert.equal(result.status, "Success");
    assert.equal(result.summary.issueCount, 0);
    assert.equal(parent_result.status, "Success");
    assert.equal(child_result.status, "Success");
    AssertRectClose(parent_result.rect, { x: 100, y: 100, width: 200, height: 150 });
    AssertRectClose(child_result.rect, parent_result.rect);
}

function TestCyclicParentReportsIssue() {
    const document = model.CreateDefaultDocument();
    document.schema.rootNodeId = 1;
    document.nodes = [
        CreateTestNode(1, 2, "CycleA", CreateFullRectTransform(), 0),
        CreateTestNode(2, 1, "CycleB", CreateFullRectTransform(), 1)
    ];
    document.layouts = [];
    document.styleRefs = [];
    document.resourceRefs = [];
    document.eventBindings = [];
    document.statePreview = [];
    const result = model.ValidateDocument(document);
    const resolved = model.ResolveDocumentRects(document);
    const hierarchy = model.BuildHierarchy(document);
    const issue = result.issues.find(function MatchIssue(record) {
        return record.kind === "CyclicParentNode";
    });
    assert.equal(result.status, "IssuesFound");
    assert.ok(issue);
    assert.equal(hierarchy.length, 2);
    assert.equal(resolved.get(1).status, "CyclicParentNode");
    assert.equal(resolved.get(2).status, "CyclicParentNode");
}

function TestRectTransformGoldenResolve() {
    const parent = { x: 10, y: 20, width: 400, height: 200 };
    const transform = {
        anchorMin: { x: 0.25, y: 0.25 },
        anchorMax: { x: 0.75, y: 0.75 },
        pivot: { x: 0.25, y: 0.75 },
        offsetMin: { x: 5, y: 7 },
        offsetMax: { x: -11, y: -13 },
        margin: { left: 2, top: 3, right: 4, bottom: 5 },
        padding: { left: 1, top: 2, right: 3, bottom: 4 },
        dpiScale: 2
    };
    const result = model.ResolveRectTransform(parent, transform);
    assert.equal(result.status, "Success");
    AssertRectClose(result.rect, { x: 124, y: 94, width: 156, height: 44 });
    AssertRectClose(result.contentRect, { x: 126, y: 102, width: 148, height: 32 });
    AssertClose(result.pivotPoint.x, 163);
    AssertClose(result.pivotPoint.y, 127);
}

function TestAdapterForwardConversion() {
    const engine_rect = { x: 124, y: 94, width: 156, height: 44 };
    const viewport = {
        runtimeRect: { x: 0, y: 0, width: 500, height: 300 },
        scale: 1,
        panX: 0,
        panY: 0
    };
    const canvas_rect = model.EngineRectToCanvasRect(engine_rect, viewport);
    AssertCanvasRectClose(canvas_rect, { left: 124, top: 162, width: 156, height: 44 });
}

function TestAdapterViewportScaleAndPanBoundary() {
    const engine_rect = { x: -100, y: -50, width: 1000, height: 600 };
    const viewport = {
        runtimeRect: { x: -100, y: -50, width: 1000, height: 600 },
        scale: 1.5,
        panX: 12,
        panY: 20
    };
    const canvas_rect = model.EngineRectToCanvasRect(engine_rect, viewport);
    const inverse_rect = model.CanvasRectToEngineRect(canvas_rect, viewport);
    AssertCanvasRectClose(canvas_rect, { left: 12, top: 20, width: 1500, height: 900 });
    AssertRectClose(inverse_rect, engine_rect);
}

function TestAdapterInverseEditPath() {
    const document = model.CreateDefaultDocument();
    const before_item = FindCanvasItem(document, 2);
    const before_runtime = before_item.runtimeRect;
    const next_canvas_rect = {
        left: before_item.canvasRect.left + 10,
        top: before_item.canvasRect.top + 20,
        width: before_item.canvasRect.width + 30,
        height: before_item.canvasRect.height + 10
    };
    const next_document = model.UpdateNodeFromCanvasRect(document, 2, next_canvas_rect);
    const after_item = FindCanvasItem(next_document, 2);
    AssertCanvasRectClose(after_item.canvasRect, next_canvas_rect);
    AssertClose(after_item.runtimeRect.x, before_runtime.x + 10);
    AssertClose(after_item.runtimeRect.y, before_runtime.y - 30);
    AssertClose(after_item.runtimeRect.width, before_runtime.width + 30);
    AssertClose(after_item.runtimeRect.height, before_runtime.height + 10);
}

function TestAddNodeUpdatesSnapshots() {
    const document = model.CreateDefaultDocument();
    const next_document = model.AddNode(document, "Text");
    const hierarchy = model.BuildHierarchy(next_document);
    const canvas = model.BuildCanvasItems(next_document);
    const inspector = model.BuildInspector(next_document);
    const item = FindCanvasItem(next_document, inspector.node.nodeId);
    assert.equal(next_document.nodes.length, 4);
    assert.equal(hierarchy.length, 4);
    assert.equal(canvas.length, 4);
    assert.equal(inspector.node.component, "Text");
    assert.equal(inspector.node.text, undefined);
    assert.equal(inspector.node.components.text.content, "Text4");
    assert.ok(inspector.node.rectTransform);
    assert.ok(item.canvasRect);
    assert.ok(item.runtimeRect);
}

function TestAddImageAndSliderUseTypedRecords() {
    const document = model.CreateDefaultDocument();
    const with_image = model.AddNode(document, "Image");
    const with_slider = model.AddNode(with_image, "Slider");
    const normalized = model.NormalizeDocument(with_slider);
    const image_node = FindRuntimeNode(normalized, 4);
    const slider_node = FindRuntimeNode(normalized, 5);
    assert.equal(image_node.component, "Image");
    assert.equal(image_node.components.image.imageType, "Simple");
    assert.equal(slider_node.component, "Slider");
    assert.equal(slider_node.components.slider.axis, "Horizontal");
    assert.equal(slider_node.hitTestable, true);
}

function TestMoveNodeReparentsAndPreservesRuntimeRect() {
    const document = model.CreateDefaultDocument();
    const before_item = FindCanvasItem(document, 3);
    const next_document = model.MoveNode(document, 3, 2, 0);
    const after_node = FindRuntimeNode(next_document, 3);
    const after_item = FindCanvasItem(next_document, 3);
    assert.equal(after_node.parentId, 2);
    assert.equal(after_node.siblingOrder, 0);
    assert.equal(next_document.editor.selectedNodeId, 3);
    assert.equal(next_document.editor.dirty, true);
    AssertCanvasRectClose(after_item.canvasRect, before_item.canvasRect);
}

function TestMoveNodeReordersSiblings() {
    const document = model.AddNode(model.CreateDefaultDocument(), "Image");
    const next_document = model.MoveNode(document, 2, 1, 2);
    const root_children = model.BuildHierarchy(next_document).filter(function MatchRootChild(item) {
        return item.parentId === 1;
    }).map(function MapNodeId(item) {
        return item.nodeId;
    });
    assert.deepEqual(root_children, [3, 4, 2]);
}

function TestMoveNodePreventsCyclesAndRootMove() {
    const document = model.CreateDefaultDocument();
    const with_node = model.AddNode(document, "Container");
    const nested_document = model.MoveNode(with_node, 4, 2, 0);
    const cyclic_document = model.MoveNode(nested_document, 2, 4, 0);
    const root_document = model.MoveNode(nested_document, 1, 2, 0);
    assert.equal(model.CanMoveNode(nested_document, 2, 4), false);
    assert.equal(model.CanMoveNode(nested_document, 1, 2), false);
    assert.equal(FindRuntimeNode(cyclic_document, 2).parentId, 1);
    assert.equal(FindRuntimeNode(root_document, 1).parentId, 0);
}

function TestValidParentOptionsExcludeDescendants() {
    const document = model.CreateDefaultDocument();
    const with_node = model.AddNode(document, "Container");
    const nested_document = model.MoveNode(with_node, 4, 2, 0);
    const options = model.GetValidParentOptions(nested_document, 2).map(function MapOption(option) {
        return option.nodeId;
    });
    assert.ok(options.includes(1));
    assert.equal(options.includes(2), false);
    assert.equal(options.includes(4), false);
    assert.deepEqual(model.GetValidParentOptions(nested_document, 1), []);
}

function TestRemoveNodeRemovesChildRecords() {
    const document = model.CreateDefaultDocument();
    const next_document = model.RemoveNode(document, 3);
    const runtime_document = model.BuildRuntimeDocument(next_document);
    const node = runtime_document.nodes.find(function MatchNode(record) {
        return record.nodeId === 3;
    });
    const binding = runtime_document.eventBindings.find(function MatchBinding(record) {
        return record.nodeId === 3;
    });
    assert.equal(node, undefined);
    assert.equal(binding, undefined);
}

function TestRuntimeExportStripsEditorState() {
    const document = model.CreateDefaultDocument();
    const runtime_document = model.BuildRuntimeDocument(document);
    assert.equal(runtime_document.editor, undefined);
    assert.equal(runtime_document.statePreview, undefined);
    assert.equal(runtime_document.nodes.length, document.nodes.length);
    assert.equal(runtime_document.theme.tokens.length, document.theme.tokens.length);
    assert.equal(runtime_document.nodes[0].rect, undefined);
    assert.ok(runtime_document.nodes[0].rectTransform);
    assert.equal(runtime_document.nodes[1].text, undefined);
    assert.equal(runtime_document.nodes[1].components.text.content, "Title");
    assert.equal(runtime_document.nodes[2].components.button.label, "Action");
    assert.equal(runtime_document.nodes[2].components.text, undefined);
    assert.equal(runtime_document.nodes[2].components.common.styleKey, 1103);
    runtime_document.nodes[0].name = "Changed";
    runtime_document.nodes[0].rectTransform.offsetMin.x = 77;
    assert.equal(document.nodes[0].name, "Root");
    assert.notEqual(document.nodes[0].rectTransform.offsetMin.x, 77);
}

function TestLegacyRectMigratesToRectTransform() {
    const document = model.CreateDefaultDocument();
    delete document.nodes[1].rectTransform;
    document.nodes[1].rect = {
        x: 40,
        y: 36,
        width: 420,
        height: 48
    };
    const normalized = model.NormalizeDocument(document);
    const item = FindCanvasItem(normalized, 2);
    assert.equal(normalized.nodes[1].rect, undefined);
    assert.ok(normalized.nodes[1].rectTransform);
    AssertCanvasRectClose(item.canvasRect, { left: 40, top: 36, width: 420, height: 48 });
}

function TestLegacyTextMigratesToTypedTextRecord() {
    const document = model.CreateDefaultDocument();
    delete document.nodes[1].components;
    document.nodes[1].text = "Legacy Title";
    const normalized = model.NormalizeDocument(document);
    const runtime_document = model.BuildRuntimeDocument(document);
    assert.equal(normalized.nodes[1].text, undefined);
    assert.equal(normalized.nodes[1].components.text.content, "Legacy Title");
    assert.equal(runtime_document.nodes[1].text, undefined);
    assert.equal(runtime_document.nodes[1].components.text.content, "Legacy Title");
}

function TestBacklogComponentReportsNeedsNativeRuntime() {
    const document = model.CreateDefaultDocument();
    document.nodes[1].component = "Toggle";
    const result = model.ValidateDocument(document);
    const issue = result.issues.find(function MatchIssue(record) {
        return record.kind === "NeedsNativeRuntime";
    });
    assert.equal(result.status, "IssuesFound");
    assert.ok(issue);
}

function TestSampleFixtureValidates() {
    const document = LoadSampleDocument();
    const result = model.ValidateDocument(document);
    const runtime_document = model.BuildRuntimeDocument(document);
    assert.equal(result.status, "Success");
    assert.equal(result.summary.layoutCount, 1);
    assert.equal(result.summary.resourceRefCount, 3);
    assert.equal(result.summary.eventBindingCount, 2);
    assert.equal(result.summary.componentCounts.Image, 1);
    assert.equal(result.summary.componentCounts.Slider, 1);
    assert.equal(runtime_document.nodes[0].rect, undefined);
    assert.ok(runtime_document.nodes[0].rectTransform);
    assert.equal(runtime_document.nodes[0].components.container.layoutType, "Stack");
    assert.equal(runtime_document.nodes[1].components.text.content, "Sample Title");
    assert.equal(runtime_document.nodes[2].components.image.spriteResourceKey, 23002);
    assert.equal(runtime_document.nodes[3].components.button.submitEventKey, 43001);
    assert.equal(runtime_document.nodes[4].components.slider.value, 60);
}

function TestHtmlShellReferencesStaticAssets() {
    const html = LoadHtmlShell();
    assert.ok(html.includes("Style.css"));
    assert.ok(html.includes("EditorModel.js"));
    assert.ok(html.includes("App.js"));
    assert.ok(html.includes("hierarchy-list"));
    assert.ok(html.includes("canvas-surface"));
    assert.ok(html.includes("canvas-fit-button"));
    assert.ok(html.includes("runtime-json-toggle-button"));
    assert.ok(html.includes("add-image-button"));
    assert.ok(html.includes("add-slider-button"));
    assert.ok(html.includes("inspector-panel"));
    assert.ok(html.includes("validation-panel"));
}

function RunTests() {
    const tests = [
        { name: "UiWebEditorWeb_DefaultDocumentValidates", run: TestDefaultDocumentValidates },
        { name: "UiWebEditorWeb_ComponentMatrixDeclaresImplementedAndBacklog", run: TestComponentMatrixDeclaresImplementedAndBacklog },
        { name: "UiWebEditorWeb_DuplicateNodeReportsIssue", run: TestDuplicateNodeReportsIssue },
        { name: "UiWebEditorWeb_MissingParentReportsIssue", run: TestMissingParentReportsIssue },
        { name: "UiWebEditorWeb_RectTransformResolveIgnoresNodeRecordOrder", run: TestRectTransformResolveIgnoresNodeRecordOrder },
        { name: "UiWebEditorWeb_CyclicParentReportsIssue", run: TestCyclicParentReportsIssue },
        { name: "UiWebEditorWeb_RectTransformGoldenResolve", run: TestRectTransformGoldenResolve },
        { name: "UiWebEditorWeb_AdapterForwardConversion", run: TestAdapterForwardConversion },
        { name: "UiWebEditorWeb_AdapterViewportScaleAndPanBoundary", run: TestAdapterViewportScaleAndPanBoundary },
        { name: "UiWebEditorWeb_AdapterInverseEditPath", run: TestAdapterInverseEditPath },
        { name: "UiWebEditorWeb_AddNodeUpdatesSnapshots", run: TestAddNodeUpdatesSnapshots },
        { name: "UiWebEditorWeb_AddImageAndSliderUseTypedRecords", run: TestAddImageAndSliderUseTypedRecords },
        { name: "UiWebEditorWeb_MoveNodeReparentsAndPreservesRuntimeRect", run: TestMoveNodeReparentsAndPreservesRuntimeRect },
        { name: "UiWebEditorWeb_MoveNodeReordersSiblings", run: TestMoveNodeReordersSiblings },
        { name: "UiWebEditorWeb_MoveNodePreventsCyclesAndRootMove", run: TestMoveNodePreventsCyclesAndRootMove },
        { name: "UiWebEditorWeb_ValidParentOptionsExcludeDescendants", run: TestValidParentOptionsExcludeDescendants },
        { name: "UiWebEditorWeb_RemoveNodeRemovesChildRecords", run: TestRemoveNodeRemovesChildRecords },
        { name: "UiWebEditorWeb_RuntimeExportStripsEditorState", run: TestRuntimeExportStripsEditorState },
        { name: "UiWebEditorWeb_LegacyRectMigratesToRectTransform", run: TestLegacyRectMigratesToRectTransform },
        { name: "UiWebEditorWeb_LegacyTextMigratesToTypedTextRecord", run: TestLegacyTextMigratesToTypedTextRecord },
        { name: "UiWebEditorWeb_BacklogComponentReportsNeedsNativeRuntime", run: TestBacklogComponentReportsNeedsNativeRuntime },
        { name: "UiWebEditorWeb_SampleFixtureValidates", run: TestSampleFixtureValidates },
        { name: "UiWebEditorWeb_HtmlShellReferencesStaticAssets", run: TestHtmlShellReferencesStaticAssets }
    ];

    tests.forEach(function RunTest(test) {
        test.run();
        console.log("PASS " + test.name);
    });
}

RunTests();
