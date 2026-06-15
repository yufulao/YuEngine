// Module: YuEngine Object
// File: Src/YuEngine/Object/Include/YuEngine/Object/ObjectRegistrationResult.h

#pragma once

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/Object/ObjectStatus.h"

namespace yuengine::object {
struct ObjectRegistrationResult final {
    ObjectStatus status;
    ObjectHandle handle;

    /**
     * @comment Creates a successful result.
     * @param handle Input handle.
     * @return Explicit operation result.
     */
    static ObjectRegistrationResult Success(ObjectHandle handle) {
        return ObjectRegistrationResult{ObjectStatus::Success, handle};
    }

    /**
     * @comment Creates a failed result.
     * @param status Input status.
     * @return Explicit operation result.
     */
    static ObjectRegistrationResult Failure(ObjectStatus status) {
        return ObjectRegistrationResult{status, ObjectHandle{}};
    }

    /**
     * @comment Checks whether the result succeeded.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool Succeeded() const {
        return status == ObjectStatus::Success;
    }
};
}
