// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCameraStatus.h

#pragma once

namespace yuengine::rendercore {
enum class RenderCameraStatus {
    Success,
    InvalidOutput,
    InvalidPose,
    InvalidProjection
};
}
