#pragma once

#include "YuEngine/Package/PackageLoadPlan.h"
#include "YuEngine/Package/PackageStatus.h"

namespace yuengine::package {
struct PackageLoadPlanResult final {
    PackageStatus status;
    PackageLoadPlan plan;

    static PackageLoadPlanResult Success(PackageLoadPlan plan);
    static PackageLoadPlanResult Failure(PackageStatus status);
    bool Succeeded() const;
};
}
