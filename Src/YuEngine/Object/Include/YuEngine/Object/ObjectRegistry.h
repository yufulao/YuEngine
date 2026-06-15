// Module: YuEngine Object
// File: Src/YuEngine/Object/Include/YuEngine/Object/ObjectRegistry.h

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
    /**
     * @comment Constructs a ObjectRegistry instance.
     */
    ObjectRegistry();
    /**
     * @comment Constructs a ObjectRegistry instance.
     * @param desc Input descriptor.
     */
    explicit ObjectRegistry(ObjectRegistryDesc desc);

    /**
     * @comment Creates synthetic object.
     * @param descriptor Input descriptor.
     * @return Explicit operation result.
     */
    ObjectRegistrationResult CreateSyntheticObject(const ObjectDescriptor& descriptor);
    /**
     * @comment Validates the operation.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    ObjectStatus Validate(ObjectHandle handle);
    /**
     * @comment Acquires the operation.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    ObjectStatus Acquire(ObjectHandle handle);
    /**
     * @comment Releases the operation.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    ObjectStatus Release(ObjectHandle handle);
    /**
     * @comment Destroys the operation.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    ObjectStatus Destroy(ObjectHandle handle);
    /**
     * @comment Returns a snapshot of the current state.
     * @return Snapshot value.
     */
    ObjectSnapshot Snapshot() const;

private:
    ObjectStatus RecordFailure(ObjectStatus status);
    void RecordSuccess();
    ObjectStatus ResolveHandle(ObjectHandle handle, std::size_t& out_index) const;
    ObjectStatus RegisterTypeIfNeeded(ObjectTypeId type);
    bool HasType(ObjectTypeId type) const;
    void AdvanceGeneration(ObjectSlot& slot);

    std::array<ObjectSlot, MAX_OBJECT_COUNT> slots_;
    std::array<ObjectTypeId, MAX_OBJECT_TYPE_COUNT> types_;
    ObjectSnapshot snapshot_;
};
}
