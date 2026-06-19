// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiDrawListStatus.h

#pragma once

namespace yuengine::uicore {
enum class UiDrawListStatus {
    Success,
    InvalidOutputBuffer,
    NodeNotFound,
    OutputCapacityExceeded
};
}
