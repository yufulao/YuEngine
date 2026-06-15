// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentSnapshotRecord.h

#pragma once

#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::world {
struct WorldComponentAttachmentSnapshotRecord final {
    WorldObjectId world_object_id{};
    WorldComponentTypeId component_type_id{};
    WorldComponentSlotId component_slot_id{};
};
}
