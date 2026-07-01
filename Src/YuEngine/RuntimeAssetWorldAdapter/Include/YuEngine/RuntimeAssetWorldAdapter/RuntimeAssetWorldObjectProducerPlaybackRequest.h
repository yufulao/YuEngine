// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectProducerPlaybackRequest.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/Animation/AnimationRuntimeSampler.h"
#include "YuEngine/Kernel/RuntimeFrameContext.h"

namespace yuengine::runtimeasset {
struct RuntimeAssetRuntimeInstanceMappingRecord;
}

namespace yuengine::world {
class WorldTransformBridge;
}

namespace yuengine::runtimeassetworldadapter {
struct RuntimeAssetWorldObjectAdapterIdentityRecord;

struct RuntimeAssetWorldObjectProducerPlaybackRequest final {
    const yuengine::runtimeasset::RuntimeAssetRuntimeInstanceMappingRecord *runtime_instance_mappings = nullptr;
    std::uint32_t runtime_instance_mapping_count = 0U;
    const RuntimeAssetWorldObjectAdapterIdentityRecord *identity_records = nullptr;
    std::uint32_t identity_record_count = 0U;
    yuengine::world::WorldTransformBridge *transform_destination = nullptr;
    std::uint32_t export_clip_id = 0U;
    std::span<const yuengine::animation::AnimationRuntimeClipRecord> export_clips{};
    std::span<const yuengine::animation::AnimationRuntimeTrackRecord> export_tracks{};
    std::span<const yuengine::animation::AnimationRuntimeKeyframeRecord> export_keyframes{};
    yuengine::kernel::RuntimeFrameContext playback_frame_context{};
    std::uint64_t export_clip_start_time_nanoseconds = 0U;
    yuengine::animation::AnimationRuntimeSampledValue *sampled_value_scratch = nullptr;
    std::uint32_t sampled_value_scratch_capacity = 0U;
    yuengine::animation::AnimationRuntimeSampledValue *sampled_value_output = nullptr;
    std::uint32_t sampled_value_output_capacity = 0U;
};

struct RuntimeAssetWorldObjectProducerPlaybackBatchRequest final {
    const RuntimeAssetWorldObjectProducerPlaybackRequest *producer_playback_requests = nullptr;
    std::uint32_t producer_playback_request_count = 0U;
    yuengine::animation::AnimationRuntimeSampledValue *sampled_value_scratch = nullptr;
    std::uint32_t sampled_value_scratch_capacity = 0U;
    yuengine::animation::AnimationRuntimeSampledValue *sampled_value_output = nullptr;
    std::uint32_t sampled_value_output_capacity = 0U;
};
}
