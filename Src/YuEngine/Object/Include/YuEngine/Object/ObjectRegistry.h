#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/Object/ObjectConstants.h"
#include "YuEngine/Object/ObjectDescriptor.h"
#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/Object/ObjectRegistrationResult.h"
#include "YuEngine/Object/ObjectRegistryDesc.h"
#include "YuEngine/Object/ObjectSnapshot.h"
#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/Object/ObjectTypeId.h"
#include "YuEngine/Object/ObjectSlot.h"

namespace yuengine::object {
class ObjectRegistry final {
public:
    ObjectRegistry();
    explicit ObjectRegistry(ObjectRegistryDesc desc);

    ObjectRegistrationResult CreateSyntheticObject(const ObjectDescriptor& descriptor);
    ObjectStatus Validate(ObjectHandle handle);
    ObjectStatus Acquire(ObjectHandle handle);
    ObjectStatus Release(ObjectHandle handle);
    ObjectStatus Destroy(ObjectHandle handle);
    ObjectSnapshot Snapshot() const;

private:
    ObjectStatus RecordFailure(ObjectStatus status);
    void RecordSuccess();
    ObjectStatus ResolveHandle(ObjectHandle handle, std::size_t& outIndex) const;
    ObjectStatus RegisterTypeIfNeeded(ObjectTypeId type);
    bool HasType(ObjectTypeId type) const;
    void AdvanceGeneration(ObjectSlot& slot);

    std::array<ObjectSlot, MAX_OBJECT_COUNT> slots_;
    std::array<ObjectTypeId, MAX_OBJECT_TYPE_COUNT> types_;
    ObjectSnapshot snapshot_;
};
}
