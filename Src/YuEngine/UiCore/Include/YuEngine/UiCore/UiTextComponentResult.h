// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiTextComponentResult.h

#pragma once

#include <cstdint>

#include "YuEngine/UiCore/UiDirtyChangeType.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiTextComponentDesc.h"
#include "YuEngine/UiCore/UiTextComponentStatus.h"

namespace yuengine::uicore {
struct UiTextComponentResult final {
    UiTextComponentStatus status = UiTextComponentStatus::InvalidOutputBuffer;
    UiNodeId node_id;
    UiTextSourceType source_type = UiTextSourceType::PlainText;
    std::uint32_t text_key = 0U;
    std::uint32_t localize_key = 0U;
    std::uint32_t draw_record_count = 0U;
    std::uint32_t required_draw_record_count = 0U;
    std::uint32_t visible_codepoint_count = 0U;
    std::uint32_t source_codepoint_count = 0U;
    std::uint32_t line_count = 0U;
    std::uint32_t missing_codepoint = 0U;
    bool overflowed = false;
    UiDirtyChangeType dirty_change_type = UiDirtyChangeType::Text;

    /**
     * @comment 检查 Text component 输出是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiTextComponentStatus::Success;
    }
};
}
