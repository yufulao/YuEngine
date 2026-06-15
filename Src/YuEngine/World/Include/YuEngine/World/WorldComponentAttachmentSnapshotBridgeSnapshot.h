// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentSnapshotBridgeSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotStatus.h"

namespace yuengine::world {
struct WorldComponentAttachmentSnapshotBridgeSnapshot final {
    std::uint32_t attachment_capacity = 0U;
    std::uint64_t write_count = 0U;
    std::uint64_t read_count = 0U;
    std::uint64_t written_record_count = 0U;
    std::uint64_t read_record_count = 0U;
    std::uint32_t rejected_record_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    yuengine::serialize::SerializeStatus last_serialize_status =
        yuengine::serialize::SerializeStatus::Success;
    WorldComponentAttachmentSnapshotStatus last_status =
        WorldComponentAttachmentSnapshotStatus::Success;
};
}
