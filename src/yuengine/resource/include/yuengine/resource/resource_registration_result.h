#pragma once

#include "yuengine/resource/resource_handle.h"
#include "yuengine/resource/resource_status.h"

namespace yuengine::resource {
struct ResourceRegistrationResult final {
    RESOURCE_STATUS Status;
    ResourceHandle Handle;

    static ResourceRegistrationResult Success(ResourceHandle handle) {
        return ResourceRegistrationResult{RESOURCE_STATUS::Success, handle};
    }

    static ResourceRegistrationResult Failure(RESOURCE_STATUS status) {
        return ResourceRegistrationResult{status, ResourceHandle{}};
    }

    bool Succeeded() const {
        return Status == RESOURCE_STATUS::Success;
    }
};
}
