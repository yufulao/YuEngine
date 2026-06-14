#pragma once

#include "yuengine/package/package_load_plan.h"
#include "yuengine/package/package_status.h"

namespace yuengine::package {
struct PackageLoadPlanResult final {
    PackageStatus Status;
    PackageLoadPlan Plan;

    static PackageLoadPlanResult Success(PackageLoadPlan plan);
    static PackageLoadPlanResult Failure(PackageStatus status);
    bool Succeeded() const;
};
}
