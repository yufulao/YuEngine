// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFixturePassStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment 定义 explicit fixture pass execution 状态 值.
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
