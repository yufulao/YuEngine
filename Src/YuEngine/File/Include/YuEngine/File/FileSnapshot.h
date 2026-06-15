// Module: YuEngine File
// File: Src/YuEngine/File/Include/YuEngine/File/FileSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/File/FileStatus.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"

namespace yuengine::file {
struct FileSnapshot {
    std::uint64_t path_normalization_count;
    std::uint64_t rejected_path_count;
    std::uint64_t mount_count;
    std::uint64_t lookup_count;
    std::uint64_t read_byte_count;
    std::size_t max_fixture_path_length;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status;
    FileStatus last_read_status;
};
}
