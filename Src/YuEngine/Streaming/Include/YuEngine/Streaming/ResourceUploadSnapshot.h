// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceUploadSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Streaming/ResourceUploadKind.h"
#include "YuEngine/Streaming/ResourceUploadStatus.h"

namespace yuengine::streaming {
struct ResourceUploadSnapshot final {
    std::uint32_t request_capacity = 0U;
    std::uint32_t completion_capacity = 0U;
    std::uint32_t pending_count = 0U;
    std::uint32_t completion_count = 0U;
    std::uint32_t required_completion_count = 0U;
    std::uint32_t max_pending_count = 0U;
    std::uint32_t max_completion_count = 0U;
    std::uint64_t submitted_count = 0U;
    std::uint64_t completed_count = 0U;
    std::uint64_t failed_count = 0U;
    std::uint64_t rejected_count = 0U;
    std::uint64_t duplicate_upload_count = 0U;
    std::uint64_t rhi_upload_failed_count = 0U;
    ResourceUploadStatus last_status = ResourceUploadStatus::Success;
    resource::ResourceStatus last_resource_status = resource::ResourceStatus::Success;
    rhi::RhiStatus last_rhi_status = rhi::RhiStatus::Success;
    memory::MemoryAccountingStatus allocation_accounting_status =
        memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    std::uint64_t last_failed_upload_id = 0U;
    ResourceUploadKind last_failed_upload_kind = ResourceUploadKind::Unsupported;
    resource::ResourceHandle last_failed_upload_resource;
    resource::ResourceTypeId last_failed_upload_expected_type;
    std::uint32_t last_failed_upload_request_capacity = 0U;
    std::uint32_t last_failed_upload_completion_capacity = 0U;
    std::uint32_t last_required_upload_request_count = 0U;
    std::uint32_t last_required_upload_completion_count = 0U;
};
}
