#pragma once

#include "YuEngine/Package/PackageLoadPlan.h"
#include "YuEngine/Package/PackageStatus.h"

namespace yuengine::package {
struct PackageLoadPlanResult final {
    PackageStatus Status;
    PackageLoadPlan Plan;

    static PackageLoadPlanResult Success(PackageLoadPlan plan);
    static PackageLoadPlanResult Failure(PackageStatus status);
    bool Succeeded() const;
};
}
