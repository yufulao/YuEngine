#pragma once

#include "yuengine/package/package_load_plan.h"
#include "yuengine/package/package_status.h"

namespace yuengine::package {
struct package_load_plan_result_t final {
    PackageStatus Status;
    package_load_plan_t Plan;

    static package_load_plan_result_t Success(package_load_plan_t plan);
    static package_load_plan_result_t Failure(PackageStatus status);
    bool Succeeded() const;
};
}
