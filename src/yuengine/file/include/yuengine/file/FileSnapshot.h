#pragma once

#include <cstddef>
#include <cstdint>

#include "yuengine/file/FileStatus.h"
#include "yuengine/memory/MemoryAccountingStatus.h"

namespace yuengine::file {
struct FileSnapshot {
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
