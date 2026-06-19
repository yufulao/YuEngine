// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldIdentityBaselineSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/World/WorldComponentAttachmentStatus.h"
#include "YuEngine/World/WorldIdentityBaselineStatus.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"
#include "YuEngine/World/WorldStatus.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::world {
struct WorldIdentityBaselineSnapshot final {
    std::uint32_t record_capacity = 0U;
    std::uint32_t active_record_count = 0U;
    std::uint64_t created_record_count = 0U;
    std::uint64_t destroyed_record_count = 0U;
    std::uint64_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    yuengine::object::ObjectStatus last_object_status = yuengine::object::ObjectStatus::Success;
    WorldStatus last_world_status = WorldStatus::Success;
    WorldObjectIdentityStatus last_identity_status = WorldObjectIdentityStatus::Success;
    WorldTransformStatus last_transform_status = WorldTransformStatus::Success;
    WorldComponentAttachmentStatus last_component_status = WorldComponentAttachmentStatus::Success;
    WorldIdentityBaselineStatus last_status = WorldIdentityBaselineStatus::Success;
};
}
