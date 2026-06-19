// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetRuntimeFixtureRequest.h

#pragma once

#include <span>

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportRecord.h"
#include "YuEngine/Resource/ResourceDecodedPayloadRecord.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeResult.h"

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::asset {
class AssetManager;

struct AssetRuntimeFixtureRequest final {
    AssetManager *manager = nullptr;
    yuengine::resource::ResourceRegistry *resource_registry = nullptr;
    AssetHandle root_asset;
    std::span<AssetHandle> dependency_output{};
    const yuengine::resource::ResourceDecodedPayloadRecord *decoded_payload = nullptr;
    const yuengine::streaming::ResourceDecodedTextureBridgeResult *texture_result = nullptr;
    const yuengine::audioresource::AudioResourcePcmPacketImportRecord *audio_record = nullptr;
    bool mark_loading = true;
    bool refresh_state_from_resource = false;
};
}
