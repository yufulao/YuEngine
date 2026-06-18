// Module: YuEngine Streaming
// File: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceDecodedTextureBridgeRequest.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/Resource/ResourceDecodedPayloadRequest.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::rhi {
class IRhiDevice;
}

namespace yuengine::streaming {
struct ResourceDecodedTextureBridgeRequest final {
    resource::ResourceRegistry *resource_registry = nullptr;
    rhi::IRhiDevice *rhi_device = nullptr;
    resource::ResourceDecodedPayloadRequest decoded_payload;
    std::span<std::uint8_t> scratch_bytes{};
    rhi::RhiTextureDesc texture_desc;
    rhi::RhiTextureHandle *output_texture_handle = nullptr;
    std::uint64_t staging_request_id = 0U;
    std::uint64_t upload_id = 0U;
    std::uint32_t sampled_texture_slot = 0U;
};
}
