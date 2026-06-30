// 模块：Tests RenderCore
// 文件：Tests/RenderCore/RenderViewPacketTests.cpp

#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>
#include <vector>

#include "YuEngine/RenderCore/RenderCamera.h"
#include "YuEngine/RenderCore/RenderCameraProjectionKind.h"
#include "YuEngine/RenderCore/RenderCameraShaderConstants.h"
#include "YuEngine/RenderCore/RenderCameraShaderConstantsWriter.h"
#include "YuEngine/RenderCore/RenderFixturePassRequest.h"
#include "YuEngine/RenderCore/RenderMaterialStatus.h"
#include "YuEngine/RenderCore/RenderViewPacket.h"
#include "YuEngine/RenderCore/RenderViewPacketDesc.h"
#include "YuEngine/RenderCore/RenderViewPacketRequest.h"
#include "YuEngine/RenderCore/RenderViewPacketStatus.h"
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

using RenderCamera = yuengine::rendercore::RenderCamera;
using RenderCameraFrame = yuengine::rendercore::RenderCameraFrame;
using RenderCameraPose = yuengine::rendercore::RenderCameraPose;
using RenderCameraProjectionDesc = yuengine::rendercore::RenderCameraProjectionDesc;
using RenderCameraShaderConstants = yuengine::rendercore::RenderCameraShaderConstants;
using RenderCameraShaderConstantsWriter = yuengine::rendercore::RenderCameraShaderConstantsWriter;
using RenderFixturePassRequest = yuengine::rendercore::RenderFixturePassRequest;
using yuengine::rendercore::RenderCameraProjectionKind;
using yuengine::rendercore::RenderDrawPacketStatus;
using yuengine::rendercore::RenderMaterialStatus;
using RenderViewPacket = yuengine::rendercore::RenderViewPacket;
using RenderViewPacketDesc = yuengine::rendercore::RenderViewPacketDesc;
using RenderViewPacketRequest = yuengine::rendercore::RenderViewPacketRequest;
using yuengine::rendercore::RenderViewPacketStatus;
using RhiBufferHandle = yuengine::rhi::RhiBufferHandle;
using RhiColor = yuengine::rhi::RhiColor;
using RhiIndexBufferView = yuengine::rhi::RhiIndexBufferView;
using yuengine::rhi::RhiIndexFormat;
using RhiPipelineHandle = yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiPrimitiveTopology;
using RhiSampledTextureBinding = yuengine::rhi::RhiSampledTextureBinding;
using RhiSamplerBinding = yuengine::rhi::RhiSamplerBinding;
using RhiSamplerHandle = yuengine::rhi::RhiSamplerHandle;
using RhiTextureHandle = yuengine::rhi::RhiTextureHandle;
using RhiVertexBufferView = yuengine::rhi::RhiVertexBufferView;

namespace {
constexpr const char *TEST_BUILDS_PASS = "RenderCore_ViewPacketBuildsCameraMaterialDrawPass";
constexpr const char *TEST_PASS_MISMATCH = "RenderCore_ViewPacketRejectsPassMismatchWithoutOutputMutation";
constexpr const char *TEST_MATERIAL_FAILURE = "RenderCore_ViewPacketRejectsMaterialFailureWithoutOutputMutation";
constexpr const char *TEST_DRAW_FAILURE = "RenderCore_ViewPacketRejectsDrawFailureWithoutOutputMutation";
constexpr const char *TEST_DUPLICATE_ID = "RenderCore_ViewPacketRejectsDuplicateViewId";
constexpr const char *TEST_CAPACITY = "RenderCore_ViewPacketRejectsCapacityExceeded";
constexpr const char *TEST_SNAPSHOT = "RenderCore_ViewPacketSnapshotTracksCounters";
constexpr const char *TEST_BOUNDARY = "RenderCore_ViewPacketHasNarrowDependencyBoundary";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t VIEW_ID = 501U;
constexpr std::uint32_t NEXT_VIEW_ID = 503U;
constexpr std::uint32_t FRAME_ID = 601U;
constexpr std::uint32_t PASS_ID = 701U;
constexpr std::uint32_t MATERIAL_ID = 801U;
constexpr std::uint32_t DRAW_ID = 901U;
constexpr std::size_t CAPTURE_BYTES = 16U;
constexpr std::size_t VERTEX_STRIDE_BYTES = 24U;
constexpr std::size_t VERTEX_BUFFER_BYTES = VERTEX_STRIDE_BYTES * 3U;
constexpr std::size_t INDEX_BUFFER_BYTES = sizeof(std::uint16_t) * 3U;
constexpr float HALF_PI = 1.57079632679F;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

RenderCameraShaderConstants CameraConstants() {
    RenderCameraPose pose{};
    pose.position = {0.0F, 0.0F, -4.0F};
    pose.target = {0.0F, 0.0F, 0.0F};
    pose.up = {0.0F, 1.0F, 0.0F};

    RenderCameraProjectionDesc projection{};
    projection.kind = RenderCameraProjectionKind::Perspective;
    projection.vertical_fov_radians = HALF_PI;
    projection.aspect_ratio = 1.0F;
    projection.near_z = 0.1F;
    projection.far_z = 100.0F;

    RenderCamera camera;
    RenderCameraFrame frame{};
    camera.BuildFrame(pose, projection, &frame);

    RenderCameraShaderConstantsWriter writer;
    RenderCameraShaderConstants constants{};
    writer.WriteViewProjection(frame, &constants);
    return constants;
}

RhiVertexBufferView VertexBufferView() {
    RhiVertexBufferView view{};
    view.buffer = RhiBufferHandle{1U, 1U};
    view.offset_bytes = 0U;
    view.stride_bytes = VERTEX_STRIDE_BYTES;
    view.size_bytes = VERTEX_BUFFER_BYTES;
    return view;
}

RhiIndexBufferView IndexBufferView() {
    RhiIndexBufferView view{};
    view.buffer = RhiBufferHandle{2U, 1U};
    view.offset_bytes = 0U;
    view.size_bytes = INDEX_BUFFER_BYTES;
    view.format = RhiIndexFormat::Uint16;
    return view;
}

RenderViewPacketRequest DefaultRequest(
    const RenderCameraShaderConstants &constants,
    std::vector<std::uint8_t> &capture,
    std::uint32_t view_id=VIEW_ID) {
    const auto byte_count = constants.view_projection_values.size() * sizeof(float);
    const auto *bytes = reinterpret_cast<const std::uint8_t *>(constants.view_projection_values.data());

    RenderViewPacketRequest request{};
    request.view_id = view_id;
    request.frame_id = FRAME_ID;
    request.target = RhiTextureHandle{1U, 1U};
    request.clear_color = RhiColor{4U, 8U, 12U, 255U};
    request.capture_output = std::span<std::uint8_t>(capture.data(), capture.size());
    request.capture_byte_budget = capture.size();
    request.material.material_id = MATERIAL_ID;
    request.material.program_id = 11U;
    request.material.pipeline = RhiPipelineHandle{3U, 1U};
    request.material.sampled_texture = RhiSampledTextureBinding{RhiTextureHandle{4U, 1U}, 0U};
    request.material.sampler = RhiSamplerBinding{RhiSamplerHandle{5U, 1U}, 0U};
    request.material.constant_bytes = std::span<const std::uint8_t>(bytes, byte_count);
    request.material.pass_id = PASS_ID;
    request.draw.draw_id = DRAW_ID;
    request.draw.pass_id = PASS_ID;
    request.draw.material_id = MATERIAL_ID;
    request.draw.vertex_buffer = VertexBufferView();
    request.draw.index_buffer = IndexBufferView();
    request.draw.draw.topology = RhiPrimitiveTopology::TriangleList;
    request.draw.draw.index_count = 3U;
    request.draw.draw.first_index = 0U;
    request.draw.draw.vertex_offset = 0;
    return request;
}

RenderFixturePassRequest SentinelPassRequest() {
    RenderFixturePassRequest request{};
    request.target = RhiTextureHandle{77U, 77U};
    request.pipeline = RhiPipelineHandle{77U, 77U};
    request.pass_id = 77U;
    request.material_id = 77U;
    request.capture_byte_budget = 77U;
    return request;
}

bool PassRequestMatchesSentinel(const RenderFixturePassRequest &request) {
    if (request.target.slot != 77U || request.target.generation != 77U) {
        return false;
    }

    if (request.pipeline.slot != 77U || request.pipeline.generation != 77U) {
        return false;
    }

    if (request.pass_id != 77U || request.material_id != 77U) {
        return false;
    }

    return request.capture_byte_budget == 77U;
}

bool PassRequestMatchesView(const RenderFixturePassRequest &request) {
    if (request.target.generation != 1U || request.pipeline.generation != 1U) {
        return false;
    }

    if (request.vertex_buffer.buffer.generation != 1U || request.index_buffer.buffer.generation != 1U) {
        return false;
    }

    if (request.sampled_texture.texture.generation != 1U || request.sampler.sampler.generation != 1U) {
        return false;
    }

    if (request.pass_id != PASS_ID || request.material_id != MATERIAL_ID) {
        return false;
    }

    if (request.material_constant_byte_count != yuengine::rendercore::RENDER_CAMERA_SHADER_CONSTANT_BYTES) {
        return false;
    }

    return request.draw.index_count == 3U;
}

int RenderCoreViewPacketBuildsCameraMaterialDrawPass() {
    const RenderCameraShaderConstants constants = CameraConstants();
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, 0U);
    RenderViewPacket packet;
    RenderFixturePassRequest pass_request = SentinelPassRequest();

    const auto result = packet.BuildPassRequest(DefaultRequest(constants, capture), &pass_request);
    if (result.status != RenderViewPacketStatus::Success) {
        return Fail("view packet rejected valid request");
    }

    if (!PassRequestMatchesView(pass_request)) {
        return Fail("view packet did not write camera material draw fields");
    }

    const auto snapshot = packet.Snapshot();
    if (snapshot.accepted_view_count != 1U || snapshot.last_status != RenderViewPacketStatus::Success) {
        return Fail("view packet snapshot missed accepted request");
    }

    return 0;
}

int RenderCoreViewPacketRejectsPassMismatchWithoutOutputMutation() {
    const RenderCameraShaderConstants constants = CameraConstants();
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, 0U);
    RenderViewPacketRequest request = DefaultRequest(constants, capture);
    request.material.pass_id = PASS_ID + 1U;

    RenderViewPacket packet;
    RenderFixturePassRequest pass_request = SentinelPassRequest();
    const auto result = packet.BuildPassRequest(request, &pass_request);
    if (result.status != RenderViewPacketStatus::MismatchedPassId) {
        return Fail("view packet accepted mismatched pass id");
    }

    if (!PassRequestMatchesSentinel(pass_request)) {
        return Fail("view packet mutated output after pass mismatch");
    }

    return 0;
}

int RenderCoreViewPacketRejectsMaterialFailureWithoutOutputMutation() {
    const RenderCameraShaderConstants constants = CameraConstants();
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, 0U);
    RenderViewPacketRequest request = DefaultRequest(constants, capture);
    request.material.pipeline = RhiPipelineHandle{};

    RenderViewPacket packet;
    RenderFixturePassRequest pass_request = SentinelPassRequest();
    const auto result = packet.BuildPassRequest(request, &pass_request);
    if (result.status != RenderViewPacketStatus::MaterialFailed) {
        return Fail("view packet accepted invalid material");
    }

    if (result.material_status != RenderMaterialStatus::InvalidPipeline) {
        return Fail("view packet reported wrong material status");
    }

    if (!PassRequestMatchesSentinel(pass_request)) {
        return Fail("view packet mutated output after material failure");
    }

    return 0;
}

int RenderCoreViewPacketRejectsDrawFailureWithoutOutputMutation() {
    const RenderCameraShaderConstants constants = CameraConstants();
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, 0U);
    RenderViewPacketRequest request = DefaultRequest(constants, capture);
    request.draw.draw.index_count = 0U;

    RenderViewPacket packet;
    RenderFixturePassRequest pass_request = SentinelPassRequest();
    const auto result = packet.BuildPassRequest(request, &pass_request);
    if (result.status != RenderViewPacketStatus::DrawFailed) {
        return Fail("view packet accepted invalid draw");
    }

    if (result.draw_status != RenderDrawPacketStatus::InvalidDraw) {
        return Fail("view packet reported wrong draw status");
    }

    if (!PassRequestMatchesSentinel(pass_request)) {
        return Fail("view packet mutated output after draw failure");
    }

    return 0;
}

int RenderCoreViewPacketRejectsDuplicateViewId() {
    const RenderCameraShaderConstants constants = CameraConstants();
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, 0U);
    RenderViewPacket packet;
    RenderFixturePassRequest pass_request{};
    if (packet.BuildPassRequest(DefaultRequest(constants, capture), &pass_request).status != RenderViewPacketStatus::Success) {
        return Fail("view packet duplicate setup failed");
    }

    RenderFixturePassRequest rejected_request = SentinelPassRequest();
    const auto result = packet.BuildPassRequest(DefaultRequest(constants, capture), &rejected_request);
    if (result.status != RenderViewPacketStatus::DuplicateViewId) {
        return Fail("view packet accepted duplicate view id");
    }

    if (!PassRequestMatchesSentinel(rejected_request)) {
        return Fail("view packet mutated output after duplicate view id");
    }

    return 0;
}

int RenderCoreViewPacketRejectsCapacityExceeded() {
    RenderViewPacketDesc desc{};
    desc.view_record_capacity = 1U;
    RenderViewPacket packet(desc);
    const RenderCameraShaderConstants constants = CameraConstants();
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, 0U);

    RenderFixturePassRequest pass_request{};
    if (packet.BuildPassRequest(DefaultRequest(constants, capture), &pass_request).status != RenderViewPacketStatus::Success) {
        return Fail("view packet capacity setup failed");
    }

    RenderFixturePassRequest rejected_request = SentinelPassRequest();
    const auto result = packet.BuildPassRequest(DefaultRequest(constants, capture, NEXT_VIEW_ID), &rejected_request);
    if (result.status != RenderViewPacketStatus::ViewCapacityExceeded) {
        return Fail("view packet accepted capacity overflow");
    }

    if (result.required_view_record_count != 2U) {
        return Fail("view packet capacity required count mismatch");
    }

    if (!PassRequestMatchesSentinel(rejected_request)) {
        return Fail("view packet mutated output after capacity overflow");
    }

    const auto snapshot = packet.Snapshot();
    if (snapshot.last_required_view_record_count != 2U) {
        return Fail("view packet snapshot required count mismatch");
    }

    return 0;
}

int RenderCoreViewPacketSnapshotTracksCounters() {
    const RenderCameraShaderConstants constants = CameraConstants();
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, 0U);
    RenderViewPacket packet;
    RenderFixturePassRequest pass_request{};
    if (packet.BuildPassRequest(DefaultRequest(constants, capture), &pass_request).status != RenderViewPacketStatus::Success) {
        return Fail("view packet snapshot setup failed");
    }

    RenderViewPacketRequest request = DefaultRequest(constants, capture, NEXT_VIEW_ID);
    request.material.pipeline = RhiPipelineHandle{};
    if (packet.BuildPassRequest(request, &pass_request).status != RenderViewPacketStatus::MaterialFailed) {
        return Fail("view packet material failure setup failed");
    }

    const auto snapshot = packet.Snapshot();
    if (snapshot.accepted_view_count != 1U || snapshot.material_failure_count != 1U) {
        return Fail("view packet snapshot counters were wrong");
    }

    packet.Reset();
    const auto reset_snapshot = packet.Snapshot();
    if (reset_snapshot.view_record_count != 0U || reset_snapshot.accepted_view_count != 0U) {
        return Fail("view packet reset did not clear counters");
    }

    return 0;
}

int RenderCoreViewPacketHasNarrowDependencyBoundary() {
    const RenderCameraShaderConstants constants = CameraConstants();
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, 0U);
    RenderViewPacket packet;
    RenderFixturePassRequest pass_request{};
    const auto result = packet.BuildPassRequest(DefaultRequest(constants, capture), &pass_request);
    if (result.status != RenderViewPacketStatus::Success) {
        return Fail("view packet dependency smoke setup failed");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_BUILDS_PASS) {
        return RenderCoreViewPacketBuildsCameraMaterialDrawPass();
    }

    if (name == TEST_PASS_MISMATCH) {
        return RenderCoreViewPacketRejectsPassMismatchWithoutOutputMutation();
    }

    if (name == TEST_MATERIAL_FAILURE) {
        return RenderCoreViewPacketRejectsMaterialFailureWithoutOutputMutation();
    }

    if (name == TEST_DRAW_FAILURE) {
        return RenderCoreViewPacketRejectsDrawFailureWithoutOutputMutation();
    }

    if (name == TEST_DUPLICATE_ID) {
        return RenderCoreViewPacketRejectsDuplicateViewId();
    }

    if (name == TEST_CAPACITY) {
        return RenderCoreViewPacketRejectsCapacityExceeded();
    }

    if (name == TEST_SNAPSHOT) {
        return RenderCoreViewPacketSnapshotTracksCounters();
    }

    if (name == TEST_BOUNDARY) {
        return RenderCoreViewPacketHasNarrowDependencyBoundary();
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
