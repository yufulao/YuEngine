// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneDecodedRestorePlanRecord.h

#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanStatus.h"

namespace yuengine::world {
enum class WorldSceneDecodedRestorePlanRecordFamily {
    None,
    Identity,
    Transform,
    Attachment,
    Binding
};

struct WorldSceneDecodedRestorePlanRecord final {
    WorldSceneDecodedRestorePlanRecordFamily family =
        WorldSceneDecodedRestorePlanRecordFamily::None;
    std::uint32_t input_index = 0U;
    WorldObjectId world_object_id{};
    yuengine::object::ObjectHandle object_handle{};
    WorldComponentTypeId component_type_id{};
    WorldComponentSlotId component_slot_id{};
    yuengine::resource::ResourceHandle resource_handle{};
    yuengine::resource::ResourceTypeId expected_resource_type{};
    std::uint32_t projected_object_acquire_count = 0U;
    std::uint32_t projected_resource_acquire_count = 0U;
    WorldSceneDecodedRestorePlanStatus status =
        WorldSceneDecodedRestorePlanStatus::Success;
};
}
