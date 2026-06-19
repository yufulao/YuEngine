// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiInvalidationStatus.h

#pragma once

namespace yuengine::uicore {
enum class UiInvalidationStatus {
    Success,
    InvalidOutput,
    NodeNotFound,
    OutputCapacityExceeded
};
}
