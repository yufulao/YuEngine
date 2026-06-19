// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiNodeTreeStatus.h

#pragma once

namespace yuengine::uicore {
enum class UiNodeTreeStatus {
    Success,
    InvalidCapacity,
    InvalidNodeId,
    DuplicateNodeId,
    NodeNotFound,
    ParentNotFound,
    SelfParent,
    CycleRejected,
    CapacityExceeded,
    InvalidRect
};
}
