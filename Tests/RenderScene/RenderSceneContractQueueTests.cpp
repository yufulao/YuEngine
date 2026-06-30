// 模块: Tests RenderScene
// 文件: Tests/RenderScene/RenderSceneContractQueueTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/RenderCore/RenderViewPacketRequest.h"
#include "YuEngine/RenderScene/RenderSceneCameraRecord.h"
#include "YuEngine/RenderScene/RenderSceneContractQueue.h"
#include "YuEngine/RenderScene/RenderSceneEntityRecord.h"
#include "YuEngine/RenderScene/RenderSceneStatus.h"
#include "YuEngine/RenderScene/RenderSceneSubmitRequest.h"
#include "YuEngine/RenderScene/RenderSceneSubmitResult.h"
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
#include "YuEngine/World/WorldObjectId.h"

using yuengine::asset::AssetHandle;
using yuengine::rendercore::RenderViewPacketRequest;
using yuengine::renderscene::RenderSceneCameraRecord;
using yuengine::renderscene::RenderSceneContractQueue;
using yuengine::renderscene::RenderSceneEntityRecord;
using yuengine::renderscene::RenderSceneStatus;
using yuengine::renderscene::RenderSceneSubmitRequest;
using yuengine::renderscene::RenderSceneSubmitResult;
using yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiColor;
using yuengine::rhi::RhiDrawIndexedDesc;
using yuengine::rhi::RhiIndexBufferView;
using yuengine::rhi::RhiIndexFormat;
using yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiPrimitiveTopology;
using yuengine::rhi::RhiSampledTextureBinding;
using yuengine::rhi::RhiSamplerBinding;
using yuengine::rhi::RhiSamplerHandle;
using yuengine::rhi::RhiTextureHandle;
using yuengine::rhi::RhiVertexBufferView;
using yuengine::world::WorldObjectId;

namespace {
constexpr const char *TEST_BUILD = "RenderScene_BuildsRenderCoreViewPacket";
constexpr const char *TEST_FULL_LIST_VISIBILITY =
    "RenderScene_FullListVisibilityOutputsOnlyVisibleEntities";
constexpr const char *TEST_MISSING_MESH =
    "RenderScene_MissingMeshAssetDoesNotMutateOutput";
constexpr const char *TEST_MISSING_MATERIAL =
    "RenderScene_MissingMaterialAssetDoesNotMutateOutput";
constexpr const char *TEST_MISSING_CAMERA =
    "RenderScene_MissingCameraDoesNotMutateOutput";
constexpr const char *TEST_OUTPUT_CAPACITY =
    "RenderScene_OutputCapacityFailureDoesNotMutateOutput";
constexpr const char *TEST_BOUNDARY = "RenderScene_NoNativeOrUpperDependency";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t FRAME_ID = 1001U;
constexpr std::uint32_t CAMERA_ID = 2001U;
constexpr std::uint32_t PASS_ID = 3001U;
constexpr std::uint32_t MATERIAL_ID = 4001U;
constexpr std::uint32_t PROGRAM_ID = 5001U;
constexpr std::uint32_t DRAW_ID = 6001U;
constexpr std::size_t CAPTURE_BYTES = 16U;
constexpr std::size_t VERTEX_STRIDE_BYTES = 24U;
constexpr std::size_t VERTEX_BUFFER_BYTES = VERTEX_STRIDE_BYTES * 3U;
constexpr std::size_t INDEX_BUFFER_BYTES = sizeof(std::uint16_t) * 3U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
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

RenderSceneCameraRecord CameraRecord() {
    RenderSceneCameraRecord camera{};
    camera.camera_id = CAMERA_ID;
    camera.target = RhiTextureHandle{3U, 1U};
    camera.clear_color = RhiColor{16U, 32U, 48U, 255U};
    camera.is_active = true;
    return camera;
}

RenderSceneEntityRecord EntityRecord(std::span<const std::uint8_t> constants, std::uint32_t draw_id=DRAW_ID) {
    RenderSceneEntityRecord entity{};
    entity.world_object_id = WorldObjectId{11U};
    entity.mesh_asset = AssetHandle{2U, 1U};
    entity.material_asset = AssetHandle{3U, 1U};
    entity.texture_ready.sampled_texture = RhiSampledTextureBinding{RhiTextureHandle{4U, 1U}, 0U};
    entity.texture_ready.is_ready = true;
    entity.material.material_id = MATERIAL_ID;
    entity.material.program_id = PROGRAM_ID;
    entity.material.pipeline = RhiPipelineHandle{5U, 1U};
    entity.material.sampler = RhiSamplerBinding{RhiSamplerHandle{6U, 1U}, 0U};
    entity.material.constant_bytes = constants;
    entity.material.pass_id = PASS_ID;
    entity.draw.draw_id = draw_id;
    entity.draw.pass_id = PASS_ID;
    entity.draw.material_id = MATERIAL_ID;
    entity.draw.vertex_buffer = VertexBufferView();
    entity.draw.index_buffer = IndexBufferView();
    entity.draw.draw = RhiDrawIndexedDesc{RhiPrimitiveTopology::TriangleList, 3U, 0U, 0};
    entity.is_active = true;
    return entity;
}

RenderSceneSubmitRequest SubmitRequest(
    std::span<const RenderSceneCameraRecord> cameras,
    std::span<const RenderSceneEntityRecord> entities,
    std::span<std::uint8_t> capture) {
    RenderSceneSubmitRequest request{};
    request.frame_id = FRAME_ID;
    request.active_camera_id = CAMERA_ID;
    request.cameras = cameras;
    request.entities = entities;
    request.capture_output = capture;
    request.capture_byte_budget = capture.size();
    return request;
}

int RenderSceneBuildsRenderCoreViewPacket() {
    std::array<std::uint8_t, CAPTURE_BYTES> capture{};
    std::array<std::uint8_t, 4U> constants{1U, 2U, 3U, 4U};
    const std::array<RenderSceneCameraRecord, 1U> cameras{CameraRecord()};
    const std::array<RenderSceneEntityRecord, 1U> entities{EntityRecord(constants)};
    std::array<RenderViewPacketRequest, 1U> packets{};
    RenderSceneSubmitResult result{};

    RenderSceneContractQueue queue;
    const RenderSceneStatus status =
        queue.BuildRenderCorePackets(SubmitRequest(cameras, entities, capture), packets, &result);
    if (status != RenderSceneStatus::Success) {
        return Fail("render scene packet build failed");
    }

    if (result.output_packet_count != 1U) {
        return Fail("render scene output count mismatch");
    }

    if (packets[0].frame_id != FRAME_ID) {
        return Fail("render scene frame id mismatch");
    }

    if (packets[0].view_id != DRAW_ID) {
        return Fail("render scene view id mismatch");
    }

    if (packets[0].target.generation != 1U) {
        return Fail("render scene target was not copied");
    }

    if (packets[0].material.sampled_texture.texture.generation != 1U) {
        return Fail("render scene texture ready binding was not copied");
    }

    if (packets[0].draw.draw.index_count != 3U) {
        return Fail("render scene draw record was not copied");
    }

    return 0;
}

int RenderSceneFullListVisibilityOutputsOnlyVisibleEntities() {
    std::array<std::uint8_t, CAPTURE_BYTES> capture{};
    std::array<std::uint8_t, 4U> constants{1U, 2U, 3U, 4U};
    const std::array<RenderSceneCameraRecord, 1U> cameras{CameraRecord()};
    std::array<RenderSceneEntityRecord, 4U> entities{
        EntityRecord(constants, DRAW_ID),
        EntityRecord(constants, DRAW_ID + 1U),
        EntityRecord(constants, DRAW_ID + 2U),
        EntityRecord(constants, DRAW_ID + 3U),
    };
    entities[1].is_visible = false;
    entities[2].is_active = false;
    std::array<RenderViewPacketRequest, 2U> packets{};
    RenderSceneSubmitResult result{};

    RenderSceneContractQueue queue;
    const RenderSceneStatus status =
        queue.BuildRenderCorePackets(SubmitRequest(cameras, entities, capture), packets, &result);
    if (status != RenderSceneStatus::Success) {
        return Fail("render scene full list submit failed");
    }

    if (result.visible_entity_count != 2U) {
        return Fail("render scene visible entity count mismatch");
    }

    if (result.output_packet_count != 2U) {
        return Fail("render scene full list output count mismatch");
    }

    if (result.skipped_entity_count != 2U) {
        return Fail("render scene skipped entity count mismatch");
    }

    if (packets[0].view_id != DRAW_ID) {
        return Fail("render scene first visible entity mismatch");
    }

    if (packets[1].view_id != DRAW_ID + 3U) {
        return Fail("render scene second visible entity mismatch");
    }

    if (queue.Snapshot().last_skipped_entity_count != 2U) {
        return Fail("render scene snapshot skipped entity count mismatch");
    }

    return 0;
}

int RenderSceneMissingMeshAssetDoesNotMutateOutput() {
    std::array<std::uint8_t, CAPTURE_BYTES> capture{};
    std::array<std::uint8_t, 4U> constants{1U, 2U, 3U, 4U};
    const std::array<RenderSceneCameraRecord, 1U> cameras{CameraRecord()};
    std::array<RenderSceneEntityRecord, 1U> entities{EntityRecord(constants)};
    entities[0].mesh_asset = AssetHandle{};
    std::array<RenderViewPacketRequest, 1U> packets{};
    packets[0].view_id = 88U;
    RenderSceneSubmitResult result{};

    RenderSceneContractQueue queue;
    const RenderSceneStatus status =
        queue.BuildRenderCorePackets(SubmitRequest(cameras, entities, capture), packets, &result);
    if (status != RenderSceneStatus::MissingMeshAsset) {
        return Fail("render scene did not report missing mesh asset");
    }

    if (packets[0].view_id != 88U) {
        return Fail("render scene mutated output on failed mesh validation");
    }

    return 0;
}

int RenderSceneMissingMaterialAssetDoesNotMutateOutput() {
    std::array<std::uint8_t, CAPTURE_BYTES> capture{};
    std::array<std::uint8_t, 4U> constants{1U, 2U, 3U, 4U};
    const std::array<RenderSceneCameraRecord, 1U> cameras{CameraRecord()};
    std::array<RenderSceneEntityRecord, 1U> entities{EntityRecord(constants)};
    entities[0].material_asset = AssetHandle{};
    std::array<RenderViewPacketRequest, 1U> packets{};
    packets[0].view_id = 99U;
    RenderSceneSubmitResult result{};

    RenderSceneContractQueue queue;
    const RenderSceneStatus status =
        queue.BuildRenderCorePackets(SubmitRequest(cameras, entities, capture), packets, &result);
    if (status != RenderSceneStatus::MissingMaterialAsset) {
        return Fail("render scene did not report missing material asset");
    }

    if (packets[0].view_id != 99U) {
        return Fail("render scene mutated output on failed material validation");
    }

    return 0;
}

int RenderSceneMissingCameraDoesNotMutateOutput() {
    std::array<std::uint8_t, CAPTURE_BYTES> capture{};
    std::array<std::uint8_t, 4U> constants{1U, 2U, 3U, 4U};
    std::array<RenderSceneCameraRecord, 1U> cameras{CameraRecord()};
    cameras[0].is_active = false;
    const std::array<RenderSceneEntityRecord, 1U> entities{EntityRecord(constants)};
    std::array<RenderViewPacketRequest, 1U> packets{};
    packets[0].view_id = 66U;
    RenderSceneSubmitResult result{};

    RenderSceneContractQueue queue;
    const RenderSceneStatus status =
        queue.BuildRenderCorePackets(SubmitRequest(cameras, entities, capture), packets, &result);
    if (status != RenderSceneStatus::MissingCamera) {
        return Fail("render scene did not report missing camera");
    }

    if (packets[0].view_id != 66U) {
        return Fail("render scene mutated output on missing camera");
    }

    return 0;
}

int RenderSceneOutputCapacityFailureDoesNotMutateOutput() {
    std::array<std::uint8_t, CAPTURE_BYTES> capture{};
    std::array<std::uint8_t, 4U> constants{1U, 2U, 3U, 4U};
    const std::array<RenderSceneCameraRecord, 1U> cameras{CameraRecord()};
    const std::array<RenderSceneEntityRecord, 2U> entities{
        EntityRecord(constants, DRAW_ID),
        EntityRecord(constants, DRAW_ID + 1U),
    };
    std::array<RenderViewPacketRequest, 1U> packets{};
    packets[0].view_id = 77U;
    RenderSceneSubmitResult result{};

    RenderSceneContractQueue queue;
    const RenderSceneStatus status =
        queue.BuildRenderCorePackets(SubmitRequest(cameras, entities, capture), packets, &result);
    if (status != RenderSceneStatus::OutputCapacityExceeded) {
        return Fail("render scene did not report output capacity");
    }

    if (result.status != RenderSceneStatus::OutputCapacityExceeded) {
        return Fail("render scene capacity result status mismatch");
    }

    if (result.visible_entity_count != 2U) {
        return Fail("render scene capacity visible count mismatch");
    }

    if (packets[0].view_id != 77U) {
        return Fail("render scene mutated output on capacity failure");
    }

    if (queue.Snapshot().last_status != RenderSceneStatus::OutputCapacityExceeded) {
        return Fail("render scene capacity snapshot status mismatch");
    }

    if (queue.Snapshot().last_visible_entity_count != 2U) {
        return Fail("render scene capacity snapshot visible count mismatch");
    }

    return 0;
}

int RenderSceneNoNativeOrUpperDependency() {
    RenderSceneContractQueue queue;
    if (queue.Snapshot().last_status != RenderSceneStatus::Success) {
        return Fail("render scene default snapshot status changed");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_BUILD) {
        return RenderSceneBuildsRenderCoreViewPacket();
    }

    if (name == TEST_FULL_LIST_VISIBILITY) {
        return RenderSceneFullListVisibilityOutputsOnlyVisibleEntities();
    }

    if (name == TEST_MISSING_MESH) {
        return RenderSceneMissingMeshAssetDoesNotMutateOutput();
    }

    if (name == TEST_MISSING_MATERIAL) {
        return RenderSceneMissingMaterialAssetDoesNotMutateOutput();
    }

    if (name == TEST_MISSING_CAMERA) {
        return RenderSceneMissingCameraDoesNotMutateOutput();
    }

    if (name == TEST_OUTPUT_CAPACITY) {
        return RenderSceneOutputCapacityFailureDoesNotMutateOutput();
    }

    if (name == TEST_BOUNDARY) {
        return RenderSceneNoNativeOrUpperDependency();
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
