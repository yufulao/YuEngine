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
