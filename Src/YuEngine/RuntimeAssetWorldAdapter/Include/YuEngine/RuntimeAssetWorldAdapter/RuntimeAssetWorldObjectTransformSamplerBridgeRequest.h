// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectTransformSamplerBridgeRequest.h

#pragma once

#include <cstdint>

namespace yuengine::animation {
struct AnimationRuntimeSampleRequest;
struct AnimationRuntimeSampledValue;
}

namespace yuengine::runtimeasset {
struct RuntimeAssetRuntimeInstanceMappingRecord;
}

namespace yuengine::world {
class WorldTransformBridge;
}

namespace yuengine::runtimeassetworldadapter {
struct RuntimeAssetWorldObjectAdapterIdentityRecord;

struct RuntimeAssetWorldObjectTransformSamplerBridgeRequest final {
    const yuengine::runtimeasset::RuntimeAssetRuntimeInstanceMappingRecord *runtime_instance_mappings = nullptr;
    std::uint32_t runtime_instance_mapping_count = 0U;
    const RuntimeAssetWorldObjectAdapterIdentityRecord *identity_records = nullptr;
    std::uint32_t identity_record_count = 0U;
    yuengine::world::WorldTransformBridge *transform_destination = nullptr;
    const yuengine::animation::AnimationRuntimeSampleRequest *sample_request = nullptr;
    yuengine::animation::AnimationRuntimeSampledValue *sampled_value_output = nullptr;
    std::uint32_t sampled_value_output_capacity = 0U;
};
}
