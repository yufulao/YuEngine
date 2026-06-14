#include "yuengine/package/package_load_plan_result.h"

#include <utility>

namespace yuengine::package {
PackageLoadPlanResult PackageLoadPlanResult::Success(PackageLoadPlan plan) {
    return PackageLoadPlanResult{PACKAGE_STATUS::Success, std::move(plan)};
}

PackageLoadPlanResult PackageLoadPlanResult::Failure(PACKAGE_STATUS status) {
    return PackageLoadPlanResult{status, PackageLoadPlan{}};
}

bool PackageLoadPlanResult::Succeeded() const {
    return Status == PACKAGE_STATUS::Success;
}
}
