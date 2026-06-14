#pragma once

#include "yuengine/rhi/rhi_color.h"
#include "yuengine/rhi/rhi_command_type.h"
#include "yuengine/rhi/rhi_texture_handle.h"

namespace yuengine::rhi {
struct RhiCommandRecord final {
    RhiCommandType Type = RhiCommandType::BeginFrame;
    RhiTextureHandle Target{};
    RhiColor Color{};
};
}
