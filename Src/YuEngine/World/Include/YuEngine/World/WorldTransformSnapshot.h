// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldTransformSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::world {
struct WorldTransformSnapshot final {
    std::uint32_t bridge_capacity = 0U;
    std::uint32_t record_count = 0U;
    std::uint64_t updated_record_count = 0U;
    std::uint64_t removed_record_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    WorldTransformStatus last_status = WorldTransformStatus::Success;
};
}
