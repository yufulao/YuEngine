#pragma once

#include "yuengine/object/ObjectHandle.h"
#include "yuengine/object/ObjectStatus.h"

namespace yuengine::object
{
struct ObjectRegistrationResult final
{
    ObjectStatus Status;
    ObjectHandle Handle;

    static ObjectRegistrationResult Success(ObjectHandle handle)
    {
        return ObjectRegistrationResult{ObjectStatus::Success, handle};
    }

    static ObjectRegistrationResult Failure(ObjectStatus status)
    {
        return ObjectRegistrationResult{status, ObjectHandle{}};
    }

    bool Succeeded() const
    {
        return Status == ObjectStatus::Success;
    }
};
}
