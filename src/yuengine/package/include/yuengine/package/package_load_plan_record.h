#pragma once

#include <cstdint>

#include "yuengine/package/package_entry_id.h"
#include "yuengine/package/package_id.h"
#include "yuengine/package/package_source_key.h"
#include "yuengine/resource/resource_logical_key.h"
#include "yuengine/resource/resource_type_id.h"

namespace yuengine::package {
using resource::ResourceLogicalKey;
using resource::ResourceTypeId;

struct PackageLoadPlanRecord final {
    PackageId Package;
    PackageEntryId Entry;
    ResourceTypeId Type;
    ResourceLogicalKey LogicalKey;
    PackageSourceKey SourceKey;
    std::uint32_t ByteOffset = 0U;
    std::uint32_t ByteSize = 0U;
};
}
