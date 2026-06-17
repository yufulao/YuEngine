// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiSwapchainResizeRequest.h

#pragma once

#include "YuEngine/Rhi/RhiExtent2D.h"

namespace yuengine::rhi {
struct RhiSwapchainResizeRequest final {
    RhiExtent2D extent{};
};
}
