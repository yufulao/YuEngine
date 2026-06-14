#include "YuEngine/Package/PackageLoadPlanResult.h"

#include <utility>

namespace yuengine::package {
PackageLoadPlanResult PackageLoadPlanResult::Success(PackageLoadPlan plan) {
    return PackageLoadPlanResult{PackageStatus::Success, std::move(plan)};
}

PackageLoadPlanResult PackageLoadPlanResult::Failure(PackageStatus status) {
    return PackageLoadPlanResult{status, PackageLoadPlan{}};
}

bool PackageLoadPlanResult::Succeeded() const {
    return Status == PackageStatus::Success;
}
}
