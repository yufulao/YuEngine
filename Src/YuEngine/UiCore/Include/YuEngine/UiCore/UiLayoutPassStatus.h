// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiLayoutPassStatus.h

#pragma once

namespace yuengine::uicore {
enum class UiLayoutPassStatus {
    Success,
    InvalidTree,
    ContainerNotFound,
    InvalidGridColumnCount,
    InvalidItemSize,
    TreeMutationFailed
};
}
