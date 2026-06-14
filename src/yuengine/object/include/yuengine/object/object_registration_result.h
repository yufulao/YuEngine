#pragma once

#include "yuengine/object/object_handle.h"
#include "yuengine/object/object_status.h"

namespace yuengine::object {
struct object_registration_result_t final {
    OBJECT_STATUS Status;
    object_handle_t Handle;

    static object_registration_result_t Success(object_handle_t handle) {
        return object_registration_result_t{OBJECT_STATUS::Success, handle};
    }

    static object_registration_result_t Failure(OBJECT_STATUS status) {
        return object_registration_result_t{status, object_handle_t{}};
    }

    bool Succeeded() const {
        return Status == OBJECT_STATUS::Success;
    }
};
}
