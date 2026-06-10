#pragma once

#include "yuengine/package/PackageLoadPlan.h"
#include "yuengine/package/PackageStatus.h"

namespace yuengine::package
{
struct PackageLoadPlanResult final
{
    PackageStatus Status;
    PackageLoadPlan Plan;

    static PackageLoadPlanResult Success(PackageLoadPlan plan);
    static PackageLoadPlanResult Failure(PackageStatus status);
    bool Succeeded() const;
};
}
