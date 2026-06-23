// Module: YuEngine AnimationEditor
// File: Src/YuEngine/AnimationEditor/Include/YuEngine/AnimationEditor/AnimationEditorSurface.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/Animation/AnimationRuntimeSampler.h"
#include "YuEngine/Kernel/RuntimeFrameContext.h"
#include "YuEngine/PreviewHost/PreviewHost.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

namespace yuengine::animationeditor {
constexpr std::size_t MAX_ANIMATION_EDITOR_TIMELINE_TRACKS = 16U;
constexpr std::size_t MAX_ANIMATION_EDITOR_TIMELINE_KEYFRAMES = 64U;

enum class AnimationEditorSurfaceStatus {
    Success,
    InvalidArgument,
    MissingClip,
    InvalidClip,
    MissingTrack,
    InvalidTrack,
    MissingKeyframe,
    InvalidKeyframe,
    InvalidTime,
    UnsupportedInterpolation,
    UnsupportedChannel,
    OutputCapacityExceeded,
    SampleFailed,
    PreviewFeedbackMissing
};

enum class AnimationEditorSurfaceBlockedLayer {
    None,
    RuntimeAnimationRecords,
    TimelineOutput,
    RuntimeSampler,
    PreviewHostFeedback
};

enum class AnimationEditorTimelineWorkflowCommand {
    Scrub,
    PlaybackTick
};

struct AnimationEditorTimelineClipRow final {
    std::uint32_t clip_id = 0U;
    float duration_seconds = 0.0F;
    std::size_t first_track_index = 0U;
    std::size_t track_count = 0U;
    std::uint32_t layer_count = 0U;
    bool selected = false;
    bool runtime_valid = false;
};

struct AnimationEditorTimelineTrackRow final {
    std::uint32_t clip_id = 0U;
    std::uint32_t track_id = 0U;
    yuengine::world::WorldObjectId target{};
    yuengine::animation::AnimationRuntimeChannel channel =
        yuengine::animation::AnimationRuntimeChannel::TranslationX;
    yuengine::animation::AnimationRuntimeInterpolation interpolation =
        yuengine::animation::AnimationRuntimeInterpolation::Linear;
    std::size_t first_keyframe_index = 0U;
    std::size_t keyframe_count = 0U;
    float sample_time_seconds = 0.0F;
    float sampled_value = 0.0F;
    bool active = false;
    bool selected = false;
    bool sampled = false;
    bool preview_feedback_visible = false;
};

struct AnimationEditorTimelineKeyframeMarker final {
    std::uint32_t clip_id = 0U;
    std::uint32_t track_id = 0U;
    yuengine::world::WorldObjectId target{};
    yuengine::animation::AnimationRuntimeChannel channel =
        yuengine::animation::AnimationRuntimeChannel::TranslationX;
    std::size_t keyframe_index = 0U;
    float time_seconds = 0.0F;
    float value = 0.0F;
    bool valid = false;
    bool selected = false;
    bool at_or_before_sample_time = false;
};

struct AnimationEditorPreviewFeedbackRecord final {
    std::uint32_t clip_id = 0U;
    std::uint32_t track_id = 0U;
    yuengine::world::WorldObjectId target{};
    yuengine::animation::AnimationRuntimeChannel channel =
        yuengine::animation::AnimationRuntimeChannel::TranslationX;
    float sample_time_seconds = 0.0F;
    float sampled_value = 0.0F;
    yuengine::world::WorldTransformState preview_transform{};
    bool matched_preview_host_feedback = false;
    bool feedback_from_preview_host = false;
};

struct AnimationEditorTimelineSelectionFeedbackRecord final {
    std::uint32_t clip_id = 0U;
    std::uint32_t track_id = 0U;
    yuengine::world::WorldObjectId target{};
    yuengine::animation::AnimationRuntimeChannel channel =
        yuengine::animation::AnimationRuntimeChannel::TranslationX;
    std::size_t keyframe_index = 0U;
    float keyframe_time_seconds = 0.0F;
    float keyframe_value = 0.0F;
    float sample_time_seconds = 0.0F;
    float sampled_value = 0.0F;
    yuengine::world::WorldTransformState preview_transform{};
    bool selected_track = false;
    bool selected_keyframe = false;
    bool matched_preview_host_feedback = false;
    bool feedback_from_preview_host = false;
};

struct AnimationEditorTimelineSurfaceRequest final {
    std::uint32_t clip_id = 0U;
    std::span<const yuengine::animation::AnimationRuntimeClipRecord> clips{};
    std::span<const yuengine::animation::AnimationRuntimeTrackRecord> tracks{};
    std::span<const yuengine::animation::AnimationRuntimeKeyframeRecord> keyframes{};
    yuengine::kernel::RuntimeFrameContext frame_context{};
    std::uint64_t clip_start_time_nanoseconds = 0U;
    std::span<const yuengine::previewhost::PreviewHostTransformFeedback>
        preview_transform_feedback{};
    bool require_preview_feedback = false;
    std::span<AnimationEditorTimelineClipRow> clip_rows{};
    std::span<AnimationEditorTimelineTrackRow> track_rows{};
    std::span<AnimationEditorTimelineKeyframeMarker> keyframe_markers{};
    std::span<AnimationEditorPreviewFeedbackRecord> preview_feedback_output{};
};

struct AnimationEditorTimelineSurfaceResult final {
    AnimationEditorSurfaceStatus status = AnimationEditorSurfaceStatus::InvalidArgument;
    yuengine::animation::AnimationRuntimeStatus animation_status =
        yuengine::animation::AnimationRuntimeStatus::Success;
    AnimationEditorSurfaceBlockedLayer blocked_layer =
        AnimationEditorSurfaceBlockedLayer::RuntimeAnimationRecords;
    std::uint32_t clip_id = 0U;
    float sample_time_seconds = 0.0F;
    std::size_t clip_row_count = 0U;
    std::size_t track_row_count = 0U;
    std::size_t keyframe_marker_count = 0U;
    std::size_t sampled_value_count = 0U;
    std::size_t preview_feedback_count = 0U;
    bool consumed_runtime_records = false;
    bool built_timeline_rows = false;
    bool sampled_runtime_values = false;
    bool consumed_preview_host_feedback = false;
    bool emitted_preview_feedback = false;
    bool mutated_runtime_data = false;
    bool opened_native_window = false;
    bool used_web_timeline = false;

    bool Succeeded() const {
        return status == AnimationEditorSurfaceStatus::Success;
    }
};

struct AnimationEditorTimelineWorkflowRequest final {
    AnimationEditorTimelineWorkflowCommand command =
        AnimationEditorTimelineWorkflowCommand::Scrub;
    std::uint32_t clip_id = 0U;
    std::span<const yuengine::animation::AnimationRuntimeClipRecord> clips{};
    std::span<const yuengine::animation::AnimationRuntimeTrackRecord> tracks{};
    std::span<const yuengine::animation::AnimationRuntimeKeyframeRecord> keyframes{};
    yuengine::kernel::RuntimeFrameContext frame_context{};
    float current_sample_time_seconds = 0.0F;
    float requested_sample_time_seconds = 0.0F;
    std::uint32_t selected_track_id = 0U;
    std::size_t selected_keyframe_index = 0U;
    std::span<const yuengine::previewhost::PreviewHostTransformFeedback>
        preview_transform_feedback{};
    std::span<AnimationEditorTimelineClipRow> clip_rows{};
    std::span<AnimationEditorTimelineTrackRow> track_rows{};
    std::span<AnimationEditorTimelineKeyframeMarker> keyframe_markers{};
    std::span<AnimationEditorPreviewFeedbackRecord> preview_feedback_output{};
    std::span<AnimationEditorTimelineSelectionFeedbackRecord> selection_feedback_output{};
};

struct AnimationEditorTimelineWorkflowResult final {
    AnimationEditorSurfaceStatus status = AnimationEditorSurfaceStatus::InvalidArgument;
    yuengine::animation::AnimationRuntimeStatus animation_status =
        yuengine::animation::AnimationRuntimeStatus::Success;
    AnimationEditorSurfaceBlockedLayer blocked_layer =
        AnimationEditorSurfaceBlockedLayer::RuntimeAnimationRecords;
    AnimationEditorTimelineSurfaceResult timeline_surface{};
    std::uint32_t clip_id = 0U;
    std::uint32_t selected_track_id = 0U;
    std::size_t selected_keyframe_index = 0U;
    float sample_time_seconds = 0.0F;
    std::size_t clip_row_count = 0U;
    std::size_t track_row_count = 0U;
    std::size_t keyframe_marker_count = 0U;
    std::size_t preview_feedback_count = 0U;
    std::size_t selection_feedback_count = 0U;
    bool consumed_workflow_command = false;
    bool scrub_applied = false;
    bool playback_tick_applied = false;
    bool updated_sample_time = false;
    bool consumed_runtime_records = false;
    bool built_timeline_rows = false;
    bool sampled_runtime_values = false;
    bool updated_track_row_state = false;
    bool consumed_preview_host_feedback = false;
    bool emitted_preview_feedback = false;
    bool emitted_selected_track_feedback = false;
    bool emitted_selected_key_feedback = false;
    bool mutated_runtime_data = false;
    bool opened_native_window = false;
    bool used_web_timeline = false;

    bool Succeeded() const {
        return status == AnimationEditorSurfaceStatus::Success;
    }
};

AnimationEditorSurfaceStatus BuildAnimationEditorTimelineSurface(
    const AnimationEditorTimelineSurfaceRequest &request,
    AnimationEditorTimelineSurfaceResult *out_result);

AnimationEditorSurfaceStatus BuildAnimationEditorTimelineWorkflow(
    const AnimationEditorTimelineWorkflowRequest &request,
    AnimationEditorTimelineWorkflowResult *out_result);
}
