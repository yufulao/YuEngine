// Module: YuEngine Package
// File: Src/YuEngine/Package/Include/YuEngine/Package/DependencyEdge.h

#pragma once

#include "YuEngine/Package/PackageEntryId.h"
#include "YuEngine/Package/PackageId.h"

namespace yuengine::package {
struct DependencyEdge final {
    PackageId package{};
    PackageEntryId dependent{};
    PackageEntryId dependency{};
    bool is_active = false;
};
}
