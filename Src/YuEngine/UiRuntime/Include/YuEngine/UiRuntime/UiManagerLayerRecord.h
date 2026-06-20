// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerLayerRecord.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/UiRuntime/UiManagerLayerId.h"
#include "YuEngine/UiRuntime/UiManagerLayerModelConstants.h"
#include "YuEngine/UiRuntime/UiManagerLayerType.h"

namespace yuengine::uiruntime {
struct UiManagerLayerRootRef final {
    std::uint32_t root_key = INVALID_UI_MANAGER_LAYER_ROOT_KEY;

    bool IsValid() const {
        return root_key != INVALID_UI_MANAGER_LAYER_ROOT_KEY;
    }
};

struct UiManagerLayerRecord final {
    UiManagerLayerId layer_id;
    UiManagerLayerType type = UiManagerLayerType::Game;
    std::int32_t order = 0;
    UiManagerLayerRootRef root_ref;

    bool IsValid() const {
        if (!layer_id.IsValid()) {
            return false;
        }

        return root_ref.IsValid();
    }
};

struct UiManagerLayerSet final {
    std::span<const UiManagerLayerRecord> records;
};
}
