// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectTimelineTransformSampleRequest.h

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

struct RuntimeAssetWorldObjectTimelineTransformSampleRequest final {
    const yuengine::runtimeasset::RuntimeAssetRuntimeInstanceMappingRecord *runtime_instance_mappings = nullptr;
    std::uint32_t runtime_instance_mapping_count = 0U;
    const RuntimeAssetWorldObjectAdapterIdentityRecord *identity_records = nullptr;
    std::uint32_t identity_record_count = 0U;
    yuengine::world::WorldTransformBridge *transform_destination = nullptr;
    std::uint32_t clip_id = 0U;
    std::span<const yuengine::animation::AnimationRuntimeClipRecord> clips{};
    std::span<const yuengine::animation::AnimationRuntimeTrackRecord> tracks{};
    std::span<const yuengine::animation::AnimationRuntimeKeyframeRecord> keyframes{};
    yuengine::kernel::RuntimeFrameContext frame_context{};
    std::uint64_t clip_start_time_nanoseconds = 0U;
    yuengine::animation::AnimationRuntimeSampledValue *sampled_value_scratch = nullptr;
    std::uint32_t sampled_value_scratch_capacity = 0U;
    yuengine::animation::AnimationRuntimeSampledValue *sampled_value_output = nullptr;
    std::uint32_t sampled_value_output_capacity = 0U;
};
}
