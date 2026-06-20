// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiImageComponentStatus.h

#pragma once

namespace yuengine::uicore {
enum class UiImageComponentStatus {
    Success,
    InvalidOutputBuffer,
    InvalidDesc,
    NodeNotFound,
    SpriteNotFound,
    InvalidAtlasMetadata,
    InvalidNineSliceTarget,
    OutputCapacityExceeded
};
}
