#pragma once

#include "yuengine/resource/resource_handle.h"
#include "yuengine/resource/resource_status.h"

namespace yuengine::resource {
struct resource_registration_result_t final {
    ResourceStatus Status;
    resource_handle_t Handle;

    static resource_registration_result_t Success(resource_handle_t handle) {
        return resource_registration_result_t{ResourceStatus::Success, handle};
    }

    static resource_registration_result_t Failure(ResourceStatus status) {
        return resource_registration_result_t{status, resource_handle_t{}};
    }

    bool Succeeded() const {
        return Status == ResourceStatus::Success;
    }
};
}
