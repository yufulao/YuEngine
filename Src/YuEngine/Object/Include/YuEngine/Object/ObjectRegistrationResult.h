#pragma once

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/Object/ObjectStatus.h"

namespace yuengine::object {
struct ObjectRegistrationResult final {
    ObjectStatus status;
    ObjectHandle handle;

    static ObjectRegistrationResult Success(ObjectHandle handle) {
        return ObjectRegistrationResult{ObjectStatus::Success, handle};
    }

    static ObjectRegistrationResult Failure(ObjectStatus status) {
        return ObjectRegistrationResult{status, ObjectHandle{}};
    }

    bool Succeeded() const {
        return status == ObjectStatus::Success;
    }
};
}
