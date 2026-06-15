// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceRegistrationResult.h

#pragma once

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceStatus.h"

namespace yuengine::resource {
struct ResourceRegistrationResult final {
    ResourceStatus status;
    ResourceHandle handle;

    /**
     * @comment Creates a successful result.
     * @param handle Input handle.
     * @return Explicit operation result.
     */
    static ResourceRegistrationResult Success(ResourceHandle handle) {
        return ResourceRegistrationResult{ResourceStatus::Success, handle};
    }

    /**
     * @comment Creates a failed result.
     * @param status Input status.
     * @return Explicit operation result.
     */
    static ResourceRegistrationResult Failure(ResourceStatus status) {
        return ResourceRegistrationResult{status, ResourceHandle{}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool Succeeded() const {
        return status == ResourceStatus::Success;
    }
};
}
