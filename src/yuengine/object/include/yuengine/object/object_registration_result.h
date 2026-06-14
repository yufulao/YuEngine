#pragma once

#include "yuengine/object/object_handle.h"
#include "yuengine/object/object_status.h"

namespace yuengine::object {
struct ObjectRegistrationResult final {
    OBJECT_STATUS Status;
    ObjectHandle Handle;

    static ObjectRegistrationResult Success(ObjectHandle handle) {
        return ObjectRegistrationResult{OBJECT_STATUS::Success, handle};
    }

    static ObjectRegistrationResult Failure(OBJECT_STATUS status) {
        return ObjectRegistrationResult{status, ObjectHandle{}};
    }

    bool Succeeded() const {
        return Status == OBJECT_STATUS::Success;
    }
};
}
