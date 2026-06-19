// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetRuntimeFixtureSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Asset/AssetLoadState.h"
#include "YuEngine/Asset/AssetRuntimeFixtureStatus.h"
#include "YuEngine/Asset/AssetStatus.h"

namespace yuengine::asset {
struct AssetRuntimeFixtureSnapshot final {
    std::uint32_t executed_count = 0U;
    std::uint32_t rejected_count = 0U;
    std::uint32_t dependency_traversal_count = 0U;
    AssetRuntimeFixtureStatus last_status = AssetRuntimeFixtureStatus::Success;
    AssetStatus last_asset_status = AssetStatus::Success;
    AssetLoadState last_root_state = AssetLoadState::Unloaded;
};
}
