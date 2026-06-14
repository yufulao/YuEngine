#pragma once

#include <cstdint>

#include "yuengine/package/package_entry_id.h"
#include "yuengine/package/package_id.h"
#include "yuengine/package/package_source_key.h"
#include "yuengine/resource/resource_logical_key.h"
#include "yuengine/resource/resource_type_id.h"

namespace yuengine::package {
using resource::ResourceLogicalKey;
using resource::resource_type_id_t;

struct package_entry_descriptor_t final {
    package_id_t Package;
    package_entry_id_t Entry;
    resource_type_id_t Type;
    ResourceLogicalKey LogicalKey;
    PackageSourceKey SourceKey;
    std::uint32_t ByteOffset = 0U;
    std::uint32_t ByteSize = 0U;
};
}
