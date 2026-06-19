// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetRuntimeFixtureResult.h

#pragma once

#include <cstdint>

#include "YuEngine/Asset/AssetRecord.h"
#include "YuEngine/Asset/AssetRuntimeFixtureStatus.h"
#include "YuEngine/Asset/AssetSnapshot.h"
#include "YuEngine/Asset/AssetStatus.h"

namespace yuengine::asset {
struct AssetRuntimeFixtureResult final {
    AssetRuntimeFixtureStatus status = AssetRuntimeFixtureStatus::InvalidArgument;
    AssetStatus last_asset_status = AssetStatus::Success;
    AssetRecord root_record;
    AssetSnapshot asset_snapshot;
    std::uint32_t dependency_count = 0U;
    bool decoded_applied = false;
    bool texture_ready_applied = false;
    bool audio_ready_applied = false;
    bool resource_state_refreshed = false;
};
}
