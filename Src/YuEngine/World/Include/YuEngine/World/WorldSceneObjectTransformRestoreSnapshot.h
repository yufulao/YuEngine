// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformRestoreSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreStatus.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::world {
struct WorldSceneObjectTransformRestoreSnapshot final {
    std::uint32_t identity_capacity = 0U;
    std::uint32_t transform_capacity = 0U;
    std::uint64_t restore_attempt_count = 0U;
    std::uint64_t restored_identity_count = 0U;
    std::uint64_t restored_transform_count = 0U;
    std::uint64_t rejected_record_count = 0U;
    std::uint32_t rollback_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    WorldObjectIdentityStatus last_identity_status = WorldObjectIdentityStatus::Success;
    WorldTransformStatus last_transform_status = WorldTransformStatus::Success;
    yuengine::object::ObjectStatus last_object_status = yuengine::object::ObjectStatus::Success;
    WorldSceneObjectTransformRestoreStatus last_status =
        WorldSceneObjectTransformRestoreStatus::Success;
};
}
