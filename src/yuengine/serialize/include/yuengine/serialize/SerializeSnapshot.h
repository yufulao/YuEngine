#pragma once

#include <cstdint>

#include "yuengine/memory/MemoryAccountingStatus.h"
#include "yuengine/serialize/SerializeStatus.h"

namespace yuengine::serialize
{
using yuengine::memory::MemoryAccountingStatus;

struct SerializeSnapshot final
{
    std::uint16_t MajorVersion;
    std::uint16_t MinorVersion;
    std::uint32_t CommittedByteCount;
    std::uint32_t RecordCount;
    std::uint32_t FieldCount;
    std::uint32_t AcceptedOperationCount;
    std::uint32_t FailedOperationCount;
    MemoryAccountingStatus AllocationAccountingStatus;
    SerializeStatus LastStatus;
};
}
