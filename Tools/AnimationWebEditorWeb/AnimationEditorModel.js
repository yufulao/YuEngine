// 模块: Tools AnimationWebEditorWeb
// 文件: Tools/AnimationWebEditorWeb/AnimationEditorModel.js

(function RegisterAnimationWebEditorModel(root, factory) {
    const api = factory();
    if (typeof module !== "undefined" && module.exports) {
        module.exports = api;
    }
    if (root) {
        root.AnimationWebEditorModel = api;
    }
})(typeof globalThis !== "undefined" ? globalThis : this, function CreateAnimationWebEditorModel() {
    const SCHEMA_ID = "YUANIM";
    const SCHEMA_VERSION = 1;
    const LOOP_MODES = ["Once", "Loop", "PingPong"];
    const CHANNEL_KINDS = ["Translation3", "RotationQuat", "Scale3", "Scalar", "Discrete"];
    const SAMPLE_FORMATS = ["Float32", "Int16", "Uint16", "StepBool"];
    const EVENT_STATUSES = ["Success", "DuplicateEventMarker", "EventOverflow", "InvalidTimeRange", "MissingClip", "Unsupported"];
    const RESOURCE_KINDS = ["ClipSource", "Payload", "Custom"];
    const MAX_EVENT_MARKER_COUNT = 32;
    const DEFAULT_TIMELINE = {
        zoom: 0.08,
        scrollX: 0,
        scrollY: 0,
        scrubberTick: 0
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

    function NormalizeInteger(value, fallback) {
        const number_value = Math.trunc(ToNumber(value, fallback));
        if (Number.isFinite(number_value)) {
            return number_value;
        }
        return fallback;
    }

    function NormalizePositiveId(value, fallback) {
        const id = NormalizeInteger(value, fallback);
        if (id > 0) {
            return id;
        }
        return fallback;
    }

    function NormalizeNonNegativeInteger(value, fallback) {
        const number_value = NormalizeInteger(value, fallback);
        if (number_value >= 0) {
            return number_value;
        }
        return fallback;
    }

    function NormalizePositiveInteger(value, fallback) {
        const number_value = NormalizeInteger(value, fallback);
        if (number_value > 0) {
            return number_value;
        }
        return fallback;
    }

    function NormalizeOption(value, fallback, options) {
        const text = ToString(value, fallback);
        if (options.includes(text)) {
            return text;
        }
        return fallback;
    }

    function NormalizeClip(record, index) {
        return {
            clipId: NormalizePositiveId(record.clipId, index + 1),
            nameHash: NormalizePositiveId(record.nameHash, 1000 + index + 1),
            durationTicks: NormalizePositiveInteger(record.durationTicks, 60000),
            sampleRateHz: NormalizePositiveInteger(record.sampleRateHz, 60),
            trackCount: NormalizeNonNegativeInteger(record.trackCount, 0),
            eventCount: NormalizeNonNegativeInteger(record.eventCount, 0),
            loopMode: ToString(record.loopMode, "Once"),
            order: NormalizeNonNegativeInteger(record.order, index)
        };
    }

    function NormalizeTrack(record, index) {
        return {
            trackId: NormalizePositiveId(record.trackId, index + 1),
            clipId: NormalizePositiveId(record.clipId, 0),
            targetId: NormalizePositiveId(record.targetId, 0),
            channelKind: ToString(record.channelKind, "Scalar"),
            sampleFormat: ToString(record.sampleFormat, "Float32"),
            keyCount: NormalizeNonNegativeInteger(record.keyCount, 0),
            order: NormalizeNonNegativeInteger(record.order, index)
        };
    }

    function NormalizeEventMarker(record, index) {
        const start_tick = NormalizeNonNegativeInteger(record.startTick, 0);
        return {
            markerId: NormalizePositiveId(record.markerId, index + 1),
            clipId: NormalizePositiveId(record.clipId, 0),
            eventId: NormalizePositiveId(record.eventId, 0),
            startTick: start_tick,
            endTick: NormalizeNonNegativeInteger(record.endTick, start_tick),
            payloadId: NormalizeNonNegativeInteger(record.payloadId, 0),
            validationStatus: ToString(record.validationStatus, "Success"),
            order: NormalizeNonNegativeInteger(record.order, index)
        };
    }

    function NormalizeResourceRef(record, index) {
        return {
            resourceId: NormalizePositiveId(record.resourceId, index + 1),
            kind: ToString(record.kind, "Custom"),
            resourceKey: ToString(record.resourceKey, ""),
            label: ToString(record.label, "Resource " + String(index + 1))
        };
    }

    function NormalizeLabelMap(value) {
        const source = value || {};
        const result = {};
        Object.keys(source).forEach(function CopyLabel(key) {
            result[key] = ToString(source[key], "");
        });
        return result;
    }

    function NormalizeTrackColorMap(value) {
        const source = value || {};
        const result = {};
        Object.keys(source).forEach(function CopyColor(key) {
            result[key] = ToString(source[key], "#0f766e");
        });
        return result;
    }

    function NormalizeEditorSidecar(value, selected_clip_id) {
        const source = value || {};
        const timeline = source.timeline || {};
        return {
            selectedClipId: NormalizePositiveId(source.selectedClipId, selected_clip_id),
            selectedTrackId: NormalizePositiveId(source.selectedTrackId, 0),
            selectedMarkerId: NormalizePositiveId(source.selectedMarkerId, 0),
            dirty: ToBool(source.dirty, false),
            timeline: {
                zoom: Math.max(0.01, ToNumber(timeline.zoom, DEFAULT_TIMELINE.zoom)),
                scrollX: ToNumber(timeline.scrollX, DEFAULT_TIMELINE.scrollX),
                scrollY: ToNumber(timeline.scrollY, DEFAULT_TIMELINE.scrollY),
                scrubberTick: NormalizeNonNegativeInteger(timeline.scrubberTick, DEFAULT_TIMELINE.scrubberTick)
            },
            clipLabels: NormalizeLabelMap(source.clipLabels),
            trackLabels: NormalizeLabelMap(source.trackLabels),
            markerLabels: NormalizeLabelMap(source.markerLabels),
            trackColors: NormalizeTrackColorMap(source.trackColors),
            foldouts: source.foldouts || {},
            paneState: source.paneState || {}
        };
    }

    function CompareClipOrder(left, right) {
        if (left.order !== right.order) {
            return left.order - right.order;
        }
        return left.clipId - right.clipId;
    }

    function CompareTrackOrder(left, right) {
        if (left.clipId !== right.clipId) {
            return left.clipId - right.clipId;
        }
        if (left.order !== right.order) {
            return left.order - right.order;
        }
        return left.trackId - right.trackId;
    }

    function CompareEventOrder(left, right) {
        if (left.clipId !== right.clipId) {
            return left.clipId - right.clipId;
        }
        if (left.startTick !== right.startTick) {
            return left.startTick - right.startTick;
        }
        if (left.order !== right.order) {
            return left.order - right.order;
        }
        return left.markerId - right.markerId;
    }

    function CompareResourceOrder(left, right) {
        if (left.kind !== right.kind) {
            return left.kind < right.kind ? -1 : 1;
        }
        if (left.resourceKey !== right.resourceKey) {
            return left.resourceKey < right.resourceKey ? -1 : 1;
        }
        return left.resourceId - right.resourceId;
    }

    function NormalizeOrders(document) {
        document.clips.slice().sort(CompareClipOrder).forEach(function AssignClipOrder(record, index) {
            record.order = index;
        });
        document.tracks.slice().sort(CompareTrackOrder).forEach(function AssignTrackOrder(record, index) {
            record.order = index;
        });
        document.eventMarkers.slice().sort(CompareEventOrder).forEach(function AssignMarkerOrder(record, index) {
            record.order = index;
        });
    }

    function CreateDefaultDocument() {
        return {
            schema: {
                schemaId: SCHEMA_ID,
                schemaVersion: SCHEMA_VERSION,
                animationDocumentId: "AnimationDocument001",
                timeTicksPerSecond: 1000,
                documentHash: "",
                clipCount: 2,
                trackCount: 3,
                eventCount: 2
            },
            clips: [
                {
                    clipId: 1,
                    nameHash: 1001,
                    durationTicks: 1200,
                    sampleRateHz: 60,
                    trackCount: 2,
                    eventCount: 1,
                    loopMode: "Loop",
                    order: 0
                },
                {
                    clipId: 2,
                    nameHash: 1002,
                    durationTicks: 900,
                    sampleRateHz: 60,
                    trackCount: 1,
                    eventCount: 1,
                    loopMode: "Once",
                    order: 1
                }
            ],
            tracks: [
                {
                    trackId: 11,
                    clipId: 1,
                    targetId: 201,
                    channelKind: "Translation3",
                    sampleFormat: "Float32",
                    keyCount: 8,
                    order: 0
                },
                {
                    trackId: 12,
                    clipId: 1,
                    targetId: 201,
                    channelKind: "RotationQuat",
                    sampleFormat: "Float32",
                    keyCount: 8,
                    order: 1
                },
                {
                    trackId: 21,
                    clipId: 2,
                    targetId: 301,
                    channelKind: "Scalar",
                    sampleFormat: "Float32",
                    keyCount: 5,
                    order: 2
                }
            ],
            eventMarkers: [
                {
                    markerId: 10001,
                    clipId: 1,
                    eventId: 3001,
                    startTick: 300,
                    endTick: 300,
                    payloadId: 7001,
                    validationStatus: "Success",
                    order: 0
                },
                {
                    markerId: 10002,
                    clipId: 2,
                    eventId: 3002,
                    startTick: 600,
                    endTick: 620,
                    payloadId: 7002,
                    validationStatus: "Success",
                    order: 1
                }
            ],
            resourceRefs: [
                {
                    resourceId: 1,
                    kind: "ClipSource",
                    resourceKey: "pkg://animation/sample-clip",
                    label: "Sample Clip Source"
                },
                {
                    resourceId: 2,
                    kind: "Payload",
                    resourceKey: "pkg://animation/sample-event-payload",
                    label: "Sample Event Payload"
                }
            ],
            editor: {
                selectedClipId: 1,
                selectedTrackId: 11,
                selectedMarkerId: 10001,
                dirty: false,
                timeline: Clone(DEFAULT_TIMELINE),
                clipLabels: {
                    "1": "Idle Loop",
                    "2": "Confirm Pulse"
                },
                trackLabels: {
                    "11": "Root Offset",
                    "12": "Root Rotation",
                    "21": "Button Glow"
                },
                markerLabels: {
                    "10001": "Footstep",
                    "10002": "Confirm Event"
                },
                trackColors: {
                    "11": "#0f766e",
                    "12": "#2563eb",
                    "21": "#9333ea"
                },
                foldouts: {
                    document: true,
                    clip: true,
                    track: true,
                    marker: true,
                    resources: true,
                    editor: false
                },
                paneState: {}
            }
        };
    }

    function HasOwnValue(source, key) {
        return Object.prototype.hasOwnProperty.call(source, key);
    }

    function NormalizeSchema(schema, document) {
        const source = schema || {};
        const clip_count = HasOwnValue(source, "clipCount") ? NormalizeNonNegativeInteger(source.clipCount, 0) : document.clips.length;
        const track_count = HasOwnValue(source, "trackCount") ? NormalizeNonNegativeInteger(source.trackCount, 0) : document.tracks.length;
        const event_count = HasOwnValue(source, "eventCount") ? NormalizeNonNegativeInteger(source.eventCount, 0) : document.eventMarkers.length;
        return {
            schemaId: ToString(source.schemaId, SCHEMA_ID),
            schemaVersion: NormalizeInteger(source.schemaVersion, SCHEMA_VERSION),
            animationDocumentId: ToString(source.animationDocumentId, "AnimationDocument001"),
            timeTicksPerSecond: NormalizePositiveInteger(source.timeTicksPerSecond, 1000),
            documentHash: ToString(source.documentHash, ""),
            clipCount: clip_count,
            trackCount: track_count,
            eventCount: event_count
        };
    }

    function NormalizeDocument(source) {
        const base = source && typeof source === "object" ? source : CreateDefaultDocument();
        const document = Clone(base);
        document.clips = GetArray(document.clips).map(NormalizeClip);
        document.tracks = GetArray(document.tracks).map(NormalizeTrack);
        document.eventMarkers = GetArray(document.eventMarkers).map(NormalizeEventMarker);
        document.resourceRefs = GetArray(document.resourceRefs).map(NormalizeResourceRef);
        NormalizeOrders(document);
        document.schema = NormalizeSchema(document.schema, document);
        const first_clip = document.clips[0] || { clipId: 0 };
        document.editor = NormalizeEditorSidecar(document.editor, first_clip.clipId);
        return document;
    }

    function FindClip(document, clip_id) {
        return document.clips.find(function MatchClip(record) {
            return record.clipId === clip_id;
        }) || null;
    }

    function FindTrack(document, track_id) {
        return document.tracks.find(function MatchTrack(record) {
            return record.trackId === track_id;
        }) || null;
    }

    function FindMarker(document, marker_id) {
        return document.eventMarkers.find(function MatchMarker(record) {
            return record.markerId === marker_id;
        }) || null;
    }

    function CountTracksForClip(document, clip_id) {
        return document.tracks.filter(function MatchTrack(record) {
            return record.clipId === clip_id;
        }).length;
    }

    function CountEventsForClip(document, clip_id) {
        return document.eventMarkers.filter(function MatchMarker(record) {
            return record.clipId === clip_id;
        }).length;
    }

    function PushIssue(issues, kind, message, clip_id, record_id) {
        issues.push({
            kind: kind,
            message: message,
            clipId: clip_id || 0,
            recordId: record_id || 0
        });
    }

    function CountByKey(records, key_selector) {
        const counts = new Map();
        records.forEach(function CountRecord(record) {
            const key = key_selector(record);
            counts.set(key, (counts.get(key) || 0) + 1);
        });
        return counts;
    }

    function PushDuplicateIssues(issues, counts, kind, message) {
        counts.forEach(function CheckCount(count, key) {
            if (count > 1) {
                PushIssue(issues, kind, message, 0, Number(key) || 0);
            }
        });
    }

    function ComputeExpectedEventStatus(document, marker, duplicate_tuples) {
        const clip = FindClip(document, marker.clipId);
        if (!clip) {
            return "MissingClip";
        }
        if (marker.startTick > marker.endTick || marker.endTick > clip.durationTicks) {
            return "InvalidTimeRange";
        }
        const tuple_key = GetEventTupleKey(marker);
        if ((duplicate_tuples.get(tuple_key) || 0) > 1) {
            return "DuplicateEventMarker";
        }
        if (document.eventMarkers.length > MAX_EVENT_MARKER_COUNT) {
            return "EventOverflow";
        }
        return "Success";
    }

    function GetEventTupleKey(marker) {
        return [
            marker.clipId,
            marker.eventId,
            marker.startTick,
            marker.endTick,
            marker.payloadId
        ].join("|");
    }

    function ValidateDocument(source) {
        const document = NormalizeDocument(source);
        const issues = [];
        if (document.schema.schemaId !== SCHEMA_ID) {
            PushIssue(issues, "InvalidHeader", "Schema id must be YUANIM", 0, 0);
        }
        if (document.schema.schemaVersion !== SCHEMA_VERSION) {
            PushIssue(issues, "InvalidHeader", "Schema version must be 1", 0, 0);
        }
        if (!document.schema.animationDocumentId) {
            PushIssue(issues, "InvalidHeader", "Animation document id is required", 0, 0);
        }
        if (document.schema.timeTicksPerSecond <= 0) {
            PushIssue(issues, "InvalidHeader", "Time tick rate must be positive", 0, 0);
        }
        if (document.schema.clipCount !== document.clips.length) {
            PushIssue(issues, "HeaderCountMismatch", "Clip count does not match clip records", 0, 0);
        }
        if (document.schema.trackCount !== document.tracks.length) {
            PushIssue(issues, "HeaderCountMismatch", "Track count does not match track records", 0, 0);
        }
        if (document.schema.eventCount !== document.eventMarkers.length) {
            PushIssue(issues, "HeaderCountMismatch", "Event count does not match marker records", 0, 0);
        }
        if (document.clips.length === 0) {
            PushIssue(issues, "MissingClipRecord", "Animation document requires at least one clip", 0, 0);
        }

        PushDuplicateIssues(issues, CountByKey(document.clips, function SelectClipId(record) {
            return record.clipId;
        }), "DuplicateClipId", "Clip id is duplicated");
        PushDuplicateIssues(issues, CountByKey(document.tracks, function SelectTrackId(record) {
            return record.trackId;
        }), "DuplicateTrackId", "Track id is duplicated");
        PushDuplicateIssues(issues, CountByKey(document.eventMarkers, function SelectMarkerId(record) {
            return record.markerId;
        }), "DuplicateMarkerId", "Marker id is duplicated");

        document.clips.forEach(function CheckClip(record) {
            if (record.clipId <= 0) {
                PushIssue(issues, "InvalidClipId", "Clip id must be positive", record.clipId, record.clipId);
            }
            if (record.nameHash <= 0) {
                PushIssue(issues, "InvalidNameHash", "Clip name hash must be positive", record.clipId, record.nameHash);
            }
            if (record.durationTicks <= 0) {
                PushIssue(issues, "InvalidClipDuration", "Clip duration must be positive", record.clipId, record.clipId);
            }
            if (record.sampleRateHz <= 0) {
                PushIssue(issues, "InvalidSampleRate", "Sample rate must be positive", record.clipId, record.clipId);
            }
            if (record.trackCount !== CountTracksForClip(document, record.clipId)) {
                PushIssue(issues, "ClipTrackCountMismatch", "Clip track count does not match descriptors", record.clipId, record.clipId);
            }
            if (record.eventCount !== CountEventsForClip(document, record.clipId)) {
                PushIssue(issues, "ClipEventCountMismatch", "Clip event count does not match markers", record.clipId, record.clipId);
            }
            if (!LOOP_MODES.includes(record.loopMode)) {
                PushIssue(issues, "UnsupportedLoopMode", "Clip loop mode is not supported", record.clipId, record.clipId);
            }
        });

        document.tracks.forEach(function CheckTrack(record) {
            if (!FindClip(document, record.clipId)) {
                PushIssue(issues, "TrackWithoutClip", "Track references a missing clip", record.clipId, record.trackId);
            }
            if (record.targetId <= 0) {
                PushIssue(issues, "InvalidTargetId", "Track target id must be positive", record.clipId, record.trackId);
            }
            if (!CHANNEL_KINDS.includes(record.channelKind)) {
                PushIssue(issues, "UnsupportedChannelKind", "Track channel kind is not supported", record.clipId, record.trackId);
            }
            if (!SAMPLE_FORMATS.includes(record.sampleFormat)) {
                PushIssue(issues, "UnsupportedSampleFormat", "Track sample format is not supported", record.clipId, record.trackId);
            }
        });

        const duplicate_event_tuples = CountByKey(document.eventMarkers, GetEventTupleKey);
        document.eventMarkers.forEach(function CheckEventMarker(record) {
            const expected_status = ComputeExpectedEventStatus(document, record, duplicate_event_tuples);
            if (record.eventId <= 0) {
                PushIssue(issues, "InvalidEventId", "Event id must be positive", record.clipId, record.markerId);
            }
            if (!EVENT_STATUSES.includes(record.validationStatus)) {
                PushIssue(issues, "UnsupportedEventStatus", "Event validation status is not supported", record.clipId, record.markerId);
            }
            if (record.validationStatus !== expected_status) {
                PushIssue(issues, "EventStatusMismatch", "Event validation status does not match marker data", record.clipId, record.markerId);
            }
        });

        const resource_ids = CountByKey(document.resourceRefs, function SelectResourceId(record) {
            return record.resourceId;
        });
        PushDuplicateIssues(issues, resource_ids, "DuplicateResourceId", "Resource id is duplicated");
        document.resourceRefs.forEach(function CheckResource(record) {
            if (!RESOURCE_KINDS.includes(record.kind)) {
                PushIssue(issues, "InvalidResourceKind", "Resource kind is not supported", 0, record.resourceId);
            }
            if (!record.resourceKey) {
                PushIssue(issues, "InvalidResourceKey", "Resource key is required", 0, record.resourceId);
            }
        });

        const status = issues.length === 0 ? "Success" : "IssuesFound";
        return {
            status: status,
            issues: issues,
            summary: {
                clipCount: document.clips.length,
                trackCount: document.tracks.length,
                eventCount: document.eventMarkers.length,
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
                animationDocumentId: document.schema.animationDocumentId,
                timeTicksPerSecond: document.schema.timeTicksPerSecond,
                documentHash: "",
                clipCount: document.clips.length,
                trackCount: document.tracks.length,
                eventCount: document.eventMarkers.length
            },
            clips: document.clips.slice().sort(CompareClipOrder).map(function CloneClip(record) {
                const clip = Clone(record);
                delete clip.order;
                return clip;
            }),
            tracks: document.tracks.slice().sort(CompareTrackOrder).map(function CloneTrack(record) {
                const track = Clone(record);
                delete track.order;
                return track;
            }),
            eventMarkers: document.eventMarkers.slice().sort(CompareEventOrder).map(function CloneMarker(record) {
                const marker = Clone(record);
                delete marker.order;
                return marker;
            }),
            resourceRefs: document.resourceRefs.slice().sort(CompareResourceOrder).map(function CloneResource(record) {
                const resource = Clone(record);
                delete resource.label;
                return resource;
            })
        };
        runtime_document.schema.documentHash = ComputeDocumentHash(runtime_document);
        return Clone(runtime_document);
    }

    function GetClipLabel(document, clip_id) {
        const key = String(clip_id);
        return document.editor.clipLabels[key] || "Clip " + key;
    }

    function GetTrackLabel(document, track_id) {
        const key = String(track_id);
        return document.editor.trackLabels[key] || "Track " + key;
    }

    function GetMarkerLabel(document, marker_id) {
        const key = String(marker_id);
        return document.editor.markerLabels[key] || "Marker " + key;
    }

    function BuildClipList(source) {
        const document = NormalizeDocument(source);
        return document.clips.slice().sort(CompareClipOrder).map(function MapClip(record) {
            return {
                clipId: record.clipId,
                label: GetClipLabel(document, record.clipId),
                durationTicks: record.durationTicks,
                sampleRateHz: record.sampleRateHz,
                trackCount: CountTracksForClip(document, record.clipId),
                eventCount: CountEventsForClip(document, record.clipId),
                selected: record.clipId === document.editor.selectedClipId
            };
        });
    }

    function BuildTrackRows(source) {
        const document = NormalizeDocument(source);
        const selected_clip = document.editor.selectedClipId;
        return document.tracks.slice().sort(CompareTrackOrder).filter(function MatchClip(record) {
            return record.clipId === selected_clip;
        }).map(function MapTrack(record) {
            return {
                trackId: record.trackId,
                label: GetTrackLabel(document, record.trackId),
                channelKind: record.channelKind,
                sampleFormat: record.sampleFormat,
                keyCount: record.keyCount,
                color: document.editor.trackColors[String(record.trackId)] || "#0f766e",
                selected: record.trackId === document.editor.selectedTrackId
            };
        });
    }

    function BuildTimelineMarkers(source) {
        const document = NormalizeDocument(source);
        const selected_clip = FindClip(document, document.editor.selectedClipId);
        if (!selected_clip) {
            return [];
        }
        const zoom = document.editor.timeline.zoom;
        return document.eventMarkers.slice().sort(CompareEventOrder).filter(function MatchClip(record) {
            return record.clipId === selected_clip.clipId;
        }).map(function MapMarker(record) {
            return {
                markerId: record.markerId,
                label: GetMarkerLabel(document, record.markerId),
                left: record.startTick * zoom - document.editor.timeline.scrollX,
                width: Math.max(8, (record.endTick - record.startTick) * zoom),
                startTick: record.startTick,
                endTick: record.endTick,
                validationStatus: record.validationStatus,
                selected: record.markerId === document.editor.selectedMarkerId
            };
        });
    }

    function BuildInspector(source) {
        const document = NormalizeDocument(source);
        const clip = FindClip(document, document.editor.selectedClipId) || document.clips[0] || null;
        const track = FindTrack(document, document.editor.selectedTrackId) || document.tracks.find(function MatchClip(record) {
            return clip && record.clipId === clip.clipId;
        }) || null;
        const marker = FindMarker(document, document.editor.selectedMarkerId) || document.eventMarkers.find(function MatchClip(record) {
            return clip && record.clipId === clip.clipId;
        }) || null;
        return {
            schema: Clone(document.schema),
            clip: clip ? Clone(clip) : null,
            clipLabel: clip ? GetClipLabel(document, clip.clipId) : "",
            track: track ? Clone(track) : null,
            trackLabel: track ? GetTrackLabel(document, track.trackId) : "",
            marker: marker ? Clone(marker) : null,
            markerLabel: marker ? GetMarkerLabel(document, marker.markerId) : "",
            resourceRefs: document.resourceRefs.map(function CloneResource(record) {
                return Clone(record);
            }),
            editor: Clone(document.editor)
        };
    }

    function UpdateRuntimeCounts(document) {
        document.schema.clipCount = document.clips.length;
        document.schema.trackCount = document.tracks.length;
        document.schema.eventCount = document.eventMarkers.length;
        document.clips.forEach(function UpdateClipCounts(clip) {
            clip.trackCount = CountTracksForClip(document, clip.clipId);
            clip.eventCount = CountEventsForClip(document, clip.clipId);
        });
    }

    function UpdateSchema(source, patch) {
        const document = NormalizeDocument(source);
        document.schema = Object.assign({}, document.schema, patch || {});
        UpdateRuntimeCounts(document);
        document.editor.dirty = true;
        return document;
    }

    function UpdateClip(source, clip_id, patch) {
        const document = NormalizeDocument(source);
        document.clips = document.clips.map(function MapClip(record) {
            if (record.clipId !== clip_id) {
                return record;
            }
            return NormalizeClip(Object.assign({}, record, patch || {}), record.order);
        });
        UpdateRuntimeCounts(document);
        document.editor.selectedClipId = clip_id;
        document.editor.dirty = true;
        return document;
    }

    function UpdateTrack(source, track_id, patch) {
        const document = NormalizeDocument(source);
        document.tracks = document.tracks.map(function MapTrack(record) {
            if (record.trackId !== track_id) {
                return record;
            }
            return NormalizeTrack(Object.assign({}, record, patch || {}), record.order);
        });
        UpdateRuntimeCounts(document);
        document.editor.selectedTrackId = track_id;
        document.editor.dirty = true;
        return document;
    }

    function UpdateEventMarker(source, marker_id, patch) {
        const document = NormalizeDocument(source);
        document.eventMarkers = document.eventMarkers.map(function MapMarker(record) {
            if (record.markerId !== marker_id) {
                return record;
            }
            return NormalizeEventMarker(Object.assign({}, record, patch || {}), record.order);
        });
        UpdateRuntimeCounts(document);
        document.editor.selectedMarkerId = marker_id;
        document.editor.dirty = true;
        return document;
    }

    function UpdateEditor(source, patch) {
        const document = NormalizeDocument(source);
        document.editor = NormalizeEditorSidecar(Object.assign({}, document.editor, patch || {}), document.editor.selectedClipId);
        document.editor.dirty = true;
        return document;
    }

    function UpdateLabel(source, group, record_id, value) {
        const document = NormalizeDocument(source);
        const key = String(record_id);
        document.editor[group][key] = ToString(value, "");
        document.editor.dirty = true;
        return document;
    }

    function SelectClip(source, clip_id) {
        const document = NormalizeDocument(source);
        const clip = FindClip(document, clip_id);
        if (!clip) {
            return document;
        }
        document.editor.selectedClipId = clip.clipId;
        const track = document.tracks.find(function MatchTrack(record) {
            return record.clipId === clip.clipId;
        });
        const marker = document.eventMarkers.find(function MatchMarker(record) {
            return record.clipId === clip.clipId;
        });
        document.editor.selectedTrackId = track ? track.trackId : 0;
        document.editor.selectedMarkerId = marker ? marker.markerId : 0;
        return document;
    }

    function SelectTrack(source, track_id) {
        const document = NormalizeDocument(source);
        const track = FindTrack(document, track_id);
        if (!track) {
            return document;
        }
        document.editor.selectedTrackId = track.trackId;
        document.editor.selectedClipId = track.clipId;
        return document;
    }

    function SelectMarker(source, marker_id) {
        const document = NormalizeDocument(source);
        const marker = FindMarker(document, marker_id);
        if (!marker) {
            return document;
        }
        document.editor.selectedMarkerId = marker.markerId;
        document.editor.selectedClipId = marker.clipId;
        return document;
    }

    function GetNextId(records, field_name, fallback) {
        let next_id = fallback;
        records.forEach(function VisitRecord(record) {
            if (record[field_name] >= next_id) {
                next_id = record[field_name] + 1;
            }
        });
        return next_id;
    }

    function AddClip(source) {
        const document = NormalizeDocument(source);
        const clip_id = GetNextId(document.clips, "clipId", 1);
        document.clips.push({
            clipId: clip_id,
            nameHash: 1000 + clip_id,
            durationTicks: 1000,
            sampleRateHz: 60,
            trackCount: 0,
            eventCount: 0,
            loopMode: "Once",
            order: document.clips.length
        });
        document.editor.clipLabels[String(clip_id)] = "Clip " + String(clip_id);
        document.editor.selectedClipId = clip_id;
        document.editor.selectedTrackId = 0;
        document.editor.selectedMarkerId = 0;
        document.editor.dirty = true;
        UpdateRuntimeCounts(document);
        return document;
    }

    function AddTrack(source) {
        const document = NormalizeDocument(source);
        const clip = FindClip(document, document.editor.selectedClipId) || document.clips[0];
        if (!clip) {
            return document;
        }
        const track_id = GetNextId(document.tracks, "trackId", 1);
        document.tracks.push({
            trackId: track_id,
            clipId: clip.clipId,
            targetId: 100 + track_id,
            channelKind: "Scalar",
            sampleFormat: "Float32",
            keyCount: 0,
            order: document.tracks.length
        });
        document.editor.trackLabels[String(track_id)] = "Track " + String(track_id);
        document.editor.trackColors[String(track_id)] = "#0f766e";
        document.editor.selectedTrackId = track_id;
        document.editor.dirty = true;
        UpdateRuntimeCounts(document);
        return document;
    }

    function AddEventMarker(source) {
        const document = NormalizeDocument(source);
        const clip = FindClip(document, document.editor.selectedClipId) || document.clips[0];
        if (!clip) {
            return document;
        }
        const marker_id = GetNextId(document.eventMarkers, "markerId", 1);
        const start_tick = Math.min(clip.durationTicks, document.editor.timeline.scrubberTick);
        document.eventMarkers.push({
            markerId: marker_id,
            clipId: clip.clipId,
            eventId: 3000 + marker_id,
            startTick: start_tick,
            endTick: start_tick,
            payloadId: 0,
            validationStatus: "Success",
            order: document.eventMarkers.length
        });
        document.editor.markerLabels[String(marker_id)] = "Marker " + String(marker_id);
        document.editor.selectedMarkerId = marker_id;
        document.editor.dirty = true;
        UpdateRuntimeCounts(document);
        return document;
    }

    function RemoveSelected(source) {
        const document = NormalizeDocument(source);
        if (document.editor.selectedMarkerId > 0) {
            document.eventMarkers = document.eventMarkers.filter(function KeepMarker(record) {
                return record.markerId !== document.editor.selectedMarkerId;
            });
            document.editor.selectedMarkerId = 0;
            document.editor.dirty = true;
            UpdateRuntimeCounts(document);
            return document;
        }
        if (document.editor.selectedTrackId > 0) {
            document.tracks = document.tracks.filter(function KeepTrack(record) {
                return record.trackId !== document.editor.selectedTrackId;
            });
            document.editor.selectedTrackId = 0;
            document.editor.dirty = true;
            UpdateRuntimeCounts(document);
            return document;
        }
        if (document.clips.length <= 1) {
            return document;
        }
        const remove_clip_id = document.editor.selectedClipId;
        document.clips = document.clips.filter(function KeepClip(record) {
            return record.clipId !== remove_clip_id;
        });
        document.tracks = document.tracks.filter(function KeepTrack(record) {
            return record.clipId !== remove_clip_id;
        });
        document.eventMarkers = document.eventMarkers.filter(function KeepMarker(record) {
            return record.clipId !== remove_clip_id;
        });
        const first_clip = document.clips[0] || { clipId: 0 };
        document.editor.selectedClipId = first_clip.clipId;
        document.editor.selectedTrackId = 0;
        document.editor.selectedMarkerId = 0;
        document.editor.dirty = true;
        UpdateRuntimeCounts(document);
        return document;
    }

    function FormatJson(value) {
        return JSON.stringify(value, null, 4);
    }

    return {
        SCHEMA_ID: SCHEMA_ID,
        SCHEMA_VERSION: SCHEMA_VERSION,
        LOOP_MODES: LOOP_MODES.slice(),
        CHANNEL_KINDS: CHANNEL_KINDS.slice(),
        SAMPLE_FORMATS: SAMPLE_FORMATS.slice(),
        EVENT_STATUSES: EVENT_STATUSES.slice(),
        RESOURCE_KINDS: RESOURCE_KINDS.slice(),
        MAX_EVENT_MARKER_COUNT: MAX_EVENT_MARKER_COUNT,
        CreateDefaultDocument: CreateDefaultDocument,
        NormalizeDocument: NormalizeDocument,
        ValidateDocument: ValidateDocument,
        BuildRuntimeDocument: BuildRuntimeDocument,
        BuildClipList: BuildClipList,
        BuildTrackRows: BuildTrackRows,
        BuildTimelineMarkers: BuildTimelineMarkers,
        BuildInspector: BuildInspector,
        UpdateSchema: UpdateSchema,
        UpdateClip: UpdateClip,
        UpdateTrack: UpdateTrack,
        UpdateEventMarker: UpdateEventMarker,
        UpdateEditor: UpdateEditor,
        UpdateLabel: UpdateLabel,
        SelectClip: SelectClip,
        SelectTrack: SelectTrack,
        SelectMarker: SelectMarker,
        AddClip: AddClip,
        AddTrack: AddTrack,
        AddEventMarker: AddEventMarker,
        RemoveSelected: RemoveSelected,
        FormatJson: FormatJson
    };
});
