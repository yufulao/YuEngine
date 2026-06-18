// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiTextureDesc.h

#pragma once

#include "YuEngine/Rhi/RhiExtent2D.h"
#include "YuEngine/Rhi/RhiFormat.h"

namespace yuengine::rhi {
struct RhiTextureDesc final {
    RhiFormat format = RhiFormat::Rgba8Unorm;
    RhiExtent2D extent{};
};
}
