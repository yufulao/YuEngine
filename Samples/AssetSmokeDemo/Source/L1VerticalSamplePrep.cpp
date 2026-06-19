/*
模块: AssetSmokeDemo
文件: Samples/AssetSmokeDemo/Source/L1VerticalSamplePrep.cpp
用途: L1 纵向 sample synthetic scene manifest 和 submit prep。
*/

#include "L1VerticalSamplePrep.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "YuEngine/Asset/AssetAudioReadyRecord.h"
#include "YuEngine/Asset/AssetDescriptor.h"
#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Asset/AssetManager.h"
#include "YuEngine/Asset/AssetRecord.h"
#include "YuEngine/Asset/AssetStatus.h"
#include "YuEngine/Asset/AssetTextureReadyRecord.h"
#include "YuEngine/Asset/AssetTypeId.h"
#include "YuEngine/Audio/AudioPcmSamplePacketHandle.h"
#include "YuEngine/Audio/AudioPcmSamplePacketRequest.h"
#include "YuEngine/Audio/AudioPcmStreamQueueRequest.h"
#include "YuEngine/Audio/AudioSampleFormat.h"
#include "YuEngine/Audio/AudioSourceId.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportHandle.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportRecord.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportStatus.h"
#include "YuEngine/AudioScene/AudioSceneContractQueue.h"
#include "YuEngine/AudioScene/AudioSceneSourceRecord.h"
#include "YuEngine/AudioScene/AudioSceneSourceState.h"
#include "YuEngine/AudioScene/AudioSceneStatus.h"
#include "YuEngine/AudioScene/AudioSceneSubmitRequest.h"
#include "YuEngine/AudioScene/AudioSceneSubmitResult.h"
#include "YuEngine/Input/InputCommandBinding.h"
#include "YuEngine/Input/InputCommandMapper.h"
#include "YuEngine/Input/InputCommandSnapshot.h"
#include "YuEngine/Input/InputCommandValueKind.h"
#include "YuEngine/Input/InputContextFocusMode.h"
#include "YuEngine/Input/InputContextId.h"
#include "YuEngine/Input/InputControlId.h"
#include "YuEngine/Input/InputDeviceId.h"
#include "YuEngine/Input/InputEvent.h"
#include "YuEngine/Input/InputEventType.h"
#include "YuEngine/Input/InputStatus.h"
#include "YuEngine/Kernel/EngineKernel.h"
#include "YuEngine/Kernel/RuntimeApp.h"
#include "YuEngine/Kernel/RuntimeAppDesc.h"
#include "YuEngine/Kernel/RuntimeAppStatus.h"
#include "YuEngine/Kernel/RuntimeFramePhase.h"
#include "YuEngine/Object/ObjectDescriptor.h"
#include "YuEngine/Object/ObjectTypeId.h"
#include "YuEngine/RenderCore/RenderDrawPacketRequest.h"
#include "YuEngine/RenderCore/RenderMaterialRequest.h"
#include "YuEngine/RenderCore/RenderViewPacketRequest.h"
#include "YuEngine/RenderScene/RenderSceneCameraRecord.h"
#include "YuEngine/RenderScene/RenderSceneContractQueue.h"
#include "YuEngine/RenderScene/RenderSceneEntityRecord.h"
#include "YuEngine/RenderScene/RenderSceneStatus.h"
#include "YuEngine/RenderScene/RenderSceneSubmitRequest.h"
#include "YuEngine/RenderScene/RenderSceneSubmitResult.h"
#include "YuEngine/Resource/ResourceDecodedPayloadRecord.h"
#include "YuEngine/Resource/ResourceDecodedPayloadStatus.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodeResultClass.h"
#include "YuEngine/Resource/ResourceDescriptor.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceRegistrationResult.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiDrawIndexedDesc.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"
#include "YuEngine/Rhi/RhiSamplerHandle.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeResult.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeStatus.h"
#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldIdentityBaseline.h"
#include "YuEngine/World/WorldIdentityBaselineObjectDesc.h"
#include "YuEngine/World/WorldIdentityBaselineResult.h"
#include "YuEngine/World/WorldIdentityBaselineStatus.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

namespace asset_smoke_demo {
namespace {
constexpr std::uint32_t SCENE_MANIFEST_VERSION = 1U;
constexpr std::uint32_t FRAME_COUNT = 2U;
constexpr std::uint64_t FIXED_DELTA_TIME_NANOSECONDS = 16666666U;
constexpr std::uint32_t WORLD_OBJECT_ID = 1001U;
constexpr std::uint32_t OBJECT_TYPE_ID = 11U;
constexpr std::uint32_t COMPONENT_TYPE_ID = 21U;
constexpr std::uint32_t COMPONENT_SLOT_ID = 31U;
constexpr std::uint32_t CAMERA_ID = 41U;
constexpr std::uint32_t DRAW_ID = 51U;
constexpr std::uint32_t MATERIAL_ID = 61U;
constexpr std::uint32_t PROGRAM_ID = 71U;
constexpr std::uint32_t PASS_ID = 81U;
constexpr std::uint32_t INPUT_CONTEXT_ID = 1U;
constexpr std::uint32_t KEYBOARD_DEVICE_ID = 1U;
constexpr std::uint32_t MOVE_ACTION_ID = 2U;
constexpr std::uint32_t KEY_A_CONTROL_ID = 65U;
constexpr std::uint64_t TEXTURE_ASSET_ID = 10001U;
constexpr std::uint64_t MATERIAL_ASSET_ID = 10002U;
constexpr std::uint64_t MESH_ASSET_ID = 10003U;
constexpr std::uint64_t AUDIO_ASSET_ID = 10004U;
constexpr std::uint32_t RESOURCE_TYPE_TEXTURE = 101U;
constexpr std::uint32_t RESOURCE_TYPE_MATERIAL = 102U;
constexpr std::uint32_t RESOURCE_TYPE_MESH = 103U;
constexpr std::uint32_t RESOURCE_TYPE_AUDIO = 104U;
constexpr std::uint32_t ASSET_TYPE_TEXTURE = 201U;
constexpr std::uint32_t ASSET_TYPE_MATERIAL = 202U;
constexpr std::uint32_t ASSET_TYPE_MESH = 203U;
constexpr std::uint32_t ASSET_TYPE_AUDIO = 204U;
constexpr std::uint32_t PACKET_ID = 3001U;
constexpr std::uint32_t AUDIO_SAMPLE_RATE = 48000U;
constexpr std::uint16_t AUDIO_CHANNEL_COUNT = 2U;
constexpr std::size_t AUDIO_FRAME_COUNT = 8U;
constexpr std::size_t AUDIO_SAMPLE_COUNT = 16U;
constexpr std::size_t AUDIO_BYTE_COUNT = AUDIO_SAMPLE_COUNT * sizeof(std::int16_t);
constexpr std::size_t CONSTANT_BYTE_COUNT = 16U;
constexpr std::size_t CAPTURE_BYTE_COUNT = 16U;
constexpr std::size_t VERTEX_STRIDE_BYTES = 24U;
constexpr std::size_t VERTEX_BUFFER_BYTES = VERTEX_STRIDE_BYTES * 3U;
constexpr std::size_t INDEX_BUFFER_BYTES = sizeof(std::uint16_t) * 3U;

struct SyntheticSceneManifest final {
    std::uint32_t manifest_version = SCENE_MANIFEST_VERSION;
    yuengine::world::WorldObjectId world_object_id{WORLD_OBJECT_ID};
    std::uint32_t camera_id = CAMERA_ID;
    std::uint64_t texture_asset_id = TEXTURE_ASSET_ID;
    std::uint64_t material_asset_id = MATERIAL_ASSET_ID;
    std::uint64_t mesh_asset_id = MESH_ASSET_ID;
    std::uint64_t audio_asset_id = AUDIO_ASSET_ID;
    yuengine::input::InputContextId input_context{INPUT_CONTEXT_ID};
    yuengine::input::InputActionId move_action{MOVE_ACTION_ID};
    std::uint32_t fixed_frame_count = FRAME_COUNT;
};

struct L1AssetBindings final {
    yuengine::asset::AssetHandle texture_asset;
    yuengine::asset::AssetHandle material_asset;
    yuengine::asset::AssetHandle mesh_asset;
    yuengine::asset::AssetHandle audio_asset;
    yuengine::asset::AssetTextureReadyRecord texture_ready;
    yuengine::asset::AssetAudioReadyRecord audio_ready;
    yuengine::audio::AudioPcmSamplePacketHandle audio_packet;
};

bool FailStage(L1VerticalSamplePrepResult *result, const char *stage) {
    if (result != nullptr) {
        result->failure_stage = stage;
    }

    return false;
}

bool ValidateSyntheticSceneManifest(const SyntheticSceneManifest &manifest) {
    if (manifest.manifest_version != SCENE_MANIFEST_VERSION) {
        return false;
    }

    if (!manifest.world_object_id.IsValid()) {
        return false;
    }

    if (manifest.camera_id == 0U) {
        return false;
    }

    if (manifest.texture_asset_id == 0U) {
        return false;
    }

    if (manifest.material_asset_id == 0U) {
        return false;
    }

    if (manifest.mesh_asset_id == 0U) {
        return false;
    }

    if (manifest.audio_asset_id == 0U) {
        return false;
    }

    if (manifest.input_context.value == 0U) {
        return false;
    }

    if (manifest.move_action.value == 0U) {
        return false;
    }

    return manifest.fixed_frame_count == FRAME_COUNT;
}

bool RunRuntimeBoot(L1VerticalSamplePrepResult *result) {
    yuengine::kernel::EngineKernel kernel;
    yuengine::kernel::RuntimeApp app;
    yuengine::kernel::RuntimeAppDesc desc{};
    desc.frame_count = FRAME_COUNT;
    desc.fixed_delta_time_nanoseconds = FIXED_DELTA_TIME_NANOSECONDS;

    std::vector<std::string> lifecycle_trace;
    std::vector<yuengine::kernel::RuntimeFramePhase> phase_trace;
    if (!app.Initialize(&kernel, desc)) {
        return FailStage(result, "runtime_initialize");
    }

    const yuengine::kernel::RuntimeAppRunResult run_result =
        app.RunFixedFrames(&lifecycle_trace, &phase_trace);
    if (run_result.status != yuengine::kernel::RuntimeAppStatus::Success) {
        return FailStage(result, "runtime_run");
    }

    if (run_result.completed_frame_count != FRAME_COUNT) {
        return FailStage(result, "runtime_frame_count");
    }

    if (phase_trace.size() != FRAME_COUNT * 10U) {
        return FailStage(result, "runtime_phase_trace");
    }

    result->runtime_boot = true;
    result->completed_frame_count = run_result.completed_frame_count;
    return true;
}

yuengine::world::WorldIdentityBaselineObjectDesc MakeWorldObjectDesc(
    const SyntheticSceneManifest &manifest) {
    yuengine::world::WorldIdentityBaselineObjectDesc desc{};
    desc.world_object_id = manifest.world_object_id;
    desc.object_descriptor = yuengine::object::ObjectDescriptor{
        yuengine::object::ObjectTypeId{OBJECT_TYPE_ID},
        0U};
    desc.component_type_id = yuengine::world::WorldComponentTypeId{COMPONENT_TYPE_ID};
    desc.component_slot_id = yuengine::world::WorldComponentSlotId{COMPONENT_SLOT_ID};
    desc.transform_state.translation_z = 3.0F;
    desc.is_enabled = true;
    return desc;
}

bool CreateWorldObject(
    const SyntheticSceneManifest &manifest,
    yuengine::world::WorldIdentityBaselineRecord *output_record,
    L1VerticalSamplePrepResult *result) {
    if (output_record == nullptr) {
        return FailStage(result, "world_output");
    }

    yuengine::world::WorldIdentityBaseline baseline;
    const yuengine::world::WorldIdentityBaselineObjectDesc desc = MakeWorldObjectDesc(manifest);
    const yuengine::world::WorldIdentityBaselineResult create_result = baseline.CreateObject(desc);
    if (create_result.status != yuengine::world::WorldIdentityBaselineStatus::Success) {
        return FailStage(result, "world_object_create");
    }

    *output_record = create_result.record;
    result->world_object = true;
    result->world_object_count = baseline.Snapshot().active_record_count;
    return true;
}

yuengine::resource::ResourceRegistrationResult RegisterResource(
    yuengine::resource::ResourceRegistry &registry,
    yuengine::resource::ResourceTypeId type,
    const char *key) {
    yuengine::resource::ResourceDescriptor descriptor{};
    descriptor.type = type;
    descriptor.logical_key = yuengine::resource::ResourceLogicalKey(key);
    return registry.RegisterSyntheticDescriptor(descriptor);
}

yuengine::asset::AssetRegistrationResult RegisterAsset(
    yuengine::asset::AssetManager &manager,
    yuengine::resource::ResourceRegistry &registry,
    std::uint64_t stable_id,
    yuengine::asset::AssetTypeId asset_type,
    yuengine::resource::ResourceHandle resource,
    yuengine::resource::ResourceTypeId resource_type) {
    yuengine::asset::AssetDescriptor descriptor{};
    descriptor.stable_id = stable_id;
    descriptor.asset_type = asset_type;
    descriptor.resource = resource;
    descriptor.resource_type = resource_type;
    return manager.RegisterRuntimeAsset(&registry, descriptor);
}

yuengine::resource::ResourceDecodedPayloadRecord MakeDecodedPayloadRecord(
    yuengine::resource::ResourceHandle resource,
    yuengine::resource::ResourceTypeId resource_type) {
    yuengine::resource::ResourceDecodedPayloadRecord record{};
    record.resource = resource;
    record.expected_type = resource_type;
    record.payload_id = 401U;
    record.decode_plan_id = 501U;
    record.decode_result_id = 601U;
    record.decoded_payload_id = 701U;
    record.asset_class = yuengine::resource::ResourceDecodePlanAssetClass::Texture;
    record.result_class = yuengine::resource::ResourceDecodeResultClass::Texture;
    record.decoded_byte_count = 16U;
    record.status = yuengine::resource::ResourceDecodedPayloadStatus::Success;
    record.is_active = true;
    return record;
}

yuengine::streaming::ResourceDecodedTextureBridgeResult MakeTextureReadyResult(
    yuengine::resource::ResourceHandle resource,
    yuengine::resource::ResourceTypeId resource_type) {
    yuengine::streaming::ResourceDecodedTextureBridgeResult result{};
    result.status = yuengine::streaming::ResourceDecodedTextureBridgeStatus::Success;
    result.decoded_payload_record = MakeDecodedPayloadRecord(resource, resource_type);
    result.texture_handle = yuengine::rhi::RhiTextureHandle{1U, 1U};
    result.sampled_texture = yuengine::rhi::RhiSampledTextureBinding{result.texture_handle, 0U};
    result.decoded_byte_count = 16U;
    result.uploaded_byte_count = 16U;
    return result;
}

yuengine::audio::AudioPcmSamplePacketRequest MakePacketRequest() {
    yuengine::audio::AudioPcmSamplePacketRequest request{};
    request.packet_id = PACKET_ID;
    request.format = yuengine::audio::AudioSampleFormat::Signed16;
    request.sample_rate = AUDIO_SAMPLE_RATE;
    request.channel_count = AUDIO_CHANNEL_COUNT;
    request.frame_count = AUDIO_FRAME_COUNT;
    request.interleaved_sample_count = AUDIO_SAMPLE_COUNT;
    request.byte_count = AUDIO_BYTE_COUNT;
    return request;
}

yuengine::audioresource::AudioResourcePcmPacketImportRecord MakeAudioReadyRecord(
    yuengine::resource::ResourceHandle resource,
    yuengine::resource::ResourceTypeId resource_type) {
    yuengine::audioresource::AudioResourcePcmPacketImportRecord record{};
    record.handle = yuengine::audioresource::AudioResourcePcmPacketImportHandle{1U, 1U};
    record.import_id = 801U;
    record.resource = resource;
    record.expected_type = resource_type;
    record.payload_id = 402U;
    record.decode_plan_id = 502U;
    record.decode_result_id = 602U;
    record.asset_class = yuengine::resource::ResourceDecodePlanAssetClass::Audio;
    record.result_class = yuengine::resource::ResourceDecodeResultClass::Audio;
    record.decoded_byte_count = static_cast<std::uint32_t>(AUDIO_BYTE_COUNT);
    record.packet_request = MakePacketRequest();
    record.status = yuengine::audioresource::AudioResourcePcmPacketImportStatus::Success;
    record.is_active = true;
    return record;
}

bool QueryAssetRecord(
    yuengine::asset::AssetManager &manager,
    yuengine::asset::AssetHandle handle,
    yuengine::asset::AssetRecord *record,
    L1VerticalSamplePrepResult *result,
    const char *stage) {
    if (record == nullptr) {
        return FailStage(result, stage);
    }

    if (manager.QueryAsset(handle, record) != yuengine::asset::AssetStatus::Success) {
        return FailStage(result, stage);
    }

    return true;
}

bool BindAssets(
    const SyntheticSceneManifest &manifest,
    yuengine::resource::ResourceRegistry &resource_registry,
    yuengine::asset::AssetManager &asset_manager,
    L1AssetBindings *bindings,
    L1VerticalSamplePrepResult *result) {
    if (bindings == nullptr) {
        return FailStage(result, "asset_output");
    }

    const yuengine::resource::ResourceTypeId texture_resource_type{RESOURCE_TYPE_TEXTURE};
    const yuengine::resource::ResourceTypeId material_resource_type{RESOURCE_TYPE_MATERIAL};
    const yuengine::resource::ResourceTypeId mesh_resource_type{RESOURCE_TYPE_MESH};
    const yuengine::resource::ResourceTypeId audio_resource_type{RESOURCE_TYPE_AUDIO};
    const yuengine::resource::ResourceRegistrationResult texture_resource =
        RegisterResource(resource_registry, texture_resource_type, "sample.texture.synthetic");
    const yuengine::resource::ResourceRegistrationResult material_resource =
        RegisterResource(resource_registry, material_resource_type, "sample.material.synthetic");
    const yuengine::resource::ResourceRegistrationResult mesh_resource =
        RegisterResource(resource_registry, mesh_resource_type, "sample.mesh.synthetic");
    const yuengine::resource::ResourceRegistrationResult audio_resource =
        RegisterResource(resource_registry, audio_resource_type, "sample.audio.synthetic");
    if (!texture_resource.Succeeded() || !material_resource.Succeeded() ||
        !mesh_resource.Succeeded() || !audio_resource.Succeeded()) {
        return FailStage(result, "resource_register");
    }

    const yuengine::asset::AssetRegistrationResult texture_asset = RegisterAsset(
        asset_manager,
        resource_registry,
        manifest.texture_asset_id,
        yuengine::asset::AssetTypeId{ASSET_TYPE_TEXTURE},
        texture_resource.handle,
        texture_resource_type);
    const yuengine::asset::AssetRegistrationResult material_asset = RegisterAsset(
        asset_manager,
        resource_registry,
        manifest.material_asset_id,
        yuengine::asset::AssetTypeId{ASSET_TYPE_MATERIAL},
        material_resource.handle,
        material_resource_type);
    const yuengine::asset::AssetRegistrationResult mesh_asset = RegisterAsset(
        asset_manager,
        resource_registry,
        manifest.mesh_asset_id,
        yuengine::asset::AssetTypeId{ASSET_TYPE_MESH},
        mesh_resource.handle,
        mesh_resource_type);
    const yuengine::asset::AssetRegistrationResult audio_asset = RegisterAsset(
        asset_manager,
        resource_registry,
        manifest.audio_asset_id,
        yuengine::asset::AssetTypeId{ASSET_TYPE_AUDIO},
        audio_resource.handle,
        audio_resource_type);
    if (!texture_asset.Succeeded() || !material_asset.Succeeded() ||
        !mesh_asset.Succeeded() || !audio_asset.Succeeded()) {
        return FailStage(result, "asset_register");
    }

    const yuengine::streaming::ResourceDecodedTextureBridgeResult texture_ready =
        MakeTextureReadyResult(texture_resource.handle, texture_resource_type);
    if (asset_manager.MarkTextureReady(texture_asset.handle, texture_ready) != yuengine::asset::AssetStatus::Success) {
        return FailStage(result, "texture_ready");
    }

    const yuengine::audioresource::AudioResourcePcmPacketImportRecord audio_ready =
        MakeAudioReadyRecord(audio_resource.handle, audio_resource_type);
    if (asset_manager.MarkAudioReady(audio_asset.handle, audio_ready) != yuengine::asset::AssetStatus::Success) {
        return FailStage(result, "audio_ready");
    }

    yuengine::asset::AssetRecord texture_record{};
    yuengine::asset::AssetRecord audio_record{};
    if (!QueryAssetRecord(asset_manager, texture_asset.handle, &texture_record, result, "texture_query")) {
        return false;
    }

    if (!QueryAssetRecord(asset_manager, audio_asset.handle, &audio_record, result, "audio_query")) {
        return false;
    }

    bindings->texture_asset = texture_asset.handle;
    bindings->material_asset = material_asset.handle;
    bindings->mesh_asset = mesh_asset.handle;
    bindings->audio_asset = audio_asset.handle;
    bindings->texture_ready = texture_record.texture_ready;
    bindings->audio_ready = audio_record.audio_ready;
    bindings->audio_packet = yuengine::audio::AudioPcmSamplePacketHandle{1U, 1U};

    result->asset_bindings = true;
    result->asset_count = asset_manager.Snapshot().active_asset_count;
    return true;
}

yuengine::input::InputCommandBinding MakeMoveBinding(const SyntheticSceneManifest &manifest) {
    yuengine::input::InputCommandBinding binding{};
    binding.context = manifest.input_context;
    binding.device = yuengine::input::InputDeviceId{KEYBOARD_DEVICE_ID};
    binding.control = yuengine::input::InputControlId{KEY_A_CONTROL_ID};
    binding.action = manifest.move_action;
    binding.value_kind = yuengine::input::InputCommandValueKind::Button;
    return binding;
}

bool BuildInputCommand(
    const SyntheticSceneManifest &manifest,
    yuengine::input::InputCommandSnapshot *snapshot,
    L1VerticalSamplePrepResult *result) {
    if (snapshot == nullptr) {
        return FailStage(result, "input_output");
    }

    yuengine::input::InputCommandMapper mapper;
    if (mapper.RegisterContext(manifest.input_context) != yuengine::input::InputStatus::Success) {
        return FailStage(result, "input_context");
    }

    if (mapper.SetActiveContext(
        manifest.input_context,
        yuengine::input::InputContextFocusMode::AcceptInput) != yuengine::input::InputStatus::Success) {
        return FailStage(result, "input_active_context");
    }

    const yuengine::input::InputCommandBinding binding = MakeMoveBinding(manifest);
    if (mapper.RegisterBinding(binding) != yuengine::input::InputStatus::Success) {
        return FailStage(result, "input_binding");
    }

    const std::array<yuengine::input::InputEvent, 1U> events{
        yuengine::input::InputEvent{
            yuengine::input::InputDeviceId{KEYBOARD_DEVICE_ID},
            yuengine::input::InputControlId{KEY_A_CONTROL_ID},
            yuengine::input::InputEventType::ButtonPressed,
            0}};
    if (mapper.BuildSnapshot(1U, events, snapshot) != yuengine::input::InputStatus::Success) {
        return FailStage(result, "input_snapshot");
    }

    if (snapshot->command_count != 1U) {
        return FailStage(result, "input_command_count");
    }

    if (!snapshot->commands[0U].pressed_this_frame) {
        return FailStage(result, "input_command_state");
    }

    result->input_command = true;
    result->input_command_count = static_cast<std::uint32_t>(snapshot->command_count);
    return true;
}

yuengine::rhi::RhiVertexBufferView MakeVertexBufferView() {
    yuengine::rhi::RhiVertexBufferView view{};
    view.buffer = yuengine::rhi::RhiBufferHandle{1U, 1U};
    view.stride_bytes = VERTEX_STRIDE_BYTES;
    view.size_bytes = VERTEX_BUFFER_BYTES;
    return view;
}

yuengine::rhi::RhiIndexBufferView MakeIndexBufferView() {
    yuengine::rhi::RhiIndexBufferView view{};
    view.buffer = yuengine::rhi::RhiBufferHandle{2U, 1U};
    view.size_bytes = INDEX_BUFFER_BYTES;
    view.format = yuengine::rhi::RhiIndexFormat::Uint16;
    return view;
}

yuengine::renderscene::RenderSceneCameraRecord MakeCameraRecord(
    const SyntheticSceneManifest &manifest) {
    yuengine::renderscene::RenderSceneCameraRecord camera{};
    camera.camera_id = manifest.camera_id;
    camera.target = yuengine::rhi::RhiTextureHandle{3U, 1U};
    camera.clear_color = yuengine::rhi::RhiColor{8U, 16U, 24U, 255U};
    camera.is_active = true;
    return camera;
}

yuengine::renderscene::RenderSceneEntityRecord MakeRenderEntityRecord(
    const SyntheticSceneManifest &manifest,
    const yuengine::world::WorldIdentityBaselineRecord &world_record,
    const L1AssetBindings &bindings,
    std::span<const std::uint8_t> constants) {
    yuengine::renderscene::RenderSceneEntityRecord entity{};
    entity.world_object_id = manifest.world_object_id;
    entity.transform = world_record.transform_state;
    entity.mesh_asset = bindings.mesh_asset;
    entity.material_asset = bindings.material_asset;
    entity.texture_ready = bindings.texture_ready;
    entity.material.material_id = MATERIAL_ID;
    entity.material.program_id = PROGRAM_ID;
    entity.material.pipeline = yuengine::rhi::RhiPipelineHandle{4U, 1U};
    entity.material.sampler = yuengine::rhi::RhiSamplerBinding{yuengine::rhi::RhiSamplerHandle{5U, 1U}, 0U};
    entity.material.constant_bytes = constants;
    entity.material.pass_id = PASS_ID;
    entity.draw.draw_id = DRAW_ID;
    entity.draw.pass_id = PASS_ID;
    entity.draw.material_id = MATERIAL_ID;
    entity.draw.vertex_buffer = MakeVertexBufferView();
    entity.draw.index_buffer = MakeIndexBufferView();
    entity.draw.draw = yuengine::rhi::RhiDrawIndexedDesc{
        yuengine::rhi::RhiPrimitiveTopology::TriangleList,
        3U,
        0U,
        0};
    entity.is_visible = true;
    entity.is_active = true;
    return entity;
}

bool SubmitRenderScene(
    const SyntheticSceneManifest &manifest,
    const yuengine::world::WorldIdentityBaselineRecord &world_record,
    const L1AssetBindings &bindings,
    L1VerticalSamplePrepResult *result) {
    std::array<std::uint8_t, CONSTANT_BYTE_COUNT> constants{};
    std::array<std::uint8_t, CAPTURE_BYTE_COUNT> capture{};
    const std::array<yuengine::renderscene::RenderSceneCameraRecord, 1U> cameras{
        MakeCameraRecord(manifest)};
    const std::array<yuengine::renderscene::RenderSceneEntityRecord, 1U> entities{
        MakeRenderEntityRecord(manifest, world_record, bindings, constants)};
    std::array<yuengine::rendercore::RenderViewPacketRequest, 1U> packets{};
    yuengine::renderscene::RenderSceneSubmitResult submit_result{};
    yuengine::renderscene::RenderSceneSubmitRequest request{};
    request.frame_id = 1U;
    request.active_camera_id = manifest.camera_id;
    request.cameras = cameras;
    request.entities = entities;
    request.capture_output = capture;
    request.capture_byte_budget = capture.size();

    yuengine::renderscene::RenderSceneContractQueue queue;
    const yuengine::renderscene::RenderSceneStatus status =
        queue.BuildRenderCorePackets(request, packets, &submit_result);
    if (status != yuengine::renderscene::RenderSceneStatus::Success) {
        return FailStage(result, "render_submit");
    }

    if (submit_result.output_packet_count != 1U) {
        return FailStage(result, "render_packet_count");
    }

    result->render_scene_submit = true;
    result->render_packet_count = static_cast<std::uint32_t>(submit_result.output_packet_count);
    return true;
}

bool SubmitAudioScene(
    const L1AssetBindings &bindings,
    L1VerticalSamplePrepResult *result) {
    yuengine::audioscene::AudioSceneSourceRecord source{};
    source.source_id = yuengine::audio::AudioSourceId{1U, 1U};
    source.sound_asset = bindings.audio_asset;
    source.audio_ready = bindings.audio_ready;
    source.packet = bindings.audio_packet;
    source.state = yuengine::audioscene::AudioSceneSourceState::Playing;
    source.is_active = true;

    const std::array<yuengine::audioscene::AudioSceneSourceRecord, 1U> sources{source};
    std::array<yuengine::audio::AudioPcmStreamQueueRequest, 1U> requests{};
    yuengine::audioscene::AudioSceneSubmitRequest submit_request{};
    submit_request.frame_id = 1U;
    submit_request.sources = sources;
    yuengine::audioscene::AudioSceneSubmitResult submit_result{};

    yuengine::audioscene::AudioSceneContractQueue queue;
    const yuengine::audioscene::AudioSceneStatus status =
        queue.SubmitSourceUpdates(submit_request, requests, &submit_result);
    if (status != yuengine::audioscene::AudioSceneStatus::Success) {
        return FailStage(result, "audio_submit");
    }

    if (submit_result.queue_request_count != 1U) {
        return FailStage(result, "audio_queue_request_count");
    }

    if (requests[0U].expected_packet_id != PACKET_ID) {
        return FailStage(result, "audio_packet_id");
    }

    result->audio_scene_submit = true;
    result->audio_queue_request_count = static_cast<std::uint32_t>(submit_result.queue_request_count);
    return true;
}
}

bool RunL1VerticalSamplePrep(L1VerticalSamplePrepResult *result) {
    if (result == nullptr) {
        return false;
    }

    *result = L1VerticalSamplePrepResult{};
    if (!RunRuntimeBoot(result)) {
        return false;
    }

    const SyntheticSceneManifest manifest{};
    if (!ValidateSyntheticSceneManifest(manifest)) {
        return FailStage(result, "synthetic_manifest");
    }

    result->synthetic_manifest = true;

    yuengine::world::WorldIdentityBaselineRecord world_record{};
    if (!CreateWorldObject(manifest, &world_record, result)) {
        return false;
    }

    yuengine::resource::ResourceRegistry resource_registry;
    yuengine::asset::AssetManager asset_manager;
    L1AssetBindings bindings{};
    if (!BindAssets(manifest, resource_registry, asset_manager, &bindings, result)) {
        return false;
    }

    yuengine::input::InputCommandSnapshot input_snapshot{};
    if (!BuildInputCommand(manifest, &input_snapshot, result)) {
        return false;
    }

    if (!SubmitRenderScene(manifest, world_record, bindings, result)) {
        return false;
    }

    if (!SubmitAudioScene(bindings, result)) {
        return false;
    }

    result->failure_stage = "ok";
    return true;
}
}
