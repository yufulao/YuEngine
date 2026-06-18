// 模块：Tests Streaming
// 文件：Tests/Streaming/ResourceDecodedTextureBridgeTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>
#include <unordered_map>

#include "YuEngine/RenderCore/MaterialBindingFixture.h"
#include "YuEngine/RenderCore/MaterialBindingFixtureRequest.h"
#include "YuEngine/RenderCore/MaterialBindingFixtureStatus.h"
#include "YuEngine/RenderCore/RenderFixturePassRequest.h"
#include "YuEngine/Resource/ResourceCachePayloadBudgetDesc.h"
#include "YuEngine/Resource/ResourceCachePayloadRequest.h"
#include "YuEngine/Resource/ResourceCachePayloadStatus.h"
#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceDecodedPayloadBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodedPayloadRequest.h"
#include "YuEngine/Resource/ResourceDecodedPayloadStatus.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodePlanRequest.h"
#include "YuEngine/Resource/ResourceDecodePlanStatus.h"
#include "YuEngine/Resource/ResourceDecodeResultBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodeResultClass.h"
#include "YuEngine/Resource/ResourceDecodeResultRequest.h"
#include "YuEngine/Resource/ResourceDecodeResultStatus.h"
#include "YuEngine/Resource/ResourceDescriptor.h"
#include "YuEngine/Resource/ResourceLoadCommitRequest.h"
#include "YuEngine/Resource/ResourceLoadCommitStatus.h"
#include "YuEngine/Resource/ResourceLoadState.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/Resource/ResourceResidencyBudgetDesc.h"
#include "YuEngine/Resource/ResourceResidencyRequest.h"
#include "YuEngine/Resource/ResourceResidencyStatus.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/Rhi/NullRhiDevice.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiDeviceSnapshot.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"
#include "YuEngine/Rhi/RhiSamplerHandle.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridge.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeRequest.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeResult.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeSnapshot.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeStatus.h"
#include "YuEngine/Streaming/ResourceUploadStatus.h"

using yuengine::rendercore::MaterialBindingFixture;
using yuengine::rendercore::MaterialBindingFixtureRequest;
using yuengine::rendercore::MaterialBindingFixtureStatus;
using yuengine::rendercore::RenderFixturePassRequest;
using yuengine::resource::ResourceCachePayloadBudgetDesc;
using yuengine::resource::ResourceCachePayloadRequest;
using yuengine::resource::ResourceCachePayloadStatus;
using yuengine::resource::ResourceDecodedPayloadBudgetDesc;
using yuengine::resource::ResourceDecodedPayloadRequest;
using yuengine::resource::ResourceDecodedPayloadStatus;
using yuengine::resource::ResourceDecodePlanAssetClass;
using yuengine::resource::ResourceDecodePlanRequest;
using yuengine::resource::ResourceDecodePlanStatus;
using yuengine::resource::ResourceDecodeResultBudgetDesc;
using yuengine::resource::ResourceDecodeResultClass;
using yuengine::resource::ResourceDecodeResultRequest;
using yuengine::resource::ResourceDecodeResultStatus;
using yuengine::resource::ResourceDescriptor;
using yuengine::resource::ResourceHandle;
using yuengine::resource::ResourceLoadCommitRequest;
using yuengine::resource::ResourceLoadCommitStatus;
using yuengine::resource::ResourceLoadState;
using yuengine::resource::ResourceLogicalKey;
using yuengine::resource::ResourceRegistry;
using yuengine::resource::ResourceRegistrationResult;
using yuengine::resource::ResourceResidencyBudgetDesc;
using yuengine::resource::ResourceResidencyRequest;
using yuengine::resource::ResourceResidencyStatus;
using yuengine::resource::ResourceTypeId;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_0;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_1;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_2;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_3;
using yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_VERSION;
using yuengine::rhi::NullRhiDevice;
using yuengine::rhi::RhiDeviceDesc;
using yuengine::rhi::RhiDeviceSnapshot;
using yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiSamplerBinding;
using yuengine::rhi::RhiSamplerHandle;
using yuengine::rhi::RhiStatus;
using yuengine::rhi::RhiTextureDesc;
using yuengine::rhi::RhiTextureHandle;
using yuengine::streaming::ResourceDecodedTextureBridge;
using yuengine::streaming::ResourceDecodedTextureBridgeRequest;
using yuengine::streaming::ResourceDecodedTextureBridgeResult;
using yuengine::streaming::ResourceDecodedTextureBridgeSnapshot;
using yuengine::streaming::ResourceDecodedTextureBridgeStatus;
using yuengine::streaming::ResourceUploadStatus;

namespace {
constexpr const char *TEST_UPLOADS_TEXTURE =
    "Streaming_ResourceDecodedTextureBridge_UploadsDecodedPayloadAsTextureBinding";
constexpr const char *TEST_REJECTS_MISMATCH =
    "Streaming_ResourceDecodedTextureBridge_RejectsTextureByteMismatchWithoutRhiMutation";
constexpr const char *TEST_REJECTS_SMALL_SCRATCH =
    "Streaming_ResourceDecodedTextureBridge_RejectsSmallScratchWithoutRhiMutation";
constexpr const char *TEST_REJECTS_SAMPLED_TEXTURE_SLOT =
    "Streaming_ResourceDecodedTextureBridge_RejectsSampledTextureSlotOutOfRangeWithoutRhiMutation";
constexpr const char *TEST_REPORTS_RHI_CAPACITY =
    "Streaming_ResourceDecodedTextureBridge_ReportsRhiTextureCapacityWithoutWritingOutput";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr ResourceTypeId TYPE_TEXTURE{1U};
constexpr const char *RESOURCE_KEY = "decoded_texture_bridge";
constexpr std::uint64_t COMMIT_ONE = 101U;
constexpr std::uint64_t UPLOAD_ONE = 201U;
constexpr std::uint64_t PAYLOAD_ONE = 301U;
constexpr std::uint64_t DECODE_PLAN_ONE = 401U;
constexpr std::uint64_t DECODE_RESULT_ONE = 501U;
constexpr std::uint64_t DECODED_PAYLOAD_ONE = 601U;
constexpr std::uint64_t STAGING_ONE = 701U;
constexpr std::uint64_t BRIDGE_UPLOAD_ONE = 801U;
constexpr std::uint32_t TEXTURE_WIDTH = 2U;
constexpr std::uint32_t TEXTURE_HEIGHT = 2U;
constexpr std::uint32_t TEXTURE_BYTE_COUNT = 16U;
constexpr std::uint32_t DECODE_PLAN_PAYLOAD_BYTE_COUNT = RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT + 4U;
constexpr std::uint32_t MATERIAL_ID = 1U;
constexpr std::uint32_t PASS_ID = 2U;
using TestFunction = int (*)();

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

ResourceRegistrationResult RegisterTexture(ResourceRegistry &registry) {
    const ResourceDescriptor descriptor{TYPE_TEXTURE, ResourceLogicalKey(RESOURCE_KEY), 0U};
    return registry.RegisterSyntheticDescriptor(descriptor);
}

ResourceLoadCommitRequest LoadCommitRequest(ResourceHandle resource) {
    ResourceLoadCommitRequest request;
    request.resource = resource;
    request.expected_type = TYPE_TEXTURE;
    request.load_state = ResourceLoadState::Uploaded;
    request.commit_id = COMMIT_ONE;
    request.upload_id = UPLOAD_ONE;
    request.staging_request_id = STAGING_ONE;
    request.upload_byte_count = TEXTURE_BYTE_COUNT;
    return request;
}

ResourceResidencyRequest ResidencyRequest(ResourceHandle resource) {
    ResourceResidencyRequest request;
    request.resource = resource;
    request.expected_type = TYPE_TEXTURE;
    return request;
}

ResourceCachePayloadRequest CachePayloadRequest(
    ResourceHandle resource,
    const std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> &payload) {
    ResourceCachePayloadRequest request;
    request.resource = resource;
    request.expected_type = TYPE_TEXTURE;
    request.payload_id = PAYLOAD_ONE;
    request.payload_bytes = payload.data();
    request.payload_byte_count = static_cast<std::uint32_t>(payload.size());
    return request;
}

ResourceDecodePlanRequest DecodePlanRequest(ResourceHandle resource, std::uint32_t decoded_byte_count) {
    ResourceDecodePlanRequest request;
    request.resource = resource;
    request.expected_type = TYPE_TEXTURE;
    request.payload_id = PAYLOAD_ONE;
    request.decode_plan_id = DECODE_PLAN_ONE;
    request.asset_class = ResourceDecodePlanAssetClass::Texture;
    request.source_byte_count = DECODE_PLAN_PAYLOAD_BYTE_COUNT;
    request.expected_decoded_byte_count = decoded_byte_count;
    return request;
}

ResourceDecodeResultRequest DecodeResultRequest(ResourceHandle resource, std::uint32_t decoded_byte_count) {
    ResourceDecodeResultRequest request;
    request.resource = resource;
    request.expected_type = TYPE_TEXTURE;
    request.payload_id = PAYLOAD_ONE;
    request.decode_plan_id = DECODE_PLAN_ONE;
    request.decode_result_id = DECODE_RESULT_ONE;
    request.asset_class = ResourceDecodePlanAssetClass::Texture;
    request.result_class = ResourceDecodeResultClass::Texture;
    request.decoded_byte_count = decoded_byte_count;
    return request;
}

ResourceDecodedPayloadRequest DecodedPayloadRequest(
    ResourceHandle resource,
    const std::uint8_t *decoded_bytes,
    std::uint32_t decoded_byte_count) {
    ResourceDecodedPayloadRequest request;
    request.resource = resource;
    request.expected_type = TYPE_TEXTURE;
    request.payload_id = PAYLOAD_ONE;
    request.decode_plan_id = DECODE_PLAN_ONE;
    request.decode_result_id = DECODE_RESULT_ONE;
    request.decoded_payload_id = DECODED_PAYLOAD_ONE;
    request.asset_class = ResourceDecodePlanAssetClass::Texture;
    request.result_class = ResourceDecodeResultClass::Texture;
    request.decoded_bytes = decoded_bytes;
    request.decoded_byte_count = decoded_byte_count;
    return request;
}

void WriteU32LittleEndian(
    std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> &bytes,
    std::uint32_t offset,
    std::uint32_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[offset + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[offset + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> DecodePlanPayload(std::uint32_t decoded_byte_count) {
    std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> payload{};
    payload[0U] = RESOURCE_DECODE_PLAN_HEADER_MAGIC_0;
    payload[1U] = RESOURCE_DECODE_PLAN_HEADER_MAGIC_1;
    payload[2U] = RESOURCE_DECODE_PLAN_HEADER_MAGIC_2;
    payload[3U] = RESOURCE_DECODE_PLAN_HEADER_MAGIC_3;
    WriteU32LittleEndian(payload, 4U, RESOURCE_DECODE_PLAN_HEADER_VERSION);
    WriteU32LittleEndian(payload, 8U, static_cast<std::uint32_t>(ResourceDecodePlanAssetClass::Texture));
    WriteU32LittleEndian(payload, 12U, DECODE_PLAN_PAYLOAD_BYTE_COUNT);
    WriteU32LittleEndian(payload, 16U, decoded_byte_count);
    payload[RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT] = 0x7AU;
    return payload;
}

bool ConfigureResourceBudgets(ResourceRegistry &registry) {
    ResourceResidencyBudgetDesc residency_budget;
    residency_budget.byte_capacity = 256U;
    if (registry.SetResidencyBudget(residency_budget) != ResourceResidencyStatus::Success) {
        return false;
    }

    ResourceCachePayloadBudgetDesc cache_budget;
    cache_budget.byte_capacity = 256U;
    if (registry.SetCachePayloadBudget(cache_budget) != ResourceCachePayloadStatus::Success) {
        return false;
    }

    ResourceDecodeResultBudgetDesc result_budget;
    result_budget.decoded_byte_capacity = 256U;
    if (registry.SetDecodeResultBudget(result_budget) != ResourceDecodeResultStatus::Success) {
        return false;
    }

    ResourceDecodedPayloadBudgetDesc decoded_budget;
    decoded_budget.decoded_byte_capacity = 256U;
    return registry.SetDecodedPayloadBudget(decoded_budget) == ResourceDecodedPayloadStatus::Success;
}

bool BuildDecodedTextureResource(
    ResourceRegistry &registry,
    ResourceHandle resource,
    const std::array<std::uint8_t, TEXTURE_BYTE_COUNT> &decoded_bytes) {
    if (!ConfigureResourceBudgets(registry)) {
        return false;
    }

    if (registry.CommitUploadCompletion(LoadCommitRequest(resource)) != ResourceLoadCommitStatus::Success) {
        return false;
    }

    if (registry.AdmitResident(ResidencyRequest(resource)) != ResourceResidencyStatus::Success) {
        return false;
    }

    const std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> plan_payload =
        DecodePlanPayload(TEXTURE_BYTE_COUNT);
    if (registry.StoreCachePayload(CachePayloadRequest(resource, plan_payload)) !=
        ResourceCachePayloadStatus::Success) {
        return false;
    }

    if (registry.CreateDecodePlan(DecodePlanRequest(resource, TEXTURE_BYTE_COUNT)) !=
        ResourceDecodePlanStatus::Success) {
        return false;
    }

    if (registry.CommitDecodeResult(DecodeResultRequest(resource, TEXTURE_BYTE_COUNT)) !=
        ResourceDecodeResultStatus::Success) {
        return false;
    }

    const ResourceDecodedPayloadRequest request = DecodedPayloadRequest(
        resource,
        decoded_bytes.data(),
        static_cast<std::uint32_t>(decoded_bytes.size()));
    return registry.StoreDecodedPayload(request) == ResourceDecodedPayloadStatus::Success;
}

RhiTextureDesc TextureDesc(std::uint32_t width, std::uint32_t height) {
    RhiTextureDesc desc;
    desc.extent.width = static_cast<std::uint16_t>(width);
    desc.extent.height = static_cast<std::uint16_t>(height);
    return desc;
}

ResourceDecodedTextureBridgeRequest BridgeRequest(
    ResourceRegistry &registry,
    NullRhiDevice &device,
    ResourceHandle resource,
    std::span<std::uint8_t> scratch_bytes,
    RhiTextureDesc texture_desc,
    RhiTextureHandle *output_texture) {
    ResourceDecodedTextureBridgeRequest request;
    request.resource_registry = &registry;
    request.rhi_device = &device;
    request.decoded_payload = DecodedPayloadRequest(resource, nullptr, TEXTURE_BYTE_COUNT);
    request.scratch_bytes = scratch_bytes;
    request.texture_desc = texture_desc;
    request.output_texture_handle = output_texture;
    request.staging_request_id = STAGING_ONE;
    request.upload_id = BRIDGE_UPLOAD_ONE;
    request.sampled_texture_slot = 0U;
    return request;
}

int StreamingResourceDecodedTextureBridgeUploadsDecodedPayloadAsTextureBinding() {
    ResourceRegistry registry;
    const ResourceRegistrationResult registration = RegisterTexture(registry);
    if (!registration.Succeeded()) {
        return Fail("texture resource registration failed");
    }

    const std::array<std::uint8_t, TEXTURE_BYTE_COUNT> decoded_bytes{
        255U, 0U, 0U, 255U,
        0U, 255U, 0U, 255U,
        0U, 0U, 255U, 255U,
        255U, 255U, 255U, 255U};
    if (!BuildDecodedTextureResource(registry, registration.handle, decoded_bytes)) {
        return Fail("decoded texture resource setup failed");
    }

    NullRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("null rhi initialize failed");
    }

    std::array<std::uint8_t, TEXTURE_BYTE_COUNT> scratch_bytes{};
    RhiTextureHandle output_texture{};
    ResourceDecodedTextureBridge bridge;
    const ResourceDecodedTextureBridgeRequest request = BridgeRequest(
        registry,
        device,
        registration.handle,
        std::span<std::uint8_t>(scratch_bytes.data(), scratch_bytes.size()),
        TextureDesc(TEXTURE_WIDTH, TEXTURE_HEIGHT),
        &output_texture);
    const ResourceDecodedTextureBridgeResult result = bridge.UploadTexture(request);
    if (result.status != ResourceDecodedTextureBridgeStatus::Success) {
        return Fail("decoded texture bridge did not complete");
    }

    if (result.texture_handle.generation == 0U || output_texture.generation == 0U) {
        return Fail("decoded texture bridge did not create texture handle");
    }

    if (result.texture_handle.generation != output_texture.generation ||
        result.texture_handle.slot != output_texture.slot) {
        return Fail("decoded texture bridge result and output handle diverged");
    }

    if (result.sampled_texture.texture.generation != output_texture.generation) {
        return Fail("decoded texture bridge did not expose sampled texture binding");
    }

    const RhiDeviceSnapshot rhi_snapshot = device.Snapshot();
    if (rhi_snapshot.resources.texture_count != 1U || rhi_snapshot.resources.created_primitive_count != 1U) {
        return Fail("decoded texture bridge did not create one rhi texture");
    }

    MaterialBindingFixture material_binding;
    RenderFixturePassRequest pass_request;
    MaterialBindingFixtureRequest material_request;
    material_request.material_id = MATERIAL_ID;
    material_request.pipeline = RhiPipelineHandle{1U, 1U};
    material_request.sampled_texture = result.sampled_texture;
    material_request.sampler = RhiSamplerBinding{RhiSamplerHandle{1U, 1U}, 0U};
    material_request.pass_id = PASS_ID;
    const auto material_result = material_binding.Bind(material_request, &pass_request);
    if (material_result.status != MaterialBindingFixtureStatus::Success) {
        return Fail("rendercore material binding rejected decoded texture binding");
    }

    if (pass_request.sampled_texture.texture.generation != output_texture.generation) {
        return Fail("rendercore pass did not receive decoded texture binding");
    }

    const ResourceDecodedTextureBridgeSnapshot snapshot = bridge.Snapshot();
    if (snapshot.completed_count != 1U || snapshot.last_uploaded_byte_count != TEXTURE_BYTE_COUNT) {
        return Fail("decoded texture bridge snapshot did not track completion");
    }

    return 0;
}

int StreamingResourceDecodedTextureBridgeRejectsTextureByteMismatchWithoutRhiMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult registration = RegisterTexture(registry);
    if (!registration.Succeeded()) {
        return Fail("mismatch texture resource registration failed");
    }

    const std::array<std::uint8_t, TEXTURE_BYTE_COUNT> decoded_bytes{};
    if (!BuildDecodedTextureResource(registry, registration.handle, decoded_bytes)) {
        return Fail("mismatch decoded texture resource setup failed");
    }

    NullRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("mismatch null rhi initialize failed");
    }

    std::array<std::uint8_t, TEXTURE_BYTE_COUNT> scratch_bytes{};
    RhiTextureHandle output_texture{};
    ResourceDecodedTextureBridge bridge;
    const ResourceDecodedTextureBridgeRequest request = BridgeRequest(
        registry,
        device,
        registration.handle,
        std::span<std::uint8_t>(scratch_bytes.data(), scratch_bytes.size()),
        TextureDesc(1U, 1U),
        &output_texture);
    const ResourceDecodedTextureBridgeResult result = bridge.UploadTexture(request);
    if (result.status != ResourceDecodedTextureBridgeStatus::TextureByteCountMismatch) {
        return Fail("decoded texture bridge did not reject byte mismatch");
    }

    if (device.Snapshot().resources.texture_count != 0U) {
        return Fail("decoded texture mismatch created rhi texture");
    }

    if (output_texture.generation != 0U) {
        return Fail("decoded texture mismatch wrote output handle");
    }

    return 0;
}

int StreamingResourceDecodedTextureBridgeRejectsSmallScratchWithoutRhiMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult registration = RegisterTexture(registry);
    if (!registration.Succeeded()) {
        return Fail("scratch texture resource registration failed");
    }

    const std::array<std::uint8_t, TEXTURE_BYTE_COUNT> decoded_bytes{};
    if (!BuildDecodedTextureResource(registry, registration.handle, decoded_bytes)) {
        return Fail("scratch decoded texture resource setup failed");
    }

    NullRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("scratch null rhi initialize failed");
    }

    std::array<std::uint8_t, 4U> scratch_bytes{};
    RhiTextureHandle output_texture{};
    ResourceDecodedTextureBridge bridge;
    const ResourceDecodedTextureBridgeRequest request = BridgeRequest(
        registry,
        device,
        registration.handle,
        std::span<std::uint8_t>(scratch_bytes.data(), scratch_bytes.size()),
        TextureDesc(TEXTURE_WIDTH, TEXTURE_HEIGHT),
        &output_texture);
    const ResourceDecodedTextureBridgeResult result = bridge.UploadTexture(request);
    if (result.status != ResourceDecodedTextureBridgeStatus::ScratchBufferTooSmall) {
        return Fail("decoded texture bridge did not reject small scratch");
    }

    if (device.Snapshot().resources.texture_count != 0U) {
        return Fail("decoded texture small scratch created rhi texture");
    }

    return 0;
}

int StreamingResourceDecodedTextureBridgeRejectsSampledTextureSlotOutOfRangeWithoutRhiMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult registration = RegisterTexture(registry);
    if (!registration.Succeeded()) {
        return Fail("slot texture resource registration failed");
    }

    NullRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("slot null rhi initialize failed");
    }

    std::array<std::uint8_t, TEXTURE_BYTE_COUNT> scratch_bytes{};
    std::span<std::uint8_t> scratch_span(scratch_bytes.data(), scratch_bytes.size());
    RhiTextureHandle output_texture{};
    ResourceDecodedTextureBridge bridge;
    ResourceDecodedTextureBridgeRequest request = BridgeRequest(
        registry,
        device,
        registration.handle,
        scratch_span,
        TextureDesc(TEXTURE_WIDTH, TEXTURE_HEIGHT),
        &output_texture);
    request.sampled_texture_slot = static_cast<std::uint32_t>(yuengine::rhi::MAX_RHI_SAMPLED_TEXTURE_SLOTS);
    const ResourceDecodedTextureBridgeResult result = bridge.UploadTexture(request);
    if (result.status != ResourceDecodedTextureBridgeStatus::SampledTextureSlotOutOfRange) {
        return Fail("decoded texture bridge accepted out of range sampled slot");
    }

    if (device.Snapshot().resources.texture_count != 0U) {
        return Fail("decoded texture sampled slot rejection created rhi texture");
    }

    if (output_texture.generation != 0U) {
        return Fail("decoded texture sampled slot rejection wrote output handle");
    }

    const ResourceDecodedTextureBridgeSnapshot snapshot = bridge.Snapshot();
    if (snapshot.rejected_count != 1U || snapshot.last_status != result.status) {
        return Fail("decoded texture sampled slot rejection snapshot mismatch");
    }

    return 0;
}

int StreamingResourceDecodedTextureBridgeReportsRhiTextureCapacityWithoutWritingOutput() {
    ResourceRegistry registry;
    const ResourceRegistrationResult registration = RegisterTexture(registry);
    if (!registration.Succeeded()) {
        return Fail("capacity texture resource registration failed");
    }

    const std::array<std::uint8_t, TEXTURE_BYTE_COUNT> decoded_bytes{};
    if (!BuildDecodedTextureResource(registry, registration.handle, decoded_bytes)) {
        return Fail("capacity decoded texture resource setup failed");
    }

    NullRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail("capacity null rhi initialize failed");
    }

    std::array<std::uint8_t, TEXTURE_BYTE_COUNT> scratch_bytes{};
    std::span<std::uint8_t> scratch_span(scratch_bytes.data(), scratch_bytes.size());
    ResourceDecodedTextureBridge bridge;
    for (std::size_t index = 0U; index < yuengine::rhi::MAX_RHI_TEXTURES; ++index) {
        RhiTextureHandle output_texture{};
        ResourceDecodedTextureBridgeRequest request = BridgeRequest(
            registry,
            device,
            registration.handle,
            scratch_span,
            TextureDesc(TEXTURE_WIDTH, TEXTURE_HEIGHT),
            &output_texture);
        request.upload_id = BRIDGE_UPLOAD_ONE + static_cast<std::uint64_t>(index);
        const ResourceDecodedTextureBridgeResult result = bridge.UploadTexture(request);
        if (result.status != ResourceDecodedTextureBridgeStatus::Success) {
            return Fail("decoded texture bridge did not fill rhi texture capacity");
        }
    }

    if (device.Snapshot().resources.texture_count != yuengine::rhi::MAX_RHI_TEXTURES) {
        return Fail("decoded texture bridge did not fill expected rhi texture capacity");
    }

    RhiTextureHandle overflow_texture{};
    ResourceDecodedTextureBridgeRequest overflow_request = BridgeRequest(
        registry,
        device,
        registration.handle,
        scratch_span,
        TextureDesc(TEXTURE_WIDTH, TEXTURE_HEIGHT),
        &overflow_texture);
    overflow_request.upload_id =
        BRIDGE_UPLOAD_ONE + static_cast<std::uint64_t>(yuengine::rhi::MAX_RHI_TEXTURES);
    const ResourceDecodedTextureBridgeResult overflow_result = bridge.UploadTexture(overflow_request);
    if (overflow_result.status != ResourceDecodedTextureBridgeStatus::UploadProcessFailed) {
        return Fail("decoded texture bridge did not report rhi capacity failure");
    }

    if (overflow_result.upload_status != ResourceUploadStatus::RhiUploadFailed) {
        return Fail("decoded texture bridge did not expose rhi upload failure status");
    }

    if (overflow_result.rhi_status != RhiStatus::CapacityExceeded) {
        return Fail("decoded texture bridge did not expose rhi capacity status");
    }

    if (overflow_texture.generation != 0U) {
        return Fail("decoded texture bridge wrote overflow output handle");
    }

    const RhiDeviceSnapshot rhi_snapshot = device.Snapshot();
    if (rhi_snapshot.resources.texture_count != yuengine::rhi::MAX_RHI_TEXTURES) {
        return Fail("decoded texture bridge mutated rhi texture count after capacity failure");
    }

    const ResourceDecodedTextureBridgeSnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.failed_count != 1U || bridge_snapshot.last_rhi_status != RhiStatus::CapacityExceeded) {
        return Fail("decoded texture bridge capacity snapshot mismatch");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_UPLOADS_TEXTURE) {
        return StreamingResourceDecodedTextureBridgeUploadsDecodedPayloadAsTextureBinding();
    }

    if (name == TEST_REJECTS_MISMATCH) {
        return StreamingResourceDecodedTextureBridgeRejectsTextureByteMismatchWithoutRhiMutation();
    }

    if (name == TEST_REJECTS_SMALL_SCRATCH) {
        return StreamingResourceDecodedTextureBridgeRejectsSmallScratchWithoutRhiMutation();
    }

    if (name == TEST_REJECTS_SAMPLED_TEXTURE_SLOT) {
        return StreamingResourceDecodedTextureBridgeRejectsSampledTextureSlotOutOfRangeWithoutRhiMutation();
    }

    if (name == TEST_REPORTS_RHI_CAPACITY) {
        return StreamingResourceDecodedTextureBridgeReportsRhiTextureCapacityWithoutWritingOutput();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
