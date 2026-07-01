// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Asset/AssetStatus.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportStatus.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceLoadCommitStatus.h"
#include "YuEngine/Resource/ResourceResidencyStatus.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeStatus.h"

namespace yuengine::asset {
struct AssetSnapshot final {
    std::uint32_t asset_capacity = 0U;
    std::uint32_t type_capacity = 0U;
    std::uint32_t dependency_edge_capacity = 0U;
    std::uint32_t type_count = 0U;
    std::uint32_t active_asset_count = 0U;
    std::uint32_t released_asset_count = 0U;
    std::uint32_t active_dependency_edge_count = 0U;
    std::uint32_t last_required_asset_count = 0U;
    std::uint32_t last_required_type_count = 0U;
    std::uint32_t last_required_dependency_edge_count = 0U;
    std::uint32_t texture_ready_count = 0U;
    std::uint32_t audio_ready_count = 0U;
    std::uint64_t registered_asset_count = 0U;
    std::uint64_t referenced_asset_count = 0U;
    std::uint64_t released_reference_count = 0U;
    std::uint32_t accepted_operation_count = 0U;
    std::uint32_t failed_operation_count = 0U;
    yuengine::memory::MemoryAccountingStatus allocation_accounting_status =
        yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly;
    AssetStatus last_status = AssetStatus::Success;
    yuengine::resource::ResourceStatus last_resource_status = yuengine::resource::ResourceStatus::Success;
    yuengine::resource::ResourceLoadCommitStatus last_resource_load_status =
        yuengine::resource::ResourceLoadCommitStatus::Success;
    yuengine::resource::ResourceResidencyStatus last_resource_residency_status =
        yuengine::resource::ResourceResidencyStatus::Success;
    yuengine::streaming::ResourceDecodedTextureBridgeStatus last_texture_status =
        yuengine::streaming::ResourceDecodedTextureBridgeStatus::Success;
    yuengine::audioresource::AudioResourcePcmPacketImportStatus last_audio_status =
        yuengine::audioresource::AudioResourcePcmPacketImportStatus::Success;
};
}
