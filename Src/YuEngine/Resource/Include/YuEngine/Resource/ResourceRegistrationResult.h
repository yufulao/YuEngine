#pragma once

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceStatus.h"

namespace yuengine::resource {
struct ResourceRegistrationResult final {
    ResourceStatus status;
    ResourceHandle handle;

    static ResourceRegistrationResult Success(ResourceHandle handle) {
        return ResourceRegistrationResult{ResourceStatus::Success, handle};
    }

    static ResourceRegistrationResult Failure(ResourceStatus status) {
        return ResourceRegistrationResult{status, ResourceHandle{}};
    }

    bool Succeeded() const {
        return status == ResourceStatus::Success;
    }
};
}
