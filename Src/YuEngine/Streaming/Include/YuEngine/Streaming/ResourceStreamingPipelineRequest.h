// Module: YuEngine Streaming
// File: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceStreamingPipelineRequest.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/File/AsyncFileReadRequest.h"
#include "YuEngine/Package/PackageLoadPlanRecord.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiFenceHandle.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Streaming/ResourceUploadKind.h"

namespace yuengine::file {
class AsyncFileReadQueue;
}

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::rhi {
class IRhiDevice;
}

namespace yuengine::streaming {
struct ResourceStreamingPipelineRequest final {
    resource::ResourceRegistry *resource_registry = nullptr;
    file::AsyncFileReadQueue *file_queue = nullptr;
    rhi::IRhiDevice *rhi_device = nullptr;
    package::PackageLoadPlanRecord package_record;
    resource::ResourceHandle resource;
    resource::ResourceTypeId expected_type;
    file::AsyncFileReadRequest file_request;
    std::span<const std::uint8_t> staged_bytes;
    std::uint32_t upload_byte_offset = 0U;
    std::uint32_t upload_byte_count = 0U;
    ResourceUploadKind upload_kind = ResourceUploadKind::Unsupported;
    rhi::RhiBufferDesc buffer_desc;
    rhi::RhiBufferHandle input_buffer_handle;
    rhi::RhiBufferHandle *output_buffer_handle = nullptr;
    rhi::RhiTextureDesc texture_desc;
    rhi::RhiTextureHandle input_texture_handle;
    rhi::RhiTextureHandle *output_texture_handle = nullptr;
    rhi::RhiFenceHandle *output_fence = nullptr;
    std::uint64_t staging_request_id = 0U;
    std::uint64_t upload_id = 0U;
    std::uint64_t commit_id = 0U;
};
}
