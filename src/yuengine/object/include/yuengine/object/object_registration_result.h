#pragma once

#include "yuengine/object/object_handle.h"
#include "yuengine/object/object_status.h"

namespace yuengine::object {
struct ObjectRegistrationResult final {
    ObjectStatus Status;
    ObjectHandle Handle;

    static ObjectRegistrationResult Success(ObjectHandle handle) {
        return ObjectRegistrationResult{ObjectStatus::Success, handle};
    }

    static ObjectRegistrationResult Failure(ObjectStatus status) {
        return ObjectRegistrationResult{status, ObjectHandle{}};
    }

    bool Succeeded() const {
        return Status == ObjectStatus::Success;
    }
};
}
