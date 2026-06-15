// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/World/WorldComponentAttachmentStatus.h"

namespace yuengine::world {
struct WorldComponentAttachmentSnapshot final {
    std::uint32_t attachment_capacity = 0U;
    std::uint32_t active_attachment_count = 0U;
    std::uint64_t added_attachment_count = 0U;
    std::uint64_t removed_attachment_count = 0U;
    std::uint64_t cleared_attachment_count = 0U;
    std::uint64_t query_count = 0U;
    std::uint32_t duplicate_rejection_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    WorldComponentAttachmentStatus last_status = WorldComponentAttachmentStatus::Success;
};
}
