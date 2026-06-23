// Module: YuEngine AnimationEditor
// File: Src/YuEngine/AnimationEditor/Src/AnimationEditorSurface.cpp

#include "YuEngine/AnimationEditor/AnimationEditorSurface.h"

#include <array>
#include <cstddef>

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
}
