// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/FileSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>

#include "YuEngine/File/FileStatus.h"
#include "YuEngine/File/MountId.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"

namespace yuengine::file {
struct FileSnapshot {
    std::uint64_t path_normalization_count;
    std::uint64_t rejected_path_count;
    std::uint64_t mount_count;
    MountId last_failed_mount_id;
    std::filesystem::path last_failed_mount_root_path;
    std::size_t last_failed_mount_capacity;
    std::size_t last_failed_mount_count;
    std::size_t last_required_mount_count;
    std::uint64_t lookup_count;
    std::uint64_t read_byte_count;
    std::uint64_t write_byte_count;
    std::size_t max_fixture_path_length;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status;
    FileStatus last_status;
    FileStatus last_read_status;
    FileStatus last_write_status;
};
}
