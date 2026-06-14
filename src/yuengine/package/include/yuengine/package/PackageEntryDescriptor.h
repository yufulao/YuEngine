#pragma once

#include <cstdint>

#include "yuengine/package/PackageEntryId.h"
#include "yuengine/package/PackageId.h"
#include "yuengine/package/PackageSourceKey.h"
#include "yuengine/resource/ResourceLogicalKey.h"
#include "yuengine/resource/ResourceTypeId.h"

namespace yuengine::package
{
using resource::ResourceLogicalKey;
using resource::ResourceTypeId;

struct PackageEntryDescriptor final
{
    PackageId Package;
    PackageEntryId Entry;
    ResourceTypeId Type;
    ResourceLogicalKey LogicalKey;
    PackageSourceKey SourceKey;
    std::uint32_t ByteOffset = 0U;
    std::uint32_t ByteSize = 0U;
};
}
