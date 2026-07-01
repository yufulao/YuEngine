// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerLayerModelSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/UiManagerLayerId.h"
#include "YuEngine/UiRuntime/UiManagerLayerModelResult.h"
#include "YuEngine/UiRuntime/UiManagerLayerModelStatus.h"
#include "YuEngine/UiRuntime/UiPanelId.h"

namespace yuengine::uiruntime {
struct UiManagerLayerModelSnapshot final {
    std::uint32_t layer_capacity = 0U;
    std::uint32_t binding_capacity = 0U;
    std::uint32_t registered_layer_count = 0U;
    std::uint32_t panel_binding_count = 0U;
    std::uint32_t accepted_operation_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    std::uint32_t rejected_operation_count = 0U;
    std::uint32_t duplicate_layer_rejected_count = 0U;
    std::uint32_t duplicate_panel_rejected_count = 0U;
    UiManagerLayerModelStatus last_status = UiManagerLayerModelStatus::Success;
    std::uint32_t required_layer_count = 0U;
    std::uint32_t required_binding_count = 0U;
    UiManagerLayerModelOperationKind last_failed_operation_kind = UiManagerLayerModelOperationKind::None;
    UiManagerLayerId last_failed_layer_id{};
    UiPanelId last_failed_panel_id{};
    std::uint32_t last_failed_record_index = 0U;
};
}
