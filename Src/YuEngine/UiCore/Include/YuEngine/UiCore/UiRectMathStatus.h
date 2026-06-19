// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiRectMathStatus.h

#pragma once

namespace yuengine::uicore {
enum class UiRectMathStatus {
    Success,
    InvalidParentRect,
    InvalidAnchor,
    InvalidPivot,
    InvalidDpiScale,
    InvalidMargin,
    InvalidPadding,
    InvalidOutputRect
};
}
