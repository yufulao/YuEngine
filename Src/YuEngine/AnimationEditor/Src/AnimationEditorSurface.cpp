// Module: YuEngine AnimationEditor
// File: Src/YuEngine/AnimationEditor/Src/AnimationEditorSurface.cpp

#include "YuEngine/AnimationEditor/AnimationEditorSurface.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace yuengine::animationeditor {
namespace {
using AnimationRuntimeChannel = yuengine::animation::AnimationRuntimeChannel;
using AnimationRuntimeClipRecord = yuengine::animation::AnimationRuntimeClipRecord;
using AnimationRuntimeInterpolation = yuengine::animation::AnimationRuntimeInterpolation;
using AnimationRuntimeKeyframeRecord = yuengine::animation::AnimationRuntimeKeyframeRecord;
using AnimationRuntimeSampledValue = yuengine::animation::AnimationRuntimeSampledValue;
using AnimationRuntimeSampleRequest = yuengine::animation::AnimationRuntimeSampleRequest;
using AnimationRuntimeSampleResult = yuengine::animation::AnimationRuntimeSampleResult;
using AnimationRuntimeSampler = yuengine::animation::AnimationRuntimeSampler;
using AnimationRuntimeStatus = yuengine::animation::AnimationRuntimeStatus;
using AnimationRuntimeTrackRecord = yuengine::animation::AnimationRuntimeTrackRecord;
using PreviewHostTransformFeedback = yuengine::previewhost::PreviewHostTransformFeedback;
using WorldObjectId = yuengine::world::WorldObjectId;

constexpr std::uint64_t NANOSECONDS_PER_SECOND = 1000000000ULL;

template <typename T>
bool IsSpanStorageValid(std::span<T> values) {
    if (values.empty()) {
        return true;
    }

    return values.data() != nullptr;
}

template <typename T>
bool IsConstSpanStorageValid(std::span<const T> values) {
    if (values.empty()) {
        return true;
    }

    return values.data() != nullptr;
}

AnimationEditorSurfaceStatus MapAnimationStatus(AnimationRuntimeStatus status) {
    switch (status) {
        case AnimationRuntimeStatus::Success:
            return AnimationEditorSurfaceStatus::Success;
        case AnimationRuntimeStatus::NullPointer:
            return AnimationEditorSurfaceStatus::InvalidArgument;
        case AnimationRuntimeStatus::MissingClip:
            return AnimationEditorSurfaceStatus::MissingClip;
        case AnimationRuntimeStatus::InvalidClip:
        case AnimationRuntimeStatus::LayerCapacityExceeded:
            return AnimationEditorSurfaceStatus::InvalidClip;
        case AnimationRuntimeStatus::MissingTrack:
            return AnimationEditorSurfaceStatus::MissingTrack;
        case AnimationRuntimeStatus::InvalidTrack:
            return AnimationEditorSurfaceStatus::InvalidTrack;
        case AnimationRuntimeStatus::MissingKeyframe:
            return AnimationEditorSurfaceStatus::MissingKeyframe;
        case AnimationRuntimeStatus::InvalidKeyframe:
            return AnimationEditorSurfaceStatus::InvalidKeyframe;
        case AnimationRuntimeStatus::InvalidTime:
        case AnimationRuntimeStatus::TimeOutOfRange:
            return AnimationEditorSurfaceStatus::InvalidTime;
        case AnimationRuntimeStatus::UnsupportedInterpolation:
            return AnimationEditorSurfaceStatus::UnsupportedInterpolation;
        case AnimationRuntimeStatus::UnsupportedChannel:
            return AnimationEditorSurfaceStatus::UnsupportedChannel;
        case AnimationRuntimeStatus::OutputCapacityExceeded:
            return AnimationEditorSurfaceStatus::OutputCapacityExceeded;
        case AnimationRuntimeStatus::MissingSample:
        case AnimationRuntimeStatus::InvalidTarget:
        case AnimationRuntimeStatus::TargetNotFound:
        case AnimationRuntimeStatus::TransformApplyFailed:
            break;
    }

    return AnimationEditorSurfaceStatus::SampleFailed;
}

AnimationEditorSurfaceBlockedLayer BlockedLayerForStatus(
    AnimationEditorSurfaceStatus status) {
    if (status == AnimationEditorSurfaceStatus::Success) {
        return AnimationEditorSurfaceBlockedLayer::None;
    }

    if (status == AnimationEditorSurfaceStatus::OutputCapacityExceeded) {
        return AnimationEditorSurfaceBlockedLayer::TimelineOutput;
    }

    if (status == AnimationEditorSurfaceStatus::SampleFailed) {
        return AnimationEditorSurfaceBlockedLayer::RuntimeSampler;
    }

    if (status == AnimationEditorSurfaceStatus::PreviewFeedbackMissing) {
        return AnimationEditorSurfaceBlockedLayer::PreviewHostFeedback;
    }

    return AnimationEditorSurfaceBlockedLayer::RuntimeAnimationRecords;
}

const AnimationRuntimeClipRecord *FindClip(
    std::span<const AnimationRuntimeClipRecord> clips,
    std::uint32_t clip_id) {
    for (const AnimationRuntimeClipRecord &clip : clips) {
        if (clip.clip_id != clip_id) {
            continue;
        }

        return &clip;
    }

    return nullptr;
}

const PreviewHostTransformFeedback *FindTransformFeedback(
    std::span<const PreviewHostTransformFeedback> feedback,
    WorldObjectId target) {
    for (const PreviewHostTransformFeedback &record : feedback) {
        if (!record.transform_available) {
            continue;
        }

        if (record.world_object_id.value != target.value) {
            continue;
        }

        return &record;
    }

    return nullptr;
}

bool OutputCapacityAvailable(
    const AnimationEditorTimelineSurfaceRequest &request,
    const AnimationRuntimeClipRecord &clip,
    std::size_t keyframe_count,
    bool preview_feedback_requested) {
    if (request.clip_rows.size() < 1U) {
        return false;
    }

    if (request.track_rows.size() < clip.track_count) {
        return false;
    }

    if (request.keyframe_markers.size() < keyframe_count) {
        return false;
    }

    if (preview_feedback_requested &&
        request.preview_feedback_output.size() < clip.track_count) {
        return false;
    }

    return true;
}

AnimationEditorTimelineClipRow BuildClipRow(
    const AnimationRuntimeClipRecord &clip) {
    AnimationEditorTimelineClipRow row{};
    row.clip_id = clip.clip_id;
    row.duration_seconds = clip.duration_seconds;
    row.first_track_index = clip.first_track_index;
    row.track_count = clip.track_count;
    row.layer_count = clip.layer_count;
    row.selected = true;
    row.runtime_valid = clip.is_valid;
    return row;
}

AnimationEditorTimelineTrackRow BuildTrackRow(
    const AnimationRuntimeClipRecord &clip,
    const AnimationRuntimeTrackRecord &track) {
    AnimationEditorTimelineTrackRow row{};
    row.clip_id = clip.clip_id;
    row.track_id = track.track_id;
    row.target = track.target;
    row.channel = track.channel;
    row.interpolation = track.interpolation;
    row.first_keyframe_index = track.first_keyframe_index;
    row.keyframe_count = track.keyframe_count;
    row.active = track.is_valid;
    return row;
}

AnimationEditorTimelineKeyframeMarker BuildKeyframeMarker(
    const AnimationRuntimeClipRecord &clip,
    const AnimationRuntimeTrackRecord &track,
    const AnimationRuntimeKeyframeRecord &keyframe,
    std::size_t keyframe_index) {
    AnimationEditorTimelineKeyframeMarker marker{};
    marker.clip_id = clip.clip_id;
    marker.track_id = track.track_id;
    marker.target = track.target;
    marker.channel = track.channel;
    marker.keyframe_index = keyframe_index;
    marker.time_seconds = keyframe.time_seconds;
    marker.value = keyframe.value;
    marker.valid = keyframe.is_valid;
    return marker;
}

AnimationEditorPreviewFeedbackRecord BuildPreviewFeedbackRecord(
    const AnimationRuntimeClipRecord &clip,
    const AnimationRuntimeTrackRecord &track,
    const AnimationRuntimeSampledValue &sample,
    const PreviewHostTransformFeedback &feedback,
    float sample_time_seconds) {
    AnimationEditorPreviewFeedbackRecord record{};
    record.clip_id = clip.clip_id;
    record.track_id = track.track_id;
    record.target = sample.target;
    record.channel = sample.channel;
    record.sample_time_seconds = sample_time_seconds;
    record.sampled_value = sample.value;
    record.preview_transform = feedback.transform;
    record.matched_preview_host_feedback = true;
    record.feedback_from_preview_host = true;
    return record;
}

void CopyTimelineRows(
    const std::array<AnimationEditorTimelineClipRow, 1U> &clip_rows,
    std::span<AnimationEditorTimelineClipRow> clip_output,
    const std::array<AnimationEditorTimelineTrackRow, MAX_ANIMATION_EDITOR_TIMELINE_TRACKS>
        &track_rows,
    std::span<AnimationEditorTimelineTrackRow> track_output,
    std::size_t track_count,
    const std::array<AnimationEditorTimelineKeyframeMarker, MAX_ANIMATION_EDITOR_TIMELINE_KEYFRAMES>
        &keyframe_markers,
    std::span<AnimationEditorTimelineKeyframeMarker> keyframe_output,
    std::size_t keyframe_count) {
    clip_output[0U] = clip_rows[0U];

    for (std::size_t index = 0U; index < track_count; ++index) {
        track_output[index] = track_rows[index];
    }

    for (std::size_t index = 0U; index < keyframe_count; ++index) {
        keyframe_output[index] = keyframe_markers[index];
    }
}

float ConvertNanosecondsToSeconds(std::uint64_t nanoseconds) {
    const double seconds =
        static_cast<double>(nanoseconds) / static_cast<double>(NANOSECONDS_PER_SECOND);
    return static_cast<float>(seconds);
}

bool ConvertSecondsToNanoseconds(float seconds, std::uint64_t *out_nanoseconds) {
    if (out_nanoseconds == nullptr) {
        return false;
    }

    if (!std::isfinite(seconds) || seconds < 0.0F) {
        return false;
    }

    const double nanoseconds =
        static_cast<double>(seconds) * static_cast<double>(NANOSECONDS_PER_SECOND);
    if (!std::isfinite(nanoseconds) ||
        nanoseconds > static_cast<double>(std::numeric_limits<std::uint64_t>::max())) {
        return false;
    }

    *out_nanoseconds = static_cast<std::uint64_t>(nanoseconds + 0.5);
    return true;
}

AnimationEditorSurfaceStatus ResolveWorkflowSampleTime(
    const AnimationEditorTimelineWorkflowRequest &request,
    float *out_sample_time_seconds) {
    if (out_sample_time_seconds == nullptr) {
        return AnimationEditorSurfaceStatus::InvalidArgument;
    }

    float sample_time_seconds = 0.0F;
    switch (request.command) {
        case AnimationEditorTimelineWorkflowCommand::Scrub:
            sample_time_seconds = request.requested_sample_time_seconds;
            break;
        case AnimationEditorTimelineWorkflowCommand::PlaybackTick:
            sample_time_seconds =
                request.current_sample_time_seconds +
                ConvertNanosecondsToSeconds(request.frame_context.delta_time_nanoseconds);
            break;
        default:
            return AnimationEditorSurfaceStatus::InvalidArgument;
    }

    if (!std::isfinite(sample_time_seconds) || sample_time_seconds < 0.0F) {
        return AnimationEditorSurfaceStatus::InvalidTime;
    }

    *out_sample_time_seconds = sample_time_seconds;
    return AnimationEditorSurfaceStatus::Success;
}

AnimationEditorSurfaceStatus CalculateWorkflowClipStartTime(
    const AnimationEditorTimelineWorkflowRequest &request,
    float sample_time_seconds,
    std::uint64_t *out_clip_start_time_nanoseconds) {
    std::uint64_t sample_time_nanoseconds = 0U;
    if (!ConvertSecondsToNanoseconds(sample_time_seconds, &sample_time_nanoseconds)) {
        return AnimationEditorSurfaceStatus::InvalidTime;
    }

    if (request.frame_context.fixed_time_nanoseconds < sample_time_nanoseconds) {
        return AnimationEditorSurfaceStatus::InvalidTime;
    }

    *out_clip_start_time_nanoseconds =
        request.frame_context.fixed_time_nanoseconds - sample_time_nanoseconds;
    return AnimationEditorSurfaceStatus::Success;
}

bool WorkflowOutputCapacityAvailable(
    const AnimationEditorTimelineWorkflowRequest &request,
    const AnimationEditorTimelineSurfaceResult &surface_result) {
    if (request.clip_rows.size() < surface_result.clip_row_count) {
        return false;
    }

    if (request.track_rows.size() < surface_result.track_row_count) {
        return false;
    }

    if (request.keyframe_markers.size() < surface_result.keyframe_marker_count) {
        return false;
    }

    if (request.preview_feedback_output.size() < surface_result.preview_feedback_count) {
        return false;
    }

    if (request.selection_feedback_output.size() < 1U) {
        return false;
    }

    return true;
}

bool FindTrackRowIndex(
    std::span<const AnimationEditorTimelineTrackRow> rows,
    std::size_t row_count,
    std::uint32_t track_id,
    std::size_t *out_index) {
    if (out_index == nullptr) {
        return false;
    }

    for (std::size_t index = 0U; index < row_count; ++index) {
        if (rows[index].track_id != track_id) {
            continue;
        }

        *out_index = index;
        return true;
    }

    return false;
}

bool FindKeyframeMarkerIndex(
    std::span<const AnimationEditorTimelineKeyframeMarker> markers,
    std::size_t marker_count,
    std::uint32_t track_id,
    std::size_t keyframe_index,
    std::size_t *out_index) {
    if (out_index == nullptr) {
        return false;
    }

    for (std::size_t index = 0U; index < marker_count; ++index) {
        const AnimationEditorTimelineKeyframeMarker &marker = markers[index];
        if (marker.track_id != track_id || marker.keyframe_index != keyframe_index) {
            continue;
        }

        *out_index = index;
        return true;
    }

    return false;
}

bool FindPreviewFeedbackIndex(
    std::span<const AnimationEditorPreviewFeedbackRecord> records,
    std::size_t record_count,
    std::uint32_t track_id,
    std::size_t *out_index) {
    if (out_index == nullptr) {
        return false;
    }

    for (std::size_t index = 0U; index < record_count; ++index) {
        if (records[index].track_id != track_id) {
            continue;
        }

        *out_index = index;
        return true;
    }

    return false;
}

AnimationEditorTimelineSelectionFeedbackRecord BuildSelectionFeedbackRecord(
    const AnimationEditorTimelineTrackRow &track_row,
    const AnimationEditorTimelineKeyframeMarker &keyframe_marker,
    const AnimationEditorPreviewFeedbackRecord &preview_feedback,
    float sample_time_seconds) {
    AnimationEditorTimelineSelectionFeedbackRecord record{};
    record.clip_id = track_row.clip_id;
    record.track_id = track_row.track_id;
    record.target = track_row.target;
    record.channel = track_row.channel;
    record.keyframe_index = keyframe_marker.keyframe_index;
    record.keyframe_time_seconds = keyframe_marker.time_seconds;
    record.keyframe_value = keyframe_marker.value;
    record.sample_time_seconds = sample_time_seconds;
    record.sampled_value = preview_feedback.sampled_value;
    record.preview_transform = preview_feedback.preview_transform;
    record.selected_track = true;
    record.selected_keyframe = true;
    record.matched_preview_host_feedback =
        preview_feedback.matched_preview_host_feedback;
    record.feedback_from_preview_host = preview_feedback.feedback_from_preview_host;
    return record;
}

void CopyPreviewFeedbackRows(
    const std::array<AnimationEditorPreviewFeedbackRecord, MAX_ANIMATION_EDITOR_TIMELINE_TRACKS>
        &preview_feedback,
    std::span<AnimationEditorPreviewFeedbackRecord> preview_output,
    std::size_t preview_feedback_count) {
    for (std::size_t index = 0U; index < preview_feedback_count; ++index) {
        preview_output[index] = preview_feedback[index];
    }
}

void CopyWorkflowResultFromSurface(
    const AnimationEditorTimelineSurfaceResult &surface_result,
    AnimationEditorTimelineWorkflowResult *workflow_result) {
    workflow_result->timeline_surface = surface_result;
    workflow_result->animation_status = surface_result.animation_status;
    workflow_result->sample_time_seconds = surface_result.sample_time_seconds;
    workflow_result->clip_row_count = surface_result.clip_row_count;
    workflow_result->track_row_count = surface_result.track_row_count;
    workflow_result->keyframe_marker_count = surface_result.keyframe_marker_count;
    workflow_result->preview_feedback_count = surface_result.preview_feedback_count;
    workflow_result->consumed_runtime_records = surface_result.consumed_runtime_records;
    workflow_result->built_timeline_rows = surface_result.built_timeline_rows;
    workflow_result->sampled_runtime_values = surface_result.sampled_runtime_values;
    workflow_result->consumed_preview_host_feedback =
        surface_result.consumed_preview_host_feedback;
    workflow_result->emitted_preview_feedback = surface_result.emitted_preview_feedback;
}
}

AnimationEditorSurfaceStatus BuildAnimationEditorTimelineSurface(
    const AnimationEditorTimelineSurfaceRequest &request,
    AnimationEditorTimelineSurfaceResult *out_result) {
    AnimationEditorTimelineSurfaceResult result{};
    result.clip_id = request.clip_id;

    if (out_result == nullptr) {
        return AnimationEditorSurfaceStatus::InvalidArgument;
    }

    if (!IsConstSpanStorageValid(request.clips) ||
        !IsConstSpanStorageValid(request.tracks) ||
        !IsConstSpanStorageValid(request.keyframes) ||
        !IsConstSpanStorageValid(request.preview_transform_feedback) ||
        !IsSpanStorageValid(request.clip_rows) ||
        !IsSpanStorageValid(request.track_rows) ||
        !IsSpanStorageValid(request.keyframe_markers) ||
        !IsSpanStorageValid(request.preview_feedback_output)) {
        result.status = AnimationEditorSurfaceStatus::InvalidArgument;
        result.blocked_layer = AnimationEditorSurfaceBlockedLayer::RuntimeAnimationRecords;
        *out_result = result;
        return result.status;
    }

    const AnimationRuntimeClipRecord *clip = FindClip(request.clips, request.clip_id);
    if (clip == nullptr) {
        result.status = AnimationEditorSurfaceStatus::MissingClip;
        result.blocked_layer = AnimationEditorSurfaceBlockedLayer::RuntimeAnimationRecords;
        *out_result = result;
        return result.status;
    }

    result.consumed_runtime_records = true;
    if (clip->track_count > MAX_ANIMATION_EDITOR_TIMELINE_TRACKS) {
        result.status = AnimationEditorSurfaceStatus::OutputCapacityExceeded;
        result.blocked_layer = AnimationEditorSurfaceBlockedLayer::TimelineOutput;
        *out_result = result;
        return result.status;
    }

    std::array<AnimationRuntimeSampledValue, MAX_ANIMATION_EDITOR_TIMELINE_TRACKS>
        sampled_values{};
    AnimationRuntimeSampleRequest sample_request{};
    sample_request.clip_id = request.clip_id;
    sample_request.clips = request.clips;
    sample_request.tracks = request.tracks;
    sample_request.keyframes = request.keyframes;
    sample_request.frame_context = request.frame_context;
    sample_request.clip_start_time_nanoseconds = request.clip_start_time_nanoseconds;

    AnimationRuntimeSampleResult sample_result{};
    AnimationRuntimeSampler sampler;
    const AnimationRuntimeStatus animation_status =
        sampler.Sample(
            sample_request,
            std::span<AnimationRuntimeSampledValue>(sampled_values.data(), clip->track_count),
            &sample_result);
    result.animation_status = animation_status;
    result.sample_time_seconds = sample_result.sample_time_seconds;
    result.sampled_value_count = sample_result.sampled_value_count;
    if (animation_status != AnimationRuntimeStatus::Success) {
        result.status = MapAnimationStatus(animation_status);
        result.blocked_layer = BlockedLayerForStatus(result.status);
        *out_result = result;
        return result.status;
    }

    result.sampled_runtime_values = true;

    std::size_t keyframe_count = 0U;
    for (std::size_t track_offset = 0U; track_offset < clip->track_count; ++track_offset) {
        const std::size_t track_index = clip->first_track_index + track_offset;
        const AnimationRuntimeTrackRecord &track = request.tracks[track_index];
        if (keyframe_count > MAX_ANIMATION_EDITOR_TIMELINE_KEYFRAMES - track.keyframe_count) {
            result.status = AnimationEditorSurfaceStatus::OutputCapacityExceeded;
            result.blocked_layer = AnimationEditorSurfaceBlockedLayer::TimelineOutput;
            *out_result = result;
            return result.status;
        }

        keyframe_count += track.keyframe_count;
    }

    const bool preview_feedback_requested =
        request.require_preview_feedback || !request.preview_feedback_output.empty();
    if (!OutputCapacityAvailable(request, *clip, keyframe_count, preview_feedback_requested)) {
        result.status = AnimationEditorSurfaceStatus::OutputCapacityExceeded;
        result.blocked_layer = AnimationEditorSurfaceBlockedLayer::TimelineOutput;
        *out_result = result;
        return result.status;
    }

    std::array<AnimationEditorTimelineClipRow, 1U> staged_clip_rows{
        BuildClipRow(*clip)};
    std::array<AnimationEditorTimelineTrackRow, MAX_ANIMATION_EDITOR_TIMELINE_TRACKS>
        staged_track_rows{};
    std::array<AnimationEditorTimelineKeyframeMarker, MAX_ANIMATION_EDITOR_TIMELINE_KEYFRAMES>
        staged_keyframe_markers{};
    std::array<AnimationEditorPreviewFeedbackRecord, MAX_ANIMATION_EDITOR_TIMELINE_TRACKS>
        staged_preview_feedback{};

    std::size_t staged_keyframe_count = 0U;
    std::size_t staged_preview_feedback_count = 0U;
    for (std::size_t track_offset = 0U; track_offset < clip->track_count; ++track_offset) {
        const std::size_t track_index = clip->first_track_index + track_offset;
        const AnimationRuntimeTrackRecord &track = request.tracks[track_index];
        staged_track_rows[track_offset] = BuildTrackRow(*clip, track);

        for (std::size_t key_offset = 0U; key_offset < track.keyframe_count; ++key_offset) {
            const std::size_t keyframe_index = track.first_keyframe_index + key_offset;
            staged_keyframe_markers[staged_keyframe_count] =
                BuildKeyframeMarker(
                    *clip,
                    track,
                    request.keyframes[keyframe_index],
                    keyframe_index);
            ++staged_keyframe_count;
        }

        if (!preview_feedback_requested) {
            continue;
        }

        const AnimationRuntimeSampledValue &sample = sampled_values[track_offset];
        const PreviewHostTransformFeedback *feedback =
            FindTransformFeedback(request.preview_transform_feedback, sample.target);
        if (feedback == nullptr) {
            result.status = AnimationEditorSurfaceStatus::PreviewFeedbackMissing;
            result.blocked_layer = AnimationEditorSurfaceBlockedLayer::PreviewHostFeedback;
            *out_result = result;
            return result.status;
        }

        result.consumed_preview_host_feedback = true;
        staged_preview_feedback[staged_preview_feedback_count] =
            BuildPreviewFeedbackRecord(
                *clip,
                track,
                sample,
                *feedback,
                sample_result.sample_time_seconds);
        ++staged_preview_feedback_count;
    }

    CopyTimelineRows(
        staged_clip_rows,
        request.clip_rows,
        staged_track_rows,
        request.track_rows,
        clip->track_count,
        staged_keyframe_markers,
        request.keyframe_markers,
        staged_keyframe_count);

    for (std::size_t index = 0U; index < staged_preview_feedback_count; ++index) {
        request.preview_feedback_output[index] = staged_preview_feedback[index];
    }

    result.status = AnimationEditorSurfaceStatus::Success;
    result.blocked_layer = AnimationEditorSurfaceBlockedLayer::None;
    result.clip_row_count = 1U;
    result.track_row_count = clip->track_count;
    result.keyframe_marker_count = staged_keyframe_count;
    result.preview_feedback_count = staged_preview_feedback_count;
    result.built_timeline_rows = true;
    result.emitted_preview_feedback = staged_preview_feedback_count > 0U;
    *out_result = result;
    return result.status;
}

AnimationEditorSurfaceStatus BuildAnimationEditorTimelineWorkflow(
    const AnimationEditorTimelineWorkflowRequest &request,
    AnimationEditorTimelineWorkflowResult *out_result) {
    AnimationEditorTimelineWorkflowResult result{};
    result.clip_id = request.clip_id;
    result.selected_track_id = request.selected_track_id;
    result.selected_keyframe_index = request.selected_keyframe_index;

    if (out_result == nullptr) {
        return AnimationEditorSurfaceStatus::InvalidArgument;
    }

    if (!IsConstSpanStorageValid(request.clips) ||
        !IsConstSpanStorageValid(request.tracks) ||
        !IsConstSpanStorageValid(request.keyframes) ||
        !IsConstSpanStorageValid(request.preview_transform_feedback) ||
        !IsSpanStorageValid(request.clip_rows) ||
        !IsSpanStorageValid(request.track_rows) ||
        !IsSpanStorageValid(request.keyframe_markers) ||
        !IsSpanStorageValid(request.preview_feedback_output) ||
        !IsSpanStorageValid(request.selection_feedback_output)) {
        result.status = AnimationEditorSurfaceStatus::InvalidArgument;
        result.blocked_layer = AnimationEditorSurfaceBlockedLayer::RuntimeAnimationRecords;
        *out_result = result;
        return result.status;
    }

    if (request.selected_track_id == 0U) {
        result.status = AnimationEditorSurfaceStatus::MissingTrack;
        result.blocked_layer = AnimationEditorSurfaceBlockedLayer::RuntimeAnimationRecords;
        *out_result = result;
        return result.status;
    }

    float workflow_sample_time_seconds = 0.0F;
    AnimationEditorSurfaceStatus status =
        ResolveWorkflowSampleTime(request, &workflow_sample_time_seconds);
    if (status != AnimationEditorSurfaceStatus::Success) {
        result.status = status;
        result.blocked_layer = BlockedLayerForStatus(status);
        *out_result = result;
        return result.status;
    }

    std::uint64_t workflow_clip_start_time_nanoseconds = 0U;
    status =
        CalculateWorkflowClipStartTime(
            request,
            workflow_sample_time_seconds,
            &workflow_clip_start_time_nanoseconds);
    if (status != AnimationEditorSurfaceStatus::Success) {
        result.status = status;
        result.blocked_layer = BlockedLayerForStatus(status);
        *out_result = result;
        return result.status;
    }

    std::array<AnimationEditorTimelineClipRow, 1U> staged_clip_rows{};
    std::array<AnimationEditorTimelineTrackRow, MAX_ANIMATION_EDITOR_TIMELINE_TRACKS>
        staged_track_rows{};
    std::array<AnimationEditorTimelineKeyframeMarker, MAX_ANIMATION_EDITOR_TIMELINE_KEYFRAMES>
        staged_keyframe_markers{};
    std::array<AnimationEditorPreviewFeedbackRecord, MAX_ANIMATION_EDITOR_TIMELINE_TRACKS>
        staged_preview_feedback{};

    AnimationEditorTimelineSurfaceRequest surface_request{};
    surface_request.clip_id = request.clip_id;
    surface_request.clips = request.clips;
    surface_request.tracks = request.tracks;
    surface_request.keyframes = request.keyframes;
    surface_request.frame_context = request.frame_context;
    surface_request.clip_start_time_nanoseconds = workflow_clip_start_time_nanoseconds;
    surface_request.preview_transform_feedback = request.preview_transform_feedback;
    surface_request.require_preview_feedback = true;
    surface_request.clip_rows = staged_clip_rows;
    surface_request.track_rows = staged_track_rows;
    surface_request.keyframe_markers = staged_keyframe_markers;
    surface_request.preview_feedback_output = staged_preview_feedback;

    AnimationEditorTimelineSurfaceResult surface_result{};
    status = BuildAnimationEditorTimelineSurface(surface_request, &surface_result);
    CopyWorkflowResultFromSurface(surface_result, &result);
    if (status != AnimationEditorSurfaceStatus::Success) {
        result.status = status;
        result.blocked_layer = surface_result.blocked_layer;
        *out_result = result;
        return result.status;
    }

    if (!WorkflowOutputCapacityAvailable(request, surface_result)) {
        result.status = AnimationEditorSurfaceStatus::OutputCapacityExceeded;
        result.blocked_layer = AnimationEditorSurfaceBlockedLayer::TimelineOutput;
        *out_result = result;
        return result.status;
    }

    std::size_t selected_track_row_index = 0U;
    if (!FindTrackRowIndex(
            std::span<const AnimationEditorTimelineTrackRow>(
                staged_track_rows.data(),
                surface_result.track_row_count),
            surface_result.track_row_count,
            request.selected_track_id,
            &selected_track_row_index)) {
        result.status = AnimationEditorSurfaceStatus::MissingTrack;
        result.blocked_layer = AnimationEditorSurfaceBlockedLayer::RuntimeAnimationRecords;
        *out_result = result;
        return result.status;
    }

    std::size_t selected_keyframe_marker_index = 0U;
    if (!FindKeyframeMarkerIndex(
            std::span<const AnimationEditorTimelineKeyframeMarker>(
                staged_keyframe_markers.data(),
                surface_result.keyframe_marker_count),
            surface_result.keyframe_marker_count,
            request.selected_track_id,
            request.selected_keyframe_index,
            &selected_keyframe_marker_index)) {
        result.status = AnimationEditorSurfaceStatus::MissingKeyframe;
        result.blocked_layer = AnimationEditorSurfaceBlockedLayer::RuntimeAnimationRecords;
        *out_result = result;
        return result.status;
    }

    std::size_t selected_preview_feedback_index = 0U;
    if (!FindPreviewFeedbackIndex(
            std::span<const AnimationEditorPreviewFeedbackRecord>(
                staged_preview_feedback.data(),
                surface_result.preview_feedback_count),
            surface_result.preview_feedback_count,
            request.selected_track_id,
            &selected_preview_feedback_index)) {
        result.status = AnimationEditorSurfaceStatus::PreviewFeedbackMissing;
        result.blocked_layer = AnimationEditorSurfaceBlockedLayer::PreviewHostFeedback;
        *out_result = result;
        return result.status;
    }

    AnimationEditorTimelineTrackRow &selected_track_row =
        staged_track_rows[selected_track_row_index];
    AnimationEditorTimelineKeyframeMarker &selected_keyframe_marker =
        staged_keyframe_markers[selected_keyframe_marker_index];
    const AnimationEditorPreviewFeedbackRecord &selected_preview_feedback =
        staged_preview_feedback[selected_preview_feedback_index];

    selected_track_row.selected = true;
    selected_track_row.sampled = true;
    selected_track_row.sample_time_seconds = surface_result.sample_time_seconds;
    selected_track_row.sampled_value = selected_preview_feedback.sampled_value;
    selected_track_row.preview_feedback_visible =
        selected_preview_feedback.feedback_from_preview_host;

    selected_keyframe_marker.selected = true;
    selected_keyframe_marker.at_or_before_sample_time =
        selected_keyframe_marker.time_seconds <= surface_result.sample_time_seconds;

    const AnimationEditorTimelineSelectionFeedbackRecord selection_feedback =
        BuildSelectionFeedbackRecord(
            selected_track_row,
            selected_keyframe_marker,
            selected_preview_feedback,
            surface_result.sample_time_seconds);

    CopyTimelineRows(
        staged_clip_rows,
        request.clip_rows,
        staged_track_rows,
        request.track_rows,
        surface_result.track_row_count,
        staged_keyframe_markers,
        request.keyframe_markers,
        surface_result.keyframe_marker_count);
    CopyPreviewFeedbackRows(
        staged_preview_feedback,
        request.preview_feedback_output,
        surface_result.preview_feedback_count);
    request.selection_feedback_output[0U] = selection_feedback;

    result.status = AnimationEditorSurfaceStatus::Success;
    result.blocked_layer = AnimationEditorSurfaceBlockedLayer::None;
    result.consumed_workflow_command = true;
    result.scrub_applied =
        request.command == AnimationEditorTimelineWorkflowCommand::Scrub;
    result.playback_tick_applied =
        request.command == AnimationEditorTimelineWorkflowCommand::PlaybackTick;
    result.updated_sample_time = true;
    result.updated_track_row_state = true;
    result.selection_feedback_count = 1U;
    result.emitted_selected_track_feedback = true;
    result.emitted_selected_key_feedback = true;
    *out_result = result;
    return result.status;
}
}
