// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldObjectDesc.h

#pragma once

#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::world {
struct WorldObjectDesc final {
    WorldObjectId id{};
    bool is_enabled = true;
};
}
