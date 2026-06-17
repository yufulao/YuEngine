// Module: Tests RenderCore
// File: Tests/RenderCore/RenderGraphExecutionPlanTests.cpp

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>
#include <vector>

#include "YuEngine/RenderCore/RenderFixturePass.h"
#include "YuEngine/RenderCore/RenderFixturePassRequest.h"
#include "YuEngine/RenderCore/RenderFixturePassResult.h"
#include "YuEngine/RenderCore/RenderFixturePassSnapshot.h"
#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/RenderCore/RenderFramePacketFixture.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureSnapshot.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureStatus.h"
#include "YuEngine/RenderCore/RenderGraphExecutionPlan.h"
#include "YuEngine/RenderCore/RenderGraphExecutionPlanDesc.h"
#include "YuEngine/RenderCore/RenderGraphExecutionPlanRecord.h"
#include "YuEngine/RenderCore/RenderGraphExecutionPlanRequest.h"
#include "YuEngine/RenderCore/RenderGraphExecutionPlanStatus.h"
#include "YuEngine/RenderCore/RenderGraphExecutionPlanOperation.h"
#include "YuEngine/RenderCore/RenderGraphSkeleton.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonDependencyDeclaration.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonPassDeclaration.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonRequest.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonResult.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonSnapshot.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonStatus.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixture.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureSnapshot.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureStatus.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/NullRhiDevice.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiBufferUsage.h"
#include "YuEngine/Rhi/RhiColorTargetDesc.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiDeviceSnapshot.h"
#include "YuEngine/Rhi/RhiDrawIndexedDesc.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiInputLayoutDesc.h"
#include "YuEngine/Rhi/RhiPipelineDesc.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"
#include "YuEngine/Rhi/RhiSamplerDesc.h"
#include "YuEngine/Rhi/RhiSamplerHandle.h"
#include "YuEngine/Rhi/RhiShaderModuleDesc.h"
#include "YuEngine/Rhi/RhiShaderModuleHandle.h"
#include "YuEngine/Rhi/RhiShaderStage.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"

using RenderFixturePass = yuengine::rendercore::RenderFixturePass;
using RenderFixturePassRequest = yuengine::rendercore::RenderFixturePassRequest;
using RenderFixturePassResult = yuengine::rendercore::RenderFixturePassResult;
using RenderFixturePassSnapshot = yuengine::rendercore::RenderFixturePassSnapshot;
using yuengine::rendercore::RenderFixturePassStatus;
using RenderFramePacketFixture = yuengine::rendercore::RenderFramePacketFixture;
using RenderFramePacketFixtureSnapshot = yuengine::rendercore::RenderFramePacketFixtureSnapshot;
using yuengine::rendercore::RenderFramePacketFixtureStatus;
using RenderGraphExecutionPlan = yuengine::rendercore::RenderGraphExecutionPlan;
using RenderGraphExecutionPlanDesc = yuengine::rendercore::RenderGraphExecutionPlanDesc;
using yuengine::rendercore::RenderGraphExecutionPlanOperation;
using RenderGraphExecutionPlanRecord = yuengine::rendercore::RenderGraphExecutionPlanRecord;
using RenderGraphExecutionPlanRequest = yuengine::rendercore::RenderGraphExecutionPlanRequest;
using yuengine::rendercore::RenderGraphExecutionPlanStatus;
using RenderGraphSkeleton = yuengine::rendercore::RenderGraphSkeleton;
using RenderGraphSkeletonDependencyDeclaration = yuengine::rendercore::RenderGraphSkeletonDependencyDeclaration;
using RenderGraphSkeletonPassDeclaration = yuengine::rendercore::RenderGraphSkeletonPassDeclaration;
using RenderGraphSkeletonRequest = yuengine::rendercore::RenderGraphSkeletonRequest;
using RenderGraphSkeletonResult = yuengine::rendercore::RenderGraphSkeletonResult;
using RenderGraphSkeletonSnapshot = yuengine::rendercore::RenderGraphSkeletonSnapshot;
using yuengine::rendercore::RenderGraphSkeletonStatus;
using RenderSubmissionBatchFixture = yuengine::rendercore::RenderSubmissionBatchFixture;
using RenderSubmissionBatchFixtureSnapshot = yuengine::rendercore::RenderSubmissionBatchFixtureSnapshot;
using yuengine::rendercore::RenderSubmissionBatchFixtureStatus;
using IRhiDevice = yuengine::rhi::IRhiDevice;
using NullRhiDevice = yuengine::rhi::NullRhiDevice;
using RhiBufferDesc = yuengine::rhi::RhiBufferDesc;
using RhiBufferHandle = yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiBufferUsage;
using RhiColor = yuengine::rhi::RhiColor;
using RhiColorTargetDesc = yuengine::rhi::RhiColorTargetDesc;
using RhiDeviceDesc = yuengine::rhi::RhiDeviceDesc;
using RhiDeviceSnapshot = yuengine::rhi::RhiDeviceSnapshot;
using RhiDrawIndexedDesc = yuengine::rhi::RhiDrawIndexedDesc;
using yuengine::rhi::RhiFormat;
using RhiIndexBufferView = yuengine::rhi::RhiIndexBufferView;
using yuengine::rhi::RhiIndexFormat;
using yuengine::rhi::RhiInputElementFormat;
using yuengine::rhi::RhiInputElementSemantic;
using RhiInputLayoutDesc = yuengine::rhi::RhiInputLayoutDesc;
using RhiPipelineDesc = yuengine::rhi::RhiPipelineDesc;
using RhiPipelineHandle = yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiPrimitiveTopology;
using RhiSampledTextureBinding = yuengine::rhi::RhiSampledTextureBinding;
using RhiSamplerBinding = yuengine::rhi::RhiSamplerBinding;
using RhiSamplerDesc = yuengine::rhi::RhiSamplerDesc;
using RhiSamplerHandle = yuengine::rhi::RhiSamplerHandle;
using RhiShaderModuleDesc = yuengine::rhi::RhiShaderModuleDesc;
using RhiShaderModuleHandle = yuengine::rhi::RhiShaderModuleHandle;
using yuengine::rhi::RhiShaderStage;
using yuengine::rhi::RhiStatus;
using RhiTextureDesc = yuengine::rhi::RhiTextureDesc;
using RhiTextureHandle = yuengine::rhi::RhiTextureHandle;
using RhiVertexBufferView = yuengine::rhi::RhiVertexBufferView;

namespace {
constexpr const char *TEST_EXECUTES = "RenderCore_GraphExecutionPlan_ExecutesPreparedSkeletonBatch";
constexpr const char *TEST_FAILED_SKELETON = "RenderCore_GraphExecutionPlan_RejectsFailedSkeletonResultWithoutMutation";
constexpr const char *TEST_NULL_FRAME = "RenderCore_GraphExecutionPlan_RejectsNullFrameExecutorWithoutMutation";
constexpr const char *TEST_INVALID_FRAME = "RenderCore_GraphExecutionPlan_RejectsInvalidFrameIdWithoutMutation";
constexpr const char *TEST_INVALID_BATCH = "RenderCore_GraphExecutionPlan_RejectsInvalidPreparedBatchWithoutMutation";
constexpr const char *TEST_DUPLICATE_PLAN = "RenderCore_GraphExecutionPlan_RejectsDuplicatePlanIdWithoutMutation";
constexpr const char *TEST_DUPLICATE_GRAPH = "RenderCore_GraphExecutionPlan_RejectsDuplicateGraphExecutionWithoutMutation";
constexpr const char *TEST_CAPACITY = "RenderCore_GraphExecutionPlan_RejectsPlanCapacityOverflowWithoutMutation";
constexpr const char *TEST_FRAME_FAILURE = "RenderCore_GraphExecutionPlan_PropagatesFramePacketFailure";
constexpr const char *TEST_QUERY = "RenderCore_GraphExecutionPlan_QueryReturnsPlanRecord";
constexpr const char *TEST_RELEASE = "RenderCore_GraphExecutionPlan_ReleaseClearsPlanRecord";
constexpr const char *TEST_SNAPSHOT = "RenderCore_GraphExecutionPlan_SnapshotTracksBoundedCounters";
constexpr const char *TEST_RESET = "RenderCore_GraphExecutionPlan_ResetClearsPlanRecords";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint8_t SENTINEL_BYTE = 0xA5U;
constexpr std::uint32_t GRAPH_ID = 9001U;
constexpr std::uint32_t NEXT_GRAPH_ID = 9003U;
constexpr std::uint32_t PLAN_ID = 7001U;
constexpr std::uint32_t NEXT_PLAN_ID = 7003U;
constexpr std::uint32_t FRAME_ID = 5001U;
constexpr std::uint32_t NEXT_FRAME_ID = 5003U;
constexpr std::uint32_t FIRST_PASS_ID = 101U;
constexpr std::uint32_t SECOND_PASS_ID = 103U;
constexpr std::uint32_t FIRST_MATERIAL_ID = 201U;
constexpr std::uint32_t SECOND_MATERIAL_ID = 203U;
constexpr std::uint32_t TRIANGLE_VERTEX_COUNT = 3U;
constexpr std::uint32_t TRIANGLE_INDEX_COUNT = 3U;
constexpr std::size_t TRIANGLE_VERTEX_STRIDE_BYTES = sizeof(float) * 6U;
constexpr std::size_t TRIANGLE_VERTEX_BUFFER_BYTES = TRIANGLE_VERTEX_STRIDE_BYTES * TRIANGLE_VERTEX_COUNT;
constexpr std::size_t TRIANGLE_INDEX_BUFFER_BYTES = sizeof(std::uint16_t) * TRIANGLE_INDEX_COUNT;
constexpr std::size_t CAPTURE_BYTES = 16U;

struct PreparedGraphFixture final {
    NullRhiDevice device{};
    RenderFixturePass pass{};
    RenderGraphSkeleton graph{};
    RenderFramePacketFixture frame_packet{};
    RenderSubmissionBatchFixture submission_batch{};
    std::array<RenderGraphSkeletonPassDeclaration, 2U> declarations{};
    std::array<RenderGraphSkeletonDependencyDeclaration, 1U> dependencies{};
    std::array<RenderFixturePassRequest, 2U> prepared_requests{};
    std::array<RenderFixturePassResult, 2U> pass_results{};
    std::vector<std::uint8_t> first_capture{};
    std::vector<std::uint8_t> second_capture{};
    RenderGraphSkeletonResult graph_result{};
};

struct DependencySnapshots final {
    RenderGraphSkeletonSnapshot graph{};
    RenderFramePacketFixtureSnapshot frame{};
    RenderSubmissionBatchFixtureSnapshot batch{};
    RenderFixturePassSnapshot pass{};
    RhiDeviceSnapshot rhi{};
};

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

RhiColorTargetDesc SmallTargetDesc() {
    return RhiColorTargetDesc{RhiFormat::Rgba8Unorm, {2U, 2U}};
}

RhiBufferDesc TriangleVertexBufferDesc() {
    return RhiBufferDesc{RhiBufferUsage::Vertex, TRIANGLE_VERTEX_BUFFER_BYTES};
}

RhiBufferDesc TriangleIndexBufferDesc() {
    return RhiBufferDesc{RhiBufferUsage::Index, TRIANGLE_INDEX_BUFFER_BYTES};
}

RhiTextureDesc SmallTextureDesc() {
    return RhiTextureDesc{RhiFormat::Rgba8Unorm, {2U, 2U}};
}

RhiInputLayoutDesc TriangleInputLayoutDesc() {
    RhiInputLayoutDesc desc{};
    desc.elements[0U].semantic = RhiInputElementSemantic::Position;
    desc.elements[0U].format = RhiInputElementFormat::Float32x2;
    desc.elements[0U].offset_bytes = 0U;
    desc.elements[1U].semantic = RhiInputElementSemantic::Color;
    desc.elements[1U].format = RhiInputElementFormat::Float32x4;
    desc.elements[1U].offset_bytes = sizeof(float) * 2U;
    desc.element_count = 2U;
    desc.stride_bytes = TRIANGLE_VERTEX_STRIDE_BYTES;
    return desc;
}

RhiDrawIndexedDesc TriangleDrawIndexedDesc() {
    RhiDrawIndexedDesc desc{};
    desc.topology = RhiPrimitiveTopology::TriangleList;
    desc.index_count = TRIANGLE_INDEX_COUNT;
    desc.first_index = 0U;
    desc.vertex_offset = 0;
    return desc;
}

RhiVertexBufferView TriangleVertexBufferViewFor(RhiBufferHandle handle) {
    RhiVertexBufferView view{};
    view.buffer = handle;
    view.offset_bytes = 0U;
    view.stride_bytes = TRIANGLE_VERTEX_STRIDE_BYTES;
    view.size_bytes = TRIANGLE_VERTEX_BUFFER_BYTES;
    return view;
}

RhiIndexBufferView TriangleIndexBufferViewFor(RhiBufferHandle handle) {
    RhiIndexBufferView view{};
    view.buffer = handle;
    view.offset_bytes = 0U;
    view.size_bytes = TRIANGLE_INDEX_BUFFER_BYTES;
    view.format = RhiIndexFormat::Uint16;
    return view;
}

RhiSampledTextureBinding SampledTextureBindingFor(RhiTextureHandle texture) {
    RhiSampledTextureBinding binding{};
    binding.texture = texture;
    binding.slot = 0U;
    return binding;
}

RhiSamplerBinding SamplerBindingFor(RhiSamplerHandle sampler) {
    RhiSamplerBinding binding{};
    binding.sampler = sampler;
    binding.slot = 0U;
    return binding;
}

std::array<std::uint8_t, 4U> SmallShaderBytes() {
    return std::array<std::uint8_t, 4U>{1U, 2U, 3U, 4U};
}

bool CreateTarget(IRhiDevice &device, RhiTextureHandle &out_handle) {
    return device.CreateColorTarget(SmallTargetDesc(), out_handle) == RhiStatus::Success;
}

bool CreateShaderModule(IRhiDevice &device, RhiShaderStage stage, RhiShaderModuleHandle &out_handle) {
    const std::array<std::uint8_t, 4U> bytes = SmallShaderBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const RhiShaderModuleDesc desc{stage, byte_span};
    return device.CreateShaderModule(desc, out_handle) == RhiStatus::Success;
}

bool CreateTrianglePipeline(IRhiDevice &device, RhiPipelineHandle &out_handle) {
    RhiShaderModuleHandle vertex_shader{};
    if (!CreateShaderModule(device, RhiShaderStage::Vertex, vertex_shader)) {
        return false;
    }

    RhiShaderModuleHandle pixel_shader{};
    if (!CreateShaderModule(device, RhiShaderStage::Pixel, pixel_shader)) {
        return false;
    }

    RhiPipelineDesc desc{};
    desc.vertex_shader = vertex_shader;
    desc.pixel_shader = pixel_shader;
    desc.input_layout = TriangleInputLayoutDesc();
    return device.CreatePipeline(desc, out_handle) == RhiStatus::Success;
}

bool CreateTriangleBuffer(IRhiDevice &device, RhiBufferHandle &out_handle) {
    const std::span<const std::uint8_t> empty_bytes{};
    return device.CreateBuffer(TriangleVertexBufferDesc(), empty_bytes, out_handle) == RhiStatus::Success;
}

bool CreateTriangleIndexBuffer(IRhiDevice &device, RhiBufferHandle &out_handle) {
    const std::array<std::uint16_t, TRIANGLE_INDEX_COUNT> indices{0U, 1U, 2U};
    const auto *index_byte_pointer = reinterpret_cast<const std::uint8_t *>(indices.data());
    const std::span<const std::uint8_t> index_bytes(index_byte_pointer, TRIANGLE_INDEX_BUFFER_BYTES);
    return device.CreateBuffer(TriangleIndexBufferDesc(), index_bytes, out_handle) == RhiStatus::Success;
}

bool CreateSampledTexture(IRhiDevice &device, RhiTextureHandle &out_handle) {
    const std::array<std::uint8_t, CAPTURE_BYTES> texture_bytes{
        255U, 0U, 0U, 255U,
        0U, 255U, 0U, 255U,
        0U, 0U, 255U, 255U,
        255U, 255U, 255U, 255U};
    const std::span<const std::uint8_t> texture_span(texture_bytes.data(), texture_bytes.size());
    return device.CreateTexture(SmallTextureDesc(), texture_span, out_handle) == RhiStatus::Success;
}

bool CreateSamplerPrimitive(IRhiDevice &device, RhiSamplerHandle &out_handle) {
    RhiSamplerDesc desc{};
    desc.linear_filter = false;
    desc.clamp_to_edge = true;
    return device.CreateSampler(desc, out_handle) == RhiStatus::Success;
}

bool FillValidFixture(
    NullRhiDevice &device,
    RenderFixturePassRequest &request,
    std::vector<std::uint8_t> &capture,
    std::uint32_t pass_id,
    std::uint32_t material_id) {
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device_interface, target)) {
        return false;
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, pipeline)) {
        return false;
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, vertex_buffer)) {
        return false;
    }

    RhiBufferHandle index_buffer{};
    if (!CreateTriangleIndexBuffer(device_interface, index_buffer)) {
        return false;
    }

    RhiTextureHandle sampled_texture{};
    if (!CreateSampledTexture(device_interface, sampled_texture)) {
        return false;
    }

    RhiSamplerHandle sampler{};
    if (!CreateSamplerPrimitive(device_interface, sampler)) {
        return false;
    }

    request.rhi_device = &device_interface;
    request.target = target;
    request.pipeline = pipeline;
    request.vertex_buffer = TriangleVertexBufferViewFor(vertex_buffer);
    request.index_buffer = TriangleIndexBufferViewFor(index_buffer);
    request.sampled_texture = SampledTextureBindingFor(sampled_texture);
    request.sampler = SamplerBindingFor(sampler);
    request.draw = TriangleDrawIndexedDesc();
    request.clear_color = RhiColor{7U, 11U, 13U, 255U};
    request.capture_output = std::span<std::uint8_t>(capture.data(), capture.size());
    request.capture_byte_budget = capture.size();
    request.pass_id = pass_id;
    request.material_id = material_id;
    return true;
}

bool FillTwoPassDeclarations(PreparedGraphFixture &fixture) {
    fixture.declarations[0U].pass_id = FIRST_PASS_ID;
    if (!FillValidFixture(
        fixture.device,
        fixture.declarations[0U].pass_request,
        fixture.first_capture,
        FIRST_PASS_ID,
        FIRST_MATERIAL_ID)) {
        return false;
    }

    fixture.declarations[1U].pass_id = SECOND_PASS_ID;
    return FillValidFixture(
        fixture.device,
        fixture.declarations[1U].pass_request,
        fixture.second_capture,
        SECOND_PASS_ID,
        SECOND_MATERIAL_ID);
}

RenderGraphSkeletonRequest GraphRequestFrom(PreparedGraphFixture &fixture, std::uint32_t graph_id) {
    RenderGraphSkeletonRequest request{};
    request.graph_id = graph_id;
    request.pass = &fixture.pass;
    request.pass_declarations = std::span<const RenderGraphSkeletonPassDeclaration>(
        fixture.declarations.data(),
        fixture.declarations.size());
    request.dependency_declarations = std::span<const RenderGraphSkeletonDependencyDeclaration>(
        fixture.dependencies.data(),
        fixture.dependencies.size());
    request.prepared_pass_requests = std::span<RenderFixturePassRequest>(
        fixture.prepared_requests.data(),
        fixture.prepared_requests.size());
    request.pass_results = std::span<RenderFixturePassResult>(
        fixture.pass_results.data(),
        fixture.pass_results.size());
    return request;
}

bool PrepareFixture(PreparedGraphFixture &fixture, std::uint32_t graph_id=GRAPH_ID) {
    if (fixture.device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return false;
    }

    fixture.first_capture = std::vector<std::uint8_t>(CAPTURE_BYTES, SENTINEL_BYTE);
    fixture.second_capture = std::vector<std::uint8_t>(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillTwoPassDeclarations(fixture)) {
        return false;
    }

    fixture.dependencies[0U] = RenderGraphSkeletonDependencyDeclaration{FIRST_PASS_ID, SECOND_PASS_ID};
    const RenderGraphSkeletonRequest request = GraphRequestFrom(fixture, graph_id);
    fixture.graph_result = fixture.graph.Prepare(request);
    return fixture.graph_result.status == RenderGraphSkeletonStatus::Success;
}

RenderGraphExecutionPlanRequest ExecutionRequestFrom(
    PreparedGraphFixture &fixture,
    std::uint32_t plan_id=PLAN_ID,
    std::uint32_t frame_id=FRAME_ID) {
    RenderGraphExecutionPlanRequest request{};
    request.plan_id = plan_id;
    request.frame_id = frame_id;
    request.prepared_graph_result = &fixture.graph_result;
    request.frame_packet = &fixture.frame_packet;
    request.submission_batch = &fixture.submission_batch;
    return request;
}

DependencySnapshots CaptureDependencySnapshots(const PreparedGraphFixture &fixture) {
    DependencySnapshots snapshots{};
    snapshots.graph = fixture.graph.Snapshot();
    snapshots.frame = fixture.frame_packet.Snapshot();
    snapshots.batch = fixture.submission_batch.Snapshot();
    snapshots.pass = fixture.pass.Snapshot();
    snapshots.rhi = fixture.device.Snapshot();
    return snapshots;
}

bool WorkCountersMatch(const RhiDeviceSnapshot &left, const RhiDeviceSnapshot &right) {
    if (left.recorded_command_count != right.recorded_command_count) {
        return false;
    }

    if (left.submit_count != right.submit_count) {
        return false;
    }

    if (left.present_count != right.present_count) {
        return false;
    }

    if (left.capture_count != right.capture_count) {
        return false;
    }

    return left.failed_operation_count == right.failed_operation_count;
}

bool DependencySnapshotsMatch(const DependencySnapshots &left, const DependencySnapshots &right) {
    if (left.graph.graph_record_count != right.graph.graph_record_count) {
        return false;
    }

    if (left.frame.frame_packet_record_count != right.frame.frame_packet_record_count) {
        return false;
    }

    if (left.frame.failed_validation_count != right.frame.failed_validation_count) {
        return false;
    }

    if (left.batch.submission_record_count != right.batch.submission_record_count) {
        return false;
    }

    if (left.batch.failed_validation_count != right.batch.failed_validation_count) {
        return false;
    }

    if (left.pass.pass_record_count != right.pass.pass_record_count) {
        return false;
    }

    if (left.pass.failed_validation_count != right.pass.failed_validation_count) {
        return false;
    }

    return WorkCountersMatch(left.rhi, right.rhi);
}

int ExpectPlanValidationFailure(
    RenderGraphExecutionPlanStatus expected_status,
    RenderGraphExecutionPlan &plan,
    PreparedGraphFixture &fixture,
    const RenderGraphExecutionPlanRequest &request) {
    const DependencySnapshots before = CaptureDependencySnapshots(fixture);
    const std::size_t before_plan_record_count = plan.Snapshot().plan_record_count;
    const auto result = plan.Execute(request);
    if (result.status != expected_status) {
        return Fail("execution plan returned unexpected validation status");
    }

    const DependencySnapshots after = CaptureDependencySnapshots(fixture);
    if (!DependencySnapshotsMatch(before, after)) {
        return Fail("execution plan validation failure mutated dependency state");
    }

    if (plan.Snapshot().plan_record_count != before_plan_record_count) {
        return Fail("execution plan validation failure changed plan records");
    }

    return 0;
}

int RenderCoreGraphExecutionPlanExecutesPreparedSkeletonBatch() {
    PreparedGraphFixture fixture;
    if (!PrepareFixture(fixture)) {
        return Fail("execution plan setup failed");
    }

    RenderGraphExecutionPlan plan;
    const auto result = plan.Execute(ExecutionRequestFrom(fixture));
    if (result.status != RenderGraphExecutionPlanStatus::Success) {
        return Fail("execution plan did not execute prepared graph");
    }

    if (result.frame_status != RenderFramePacketFixtureStatus::Success) {
        return Fail("execution plan reported wrong frame status");
    }

    if (result.batch_status != RenderSubmissionBatchFixtureStatus::Success) {
        return Fail("execution plan reported wrong batch status");
    }

    if (result.pass_status != RenderFixturePassStatus::Success || result.rhi_status != RhiStatus::Success) {
        return Fail("execution plan reported wrong pass or rhi status");
    }

    const auto snapshot = plan.Snapshot();
    if (snapshot.plan_record_count != 1U || snapshot.completed_plan_count != 1U) {
        return Fail("execution plan did not record completed plan");
    }

    if (fixture.frame_packet.Snapshot().completed_packet_count != 1U) {
        return Fail("execution plan did not hand off to frame packet");
    }

    return 0;
}

int RenderCoreGraphExecutionPlanRejectsFailedSkeletonResultWithoutMutation() {
    PreparedGraphFixture fixture;
    if (!PrepareFixture(fixture)) {
        return Fail("failed skeleton setup failed");
    }

    RenderGraphSkeletonResult failed_result{};
    failed_result.status = RenderGraphSkeletonStatus::InvalidGraphId;
    failed_result.graph_id = GRAPH_ID;
    failed_result.pass_count = fixture.graph_result.pass_count;
    RenderGraphExecutionPlanRequest request = ExecutionRequestFrom(fixture);
    request.prepared_graph_result = &failed_result;

    RenderGraphExecutionPlan plan;
    return ExpectPlanValidationFailure(RenderGraphExecutionPlanStatus::FailedSkeletonPrepare, plan, fixture, request);
}

int RenderCoreGraphExecutionPlanRejectsNullFrameExecutorWithoutMutation() {
    PreparedGraphFixture fixture;
    if (!PrepareFixture(fixture)) {
        return Fail("null frame setup failed");
    }

    RenderGraphExecutionPlanRequest request = ExecutionRequestFrom(fixture);
    request.frame_packet = nullptr;

    RenderGraphExecutionPlan plan;
    return ExpectPlanValidationFailure(RenderGraphExecutionPlanStatus::InvalidFrameExecutor, plan, fixture, request);
}

int RenderCoreGraphExecutionPlanRejectsInvalidFrameIdWithoutMutation() {
    PreparedGraphFixture fixture;
    if (!PrepareFixture(fixture)) {
        return Fail("invalid frame setup failed");
    }

    RenderGraphExecutionPlanRequest request = ExecutionRequestFrom(fixture);
    request.frame_id = 0U;

    RenderGraphExecutionPlan plan;
    return ExpectPlanValidationFailure(RenderGraphExecutionPlanStatus::InvalidFrameId, plan, fixture, request);
}

int RenderCoreGraphExecutionPlanRejectsInvalidPreparedBatchWithoutMutation() {
    PreparedGraphFixture fixture;
    if (!PrepareFixture(fixture)) {
        return Fail("invalid prepared batch setup failed");
    }

    RenderGraphSkeletonResult invalid_result = fixture.graph_result;
    invalid_result.submission_batch_request.pass_results = std::span<RenderFixturePassResult>();

    RenderGraphExecutionPlanRequest request = ExecutionRequestFrom(fixture);
    request.prepared_graph_result = &invalid_result;

    RenderGraphExecutionPlan plan;
    return ExpectPlanValidationFailure(RenderGraphExecutionPlanStatus::MissingPassResultStorage, plan, fixture, request);
}

int RenderCoreGraphExecutionPlanRejectsDuplicatePlanIdWithoutMutation() {
    PreparedGraphFixture first_fixture;
    if (!PrepareFixture(first_fixture, GRAPH_ID)) {
        return Fail("first duplicate plan setup failed");
    }

    PreparedGraphFixture second_fixture;
    if (!PrepareFixture(second_fixture, NEXT_GRAPH_ID)) {
        return Fail("second duplicate plan setup failed");
    }

    RenderGraphExecutionPlan plan;
    if (plan.Execute(ExecutionRequestFrom(first_fixture, PLAN_ID, FRAME_ID)).status != RenderGraphExecutionPlanStatus::Success) {
        return Fail("first duplicate plan execution failed");
    }

    return ExpectPlanValidationFailure(
        RenderGraphExecutionPlanStatus::DuplicatePlanId,
        plan,
        second_fixture,
        ExecutionRequestFrom(second_fixture, PLAN_ID, NEXT_FRAME_ID));
}

int RenderCoreGraphExecutionPlanRejectsDuplicateGraphExecutionWithoutMutation() {
    PreparedGraphFixture fixture;
    if (!PrepareFixture(fixture)) {
        return Fail("duplicate graph setup failed");
    }

    RenderGraphExecutionPlan plan;
    if (plan.Execute(ExecutionRequestFrom(fixture, PLAN_ID, FRAME_ID)).status != RenderGraphExecutionPlanStatus::Success) {
        return Fail("first duplicate graph execution failed");
    }

    const DependencySnapshots before = CaptureDependencySnapshots(fixture);
    const auto result = plan.Execute(ExecutionRequestFrom(fixture, NEXT_PLAN_ID, NEXT_FRAME_ID));
    if (result.status != RenderGraphExecutionPlanStatus::DuplicateGraphExecution) {
        return Fail("execution plan accepted duplicate graph execution");
    }

    const DependencySnapshots after = CaptureDependencySnapshots(fixture);
    if (!DependencySnapshotsMatch(before, after)) {
        return Fail("duplicate graph execution mutated dependency state");
    }

    return 0;
}

int RenderCoreGraphExecutionPlanRejectsPlanCapacityOverflowWithoutMutation() {
    PreparedGraphFixture first_fixture;
    if (!PrepareFixture(first_fixture, GRAPH_ID)) {
        return Fail("first capacity setup failed");
    }

    PreparedGraphFixture second_fixture;
    if (!PrepareFixture(second_fixture, NEXT_GRAPH_ID)) {
        return Fail("second capacity setup failed");
    }

    RenderGraphExecutionPlanDesc desc{};
    desc.plan_record_capacity = 1U;
    RenderGraphExecutionPlan plan(desc);
    if (plan.Execute(ExecutionRequestFrom(first_fixture, PLAN_ID, FRAME_ID)).status != RenderGraphExecutionPlanStatus::Success) {
        return Fail("first capacity execution failed");
    }

    return ExpectPlanValidationFailure(
        RenderGraphExecutionPlanStatus::PlanCapacityExceeded,
        plan,
        second_fixture,
        ExecutionRequestFrom(second_fixture, NEXT_PLAN_ID, NEXT_FRAME_ID));
}

int RenderCoreGraphExecutionPlanPropagatesFramePacketFailure() {
    PreparedGraphFixture fixture;
    if (!PrepareFixture(fixture)) {
        return Fail("frame failure setup failed");
    }

    fixture.prepared_requests[0U].target = RhiTextureHandle{999U, 1U};
    RenderGraphExecutionPlan plan;
    const auto result = plan.Execute(ExecutionRequestFrom(fixture));
    if (result.status != RenderGraphExecutionPlanStatus::FrameExecutionFailed) {
        return Fail("execution plan did not propagate frame failure");
    }

    if (result.frame_status != RenderFramePacketFixtureStatus::SubmissionBatchFailed) {
        return Fail("execution plan reported wrong frame failure status");
    }

    if (result.batch_status != RenderSubmissionBatchFixtureStatus::RenderFixturePassFailed) {
        return Fail("execution plan reported wrong batch failure status");
    }

    if (result.pass_status != RenderFixturePassStatus::RhiFailure || result.rhi_status != RhiStatus::InvalidHandle) {
        return Fail("execution plan reported wrong pass failure status");
    }

    const auto snapshot = plan.Snapshot();
    if (snapshot.frame_failed_plan_count != 1U || snapshot.plan_record_count != 1U) {
        return Fail("execution plan did not record frame failure");
    }

    return 0;
}

int RenderCoreGraphExecutionPlanQueryReturnsPlanRecord() {
    PreparedGraphFixture fixture;
    if (!PrepareFixture(fixture)) {
        return Fail("query setup failed");
    }

    RenderGraphExecutionPlan plan;
    if (plan.Execute(ExecutionRequestFrom(fixture)).status != RenderGraphExecutionPlanStatus::Success) {
        return Fail("query execution failed");
    }

    std::array<RenderGraphExecutionPlanRecord, 2U> records{};
    const std::size_t copied_count = plan.QueryRecords(
        std::span<RenderGraphExecutionPlanRecord>(records.data(), records.size()));
    if (copied_count != 1U) {
        return Fail("execution plan query returned wrong record count");
    }

    if (records[0U].plan_id != PLAN_ID || records[0U].graph_id != GRAPH_ID || records[0U].frame_id != FRAME_ID) {
        return Fail("execution plan query returned wrong record ids");
    }

    if (records[0U].status != RenderGraphExecutionPlanStatus::Success) {
        return Fail("execution plan query returned wrong status");
    }

    if (plan.Snapshot().query_count != 1U) {
        return Fail("execution plan query counter was not updated");
    }

    return 0;
}

int RenderCoreGraphExecutionPlanReleaseClearsPlanRecord() {
    PreparedGraphFixture fixture;
    if (!PrepareFixture(fixture)) {
        return Fail("release setup failed");
    }

    RenderGraphExecutionPlan plan;
    if (plan.Execute(ExecutionRequestFrom(fixture)).status != RenderGraphExecutionPlanStatus::Success) {
        return Fail("release execution failed");
    }

    if (plan.Release(PLAN_ID) != RenderGraphExecutionPlanStatus::Success) {
        return Fail("execution plan release failed");
    }

    std::array<RenderGraphExecutionPlanRecord, 1U> records{};
    if (plan.QueryRecords(std::span<RenderGraphExecutionPlanRecord>(records.data(), records.size())) != 0U) {
        return Fail("execution plan query returned released record");
    }

    const auto snapshot = plan.Snapshot();
    if (snapshot.plan_record_count != 0U || snapshot.released_plan_count != 1U) {
        return Fail("execution plan release counters were not updated");
    }

    return 0;
}

int RenderCoreGraphExecutionPlanSnapshotTracksBoundedCounters() {
    PreparedGraphFixture first_fixture;
    if (!PrepareFixture(first_fixture, GRAPH_ID)) {
        return Fail("first snapshot setup failed");
    }

    PreparedGraphFixture second_fixture;
    if (!PrepareFixture(second_fixture, NEXT_GRAPH_ID)) {
        return Fail("second snapshot setup failed");
    }

    RenderGraphExecutionPlan plan;
    if (plan.Execute(ExecutionRequestFrom(first_fixture, PLAN_ID, FRAME_ID)).status != RenderGraphExecutionPlanStatus::Success) {
        return Fail("snapshot first execution failed");
    }

    const auto duplicate_result = plan.Execute(ExecutionRequestFrom(second_fixture, PLAN_ID, NEXT_FRAME_ID));
    if (duplicate_result.status != RenderGraphExecutionPlanStatus::DuplicatePlanId) {
        return Fail("snapshot duplicate plan was not rejected");
    }

    std::array<RenderGraphExecutionPlanRecord, 2U> records{};
    if (plan.QueryRecords(std::span<RenderGraphExecutionPlanRecord>(records.data(), records.size())) != 1U) {
        return Fail("snapshot query returned wrong count");
    }

    if (plan.Release(PLAN_ID) != RenderGraphExecutionPlanStatus::Success) {
        return Fail("snapshot release failed");
    }

    const auto snapshot = plan.Snapshot();
    if (snapshot.completed_plan_count != 1U || snapshot.duplicate_plan_id_count != 1U) {
        return Fail("execution plan snapshot missed execution counters");
    }

    if (snapshot.query_count != 1U || snapshot.released_plan_count != 1U) {
        return Fail("execution plan snapshot missed query or release counters");
    }

    if (snapshot.last_operation != RenderGraphExecutionPlanOperation::Release) {
        return Fail("execution plan snapshot missed last operation");
    }

    return 0;
}

int RenderCoreGraphExecutionPlanResetClearsPlanRecords() {
    PreparedGraphFixture fixture;
    if (!PrepareFixture(fixture)) {
        return Fail("reset setup failed");
    }

    RenderGraphExecutionPlan plan;
    if (plan.Execute(ExecutionRequestFrom(fixture)).status != RenderGraphExecutionPlanStatus::Success) {
        return Fail("reset execution failed");
    }

    plan.Reset();
    const auto snapshot = plan.Snapshot();
    if (snapshot.plan_record_count != 0U || snapshot.completed_plan_count != 0U) {
        return Fail("execution plan reset did not clear counters");
    }

    if (snapshot.reset_count != 1U || snapshot.last_operation != RenderGraphExecutionPlanOperation::Reset) {
        return Fail("execution plan reset snapshot was not updated");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_EXECUTES) {
        return RenderCoreGraphExecutionPlanExecutesPreparedSkeletonBatch();
    }

    if (name == TEST_FAILED_SKELETON) {
        return RenderCoreGraphExecutionPlanRejectsFailedSkeletonResultWithoutMutation();
    }

    if (name == TEST_NULL_FRAME) {
        return RenderCoreGraphExecutionPlanRejectsNullFrameExecutorWithoutMutation();
    }

    if (name == TEST_INVALID_FRAME) {
        return RenderCoreGraphExecutionPlanRejectsInvalidFrameIdWithoutMutation();
    }

    if (name == TEST_INVALID_BATCH) {
        return RenderCoreGraphExecutionPlanRejectsInvalidPreparedBatchWithoutMutation();
    }

    if (name == TEST_DUPLICATE_PLAN) {
        return RenderCoreGraphExecutionPlanRejectsDuplicatePlanIdWithoutMutation();
    }

    if (name == TEST_DUPLICATE_GRAPH) {
        return RenderCoreGraphExecutionPlanRejectsDuplicateGraphExecutionWithoutMutation();
    }

    if (name == TEST_CAPACITY) {
        return RenderCoreGraphExecutionPlanRejectsPlanCapacityOverflowWithoutMutation();
    }

    if (name == TEST_FRAME_FAILURE) {
        return RenderCoreGraphExecutionPlanPropagatesFramePacketFailure();
    }

    if (name == TEST_QUERY) {
        return RenderCoreGraphExecutionPlanQueryReturnsPlanRecord();
    }

    if (name == TEST_RELEASE) {
        return RenderCoreGraphExecutionPlanReleaseClearsPlanRecord();
    }

    if (name == TEST_SNAPSHOT) {
        return RenderCoreGraphExecutionPlanSnapshotTracksBoundedCounters();
    }

    if (name == TEST_RESET) {
        return RenderCoreGraphExecutionPlanResetClearsPlanRecords();
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
