// Module: YuEngine Serialize
// File: Src/YuEngine/Serialize/Include/YuEngine/Serialize/SerializeSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Serialize/SerializeStatus.h"

namespace yuengine::serialize {
using yuengine::memory::MemoryAccountingStatus;

struct SerializeSnapshot final {
    std::uint16_t major_version;
    std::uint16_t minor_version;
    std::uint32_t committed_byte_count;
    std::uint32_t record_count;
    std::uint32_t field_count;
    std::uint32_t accepted_operation_count;
    std::uint32_t failed_operation_count;
    MemoryAccountingStatus allocation_accounting_status;
    SerializeStatus last_status;
};
}
