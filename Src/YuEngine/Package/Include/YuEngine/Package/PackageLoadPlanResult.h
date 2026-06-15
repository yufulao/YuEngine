// Module: YuEngine Package
// File: Src/YuEngine/Package/Include/YuEngine/Package/PackageLoadPlanResult.h

#pragma once

#include "YuEngine/Package/PackageLoadPlan.h"
#include "YuEngine/Package/PackageStatus.h"

namespace yuengine::package {
struct PackageLoadPlanResult final {
    PackageStatus status;
    PackageLoadPlan plan;

    /**
     * @comment Creates a successful result.
     * @param plan Input plan.
     * @return Explicit operation result.
     */
    static PackageLoadPlanResult Success(PackageLoadPlan plan);
    /**
     * @comment Creates a failed result.
     * @param status Input status.
     * @return Explicit operation result.
     */
    static PackageLoadPlanResult Failure(PackageStatus status);
    /**
     * @comment Checks whether the result succeeded.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool Succeeded() const;
};
}
