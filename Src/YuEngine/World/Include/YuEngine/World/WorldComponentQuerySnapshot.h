// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentQuerySnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/World/WorldComponentQueryDesc.h"
#include "YuEngine/World/WorldComponentQueryStatus.h"

namespace yuengine::world {
struct WorldComponentQuerySnapshot final {
    std::uint64_t query_count = 0U;
    std::uint64_t matched_record_count = 0U;
    std::uint32_t overflow_rejection_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    WorldComponentQueryKind last_failed_query_kind = WorldComponentQueryKind::None;
    WorldObjectId last_failed_world_object_id{};
    WorldComponentTypeId last_failed_component_type_id{};
    std::uint32_t last_failed_output_capacity = 0U;
    std::uint32_t last_required_output_count = 0U;
    WorldComponentQueryStatus last_status = WorldComponentQueryStatus::Success;
};
}
