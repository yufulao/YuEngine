// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandRecord.h

#pragma once

#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiCommandType.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::rhi {
struct RhiCommandRecord final {
    RhiCommandType type = RhiCommandType::BeginFrame;
    RhiTextureHandle target{};
    RhiColor color{};
};
}
