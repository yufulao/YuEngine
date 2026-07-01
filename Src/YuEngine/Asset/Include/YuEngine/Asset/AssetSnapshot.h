// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Asset/AssetStatus.h"
#include "YuEngine/Asset/AssetTypeId.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportStatus.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceLoadCommitStatus.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceResidencyStatus.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/Resource/ResourceTypeId.h"
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
    std::uint64_t last_capacity_entry_asset_id = 0U;
    yuengine::resource::ResourceHandle last_capacity_entry_resource_handle;
    yuengine::resource::ResourceTypeId last_capacity_entry_resource_type;
    AssetTypeId last_capacity_entry_asset_type;
    AssetHandle last_capacity_entry_dependent_asset;
    AssetHandle last_capacity_entry_dependency_asset;
    std::uint32_t last_capacity_entry_asset_capacity = 0U;
    std::uint32_t last_capacity_entry_type_capacity = 0U;
    std::uint32_t last_capacity_entry_dependency_edge_capacity = 0U;
    std::uint32_t last_capacity_entry_asset_count = 0U;
    std::uint32_t last_capacity_entry_type_count = 0U;
    std::uint32_t last_capacity_entry_dependency_edge_count = 0U;
    AssetHandle last_failed_dependency_output_dependent{};
    std::uint32_t last_dependency_output_capacity = 0U;
    std::uint32_t last_required_dependency_output_count = 0U;
    AssetHandle last_failed_dependency_output_dependency{};
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
