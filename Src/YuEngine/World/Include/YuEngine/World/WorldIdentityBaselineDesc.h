// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldIdentityBaselineDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectRegistryDesc.h"
#include "YuEngine/World/WorldComponentAttachmentBridgeDesc.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldDesc.h"
#include "YuEngine/World/WorldObjectIdentityBridgeDesc.h"
#include "YuEngine/World/WorldTransformBridgeDesc.h"

namespace yuengine::world {
struct WorldIdentityBaselineDesc final {
    WorldDesc world_desc{};
    yuengine::object::ObjectRegistryDesc object_registry_desc{};
    WorldObjectIdentityBridgeDesc identity_bridge_desc{};
    WorldTransformBridgeDesc transform_bridge_desc{};
    WorldComponentAttachmentBridgeDesc component_attachment_desc{};
    std::uint32_t record_capacity = MAX_WORLD_OBJECT_COUNT;
};
}
