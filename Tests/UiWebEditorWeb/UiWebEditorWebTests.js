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

function TestDefaultDocumentValidates() {
    const document = model.CreateDefaultDocument();
    const result = model.ValidateDocument(document);
    assert.equal(result.status, "Success");
    assert.equal(result.summary.nodeCount, 3);
    assert.equal(result.summary.issueCount, 0);
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

function TestAddNodeUpdatesSnapshots() {
    const document = model.CreateDefaultDocument();
    const next_document = model.AddNode(document, "Text");
    const hierarchy = model.BuildHierarchy(next_document);
    const canvas = model.BuildCanvasItems(next_document);
    const inspector = model.BuildInspector(next_document);
    assert.equal(next_document.nodes.length, 4);
    assert.equal(hierarchy.length, 4);
    assert.equal(canvas.length, 4);
    assert.equal(inspector.node.component, "Text");
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
    runtime_document.nodes[0].name = "Changed";
    assert.equal(document.nodes[0].name, "Root");
}

function TestSampleFixtureValidates() {
    const document = LoadSampleDocument();
    const result = model.ValidateDocument(document);
    assert.equal(result.status, "Success");
    assert.equal(result.summary.layoutCount, 1);
    assert.equal(result.summary.resourceRefCount, 1);
    assert.equal(result.summary.eventBindingCount, 1);
}

function TestHtmlShellReferencesStaticAssets() {
    const html = LoadHtmlShell();
    assert.ok(html.includes("Style.css"));
    assert.ok(html.includes("EditorModel.js"));
    assert.ok(html.includes("App.js"));
    assert.ok(html.includes("hierarchy-list"));
    assert.ok(html.includes("canvas-surface"));
    assert.ok(html.includes("inspector-panel"));
    assert.ok(html.includes("validation-panel"));
}

function RunTests() {
    const tests = [
        { name: "UiWebEditorWeb_DefaultDocumentValidates", run: TestDefaultDocumentValidates },
        { name: "UiWebEditorWeb_DuplicateNodeReportsIssue", run: TestDuplicateNodeReportsIssue },
        { name: "UiWebEditorWeb_MissingParentReportsIssue", run: TestMissingParentReportsIssue },
        { name: "UiWebEditorWeb_AddNodeUpdatesSnapshots", run: TestAddNodeUpdatesSnapshots },
        { name: "UiWebEditorWeb_RemoveNodeRemovesChildRecords", run: TestRemoveNodeRemovesChildRecords },
        { name: "UiWebEditorWeb_RuntimeExportStripsEditorState", run: TestRuntimeExportStripsEditorState },
        { name: "UiWebEditorWeb_SampleFixtureValidates", run: TestSampleFixtureValidates },
        { name: "UiWebEditorWeb_HtmlShellReferencesStaticAssets", run: TestHtmlShellReferencesStaticAssets }
    ];

    tests.forEach(function RunTest(test) {
        test.run();
        console.log("PASS " + test.name);
    });
}

RunTests();
