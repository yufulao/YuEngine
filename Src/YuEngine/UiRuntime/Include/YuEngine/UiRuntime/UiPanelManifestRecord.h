// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiPanelManifestRecord.h

#pragma once

#include <array>
#include <cstdint>
#include <span>

#include "YuEngine/UiRuntime/UiPanelId.h"
#include "YuEngine/UiRuntime/UiPanelRegistryConstants.h"

namespace yuengine::uiruntime {
struct UiPanelLayoutRef final {
    std::uint32_t layout_asset_key = 0U;
    std::uint32_t layout_variant_key = 0U;

    bool IsValid() const {
        return layout_asset_key != 0U;
    }
};

struct UiPanelControllerRef final {
    std::uint32_t controller_type_key = 0U;

    bool IsValid() const {
        return controller_type_key != 0U;
    }
};

struct UiPanelResourceRef final {
    std::uint32_t resource_key = 0U;
    std::uint32_t resource_type_key = 0U;

    bool IsValid() const {
        if (resource_key == 0U) {
            return false;
        }

        return resource_type_key != 0U;
    }
};

struct UiPanelManifestRecord final {
    UiPanelId panel_id;
    UiPanelLayoutRef layout_ref;
    UiPanelControllerRef controller_ref;
    std::array<UiPanelResourceRef, MAX_UI_PANEL_RESOURCE_REF_COUNT> resource_refs{};
    std::uint32_t resource_ref_count = 0U;

    bool IsValid() const {
        if (!panel_id.IsValid()) {
            return false;
        }

        if (!layout_ref.IsValid()) {
            return false;
        }

        return controller_ref.IsValid();
    }
};

struct UiPanelTestManifest final {
    std::span<const UiPanelManifestRecord> records;
};
}
