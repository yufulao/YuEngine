// 模块: YuEngine Package
// 文件: Src/YuEngine/Package/Include/YuEngine/Package/PackageEntryDescriptor.h

#pragma once

#include <cstdint>

#include "YuEngine/Package/PackageEntryId.h"
#include "YuEngine/Package/PackageId.h"
#include "YuEngine/Package/PackageSourceKey.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::package {
using resource::ResourceLogicalKey;
using resource::ResourceTypeId;

struct PackageEntryDescriptor final {
    PackageId package;
    PackageEntryId entry;
    ResourceTypeId type;
    ResourceLogicalKey logical_key;
    PackageSourceKey source_key;
    std::uint32_t byte_offset = 0U;
    std::uint32_t byte_size = 0U;
    std::uint64_t archive_byte_offset = 0ULL;
    std::uint64_t archive_byte_size = 0ULL;
    std::uint64_t payload_hash = 0ULL;
    std::uint64_t payload_logical_byte_count = 0ULL;
    std::uint64_t payload_window_byte_offset = 0ULL;
    std::uint64_t payload_window_byte_size = 0ULL;
};
}
