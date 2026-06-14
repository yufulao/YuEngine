#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "yuengine/object/ObjectConstants.h"
#include "yuengine/object/ObjectDescriptor.h"
#include "yuengine/object/ObjectHandle.h"
#include "yuengine/object/ObjectRegistrationResult.h"
#include "yuengine/object/ObjectRegistryDesc.h"
#include "yuengine/object/ObjectSnapshot.h"
#include "yuengine/object/ObjectStatus.h"
#include "yuengine/object/ObjectTypeId.h"

namespace yuengine::object
{
class ObjectRegistry final
{
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
    struct ObjectSlot final
    {
        ObjectTypeId Type;
        std::uint32_t Generation = INVALID_OBJECT_GENERATION;
        std::uint32_t ReferenceCount = 0U;
        bool IsActive = false;
    };

    ObjectStatus RecordFailure(ObjectStatus status);
    void RecordSuccess();
    ObjectStatus ResolveHandle(ObjectHandle handle, std::size_t& outIndex) const;
    ObjectStatus RegisterTypeIfNeeded(ObjectTypeId type);
    bool HasType(ObjectTypeId type) const;
    void AdvanceGeneration(ObjectSlot& slot);

    std::array<ObjectSlot, MAX_OBJECT_COUNT> _slots;
    std::array<ObjectTypeId, MAX_OBJECT_TYPE_COUNT> _types;
    ObjectSnapshot _snapshot;
};
}
