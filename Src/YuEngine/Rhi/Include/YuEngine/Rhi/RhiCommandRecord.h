#pragma once

#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiCommandType.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::rhi {
struct RhiCommandRecord final {
    RhiCommandType Type = RhiCommandType::BeginFrame;
    RhiTextureHandle Target{};
    RhiColor Color{};
};
}
