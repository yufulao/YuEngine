// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneDecodedRestorePlanSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/World/WorldComponentAttachmentStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingStatus.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"
#include "YuEngine/World/WorldSceneDecodedRestorePlanStatus.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::world {
struct WorldSceneDecodedRestorePlanSnapshot final {
    std::uint32_t identity_capacity = 0U;
    std::uint32_t transform_capacity = 0U;
    std::uint32_t attachment_capacity = 0U;
    std::uint32_t binding_capacity = 0U;
    std::uint32_t plan_capacity = 0U;
    std::uint64_t plan_attempt_count = 0U;
    std::uint64_t planned_identity_count = 0U;
    std::uint64_t planned_transform_count = 0U;
    std::uint64_t planned_attachment_count = 0U;
    std::uint64_t planned_binding_count = 0U;
    std::uint64_t rejected_record_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    WorldObjectIdentityStatus last_identity_status = WorldObjectIdentityStatus::Success;
    WorldTransformStatus last_transform_status = WorldTransformStatus::Success;
    WorldComponentAttachmentStatus last_attachment_status =
        WorldComponentAttachmentStatus::Success;
    WorldComponentResourceBindingStatus last_binding_status =
        WorldComponentResourceBindingStatus::Success;
    yuengine::object::ObjectStatus last_object_status =
        yuengine::object::ObjectStatus::Success;
    yuengine::resource::ResourceStatus last_resource_status =
        yuengine::resource::ResourceStatus::Success;
    WorldSceneDecodedRestorePlanStatus last_status =
        WorldSceneDecodedRestorePlanStatus::Success;
};
}
