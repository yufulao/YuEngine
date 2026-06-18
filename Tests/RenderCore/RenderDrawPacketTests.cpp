// 模块：Tests RenderCore
// 文件：Tests/RenderCore/RenderDrawPacketTests.cpp

#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/RenderCore/RenderDrawPacket.h"
#include "YuEngine/RenderCore/RenderDrawPacketDesc.h"
#include "YuEngine/RenderCore/RenderDrawPacketRequest.h"
#include "YuEngine/RenderCore/RenderDrawPacketStatus.h"
#include "YuEngine/RenderCore/RenderFixturePassRequest.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiDrawIndexedDesc.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"

using RenderDrawPacket = yuengine::rendercore::RenderDrawPacket;
using RenderDrawPacketDesc = yuengine::rendercore::RenderDrawPacketDesc;
using RenderDrawPacketRequest = yuengine::rendercore::RenderDrawPacketRequest;
using yuengine::rendercore::RenderDrawPacketStatus;
using RenderFixturePassRequest = yuengine::rendercore::RenderFixturePassRequest;
using RhiBufferHandle = yuengine::rhi::RhiBufferHandle;
using RhiDrawIndexedDesc = yuengine::rhi::RhiDrawIndexedDesc;
using RhiIndexBufferView = yuengine::rhi::RhiIndexBufferView;
using yuengine::rhi::RhiIndexFormat;
using RhiPipelineHandle = yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiPrimitiveTopology;
using RhiVertexBufferView = yuengine::rhi::RhiVertexBufferView;

namespace {
constexpr const char *TEST_BUILDS_PASS = "RenderCore_DrawPacketBuildsPassGeometry";
constexpr const char *TEST_INVALID_VERTEX = "RenderCore_DrawPacketRejectsMissingVertexWithoutOutputMutation";
constexpr const char *TEST_INVALID_INDEX = "RenderCore_DrawPacketRejectsMissingIndexWithoutOutputMutation";
constexpr const char *TEST_INVALID_DRAW = "RenderCore_DrawPacketRejectsInvalidDrawRangeWithoutOutputMutation";
constexpr const char *TEST_DUPLICATE_ID = "RenderCore_DrawPacketRejectsDuplicateDrawId";
constexpr const char *TEST_CAPACITY = "RenderCore_DrawPacketRejectsCapacityExceeded";
constexpr const char *TEST_SNAPSHOT = "RenderCore_DrawPacketSnapshotTracksCounters";
constexpr const char *TEST_BOUNDARY = "RenderCore_DrawPacketHasNarrowDependencyBoundary";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t DRAW_ID = 101U;
constexpr std::uint32_t NEXT_DRAW_ID = 103U;
constexpr std::uint32_t PASS_ID = 201U;
constexpr std::uint32_t MATERIAL_ID = 301U;
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

RhiDrawIndexedDesc DrawDesc() {
    RhiDrawIndexedDesc desc{};
    desc.topology = RhiPrimitiveTopology::TriangleList;
    desc.index_count = 3U;
    desc.first_index = 0U;
    desc.vertex_offset = 0;
    return desc;
}

RenderDrawPacketRequest DefaultRequest() {
    RenderDrawPacketRequest request{};
    request.draw_id = DRAW_ID;
    request.pass_id = PASS_ID;
    request.material_id = MATERIAL_ID;
    request.vertex_buffer = VertexBufferView();
    request.index_buffer = IndexBufferView();
    request.draw = DrawDesc();
    return request;
}

RenderFixturePassRequest SentinelPassRequest() {
    RenderFixturePassRequest request{};
    request.pipeline = RhiPipelineHandle{77U, 77U};
    request.pass_id = 77U;
    request.material_id = 77U;
    return request;
}

bool PassRequestMatchesSentinel(const RenderFixturePassRequest &request) {
    if (request.pipeline.slot != 77U || request.pipeline.generation != 77U) {
        return false;
    }

    if (request.pass_id != 77U) {
        return false;
    }

    return request.material_id == 77U;
}

bool PassRequestHasDrawFields(const RenderFixturePassRequest &request) {
    if (request.pass_id != PASS_ID || request.material_id != MATERIAL_ID) {
        return false;
    }

    if (request.vertex_buffer.buffer.generation != 1U || request.index_buffer.buffer.generation != 1U) {
        return false;
    }

    return request.draw.index_count == 3U;
}

int RenderCoreDrawPacketBuildsPassGeometry() {
    RenderDrawPacket packet;
    RenderFixturePassRequest pass_request = SentinelPassRequest();

    const auto result = packet.BuildPassRequest(DefaultRequest(), &pass_request);
    if (result.status != RenderDrawPacketStatus::Success) {
        return Fail("draw packet rejected valid request");
    }

    if (!PassRequestHasDrawFields(pass_request)) {
        return Fail("draw packet did not write pass geometry fields");
    }

    if (pass_request.pipeline.slot != 77U || pass_request.pipeline.generation != 77U) {
        return Fail("draw packet overwrote material pipeline fields");
    }

    const auto snapshot = packet.Snapshot();
    if (snapshot.accepted_draw_count != 1U || snapshot.last_status != RenderDrawPacketStatus::Success) {
        return Fail("draw packet snapshot missed accepted request");
    }

    return 0;
}

int RenderCoreDrawPacketRejectsMissingVertexWithoutOutputMutation() {
    RenderDrawPacketRequest request = DefaultRequest();
    request.vertex_buffer.buffer = RhiBufferHandle{};

    RenderDrawPacket packet;
    RenderFixturePassRequest pass_request = SentinelPassRequest();
    const auto result = packet.BuildPassRequest(request, &pass_request);
    if (result.status != RenderDrawPacketStatus::MissingVertexBuffer) {
        return Fail("draw packet accepted missing vertex buffer");
    }

    if (!PassRequestMatchesSentinel(pass_request)) {
        return Fail("draw packet mutated output after missing vertex buffer");
    }

    return 0;
}

int RenderCoreDrawPacketRejectsMissingIndexWithoutOutputMutation() {
    RenderDrawPacketRequest request = DefaultRequest();
    request.index_buffer.format = RhiIndexFormat::Unsupported;

    RenderDrawPacket packet;
    RenderFixturePassRequest pass_request = SentinelPassRequest();
    const auto result = packet.BuildPassRequest(request, &pass_request);
    if (result.status != RenderDrawPacketStatus::MissingIndexBuffer) {
        return Fail("draw packet accepted missing index buffer");
    }

    if (!PassRequestMatchesSentinel(pass_request)) {
        return Fail("draw packet mutated output after missing index buffer");
    }

    return 0;
}

int RenderCoreDrawPacketRejectsInvalidDrawRangeWithoutOutputMutation() {
    RenderDrawPacketRequest request = DefaultRequest();
    request.draw.first_index = 2U;
    request.draw.index_count = 3U;

    RenderDrawPacket packet;
    RenderFixturePassRequest pass_request = SentinelPassRequest();
    const auto result = packet.BuildPassRequest(request, &pass_request);
    if (result.status != RenderDrawPacketStatus::InvalidDraw) {
        return Fail("draw packet accepted invalid draw range");
    }

    if (!PassRequestMatchesSentinel(pass_request)) {
        return Fail("draw packet mutated output after invalid draw range");
    }

    return 0;
}

int RenderCoreDrawPacketRejectsDuplicateDrawId() {
    RenderDrawPacket packet;
    RenderFixturePassRequest pass_request{};
    if (packet.BuildPassRequest(DefaultRequest(), &pass_request).status != RenderDrawPacketStatus::Success) {
        return Fail("draw packet duplicate setup failed");
    }

    RenderFixturePassRequest rejected_request = SentinelPassRequest();
    const auto result = packet.BuildPassRequest(DefaultRequest(), &rejected_request);
    if (result.status != RenderDrawPacketStatus::DuplicateDrawId) {
        return Fail("draw packet accepted duplicate draw id");
    }

    if (!PassRequestMatchesSentinel(rejected_request)) {
        return Fail("draw packet mutated output after duplicate draw id");
    }

    return 0;
}

int RenderCoreDrawPacketRejectsCapacityExceeded() {
    RenderDrawPacketDesc desc{};
    desc.draw_record_capacity = 1U;
    RenderDrawPacket packet(desc);

    RenderFixturePassRequest pass_request{};
    if (packet.BuildPassRequest(DefaultRequest(), &pass_request).status != RenderDrawPacketStatus::Success) {
        return Fail("draw packet capacity setup failed");
    }

    RenderDrawPacketRequest request = DefaultRequest();
    request.draw_id = NEXT_DRAW_ID;
    RenderFixturePassRequest rejected_request = SentinelPassRequest();
    const auto result = packet.BuildPassRequest(request, &rejected_request);
    if (result.status != RenderDrawPacketStatus::DrawCapacityExceeded) {
        return Fail("draw packet accepted capacity overflow");
    }

    if (!PassRequestMatchesSentinel(rejected_request)) {
        return Fail("draw packet mutated output after capacity overflow");
    }

    return 0;
}

int RenderCoreDrawPacketSnapshotTracksCounters() {
    RenderDrawPacket packet;
    RenderFixturePassRequest pass_request{};
    if (packet.BuildPassRequest(DefaultRequest(), &pass_request).status != RenderDrawPacketStatus::Success) {
        return Fail("draw packet snapshot setup failed");
    }

    RenderDrawPacketRequest request = DefaultRequest();
    request.draw_id = NEXT_DRAW_ID;
    request.draw.index_count = 0U;
    if (packet.BuildPassRequest(request, &pass_request).status != RenderDrawPacketStatus::InvalidDraw) {
        return Fail("draw packet rejection setup failed");
    }

    const auto snapshot = packet.Snapshot();
    if (snapshot.accepted_draw_count != 1U || snapshot.failed_validation_count != 1U) {
        return Fail("draw packet snapshot counters were wrong");
    }

    packet.Reset();
    const auto reset_snapshot = packet.Snapshot();
    if (reset_snapshot.draw_record_count != 0U || reset_snapshot.accepted_draw_count != 0U) {
        return Fail("draw packet reset did not clear counters");
    }

    return 0;
}

int RenderCoreDrawPacketHasNarrowDependencyBoundary() {
    RenderDrawPacket packet;
    RenderFixturePassRequest pass_request{};
    const auto result = packet.BuildPassRequest(DefaultRequest(), &pass_request);
    if (result.status != RenderDrawPacketStatus::Success) {
        return Fail("draw packet dependency smoke setup failed");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_BUILDS_PASS) {
        return RenderCoreDrawPacketBuildsPassGeometry();
    }

    if (name == TEST_INVALID_VERTEX) {
        return RenderCoreDrawPacketRejectsMissingVertexWithoutOutputMutation();
    }

    if (name == TEST_INVALID_INDEX) {
        return RenderCoreDrawPacketRejectsMissingIndexWithoutOutputMutation();
    }

    if (name == TEST_INVALID_DRAW) {
        return RenderCoreDrawPacketRejectsInvalidDrawRangeWithoutOutputMutation();
    }

    if (name == TEST_DUPLICATE_ID) {
        return RenderCoreDrawPacketRejectsDuplicateDrawId();
    }

    if (name == TEST_CAPACITY) {
        return RenderCoreDrawPacketRejectsCapacityExceeded();
    }

    if (name == TEST_SNAPSHOT) {
        return RenderCoreDrawPacketSnapshotTracksCounters();
    }

    if (name == TEST_BOUNDARY) {
        return RenderCoreDrawPacketHasNarrowDependencyBoundary();
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
