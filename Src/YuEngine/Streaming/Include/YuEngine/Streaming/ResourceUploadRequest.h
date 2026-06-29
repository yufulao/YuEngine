// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceUploadRequest.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiFenceHandle.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Streaming/PackageResourceStagingCompletion.h"
#include "YuEngine/Streaming/ResourceUploadKind.h"

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::rhi {
class IRhiDevice;
}

namespace yuengine::streaming {
struct ResourceUploadRequest final {
    const resource::ResourceRegistry *resource_registry = nullptr;
    rhi::IRhiDevice *rhi_device = nullptr;
    PackageResourceStagingCompletion staging_completion;
    resource::ResourceHandle resource;
    resource::ResourceTypeId expected_type;
    std::span<const std::uint8_t> staged_bytes;
    std::uint32_t upload_byte_offset = 0U;
    std::uint32_t upload_byte_count = 0U;
    std::uint64_t destination_byte_offset = 0ULL;
    ResourceUploadKind upload_kind = ResourceUploadKind::Unsupported;
    rhi::RhiBufferDesc buffer_desc;
    rhi::RhiBufferHandle input_buffer_handle;
    rhi::RhiBufferHandle *output_buffer_handle = nullptr;
    rhi::RhiTextureDesc texture_desc;
    rhi::RhiTextureHandle input_texture_handle;
    rhi::RhiTextureHandle *output_texture_handle = nullptr;
    rhi::RhiFenceHandle *output_fence = nullptr;
    std::uint64_t upload_id = 0U;
};
}
