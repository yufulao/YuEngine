#pragma once

#include "yuengine/resource/resource_handle.h"
#include "yuengine/resource/resource_status.h"

namespace yuengine::resource {
struct resource_registration_result_t final {
    RESOURCE_STATUS Status;
    resource_handle_t Handle;

    static resource_registration_result_t Success(resource_handle_t handle) {
        return resource_registration_result_t{RESOURCE_STATUS::Success, handle};
    }

    static resource_registration_result_t Failure(RESOURCE_STATUS status) {
        return resource_registration_result_t{status, resource_handle_t{}};
    }

    bool Succeeded() const {
        return Status == RESOURCE_STATUS::Success;
    }
};
}
