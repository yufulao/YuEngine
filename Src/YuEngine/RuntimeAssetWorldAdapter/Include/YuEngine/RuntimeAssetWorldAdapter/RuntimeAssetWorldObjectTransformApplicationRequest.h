// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectTransformApplicationRequest.h

#pragma once

#include <cstdint>

namespace yuengine::animation {
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

struct RuntimeAssetWorldObjectTransformApplicationRequest final {
    const yuengine::runtimeasset::RuntimeAssetRuntimeInstanceMappingRecord *runtime_instance_mappings = nullptr;
    std::uint32_t runtime_instance_mapping_count = 0U;
    const RuntimeAssetWorldObjectAdapterIdentityRecord *identity_records = nullptr;
    std::uint32_t identity_record_count = 0U;
    yuengine::world::WorldTransformBridge *transform_destination = nullptr;
    const yuengine::animation::AnimationRuntimeSampledValue *sampled_values = nullptr;
    std::uint32_t sampled_value_count = 0U;
};
}
