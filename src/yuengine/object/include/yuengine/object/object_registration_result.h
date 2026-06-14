#pragma once

#include "yuengine/object/object_handle.h"
#include "yuengine/object/object_status.h"

namespace yuengine::object {
struct object_registration_result_t final {
    ObjectStatus Status;
    object_handle_t Handle;

    static object_registration_result_t Success(object_handle_t handle) {
        return object_registration_result_t{ObjectStatus::Success, handle};
    }

    static object_registration_result_t Failure(ObjectStatus status) {
        return object_registration_result_t{status, object_handle_t{}};
    }

    bool Succeeded() const {
        return Status == ObjectStatus::Success;
    }
};
}
