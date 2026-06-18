// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceDecodedTextureBridgeResult.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceDecodedPayloadRecord.h"
#include "YuEngine/Resource/ResourceDecodedPayloadStatus.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeStatus.h"
#include "YuEngine/Streaming/ResourceUploadCompletion.h"
#include "YuEngine/Streaming/ResourceUploadStatus.h"

namespace yuengine::streaming {
struct ResourceDecodedTextureBridgeResult final {
    ResourceDecodedTextureBridgeStatus status = ResourceDecodedTextureBridgeStatus::InvalidArgument;
    resource::ResourceDecodedPayloadStatus decoded_payload_status =
        resource::ResourceDecodedPayloadStatus::Success;
    ResourceUploadStatus upload_status = ResourceUploadStatus::Success;
    rhi::RhiStatus rhi_status = rhi::RhiStatus::Success;
    resource::ResourceDecodedPayloadRecord decoded_payload_record;
    ResourceUploadCompletion upload_completion;
    rhi::RhiTextureHandle texture_handle;
    rhi::RhiSampledTextureBinding sampled_texture;
    std::uint32_t decoded_byte_count = 0U;
    std::uint32_t uploaded_byte_count = 0U;
};
}
