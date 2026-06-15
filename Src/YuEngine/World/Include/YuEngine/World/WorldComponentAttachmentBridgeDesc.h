// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentBridgeDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldConstants.h"

namespace yuengine::world {
struct WorldComponentAttachmentBridgeDesc final {
    std::uint32_t attachment_capacity = MAX_WORLD_OBJECT_COUNT;
};
}
