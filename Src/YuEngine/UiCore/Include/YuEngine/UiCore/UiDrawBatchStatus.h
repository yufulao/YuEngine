// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiDrawBatchStatus.h

#pragma once

namespace yuengine::uicore {
enum class UiDrawBatchStatus {
    Success,
    InvalidOutputBuffer,
    InvalidDrawElement,
    OutputCapacityExceeded
};
}
