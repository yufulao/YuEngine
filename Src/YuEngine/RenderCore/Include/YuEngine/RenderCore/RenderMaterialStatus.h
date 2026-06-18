// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderMaterialStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment Reports render material validation and build status.
 */
enum class RenderMaterialStatus {
    Success,
    InvalidArgument,
    InvalidMaterialId,
    InvalidProgramId,
    InvalidPipeline,
    InvalidTextureBinding,
    InvalidSamplerBinding,
    OversizedConstants,
    DuplicateMaterialId,
    MaterialCapacityExceeded
};
}
