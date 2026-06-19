// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetRecord.h

#pragma once

#include <cstdint>

#include "YuEngine/Asset/AssetAudioReadyRecord.h"
#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Asset/AssetLoadState.h"
#include "YuEngine/Asset/AssetTextureReadyRecord.h"
#include "YuEngine/Asset/AssetTypeId.h"
#include "YuEngine/Resource/ResourceDecodedPayloadRecord.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"

namespace yuengine::asset {
struct AssetRecord final {
    AssetHandle handle;
    std::uint64_t stable_id = 0U;
    AssetTypeId asset_type;
    yuengine::resource::ResourceHandle resource;
    yuengine::resource::ResourceTypeId resource_type;
    AssetLoadState state = AssetLoadState::Unloaded;
    std::uint32_t reference_count = 0U;
    yuengine::resource::ResourceDecodedPayloadRecord decoded_payload;
    AssetTextureReadyRecord texture_ready;
    AssetAudioReadyRecord audio_ready;
    bool is_active = false;
};
}
