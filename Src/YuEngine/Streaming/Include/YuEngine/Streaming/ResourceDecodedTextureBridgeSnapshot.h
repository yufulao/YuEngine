// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceDecodedTextureBridgeSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceDecodedPayloadStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeStatus.h"
#include "YuEngine/Streaming/ResourceUploadStatus.h"

namespace yuengine::streaming {
struct ResourceDecodedTextureBridgeSnapshot final {
    std::uint64_t submitted_count = 0U;
    std::uint64_t completed_count = 0U;
    std::uint64_t failed_count = 0U;
    std::uint64_t rejected_count = 0U;
    std::uint32_t last_decoded_byte_count = 0U;
    std::uint32_t last_uploaded_byte_count = 0U;
    ResourceDecodedTextureBridgeStatus last_status = ResourceDecodedTextureBridgeStatus::Success;
    resource::ResourceDecodedPayloadStatus last_decoded_payload_status =
        resource::ResourceDecodedPayloadStatus::Success;
    ResourceUploadStatus last_upload_status = ResourceUploadStatus::Success;
    rhi::RhiStatus last_rhi_status = rhi::RhiStatus::Success;
    memory::MemoryAccountingStatus allocation_accounting_status =
        memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
};
}
