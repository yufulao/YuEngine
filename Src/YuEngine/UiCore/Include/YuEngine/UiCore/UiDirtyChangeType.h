// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiDirtyChangeType.h

#pragma once

namespace yuengine::uicore {
enum class UiDirtyChangeType {
    Layout,
    PaintOnly,
    Transform,
    HitTest,
    Text,
    HoverState,
    ScrollOffset,
    AtlasPage
};
}
