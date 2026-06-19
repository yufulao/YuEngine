// 模块: YuEngine UiRenderCoreBridge
// 文件: Src/YuEngine/UiRenderCoreBridge/Include/YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeStatus.h

#pragma once

namespace yuengine::uirendercorebridge {
/**
 * @comment 定义 UiCore draw element 到 RenderCore fixture bridge 的显式状态。
 */
enum class UiRenderCoreBridgeStatus {
    Success,
    InvalidArgument,
    EmptyDrawList,
    InvalidDrawElement,
    OutputCapacityExceeded,
    PassIdOverflow,
    SubmissionFailed
};
}
