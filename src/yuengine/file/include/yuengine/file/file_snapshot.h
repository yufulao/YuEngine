#pragma once

#include <cstddef>
#include <cstdint>

#include "yuengine/file/file_status.h"
#include "yuengine/memory/memory_accounting_status.h"

namespace yuengine::file {
struct file_snapshot_t {
    std::uint64_t PathNormalizationCount;
    std::uint64_t RejectedPathCount;
    std::uint64_t MountCount;
    std::uint64_t LookupCount;
    std::uint64_t ReadByteCount;
    std::size_t MaxFixturePathLength;
    yuengine::memory::MemoryAccountingStatus AllocationAccountingStatus;
    FileStatus LastReadStatus;
};
}
