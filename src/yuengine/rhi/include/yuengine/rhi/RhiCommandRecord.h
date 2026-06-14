#pragma once

#include "yuengine/rhi/RhiColor.h"
#include "yuengine/rhi/RhiCommandType.h"
#include "yuengine/rhi/RhiTextureHandle.h"

namespace yuengine::rhi {
struct RhiCommandRecord final {
    RhiCommandType Type = RhiCommandType::BeginFrame;
    RhiTextureHandle Target{};
    RhiColor Color{};
};
}
