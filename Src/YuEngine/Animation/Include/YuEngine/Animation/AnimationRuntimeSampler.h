// 模块: YuEngine Animation
// 文件: Src/YuEngine/Animation/Include/YuEngine/Animation/AnimationRuntimeSampler.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/Kernel/RuntimeFrameContext.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformBridge.h"
#include "YuEngine/World/WorldTransformState.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::animation {
constexpr std::uint32_t MAX_ANIMATION_RUNTIME_LAYER_COUNT = 1U;

enum class AnimationRuntimeStatus {
    Success,
    NullPointer,
    MissingClip,
    InvalidClip,
    MissingTrack,
    InvalidTrack,
    MissingKeyframe,
    InvalidKeyframe,
    InvalidTime,
    TimeOutOfRange,
    UnsupportedInterpolation,
    UnsupportedChannel,
    OutputCapacityExceeded,
    LayerCapacityExceeded,
    MissingSample,
    InvalidTarget,
    TargetNotFound,
    TransformApplyFailed
};

enum class AnimationRuntimeInterpolation {
    Step,
    Linear
};

enum class AnimationRuntimeChannel {
    TranslationX,
    TranslationY,
    TranslationZ,
    RotationX,
    RotationY,
    RotationZ,
    RotationW,
    ScaleX,
    ScaleY,
    ScaleZ
};

struct AnimationRuntimeClipRecord final {
    std::uint32_t clip_id = 0U;
    float duration_seconds = 0.0F;
    std::size_t first_track_index = 0U;
    std::size_t track_count = 0U;
    std::uint32_t layer_count = 1U;
    bool is_valid = false;
};

struct AnimationRuntimeTrackRecord final {
    std::uint32_t track_id = 0U;
    yuengine::world::WorldObjectId target{};
    AnimationRuntimeChannel channel = AnimationRuntimeChannel::TranslationX;
    AnimationRuntimeInterpolation interpolation = AnimationRuntimeInterpolation::Linear;
    std::size_t first_keyframe_index = 0U;
    std::size_t keyframe_count = 0U;
    bool is_valid = false;
};

struct AnimationRuntimeKeyframeRecord final {
    float time_seconds = 0.0F;
    float value = 0.0F;
    bool is_valid = false;
};

struct AnimationRuntimeSampledValue final {
    yuengine::world::WorldObjectId target{};
    AnimationRuntimeChannel channel = AnimationRuntimeChannel::TranslationX;
    float value = 0.0F;
};

struct AnimationRuntimeSampleRequest final {
    std::uint32_t clip_id = 0U;
    std::span<const AnimationRuntimeClipRecord> clips{};
    std::span<const AnimationRuntimeTrackRecord> tracks{};
    std::span<const AnimationRuntimeKeyframeRecord> keyframes{};
    yuengine::kernel::RuntimeFrameContext frame_context{};
    std::uint64_t clip_start_time_nanoseconds = 0U;
};

struct AnimationRuntimeSampleResult final {
    AnimationRuntimeStatus status = AnimationRuntimeStatus::Success;
    std::uint32_t clip_id = 0U;
    float sample_time_seconds = 0.0F;
    std::size_t sampled_value_count = 0U;
    std::size_t required_sampled_value_count = 0U;
    std::uint32_t required_layer_count = 0U;
    std::size_t failed_track_index = 0U;
    std::uint32_t failed_track_id = 0U;
    yuengine::world::WorldObjectId failed_target_id{};
    AnimationRuntimeChannel failed_channel = AnimationRuntimeChannel::TranslationX;
    std::uint32_t failed_layer_index = 0U;
};

struct AnimationRuntimeTransformApplyRequest final {
    yuengine::world::WorldTransformBridge *transform_bridge = nullptr;
    std::span<const AnimationRuntimeSampledValue> sampled_values{};
};

struct AnimationRuntimeTransformApplyResult final {
    AnimationRuntimeStatus status = AnimationRuntimeStatus::Success;
    std::size_t applied_value_count = 0U;
    std::size_t updated_object_count = 0U;
    yuengine::world::WorldTransformStatus last_world_status =
        yuengine::world::WorldTransformStatus::Success;
};

class AnimationRuntimeSampler final {
public:
    /**
     * @comment 使用 RuntimeFrameContext 的 fixed time 对 bounded runtime animation records 采样。
     * @param request 输入采样请求。
     * @param out_values 调用方持有的输出 value storage。
     * @param out_result 调用方持有的输出结果。
     * @return 显式采样状态。
     */
    AnimationRuntimeStatus Sample(
        const AnimationRuntimeSampleRequest &request,
        std::span<AnimationRuntimeSampledValue> out_values,
        AnimationRuntimeSampleResult *out_result) const;
    /**
     * @comment 将 sampled transform channel values 写入 WorldTransformBridge。
     * @param request 输入 transform apply 请求。
     * @param out_result 调用方持有的输出结果。
     * @return 显式 apply 状态。
     */
    AnimationRuntimeStatus ApplySampledTransform(
        const AnimationRuntimeTransformApplyRequest &request,
        AnimationRuntimeTransformApplyResult *out_result) const;

private:
    AnimationRuntimeStatus CalculateSampleTime(
        const AnimationRuntimeSampleRequest &request,
        float *out_sample_time_seconds) const;
    const AnimationRuntimeClipRecord *FindClip(
        const AnimationRuntimeSampleRequest &request) const;
    AnimationRuntimeStatus ValidateClip(
        const AnimationRuntimeSampleRequest &request,
        const AnimationRuntimeClipRecord &clip,
        float sample_time_seconds,
        std::span<AnimationRuntimeSampledValue> out_values) const;
    AnimationRuntimeStatus ValidateTrack(
        const AnimationRuntimeSampleRequest &request,
        const AnimationRuntimeTrackRecord &track,
        float sample_time_seconds) const;
    AnimationRuntimeStatus ValidateKeyframes(
        std::span<const AnimationRuntimeKeyframeRecord> keyframes,
        float sample_time_seconds) const;
    AnimationRuntimeStatus SampleTrack(
        const AnimationRuntimeTrackRecord &track,
        std::span<const AnimationRuntimeKeyframeRecord> keyframes,
        float sample_time_seconds,
        AnimationRuntimeSampledValue *out_value) const;
    AnimationRuntimeStatus ValidateApplyRequest(
        const AnimationRuntimeTransformApplyRequest &request) const;
    AnimationRuntimeStatus ApplyValue(
        yuengine::world::WorldTransformBridge &transform_bridge,
        const AnimationRuntimeSampledValue &value,
        yuengine::world::WorldTransformStatus *out_world_status) const;
    bool IsFirstTargetOccurrence(
        std::span<const AnimationRuntimeSampledValue> values,
        std::size_t current_index) const;
};
}
