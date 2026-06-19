// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneActiveRestoreGateRecord.h

#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldSceneActiveRestoreGateCleanupPolicy.h"
#include "YuEngine/World/WorldSceneActiveRestoreGateStatus.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofRecord.h"

namespace yuengine::world {
struct WorldSceneActiveRestoreGateRecord final {
    WorldSceneApplyTimeRestoreProofFamily family =
        WorldSceneApplyTimeRestoreProofFamily::None;
    WorldSceneActiveRestoreGateCleanupPolicy cleanup_policy =
        WorldSceneActiveRestoreGateCleanupPolicy::None;
    std::uint32_t gate_index = 0U;
    std::uint32_t plan_index = 0U;
    std::uint32_t input_index = 0U;
    WorldObjectId world_object_id{};
    yuengine::object::ObjectHandle object_handle{};
    WorldComponentTypeId component_type_id{};
    WorldComponentSlotId component_slot_id{};
    yuengine::resource::ResourceHandle resource_handle{};
    yuengine::resource::ResourceTypeId expected_resource_type{};
    WorldSceneActiveRestoreGateStatus status =
        WorldSceneActiveRestoreGateStatus::Success;
};
}
