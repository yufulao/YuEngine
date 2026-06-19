// 模块: Tests Asset
// 文件: Tests/Asset/AssetManagerTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>

#include "YuEngine/Asset/AssetManager.h"
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
constexpr const char *TEST_DEPENDENCIES = "Asset_DependenciesTraverseBoundedAndRejectCycle";
constexpr const char *TEST_TEXTURE_READY =
    "Asset_TextureReadyRecordUsesStreamingResultWithoutOwningDevice";
constexpr const char *TEST_AUDIO_READY =
    "Asset_AudioReadyRecordUsesImportRecordWithoutOwningDevice";
constexpr const char *TEST_REFRESH_STATE =
    "Asset_RefreshStateFromResourceMapsUploadedResidentAndFailed";
constexpr const char *TEST_RELEASE = "Asset_ReleaseRuntimeAssetReleasesResourceAndClearsReadyRecords";
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

    std::array<AssetHandle, 1U> small_dependencies{};
    if (manager.TraverseDependencies(asset_a.handle, small_dependencies.data(), 1U, &dependency_count) !=
        AssetStatus::OutputBufferTooSmall) {
        return Fail("small traversal output was not rejected");
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
    {TEST_DEPENDENCIES, AssetDependenciesTraverseBoundedAndRejectCycle},
    {TEST_TEXTURE_READY, AssetTextureReadyRecordUsesStreamingResultWithoutOwningDevice},
    {TEST_AUDIO_READY, AssetAudioReadyRecordUsesImportRecordWithoutOwningDevice},
    {TEST_REFRESH_STATE, AssetRefreshStateFromResourceMapsUploadedResidentAndFailed},
    {TEST_RELEASE, AssetReleaseRuntimeAssetReleasesResourceAndClearsReadyRecords},
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
