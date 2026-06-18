// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCameraShaderConstantsStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment Reports camera shader constant write status.
 */
enum class RenderCameraShaderConstantsStatus {
    Success,
    InvalidArgument,
    InvalidFrame
};
}
