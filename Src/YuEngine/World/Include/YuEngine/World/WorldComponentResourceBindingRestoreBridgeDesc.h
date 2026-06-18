// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingRestoreBridgeDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldConstants.h"

namespace yuengine::world {
struct WorldComponentResourceBindingRestoreBridgeDesc final {
    std::uint32_t binding_capacity = MAX_WORLD_OBJECT_COUNT;
};
}
