// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/MaterialBindingFixtureStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment Defines explicit material binding fixture status values.
 */
enum class MaterialBindingFixtureStatus {
    Success,
    InvalidArgument,
    InvalidMaterialId,
    InvalidPipeline,
    InvalidTextureBinding,
    InvalidSamplerBinding,
    OversizedConstants,
    DuplicateMaterialId,
    BindingCapacityExceeded,
    RenderFixturePassFailed
};
}
