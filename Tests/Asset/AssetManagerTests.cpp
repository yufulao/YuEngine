// 模块: Tests Asset
// 文件: Tests/Asset/AssetManagerTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

#include "YuEngine/Asset/AssetManager.h"
#include "YuEngine/Asset/AssetRuntimeFixture.h"
#include "YuEngine/Audio/AudioSampleFormat.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceDescriptor.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

using yuengine::asset::AssetDescriptor;
using yuengine::asset::AssetHandle;
using yuengine::asset::AssetLoadState;
using AssetManager = yuengine::asset::AssetManager;
using yuengine::asset::AssetManagerDesc;
using yuengine::asset::AssetRecord;
using yuengine::asset::AssetRegistrationResult;
using yuengine::asset::AssetRuntimeFixture;
using yuengine::asset::AssetRuntimeFixtureRequest;
using yuengine::asset::AssetRuntimeFixtureResult;
using yuengine::asset::AssetRuntimeFixtureSnapshot;
using yuengine::asset::AssetRuntimeFixtureStatus;
using yuengine::asset::AssetSnapshot;
using yuengine::asset::AssetStatus;
using yuengine::asset::AssetTypeId;
using yuengine::audio::AudioPcmSamplePacketRequest;
using yuengine::audio::AudioSampleFormat;
using yuengine::audioresource::AudioResourcePcmPacketImportHandle;
using yuengine::audioresource::AudioResourcePcmPacketImportRecord;
using yuengine::audioresource::AudioResourcePcmPacketImportStatus;
using yuengine::memory::MemoryAccountingStatus;
using yuengine::resource::ResourceDecodePlanAssetClass;
using yuengine::resource::ResourceDecodeResultClass;
using yuengine::resource::ResourceDecodedPayloadRecord;
using yuengine::resource::ResourceDecodedPayloadStatus;
using yuengine::resource::ResourceDescriptor;
using yuengine::resource::ResourceHandle;
using yuengine::resource::ResourceLoadCommitRequest;
using yuengine::resource::ResourceLoadCommitStatus;
using yuengine::resource::ResourceLoadState;
using yuengine::resource::ResourceLogicalKey;
using ResourceRegistry = yuengine::resource::ResourceRegistry;
using yuengine::resource::ResourceRegistrationResult;
using yuengine::resource::ResourceResidencyBudgetDesc;
using yuengine::resource::ResourceResidencyRequest;
using yuengine::resource::ResourceResidencyState;
using yuengine::resource::ResourceResidencyStatus;
using yuengine::resource::ResourceStatus;
using yuengine::resource::ResourceTypeId;
using yuengine::rhi::RhiSampledTextureBinding;
using yuengine::rhi::RhiTextureHandle;
using yuengine::streaming::ResourceDecodedTextureBridgeResult;
using yuengine::streaming::ResourceDecodedTextureBridgeStatus;

namespace {
constexpr const char *TEST_REGISTER = "Asset_RegisterRuntimeAsset_ReturnsStableHandleAndState";
constexpr const char *TEST_REGISTRATION_CAPACITY_ENTRY =
    "Asset_RegisterRuntimeAssetCapacityEntryPreservesRejectedIdentity";
constexpr const char *TEST_DEPENDENCIES = "Asset_DependenciesTraverseBoundedAndRejectCycle";
constexpr const char *TEST_DEPENDENCY_OUTPUT_CAPACITY_ENTRY =
    "Asset_DependencyOutputCapacityReportsEntryIdentity";
constexpr const char *TEST_TEXTURE_READY =
    "Asset_TextureReadyRecordUsesStreamingResultWithoutOwningDevice";
constexpr const char *TEST_AUDIO_READY =
    "Asset_AudioReadyRecordUsesImportRecordWithoutOwningDevice";
constexpr const char *TEST_REFRESH_STATE =
    "Asset_RefreshStateFromResourceMapsUploadedResidentAndFailed";
constexpr const char *TEST_RELEASE = "Asset_ReleaseRuntimeAssetReleasesResourceAndClearsReadyRecords";
constexpr const char *TEST_SUCCESS_LAST_STATUS = "Asset_ManagerSuccessClearsLastStatusAfterFailure";
constexpr const char *TEST_RUNTIME_FIXTURE =
    "Asset_RuntimeFixtureClosesResidentTexturePathAndDependencies";
constexpr const char *TEST_RUNTIME_FIXTURE_BOUNDARY =
    "Asset_RuntimeFixtureRejectsSmallDependencyOutputBeforeMutation";
constexpr const char *TEST_NO_UPPER_DEPENDENCY = "Asset_NoWorldGameAdapterUiDependency";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr AssetTypeId ASSET_TYPE_TEXTURE{1U};
constexpr AssetTypeId ASSET_TYPE_AUDIO{2U};
constexpr ResourceTypeId RESOURCE_TYPE_TEXTURE{11U};
constexpr ResourceTypeId RESOURCE_TYPE_AUDIO{12U};
using TestFunction = int (*)();

int Fail(const std::string &message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

ResourceRegistrationResult RegisterResource(
    ResourceRegistry &registry,
    std::uint32_t key_index,
    ResourceTypeId resource_type) {
    const std::string key = "asset_" + std::to_string(key_index);
    ResourceDescriptor descriptor{};
    descriptor.type = resource_type;
    descriptor.logical_key = ResourceLogicalKey(key);
    return registry.RegisterSyntheticDescriptor(descriptor);
}

AssetDescriptor MakeAssetDescriptor(
    std::uint64_t stable_id,
    AssetTypeId asset_type,
    ResourceHandle resource,
    ResourceTypeId resource_type) {
    AssetDescriptor descriptor{};
    descriptor.stable_id = stable_id;
    descriptor.asset_type = asset_type;
    descriptor.resource = resource;
    descriptor.resource_type = resource_type;
    return descriptor;
}

AssetRegistrationResult RegisterAsset(
    AssetManager &manager,
    ResourceRegistry &registry,
    std::uint64_t stable_id,
    AssetTypeId asset_type,
    ResourceHandle resource,
    ResourceTypeId resource_type) {
    const AssetDescriptor descriptor = MakeAssetDescriptor(stable_id, asset_type, resource, resource_type);
    return manager.RegisterRuntimeAsset(&registry, descriptor);
}

bool DoResourceHandlesMatch(ResourceHandle left, ResourceHandle right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

bool DoAssetHandlesMatch(AssetHandle left, AssetHandle right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

bool IsDependencyOutputCapacityEntryClear(const AssetSnapshot &snapshot) {
    if (snapshot.last_failed_dependency_output_dependent.IsValid()) {
        return false;
    }

    if (snapshot.last_dependency_output_capacity != 0U) {
        return false;
    }

    if (snapshot.last_required_dependency_output_count != 0U) {
        return false;
    }

    return !snapshot.last_failed_dependency_output_dependency.IsValid();
}

int ExpectSmallDependencyOutputFailure(
    AssetManager &manager,
    AssetHandle root,
    AssetHandle sentinel_dependency) {
    std::array<AssetHandle, 1U> small_dependencies{};
    small_dependencies[0U] = sentinel_dependency;
    std::uint32_t dependency_count = 123U;
    const AssetStatus status =
        manager.TraverseDependencies(root, small_dependencies.data(), 1U, &dependency_count);
    if (status != AssetStatus::OutputBufferTooSmall) {
        return Fail("small traversal output was not rejected");
    }

    if (dependency_count != 0U) {
        return Fail("small traversal failure did not clear output count");
    }

    if (!DoAssetHandlesMatch(small_dependencies[0U], sentinel_dependency)) {
        return Fail("small traversal failure mutated output dependency");
    }

    return 0;
}

ResourceDecodedPayloadRecord MakeDecodedPayloadRecord(
    ResourceHandle resource,
    ResourceTypeId resource_type,
    std::uint32_t decoded_byte_count) {
    ResourceDecodedPayloadRecord record{};
    record.resource = resource;
    record.expected_type = resource_type;
    record.payload_id = 101U;
    record.decode_plan_id = 201U;
    record.decode_result_id = 301U;
    record.decoded_payload_id = 401U;
    record.asset_class = ResourceDecodePlanAssetClass::Texture;
    record.result_class = ResourceDecodeResultClass::Texture;
    record.decoded_byte_count = decoded_byte_count;
    record.status = ResourceDecodedPayloadStatus::Success;
    record.is_active = true;
    return record;
}

ResourceDecodedTextureBridgeResult MakeTextureResult(ResourceHandle resource, ResourceTypeId resource_type) {
    ResourceDecodedTextureBridgeResult result{};
    result.status = ResourceDecodedTextureBridgeStatus::Success;
    result.decoded_payload_record = MakeDecodedPayloadRecord(resource, resource_type, 16U);
    result.texture_handle = RhiTextureHandle{3U, 1U};
    result.sampled_texture = RhiSampledTextureBinding{result.texture_handle, 2U};
    result.decoded_byte_count = 16U;
    result.uploaded_byte_count = 16U;
    return result;
}

AudioPcmSamplePacketRequest MakePacketRequest(std::uint32_t packet_id) {
    AudioPcmSamplePacketRequest request{};
    request.packet_id = packet_id;
    request.format = AudioSampleFormat::Signed16;
    request.sample_rate = 48000U;
    request.channel_count = 2U;
    request.frame_count = 4U;
    request.interleaved_sample_count = 8U;
    request.byte_count = 16U;
    return request;
}

AudioResourcePcmPacketImportRecord MakeAudioRecord(
    ResourceHandle resource,
    ResourceTypeId resource_type,
    std::uint32_t packet_id) {
    AudioResourcePcmPacketImportRecord record{};
    record.handle = AudioResourcePcmPacketImportHandle{1U, 1U};
    record.import_id = 501U;
    record.resource = resource;
    record.expected_type = resource_type;
    record.payload_id = 102U;
    record.decode_plan_id = 202U;
    record.decode_result_id = 302U;
    record.asset_class = ResourceDecodePlanAssetClass::Audio;
    record.result_class = ResourceDecodeResultClass::Audio;
    record.decoded_byte_count = 16U;
    record.packet_request = MakePacketRequest(packet_id);
    record.status = AudioResourcePcmPacketImportStatus::Success;
    record.is_active = true;
    return record;
}

int AssetRegisterRuntimeAssetReturnsStableHandleAndState() {
    ResourceRegistry registry;
    const ResourceRegistrationResult resource_result = RegisterResource(registry, 1U, RESOURCE_TYPE_TEXTURE);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    AssetManager manager;
    const AssetRegistrationResult asset_result = RegisterAsset(
        manager,
        registry,
        1001U,
        ASSET_TYPE_TEXTURE,
        resource_result.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!asset_result.Succeeded()) {
        return Fail("asset registration failed");
    }

    if (asset_result.handle.slot != 0U) {
        return Fail("asset used unexpected slot");
    }

    AssetRecord record{};
    if (manager.QueryAsset(asset_result.handle, &record) != AssetStatus::Success) {
        return Fail("asset query failed");
    }

    if (record.stable_id != 1001U) {
        return Fail("asset stable id was not recorded");
    }

    if (record.state != AssetLoadState::Unloaded) {
        return Fail("asset initial state was not unloaded");
    }

    if (registry.Snapshot().acquired_handle_count != 1U) {
        return Fail("asset registration did not acquire lower resource");
    }

    const AssetSnapshot snapshot = manager.Snapshot();
    if (snapshot.active_asset_count != 1U) {
        return Fail("asset snapshot did not report active asset");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("asset manager allocation vocabulary changed");
    }

    const ResourceRegistrationResult asset_capacity_resource =
        RegisterResource(registry, 16U, RESOURCE_TYPE_TEXTURE);
    const ResourceRegistrationResult asset_capacity_overflow_resource =
        RegisterResource(registry, 17U, RESOURCE_TYPE_AUDIO);
    if (!asset_capacity_resource.Succeeded() || !asset_capacity_overflow_resource.Succeeded()) {
        return Fail("asset capacity resource registration failed");
    }

    AssetManager asset_capacity_manager(AssetManagerDesc{1U, 2U, 1U});
    const AssetRegistrationResult first_capacity_asset = RegisterAsset(
        asset_capacity_manager,
        registry,
        1002U,
        ASSET_TYPE_TEXTURE,
        asset_capacity_resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!first_capacity_asset.Succeeded()) {
        return Fail("asset capacity fixture registration failed");
    }

    const AssetRegistrationResult asset_capacity_result = RegisterAsset(
        asset_capacity_manager,
        registry,
        1003U,
        ASSET_TYPE_AUDIO,
        asset_capacity_overflow_resource.handle,
        RESOURCE_TYPE_AUDIO);
    if (asset_capacity_result.status != AssetStatus::CapacityExceeded) {
        return Fail("asset capacity overflow did not return explicit status");
    }

    if (asset_capacity_result.required_asset_count != 2U ||
        asset_capacity_result.required_type_count != 2U ||
        asset_capacity_result.required_dependency_edge_count != 0U) {
        return Fail("asset capacity overflow did not report required counts");
    }

    AssetSnapshot capacity_snapshot = asset_capacity_manager.Snapshot();
    if (capacity_snapshot.last_required_asset_count != 2U ||
        capacity_snapshot.last_required_type_count != 2U ||
        capacity_snapshot.last_required_dependency_edge_count != 0U) {
        return Fail("asset capacity snapshot did not report required counts");
    }

    const ResourceRegistrationResult type_capacity_resource =
        RegisterResource(registry, 18U, RESOURCE_TYPE_TEXTURE);
    const ResourceRegistrationResult type_capacity_overflow_resource =
        RegisterResource(registry, 19U, RESOURCE_TYPE_AUDIO);
    const ResourceRegistrationResult type_capacity_recovery_resource =
        RegisterResource(registry, 20U, RESOURCE_TYPE_TEXTURE);
    if (!type_capacity_resource.Succeeded() ||
        !type_capacity_overflow_resource.Succeeded() ||
        !type_capacity_recovery_resource.Succeeded()) {
        return Fail("type capacity resource registration failed");
    }

    AssetManager type_capacity_manager(AssetManagerDesc{4U, 1U, 1U});
    const AssetRegistrationResult first_type_asset = RegisterAsset(
        type_capacity_manager,
        registry,
        1004U,
        ASSET_TYPE_TEXTURE,
        type_capacity_resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!first_type_asset.Succeeded()) {
        return Fail("type capacity fixture registration failed");
    }

    const AssetRegistrationResult type_capacity_result = RegisterAsset(
        type_capacity_manager,
        registry,
        1005U,
        ASSET_TYPE_AUDIO,
        type_capacity_overflow_resource.handle,
        RESOURCE_TYPE_AUDIO);
    if (type_capacity_result.status != AssetStatus::CapacityExceeded) {
        return Fail("asset type capacity overflow did not return explicit status");
    }

    if (type_capacity_result.required_asset_count != 2U ||
        type_capacity_result.required_type_count != 2U ||
        type_capacity_result.required_dependency_edge_count != 0U) {
        return Fail("asset type capacity overflow did not report required counts");
    }

    capacity_snapshot = type_capacity_manager.Snapshot();
    if (capacity_snapshot.last_required_asset_count != 2U ||
        capacity_snapshot.last_required_type_count != 2U ||
        capacity_snapshot.last_required_dependency_edge_count != 0U) {
        return Fail("asset type capacity snapshot did not report required counts");
    }

    const AssetRegistrationResult duplicate_result = RegisterAsset(
        type_capacity_manager,
        registry,
        1004U,
        ASSET_TYPE_TEXTURE,
        type_capacity_resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (duplicate_result.status != AssetStatus::DuplicateAsset) {
        return Fail("duplicate asset did not clear through non-capacity status");
    }

    capacity_snapshot = type_capacity_manager.Snapshot();
    if (capacity_snapshot.last_required_asset_count != 0U ||
        capacity_snapshot.last_required_type_count != 0U ||
        capacity_snapshot.last_required_dependency_edge_count != 0U) {
        return Fail("non-capacity registration failure left stale required counts");
    }

    const AssetRegistrationResult recovery_asset = RegisterAsset(
        type_capacity_manager,
        registry,
        1006U,
        ASSET_TYPE_TEXTURE,
        type_capacity_recovery_resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!recovery_asset.Succeeded()) {
        return Fail("asset registration recovery failed");
    }

    capacity_snapshot = type_capacity_manager.Snapshot();
    if (capacity_snapshot.last_required_asset_count != 0U ||
        capacity_snapshot.last_required_type_count != 0U ||
        capacity_snapshot.last_required_dependency_edge_count != 0U) {
        return Fail("successful registration left stale required counts");
    }

    return 0;
}

int AssetRegisterRuntimeAssetCapacityEntryPreservesRejectedIdentity() {
    ResourceRegistry registry;
    const ResourceRegistrationResult first_asset_resource =
        RegisterResource(registry, 21U, RESOURCE_TYPE_TEXTURE);
    const ResourceRegistrationResult asset_capacity_resource =
        RegisterResource(registry, 22U, RESOURCE_TYPE_AUDIO);
    if (!first_asset_resource.Succeeded() || !asset_capacity_resource.Succeeded()) {
        return Fail("registration capacity entry resource setup failed");
    }

    AssetManager asset_capacity_manager(AssetManagerDesc{1U, 2U, 3U});
    const AssetRegistrationResult first_asset = RegisterAsset(
        asset_capacity_manager,
        registry,
        9101U,
        ASSET_TYPE_TEXTURE,
        first_asset_resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!first_asset.Succeeded()) {
        return Fail("registration capacity entry setup asset failed");
    }

    const AssetRegistrationResult asset_capacity_result = RegisterAsset(
        asset_capacity_manager,
        registry,
        9102U,
        ASSET_TYPE_AUDIO,
        asset_capacity_resource.handle,
        RESOURCE_TYPE_AUDIO);
    if (asset_capacity_result.status != AssetStatus::CapacityExceeded) {
        return Fail("asset capacity entry did not return capacity status");
    }

    if (asset_capacity_result.required_asset_count != 2U ||
        asset_capacity_result.required_type_count != 2U ||
        asset_capacity_result.required_dependency_edge_count != 0U) {
        return Fail("asset capacity entry did not preserve required counts");
    }

    if (asset_capacity_result.capacity_entry_asset_id != 9102U) {
        return Fail("asset capacity entry did not preserve rejected asset id");
    }

    if (!DoResourceHandlesMatch(asset_capacity_result.capacity_entry_resource_handle, asset_capacity_resource.handle)) {
        return Fail("asset capacity entry did not preserve rejected resource handle");
    }

    if (asset_capacity_result.capacity_entry_resource_type.value != RESOURCE_TYPE_AUDIO.value) {
        return Fail("asset capacity entry did not preserve rejected resource type");
    }

    if (asset_capacity_result.capacity_entry_asset_type.value != ASSET_TYPE_AUDIO.value) {
        return Fail("asset capacity entry did not preserve rejected asset type");
    }

    if (asset_capacity_result.capacity_entry_asset_capacity != 1U ||
        asset_capacity_result.capacity_entry_type_capacity != 2U ||
        asset_capacity_result.capacity_entry_dependency_edge_capacity != 3U) {
        return Fail("asset capacity entry did not preserve capacities");
    }

    if (asset_capacity_result.capacity_entry_asset_count != 1U ||
        asset_capacity_result.capacity_entry_type_count != 1U ||
        asset_capacity_result.capacity_entry_dependency_edge_count != 0U) {
        return Fail("asset capacity entry did not preserve current counts");
    }

    AssetSnapshot snapshot = asset_capacity_manager.Snapshot();
    if (snapshot.active_asset_count != 1U || snapshot.type_count != 1U) {
        return Fail("asset capacity entry mutated accepted counters");
    }

    if (snapshot.last_capacity_entry_asset_id != 9102U ||
        !DoResourceHandlesMatch(snapshot.last_capacity_entry_resource_handle, asset_capacity_resource.handle) ||
        snapshot.last_capacity_entry_resource_type.value != RESOURCE_TYPE_AUDIO.value ||
        snapshot.last_capacity_entry_asset_type.value != ASSET_TYPE_AUDIO.value) {
        return Fail("asset capacity snapshot did not preserve rejected identity");
    }

    if (snapshot.last_capacity_entry_asset_capacity != 1U ||
        snapshot.last_capacity_entry_type_capacity != 2U ||
        snapshot.last_capacity_entry_dependency_edge_capacity != 3U ||
        snapshot.last_capacity_entry_asset_count != 1U ||
        snapshot.last_capacity_entry_type_count != 1U ||
        snapshot.last_capacity_entry_dependency_edge_count != 0U) {
        return Fail("asset capacity snapshot did not preserve capacity entry counts");
    }

    const AssetRegistrationResult duplicate_result = RegisterAsset(
        asset_capacity_manager,
        registry,
        9101U,
        ASSET_TYPE_TEXTURE,
        first_asset_resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (duplicate_result.status != AssetStatus::DuplicateAsset) {
        return Fail("duplicate asset did not clear capacity entry through non-capacity status");
    }

    snapshot = asset_capacity_manager.Snapshot();
    if (snapshot.last_capacity_entry_asset_id != 0U ||
        snapshot.last_capacity_entry_resource_handle.IsValid() ||
        snapshot.last_capacity_entry_resource_type.IsValid() ||
        snapshot.last_capacity_entry_asset_type.IsValid()) {
        return Fail("duplicate asset left stale registration capacity entry");
    }

    const ResourceRegistrationResult type_capacity_resource =
        RegisterResource(registry, 23U, RESOURCE_TYPE_TEXTURE);
    const ResourceRegistrationResult type_overflow_resource =
        RegisterResource(registry, 24U, RESOURCE_TYPE_AUDIO);
    const ResourceRegistrationResult recovery_resource =
        RegisterResource(registry, 25U, RESOURCE_TYPE_TEXTURE);
    if (!type_capacity_resource.Succeeded() ||
        !type_overflow_resource.Succeeded() ||
        !recovery_resource.Succeeded()) {
        return Fail("type capacity entry resource setup failed");
    }

    AssetManager type_capacity_manager(AssetManagerDesc{3U, 1U, 4U});
    const AssetRegistrationResult first_type_asset = RegisterAsset(
        type_capacity_manager,
        registry,
        9201U,
        ASSET_TYPE_TEXTURE,
        type_capacity_resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!first_type_asset.Succeeded()) {
        return Fail("type capacity entry setup asset failed");
    }

    const AssetRegistrationResult type_capacity_result = RegisterAsset(
        type_capacity_manager,
        registry,
        9202U,
        ASSET_TYPE_AUDIO,
        type_overflow_resource.handle,
        RESOURCE_TYPE_AUDIO);
    if (type_capacity_result.status != AssetStatus::CapacityExceeded) {
        return Fail("type capacity entry did not return capacity status");
    }

    if (type_capacity_result.capacity_entry_asset_id != 9202U ||
        !DoResourceHandlesMatch(type_capacity_result.capacity_entry_resource_handle, type_overflow_resource.handle) ||
        type_capacity_result.capacity_entry_resource_type.value != RESOURCE_TYPE_AUDIO.value ||
        type_capacity_result.capacity_entry_asset_type.value != ASSET_TYPE_AUDIO.value) {
        return Fail("type capacity entry did not preserve rejected identity");
    }

    if (type_capacity_result.capacity_entry_asset_capacity != 3U ||
        type_capacity_result.capacity_entry_type_capacity != 1U ||
        type_capacity_result.capacity_entry_dependency_edge_capacity != 4U ||
        type_capacity_result.capacity_entry_asset_count != 1U ||
        type_capacity_result.capacity_entry_type_count != 1U ||
        type_capacity_result.capacity_entry_dependency_edge_count != 0U) {
        return Fail("type capacity entry did not preserve capacity counts");
    }

    const ResourceHandle invalid_resource{99U, 1U};
    const AssetRegistrationResult invalid_resource_result = RegisterAsset(
        type_capacity_manager,
        registry,
        9203U,
        ASSET_TYPE_TEXTURE,
        invalid_resource,
        RESOURCE_TYPE_TEXTURE);
    if (invalid_resource_result.status != AssetStatus::ResourceAcquireFailed) {
        return Fail("invalid resource did not clear registration capacity entry");
    }

    snapshot = type_capacity_manager.Snapshot();
    if (snapshot.last_capacity_entry_asset_id != 0U ||
        snapshot.last_capacity_entry_resource_handle.IsValid() ||
        snapshot.last_capacity_entry_resource_type.IsValid() ||
        snapshot.last_capacity_entry_asset_type.IsValid()) {
        return Fail("invalid resource left stale registration capacity entry");
    }

    const AssetRegistrationResult recovery_result = RegisterAsset(
        type_capacity_manager,
        registry,
        9204U,
        ASSET_TYPE_TEXTURE,
        recovery_resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!recovery_result.Succeeded()) {
        return Fail("registration capacity entry recovery failed");
    }

    snapshot = type_capacity_manager.Snapshot();
    if (snapshot.last_capacity_entry_asset_id != 0U ||
        snapshot.last_capacity_entry_resource_handle.IsValid() ||
        snapshot.last_capacity_entry_resource_type.IsValid() ||
        snapshot.last_capacity_entry_asset_type.IsValid()) {
        return Fail("successful registration left stale capacity entry");
    }

    const ResourceRegistrationResult dependency_resource_a =
        RegisterResource(registry, 26U, RESOURCE_TYPE_TEXTURE);
    const ResourceRegistrationResult dependency_resource_b =
        RegisterResource(registry, 27U, RESOURCE_TYPE_TEXTURE);
    const ResourceRegistrationResult dependency_resource_c =
        RegisterResource(registry, 28U, RESOURCE_TYPE_TEXTURE);
    if (!dependency_resource_a.Succeeded() ||
        !dependency_resource_b.Succeeded() ||
        !dependency_resource_c.Succeeded()) {
        return Fail("dependency capacity entry resource setup failed");
    }

    AssetManager dependency_manager(AssetManagerDesc{3U, 1U, 1U});
    const AssetRegistrationResult dependency_asset_a = RegisterAsset(
        dependency_manager,
        registry,
        9301U,
        ASSET_TYPE_TEXTURE,
        dependency_resource_a.handle,
        RESOURCE_TYPE_TEXTURE);
    const AssetRegistrationResult dependency_asset_b = RegisterAsset(
        dependency_manager,
        registry,
        9302U,
        ASSET_TYPE_TEXTURE,
        dependency_resource_b.handle,
        RESOURCE_TYPE_TEXTURE);
    const AssetRegistrationResult dependency_asset_c = RegisterAsset(
        dependency_manager,
        registry,
        9303U,
        ASSET_TYPE_TEXTURE,
        dependency_resource_c.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!dependency_asset_a.Succeeded() || !dependency_asset_b.Succeeded() || !dependency_asset_c.Succeeded()) {
        return Fail("dependency capacity entry asset setup failed");
    }

    if (dependency_manager.AddDependency(dependency_asset_a.handle, dependency_asset_b.handle) !=
        AssetStatus::Success) {
        return Fail("dependency capacity entry setup edge failed");
    }

    if (dependency_manager.AddDependency(dependency_asset_a.handle, dependency_asset_c.handle) !=
        AssetStatus::CapacityExceeded) {
        return Fail("dependency capacity entry did not return capacity status");
    }

    snapshot = dependency_manager.Snapshot();
    if (snapshot.last_required_dependency_edge_count != 2U ||
        snapshot.active_dependency_edge_count != 1U) {
        return Fail("dependency capacity entry did not preserve required count");
    }

    if (!DoAssetHandlesMatch(snapshot.last_capacity_entry_dependent_asset, dependency_asset_a.handle) ||
        !DoAssetHandlesMatch(snapshot.last_capacity_entry_dependency_asset, dependency_asset_c.handle)) {
        return Fail("dependency capacity snapshot did not preserve rejected handles");
    }

    if (snapshot.last_capacity_entry_asset_capacity != 3U ||
        snapshot.last_capacity_entry_type_capacity != 1U ||
        snapshot.last_capacity_entry_dependency_edge_capacity != 1U ||
        snapshot.last_capacity_entry_asset_count != 3U ||
        snapshot.last_capacity_entry_type_count != 1U ||
        snapshot.last_capacity_entry_dependency_edge_count != 1U) {
        return Fail("dependency capacity snapshot did not preserve capacity counts");
    }

    if (dependency_manager.AddDependency(dependency_asset_a.handle, AssetHandle{99U, 1U}) !=
        AssetStatus::InvalidHandle) {
        return Fail("invalid dependency did not clear capacity entry");
    }

    snapshot = dependency_manager.Snapshot();
    if (snapshot.last_capacity_entry_dependent_asset.IsValid() ||
        snapshot.last_capacity_entry_dependency_asset.IsValid() ||
        snapshot.last_capacity_entry_asset_capacity != 0U ||
        snapshot.last_capacity_entry_dependency_edge_count != 0U) {
        return Fail("invalid dependency left stale capacity entry");
    }

    return 0;
}

int AssetDependenciesTraverseBoundedAndRejectCycle() {
    ResourceRegistry registry;
    const ResourceRegistrationResult resource_a = RegisterResource(registry, 2U, RESOURCE_TYPE_TEXTURE);
    const ResourceRegistrationResult resource_b = RegisterResource(registry, 3U, RESOURCE_TYPE_TEXTURE);
    const ResourceRegistrationResult resource_c = RegisterResource(registry, 4U, RESOURCE_TYPE_TEXTURE);
    if (!resource_a.Succeeded() || !resource_b.Succeeded() || !resource_c.Succeeded()) {
        return Fail("resource registration failed");
    }

    AssetManager manager(AssetManagerDesc{4U, 2U, 2U});
    const AssetRegistrationResult asset_a =
        RegisterAsset(manager, registry, 2001U, ASSET_TYPE_TEXTURE, resource_a.handle, RESOURCE_TYPE_TEXTURE);
    const AssetRegistrationResult asset_b =
        RegisterAsset(manager, registry, 2002U, ASSET_TYPE_TEXTURE, resource_b.handle, RESOURCE_TYPE_TEXTURE);
    const AssetRegistrationResult asset_c =
        RegisterAsset(manager, registry, 2003U, ASSET_TYPE_TEXTURE, resource_c.handle, RESOURCE_TYPE_TEXTURE);
    if (!asset_a.Succeeded() || !asset_b.Succeeded() || !asset_c.Succeeded()) {
        return Fail("asset registration failed");
    }

    if (manager.AddDependency(asset_a.handle, asset_b.handle) != AssetStatus::Success) {
        return Fail("first dependency failed");
    }

    if (manager.AddDependency(asset_b.handle, asset_c.handle) != AssetStatus::Success) {
        return Fail("second dependency failed");
    }

    std::array<AssetHandle, 2U> dependencies{};
    std::uint32_t dependency_count = 0U;
    const AssetStatus traversal_status =
        manager.TraverseDependencies(asset_a.handle, dependencies.data(), 2U, &dependency_count);
    if (traversal_status != AssetStatus::Success) {
        return Fail("dependency traversal failed");
    }

    if (dependency_count != 2U) {
        return Fail("dependency traversal count mismatch");
    }

    if (dependencies[0U].slot != asset_b.handle.slot || dependencies[1U].slot != asset_c.handle.slot) {
        return Fail("dependency traversal order mismatch");
    }

    if (manager.AddDependency(asset_c.handle, asset_a.handle) != AssetStatus::DependencyCycle) {
        return Fail("dependency cycle was not rejected");
    }

    if (manager.AddDependency(asset_a.handle, asset_c.handle) != AssetStatus::CapacityExceeded) {
        return Fail("dependency capacity overflow was not rejected");
    }

    AssetSnapshot dependency_snapshot = manager.Snapshot();
    if (dependency_snapshot.last_required_asset_count != 0U ||
        dependency_snapshot.last_required_type_count != 0U ||
        dependency_snapshot.last_required_dependency_edge_count != 3U) {
        return Fail("dependency capacity snapshot did not report required count");
    }

    if (dependency_snapshot.active_dependency_edge_count != 2U) {
        return Fail("dependency capacity overflow mutated edge count");
    }

    const AssetHandle sentinel_dependency{99U, 77U};
    std::array<AssetHandle, 1U> small_dependencies{};
    small_dependencies[0U] = sentinel_dependency;
    dependency_count = 123U;
    if (manager.TraverseDependencies(asset_a.handle, small_dependencies.data(), 1U, &dependency_count) !=
        AssetStatus::OutputBufferTooSmall) {
        return Fail("small traversal output was not rejected");
    }

    if (dependency_count != 0U) {
        return Fail("small traversal failure did not clear output count");
    }

    if (small_dependencies[0U].slot != sentinel_dependency.slot) {
        return Fail("small traversal failure mutated output slot");
    }

    if (small_dependencies[0U].generation != sentinel_dependency.generation) {
        return Fail("small traversal failure mutated output generation");
    }

    dependency_snapshot = manager.Snapshot();
    if (dependency_snapshot.last_required_asset_count != 0U ||
        dependency_snapshot.last_required_type_count != 0U ||
        dependency_snapshot.last_required_dependency_edge_count != 0U) {
        return Fail("non-capacity dependency failure left stale required counts");
    }

    return 0;
}

int AssetDependencyOutputCapacityReportsEntryIdentity() {
    ResourceRegistry registry;
    const ResourceRegistrationResult resource_a = RegisterResource(registry, 21U, RESOURCE_TYPE_TEXTURE);
    const ResourceRegistrationResult resource_b = RegisterResource(registry, 22U, RESOURCE_TYPE_TEXTURE);
    const ResourceRegistrationResult resource_c = RegisterResource(registry, 23U, RESOURCE_TYPE_TEXTURE);
    if (!resource_a.Succeeded() || !resource_b.Succeeded() || !resource_c.Succeeded()) {
        return Fail("dependency output capacity resource registration failed");
    }

    AssetManager manager(AssetManagerDesc{4U, 2U, 4U});
    const AssetRegistrationResult asset_a =
        RegisterAsset(manager, registry, 2101U, ASSET_TYPE_TEXTURE, resource_a.handle, RESOURCE_TYPE_TEXTURE);
    const AssetRegistrationResult asset_b =
        RegisterAsset(manager, registry, 2102U, ASSET_TYPE_TEXTURE, resource_b.handle, RESOURCE_TYPE_TEXTURE);
    const AssetRegistrationResult asset_c =
        RegisterAsset(manager, registry, 2103U, ASSET_TYPE_TEXTURE, resource_c.handle, RESOURCE_TYPE_TEXTURE);
    if (!asset_a.Succeeded() || !asset_b.Succeeded() || !asset_c.Succeeded()) {
        return Fail("dependency output capacity asset registration failed");
    }

    if (manager.AddDependency(asset_a.handle, asset_b.handle) != AssetStatus::Success) {
        return Fail("dependency output first edge failed");
    }

    if (manager.AddDependency(asset_b.handle, asset_c.handle) != AssetStatus::Success) {
        return Fail("dependency output second edge failed");
    }

    const AssetHandle sentinel_dependency{99U, 77U};
    if (ExpectSmallDependencyOutputFailure(manager, asset_a.handle, sentinel_dependency) != 0) {
        return 1;
    }

    AssetSnapshot snapshot = manager.Snapshot();
    if (snapshot.last_status != AssetStatus::OutputBufferTooSmall) {
        return Fail("dependency output capacity did not set last status");
    }

    if (!DoAssetHandlesMatch(snapshot.last_failed_dependency_output_dependent, asset_b.handle)) {
        return Fail("dependency output capacity did not record dependent asset");
    }

    if (snapshot.last_dependency_output_capacity != 1U) {
        return Fail("dependency output capacity did not record output capacity");
    }

    if (snapshot.last_required_dependency_output_count != 2U) {
        return Fail("dependency output capacity did not record required output count");
    }

    if (!DoAssetHandlesMatch(snapshot.last_failed_dependency_output_dependency, asset_c.handle)) {
        return Fail("dependency output capacity did not record first rejected dependency");
    }

    if (snapshot.last_required_asset_count != 0U ||
        snapshot.last_required_type_count != 0U ||
        snapshot.last_required_dependency_edge_count != 0U) {
        return Fail("dependency output capacity polluted registration counts");
    }

    if (manager.AddDependency(asset_c.handle, asset_a.handle) != AssetStatus::DependencyCycle) {
        return Fail("dependency cycle did not fail after output capacity");
    }

    snapshot = manager.Snapshot();
    if (!IsDependencyOutputCapacityEntryClear(snapshot)) {
        return Fail("dependency cycle left stale output capacity identity");
    }

    if (ExpectSmallDependencyOutputFailure(manager, asset_a.handle, sentinel_dependency) != 0) {
        return 1;
    }

    if (manager.AddDependency(asset_a.handle, asset_b.handle) != AssetStatus::DuplicateDependency) {
        return Fail("duplicate dependency did not fail after output capacity");
    }

    snapshot = manager.Snapshot();
    if (!IsDependencyOutputCapacityEntryClear(snapshot)) {
        return Fail("duplicate dependency left stale output capacity identity");
    }

    if (ExpectSmallDependencyOutputFailure(manager, asset_a.handle, sentinel_dependency) != 0) {
        return 1;
    }

    std::array<AssetHandle, 2U> dependencies{};
    std::uint32_t dependency_count = 0U;
    const AssetHandle invalid_asset{99U, 1U};
    if (manager.TraverseDependencies(invalid_asset, dependencies.data(), 2U, &dependency_count) !=
        AssetStatus::InvalidHandle) {
        return Fail("invalid traversal did not fail after output capacity");
    }

    snapshot = manager.Snapshot();
    if (!IsDependencyOutputCapacityEntryClear(snapshot)) {
        return Fail("invalid traversal left stale output capacity identity");
    }

    if (ExpectSmallDependencyOutputFailure(manager, asset_a.handle, sentinel_dependency) != 0) {
        return 1;
    }

    if (manager.TraverseDependencies(asset_a.handle, dependencies.data(), 2U, &dependency_count) !=
        AssetStatus::Success) {
        return Fail("dependency traversal recovery failed");
    }

    if (dependency_count != 2U) {
        return Fail("dependency traversal recovery count mismatch");
    }

    if (!DoAssetHandlesMatch(dependencies[0U], asset_b.handle)) {
        return Fail("dependency traversal recovery first output mismatch");
    }

    if (!DoAssetHandlesMatch(dependencies[1U], asset_c.handle)) {
        return Fail("dependency traversal recovery second output mismatch");
    }

    snapshot = manager.Snapshot();
    if (!IsDependencyOutputCapacityEntryClear(snapshot)) {
        return Fail("successful traversal left stale output capacity identity");
    }

    return 0;
}

int AssetTextureReadyRecordUsesStreamingResultWithoutOwningDevice() {
    ResourceRegistry registry;
    const ResourceRegistrationResult resource_result = RegisterResource(registry, 5U, RESOURCE_TYPE_TEXTURE);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    AssetManager manager;
    const AssetRegistrationResult asset_result = RegisterAsset(
        manager,
        registry,
        3001U,
        ASSET_TYPE_TEXTURE,
        resource_result.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!asset_result.Succeeded()) {
        return Fail("asset registration failed");
    }

    const ResourceDecodedTextureBridgeResult texture_result =
        MakeTextureResult(resource_result.handle, RESOURCE_TYPE_TEXTURE);
    if (manager.MarkTextureReady(asset_result.handle, texture_result) != AssetStatus::Success) {
        return Fail("texture ready record was rejected");
    }

    AssetRecord record{};
    if (manager.QueryAsset(asset_result.handle, &record) != AssetStatus::Success) {
        return Fail("asset query failed");
    }

    if (record.state != AssetLoadState::Uploaded) {
        return Fail("texture ready did not move asset to uploaded");
    }

    if (!record.texture_ready.is_ready) {
        return Fail("texture ready flag was not recorded");
    }

    if (record.texture_ready.sampled_texture.slot != 2U) {
        return Fail("texture sampled slot was not recorded");
    }

    if (manager.Snapshot().texture_ready_count != 1U) {
        return Fail("texture ready count mismatch");
    }

    return 0;
}

int AssetAudioReadyRecordUsesImportRecordWithoutOwningDevice() {
    ResourceRegistry registry;
    const ResourceRegistrationResult resource_result = RegisterResource(registry, 6U, RESOURCE_TYPE_AUDIO);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    AssetManager manager;
    const AssetRegistrationResult asset_result = RegisterAsset(
        manager,
        registry,
        4001U,
        ASSET_TYPE_AUDIO,
        resource_result.handle,
        RESOURCE_TYPE_AUDIO);
    if (!asset_result.Succeeded()) {
        return Fail("asset registration failed");
    }

    const AudioResourcePcmPacketImportRecord audio_record =
        MakeAudioRecord(resource_result.handle, RESOURCE_TYPE_AUDIO, 77U);
    if (manager.MarkAudioReady(asset_result.handle, audio_record) != AssetStatus::Success) {
        return Fail("audio ready record was rejected");
    }

    AssetRecord record{};
    if (manager.QueryAsset(asset_result.handle, &record) != AssetStatus::Success) {
        return Fail("asset query failed");
    }

    if (record.state != AssetLoadState::Uploaded) {
        return Fail("audio ready did not move asset to uploaded");
    }

    if (!record.audio_ready.is_ready) {
        return Fail("audio ready flag was not recorded");
    }

    if (record.audio_ready.packet_request.packet_id != 77U) {
        return Fail("audio packet id was not recorded");
    }

    if (manager.Snapshot().audio_ready_count != 1U) {
        return Fail("audio ready count mismatch");
    }

    return 0;
}

int AssetRefreshStateFromResourceMapsUploadedResidentAndFailed() {
    ResourceRegistry registry;
    const ResourceRegistrationResult uploaded_resource = RegisterResource(registry, 7U, RESOURCE_TYPE_TEXTURE);
    const ResourceRegistrationResult failed_resource = RegisterResource(registry, 8U, RESOURCE_TYPE_TEXTURE);
    if (!uploaded_resource.Succeeded() || !failed_resource.Succeeded()) {
        return Fail("resource registration failed");
    }

    AssetManager manager;
    const AssetRegistrationResult uploaded_asset = RegisterAsset(
        manager,
        registry,
        5001U,
        ASSET_TYPE_TEXTURE,
        uploaded_resource.handle,
        RESOURCE_TYPE_TEXTURE);
    const AssetRegistrationResult failed_asset = RegisterAsset(
        manager,
        registry,
        5002U,
        ASSET_TYPE_TEXTURE,
        failed_resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!uploaded_asset.Succeeded() || !failed_asset.Succeeded()) {
        return Fail("asset registration failed");
    }

    ResourceLoadCommitRequest upload_commit{};
    upload_commit.resource = uploaded_resource.handle;
    upload_commit.expected_type = RESOURCE_TYPE_TEXTURE;
    upload_commit.load_state = ResourceLoadState::Uploaded;
    upload_commit.commit_id = 1U;
    upload_commit.upload_id = 2U;
    upload_commit.staging_request_id = 3U;
    upload_commit.upload_byte_count = 64U;
    if (registry.CommitUploadCompletion(upload_commit) != ResourceLoadCommitStatus::Success) {
        return Fail("resource upload commit failed");
    }

    if (manager.RefreshStateFromResource(&registry, uploaded_asset.handle) != AssetStatus::Success) {
        return Fail("refresh uploaded asset failed");
    }

    AssetRecord uploaded_record{};
    if (manager.QueryAsset(uploaded_asset.handle, &uploaded_record) != AssetStatus::Success) {
        return Fail("uploaded asset query failed");
    }

    if (uploaded_record.state != AssetLoadState::Uploaded) {
        return Fail("resource uploaded state was not mapped");
    }

    ResourceResidencyBudgetDesc budget{};
    budget.byte_capacity = 128U;
    if (registry.SetResidencyBudget(budget) != ResourceResidencyStatus::Success) {
        return Fail("resource residency budget failed");
    }

    ResourceResidencyRequest residency_request{};
    residency_request.resource = uploaded_resource.handle;
    residency_request.expected_type = RESOURCE_TYPE_TEXTURE;
    if (registry.AdmitResident(residency_request) != ResourceResidencyStatus::Success) {
        return Fail("resource residency admit failed");
    }

    if (manager.RefreshStateFromResource(&registry, uploaded_asset.handle) != AssetStatus::Success) {
        return Fail("refresh resident asset failed");
    }

    if (manager.QueryAsset(uploaded_asset.handle, &uploaded_record) != AssetStatus::Success) {
        return Fail("resident asset query failed");
    }

    if (uploaded_record.state != AssetLoadState::Resident) {
        return Fail("resource resident state was not mapped");
    }

    ResourceLoadCommitRequest failed_commit{};
    failed_commit.resource = failed_resource.handle;
    failed_commit.expected_type = RESOURCE_TYPE_TEXTURE;
    failed_commit.load_state = ResourceLoadState::Failed;
    failed_commit.commit_id = 4U;
    failed_commit.upload_id = 5U;
    failed_commit.staging_request_id = 6U;
    failed_commit.upload_byte_count = 64U;
    if (registry.CommitUploadCompletion(failed_commit) != ResourceLoadCommitStatus::Success) {
        return Fail("resource failed commit failed");
    }

    if (manager.RefreshStateFromResource(&registry, failed_asset.handle) != AssetStatus::Success) {
        return Fail("refresh failed asset failed");
    }

    AssetRecord failed_record{};
    if (manager.QueryAsset(failed_asset.handle, &failed_record) != AssetStatus::Success) {
        return Fail("failed asset query failed");
    }

    if (failed_record.state != AssetLoadState::Failed) {
        return Fail("resource failed state was not mapped");
    }

    return 0;
}

int AssetReleaseRuntimeAssetReleasesResourceAndClearsReadyRecords() {
    ResourceRegistry registry;
    const ResourceRegistrationResult resource_result = RegisterResource(registry, 9U, RESOURCE_TYPE_TEXTURE);
    const ResourceRegistrationResult dependency_resource = RegisterResource(registry, 10U, RESOURCE_TYPE_TEXTURE);
    if (!resource_result.Succeeded() || !dependency_resource.Succeeded()) {
        return Fail("resource registration failed");
    }

    AssetManager manager;
    const AssetRegistrationResult asset_result = RegisterAsset(
        manager,
        registry,
        6001U,
        ASSET_TYPE_TEXTURE,
        resource_result.handle,
        RESOURCE_TYPE_TEXTURE);
    const AssetRegistrationResult dependency_result = RegisterAsset(
        manager,
        registry,
        6002U,
        ASSET_TYPE_TEXTURE,
        dependency_resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!asset_result.Succeeded() || !dependency_result.Succeeded()) {
        return Fail("asset registration failed");
    }

    const ResourceDecodedTextureBridgeResult texture_result =
        MakeTextureResult(resource_result.handle, RESOURCE_TYPE_TEXTURE);
    if (manager.MarkTextureReady(asset_result.handle, texture_result) != AssetStatus::Success) {
        return Fail("texture ready record failed");
    }

    if (manager.AddDependency(asset_result.handle, dependency_result.handle) != AssetStatus::Success) {
        return Fail("dependency add failed");
    }

    if (manager.AcquireAsset(asset_result.handle) != AssetStatus::Success) {
        return Fail("asset acquire failed");
    }

    if (manager.ReleaseRuntimeAsset(&registry, asset_result.handle) != AssetStatus::StillReferenced) {
        return Fail("referenced runtime asset release was not rejected");
    }

    if (manager.ReleaseAssetReference(asset_result.handle) != AssetStatus::Success) {
        return Fail("asset reference release failed");
    }

    if (manager.ReleaseRuntimeAsset(&registry, asset_result.handle) != AssetStatus::Success) {
        return Fail("runtime asset release failed");
    }

    const AssetSnapshot snapshot = manager.Snapshot();
    if (snapshot.active_asset_count != 1U) {
        return Fail("release did not remove one asset");
    }

    if (snapshot.texture_ready_count != 0U) {
        return Fail("release did not clear texture ready count");
    }

    if (snapshot.active_dependency_edge_count != 0U) {
        return Fail("release did not clear dependency edge");
    }

    if (registry.Snapshot().released_handle_count != 1U) {
        return Fail("runtime asset release did not release lower resource");
    }

    AssetRecord old_record{};
    if (manager.QueryAsset(asset_result.handle, &old_record) == AssetStatus::Success) {
        return Fail("released asset handle remained valid");
    }

    return 0;
}

int AssetManagerSuccessClearsLastStatusAfterFailure() {
    ResourceRegistry registry;
    const ResourceRegistrationResult resource_result = RegisterResource(registry, 15U, RESOURCE_TYPE_TEXTURE);
    if (!resource_result.Succeeded()) {
        return Fail("success last-status resource registration failed");
    }

    AssetManager manager;
    const ResourceHandle invalid_resource{999U, 1U};
    const AssetDescriptor invalid_descriptor = MakeAssetDescriptor(
        9001U,
        ASSET_TYPE_TEXTURE,
        invalid_resource,
        RESOURCE_TYPE_TEXTURE);
    const AssetRegistrationResult failed_register =
        manager.RegisterRuntimeAsset(&registry, invalid_descriptor);
    if (failed_register.status != AssetStatus::ResourceAcquireFailed) {
        return Fail("invalid resource register did not fail through asset status");
    }

    AssetSnapshot snapshot = manager.Snapshot();
    if (snapshot.last_status != AssetStatus::ResourceAcquireFailed) {
        return Fail("resource failure did not set asset last status");
    }

    if (snapshot.last_resource_status != ResourceStatus::InvalidHandle) {
        return Fail("resource failure did not set lower resource status");
    }

    const AssetRegistrationResult asset_result = RegisterAsset(
        manager,
        registry,
        9002U,
        ASSET_TYPE_TEXTURE,
        resource_result.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!asset_result.Succeeded()) {
        return Fail("success last-status asset registration failed");
    }

    snapshot = manager.Snapshot();
    if (snapshot.last_status != AssetStatus::Success) {
        return Fail("successful register did not clear asset last status");
    }

    if (snapshot.last_resource_status != ResourceStatus::Success) {
        return Fail("successful register did not clear lower resource status");
    }

    const AssetStatus invalid_acquire = manager.AcquireAsset(AssetHandle{99U, 1U});
    if (invalid_acquire != AssetStatus::InvalidHandle) {
        return Fail("invalid acquire did not fail");
    }

    if (manager.AcquireAsset(asset_result.handle) != AssetStatus::Success) {
        return Fail("success last-status acquire failed");
    }

    snapshot = manager.Snapshot();
    if (snapshot.last_status != AssetStatus::Success) {
        return Fail("successful acquire did not clear asset last status");
    }

    ResourceDecodedTextureBridgeResult failed_texture = MakeTextureResult(resource_result.handle, RESOURCE_TYPE_TEXTURE);
    failed_texture.status = ResourceDecodedTextureBridgeStatus::InvalidArgument;
    if (manager.MarkTextureReady(asset_result.handle, failed_texture) != AssetStatus::TextureReadyFailed) {
        return Fail("invalid texture ready did not fail");
    }

    AssetRecord record{};
    if (manager.QueryAsset(asset_result.handle, &record) != AssetStatus::Success) {
        return Fail("success last-status query failed");
    }

    snapshot = manager.Snapshot();
    if (snapshot.last_status != AssetStatus::Success) {
        return Fail("successful query did not clear asset last status");
    }

    if (snapshot.last_texture_status != ResourceDecodedTextureBridgeStatus::InvalidArgument) {
        return Fail("successful query cleared subordinate texture status");
    }

    if (manager.ReleaseRuntimeAsset(&registry, asset_result.handle) != AssetStatus::StillReferenced) {
        return Fail("referenced release did not fail");
    }

    if (manager.ReleaseAssetReference(asset_result.handle) != AssetStatus::Success) {
        return Fail("success last-status release reference failed");
    }

    snapshot = manager.Snapshot();
    if (snapshot.last_status != AssetStatus::Success) {
        return Fail("successful reference release did not clear asset last status");
    }

    if (snapshot.last_texture_status != ResourceDecodedTextureBridgeStatus::InvalidArgument) {
        return Fail("successful reference release cleared subordinate texture status");
    }

    return 0;
}

int AssetRuntimeFixtureClosesResidentTexturePathAndDependencies() {
    ResourceRegistry registry;
    const ResourceRegistrationResult root_resource = RegisterResource(registry, 11U, RESOURCE_TYPE_TEXTURE);
    const ResourceRegistrationResult dependency_resource = RegisterResource(registry, 12U, RESOURCE_TYPE_TEXTURE);
    if (!root_resource.Succeeded() || !dependency_resource.Succeeded()) {
        return Fail("resource registration failed");
    }

    AssetManager manager;
    const AssetRegistrationResult root_asset =
        RegisterAsset(manager, registry, 7001U, ASSET_TYPE_TEXTURE, root_resource.handle, RESOURCE_TYPE_TEXTURE);
    const AssetRegistrationResult dependency_asset =
        RegisterAsset(manager, registry, 7002U, ASSET_TYPE_TEXTURE, dependency_resource.handle, RESOURCE_TYPE_TEXTURE);
    if (!root_asset.Succeeded() || !dependency_asset.Succeeded()) {
        return Fail("asset registration failed");
    }

    if (manager.AddDependency(root_asset.handle, dependency_asset.handle) != AssetStatus::Success) {
        return Fail("asset dependency add failed");
    }

    ResourceLoadCommitRequest upload_commit{};
    upload_commit.resource = root_resource.handle;
    upload_commit.expected_type = RESOURCE_TYPE_TEXTURE;
    upload_commit.load_state = ResourceLoadState::Uploaded;
    upload_commit.commit_id = 7U;
    upload_commit.upload_id = 8U;
    upload_commit.staging_request_id = 9U;
    upload_commit.upload_byte_count = 64U;
    if (registry.CommitUploadCompletion(upload_commit) != ResourceLoadCommitStatus::Success) {
        return Fail("resource upload commit failed");
    }

    ResourceResidencyBudgetDesc budget{};
    budget.byte_capacity = 128U;
    if (registry.SetResidencyBudget(budget) != ResourceResidencyStatus::Success) {
        return Fail("resource residency budget failed");
    }

    ResourceResidencyRequest residency_request{};
    residency_request.resource = root_resource.handle;
    residency_request.expected_type = RESOURCE_TYPE_TEXTURE;
    if (registry.AdmitResident(residency_request) != ResourceResidencyStatus::Success) {
        return Fail("resource residency admit failed");
    }

    ResourceDecodedPayloadRecord decoded_payload =
        MakeDecodedPayloadRecord(root_resource.handle, RESOURCE_TYPE_TEXTURE, 16U);
    const ResourceDecodedTextureBridgeResult texture_result =
        MakeTextureResult(root_resource.handle, RESOURCE_TYPE_TEXTURE);
    std::array<AssetHandle, 1U> dependencies{};
    AssetRuntimeFixture fixture;
    AssetRuntimeFixtureRequest fixture_request{};
    fixture_request.manager = &manager;
    fixture_request.resource_registry = &registry;
    fixture_request.root_asset = root_asset.handle;
    fixture_request.dependency_output = std::span<AssetHandle>(dependencies.data(), dependencies.size());
    fixture_request.decoded_payload = &decoded_payload;
    fixture_request.texture_result = &texture_result;
    fixture_request.refresh_state_from_resource = true;

    const AssetRuntimeFixtureResult fixture_result = fixture.Execute(fixture_request);
    if (fixture_result.status != AssetRuntimeFixtureStatus::Success) {
        return Fail("asset runtime fixture failed");
    }

    if (fixture_result.dependency_count != 1U) {
        return Fail("asset runtime fixture dependency count mismatch");
    }

    if (dependencies[0U].slot != dependency_asset.handle.slot) {
        return Fail("asset runtime fixture dependency output mismatch");
    }

    if (!fixture_result.decoded_applied || !fixture_result.texture_ready_applied) {
        return Fail("asset runtime fixture did not apply ready records");
    }

    if (!fixture_result.resource_state_refreshed) {
        return Fail("asset runtime fixture did not refresh resource state");
    }

    if (fixture_result.root_record.state != AssetLoadState::Resident) {
        return Fail("asset runtime fixture did not map resident state");
    }

    if (!fixture_result.root_record.texture_ready.is_ready) {
        return Fail("asset runtime fixture did not keep texture ready record");
    }

    const AssetRuntimeFixtureSnapshot fixture_snapshot = fixture.Snapshot();
    if (fixture_snapshot.executed_count != 1U) {
        return Fail("asset runtime fixture execution count mismatch");
    }

    if (fixture_snapshot.dependency_traversal_count != 1U) {
        return Fail("asset runtime fixture traversal count mismatch");
    }

    if (fixture_result.asset_snapshot.texture_ready_count != 1U) {
        return Fail("asset snapshot texture ready count mismatch");
    }

    return 0;
}

int AssetRuntimeFixtureRejectsSmallDependencyOutputBeforeMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult root_resource = RegisterResource(registry, 13U, RESOURCE_TYPE_TEXTURE);
    const ResourceRegistrationResult dependency_resource = RegisterResource(registry, 14U, RESOURCE_TYPE_TEXTURE);
    if (!root_resource.Succeeded() || !dependency_resource.Succeeded()) {
        return Fail("resource registration failed");
    }

    AssetManager manager;
    const AssetRegistrationResult root_asset =
        RegisterAsset(manager, registry, 8001U, ASSET_TYPE_TEXTURE, root_resource.handle, RESOURCE_TYPE_TEXTURE);
    const AssetRegistrationResult dependency_asset =
        RegisterAsset(manager, registry, 8002U, ASSET_TYPE_TEXTURE, dependency_resource.handle, RESOURCE_TYPE_TEXTURE);
    if (!root_asset.Succeeded() || !dependency_asset.Succeeded()) {
        return Fail("asset registration failed");
    }

    if (manager.AddDependency(root_asset.handle, dependency_asset.handle) != AssetStatus::Success) {
        return Fail("asset dependency add failed");
    }

    AssetRuntimeFixture fixture;
    AssetRuntimeFixtureRequest fixture_request{};
    fixture_request.manager = &manager;
    fixture_request.root_asset = root_asset.handle;

    const AssetRuntimeFixtureResult fixture_result = fixture.Execute(fixture_request);
    if (fixture_result.status != AssetRuntimeFixtureStatus::DependencyTraversalFailed) {
        return Fail("small dependency output did not fail fixture traversal");
    }

    if (fixture_result.last_asset_status != AssetStatus::OutputBufferTooSmall) {
        return Fail("fixture did not expose asset traversal failure");
    }

    AssetRecord root_record{};
    if (manager.QueryAsset(root_asset.handle, &root_record) != AssetStatus::Success) {
        return Fail("root asset query failed");
    }

    if (root_record.state != AssetLoadState::Unloaded) {
        return Fail("fixture mutated root state after traversal failure");
    }

    if (fixture.Snapshot().rejected_count != 1U) {
        return Fail("fixture rejected count mismatch");
    }

    return 0;
}

int AssetNoWorldGameAdapterUiDependency() {
    AssetManager manager;
    const AssetSnapshot snapshot = manager.Snapshot();
    if (snapshot.asset_capacity == 0U) {
        return Fail("asset manager capacity was not bounded");
    }

    if (snapshot.dependency_edge_capacity == 0U) {
        return Fail("asset dependency capacity was not bounded");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("asset manager allocation vocabulary changed");
    }

    return 0;
}

const std::unordered_map<std::string_view, TestFunction> TESTS = {
    {TEST_REGISTER, AssetRegisterRuntimeAssetReturnsStableHandleAndState},
    {TEST_REGISTRATION_CAPACITY_ENTRY, AssetRegisterRuntimeAssetCapacityEntryPreservesRejectedIdentity},
    {TEST_DEPENDENCIES, AssetDependenciesTraverseBoundedAndRejectCycle},
    {TEST_DEPENDENCY_OUTPUT_CAPACITY_ENTRY, AssetDependencyOutputCapacityReportsEntryIdentity},
    {TEST_TEXTURE_READY, AssetTextureReadyRecordUsesStreamingResultWithoutOwningDevice},
    {TEST_AUDIO_READY, AssetAudioReadyRecordUsesImportRecordWithoutOwningDevice},
    {TEST_REFRESH_STATE, AssetRefreshStateFromResourceMapsUploadedResidentAndFailed},
    {TEST_RELEASE, AssetReleaseRuntimeAssetReleasesResourceAndClearsReadyRecords},
    {TEST_SUCCESS_LAST_STATUS, AssetManagerSuccessClearsLastStatusAfterFailure},
    {TEST_RUNTIME_FIXTURE, AssetRuntimeFixtureClosesResidentTexturePathAndDependencies},
    {TEST_RUNTIME_FIXTURE_BOUNDARY, AssetRuntimeFixtureRejectsSmallDependencyOutputBeforeMutation},
    {TEST_NO_UPPER_DEPENDENCY, AssetNoWorldGameAdapterUiDependency},
};
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::string_view test_name(argv[1]);
    const auto test = TESTS.find(test_name);
    if (test == TESTS.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test->second();
}
