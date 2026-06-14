#pragma once

#include "yuengine/package/PackageEntryId.h"
#include "yuengine/package/PackageId.h"

namespace yuengine::package {
struct DependencyEdge final {
    PackageId package{};
    PackageEntryId dependent{};
    PackageEntryId dependency{};
    bool is_active = false;
};
}
