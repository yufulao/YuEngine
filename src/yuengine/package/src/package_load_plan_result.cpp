#include "yuengine/package/package_load_plan_result.h"

#include <utility>

namespace yuengine::package {
package_load_plan_result_t package_load_plan_result_t::Success(package_load_plan_t plan) {
    return package_load_plan_result_t{PACKAGE_STATUS::Success, std::move(plan)};
}

package_load_plan_result_t package_load_plan_result_t::Failure(PACKAGE_STATUS status) {
    return package_load_plan_result_t{status, package_load_plan_t{}};
}

bool package_load_plan_result_t::Succeeded() const {
    return Status == PACKAGE_STATUS::Success;
}
}
