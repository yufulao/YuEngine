// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterRequest.h

#pragma once

#include <cstdint>

#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"

namespace yuengine::runtimeassetworldadapter {
struct RuntimeAssetWorldObjectAdapterRequest final {
    const yuengine::runtimeasset::RuntimeAssetRuntimeInstanceMappingRecord *runtime_instance_mappings = nullptr;
    std::uint32_t runtime_instance_mapping_count = 0U;
    const yuengine::runtimeasset::RuntimeAssetSceneEntityRecord *scene_entities = nullptr;
    std::uint32_t scene_entity_count = 0U;
    const yuengine::runtimeasset::RuntimeAssetSceneTransformOutputRecord *scene_transforms = nullptr;
    std::uint32_t scene_transform_count = 0U;
    const RuntimeAssetWorldObjectAdapterIdentityRecord *identity_records = nullptr;
    std::uint32_t identity_record_count = 0U;
    yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord *output_identities = nullptr;
    std::uint32_t output_identity_capacity = 0U;
    yuengine::world::WorldSceneObjectTransformRestoreTransformRecord *output_transforms = nullptr;
    std::uint32_t output_transform_capacity = 0U;
};
}
