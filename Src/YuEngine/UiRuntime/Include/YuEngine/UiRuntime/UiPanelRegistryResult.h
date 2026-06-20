// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiPanelRegistryResult.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/UiPanelManifestRecord.h"
#include "YuEngine/UiRuntime/UiPanelRegistryStatus.h"

namespace yuengine::uiruntime {
struct UiPanelRegistryResult final {
    UiPanelRegistryStatus status = UiPanelRegistryStatus::InvalidDesc;
    UiPanelManifestRecord record;
    std::uint32_t record_index = 0U;
    std::uint32_t required_record_count = 0U;

    /**
     * @comment 检查 panel registry 操作是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiPanelRegistryStatus::Success;
    }
};
}
