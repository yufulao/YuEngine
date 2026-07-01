// 模块: YuEngine Animation
// 文件: Src/YuEngine/Animation/Src/AnimationRuntimeSampler.cpp

#include "YuEngine/Animation/AnimationRuntimeSampler.h"

#include <cmath>

namespace yuengine::animation {
namespace {
constexpr std::uint64_t NANOSECONDS_PER_SECOND = 1000000000ULL;

bool IsSupportedChannel(AnimationRuntimeChannel channel) {
    switch (channel) {
    case AnimationRuntimeChannel::TranslationX:
    case AnimationRuntimeChannel::TranslationY:
    case AnimationRuntimeChannel::TranslationZ:
    case AnimationRuntimeChannel::RotationX:
    case AnimationRuntimeChannel::RotationY:
    case AnimationRuntimeChannel::RotationZ:
    case AnimationRuntimeChannel::RotationW:
    case AnimationRuntimeChannel::ScaleX:
    case AnimationRuntimeChannel::ScaleY:
    case AnimationRuntimeChannel::ScaleZ:
        return true;
    default:
        return false;
    }
}

bool IsSupportedInterpolation(AnimationRuntimeInterpolation interpolation) {
    switch (interpolation) {
    case AnimationRuntimeInterpolation::Step:
    case AnimationRuntimeInterpolation::Linear:
        return true;
    default:
        return false;
    }
}

float ConvertNanosecondsToSeconds(std::uint64_t nanoseconds) {
    const double seconds =
        static_cast<double>(nanoseconds) / static_cast<double>(NANOSECONDS_PER_SECOND);
    return static_cast<float>(seconds);
}

AnimationRuntimeStatus MapWorldStatus(yuengine::world::WorldTransformStatus status) {
    if (status == yuengine::world::WorldTransformStatus::Success) {
        return AnimationRuntimeStatus::Success;
    }

    if (status == yuengine::world::WorldTransformStatus::InvalidWorldObjectId) {
        return AnimationRuntimeStatus::InvalidTarget;
    }

    if (status == yuengine::world::WorldTransformStatus::TransformNotFound) {
        return AnimationRuntimeStatus::TargetNotFound;
    }

    return AnimationRuntimeStatus::TransformApplyFailed;
}

void ApplyChannelValue(
    AnimationRuntimeChannel channel,
    float value,
    yuengine::world::WorldTransformState *transform_state) {
    if (transform_state == nullptr) {
        return;
    }

    switch (channel) {
    case AnimationRuntimeChannel::TranslationX:
        transform_state->translation_x = value;
        break;
    case AnimationRuntimeChannel::TranslationY:
        transform_state->translation_y = value;
        break;
    case AnimationRuntimeChannel::TranslationZ:
        transform_state->translation_z = value;
        break;
    case AnimationRuntimeChannel::RotationX:
        transform_state->rotation_x = value;
        break;
    case AnimationRuntimeChannel::RotationY:
        transform_state->rotation_y = value;
        break;
    case AnimationRuntimeChannel::RotationZ:
        transform_state->rotation_z = value;
        break;
    case AnimationRuntimeChannel::RotationW:
        transform_state->rotation_w = value;
        break;
    case AnimationRuntimeChannel::ScaleX:
        transform_state->scale_x = value;
        break;
    case AnimationRuntimeChannel::ScaleY:
        transform_state->scale_y = value;
        break;
    case AnimationRuntimeChannel::ScaleZ:
        transform_state->scale_z = value;
        break;
    default:
        break;
    }
}
}

AnimationRuntimeStatus AnimationRuntimeSampler::Sample(
    const AnimationRuntimeSampleRequest &request,
    std::span<AnimationRuntimeSampledValue> out_values,
    AnimationRuntimeSampleResult *out_result) const {
    AnimationRuntimeSampleResult result{};
    result.clip_id = request.clip_id;

    if (out_result == nullptr) {
        return AnimationRuntimeStatus::NullPointer;
    }

    float sample_time_seconds = 0.0F;
    AnimationRuntimeStatus status = CalculateSampleTime(request, &sample_time_seconds);
    if (status != AnimationRuntimeStatus::Success) {
        result.status = status;
        *out_result = result;
        return status;
    }

    result.sample_time_seconds = sample_time_seconds;

    const AnimationRuntimeClipRecord *clip = FindClip(request);
    if (clip == nullptr) {
        result.status = AnimationRuntimeStatus::MissingClip;
        *out_result = result;
        return result.status;
    }

    status = ValidateClip(request, *clip, sample_time_seconds, out_values);
    if (status != AnimationRuntimeStatus::Success) {
        if (status == AnimationRuntimeStatus::OutputCapacityExceeded) {
            result.required_sampled_value_count = clip->track_count;
        }

        if (status == AnimationRuntimeStatus::LayerCapacityExceeded) {
            result.required_layer_count = clip->layer_count;
        }

        result.status = status;
        *out_result = result;
        return status;
    }

    for (std::size_t index = 0U; index < clip->track_count; ++index) {
        const std::size_t track_index = clip->first_track_index + index;
        const AnimationRuntimeTrackRecord &track = request.tracks[track_index];
        const std::size_t first_keyframe_index = track.first_keyframe_index;
        const std::size_t keyframe_count = track.keyframe_count;
        const std::span<const AnimationRuntimeKeyframeRecord> keyframes =
            request.keyframes.subspan(first_keyframe_index, keyframe_count);
        status = SampleTrack(track, keyframes, sample_time_seconds, &out_values[index]);
        if (status != AnimationRuntimeStatus::Success) {
            result.status = status;
            *out_result = result;
            return status;
        }
    }

    result.sampled_value_count = clip->track_count;
    result.status = AnimationRuntimeStatus::Success;
    *out_result = result;
    return AnimationRuntimeStatus::Success;
}

AnimationRuntimeStatus AnimationRuntimeSampler::ApplySampledTransform(
    const AnimationRuntimeTransformApplyRequest &request,
    AnimationRuntimeTransformApplyResult *out_result) const {
    AnimationRuntimeTransformApplyResult result{};

    if (out_result == nullptr) {
        return AnimationRuntimeStatus::NullPointer;
    }

    AnimationRuntimeStatus status = ValidateApplyRequest(request);
    if (status != AnimationRuntimeStatus::Success) {
        result.status = status;
        *out_result = result;
        return status;
    }

    for (std::size_t index = 0U; index < request.sampled_values.size(); ++index) {
        const AnimationRuntimeSampledValue &value = request.sampled_values[index];
        yuengine::world::WorldTransformStatus world_status =
            yuengine::world::WorldTransformStatus::Success;
        status = ApplyValue(*request.transform_bridge, value, &world_status);
        if (status != AnimationRuntimeStatus::Success) {
            result.status = status;
            result.last_world_status = world_status;
            *out_result = result;
            return status;
        }

        ++result.applied_value_count;
        result.last_world_status = world_status;
        if (IsFirstTargetOccurrence(request.sampled_values, index)) {
            ++result.updated_object_count;
        }
    }

    result.status = AnimationRuntimeStatus::Success;
    *out_result = result;
    return AnimationRuntimeStatus::Success;
}

AnimationRuntimeStatus AnimationRuntimeSampler::CalculateSampleTime(
    const AnimationRuntimeSampleRequest &request,
    float *out_sample_time_seconds) const {
    if (out_sample_time_seconds == nullptr) {
        return AnimationRuntimeStatus::NullPointer;
    }

    if (request.frame_context.fixed_time_nanoseconds < request.clip_start_time_nanoseconds) {
        return AnimationRuntimeStatus::InvalidTime;
    }

    const std::uint64_t local_time_nanoseconds =
        request.frame_context.fixed_time_nanoseconds - request.clip_start_time_nanoseconds;
    const float sample_time_seconds = ConvertNanosecondsToSeconds(local_time_nanoseconds);
    if (!std::isfinite(sample_time_seconds)) {
        return AnimationRuntimeStatus::InvalidTime;
    }

    *out_sample_time_seconds = sample_time_seconds;
    return AnimationRuntimeStatus::Success;
}

const AnimationRuntimeClipRecord *AnimationRuntimeSampler::FindClip(
    const AnimationRuntimeSampleRequest &request) const {
    for (const AnimationRuntimeClipRecord &clip : request.clips) {
        if (clip.clip_id != request.clip_id) {
            continue;
        }

        return &clip;
    }

    return nullptr;
}

AnimationRuntimeStatus AnimationRuntimeSampler::ValidateClip(
    const AnimationRuntimeSampleRequest &request,
    const AnimationRuntimeClipRecord &clip,
    float sample_time_seconds,
    std::span<AnimationRuntimeSampledValue> out_values) const {
    if (!clip.is_valid || clip.clip_id == 0U) {
        return AnimationRuntimeStatus::InvalidClip;
    }

    if (!std::isfinite(clip.duration_seconds) || clip.duration_seconds <= 0.0F) {
        return AnimationRuntimeStatus::InvalidClip;
    }

    if (clip.layer_count > MAX_ANIMATION_RUNTIME_LAYER_COUNT) {
        return AnimationRuntimeStatus::LayerCapacityExceeded;
    }

    if (clip.track_count == 0U) {
        return AnimationRuntimeStatus::MissingTrack;
    }

    if (clip.first_track_index >= request.tracks.size()) {
        return AnimationRuntimeStatus::MissingTrack;
    }

    const std::size_t end_track_index = clip.first_track_index + clip.track_count;
    if (end_track_index > request.tracks.size()) {
        return AnimationRuntimeStatus::MissingTrack;
    }

    if (sample_time_seconds > clip.duration_seconds) {
        return AnimationRuntimeStatus::TimeOutOfRange;
    }

    if (out_values.size() < clip.track_count) {
        return AnimationRuntimeStatus::OutputCapacityExceeded;
    }

    for (std::size_t index = clip.first_track_index; index < end_track_index; ++index) {
        const AnimationRuntimeTrackRecord &track = request.tracks[index];
        const AnimationRuntimeStatus status = ValidateTrack(request, track, sample_time_seconds);
        if (status != AnimationRuntimeStatus::Success) {
            return status;
        }
    }

    return AnimationRuntimeStatus::Success;
}

AnimationRuntimeStatus AnimationRuntimeSampler::ValidateTrack(
    const AnimationRuntimeSampleRequest &request,
    const AnimationRuntimeTrackRecord &track,
    float sample_time_seconds) const {
    if (!track.is_valid || track.track_id == 0U) {
        return AnimationRuntimeStatus::InvalidTrack;
    }

    if (!track.target.IsValid()) {
        return AnimationRuntimeStatus::InvalidTarget;
    }

    if (!IsSupportedChannel(track.channel)) {
        return AnimationRuntimeStatus::UnsupportedChannel;
    }

    if (!IsSupportedInterpolation(track.interpolation)) {
        return AnimationRuntimeStatus::UnsupportedInterpolation;
    }

    if (track.keyframe_count == 0U) {
        return AnimationRuntimeStatus::MissingKeyframe;
    }

    if (track.first_keyframe_index >= request.keyframes.size()) {
        return AnimationRuntimeStatus::MissingKeyframe;
    }

    const std::size_t end_keyframe_index = track.first_keyframe_index + track.keyframe_count;
    if (end_keyframe_index > request.keyframes.size()) {
        return AnimationRuntimeStatus::MissingKeyframe;
    }

    const std::span<const AnimationRuntimeKeyframeRecord> keyframes =
        request.keyframes.subspan(track.first_keyframe_index, track.keyframe_count);
    return ValidateKeyframes(keyframes, sample_time_seconds);
}

AnimationRuntimeStatus AnimationRuntimeSampler::ValidateKeyframes(
    std::span<const AnimationRuntimeKeyframeRecord> keyframes,
    float sample_time_seconds) const {
    if (keyframes.empty()) {
        return AnimationRuntimeStatus::MissingKeyframe;
    }

    float previous_time_seconds = -1.0F;
    for (const AnimationRuntimeKeyframeRecord &keyframe : keyframes) {
        if (!keyframe.is_valid) {
            return AnimationRuntimeStatus::InvalidKeyframe;
        }

        if (!std::isfinite(keyframe.time_seconds) || !std::isfinite(keyframe.value)) {
            return AnimationRuntimeStatus::InvalidKeyframe;
        }

        if (keyframe.time_seconds < 0.0F) {
            return AnimationRuntimeStatus::InvalidKeyframe;
        }

        if (keyframe.time_seconds < previous_time_seconds) {
            return AnimationRuntimeStatus::InvalidKeyframe;
        }

        previous_time_seconds = keyframe.time_seconds;
    }

    if (sample_time_seconds < keyframes.front().time_seconds) {
        return AnimationRuntimeStatus::TimeOutOfRange;
    }

    if (sample_time_seconds > keyframes.back().time_seconds) {
        return AnimationRuntimeStatus::TimeOutOfRange;
    }

    return AnimationRuntimeStatus::Success;
}

AnimationRuntimeStatus AnimationRuntimeSampler::SampleTrack(
    const AnimationRuntimeTrackRecord &track,
    std::span<const AnimationRuntimeKeyframeRecord> keyframes,
    float sample_time_seconds,
    AnimationRuntimeSampledValue *out_value) const {
    if (out_value == nullptr) {
        return AnimationRuntimeStatus::NullPointer;
    }

    std::size_t lower_index = 0U;
    std::size_t upper_index = 0U;
    for (std::size_t index = 0U; index < keyframes.size(); ++index) {
        const AnimationRuntimeKeyframeRecord &keyframe = keyframes[index];
        if (keyframe.time_seconds <= sample_time_seconds) {
            lower_index = index;
        }

        if (keyframe.time_seconds >= sample_time_seconds) {
            upper_index = index;
            break;
        }
    }

    const AnimationRuntimeKeyframeRecord &lower = keyframes[lower_index];
    const AnimationRuntimeKeyframeRecord &upper = keyframes[upper_index];
    float sampled_value = lower.value;

    if (track.interpolation == AnimationRuntimeInterpolation::Linear &&
        upper.time_seconds > lower.time_seconds) {
        const float span_seconds = upper.time_seconds - lower.time_seconds;
        const float offset_seconds = sample_time_seconds - lower.time_seconds;
        const float ratio = offset_seconds / span_seconds;
        sampled_value = lower.value + ((upper.value - lower.value) * ratio);
    }

    out_value->target = track.target;
    out_value->channel = track.channel;
    out_value->value = sampled_value;
    return AnimationRuntimeStatus::Success;
}

AnimationRuntimeStatus AnimationRuntimeSampler::ValidateApplyRequest(
    const AnimationRuntimeTransformApplyRequest &request) const {
    if (request.transform_bridge == nullptr) {
        return AnimationRuntimeStatus::NullPointer;
    }

    if (request.sampled_values.empty()) {
        return AnimationRuntimeStatus::MissingSample;
    }

    for (const AnimationRuntimeSampledValue &value : request.sampled_values) {
        if (!value.target.IsValid()) {
            return AnimationRuntimeStatus::InvalidTarget;
        }

        if (!IsSupportedChannel(value.channel)) {
            return AnimationRuntimeStatus::UnsupportedChannel;
        }

        if (!std::isfinite(value.value)) {
            return AnimationRuntimeStatus::InvalidKeyframe;
        }

        const yuengine::world::WorldTransformResult query_result =
            request.transform_bridge->Query(value.target);
        const AnimationRuntimeStatus status = MapWorldStatus(query_result.status);
        if (status != AnimationRuntimeStatus::Success) {
            return status;
        }
    }

    return AnimationRuntimeStatus::Success;
}

AnimationRuntimeStatus AnimationRuntimeSampler::ApplyValue(
    yuengine::world::WorldTransformBridge &transform_bridge,
    const AnimationRuntimeSampledValue &value,
    yuengine::world::WorldTransformStatus *out_world_status) const {
    if (out_world_status == nullptr) {
        return AnimationRuntimeStatus::NullPointer;
    }

    const yuengine::world::WorldTransformResult query_result =
        transform_bridge.Query(value.target);
    AnimationRuntimeStatus status = MapWorldStatus(query_result.status);
    if (status != AnimationRuntimeStatus::Success) {
        *out_world_status = query_result.status;
        return status;
    }

    yuengine::world::WorldTransformState transform_state = query_result.transform_state;
    ApplyChannelValue(value.channel, value.value, &transform_state);
    const yuengine::world::WorldTransformStatus world_status =
        transform_bridge.Set(value.target, transform_state);
    *out_world_status = world_status;
    status = MapWorldStatus(world_status);
    if (status != AnimationRuntimeStatus::Success) {
        return status;
    }

    return AnimationRuntimeStatus::Success;
}

bool AnimationRuntimeSampler::IsFirstTargetOccurrence(
    std::span<const AnimationRuntimeSampledValue> values,
    std::size_t current_index) const {
    const AnimationRuntimeSampledValue &current = values[current_index];
    for (std::size_t index = 0U; index < current_index; ++index) {
        const AnimationRuntimeSampledValue &candidate = values[index];
        if (candidate.target.value == current.target.value) {
            return false;
        }
    }

    return true;
}
}
