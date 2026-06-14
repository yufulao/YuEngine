#pragma once

#include "yuengine/package/package_entry_id.h"
#include "yuengine/package/package_id.h"

namespace yuengine::package {
struct DependencyEdge final {
    PackageId package{};
    PackageEntryId dependent{};
    PackageEntryId dependency{};
    bool is_active = false;
};
}
