// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerLayerModelResult.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/UiManagerLayerModelStatus.h"
#include "YuEngine/UiRuntime/UiManagerLayerRecord.h"
#include "YuEngine/UiRuntime/UiManagerPanelLayerBinding.h"

namespace yuengine::uiruntime {
struct UiManagerLayerModelResult final {
    UiManagerLayerModelStatus status = UiManagerLayerModelStatus::InvalidDesc;
    UiManagerLayerRecord layer_record;
    UiManagerPanelLayerBinding binding_record;
    std::uint32_t record_index = 0U;
    std::uint32_t required_layer_count = 0U;
    std::uint32_t required_binding_count = 0U;

    /**
     * @comment 检查 UIManager layer model 操作是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiManagerLayerModelStatus::Success;
    }
};
}
