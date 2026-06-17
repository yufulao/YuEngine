// Module: YuEngine Streaming
// File: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceUploadCompletion.h

#pragma once

#include <cstdint>

#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiFenceHandle.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Streaming/ResourceUploadKind.h"
#include "YuEngine/Streaming/ResourceUploadStatus.h"

namespace yuengine::streaming {
struct ResourceUploadCompletion final {
    ResourceUploadStatus status = ResourceUploadStatus::Success;
    ResourceUploadKind upload_kind = ResourceUploadKind::Unsupported;
    resource::ResourceHandle resource;
    resource::ResourceTypeId expected_type;
    std::uint64_t upload_id = 0U;
    std::uint64_t staging_request_id = 0U;
    std::uint32_t upload_byte_count = 0U;
    resource::ResourceStatus resource_status = resource::ResourceStatus::Success;
    rhi::RhiStatus rhi_status = rhi::RhiStatus::Success;
    rhi::RhiBufferHandle buffer_handle;
    rhi::RhiTextureHandle texture_handle;
    rhi::RhiFenceHandle fence;
};
}
