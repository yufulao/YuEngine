#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "yuengine/object/object_constants.h"
#include "yuengine/object/object_descriptor.h"
#include "yuengine/object/object_handle.h"
#include "yuengine/object/object_registration_result.h"
#include "yuengine/object/object_registry_desc.h"
#include "yuengine/object/object_snapshot.h"
#include "yuengine/object/object_status.h"
#include "yuengine/object/object_type_id.h"
#include "yuengine/object/object_slot.h"

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

    std::array<ObjectSlot, MAX_OBJECT_COUNT> _slots;
    std::array<ObjectTypeId, MAX_OBJECT_TYPE_COUNT> _types;
    ObjectSnapshot _snapshot;
};
}
