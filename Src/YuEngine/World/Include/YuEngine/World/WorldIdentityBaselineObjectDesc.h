// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldIdentityBaselineObjectDesc.h

#pragma once

#include "YuEngine/Object/ObjectDescriptor.h"
#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

namespace yuengine::world {
struct WorldIdentityBaselineObjectDesc final {
    WorldObjectId world_object_id{};
    yuengine::object::ObjectDescriptor object_descriptor{};
    WorldComponentTypeId component_type_id{};
    WorldComponentSlotId component_slot_id{};
    WorldTransformState transform_state{};
    bool is_enabled = true;
};
}
