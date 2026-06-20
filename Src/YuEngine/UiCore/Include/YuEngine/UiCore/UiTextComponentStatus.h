// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiTextComponentStatus.h

#pragma once

namespace yuengine::uicore {
enum class UiTextComponentStatus {
    Success,
    InvalidOutputBuffer,
    InvalidDesc,
    NodeNotFound,
    InvalidFontGlyphAtlas,
    FontGlyphMissing,
    OutputCapacityExceeded,
    TextCapacityExceeded
};
}
