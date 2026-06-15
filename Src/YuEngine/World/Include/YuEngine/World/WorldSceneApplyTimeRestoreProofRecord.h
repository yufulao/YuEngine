// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneApplyTimeRestoreProofRecord.h

#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldSceneApplyTimeRestoreProofStatus.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanStatus.h"

namespace yuengine::world {
enum class WorldSceneApplyTimeRestoreProofFamily {
    None,
    Identity,
    Transform,
    Attachment,
    Binding
};

struct WorldSceneApplyTimeRestoreProofRecord final {
    WorldSceneApplyTimeRestoreProofFamily family =
        WorldSceneApplyTimeRestoreProofFamily::None;
    std::uint32_t plan_index = 0U;
    std::uint32_t input_index = 0U;
    WorldObjectId world_object_id{};
    yuengine::object::ObjectHandle object_handle{};
    WorldComponentTypeId component_type_id{};
    WorldComponentSlotId component_slot_id{};
    yuengine::resource::ResourceHandle resource_handle{};
    yuengine::resource::ResourceTypeId expected_resource_type{};
    std::uint32_t projected_object_acquire_count = 0U;
    std::uint32_t projected_resource_acquire_count = 0U;
    WorldSceneDecodedRestorePlanStatus plan_status =
        WorldSceneDecodedRestorePlanStatus::Success;
    WorldSceneApplyTimeRestoreProofStatus status =
        WorldSceneApplyTimeRestoreProofStatus::Success;
};
}
