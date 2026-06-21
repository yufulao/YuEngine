// 模块: Tests RenderScene
// 文件: Tests/RenderScene/RenderSceneRuntimeVisualFoundationTests.cpp

#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/RenderCore/RenderCameraProjectionKind.h"
#include "YuEngine/RenderScene/RenderSceneCameraBindingRequest.h"
#include "YuEngine/RenderScene/RenderSceneCameraBindingResult.h"
#include "YuEngine/RenderScene/RenderSceneCameraFrameBinder.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryBuilder.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryKind.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryRecord.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryRequest.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryStatus.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeCameraRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameDrawRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameEntityRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameResult.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameStatus.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialRequest.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialStatus.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialTextureSlot.h"
#include "YuEngine/RenderScene/RenderSceneStatus.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"
#include "YuEngine/Rhi/RhiSamplerHandle.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

using yuengine::asset::AssetHandle;
using yuengine::rendercore::RenderCameraProjectionKind;
using yuengine::renderscene::RenderSceneCameraBindingRequest;
using yuengine::renderscene::RenderSceneCameraBindingResult;
using yuengine::renderscene::RenderSceneCameraFrameBinder;
using yuengine::renderscene::RenderScenePrimitiveGeometryBuilder;
using yuengine::renderscene::RenderScenePrimitiveGeometryKind;
using yuengine::renderscene::RenderScenePrimitiveGeometryRecord;
using yuengine::renderscene::RenderScenePrimitiveGeometryRequest;
using yuengine::renderscene::RenderScenePrimitiveGeometryStatus;
using yuengine::renderscene::RenderSceneRuntimeCameraRecord;
using yuengine::renderscene::RenderSceneRuntimeFrameBuilder;
using yuengine::renderscene::RenderSceneRuntimeFrameDrawRecord;
using yuengine::renderscene::RenderSceneRuntimeFrameEntityRequest;
using yuengine::renderscene::RenderSceneRuntimeFrameRequest;
using yuengine::renderscene::RenderSceneRuntimeFrameResult;
using yuengine::renderscene::RenderSceneRuntimeFrameStatus;
using yuengine::renderscene::RenderSceneRuntimeMaterialBuilder;
using yuengine::renderscene::RenderSceneRuntimeMaterialRecord;
using yuengine::renderscene::RenderSceneRuntimeMaterialRequest;
using yuengine::renderscene::RenderSceneRuntimeMaterialStatus;
using yuengine::renderscene::RenderSceneRuntimeMaterialTextureSlot;
using yuengine::renderscene::RenderSceneStatus;
using yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiColor;
using yuengine::rhi::RhiIndexBufferView;
using yuengine::rhi::RhiIndexFormat;
using yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiPrimitiveTopology;
using yuengine::rhi::RhiSamplerHandle;
using yuengine::rhi::RhiTextureHandle;
using yuengine::rhi::RhiVertexBufferView;
using yuengine::world::WorldObjectId;
using yuengine::world::WorldTransformState;

namespace {
constexpr const char *TEST_CAMERA_FRAME =
    "RenderScene_RuntimeCameraRecordBuildsDeterministicFrame";
constexpr const char *TEST_CAMERA_MISSING =
    "RenderScene_RuntimeCameraActiveBindingRejectsMissingCamera";
constexpr const char *TEST_CAMERA_CAPTURE =
    "RenderScene_RuntimeCameraCaptureMetadataRecordsFrameAndTarget";
constexpr const char *TEST_GEOMETRY_RANGES =
    "RenderScene_PrimitiveGeometryBuildsCubeCylinderConeRanges";
constexpr const char *TEST_GEOMETRY_MISSING =
    "RenderScene_PrimitiveGeometryMissingRecordReportsStatus";
constexpr const char *TEST_GEOMETRY_SMALL_BUFFER =
    "RenderScene_PrimitiveGeometryRejectsSmallBufferRanges";
constexpr const char *TEST_MATERIAL_THREE_SLOTS =
    "RenderScene_RuntimeMaterialBindsThreeTextureSlots";
constexpr const char *TEST_MATERIAL_MISSING_SLOT =
    "RenderScene_RuntimeMaterialRejectsMissingThirdSlot";
constexpr const char *TEST_MATERIAL_INVALID_TEXTURE =
    "RenderScene_RuntimeMaterialReportsInvalidTextureAsset";
constexpr const char *TEST_MATERIAL_INVALID_TEXTURE_BINDING =
    "RenderScene_RuntimeMaterialReportsInvalidTextureBinding";
constexpr const char *TEST_MATERIAL_INVALID_SAMPLER =
    "RenderScene_RuntimeMaterialReportsInvalidSamplerBinding";
constexpr const char *TEST_MATERIAL_INVALID_PIPELINE =
    "RenderScene_RuntimeMaterialReportsInvalidPipeline";
constexpr const char *TEST_FRAME_THREE_ENTITIES =
    "RenderScene_RuntimeFrameSubmitsThreeEntitiesWithSharedMaterial";
constexpr const char *TEST_FRAME_DUPLICATE_TRANSFORM =
    "RenderScene_RuntimeFrameRejectsDuplicateTransforms";
constexpr const char *TEST_FRAME_OUTPUT_CAPACITY =
    "RenderScene_RuntimeFrameRejectsSmallOutputCapacity";
constexpr const char *TEST_FRAME_MISSING_MATERIAL =
    "RenderScene_RuntimeFrameReportsMissingMaterial";
constexpr const char *TEST_FRAME_MISSING_GEOMETRY =
    "RenderScene_RuntimeFrameReportsMissingGeometry";
constexpr const char *TEST_BOUNDARY =
    "RenderScene_RuntimeVisualFoundationNoEditorWebUiInputDependency";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr float HALF_PI = 1.57079632679F;
constexpr float TOLERANCE = 0.0001F;
constexpr std::uint32_t FRAME_ID = 9101U;
constexpr std::uint32_t CAMERA_ID = 9201U;
constexpr std::uint32_t DRAW_ID = 9301U;
constexpr std::uint32_t PASS_ID = 9401U;
constexpr std::uint32_t MATERIAL_ID = 9501U;
constexpr std::uint32_t MATERIAL_ASSET_SLOT = 9601U;
constexpr std::uint32_t TEXTURE_ASSET_SLOT = 9701U;
constexpr std::size_t VERTEX_STRIDE_BYTES = 32U;
constexpr std::size_t VERTEX_BUFFER_BYTES = VERTEX_STRIDE_BYTES * 128U;
constexpr std::size_t INDEX_BUFFER_BYTES = sizeof(std::uint16_t) * 256U;
constexpr std::size_t CAPTURE_BUDGET = 4096U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool Approx(float left, float right) {
    const float delta = std::fabs(left - right);
    return delta <= TOLERANCE;
}

RenderSceneRuntimeCameraRecord CameraRecord(std::uint32_t camera_id=CAMERA_ID) {
    RenderSceneRuntimeCameraRecord camera{};
    camera.camera_id = camera_id;
    camera.pose.position = {0.0F, 0.0F, -5.0F};
    camera.pose.target = {0.0F, 0.0F, 0.0F};
    camera.pose.up = {0.0F, 1.0F, 0.0F};
    camera.projection.kind = RenderCameraProjectionKind::Perspective;
    camera.projection.vertical_fov_radians = HALF_PI;
    camera.projection.aspect_ratio = 1.0F;
    camera.projection.near_z = 0.1F;
    camera.projection.far_z = 100.0F;
    camera.target = RhiTextureHandle{7U, 1U};
    camera.clear_color = RhiColor{10U, 20U, 30U, 255U};
    camera.is_active = true;
    return camera;
}

RenderSceneCameraBindingRequest CameraRequest(
    std::span<const RenderSceneRuntimeCameraRecord> cameras,
    std::uint32_t active_camera_id=CAMERA_ID) {
    RenderSceneCameraBindingRequest request{};
    request.frame_id = FRAME_ID;
    request.active_camera_id = active_camera_id;
    request.cameras = cameras;
    request.capture_byte_budget = CAPTURE_BUDGET;
    request.capture_requested = true;
    return request;
}

RhiVertexBufferView VertexBufferView() {
    RhiVertexBufferView view{};
    view.buffer = RhiBufferHandle{1U, 1U};
    view.stride_bytes = VERTEX_STRIDE_BYTES;
    view.size_bytes = VERTEX_BUFFER_BYTES;
    return view;
}

RhiIndexBufferView IndexBufferView() {
    RhiIndexBufferView view{};
    view.buffer = RhiBufferHandle{2U, 1U};
    view.size_bytes = INDEX_BUFFER_BYTES;
    view.format = RhiIndexFormat::Uint16;
    return view;
}

RenderScenePrimitiveGeometryRequest GeometryRequest(RenderScenePrimitiveGeometryKind kind) {
    RenderScenePrimitiveGeometryRequest request{};
    request.geometry_asset = AssetHandle{3U, 1U};
    request.kind = kind;
    request.segment_count = 16U;
    request.draw_id = DRAW_ID;
    request.pass_id = PASS_ID;
    request.material_id = MATERIAL_ID;
    request.vertex_buffer = VertexBufferView();
    request.index_buffer = IndexBufferView();
    return request;
}

AssetHandle MakeAsset(std::uint32_t slot) {
    return AssetHandle{slot, 1U};
}

RhiPipelineHandle MakePipelineHandle() {
    return RhiPipelineHandle{4U, 1U};
}

RenderSceneRuntimeMaterialTextureSlot MakeMaterialTextureSlot(std::uint32_t slot) {
    RenderSceneRuntimeMaterialTextureSlot texture_slot{};
    texture_slot.slot = slot;
    texture_slot.texture_asset = MakeAsset(TEXTURE_ASSET_SLOT + slot);
    texture_slot.sampled_texture.texture = RhiTextureHandle{10U + slot, 1U};
    texture_slot.sampled_texture.slot = slot;
    texture_slot.sampler.sampler = RhiSamplerHandle{20U + slot, 1U};
    texture_slot.sampler.slot = slot;
    return texture_slot;
}

RenderSceneRuntimeMaterialRequest MakeMaterialRequest(
    std::span<const RenderSceneRuntimeMaterialTextureSlot> slots) {
    RenderSceneRuntimeMaterialRequest request{};
    request.material_asset = MakeAsset(MATERIAL_ASSET_SLOT);
    request.material_id = MATERIAL_ID;
    request.pipeline = MakePipelineHandle();
    request.texture_slots = slots;
    return request;
}

RenderSceneCameraBindingResult MakeCameraBinding() {
    const std::array<RenderSceneRuntimeCameraRecord, 1U> cameras{CameraRecord()};
    RenderSceneCameraBindingResult result{};
    RenderSceneCameraFrameBinder binder;
    binder.BuildActiveCameraFrame(CameraRequest(cameras), &result);
    return result;
}

RenderScenePrimitiveGeometryRecord MakePrimitiveGeometryRecord(RenderScenePrimitiveGeometryKind kind) {
    RenderScenePrimitiveGeometryBuilder builder;
    RenderScenePrimitiveGeometryRecord record{};
    builder.Build(GeometryRequest(kind), &record);
    return record;
}

RenderSceneRuntimeMaterialRecord MakeRuntimeMaterialRecord() {
    const std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U),
        MakeMaterialTextureSlot(2U)};
    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    builder.Build(MakeMaterialRequest(slots), &record);
    return record;
}

WorldTransformState MakeTransform(float x, float y, float z) {
    WorldTransformState transform{};
    transform.translation_x = x;
    transform.translation_y = y;
    transform.translation_z = z;
    return transform;
}

RenderSceneRuntimeFrameEntityRequest MakeRuntimeFrameEntity(
    std::uint32_t world_object_id,
    const WorldTransformState &transform,
    RenderScenePrimitiveGeometryKind kind) {
    RenderSceneRuntimeFrameEntityRequest entity{};
    entity.world_object_id = WorldObjectId{world_object_id};
    entity.transform = transform;
    entity.geometry = MakePrimitiveGeometryRecord(kind);
    entity.is_visible = true;
    entity.is_active = true;
    return entity;
}

RenderSceneRuntimeFrameRequest MakeRuntimeFrameRequest(
    const RenderSceneCameraBindingResult &camera,
    const RenderSceneRuntimeMaterialRecord &material,
    std::span<const RenderSceneRuntimeFrameEntityRequest> entities) {
    RenderSceneRuntimeFrameRequest request{};
    request.frame_id = FRAME_ID;
    request.camera = camera;
    request.material = material;
    request.entities = entities;
    return request;
}

int RenderSceneRuntimeCameraRecordBuildsDeterministicFrame() {
    const std::array<RenderSceneRuntimeCameraRecord, 1U> cameras{CameraRecord()};
    RenderSceneCameraBindingResult result{};
    RenderSceneCameraFrameBinder binder;
    const RenderSceneStatus status = binder.BuildActiveCameraFrame(CameraRequest(cameras), &result);
    if (status != RenderSceneStatus::Success) {
        return Fail("runtime camera binding failed");
    }

    if (result.camera.camera_id != CAMERA_ID || !result.camera.is_active) {
        return Fail("runtime camera binding did not select active camera");
    }

    if (!Approx(result.camera.frame.view.values[14U], 5.0F)) {
        return Fail("runtime camera view matrix was not deterministic");
    }

    if (!Approx(result.camera.frame.projection.values[0U], 1.0F)) {
        return Fail("runtime camera projection was not deterministic");
    }

    return 0;
}

int RenderSceneRuntimeCameraActiveBindingRejectsMissingCamera() {
    const std::array<RenderSceneRuntimeCameraRecord, 1U> cameras{CameraRecord()};
    RenderSceneCameraBindingResult result{};
    result.camera.camera_id = 77U;

    RenderSceneCameraFrameBinder binder;
    const RenderSceneStatus status = binder.BuildActiveCameraFrame(CameraRequest(cameras, CAMERA_ID + 1U), &result);
    if (status != RenderSceneStatus::MissingCamera) {
        return Fail("runtime camera binding did not report missing camera");
    }

    if (result.camera.camera_id != 0U) {
        return Fail("runtime camera binding leaked stale output on missing camera");
    }

    return 0;
}

int RenderSceneRuntimeCameraCaptureMetadataRecordsFrameAndTarget() {
    const std::array<RenderSceneRuntimeCameraRecord, 1U> cameras{CameraRecord()};
    RenderSceneCameraBindingResult result{};

    RenderSceneCameraFrameBinder binder;
    const RenderSceneStatus status = binder.BuildActiveCameraFrame(CameraRequest(cameras), &result);
    if (status != RenderSceneStatus::Success) {
        return Fail("runtime camera capture setup failed");
    }

    if (!result.capture.capture_requested) {
        return Fail("runtime camera capture flag was not recorded");
    }

    if (result.capture.frame_id != FRAME_ID || result.capture.camera_id != CAMERA_ID) {
        return Fail("runtime camera capture identity metadata mismatch");
    }

    if (result.capture.target.generation != 1U) {
        return Fail("runtime camera capture target metadata mismatch");
    }

    if (result.capture.output_byte_budget != CAPTURE_BUDGET) {
        return Fail("runtime camera capture budget metadata mismatch");
    }

    return 0;
}

int RenderScenePrimitiveGeometryBuildsCubeCylinderConeRanges() {
    RenderScenePrimitiveGeometryBuilder builder;
    RenderScenePrimitiveGeometryRecord cube{};
    RenderScenePrimitiveGeometryRecord cylinder{};
    RenderScenePrimitiveGeometryRecord cone{};

    if (builder.Build(GeometryRequest(RenderScenePrimitiveGeometryKind::Cube), &cube) !=
        RenderScenePrimitiveGeometryStatus::Success) {
        return Fail("cube geometry record failed");
    }

    if (builder.Build(GeometryRequest(RenderScenePrimitiveGeometryKind::Cylinder), &cylinder) !=
        RenderScenePrimitiveGeometryStatus::Success) {
        return Fail("cylinder geometry record failed");
    }

    if (builder.Build(GeometryRequest(RenderScenePrimitiveGeometryKind::Cone), &cone) !=
        RenderScenePrimitiveGeometryStatus::Success) {
        return Fail("cone geometry record failed");
    }

    if (cube.vertex_count != 24U || cube.index_count != 36U) {
        return Fail("cube geometry bounds mismatch");
    }

    if (cylinder.vertex_count != 34U || cylinder.index_count != 192U) {
        return Fail("cylinder geometry bounds mismatch");
    }

    if (cone.vertex_count != 18U || cone.index_count != 96U) {
        return Fail("cone geometry bounds mismatch");
    }

    if (cube.draw.draw.topology != RhiPrimitiveTopology::TriangleList) {
        return Fail("primitive geometry topology mismatch");
    }

    if (builder.Validate(cube) != RenderScenePrimitiveGeometryStatus::Success) {
        return Fail("cube geometry validation failed");
    }

    return 0;
}

int RenderScenePrimitiveGeometryMissingRecordReportsStatus() {
    RenderScenePrimitiveGeometryBuilder builder;
    RenderScenePrimitiveGeometryRecord record{};
    const RenderScenePrimitiveGeometryStatus status = builder.Validate(record);
    if (status != RenderScenePrimitiveGeometryStatus::MissingGeometryRecord) {
        return Fail("primitive geometry did not report missing record");
    }

    return 0;
}

int RenderScenePrimitiveGeometryRejectsSmallBufferRanges() {
    RenderScenePrimitiveGeometryBuilder builder;
    RenderScenePrimitiveGeometryRequest request = GeometryRequest(RenderScenePrimitiveGeometryKind::Cylinder);
    request.index_buffer.size_bytes = sizeof(std::uint16_t) * 16U;
    RenderScenePrimitiveGeometryRecord record{};
    const RenderScenePrimitiveGeometryStatus status = builder.Build(request, &record);
    if (status != RenderScenePrimitiveGeometryStatus::InvalidDrawRecord) {
        return Fail("primitive geometry accepted undersized index buffer");
    }

    return 0;
}

int RenderSceneRuntimeMaterialBindsThreeTextureSlots() {
    const std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        MakeMaterialTextureSlot(2U),
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U)};
    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    const RenderSceneRuntimeMaterialStatus status = builder.Build(MakeMaterialRequest(slots), &record);
    if (status != RenderSceneRuntimeMaterialStatus::Success) {
        return Fail("runtime material three slot build failed");
    }

    if (record.material_id != MATERIAL_ID || record.texture_slot_count != 3U) {
        return Fail("runtime material identity or slot count mismatch");
    }

    if (record.texture_slots[0U].slot != 0U || record.texture_slots[1U].slot != 1U) {
        return Fail("runtime material slots were not sorted");
    }

    if (record.texture_slots[2U].sampled_texture.slot != 2U) {
        return Fail("runtime material sampled texture slot mismatch");
    }

    if (record.texture_slots[2U].sampler.slot != 2U) {
        return Fail("runtime material sampler slot mismatch");
    }

    if (builder.Validate(record) != RenderSceneRuntimeMaterialStatus::Success) {
        return Fail("runtime material validation failed");
    }

    return 0;
}

int RenderSceneRuntimeMaterialRejectsMissingThirdSlot() {
    const std::array<RenderSceneRuntimeMaterialTextureSlot, 2U> slots{
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U)};
    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    const RenderSceneRuntimeMaterialStatus status = builder.Build(MakeMaterialRequest(slots), &record);
    if (status != RenderSceneRuntimeMaterialStatus::MissingTextureSlot) {
        return Fail("runtime material did not report missing third slot");
    }

    return 0;
}

int RenderSceneRuntimeMaterialReportsInvalidTextureAsset() {
    std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U),
        MakeMaterialTextureSlot(2U)};
    slots[1U].texture_asset = AssetHandle{};

    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    const RenderSceneRuntimeMaterialStatus status = builder.Build(MakeMaterialRequest(slots), &record);
    if (status != RenderSceneRuntimeMaterialStatus::InvalidTextureAsset) {
        return Fail("runtime material did not report invalid texture asset");
    }

    return 0;
}

int RenderSceneRuntimeMaterialReportsInvalidTextureBinding() {
    std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U),
        MakeMaterialTextureSlot(2U)};
    slots[0U].sampled_texture.texture.generation = 0U;

    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    const RenderSceneRuntimeMaterialStatus status = builder.Build(MakeMaterialRequest(slots), &record);
    if (status != RenderSceneRuntimeMaterialStatus::InvalidTextureBinding) {
        return Fail("runtime material did not report invalid texture binding");
    }

    return 0;
}

int RenderSceneRuntimeMaterialReportsInvalidSamplerBinding() {
    std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U),
        MakeMaterialTextureSlot(2U)};
    slots[2U].sampler.sampler.generation = 0U;

    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    const RenderSceneRuntimeMaterialStatus status = builder.Build(MakeMaterialRequest(slots), &record);
    if (status != RenderSceneRuntimeMaterialStatus::InvalidSamplerBinding) {
        return Fail("runtime material did not report invalid sampler binding");
    }

    return 0;
}

int RenderSceneRuntimeMaterialReportsInvalidPipeline() {
    const std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        MakeMaterialTextureSlot(0U),
        MakeMaterialTextureSlot(1U),
        MakeMaterialTextureSlot(2U)};
    RenderSceneRuntimeMaterialRequest request = MakeMaterialRequest(slots);
    request.pipeline.generation = 0U;

    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    const RenderSceneRuntimeMaterialStatus status = builder.Build(request, &record);
    if (status != RenderSceneRuntimeMaterialStatus::InvalidPipeline) {
        return Fail("runtime material did not report invalid pipeline");
    }

    return 0;
}

int RenderSceneRuntimeFrameSubmitsThreeEntitiesWithSharedMaterial() {
    const RenderSceneCameraBindingResult camera = MakeCameraBinding();
    const RenderSceneRuntimeMaterialRecord material = MakeRuntimeMaterialRecord();
    const std::array<RenderSceneRuntimeFrameEntityRequest, 3U> entities{
        MakeRuntimeFrameEntity(101U, MakeTransform(-2.0F, 0.0F, 0.0F), RenderScenePrimitiveGeometryKind::Cube),
        MakeRuntimeFrameEntity(102U, MakeTransform(0.0F, 1.0F, 0.0F), RenderScenePrimitiveGeometryKind::Cylinder),
        MakeRuntimeFrameEntity(103U, MakeTransform(2.0F, 0.0F, 1.0F), RenderScenePrimitiveGeometryKind::Cone)};
    std::array<RenderSceneRuntimeFrameDrawRecord, 3U> draws{};
    RenderSceneRuntimeFrameResult result{};

    RenderSceneRuntimeFrameBuilder builder;
    const RenderSceneRuntimeFrameStatus status =
        builder.Build(MakeRuntimeFrameRequest(camera, material, entities), draws, &result);
    if (status != RenderSceneRuntimeFrameStatus::Success) {
        return Fail("runtime frame three entity submission failed");
    }

    if (result.output_draw_count != 3U || result.material_id != MATERIAL_ID) {
        return Fail("runtime frame output count or material mismatch");
    }

    if (draws[0U].draw.material_id != MATERIAL_ID || draws[1U].draw.material_id != MATERIAL_ID) {
        return Fail("runtime frame did not share material across draws");
    }

    if (draws[2U].geometry_kind != RenderScenePrimitiveGeometryKind::Cone) {
        return Fail("runtime frame geometry kind mismatch");
    }

    if (draws[0U].transform.translation_x == draws[1U].transform.translation_x) {
        return Fail("runtime frame transforms were not distinct");
    }

    if (draws[1U].draw.draw.index_count != 192U) {
        return Fail("runtime frame cylinder draw range mismatch");
    }

    return 0;
}

int RenderSceneRuntimeFrameRejectsDuplicateTransforms() {
    const RenderSceneCameraBindingResult camera = MakeCameraBinding();
    const RenderSceneRuntimeMaterialRecord material = MakeRuntimeMaterialRecord();
    const WorldTransformState transform = MakeTransform(1.0F, 0.0F, 0.0F);
    const std::array<RenderSceneRuntimeFrameEntityRequest, 2U> entities{
        MakeRuntimeFrameEntity(101U, transform, RenderScenePrimitiveGeometryKind::Cube),
        MakeRuntimeFrameEntity(102U, transform, RenderScenePrimitiveGeometryKind::Cylinder)};
    std::array<RenderSceneRuntimeFrameDrawRecord, 2U> draws{};
    RenderSceneRuntimeFrameResult result{};

    RenderSceneRuntimeFrameBuilder builder;
    const RenderSceneRuntimeFrameStatus status =
        builder.Build(MakeRuntimeFrameRequest(camera, material, entities), draws, &result);
    if (status != RenderSceneRuntimeFrameStatus::DuplicateTransform) {
        return Fail("runtime frame did not report duplicate transform");
    }

    return 0;
}

int RenderSceneRuntimeFrameRejectsSmallOutputCapacity() {
    const RenderSceneCameraBindingResult camera = MakeCameraBinding();
    const RenderSceneRuntimeMaterialRecord material = MakeRuntimeMaterialRecord();
    const std::array<RenderSceneRuntimeFrameEntityRequest, 3U> entities{
        MakeRuntimeFrameEntity(101U, MakeTransform(-2.0F, 0.0F, 0.0F), RenderScenePrimitiveGeometryKind::Cube),
        MakeRuntimeFrameEntity(102U, MakeTransform(0.0F, 1.0F, 0.0F), RenderScenePrimitiveGeometryKind::Cylinder),
        MakeRuntimeFrameEntity(103U, MakeTransform(2.0F, 0.0F, 1.0F), RenderScenePrimitiveGeometryKind::Cone)};
    std::array<RenderSceneRuntimeFrameDrawRecord, 2U> draws{};
    RenderSceneRuntimeFrameResult result{};

    RenderSceneRuntimeFrameBuilder builder;
    const RenderSceneRuntimeFrameStatus status =
        builder.Build(MakeRuntimeFrameRequest(camera, material, entities), draws, &result);
    if (status != RenderSceneRuntimeFrameStatus::OutputCapacityExceeded) {
        return Fail("runtime frame did not report output capacity");
    }

    return 0;
}

int RenderSceneRuntimeFrameReportsMissingMaterial() {
    const RenderSceneCameraBindingResult camera = MakeCameraBinding();
    const RenderSceneRuntimeMaterialRecord material{};
    const std::array<RenderSceneRuntimeFrameEntityRequest, 1U> entities{
        MakeRuntimeFrameEntity(101U, MakeTransform(0.0F, 0.0F, 0.0F), RenderScenePrimitiveGeometryKind::Cube)};
    std::array<RenderSceneRuntimeFrameDrawRecord, 1U> draws{};
    RenderSceneRuntimeFrameResult result{};

    RenderSceneRuntimeFrameBuilder builder;
    const RenderSceneRuntimeFrameStatus status =
        builder.Build(MakeRuntimeFrameRequest(camera, material, entities), draws, &result);
    if (status != RenderSceneRuntimeFrameStatus::MissingMaterialRecord) {
        return Fail("runtime frame did not report missing material");
    }

    return 0;
}

int RenderSceneRuntimeFrameReportsMissingGeometry() {
    const RenderSceneCameraBindingResult camera = MakeCameraBinding();
    const RenderSceneRuntimeMaterialRecord material = MakeRuntimeMaterialRecord();
    std::array<RenderSceneRuntimeFrameEntityRequest, 1U> entities{
        MakeRuntimeFrameEntity(101U, MakeTransform(0.0F, 0.0F, 0.0F), RenderScenePrimitiveGeometryKind::Cube)};
    entities[0U].geometry = RenderScenePrimitiveGeometryRecord{};
    std::array<RenderSceneRuntimeFrameDrawRecord, 1U> draws{};
    RenderSceneRuntimeFrameResult result{};

    RenderSceneRuntimeFrameBuilder builder;
    const RenderSceneRuntimeFrameStatus status =
        builder.Build(MakeRuntimeFrameRequest(camera, material, entities), draws, &result);
    if (status != RenderSceneRuntimeFrameStatus::MissingGeometryRecord) {
        return Fail("runtime frame did not report missing geometry");
    }

    return 0;
}

int RenderSceneRuntimeVisualFoundationNoEditorWebUiInputDependency() {
    RenderSceneCameraFrameBinder binder;
    RenderScenePrimitiveGeometryBuilder builder;
    RenderSceneRuntimeFrameBuilder frame_builder;
    RenderSceneRuntimeMaterialBuilder material_builder;
    RenderScenePrimitiveGeometryRecord record{};
    const RenderScenePrimitiveGeometryStatus status = builder.Validate(record);
    if (status != RenderScenePrimitiveGeometryStatus::MissingGeometryRecord) {
        return Fail("runtime visual boundary setup failed");
    }

    const std::array<RenderSceneRuntimeCameraRecord, 1U> cameras{CameraRecord()};
    RenderSceneCameraBindingResult result{};
    if (binder.BuildActiveCameraFrame(CameraRequest(cameras), &result) != RenderSceneStatus::Success) {
        return Fail("runtime visual boundary camera setup failed");
    }

    RenderSceneRuntimeMaterialRecord material_record{};
    if (material_builder.Validate(material_record) != RenderSceneRuntimeMaterialStatus::MissingMaterialRecord) {
        return Fail("runtime visual boundary material setup failed");
    }

    RenderSceneRuntimeFrameResult frame_result{};
    std::array<RenderSceneRuntimeFrameDrawRecord, 1U> draws{};
    RenderSceneRuntimeFrameRequest frame_request{};
    frame_request.frame_id = FRAME_ID;
    if (frame_builder.Build(frame_request, draws, &frame_result) != RenderSceneRuntimeFrameStatus::MissingCamera) {
        return Fail("runtime visual boundary frame setup failed");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_CAMERA_FRAME) {
        return RenderSceneRuntimeCameraRecordBuildsDeterministicFrame();
    }

    if (name == TEST_CAMERA_MISSING) {
        return RenderSceneRuntimeCameraActiveBindingRejectsMissingCamera();
    }

    if (name == TEST_CAMERA_CAPTURE) {
        return RenderSceneRuntimeCameraCaptureMetadataRecordsFrameAndTarget();
    }

    if (name == TEST_GEOMETRY_RANGES) {
        return RenderScenePrimitiveGeometryBuildsCubeCylinderConeRanges();
    }

    if (name == TEST_GEOMETRY_MISSING) {
        return RenderScenePrimitiveGeometryMissingRecordReportsStatus();
    }

    if (name == TEST_GEOMETRY_SMALL_BUFFER) {
        return RenderScenePrimitiveGeometryRejectsSmallBufferRanges();
    }

    if (name == TEST_MATERIAL_THREE_SLOTS) {
        return RenderSceneRuntimeMaterialBindsThreeTextureSlots();
    }

    if (name == TEST_MATERIAL_MISSING_SLOT) {
        return RenderSceneRuntimeMaterialRejectsMissingThirdSlot();
    }

    if (name == TEST_MATERIAL_INVALID_TEXTURE) {
        return RenderSceneRuntimeMaterialReportsInvalidTextureAsset();
    }

    if (name == TEST_MATERIAL_INVALID_TEXTURE_BINDING) {
        return RenderSceneRuntimeMaterialReportsInvalidTextureBinding();
    }

    if (name == TEST_MATERIAL_INVALID_SAMPLER) {
        return RenderSceneRuntimeMaterialReportsInvalidSamplerBinding();
    }

    if (name == TEST_MATERIAL_INVALID_PIPELINE) {
        return RenderSceneRuntimeMaterialReportsInvalidPipeline();
    }

    if (name == TEST_FRAME_THREE_ENTITIES) {
        return RenderSceneRuntimeFrameSubmitsThreeEntitiesWithSharedMaterial();
    }

    if (name == TEST_FRAME_DUPLICATE_TRANSFORM) {
        return RenderSceneRuntimeFrameRejectsDuplicateTransforms();
    }

    if (name == TEST_FRAME_OUTPUT_CAPACITY) {
        return RenderSceneRuntimeFrameRejectsSmallOutputCapacity();
    }

    if (name == TEST_FRAME_MISSING_MATERIAL) {
        return RenderSceneRuntimeFrameReportsMissingMaterial();
    }

    if (name == TEST_FRAME_MISSING_GEOMETRY) {
        return RenderSceneRuntimeFrameReportsMissingGeometry();
    }

    if (name == TEST_BOUNDARY) {
        return RenderSceneRuntimeVisualFoundationNoEditorWebUiInputDependency();
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
