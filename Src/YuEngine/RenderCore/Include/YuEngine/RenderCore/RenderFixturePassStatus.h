// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFixturePassStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment Defines explicit fixture pass execution status values.
 */
enum class RenderFixturePassStatus {
    Success,
    InvalidArgument,
    InvalidTarget,
    InvalidPipeline,
    MissingVertexBuffer,
    MissingIndexBuffer,
    InvalidTextureBinding,
    InvalidSamplerBinding,
    InvalidDraw,
    InsufficientCaptureStorage,
    CommandCapacityExceeded,
    PassRecordCapacityExceeded,
    RhiFailure
};
}
