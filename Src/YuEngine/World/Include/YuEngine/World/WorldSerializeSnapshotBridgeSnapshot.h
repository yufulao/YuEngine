// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSerializeSnapshotBridgeSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/World/WorldSerializeSnapshotStatus.h"

namespace yuengine::world {
struct WorldSerializeSnapshotBridgeSnapshot final {
    std::uint32_t phase_trace_capacity = 0U;
    std::uint64_t written_snapshot_count = 0U;
    std::uint64_t written_trace_count = 0U;
    std::uint64_t written_transform_snapshot_count = 0U;
    std::uint64_t read_snapshot_count = 0U;
    std::uint64_t read_trace_count = 0U;
    std::uint64_t read_transform_snapshot_count = 0U;
    std::uint64_t skipped_optional_record_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    yuengine::serialize::SerializeStatus last_serialize_status =
        yuengine::serialize::SerializeStatus::Success;
    WorldSerializeSnapshotStatus last_status = WorldSerializeSnapshotStatus::Success;
};
}
