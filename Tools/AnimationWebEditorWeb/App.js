// 模块: Tools AnimationWebEditorWeb
// 文件: Tools/AnimationWebEditorWeb/App.js

(function StartAnimationWebEditorWeb() {
    const model = window.AnimationWebEditorModel;
    const STORAGE_THEME = "YuEngine.AnimationWebEditor.Theme";
    const STORAGE_LAYOUT = "YuEngine.AnimationWebEditor.Layout";
    const state = {
        document: model.CreateDefaultDocument(),
        showRuntimeData: false,
        theme: "light",
        markerDrag: null,
        layoutDrag: null,
        layout: {
            leftWidth: 320,
            rightWidth: 380,
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

    function SelectClip(clip_id) {
        state.document = model.SelectClip(state.document, clip_id);
        Render();
    }

    function SelectTrack(track_id) {
        state.document = model.SelectTrack(state.document, track_id);
        Render();
    }

    function SelectMarker(marker_id) {
        state.document = model.SelectMarker(state.document, marker_id);
        Render();
    }

    function UpdateSelectedClip(patch) {
        const inspector = GetSelectedInspector();
        if (!inspector.clip) {
            return;
        }
        state.document = model.UpdateClip(state.document, inspector.clip.clipId, patch);
        Render();
    }

    function UpdateSelectedTrack(patch) {
        const inspector = GetSelectedInspector();
        if (!inspector.track) {
            return;
        }
        state.document = model.UpdateTrack(state.document, inspector.track.trackId, patch);
        Render();
    }

    function UpdateSelectedMarker(patch) {
        const inspector = GetSelectedInspector();
        if (!inspector.marker) {
            return;
        }
        state.document = model.UpdateEventMarker(state.document, inspector.marker.markerId, patch);
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

    function AppendSelect(container, label_text, value, options, on_input) {
        const group = CreateElement("label", "field", "");
        const label = CreateElement("span", "", label_text);
        const select = CreateElement("select", "", "");
        options.forEach(function AddOption(option) {
            const item = CreateElement("option", "", option);
            item.value = option;
            item.selected = option === value;
            select.appendChild(item);
        });
        select.addEventListener("change", function OnChangeEvent() {
            on_input(select.value);
        });
        group.appendChild(label);
        group.appendChild(select);
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
        SetText("clip-count", String(result.summary.clipCount));
        SetText("track-count", String(result.summary.trackCount));
        SetText("event-count", String(result.summary.eventCount));
        SetText("issue-count", String(result.summary.issueCount));
        SetText("document-hash", runtime_document.schema.documentHash);
    }

    function RenderClipList() {
        const panel = GetElement("clip-list");
        Clear(panel);
        model.BuildClipList(state.document).forEach(function RenderClip(record) {
            const row = CreateElement("button", "clip-row", "");
            row.type = "button";
            row.dataset.selected = record.selected ? "true" : "false";
            row.addEventListener("click", function OnClickEvent() {
                SelectClip(record.clipId);
            });
            row.appendChild(CreateElement("span", "clip-badge", "C"));
            row.appendChild(CreateElement("span", "clip-name", record.label));
            row.appendChild(CreateElement("span", "clip-meta", String(record.durationTicks) + " ticks"));
            panel.appendChild(row);
        });
    }

    function RenderTrackList() {
        const panel = GetElement("track-list");
        Clear(panel);
        const rows = model.BuildTrackRows(state.document);
        if (rows.length === 0) {
            panel.appendChild(CreateElement("p", "empty-text", "No tracks in selected clip"));
            return;
        }
        rows.forEach(function RenderTrack(record) {
            const row = CreateElement("button", "track-row", "");
            row.type = "button";
            row.dataset.selected = record.selected ? "true" : "false";
            row.addEventListener("click", function OnClickEvent() {
                SelectTrack(record.trackId);
            });
            const swatch = CreateElement("span", "track-color", "");
            swatch.style.background = record.color;
            row.appendChild(swatch);
            row.appendChild(CreateElement("span", "track-name", record.label));
            row.appendChild(CreateElement("span", "track-meta", record.channelKind));
            panel.appendChild(row);
        });
    }

    function StartMarkerDrag(event, marker) {
        event.preventDefault();
        event.stopPropagation();
        state.markerDrag = {
            markerId: marker.markerId,
            startX: event.clientX,
            startTick: marker.startTick,
            endTick: marker.endTick
        };
        SelectMarker(marker.markerId);
    }

    function UpdateMarkerDrag(event) {
        if (!state.markerDrag) {
            return;
        }
        const timeline = state.document.editor.timeline;
        const delta_tick = Math.round((event.clientX - state.markerDrag.startX) / timeline.zoom);
        const start_tick = Math.max(0, state.markerDrag.startTick + delta_tick);
        const width_tick = state.markerDrag.endTick - state.markerDrag.startTick;
        const end_tick = Math.max(start_tick, start_tick + width_tick);
        state.document = model.UpdateEventMarker(state.document, state.markerDrag.markerId, {
            startTick: start_tick,
            endTick: end_tick
        });
        Render();
    }

    function FinishMarkerDrag() {
        state.markerDrag = null;
    }

    function RenderTimeline() {
        const panel = GetElement("timeline-view");
        Clear(panel);
        const inspector = model.BuildInspector(state.document);
        const clip = inspector.clip;
        if (!clip) {
            panel.appendChild(CreateElement("p", "empty-text", "No clip selected"));
            return;
        }
        const timeline = state.document.editor.timeline;
        SetText("timeline-scale-label", String(timeline.zoom.toFixed(3)) + " px/tick");
        const ruler = CreateElement("div", "timeline-ruler", "");
        const end_label = CreateElement("span", "", String(clip.durationTicks) + " ticks");
        ruler.appendChild(CreateElement("span", "", "0"));
        ruler.appendChild(end_label);
        panel.appendChild(ruler);

        const tracks = model.BuildTrackRows(state.document);
        tracks.forEach(function RenderTrackRow(track) {
            const row = CreateElement("div", "timeline-track", "");
            row.dataset.selected = track.selected ? "true" : "false";
            const label = CreateElement("button", "timeline-track-label", track.label);
            label.type = "button";
            label.addEventListener("click", function OnClickEvent() {
                SelectTrack(track.trackId);
            });
            const lane = CreateElement("div", "timeline-track-lane", "");
            lane.style.setProperty("--track-color", track.color);
            row.appendChild(label);
            row.appendChild(lane);
            panel.appendChild(row);
        });

        const event_lane = CreateElement("div", "timeline-event-lane", "");
        model.BuildTimelineMarkers(state.document).forEach(function RenderMarker(marker) {
            const item = CreateElement("button", "event-marker", marker.label);
            item.type = "button";
            item.dataset.selected = marker.selected ? "true" : "false";
            item.dataset.status = marker.validationStatus;
            item.style.left = String(marker.left) + "px";
            item.style.width = String(marker.width) + "px";
            item.addEventListener("click", function OnClickEvent(event) {
                event.stopPropagation();
                SelectMarker(marker.markerId);
            });
            item.addEventListener("pointerdown", function OnPointerDownEvent(event) {
                StartMarkerDrag(event, marker);
            });
            event_lane.appendChild(item);
        });
        panel.appendChild(event_lane);
    }

    function RenderResourceList() {
        const panel = GetElement("resource-list");
        Clear(panel);
        const refs = state.document.resourceRefs || [];
        if (refs.length === 0) {
            panel.appendChild(CreateElement("p", "empty-text", "No resource refs"));
            return;
        }
        refs.forEach(function RenderResource(record) {
            const row = CreateElement("div", "resource-row", "");
            row.appendChild(CreateElement("span", "", record.kind));
            row.appendChild(CreateElement("span", "", record.resourceKey || "empty"));
            panel.appendChild(row);
        });
    }

    function RenderInspector() {
        const panel = GetElement("inspector-panel");
        Clear(panel);
        const inspector = model.BuildInspector(state.document);
        if (!inspector.clip) {
            panel.appendChild(CreateElement("p", "empty-text", "No clip selected"));
            return;
        }

        AppendInspectorGroup(panel, "Document Header", "document", function BuildDocumentGroup(body) {
            AppendInput(body, "Document ID", inspector.schema.animationDocumentId, function UpdateDocumentId(value) {
                state.document = model.UpdateSchema(state.document, { animationDocumentId: value });
                Render();
            });
            AppendNumberInput(body, "Ticks Per Second", inspector.schema.timeTicksPerSecond, function UpdateTicks(value) {
                state.document = model.UpdateSchema(state.document, { timeTicksPerSecond: value });
                Render();
            });
        });

        AppendInspectorGroup(panel, "Clip Metadata", "clip", function BuildClipGroup(body) {
            AppendInput(body, "Clip Label", inspector.clipLabel, function UpdateLabel(value) {
                state.document = model.UpdateLabel(state.document, "clipLabels", inspector.clip.clipId, value);
                Render();
            });
            AppendNumberInput(body, "Name Hash", inspector.clip.nameHash, function UpdateNameHash(value) {
                UpdateSelectedClip({ nameHash: value });
            });
            AppendNumberInput(body, "Duration Ticks", inspector.clip.durationTicks, function UpdateDuration(value) {
                UpdateSelectedClip({ durationTicks: value });
            });
            AppendNumberInput(body, "Sample Rate", inspector.clip.sampleRateHz, function UpdateSampleRate(value) {
                UpdateSelectedClip({ sampleRateHz: value });
            });
            AppendSelect(body, "Loop Mode", inspector.clip.loopMode, model.LOOP_MODES, function UpdateLoop(value) {
                UpdateSelectedClip({ loopMode: value });
            });
        });

        if (inspector.track) {
            AppendInspectorGroup(panel, "Track Descriptor", "track", function BuildTrackGroup(body) {
                AppendInput(body, "Track Label", inspector.trackLabel, function UpdateLabel(value) {
                    state.document = model.UpdateLabel(state.document, "trackLabels", inspector.track.trackId, value);
                    Render();
                });
                AppendNumberInput(body, "Target ID", inspector.track.targetId, function UpdateTarget(value) {
                    UpdateSelectedTrack({ targetId: value });
                });
                AppendSelect(body, "Channel", inspector.track.channelKind, model.CHANNEL_KINDS, function UpdateChannel(value) {
                    UpdateSelectedTrack({ channelKind: value });
                });
                AppendSelect(body, "Sample Format", inspector.track.sampleFormat, model.SAMPLE_FORMATS, function UpdateFormat(value) {
                    UpdateSelectedTrack({ sampleFormat: value });
                });
                AppendNumberInput(body, "Key Count", inspector.track.keyCount, function UpdateKeyCount(value) {
                    UpdateSelectedTrack({ keyCount: value });
                });
            });
        }

        if (inspector.marker) {
            AppendInspectorGroup(panel, "Event Marker", "marker", function BuildMarkerGroup(body) {
                AppendInput(body, "Marker Label", inspector.markerLabel, function UpdateLabel(value) {
                    state.document = model.UpdateLabel(state.document, "markerLabels", inspector.marker.markerId, value);
                    Render();
                });
                AppendNumberInput(body, "Event ID", inspector.marker.eventId, function UpdateEvent(value) {
                    UpdateSelectedMarker({ eventId: value });
                });
                AppendNumberInput(body, "Start Tick", inspector.marker.startTick, function UpdateStart(value) {
                    UpdateSelectedMarker({ startTick: value });
                });
                AppendNumberInput(body, "End Tick", inspector.marker.endTick, function UpdateEnd(value) {
                    UpdateSelectedMarker({ endTick: value });
                });
                AppendNumberInput(body, "Payload ID", inspector.marker.payloadId, function UpdatePayload(value) {
                    UpdateSelectedMarker({ payloadId: value });
                });
                AppendSelect(body, "Status", inspector.marker.validationStatus, model.EVENT_STATUSES, function UpdateStatus(value) {
                    UpdateSelectedMarker({ validationStatus: value });
                });
            });
        }

        AppendInspectorGroup(panel, "Editor Sidecar", "editor", function BuildEditorGroup(body) {
            AppendNumberInput(body, "Scrubber Tick", inspector.editor.timeline.scrubberTick, function UpdateScrubber(value) {
                const timeline = Object.assign({}, inspector.editor.timeline, { scrubberTick: value });
                state.document = model.UpdateEditor(state.document, { timeline: timeline });
                Render();
            });
            AppendNumberInput(body, "Timeline Zoom", inspector.editor.timeline.zoom, function UpdateZoom(value) {
                const timeline = Object.assign({}, inspector.editor.timeline, { zoom: value });
                state.document = model.UpdateEditor(state.document, { timeline: timeline });
                Render();
            });
            AppendCheckbox(body, "Dirty", inspector.editor.dirty, function UpdateDirty(value) {
                state.document = model.UpdateEditor(state.document, { dirty: value });
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
            row.appendChild(CreateElement("code", "", "clip " + String(issue.clipId)));
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
        RenderClipList();
        RenderTrackList();
        RenderTimeline();
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

    function UpdateTimelineZoom(zoom) {
        const inspector = model.BuildInspector(state.document);
        const timeline = Object.assign({}, inspector.editor.timeline, {
            zoom: ClampNumber(zoom, 0.02, 0.4)
        });
        state.document = model.UpdateEditor(state.document, { timeline: timeline });
        Render();
    }

    function FitTimeline() {
        UpdateTimelineZoom(0.08);
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
            state.layout.leftWidth = ClampNumber(state.layoutDrag.leftWidth + delta_x, 260, 560);
        }
        if (state.layoutDrag.side === "Right") {
            state.layout.rightWidth = ClampNumber(state.layoutDrag.rightWidth - delta_x, 320, 620);
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
            DownloadJson("YuAnimationEditorDraft.json", state.document);
            Render();
        });
        GetElement("export-runtime-button").addEventListener("click", function OnClickEvent() {
            DownloadJson("YuAnimationRuntimeData.json", model.BuildRuntimeDocument(state.document));
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
        GetElement("add-clip-button").addEventListener("click", function OnClickEvent() {
            state.document = model.AddClip(state.document);
            Render();
        });
        GetElement("add-track-button").addEventListener("click", function OnClickEvent() {
            state.document = model.AddTrack(state.document);
            Render();
        });
        GetElement("add-marker-button").addEventListener("click", function OnClickEvent() {
            state.document = model.AddEventMarker(state.document);
            Render();
        });
        GetElement("remove-selected-button").addEventListener("click", function OnClickEvent() {
            state.document = model.RemoveSelected(state.document);
            Render();
        });
        GetElement("fit-time-button").addEventListener("click", FitTimeline);
        GetElement("zoom-out-button").addEventListener("click", function OnClickEvent() {
            UpdateTimelineZoom(state.document.editor.timeline.zoom - 0.02);
        });
        GetElement("zoom-reset-button").addEventListener("click", function OnClickEvent() {
            UpdateTimelineZoom(0.08);
        });
        GetElement("zoom-in-button").addEventListener("click", function OnClickEvent() {
            UpdateTimelineZoom(state.document.editor.timeline.zoom + 0.02);
        });
        GetElement("left-resizer").addEventListener("pointerdown", function OnPointerDownEvent(event) {
            StartPaneResize(event, "Left");
        });
        GetElement("right-resizer").addEventListener("pointerdown", function OnPointerDownEvent(event) {
            StartPaneResize(event, "Right");
        });
        window.addEventListener("pointermove", function OnPointerMoveEvent(event) {
            UpdatePaneResize(event);
            UpdateMarkerDrag(event);
        });
        window.addEventListener("pointerup", function OnPointerUpEvent() {
            FinishPaneResize();
            FinishMarkerDrag();
        });
        window.addEventListener("pointercancel", function OnPointerCancelEvent() {
            FinishPaneResize();
            FinishMarkerDrag();
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
