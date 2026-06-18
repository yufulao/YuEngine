// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentQueryDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::world {
class WorldComponentAttachmentBridge;

struct WorldComponentQueryTypeDesc final {
    const WorldComponentAttachmentBridge *source_bridge = nullptr;
    WorldComponentTypeId component_type_id{};
    WorldObjectId *output_world_object_ids = nullptr;
    std::uint32_t output_capacity = 0U;
};

struct WorldComponentQueryObjectDesc final {
    const WorldComponentAttachmentBridge *source_bridge = nullptr;
    WorldObjectId world_object_id{};
    WorldComponentAttachment *output_attachments = nullptr;
    std::uint32_t output_capacity = 0U;
};
}
