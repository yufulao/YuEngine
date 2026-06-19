// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldIdentityBaselineRecord.h

#pragma once

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

namespace yuengine::world {
struct WorldIdentityBaselineRecord final {
    WorldObjectId world_object_id{};
    yuengine::object::ObjectHandle object_handle{};
    WorldComponentTypeId component_type_id{};
    WorldComponentSlotId component_slot_id{};
    WorldTransformState transform_state{};
    bool is_active = false;
};
}
