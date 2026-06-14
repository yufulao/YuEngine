#pragma once

#include <cstdint>

#include "yuengine/package/package_constants.h"

namespace yuengine::package {
struct PackageRegistryDesc final {
    std::uint32_t ManifestCapacity = MAX_PACKAGE_MANIFEST_COUNT;
    std::uint32_t EntryCapacity = MAX_PACKAGE_ENTRY_COUNT;
    std::uint32_t DependencyEdgeCapacity = MAX_PACKAGE_DEPENDENCY_EDGE_COUNT;
    std::uint32_t LoadPlanRecordCapacity = MAX_LOAD_PLAN_RECORD_COUNT;
};
}
