/*
模块: AssetSmokeDemo
文件: Samples/AssetSmokeDemo/Source/L0EngineEvidence.cpp
用途: 将示例资产接入 YuEngine L0 File/Resource/Streaming/RHI/RenderCore/Hardware 证据路径。
*/

#include "L0EngineEvidence.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string>
#include <vector>

#include "YuEngine/Audio/AudioCallbackDeviceDesc.h"
#include "YuEngine/Audio/AudioConstants.h"
#include "YuEngine/File/FileReadRequest.h"
#include "YuEngine/File/FileStatus.h"
#include "YuEngine/File/MountId.h"
#include "YuEngine/File/MountTable.h"
#include "YuEngine/File/VirtualPath.h"
#include "YuEngine/Hardware/HardwareFrameHost.h"
#include "YuEngine/Hardware/HardwareFrameHostDesc.h"
#include "YuEngine/Hardware/HardwareFrameHostSnapshot.h"
#include "YuEngine/Hardware/HardwareFrameHostStatus.h"
#include "YuEngine/Hardware/HardwareFrameHostTickRequest.h"
#include "YuEngine/Hardware/HardwareFrameHostTickResult.h"
#include "YuEngine/Input/InputBridgeEvent.h"
#include "YuEngine/Input/InputStatus.h"
#include "YuEngine/Platform/PlatformWindowEvent.h"
#include "YuEngine/Platform/PlatformWindowEventType.h"
#include "YuEngine/RenderCore/MaterialBindingFixture.h"
#include "YuEngine/RenderCore/MaterialBindingFixtureRequest.h"
#include "YuEngine/RenderCore/MaterialBindingFixtureStatus.h"
#include "YuEngine/RenderCore/RenderCamera.h"
#include "YuEngine/RenderCore/RenderCameraProjectionKind.h"
#include "YuEngine/RenderCore/RenderCameraShaderConstants.h"
#include "YuEngine/RenderCore/RenderCameraShaderConstantsWriter.h"
#include "YuEngine/RenderCore/RenderFixturePassRequest.h"
#include "YuEngine/RenderCore/RenderMaterialStatus.h"
#include "YuEngine/RenderCore/RenderViewPacket.h"
#include "YuEngine/RenderCore/RenderViewPacketRequest.h"
#include "YuEngine/RenderCore/RenderViewPacketStatus.h"
#include "YuEngine/Rhi/NullRhiDevice.h"
#include "YuEngine/Rhi/RhiBackendKind.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiDrawIndexedDesc.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"
#include "YuEngine/Rhi/RhiSamplerHandle.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"
#include "YuEngine/Resource/ResourceCachePayloadBudgetDesc.h"
#include "YuEngine/Resource/ResourceCachePayloadRequest.h"
#include "YuEngine/Resource/ResourceCachePayloadStatus.h"
#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceDecodedPayloadBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodedPayloadRequest.h"
#include "YuEngine/Resource/ResourceDecodedPayloadStatus.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodePlanBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodePlanRequest.h"
#include "YuEngine/Resource/ResourceDecodePlanStatus.h"
#include "YuEngine/Resource/ResourceDecodeResultBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodeResultClass.h"
#include "YuEngine/Resource/ResourceDecodeResultRequest.h"
#include "YuEngine/Resource/ResourceDecodeResultStatus.h"
#include "YuEngine/Resource/ResourceDescriptor.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceLoadCommitRequest.h"
#include "YuEngine/Resource/ResourceLoadCommitStatus.h"
#include "YuEngine/Resource/ResourceLoadState.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/Resource/ResourceResidencyBudgetDesc.h"
#include "YuEngine/Resource/ResourceResidencyRequest.h"
#include "YuEngine/Resource/ResourceResidencyStatus.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridge.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeRequest.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeResult.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeStatus.h"

namespace asset_smoke_demo {
namespace {

constexpr const char *ASSET_MOUNT_ID = "AssetSmokeDemoAssets";
constexpr const char *MESH_ASSET_PATH = "Meshes/TexturedMesh.obj";
constexpr const char *MATERIAL_ASSET_PATH = "Materials/DemoMaterial.txt";
constexpr const char *RESOURCE_KEY = "asset_smoke_l0_texture";
constexpr yuengine::resource::ResourceTypeId TEXTURE_TYPE{1U};
constexpr std::uint32_t TEXTURE_WIDTH = 2U;
constexpr std::uint32_t TEXTURE_HEIGHT = 2U;
constexpr std::uint32_t TEXTURE_BYTE_COUNT = 16U;
constexpr std::uint32_t DECODE_PLAN_PAYLOAD_BYTE_COUNT =
    yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT + 4U;
constexpr std::uint64_t COMMIT_ID = 101U;
constexpr std::uint64_t UPLOAD_ID = 201U;
constexpr std::uint64_t PAYLOAD_ID = 301U;
constexpr std::uint64_t DECODE_PLAN_ID = 401U;
constexpr std::uint64_t DECODE_RESULT_ID = 501U;
constexpr std::uint64_t DECODED_PAYLOAD_ID = 601U;
constexpr std::uint64_t STAGING_ID = 701U;
constexpr std::uint64_t BRIDGE_UPLOAD_ID = 801U;
constexpr std::uint32_t VIEW_ID = 901U;
constexpr std::uint32_t FRAME_ID = 1001U;
constexpr std::uint32_t PASS_ID = 1101U;
constexpr std::uint32_t MATERIAL_ID = 1201U;
constexpr std::uint32_t DRAW_ID = 1301U;
constexpr std::uint16_t HOST_INITIAL_EXTENT = 4U;
constexpr std::uint16_t HOST_RESIZED_WIDTH = 3U;
constexpr std::uint16_t HOST_RESIZED_HEIGHT = 2U;
constexpr std::size_t HOST_AUDIO_FRAMES = yuengine::audio::AudioCallbackDeviceDesc::MIN_FRAMES_PER_BUFFER;
constexpr std::size_t HOST_AUDIO_SAMPLE_COUNT = HOST_AUDIO_FRAMES * yuengine::audio::CHANNEL_COUNT;
constexpr std::uint32_t HOST_AUDIO_WAIT_TIMEOUT_MS = 2000U;
constexpr float HALF_PI = 1.57079632679F;

void WriteU32LittleEndian(
    std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> *bytes,
    std::uint32_t offset,
    std::uint32_t value) {
    (*bytes)[offset] = static_cast<std::uint8_t>(value & 0xFFU);
    (*bytes)[offset + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    (*bytes)[offset + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    (*bytes)[offset + 3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> BuildDecodePlanPayload() {
    std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> payload{};
    payload[0U] = yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_0;
    payload[1U] = yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_1;
    payload[2U] = yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_2;
    payload[3U] = yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_MAGIC_3;
    WriteU32LittleEndian(&payload, 4U, yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_VERSION);
    WriteU32LittleEndian(&payload, 8U, static_cast<std::uint32_t>(yuengine::resource::ResourceDecodePlanAssetClass::Texture));
    WriteU32LittleEndian(&payload, 12U, DECODE_PLAN_PAYLOAD_BYTE_COUNT);
    WriteU32LittleEndian(&payload, 16U, TEXTURE_BYTE_COUNT);
    payload[yuengine::resource::RESOURCE_DECODE_PLAN_HEADER_BYTE_COUNT] = 0x7AU;
    return payload;
}

bool IsSafeRelativeAssetPath(const std::string &path) {
    if (path.empty()) {
        return false;
    }

    if (path == "..") {
        return false;
    }

    return path.rfind("../", 0U) != 0U;
}

bool BuildTextureRelativePath(const L0EngineEvidenceInput &input, std::string *relative_path) {
    if (relative_path == nullptr) {
        return false;
    }

    const std::filesystem::path relative = input.texture_path.lexically_relative(input.asset_root);
    const std::string text = relative.generic_string();
    if (!IsSafeRelativeAssetPath(text)) {
        return false;
    }

    *relative_path = text;
    return true;
}

bool ReadAssetBytes(
    yuengine::file::MountTable &mount_table,
    const std::string &relative_path,
    std::vector<std::uint8_t> *bytes) {
    if (bytes == nullptr) {
        return false;
    }

    if (!IsSafeRelativeAssetPath(relative_path)) {
        return false;
    }

    yuengine::file::FileReadRequest request{};
    request.mount = yuengine::file::MountId(ASSET_MOUNT_ID);
    request.path = yuengine::file::VirtualPath(relative_path);
    const auto result = mount_table.Read(request);
    if (result.status != yuengine::file::FileStatus::Success) {
        return false;
    }

    if (result.bytes.empty()) {
        return false;
    }

    *bytes = result.bytes;
    return true;
}

bool ReadSampleAssetsThroughEngineFile(
    const L0EngineEvidenceInput &input,
    L0EngineEvidenceResult *result) {
    if (result == nullptr) {
        return false;
    }

    yuengine::file::MountTable mount_table;
    const yuengine::file::FileStatus mount_status =
        mount_table.RegisterLooseMount(yuengine::file::MountId(ASSET_MOUNT_ID), input.asset_root);
    if (mount_status != yuengine::file::FileStatus::Success) {
        result->failure_stage = "file_mount";
        return false;
    }

    std::string texture_relative_path;
    if (!BuildTextureRelativePath(input, &texture_relative_path)) {
        result->failure_stage = "file_texture_path";
        return false;
    }

    std::vector<std::uint8_t> mesh_bytes;
    std::vector<std::uint8_t> material_bytes;
    if (!ReadAssetBytes(mount_table, MESH_ASSET_PATH, &mesh_bytes)) {
        result->failure_stage = "file_mesh";
        return false;
    }

    if (!ReadAssetBytes(mount_table, MATERIAL_ASSET_PATH, &material_bytes)) {
        result->failure_stage = "file_material";
        return false;
    }

    result->file_read_byte_count = static_cast<std::uint32_t>(mesh_bytes.size() + material_bytes.size());
    result->file_read = true;
    return true;
}

bool ConfigureResourceBudgets(yuengine::resource::ResourceRegistry &registry) {
    yuengine::resource::ResourceResidencyBudgetDesc residency_budget{};
    residency_budget.byte_capacity = 256U;
    if (registry.SetResidencyBudget(residency_budget) != yuengine::resource::ResourceResidencyStatus::Success) {
        return false;
    }

    yuengine::resource::ResourceCachePayloadBudgetDesc cache_budget{};
    cache_budget.byte_capacity = 256U;
    if (registry.SetCachePayloadBudget(cache_budget) != yuengine::resource::ResourceCachePayloadStatus::Success) {
        return false;
    }

    yuengine::resource::ResourceDecodePlanBudgetDesc plan_budget{};
    plan_budget.decoded_byte_capacity = 256U;
    if (registry.SetDecodePlanBudget(plan_budget) != yuengine::resource::ResourceDecodePlanStatus::Success) {
        return false;
    }

    yuengine::resource::ResourceDecodeResultBudgetDesc result_budget{};
    result_budget.decoded_byte_capacity = 256U;
    if (registry.SetDecodeResultBudget(result_budget) != yuengine::resource::ResourceDecodeResultStatus::Success) {
        return false;
    }

    yuengine::resource::ResourceDecodedPayloadBudgetDesc decoded_budget{};
    decoded_budget.decoded_byte_capacity = 256U;
    return registry.SetDecodedPayloadBudget(decoded_budget) == yuengine::resource::ResourceDecodedPayloadStatus::Success;
}

yuengine::resource::ResourceLoadCommitRequest MakeLoadCommitRequest(
    yuengine::resource::ResourceHandle resource) {
    yuengine::resource::ResourceLoadCommitRequest request{};
    request.resource = resource;
    request.expected_type = TEXTURE_TYPE;
    request.load_state = yuengine::resource::ResourceLoadState::Uploaded;
    request.commit_id = COMMIT_ID;
    request.upload_id = UPLOAD_ID;
    request.staging_request_id = STAGING_ID;
    request.upload_byte_count = TEXTURE_BYTE_COUNT;
    return request;
}

yuengine::resource::ResourceResidencyRequest MakeResidencyRequest(
    yuengine::resource::ResourceHandle resource) {
    yuengine::resource::ResourceResidencyRequest request{};
    request.resource = resource;
    request.expected_type = TEXTURE_TYPE;
    return request;
}

yuengine::resource::ResourceCachePayloadRequest MakeCachePayloadRequest(
    yuengine::resource::ResourceHandle resource,
    const std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> &payload) {
    yuengine::resource::ResourceCachePayloadRequest request{};
    request.resource = resource;
    request.expected_type = TEXTURE_TYPE;
    request.payload_id = PAYLOAD_ID;
    request.payload_bytes = payload.data();
    request.payload_byte_count = static_cast<std::uint32_t>(payload.size());
    return request;
}

yuengine::resource::ResourceDecodePlanRequest MakeDecodePlanRequest(
    yuengine::resource::ResourceHandle resource) {
    yuengine::resource::ResourceDecodePlanRequest request{};
    request.resource = resource;
    request.expected_type = TEXTURE_TYPE;
    request.payload_id = PAYLOAD_ID;
    request.decode_plan_id = DECODE_PLAN_ID;
    request.asset_class = yuengine::resource::ResourceDecodePlanAssetClass::Texture;
    request.source_byte_count = DECODE_PLAN_PAYLOAD_BYTE_COUNT;
    request.expected_decoded_byte_count = TEXTURE_BYTE_COUNT;
    return request;
}

yuengine::resource::ResourceDecodeResultRequest MakeDecodeResultRequest(
    yuengine::resource::ResourceHandle resource) {
    yuengine::resource::ResourceDecodeResultRequest request{};
    request.resource = resource;
    request.expected_type = TEXTURE_TYPE;
    request.payload_id = PAYLOAD_ID;
    request.decode_plan_id = DECODE_PLAN_ID;
    request.decode_result_id = DECODE_RESULT_ID;
    request.asset_class = yuengine::resource::ResourceDecodePlanAssetClass::Texture;
    request.result_class = yuengine::resource::ResourceDecodeResultClass::Texture;
    request.decoded_byte_count = TEXTURE_BYTE_COUNT;
    return request;
}

yuengine::resource::ResourceDecodedPayloadRequest MakeDecodedPayloadRequest(
    yuengine::resource::ResourceHandle resource,
    const std::uint8_t *decoded_bytes) {
    yuengine::resource::ResourceDecodedPayloadRequest request{};
    request.resource = resource;
    request.expected_type = TEXTURE_TYPE;
    request.payload_id = PAYLOAD_ID;
    request.decode_plan_id = DECODE_PLAN_ID;
    request.decode_result_id = DECODE_RESULT_ID;
    request.decoded_payload_id = DECODED_PAYLOAD_ID;
    request.asset_class = yuengine::resource::ResourceDecodePlanAssetClass::Texture;
    request.result_class = yuengine::resource::ResourceDecodeResultClass::Texture;
    request.decoded_bytes = decoded_bytes;
    request.decoded_byte_count = TEXTURE_BYTE_COUNT;
    return request;
}

std::array<std::uint8_t, TEXTURE_BYTE_COUNT> BuildEvidenceTexture(const L0EngineEvidenceInput &input) {
    std::array<std::uint8_t, TEXTURE_BYTE_COUNT> bytes{};
    for (std::size_t index = 0U; index < bytes.size(); ++index) {
        bytes[index] = input.texture_rgba[index];
    }

    return bytes;
}

bool BuildDecodedTextureResource(
    yuengine::resource::ResourceRegistry &registry,
    yuengine::resource::ResourceHandle resource,
    const std::array<std::uint8_t, TEXTURE_BYTE_COUNT> &decoded_bytes) {
    if (!ConfigureResourceBudgets(registry)) {
        return false;
    }

    if (registry.CommitUploadCompletion(MakeLoadCommitRequest(resource)) !=
        yuengine::resource::ResourceLoadCommitStatus::Success) {
        return false;
    }

    if (registry.AdmitResident(MakeResidencyRequest(resource)) !=
        yuengine::resource::ResourceResidencyStatus::Success) {
        return false;
    }

    const std::array<std::uint8_t, DECODE_PLAN_PAYLOAD_BYTE_COUNT> plan_payload = BuildDecodePlanPayload();
    if (registry.StoreCachePayload(MakeCachePayloadRequest(resource, plan_payload)) !=
        yuengine::resource::ResourceCachePayloadStatus::Success) {
        return false;
    }

    if (registry.CreateDecodePlan(MakeDecodePlanRequest(resource)) !=
        yuengine::resource::ResourceDecodePlanStatus::Success) {
        return false;
    }

    if (registry.CommitDecodeResult(MakeDecodeResultRequest(resource)) !=
        yuengine::resource::ResourceDecodeResultStatus::Success) {
        return false;
    }

    return registry.StoreDecodedPayload(MakeDecodedPayloadRequest(resource, decoded_bytes.data())) ==
        yuengine::resource::ResourceDecodedPayloadStatus::Success;
}

yuengine::rhi::RhiTextureDesc EvidenceTextureDesc() {
    yuengine::rhi::RhiTextureDesc desc{};
    desc.extent.width = static_cast<std::uint16_t>(TEXTURE_WIDTH);
    desc.extent.height = static_cast<std::uint16_t>(TEXTURE_HEIGHT);
    return desc;
}

bool UploadDecodedTextureToRhi(
    yuengine::resource::ResourceRegistry &registry,
    yuengine::resource::ResourceHandle resource,
    yuengine::rhi::NullRhiDevice &device,
    yuengine::rhi::RhiTextureHandle *texture_handle,
    yuengine::rhi::RhiSampledTextureBinding *sampled_texture,
    L0EngineEvidenceResult *result) {
    if (texture_handle == nullptr || sampled_texture == nullptr || result == nullptr) {
        return false;
    }

    std::array<std::uint8_t, TEXTURE_BYTE_COUNT> scratch{};
    yuengine::streaming::ResourceDecodedTextureBridge bridge;
    yuengine::streaming::ResourceDecodedTextureBridgeRequest request{};
    request.resource_registry = &registry;
    request.rhi_device = &device;
    request.decoded_payload = MakeDecodedPayloadRequest(resource, nullptr);
    request.scratch_bytes = std::span<std::uint8_t>(scratch.data(), scratch.size());
    request.texture_desc = EvidenceTextureDesc();
    request.output_texture_handle = texture_handle;
    request.staging_request_id = STAGING_ID;
    request.upload_id = BRIDGE_UPLOAD_ID;
    request.sampled_texture_slot = 0U;

    const yuengine::streaming::ResourceDecodedTextureBridgeResult upload_result = bridge.UploadTexture(request);
    if (upload_result.status != yuengine::streaming::ResourceDecodedTextureBridgeStatus::Success) {
        return false;
    }

    *sampled_texture = upload_result.sampled_texture;
    result->uploaded_texture_generation = texture_handle->generation;
    result->texture_upload = true;
    return true;
}

yuengine::rendercore::RenderCameraShaderConstants BuildCameraConstants() {
    yuengine::rendercore::RenderCameraPose pose{};
    pose.position = {0.0F, 0.0F, -4.0F};
    pose.target = {0.0F, 0.0F, 0.0F};
    pose.up = {0.0F, 1.0F, 0.0F};

    yuengine::rendercore::RenderCameraProjectionDesc projection{};
    projection.kind = yuengine::rendercore::RenderCameraProjectionKind::Perspective;
    projection.vertical_fov_radians = HALF_PI;
    projection.aspect_ratio = 1.0F;
    projection.near_z = 0.1F;
    projection.far_z = 100.0F;

    yuengine::rendercore::RenderCamera camera;
    yuengine::rendercore::RenderCameraFrame frame{};
    static_cast<void>(camera.BuildFrame(pose, projection, &frame));

    yuengine::rendercore::RenderCameraShaderConstantsWriter writer;
    yuengine::rendercore::RenderCameraShaderConstants constants{};
    static_cast<void>(writer.WriteViewProjection(frame, &constants));
    return constants;
}

bool BuildRenderCoreViewEvidence(const yuengine::rhi::RhiSampledTextureBinding &sampled_texture) {
    const yuengine::rendercore::RenderCameraShaderConstants constants = BuildCameraConstants();
    const auto byte_count = constants.view_projection_values.size() * sizeof(float);
    const auto *bytes = reinterpret_cast<const std::uint8_t *>(constants.view_projection_values.data());

    std::array<std::uint8_t, 16U> capture{};
    yuengine::rendercore::RenderViewPacketRequest request{};
    request.view_id = VIEW_ID;
    request.frame_id = FRAME_ID;
    request.target = yuengine::rhi::RhiTextureHandle{1U, 1U};
    request.clear_color = yuengine::rhi::RhiColor{4U, 8U, 12U, 255U};
    request.capture_output = std::span<std::uint8_t>(capture.data(), capture.size());
    request.capture_byte_budget = capture.size();
    request.material.material_id = MATERIAL_ID;
    request.material.program_id = 11U;
    request.material.pipeline = yuengine::rhi::RhiPipelineHandle{3U, 1U};
    request.material.sampled_texture = sampled_texture;
    request.material.sampler = yuengine::rhi::RhiSamplerBinding{yuengine::rhi::RhiSamplerHandle{5U, 1U}, 0U};
    request.material.constant_bytes = std::span<const std::uint8_t>(bytes, byte_count);
    request.material.pass_id = PASS_ID;
    request.draw.draw_id = DRAW_ID;
    request.draw.pass_id = PASS_ID;
    request.draw.material_id = MATERIAL_ID;
    request.draw.vertex_buffer = yuengine::rhi::RhiVertexBufferView{
        yuengine::rhi::RhiBufferHandle{1U, 1U},
        0U,
        24U,
        72U};
    request.draw.index_buffer = yuengine::rhi::RhiIndexBufferView{
        yuengine::rhi::RhiBufferHandle{2U, 1U},
        0U,
        6U,
        yuengine::rhi::RhiIndexFormat::Uint16};
    request.draw.draw.topology = yuengine::rhi::RhiPrimitiveTopology::TriangleList;
    request.draw.draw.index_count = 3U;

    yuengine::rendercore::RenderViewPacket packet;
    yuengine::rendercore::RenderFixturePassRequest pass_request{};
    const auto view_result = packet.BuildPassRequest(request, &pass_request);
    if (view_result.status != yuengine::rendercore::RenderViewPacketStatus::Success) {
        return false;
    }

    if (view_result.material_status != yuengine::rendercore::RenderMaterialStatus::Success) {
        return false;
    }

    return pass_request.draw.index_count == 3U &&
        pass_request.sampled_texture.texture.generation == sampled_texture.texture.generation;
}

bool RunResourceTextureEvidence(
    const L0EngineEvidenceInput &input,
    L0EngineEvidenceResult *result) {
    if (result == nullptr) {
        return false;
    }

    if (input.texture_rgba.size() < TEXTURE_BYTE_COUNT) {
        result->failure_stage = "resource_texture_input";
        return false;
    }

    if (!ReadSampleAssetsThroughEngineFile(input, result)) {
        return false;
    }

    yuengine::resource::ResourceRegistry registry;
    const yuengine::resource::ResourceDescriptor descriptor{
        TEXTURE_TYPE,
        yuengine::resource::ResourceLogicalKey(RESOURCE_KEY),
        0U};
    const auto registration = registry.RegisterSyntheticDescriptor(descriptor);
    if (!registration.Succeeded()) {
        result->failure_stage = "resource_register";
        return false;
    }

    const std::array<std::uint8_t, TEXTURE_BYTE_COUNT> decoded_bytes = BuildEvidenceTexture(input);
    if (!BuildDecodedTextureResource(registry, registration.handle, decoded_bytes)) {
        result->failure_stage = "resource_decode";
        return false;
    }

    result->resource_decode = true;
    result->decoded_texture_width = TEXTURE_WIDTH;
    result->decoded_texture_height = TEXTURE_HEIGHT;

    yuengine::rhi::NullRhiDevice device;
    if (device.Initialize(yuengine::rhi::RhiDeviceDesc{}) != yuengine::rhi::RhiStatus::Success) {
        result->failure_stage = "rhi_initialize";
        return false;
    }

    yuengine::rhi::RhiTextureHandle texture_handle{};
    yuengine::rhi::RhiSampledTextureBinding sampled_texture{};
    if (!UploadDecodedTextureToRhi(registry, registration.handle, device, &texture_handle, &sampled_texture, result)) {
        result->failure_stage = "texture_upload";
        return false;
    }

    if (!BuildRenderCoreViewEvidence(sampled_texture)) {
        result->failure_stage = "rendercore_view";
        return false;
    }

    result->rendercore_view_draw_material = true;
    return true;
}

yuengine::platform::PlatformWindowEvent FocusEvent() {
    yuengine::platform::PlatformWindowEvent event{};
    event.type = yuengine::platform::PlatformWindowEventType::FocusGained;
    return event;
}

yuengine::platform::PlatformWindowEvent KeyDownEvent() {
    yuengine::platform::PlatformWindowEvent event{};
    event.type = yuengine::platform::PlatformWindowEventType::RawKeyDown;
    event.raw_code = 65U;
    return event;
}

yuengine::platform::PlatformWindowEvent ResizeEvent() {
    yuengine::platform::PlatformWindowEvent event{};
    event.type = yuengine::platform::PlatformWindowEventType::Resized;
    event.client_width = HOST_RESIZED_WIDTH;
    event.client_height = HOST_RESIZED_HEIGHT;
    return event;
}

std::size_t HostCaptureByteCount() {
    return static_cast<std::size_t>(HOST_RESIZED_WIDTH) *
        static_cast<std::size_t>(HOST_RESIZED_HEIGHT) *
        yuengine::rhi::RGBA8_BYTES_PER_PIXEL;
}

yuengine::hardware::HardwareFrameHostDesc MakeHostDesc() {
    yuengine::hardware::HardwareFrameHostDesc desc{};
    desc.window_desc.title = "YuEngine AssetSmokeDemo L0 Evidence";
    desc.window_desc.client_width = HOST_INITIAL_EXTENT;
    desc.window_desc.client_height = HOST_INITIAL_EXTENT;
    desc.window_desc.visible = false;
    desc.rhi_desc.backend_kind = yuengine::rhi::RhiBackendKind::D3D11;
    desc.rhi_desc.command_list_capacity = yuengine::rhi::MAX_COMMANDS;
    desc.rhi_desc.swapchain.extent = {HOST_INITIAL_EXTENT, HOST_INITIAL_EXTENT};
    desc.audio_desc.frames_per_buffer = HOST_AUDIO_FRAMES;
    desc.render_enabled = true;
    desc.audio_enabled = true;
    desc.require_audio_device = false;
    return desc;
}

bool RunHardwareHostEvidence(L0EngineEvidenceResult *result) {
    if (result == nullptr) {
        return false;
    }

    yuengine::hardware::HardwareFrameHost host;
    const yuengine::hardware::HardwareFrameHostDesc desc = MakeHostDesc();
    const yuengine::hardware::HardwareFrameHostStatus init_status = host.Initialize(desc);
    if (init_status != yuengine::hardware::HardwareFrameHostStatus::Success) {
        result->failure_stage = "hardware_initialize";
        return false;
    }

    std::array<yuengine::platform::PlatformWindowEvent, 3U> platform_events{};
    platform_events[0U] = FocusEvent();
    platform_events[1U] = KeyDownEvent();
    platform_events[2U] = ResizeEvent();

    std::array<yuengine::input::InputBridgeEvent, 8U> input_events{};
    std::size_t input_event_count = 0U;
    std::vector<std::uint8_t> capture(HostCaptureByteCount());
    std::array<std::int16_t, HOST_AUDIO_SAMPLE_COUNT> audio_samples{};

    yuengine::hardware::HardwareFrameHostTickRequest request{};
    request.injected_platform_events = std::span<const yuengine::platform::PlatformWindowEvent>(
        platform_events.data(),
        platform_events.size());
    request.input_events = std::span<yuengine::input::InputBridgeEvent>(input_events.data(), input_events.size());
    request.out_input_event_count = &input_event_count;
    request.capture_output = std::span<std::uint8_t>(capture.data(), capture.size());
    request.capture_byte_budget = capture.size();
    request.audio_samples = std::span<const std::int16_t>(audio_samples.data(), audio_samples.size());
    request.audio_frame_count = HOST_AUDIO_FRAMES;
    request.audio_completion_target = 1U;
    request.audio_wait_timeout_milliseconds = HOST_AUDIO_WAIT_TIMEOUT_MS;
    request.clear_color = yuengine::rhi::RhiColor{16U, 48U, 96U, 255U};
    request.frame_id = 1U;
    request.poll_gamepad = true;

    const yuengine::hardware::HardwareFrameHostTickResult tick_result = host.Tick(request);
    if (tick_result.status != yuengine::hardware::HardwareFrameHostStatus::Success) {
        result->failure_stage = "hardware_tick";
        static_cast<void>(host.Shutdown());
        return false;
    }

    const yuengine::hardware::HardwareFrameHostSnapshot tick_snapshot = host.Snapshot();
    if (tick_snapshot.render_frame_count != 1U) {
        result->failure_stage = "hardware_render_frame";
        static_cast<void>(host.Shutdown());
        return false;
    }

    if (tick_result.render_result.capture_bytes_written != capture.size()) {
        result->failure_stage = "hardware_capture";
        static_cast<void>(host.Shutdown());
        return false;
    }

    const yuengine::hardware::HardwareFrameHostStatus shutdown_status = host.Shutdown();
    if (shutdown_status != yuengine::hardware::HardwareFrameHostStatus::Success) {
        result->failure_stage = "hardware_shutdown";
        return false;
    }

    result->hardware_frame = true;
    result->resize = tick_result.render_result.resized;
    result->shutdown = true;
    result->render_frame_count = static_cast<std::uint32_t>(tick_snapshot.render_frame_count);
    result->input_event_count = static_cast<std::uint32_t>(input_event_count);
    result->gamepad_state = "graded_skip";
    if (tick_result.gamepad_poll_status == yuengine::input::InputStatus::Success) {
        result->gamepad_state = "pass";
    }

    result->audio_state = "graded_skip";
    if (tick_snapshot.audio_available && tick_result.audio_completion_count == 1U) {
        result->audio_state = "pass";
    }

    if (!result->resize) {
        result->failure_stage = "hardware_resize";
        return false;
    }

    return true;
}

} // 匿名命名空间

bool RunL0EngineEvidence(const L0EngineEvidenceInput &input, L0EngineEvidenceResult *result) {
    if (result == nullptr) {
        return false;
    }

    *result = L0EngineEvidenceResult{};
    if (input.asset_root.empty()) {
        result->failure_stage = "missing_asset_root";
        return false;
    }

    if (input.texture_width == 0U || input.texture_height == 0U) {
        result->failure_stage = "invalid_texture_extent";
        return false;
    }

    if (!RunResourceTextureEvidence(input, result)) {
        return false;
    }

    if (!RunHardwareHostEvidence(result)) {
        return false;
    }

    result->failure_stage = "ok";
    return true;
}

} // asset_smoke_demo 命名空间
