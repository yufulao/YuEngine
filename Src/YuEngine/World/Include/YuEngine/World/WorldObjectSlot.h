// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldObjectSlot.h

#pragma once

#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::world {
struct WorldObjectSlot final {
    WorldObjectId id{};
    bool is_registered = false;
    bool is_enabled = false;
};
}
