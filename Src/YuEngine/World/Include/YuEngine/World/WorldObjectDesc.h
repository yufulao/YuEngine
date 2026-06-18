// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldObjectDesc.h

#pragma once

#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::world {
struct WorldObjectDesc final {
    WorldObjectId id{};
    bool is_enabled = true;
};
}
