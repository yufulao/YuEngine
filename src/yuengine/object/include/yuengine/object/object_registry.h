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
    explicit ObjectRegistry(object_registry_desc_t desc);

    object_registration_result_t CreateSyntheticObject(const object_descriptor_t& descriptor);
    ObjectStatus Validate(object_handle_t handle);
    ObjectStatus Acquire(object_handle_t handle);
    ObjectStatus Release(object_handle_t handle);
    ObjectStatus Destroy(object_handle_t handle);
    object_snapshot_t Snapshot() const;

private:
    ObjectStatus RecordFailure(ObjectStatus status);
    void RecordSuccess();
    ObjectStatus ResolveHandle(object_handle_t handle, std::size_t& outIndex) const;
    ObjectStatus RegisterTypeIfNeeded(object_type_id_t type);
    bool HasType(object_type_id_t type) const;
    void AdvanceGeneration(object_slot_t& slot);

    std::array<object_slot_t, MAX_OBJECT_COUNT> _slots;
    std::array<object_type_id_t, MAX_OBJECT_TYPE_COUNT> _types;
    object_snapshot_t _snapshot;
};
}
