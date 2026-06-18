// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCameraSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/RenderCore/RenderCameraStatus.h"

namespace yuengine::rendercore {
struct RenderCameraSnapshot final {
    std::uint32_t accepted_frame_count = 0U;
    std::uint32_t rejected_frame_count = 0U;
    RenderCameraStatus last_status = RenderCameraStatus::Success;
};
}
