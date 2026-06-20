// 模块: Tests AnimationWebEditorWeb
// 文件: Tests/AnimationWebEditorWeb/AnimationWebEditorWebTests.js

const assert = require("node:assert/strict");
const fs = require("node:fs");
const path = require("node:path");

const model = require("../../Tools/AnimationWebEditorWeb/AnimationEditorModel.js");

function LoadSampleDocument() {
    const sample_path = path.resolve(__dirname, "../../Tools/AnimationWebEditorWeb/Samples/AnimationDocument.json");
    const text = fs.readFileSync(sample_path, "utf8");
    return JSON.parse(text);
}

function LoadToolFile(relative_path) {
    const file_path = path.resolve(__dirname, "../../Tools/AnimationWebEditorWeb", relative_path);
    return fs.readFileSync(file_path, "utf8");
}

function FindIssue(result, kind) {
    return result.issues.find(function MatchIssue(issue) {
        return issue.kind === kind;
    });
}

function FindClip(document, clip_id) {
    return document.clips.find(function MatchClip(record) {
        return record.clipId === clip_id;
    });
}

function FindTrack(document, track_id) {
    return document.tracks.find(function MatchTrack(record) {
        return record.trackId === track_id;
    });
}

function FindMarker(document, marker_id) {
    return document.eventMarkers.find(function MatchMarker(record) {
        return record.markerId === marker_id;
    });
}

function TestDefaultDocumentValidates() {
    const document = model.CreateDefaultDocument();
    const normalized = model.NormalizeDocument(document);
    const result = model.ValidateDocument(document);
    const runtime_document = model.BuildRuntimeDocument(document);
    assert.equal(result.status, "Success");
    assert.equal(result.summary.clipCount, 2);
    assert.equal(result.summary.trackCount, 3);
    assert.equal(result.summary.eventCount, 2);
    assert.equal(normalized.schema.schemaId, "YUANIM");
    assert.match(runtime_document.schema.documentHash, /^0x[0-9A-F]{8}$/);
    assert.equal(runtime_document.schema.clipCount, 2);
}

function TestSampleFixtureValidates() {
    const document = LoadSampleDocument();
    const result = model.ValidateDocument(document);
    const runtime_document = model.BuildRuntimeDocument(document);
    assert.equal(result.status, "Success");
    assert.equal(runtime_document.clips.length, 2);
    assert.equal(runtime_document.tracks.length, 3);
    assert.equal(runtime_document.eventMarkers.length, 3);
    assert.equal(runtime_document.resourceRefs.length, 2);
}

function TestRuntimeExportStripsEditorSidecar() {
    const document = model.CreateDefaultDocument();
    const runtime_a = model.BuildRuntimeDocument(document);
    document.editor.selectedClipId = 2;
    document.editor.selectedTrackId = 21;
    document.editor.selectedMarkerId = 10002;
    document.editor.timeline.zoom = 0.2;
    document.editor.timeline.scrubberTick = 777;
    document.editor.clipLabels["1"] = "Changed Label";
    const runtime_b = model.BuildRuntimeDocument(document);
    assert.equal(runtime_a.schema.documentHash, runtime_b.schema.documentHash);
    assert.equal(runtime_b.editor, undefined);
    assert.equal(runtime_b.clips[0].label, undefined);
    assert.equal(runtime_b.clips[0].order, undefined);
    assert.equal(runtime_b.tracks[0].order, undefined);
    assert.equal(runtime_b.eventMarkers[0].order, undefined);
    assert.equal(runtime_b.resourceRefs[0].label, undefined);
}

function TestRuntimeHashTracksRuntimeFieldsOnly() {
    const document = model.CreateDefaultDocument();
    const runtime_a = model.BuildRuntimeDocument(document);
    document.editor.trackColors["11"] = "#ff0000";
    const runtime_b = model.BuildRuntimeDocument(document);
    document.clips[0].durationTicks += 1;
    const runtime_c = model.BuildRuntimeDocument(document);
    assert.equal(runtime_a.schema.documentHash, runtime_b.schema.documentHash);
    assert.notEqual(runtime_a.schema.documentHash, runtime_c.schema.documentHash);
}

function TestNormalizeProducesDeterministicOrder() {
    const document = model.CreateDefaultDocument();
    document.clips[0].order = 30;
    document.clips[1].order = 10;
    document.tracks[0].order = 20;
    document.tracks[1].order = 10;
    document.eventMarkers[0].startTick = 900;
    document.eventMarkers[1].clipId = 1;
    document.eventMarkers[1].startTick = 100;
    document.clips[0].eventCount = 2;
    document.clips[1].eventCount = 0;
    const normalized = model.NormalizeDocument(document);
    const clip_ids = model.BuildClipList(normalized).map(function MapClip(record) {
        return record.clipId;
    });
    const runtime_document = model.BuildRuntimeDocument(normalized);
    assert.deepEqual(clip_ids, [2, 1]);
    assert.deepEqual(runtime_document.clips.map(function MapClip(record) {
        return record.clipId;
    }), [2, 1]);
    assert.deepEqual(runtime_document.eventMarkers.map(function MapMarker(record) {
        return record.markerId;
    }), [10002, 10001]);
}

function TestHeaderAndClipCountDiagnostics() {
    const document = model.CreateDefaultDocument();
    document.schema.clipCount = 99;
    document.clips[0].trackCount = 99;
    document.clips[0].eventCount = 99;
    const result = model.ValidateDocument(document);
    assert.equal(result.status, "IssuesFound");
    assert.ok(FindIssue(result, "HeaderCountMismatch"));
    assert.ok(FindIssue(result, "ClipTrackCountMismatch"));
    assert.ok(FindIssue(result, "ClipEventCountMismatch"));
}

function TestDuplicateDiagnostics() {
    const document = model.CreateDefaultDocument();
    document.clips.push(Object.assign({}, document.clips[0]));
    document.tracks.push(Object.assign({}, document.tracks[0]));
    document.eventMarkers.push(Object.assign({}, document.eventMarkers[0]));
    document.resourceRefs.push(Object.assign({}, document.resourceRefs[0]));
    const result = model.ValidateDocument(document);
    assert.equal(result.status, "IssuesFound");
    assert.ok(FindIssue(result, "DuplicateClipId"));
    assert.ok(FindIssue(result, "DuplicateTrackId"));
    assert.ok(FindIssue(result, "DuplicateMarkerId"));
    assert.ok(FindIssue(result, "DuplicateResourceId"));
}

function TestMissingAndInvalidRecordDiagnostics() {
    const document = model.CreateDefaultDocument();
    document.clips[0].loopMode = "InvalidLoop";
    document.tracks[0].clipId = 999;
    document.tracks[0].targetId = 0;
    document.tracks[0].channelKind = "InvalidChannel";
    document.tracks[0].sampleFormat = "InvalidFormat";
    document.eventMarkers[0].clipId = 999;
    document.eventMarkers[0].validationStatus = "InvalidStatus";
    document.resourceRefs[0].kind = "InvalidResource";
    document.resourceRefs[0].resourceKey = "";
    const result = model.ValidateDocument(document);
    assert.equal(result.status, "IssuesFound");
    assert.ok(FindIssue(result, "UnsupportedLoopMode"));
    assert.ok(FindIssue(result, "TrackWithoutClip"));
    assert.ok(FindIssue(result, "InvalidTargetId"));
    assert.ok(FindIssue(result, "UnsupportedChannelKind"));
    assert.ok(FindIssue(result, "UnsupportedSampleFormat"));
    assert.ok(FindIssue(result, "UnsupportedEventStatus"));
    assert.ok(FindIssue(result, "EventStatusMismatch"));
    assert.ok(FindIssue(result, "InvalidResourceKind"));
    assert.ok(FindIssue(result, "InvalidResourceKey"));
}

function TestEventStatusDiagnostics() {
    const document = model.CreateDefaultDocument();
    document.eventMarkers[0].endTick = document.clips[0].durationTicks + 1;
    document.eventMarkers[0].validationStatus = "Success";
    const invalid_result = model.ValidateDocument(document);
    assert.ok(FindIssue(invalid_result, "EventStatusMismatch"));

    const duplicate_document = model.CreateDefaultDocument();
    const duplicate = Object.assign({}, duplicate_document.eventMarkers[0], {
        markerId: 77777,
        validationStatus: "Success"
    });
    duplicate_document.eventMarkers.push(duplicate);
    duplicate_document.schema.eventCount += 1;
    duplicate_document.clips[0].eventCount += 1;
    const duplicate_result = model.ValidateDocument(duplicate_document);
    assert.ok(FindIssue(duplicate_result, "EventStatusMismatch"));
}

function TestEventOverflowDiagnostic() {
    const document = model.CreateDefaultDocument();
    document.eventMarkers = [];
    for (let index = 0; index < model.MAX_EVENT_MARKER_COUNT + 1; ++index) {
        document.eventMarkers.push({
            markerId: 50000 + index,
            clipId: 1,
            eventId: 60000 + index,
            startTick: index,
            endTick: index,
            payloadId: 0,
            validationStatus: "Success",
            order: index
        });
    }
    document.schema.eventCount = document.eventMarkers.length;
    document.clips[0].eventCount = document.eventMarkers.length;
    document.clips[1].eventCount = 0;
    const result = model.ValidateDocument(document);
    assert.ok(FindIssue(result, "EventStatusMismatch"));
}

function TestOperationsUpdateRuntimeRecords() {
    let document = model.CreateDefaultDocument();
    document = model.AddClip(document);
    const clip_id = document.editor.selectedClipId;
    document = model.AddTrack(document);
    const track_id = document.editor.selectedTrackId;
    document = model.AddEventMarker(document);
    const marker_id = document.editor.selectedMarkerId;
    assert.ok(FindClip(document, clip_id));
    assert.ok(FindTrack(document, track_id));
    assert.ok(FindMarker(document, marker_id));
    document = model.RemoveSelected(document);
    assert.equal(FindMarker(document, marker_id), undefined);
}

function TestUpdateOperationsKeepRuntimeBoundary() {
    let document = model.CreateDefaultDocument();
    document = model.UpdateSchema(document, { animationDocumentId: "EditedDocument" });
    document = model.UpdateClip(document, 1, { durationTicks: 1500, loopMode: "Once" });
    document = model.UpdateTrack(document, 11, { targetId: 909, keyCount: 12 });
    document = model.UpdateEventMarker(document, 10001, { startTick: 400, endTick: 400 });
    document = model.UpdateLabel(document, "clipLabels", 1, "Editor Label");
    const inspector = model.BuildInspector(document);
    const runtime_document = model.BuildRuntimeDocument(document);
    assert.equal(inspector.schema.animationDocumentId, "EditedDocument");
    assert.equal(inspector.clipLabel, "Editor Label");
    assert.equal(runtime_document.schema.animationDocumentId, "EditedDocument");
    assert.equal(runtime_document.clips[0].durationTicks, 1500);
    assert.equal(runtime_document.tracks[0].targetId, 909);
    assert.equal(runtime_document.eventMarkers[0].startTick, 400);
    assert.equal(runtime_document.clips[0].label, undefined);
}

function TestTimelineItemsUseEditorSidecarOnly() {
    const document = model.CreateDefaultDocument();
    const markers_a = model.BuildTimelineMarkers(document);
    document.editor.timeline.zoom = 0.2;
    document.editor.timeline.scrollX = 40;
    const markers_b = model.BuildTimelineMarkers(document);
    const runtime_document = model.BuildRuntimeDocument(document);
    assert.notEqual(markers_a[0].left, markers_b[0].left);
    assert.equal(runtime_document.editor, undefined);
    assert.equal(runtime_document.schema.documentHash, model.BuildRuntimeDocument(model.CreateDefaultDocument()).schema.documentHash);
}

function TestHtmlShellReferencesStaticAssets() {
    const html = LoadToolFile("Index.html");
    assert.ok(html.includes("Style.css"));
    assert.ok(html.includes("AnimationEditorModel.js"));
    assert.ok(html.includes("App.js"));
    assert.ok(html.includes("runtime-json-toggle-button"));
    assert.ok(html.includes("timeline-view"));
    assert.ok(html.includes("inspector-panel"));
}

function TestForbiddenScopeTermsStayOutOfToolSurface() {
    const files = [
        "Index.html",
        "App.js",
        "AnimationEditorModel.js",
        "Samples/AnimationDocument.json"
    ];
    const forbidden_terms = [
        "Unity",
        "Unreal",
        "Animator",
        "Mecanim",
        "Sequencer",
        "Blueprint",
        "Anim" + "Graph",
        "game" + "play",
        "State" + "Machine",
        "Character",
        "Retarget",
        "Mocap",
        "Cinematic"
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
        { name: "AnimationWebEditorWeb_DefaultDocumentValidates", run: TestDefaultDocumentValidates },
        { name: "AnimationWebEditorWeb_SampleFixtureValidates", run: TestSampleFixtureValidates },
        { name: "AnimationWebEditorWeb_RuntimeExportStripsEditorSidecar", run: TestRuntimeExportStripsEditorSidecar },
        { name: "AnimationWebEditorWeb_RuntimeHashTracksRuntimeFieldsOnly", run: TestRuntimeHashTracksRuntimeFieldsOnly },
        { name: "AnimationWebEditorWeb_NormalizeProducesDeterministicOrder", run: TestNormalizeProducesDeterministicOrder },
        { name: "AnimationWebEditorWeb_HeaderAndClipCountDiagnostics", run: TestHeaderAndClipCountDiagnostics },
        { name: "AnimationWebEditorWeb_DuplicateDiagnostics", run: TestDuplicateDiagnostics },
        { name: "AnimationWebEditorWeb_MissingAndInvalidRecordDiagnostics", run: TestMissingAndInvalidRecordDiagnostics },
        { name: "AnimationWebEditorWeb_EventStatusDiagnostics", run: TestEventStatusDiagnostics },
        { name: "AnimationWebEditorWeb_EventOverflowDiagnostic", run: TestEventOverflowDiagnostic },
        { name: "AnimationWebEditorWeb_OperationsUpdateRuntimeRecords", run: TestOperationsUpdateRuntimeRecords },
        { name: "AnimationWebEditorWeb_UpdateOperationsKeepRuntimeBoundary", run: TestUpdateOperationsKeepRuntimeBoundary },
        { name: "AnimationWebEditorWeb_TimelineItemsUseEditorSidecarOnly", run: TestTimelineItemsUseEditorSidecarOnly },
        { name: "AnimationWebEditorWeb_HtmlShellReferencesStaticAssets", run: TestHtmlShellReferencesStaticAssets },
        { name: "AnimationWebEditorWeb_ForbiddenScopeTermsStayOutOfToolSurface", run: TestForbiddenScopeTermsStayOutOfToolSurface }
    ];

    tests.forEach(function RunTest(test) {
        test.run();
        console.log("PASS " + test.name);
    });
}

RunTests();
