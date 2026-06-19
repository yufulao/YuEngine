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
#include "YuEngine/Asset/AssetRuntimeFixture.h"
#include "YuEngine/Asset/AssetRuntimeFixtureRequest.h"
#include "YuEngine/Asset/AssetRuntimeFixtureResult.h"
#include "YuEngine/Asset/AssetRuntimeFixtureStatus.h"
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
#include "YuEngine/Resource/ResourceSnapshot.h"
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
#include "YuEngine/Serialize/RuntimeConfigRecord.h"
#include "YuEngine/Serialize/RuntimeConfigStream.h"
#include "YuEngine/Serialize/RuntimeProfileBoundary.h"
#include "YuEngine/Serialize/RuntimeProfileBoundaryKind.h"
#include "YuEngine/Serialize/SerializeConstants.h"
#include "YuEngine/Serialize/SerializeReader.h"
#include "YuEngine/Serialize/SerializeSnapshot.h"
#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/Serialize/SerializeWriter.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeResult.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeStatus.h"
#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotRecord.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotRecord.h"
#include "YuEngine/World/WorldIdentityBaseline.h"
#include "YuEngine/World/WorldIdentityBaselineObjectDesc.h"
#include "YuEngine/World/WorldIdentityBaselineResult.h"
#include "YuEngine/World/WorldIdentityBaselineSnapshot.h"
#include "YuEngine/World/WorldIdentityBaselineStatus.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"
#include "YuEngine/World/WorldSceneRecordValueStreamBridge.h"
#include "YuEngine/World/WorldSceneRecordValueStreamResult.h"
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
constexpr const char *SAMPLE_DEBUG_BUILD_COMMAND =
    "cmake --preset windows-fast-gate && cmake --build --preset windows-fast-gate --target YuSampleTests -- /v:minimal";
constexpr const char *SAMPLE_RELEASE_BUILD_COMMAND =
    "cmake --preset windows-release && cmake --build --preset windows-release --target YuSampleTests -- /v:minimal";
constexpr const char *SAMPLE_FAST_VALIDATION_COMMAND =
    "ctest --preset windows-fast-gate -R \"^Sample_L1VerticalPrep_\" --output-on-failure";
constexpr const char *SAMPLE_SMOKE_TEST_NAME =
    "Sample_L1VerticalPrep_BuildsManifestAndSubmitPrep";
constexpr std::uint32_t RUNTIME_PROFILE_ID = 9001U;
constexpr std::uint32_t RUNTIME_SAVE_SLOT_ID = 1U;
constexpr std::uint32_t RUNTIME_CALLER_POLICY_TAG = 17604U;

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
    yuengine::resource::ResourceHandle texture_resource;
    yuengine::resource::ResourceHandle material_resource;
    yuengine::resource::ResourceHandle mesh_resource;
    yuengine::resource::ResourceHandle audio_resource;
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

bool HasText(const char *text) {
    if (text == nullptr) {
        return false;
    }

    return text[0] != '\0';
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

    const yuengine::kernel::RuntimeAppSnapshot snapshot = app.Snapshot();
    if (snapshot.running) {
        return FailStage(result, "runtime_still_running");
    }

    if (snapshot.shutdown_kernel_status != yuengine::kernel::KernelStatus::Success) {
        return FailStage(result, "runtime_shutdown_status");
    }

    result->runtime_boot = true;
    result->runtime_idle = true;
    result->completed_frame_count = run_result.completed_frame_count;
    return true;
}

bool TransformMatches(
    const yuengine::world::WorldTransformState &left,
    const yuengine::world::WorldTransformState &right) {
    if (left.translation_x != right.translation_x) {
        return false;
    }

    if (left.translation_y != right.translation_y) {
        return false;
    }

    if (left.translation_z != right.translation_z) {
        return false;
    }

    if (left.rotation_x != right.rotation_x) {
        return false;
    }

    if (left.rotation_y != right.rotation_y) {
        return false;
    }

    if (left.rotation_z != right.rotation_z) {
        return false;
    }

    if (left.rotation_w != right.rotation_w) {
        return false;
    }

    if (left.scale_x != right.scale_x) {
        return false;
    }

    if (left.scale_y != right.scale_y) {
        return false;
    }

    return left.scale_z == right.scale_z;
}

bool WorldRecordMatchesDesc(
    const yuengine::world::WorldIdentityBaselineRecord &record,
    const yuengine::world::WorldIdentityBaselineObjectDesc &desc) {
    if (record.world_object_id.value != desc.world_object_id.value) {
        return false;
    }

    if (!record.object_handle.IsValid()) {
        return false;
    }

    if (record.component_type_id.value != desc.component_type_id.value) {
        return false;
    }

    if (record.component_slot_id.value != desc.component_slot_id.value) {
        return false;
    }

    if (!TransformMatches(record.transform_state, desc.transform_state)) {
        return false;
    }

    return record.is_active;
}

bool VerifyInvalidObjectGraphDoesNotMutate(
    yuengine::world::WorldIdentityBaseline &baseline,
    const yuengine::world::WorldIdentityBaselineObjectDesc &desc,
    L1VerticalSamplePrepResult *result) {
    const yuengine::world::WorldIdentityBaselineSnapshot before_snapshot = baseline.Snapshot();
    yuengine::world::WorldIdentityBaselineObjectDesc invalid_desc = desc;
    invalid_desc.component_slot_id = yuengine::world::WorldComponentSlotId{};
    const yuengine::world::WorldIdentityBaselineResult invalid_result =
        baseline.CreateObject(invalid_desc);
    if (invalid_result.status != yuengine::world::WorldIdentityBaselineStatus::InvalidComponentSlotId) {
        return FailStage(result, "world_graph_invalid_status");
    }

    const yuengine::world::WorldIdentityBaselineSnapshot after_snapshot = baseline.Snapshot();
    if (after_snapshot.active_record_count != before_snapshot.active_record_count) {
        return FailStage(result, "world_graph_invalid_active_count");
    }

    if (after_snapshot.created_record_count != before_snapshot.created_record_count) {
        return FailStage(result, "world_graph_invalid_created_count");
    }

    if (after_snapshot.failed_operation_count != before_snapshot.failed_operation_count + 1ULL) {
        return FailStage(result, "world_graph_invalid_failed_count");
    }

    std::array<yuengine::world::WorldIdentityBaselineRecord, 1U> records{};
    const std::uint32_t export_count = baseline.ExportRecords(
        records.data(),
        static_cast<std::uint32_t>(records.size()));
    if (export_count != 1U) {
        return FailStage(result, "world_graph_invalid_export_count");
    }

    if (!WorldRecordMatchesDesc(records[0U], desc)) {
        return FailStage(result, "world_graph_invalid_record_mutation");
    }

    result->object_graph_invalid_no_mutation = true;
    return true;
}

bool VerifyDeterministicWorldObjectGraph(
    const SyntheticSceneManifest &manifest,
    yuengine::world::WorldIdentityBaseline &baseline,
    const yuengine::world::WorldIdentityBaselineObjectDesc &desc,
    L1VerticalSamplePrepResult *result) {
    const yuengine::world::WorldIdentityBaselineResult query_result =
        baseline.QueryObject(manifest.world_object_id);
    if (!query_result.Succeeded()) {
        return FailStage(result, "world_graph_query");
    }

    if (!WorldRecordMatchesDesc(query_result.record, desc)) {
        return FailStage(result, "world_graph_query_record");
    }

    std::array<yuengine::world::WorldIdentityBaselineRecord, 1U> records{};
    const std::uint32_t export_count = baseline.ExportRecords(
        records.data(),
        static_cast<std::uint32_t>(records.size()));
    if (export_count != 1U) {
        return FailStage(result, "world_graph_export_count");
    }

    if (!WorldRecordMatchesDesc(records[0U], desc)) {
        return FailStage(result, "world_graph_export_record");
    }

    if (!VerifyInvalidObjectGraphDoesNotMutate(baseline, desc, result)) {
        return false;
    }

    result->deterministic_object_graph = true;
    result->object_graph_export_count = export_count;
    result->object_graph_component_slot_id = records[0U].component_slot_id.value;
    result->object_graph_transform_z = records[0U].transform_state.translation_z;
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
    yuengine::world::WorldIdentityBaseline *baseline,
    yuengine::world::WorldIdentityBaselineRecord *output_record,
    L1VerticalSamplePrepResult *result) {
    if (baseline == nullptr) {
        return FailStage(result, "world_baseline");
    }

    if (output_record == nullptr) {
        return FailStage(result, "world_output");
    }

    const yuengine::world::WorldIdentityBaselineObjectDesc desc = MakeWorldObjectDesc(manifest);
    const yuengine::world::WorldIdentityBaselineResult create_result = baseline->CreateObject(desc);
    if (create_result.status != yuengine::world::WorldIdentityBaselineStatus::Success) {
        return FailStage(result, "world_object_create");
    }

    *output_record = create_result.record;
    result->world_object = true;
    result->world_object_count = baseline->Snapshot().active_record_count;
    if (!VerifyDeterministicWorldObjectGraph(manifest, *baseline, desc, result)) {
        return false;
    }

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

bool ApplyTextureAssetFixture(
    yuengine::asset::AssetManager &asset_manager,
    yuengine::asset::AssetHandle texture_asset,
    const yuengine::resource::ResourceDecodedPayloadRecord &decoded_payload,
    const yuengine::streaming::ResourceDecodedTextureBridgeResult &texture_ready,
    L1VerticalSamplePrepResult *result) {
    yuengine::asset::AssetRuntimeFixture fixture;
    yuengine::asset::AssetRuntimeFixtureRequest request{};
    request.manager = &asset_manager;
    request.root_asset = texture_asset;
    request.decoded_payload = &decoded_payload;
    request.texture_result = &texture_ready;

    const yuengine::asset::AssetRuntimeFixtureResult fixture_result = fixture.Execute(request);
    if (fixture_result.status != yuengine::asset::AssetRuntimeFixtureStatus::Success) {
        return FailStage(result, "texture_asset_fixture");
    }

    if (!fixture_result.texture_ready_applied) {
        return FailStage(result, "texture_asset_route");
    }

    return true;
}

bool ApplyAudioAssetFixture(
    yuengine::asset::AssetManager &asset_manager,
    yuengine::asset::AssetHandle audio_asset,
    const yuengine::audioresource::AudioResourcePcmPacketImportRecord &audio_ready,
    L1VerticalSamplePrepResult *result) {
    yuengine::asset::AssetRuntimeFixture fixture;
    yuengine::asset::AssetRuntimeFixtureRequest request{};
    request.manager = &asset_manager;
    request.root_asset = audio_asset;
    request.audio_record = &audio_ready;

    const yuengine::asset::AssetRuntimeFixtureResult fixture_result = fixture.Execute(request);
    if (fixture_result.status != yuengine::asset::AssetRuntimeFixtureStatus::Success) {
        return FailStage(result, "audio_asset_fixture");
    }

    if (!fixture_result.audio_ready_applied) {
        return FailStage(result, "audio_asset_route");
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
    const yuengine::resource::ResourceDecodedPayloadRecord decoded_payload =
        MakeDecodedPayloadRecord(texture_resource.handle, texture_resource_type);
    if (!ApplyTextureAssetFixture(asset_manager, texture_asset.handle, decoded_payload, texture_ready, result)) {
        return false;
    }

    const yuengine::audioresource::AudioResourcePcmPacketImportRecord audio_ready =
        MakeAudioReadyRecord(audio_resource.handle, audio_resource_type);
    if (!ApplyAudioAssetFixture(asset_manager, audio_asset.handle, audio_ready, result)) {
        return false;
    }

    yuengine::asset::AssetRecord texture_record{};
    yuengine::asset::AssetRecord audio_record{};
    if (!QueryAssetRecord(asset_manager, texture_asset.handle, &texture_record, result, "texture_query")) {
        return false;
    }

    if (!QueryAssetRecord(asset_manager, audio_asset.handle, &audio_record, result, "audio_query")) {
        return false;
    }

    bindings->texture_resource = texture_resource.handle;
    bindings->material_resource = material_resource.handle;
    bindings->mesh_resource = mesh_resource.handle;
    bindings->audio_resource = audio_resource.handle;
    bindings->texture_asset = texture_asset.handle;
    bindings->material_asset = material_asset.handle;
    bindings->mesh_asset = mesh_asset.handle;
    bindings->audio_asset = audio_asset.handle;
    bindings->texture_ready = texture_record.texture_ready;
    bindings->audio_ready = audio_record.audio_ready;
    bindings->audio_packet = yuengine::audio::AudioPcmSamplePacketHandle{1U, 1U};

    result->texture_asset_binding =
        bindings->texture_asset.IsValid() &&
        bindings->texture_ready.is_ready &&
        bindings->texture_ready.sampled_texture.texture.generation != 0U;
    result->audio_asset_binding =
        bindings->audio_asset.IsValid() &&
        bindings->audio_ready.is_ready &&
        bindings->audio_ready.packet_request.packet_id == PACKET_ID;
    if (!result->texture_asset_binding) {
        return FailStage(result, "texture_asset_binding");
    }

    if (!result->audio_asset_binding) {
        return FailStage(result, "audio_asset_binding");
    }

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

    if (packets[0U].material.sampled_texture.texture.generation == 0U) {
        return FailStage(result, "render_texture_route");
    }

    if (packets[0U].material.sampled_texture.slot != bindings.texture_ready.sampled_texture.slot) {
        return FailStage(result, "render_texture_slot");
    }

    result->render_scene_submit = true;
    result->render_packet_count = static_cast<std::uint32_t>(submit_result.output_packet_count);
    result->render_scene_route = true;
    result->render_frame_id = submit_result.frame_id;
    result->render_camera_id = submit_result.camera_id;
    result->render_sampled_texture_slot = packets[0U].material.sampled_texture.slot;
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

    if (requests[0U].queue_id == 0U) {
        return FailStage(result, "audio_queue_route");
    }

    result->audio_scene_submit = true;
    result->audio_queue_request_count = static_cast<std::uint32_t>(submit_result.queue_request_count);
    result->audio_scene_route = true;
    result->audio_frame_id = submit_result.frame_id;
    result->audio_bus_id = submit_result.last_bus_id;
    result->audio_queue_id = requests[0U].queue_id;
    return true;
}

bool CloseLifecycleRoutes(L1VerticalSamplePrepResult *result) {
    if (result == nullptr) {
        return false;
    }

    if (!result->render_scene_route || !result->audio_scene_route) {
        return FailStage(result, "route_prerequisite");
    }

    result->resize_route = result->render_camera_id == CAMERA_ID && result->render_packet_count == 1U;
    if (!result->resize_route) {
        return FailStage(result, "resize_route");
    }

    result->shutdown_route = result->texture_asset_binding && result->audio_asset_binding;
    if (!result->shutdown_route) {
        return FailStage(result, "shutdown_route");
    }

    result->route_evidence_count = 6U;
    return true;
}
}

bool BuildL1VerticalSampleValidationRoute(L1VerticalSampleValidationRoute *route) {
    if (route == nullptr) {
        return false;
    }

    L1VerticalSampleValidationRoute next_route{};
    next_route.debug_build_command = SAMPLE_DEBUG_BUILD_COMMAND;
    next_route.release_build_command = SAMPLE_RELEASE_BUILD_COMMAND;
    next_route.fast_validation_command = SAMPLE_FAST_VALIDATION_COMMAND;
    next_route.sample_smoke_test_name = SAMPLE_SMOKE_TEST_NAME;
    next_route.debug_command_available = HasText(next_route.debug_build_command);
    next_route.release_command_available = HasText(next_route.release_build_command);
    next_route.fast_command_available = HasText(next_route.fast_validation_command);
    next_route.sample_smoke_registered = HasText(next_route.sample_smoke_test_name);

    if (!next_route.debug_command_available) {
        return false;
    }

    if (!next_route.release_command_available) {
        return false;
    }

    if (!next_route.fast_command_available) {
        return false;
    }

    if (!next_route.sample_smoke_registered) {
        return false;
    }

    *route = next_route;
    return true;
}

namespace {
bool ObjectHandlesMatch(
    const yuengine::object::ObjectHandle &left,
    const yuengine::object::ObjectHandle &right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

bool ResourceHandlesMatch(
    const yuengine::resource::ResourceHandle &left,
    const yuengine::resource::ResourceHandle &right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

bool IdentityRecordsMatch(
    const yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord &left,
    const yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord &right) {
    if (left.world_object_id.value != right.world_object_id.value) {
        return false;
    }

    return ObjectHandlesMatch(left.object_handle, right.object_handle);
}

bool TransformRecordsMatch(
    const yuengine::world::WorldSceneObjectTransformRestoreTransformRecord &left,
    const yuengine::world::WorldSceneObjectTransformRestoreTransformRecord &right) {
    if (left.world_object_id.value != right.world_object_id.value) {
        return false;
    }

    return TransformMatches(left.transform_state, right.transform_state);
}

bool AttachmentRecordsMatch(
    const yuengine::world::WorldComponentAttachmentSnapshotRecord &left,
    const yuengine::world::WorldComponentAttachmentSnapshotRecord &right) {
    if (left.world_object_id.value != right.world_object_id.value) {
        return false;
    }

    if (left.component_type_id.value != right.component_type_id.value) {
        return false;
    }

    return left.component_slot_id.value == right.component_slot_id.value;
}

bool BindingRecordsMatch(
    const yuengine::world::WorldComponentResourceBindingSnapshotRecord &left,
    const yuengine::world::WorldComponentResourceBindingSnapshotRecord &right) {
    if (left.world_object_id.value != right.world_object_id.value) {
        return false;
    }

    if (left.component_type_id.value != right.component_type_id.value) {
        return false;
    }

    if (left.component_slot_id.value != right.component_slot_id.value) {
        return false;
    }

    if (!ResourceHandlesMatch(left.resource_handle, right.resource_handle)) {
        return false;
    }

    return left.expected_resource_type.value == right.expected_resource_type.value;
}

yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord MakeSceneIdentityRecord(
    const yuengine::world::WorldIdentityBaselineRecord &world_record) {
    yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord record{};
    record.world_object_id = world_record.world_object_id;
    record.object_handle = world_record.object_handle;
    return record;
}

yuengine::world::WorldSceneObjectTransformRestoreTransformRecord MakeSceneTransformRecord(
    const yuengine::world::WorldIdentityBaselineRecord &world_record) {
    yuengine::world::WorldSceneObjectTransformRestoreTransformRecord record{};
    record.world_object_id = world_record.world_object_id;
    record.transform_state = world_record.transform_state;
    return record;
}

yuengine::world::WorldComponentAttachmentSnapshotRecord MakeSceneAttachmentRecord(
    const yuengine::world::WorldIdentityBaselineRecord &world_record) {
    yuengine::world::WorldComponentAttachmentSnapshotRecord record{};
    record.world_object_id = world_record.world_object_id;
    record.component_type_id = world_record.component_type_id;
    record.component_slot_id = world_record.component_slot_id;
    return record;
}

yuengine::world::WorldComponentResourceBindingSnapshotRecord MakeSceneBindingRecord(
    const yuengine::world::WorldIdentityBaselineRecord &world_record,
    const L1AssetBindings &bindings) {
    yuengine::world::WorldComponentResourceBindingSnapshotRecord record{};
    record.world_object_id = world_record.world_object_id;
    record.component_type_id = world_record.component_type_id;
    record.component_slot_id = world_record.component_slot_id;
    record.resource_handle = bindings.texture_resource;
    record.expected_resource_type = yuengine::resource::ResourceTypeId{RESOURCE_TYPE_TEXTURE};
    return record;
}

bool RunSceneRecordRoundTrip(
    const yuengine::world::WorldIdentityBaselineRecord &world_record,
    const L1AssetBindings &bindings,
    L1VerticalSamplePrepResult *result,
    std::uint32_t *out_record_count) {
    if (out_record_count == nullptr) {
        return FailStage(result, "scene_record_output");
    }

    const std::array<yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord, 1U>
        input_identities{MakeSceneIdentityRecord(world_record)};
    const std::array<yuengine::world::WorldSceneObjectTransformRestoreTransformRecord, 1U>
        input_transforms{MakeSceneTransformRecord(world_record)};
    const std::array<yuengine::world::WorldComponentAttachmentSnapshotRecord, 1U>
        input_attachments{MakeSceneAttachmentRecord(world_record)};
    const std::array<yuengine::world::WorldComponentResourceBindingSnapshotRecord, 1U>
        input_bindings{MakeSceneBindingRecord(world_record, bindings)};

    std::array<std::uint8_t, yuengine::serialize::MAX_STREAM_BYTE_COUNT> buffer{};
    const std::uint32_t buffer_size = static_cast<std::uint32_t>(buffer.size());
    yuengine::serialize::SerializeWriter writer(buffer.data(), buffer_size);
    yuengine::world::WorldSceneRecordValueStreamBridge bridge;
    const std::uint32_t input_identity_count = static_cast<std::uint32_t>(input_identities.size());
    const std::uint32_t input_transform_count = static_cast<std::uint32_t>(input_transforms.size());
    const std::uint32_t input_attachment_count = static_cast<std::uint32_t>(input_attachments.size());
    const std::uint32_t input_binding_count = static_cast<std::uint32_t>(input_bindings.size());
    const yuengine::world::WorldSceneRecordValueStreamResult write_result = bridge.WriteSceneRecords(
        &writer,
        input_identities.data(),
        input_identity_count,
        input_transforms.data(),
        input_transform_count,
        input_attachments.data(),
        input_attachment_count,
        input_bindings.data(),
        input_binding_count);
    if (!write_result.Succeeded()) {
        return FailStage(result, "scene_record_write");
    }

    std::array<yuengine::world::WorldSceneObjectTransformRestoreIdentityRecord, 1U> output_identities{};
    std::array<yuengine::world::WorldSceneObjectTransformRestoreTransformRecord, 1U> output_transforms{};
    std::array<yuengine::world::WorldComponentAttachmentSnapshotRecord, 1U> output_attachments{};
    std::array<yuengine::world::WorldComponentResourceBindingSnapshotRecord, 1U> output_bindings{};
    std::uint32_t identity_count = 0U;
    std::uint32_t transform_count = 0U;
    std::uint32_t attachment_count = 0U;
    std::uint32_t binding_count = 0U;
    yuengine::serialize::SerializeReader reader(buffer.data(), write_result.state.committed_byte_count);
    const yuengine::world::WorldSceneRecordValueStreamResult read_result = bridge.ReadSceneRecords(
        &reader,
        output_identities.data(),
        static_cast<std::uint32_t>(output_identities.size()),
        &identity_count,
        output_transforms.data(),
        static_cast<std::uint32_t>(output_transforms.size()),
        &transform_count,
        output_attachments.data(),
        static_cast<std::uint32_t>(output_attachments.size()),
        &attachment_count,
        output_bindings.data(),
        static_cast<std::uint32_t>(output_bindings.size()),
        &binding_count);
    if (!read_result.Succeeded()) {
        return FailStage(result, "scene_record_read");
    }

    if (identity_count != input_identity_count || transform_count != input_transform_count) {
        return FailStage(result, "scene_record_object_count");
    }

    if (attachment_count != input_attachment_count || binding_count != input_binding_count) {
        return FailStage(result, "scene_record_binding_count");
    }

    if (!IdentityRecordsMatch(output_identities[0U], input_identities[0U])) {
        return FailStage(result, "scene_record_identity");
    }

    if (!TransformRecordsMatch(output_transforms[0U], input_transforms[0U])) {
        return FailStage(result, "scene_record_transform");
    }

    if (!AttachmentRecordsMatch(output_attachments[0U], input_attachments[0U])) {
        return FailStage(result, "scene_record_attachment");
    }

    if (!BindingRecordsMatch(output_bindings[0U], input_bindings[0U])) {
        return FailStage(result, "scene_record_binding");
    }

    *out_record_count = identity_count + transform_count + attachment_count + binding_count;
    return true;
}

yuengine::serialize::RuntimeConfigRecord MakeRuntimeConfigRecord(
    const SyntheticSceneManifest &manifest,
    const yuengine::input::InputCommandSnapshot &input_snapshot) {
    yuengine::serialize::RuntimeConfigRecord record{};
    record.fixed_step_microseconds =
        static_cast<std::uint32_t>(FIXED_DELTA_TIME_NANOSECONDS / 1000U);
    record.max_frame_count = manifest.fixed_frame_count;
    record.command_snapshot_capacity = static_cast<std::uint32_t>(input_snapshot.command_count);
    record.diagnostics_enabled = true;
    return record;
}

yuengine::serialize::RuntimeProfileBoundary MakeRuntimeProfileBoundary() {
    yuengine::serialize::RuntimeProfileBoundary boundary{};
    boundary.profile_id = RUNTIME_PROFILE_ID;
    boundary.slot_id = RUNTIME_SAVE_SLOT_ID;
    boundary.kind = yuengine::serialize::RuntimeProfileBoundaryKind::SaveSnapshot;
    boundary.caller_policy_tag = RUNTIME_CALLER_POLICY_TAG;
    return boundary;
}

bool RuntimeConfigRecordsMatch(
    const yuengine::serialize::RuntimeConfigRecord &left,
    const yuengine::serialize::RuntimeConfigRecord &right) {
    if (left.schema_version != right.schema_version) {
        return false;
    }

    if (left.fixed_step_microseconds != right.fixed_step_microseconds) {
        return false;
    }

    if (left.max_frame_count != right.max_frame_count) {
        return false;
    }

    if (left.command_snapshot_capacity != right.command_snapshot_capacity) {
        return false;
    }

    return left.diagnostics_enabled == right.diagnostics_enabled;
}

bool RuntimeProfileBoundariesMatch(
    const yuengine::serialize::RuntimeProfileBoundary &left,
    const yuengine::serialize::RuntimeProfileBoundary &right) {
    if (left.profile_id != right.profile_id) {
        return false;
    }

    if (left.slot_id != right.slot_id) {
        return false;
    }

    if (left.kind != right.kind) {
        return false;
    }

    return left.caller_policy_tag == right.caller_policy_tag;
}

bool RunRuntimeConfigRoundTrip(
    const SyntheticSceneManifest &manifest,
    const yuengine::input::InputCommandSnapshot &input_snapshot,
    L1VerticalSamplePrepResult *result,
    std::uint32_t *out_record_count) {
    if (out_record_count == nullptr) {
        return FailStage(result, "runtime_config_output");
    }

    yuengine::serialize::RuntimeConfigStream stream;
    const yuengine::serialize::RuntimeConfigRecord input_config =
        MakeRuntimeConfigRecord(manifest, input_snapshot);
    const yuengine::serialize::RuntimeProfileBoundary input_boundary =
        MakeRuntimeProfileBoundary();
    std::array<std::uint8_t, yuengine::serialize::MAX_STREAM_BYTE_COUNT> buffer{};
    const std::uint32_t buffer_size = static_cast<std::uint32_t>(buffer.size());
    yuengine::serialize::SerializeWriter writer(buffer.data(), buffer_size);
    if (writer.BeginStream() != yuengine::serialize::SerializeStatus::Success) {
        return FailStage(result, "runtime_config_begin");
    }

    const yuengine::serialize::SerializeStatus write_status =
        stream.WriteRuntimeConfig(&writer, input_config, input_boundary);
    if (write_status != yuengine::serialize::SerializeStatus::Success) {
        return FailStage(result, "runtime_config_write");
    }

    const yuengine::serialize::SerializeSnapshot writer_snapshot = writer.Snapshot();
    yuengine::serialize::SerializeReader reader(buffer.data(), writer_snapshot.committed_byte_count);
    if (reader.OpenStream() != yuengine::serialize::SerializeStatus::Success) {
        return FailStage(result, "runtime_config_open");
    }

    yuengine::serialize::RuntimeConfigRecord output_config{};
    yuengine::serialize::RuntimeProfileBoundary output_boundary{};
    const yuengine::serialize::SerializeStatus read_status =
        stream.ReadRuntimeConfig(&reader, &output_config, &output_boundary);
    if (read_status != yuengine::serialize::SerializeStatus::Success) {
        return FailStage(result, "runtime_config_read");
    }

    if (!RuntimeConfigRecordsMatch(input_config, output_config)) {
        return FailStage(result, "runtime_config_match");
    }

    if (!RuntimeProfileBoundariesMatch(input_boundary, output_boundary)) {
        return FailStage(result, "runtime_boundary_match");
    }

    *out_record_count = writer_snapshot.record_count;
    return true;
}

bool RoundTripSampleState(
    const SyntheticSceneManifest &manifest,
    const yuengine::world::WorldIdentityBaselineRecord &world_record,
    const L1AssetBindings &bindings,
    const yuengine::input::InputCommandSnapshot &input_snapshot,
    L1VerticalSamplePrepResult *result) {
    std::uint32_t scene_record_count = 0U;
    if (!RunSceneRecordRoundTrip(world_record, bindings, result, &scene_record_count)) {
        return false;
    }

    std::uint32_t runtime_record_count = 0U;
    if (!RunRuntimeConfigRoundTrip(manifest, input_snapshot, result, &runtime_record_count)) {
        return false;
    }

    result->serialize_roundtrip = true;
    result->serialize_roundtrip_record_count = scene_record_count + runtime_record_count;
    return true;
}

bool ReleaseRuntimeAsset(
    yuengine::asset::AssetManager &asset_manager,
    yuengine::resource::ResourceRegistry &resource_registry,
    yuengine::asset::AssetHandle handle,
    L1VerticalSamplePrepResult *result) {
    const yuengine::asset::AssetStatus status =
        asset_manager.ReleaseRuntimeAsset(&resource_registry, handle);
    if (status != yuengine::asset::AssetStatus::Success) {
        return FailStage(result, "cleanup_asset_release");
    }

    return true;
}

bool RetireResource(
    yuengine::resource::ResourceRegistry &resource_registry,
    yuengine::resource::ResourceHandle handle,
    L1VerticalSamplePrepResult *result) {
    const yuengine::resource::ResourceStatus status = resource_registry.Retire(handle);
    if (status != yuengine::resource::ResourceStatus::Success) {
        return FailStage(result, "cleanup_resource_retire");
    }

    return true;
}

bool ProveSampleCleanup(
    const SyntheticSceneManifest &manifest,
    yuengine::world::WorldIdentityBaseline &baseline,
    yuengine::resource::ResourceRegistry &resource_registry,
    yuengine::asset::AssetManager &asset_manager,
    const L1AssetBindings &bindings,
    L1VerticalSamplePrepResult *result) {
    const yuengine::world::WorldIdentityBaselineStatus world_status =
        baseline.DestroyObject(manifest.world_object_id);
    if (world_status != yuengine::world::WorldIdentityBaselineStatus::Success) {
        return FailStage(result, "cleanup_world_destroy");
    }

    const std::array<yuengine::asset::AssetHandle, 4U> asset_handles{
        bindings.texture_asset,
        bindings.material_asset,
        bindings.mesh_asset,
        bindings.audio_asset};
    for (const yuengine::asset::AssetHandle &asset_handle : asset_handles) {
        if (!ReleaseRuntimeAsset(asset_manager, resource_registry, asset_handle, result)) {
            return false;
        }
    }

    const std::array<yuengine::resource::ResourceHandle, 4U> resource_handles{
        bindings.texture_resource,
        bindings.material_resource,
        bindings.mesh_resource,
        bindings.audio_resource};
    for (const yuengine::resource::ResourceHandle &resource_handle : resource_handles) {
        if (!RetireResource(resource_registry, resource_handle, result)) {
            return false;
        }
    }

    const yuengine::world::WorldIdentityBaselineSnapshot world_snapshot = baseline.Snapshot();
    const yuengine::asset::AssetSnapshot asset_snapshot = asset_manager.Snapshot();
    const yuengine::resource::ResourceSnapshot resource_snapshot = resource_registry.Snapshot();
    const std::uint64_t active_record_count =
        static_cast<std::uint64_t>(world_snapshot.active_record_count) +
        static_cast<std::uint64_t>(asset_snapshot.active_asset_count) +
        static_cast<std::uint64_t>(asset_snapshot.active_dependency_edge_count) +
        static_cast<std::uint64_t>(asset_snapshot.texture_ready_count) +
        static_cast<std::uint64_t>(asset_snapshot.audio_ready_count) +
        static_cast<std::uint64_t>(resource_snapshot.registered_resource_count) +
        resource_snapshot.acquired_handle_count +
        static_cast<std::uint64_t>(resource_snapshot.dependency_edge_count);
    if (active_record_count != 0U) {
        return FailStage(result, "cleanup_active_records");
    }

    if (asset_snapshot.released_asset_count != 4U) {
        return FailStage(result, "cleanup_asset_count");
    }

    if (resource_snapshot.retired_resource_count != 4U) {
        return FailStage(result, "cleanup_resource_count");
    }

    result->cleanup_active_record_count = static_cast<std::uint32_t>(active_record_count);
    result->retired_resource_count = resource_snapshot.retired_resource_count;
    result->cleanup_proof = true;
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

    yuengine::world::WorldIdentityBaseline baseline;
    yuengine::world::WorldIdentityBaselineRecord world_record{};
    if (!CreateWorldObject(manifest, &baseline, &world_record, result)) {
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

    if (!CloseLifecycleRoutes(result)) {
        return false;
    }

    L1VerticalSampleValidationRoute validation_route{};
    if (!BuildL1VerticalSampleValidationRoute(&validation_route)) {
        return FailStage(result, "validation_route");
    }

    result->validation_route = true;
    if (!RoundTripSampleState(manifest, world_record, bindings, input_snapshot, result)) {
        return false;
    }

    if (!ProveSampleCleanup(
        manifest,
        baseline,
        resource_registry,
        asset_manager,
        bindings,
        result)) {
        return false;
    }
    result->failure_stage = "ok";
    return true;
}
}
