// Module: Tests RenderCore
// File: Tests/RenderCore/RenderGraphSkeletonTests.cpp

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
#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/RenderCore/RenderGraphSkeleton.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonDependencyDeclaration.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonDesc.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonPassDeclaration.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonRecord.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonRequest.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonStatus.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixture.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureStatus.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/NullRhiDevice.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiBufferUsage.h"
#include "YuEngine/Rhi/RhiColorTargetDesc.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
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
using yuengine::rendercore::RenderFixturePassStatus;
using RenderGraphSkeleton = yuengine::rendercore::RenderGraphSkeleton;
using RenderGraphSkeletonDependencyDeclaration = yuengine::rendercore::RenderGraphSkeletonDependencyDeclaration;
using RenderGraphSkeletonDesc = yuengine::rendercore::RenderGraphSkeletonDesc;
using RenderGraphSkeletonPassDeclaration = yuengine::rendercore::RenderGraphSkeletonPassDeclaration;
using RenderGraphSkeletonRecord = yuengine::rendercore::RenderGraphSkeletonRecord;
using RenderGraphSkeletonRequest = yuengine::rendercore::RenderGraphSkeletonRequest;
using yuengine::rendercore::RenderGraphSkeletonStatus;
using RenderSubmissionBatchFixture = yuengine::rendercore::RenderSubmissionBatchFixture;
using yuengine::rendercore::RenderSubmissionBatchFixtureStatus;
using IRhiDevice = yuengine::rhi::IRhiDevice;
using NullRhiDevice = yuengine::rhi::NullRhiDevice;
using RhiBufferDesc = yuengine::rhi::RhiBufferDesc;
using RhiBufferHandle = yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiBufferUsage;
using RhiColor = yuengine::rhi::RhiColor;
using RhiColorTargetDesc = yuengine::rhi::RhiColorTargetDesc;
using RhiDeviceDesc = yuengine::rhi::RhiDeviceDesc;
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
constexpr const char *TEST_PREPARES_BATCH = "RenderCore_GraphSkeleton_PreparesSubmissionBatchInDeclarationOrder";
constexpr const char *TEST_QUERY = "RenderCore_GraphSkeleton_QueryReturnsGraphRecord";
constexpr const char *TEST_RELEASE = "RenderCore_GraphSkeleton_ReleaseClearsGraphRecord";
constexpr const char *TEST_ZERO_GRAPH_ID = "RenderCore_GraphSkeleton_RejectsZeroGraphIdWithoutMutation";
constexpr const char *TEST_EMPTY_GRAPH = "RenderCore_GraphSkeleton_RejectsEmptyGraphWithoutMutation";
constexpr const char *TEST_DUPLICATE_GRAPH = "RenderCore_GraphSkeleton_RejectsDuplicateGraphIdWithoutMutation";
constexpr const char *TEST_DUPLICATE_PASS = "RenderCore_GraphSkeleton_RejectsDuplicatePassIdWithoutMutation";
constexpr const char *TEST_MISSING_DEPENDENCY = "RenderCore_GraphSkeleton_RejectsMissingDependencyWithoutMutation";
constexpr const char *TEST_SELF_DEPENDENCY = "RenderCore_GraphSkeleton_RejectsSelfDependencyWithoutMutation";
constexpr const char *TEST_DEPENDENCY_CYCLE = "RenderCore_GraphSkeleton_RejectsDependencyCycleWithoutMutation";
constexpr const char *TEST_PASS_CAPACITY = "RenderCore_GraphSkeleton_RejectsPassCapacityOverflowWithoutMutation";
constexpr const char *TEST_DEPENDENCY_CAPACITY = "RenderCore_GraphSkeleton_RejectsDependencyCapacityOverflowWithoutMutation";
constexpr const char *TEST_INVALID_PASS = "RenderCore_GraphSkeleton_RejectsInvalidPassRequestWithoutMutation";
constexpr const char *TEST_SNAPSHOT = "RenderCore_GraphSkeleton_SnapshotTracksBoundedCounters";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint8_t SENTINEL_BYTE = 0xA5U;
constexpr std::uint32_t GRAPH_ID = 9001U;
constexpr std::uint32_t NEXT_GRAPH_ID = 9003U;
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
constexpr std::uint32_t SENTINEL_PASS_ID = 0xEEEEU;

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

NullRhiDevice CreateInitializedDevice() {
    NullRhiDevice device;
    device.Initialize(RhiDeviceDesc{});
    return device;
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

bool FillTwoPassDeclarations(
    NullRhiDevice &device,
    std::array<RenderGraphSkeletonPassDeclaration, 2U> &declarations,
    std::vector<std::uint8_t> &first_capture,
    std::vector<std::uint8_t> &second_capture) {
    declarations[0U].pass_id = FIRST_PASS_ID;
    if (!FillValidFixture(device, declarations[0U].pass_request, first_capture, FIRST_PASS_ID, FIRST_MATERIAL_ID)) {
        return false;
    }

    declarations[1U].pass_id = SECOND_PASS_ID;
    return FillValidFixture(device, declarations[1U].pass_request, second_capture, SECOND_PASS_ID, SECOND_MATERIAL_ID);
}

RenderGraphSkeletonRequest GraphRequestFrom(
    std::uint32_t graph_id,
    RenderFixturePass &pass,
    std::span<const RenderGraphSkeletonPassDeclaration> declarations,
    std::span<const RenderGraphSkeletonDependencyDeclaration> dependencies,
    std::span<RenderFixturePassRequest> prepared_requests,
    std::span<RenderFixturePassResult> pass_results) {
    RenderGraphSkeletonRequest request{};
    request.graph_id = graph_id;
    request.pass = &pass;
    request.pass_declarations = declarations;
    request.dependency_declarations = dependencies;
    request.prepared_pass_requests = prepared_requests;
    request.pass_results = pass_results;
    return request;
}

void FillPreparedSentinel(std::span<RenderFixturePassRequest> requests) {
    for (RenderFixturePassRequest &request : requests) {
        request.pass_id = SENTINEL_PASS_ID;
    }
}

bool PreparedSentinelUnchanged(std::span<const RenderFixturePassRequest> requests) {
    for (const RenderFixturePassRequest &request : requests) {
        if (request.pass_id != SENTINEL_PASS_ID) {
            return false;
        }
    }

    return true;
}

int PrepareAcceptedGraph(RenderGraphSkeleton &graph) {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePass pass;
    std::array<RenderGraphSkeletonPassDeclaration, 2U> declarations{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillTwoPassDeclarations(device, declarations, first_capture, second_capture)) {
        return Fail("accepted graph setup failed");
    }

    std::array<RenderGraphSkeletonDependencyDeclaration, 1U> dependencies{};
    dependencies[0U] = RenderGraphSkeletonDependencyDeclaration{FIRST_PASS_ID, SECOND_PASS_ID};
    std::array<RenderFixturePassRequest, 2U> prepared_requests{};
    std::array<RenderFixturePassResult, 2U> pass_results{};
    const RenderGraphSkeletonRequest request = GraphRequestFrom(
        GRAPH_ID,
        pass,
        std::span<const RenderGraphSkeletonPassDeclaration>(declarations.data(), declarations.size()),
        std::span<const RenderGraphSkeletonDependencyDeclaration>(dependencies.data(), dependencies.size()),
        std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()),
        std::span<RenderFixturePassResult>(pass_results.data(), pass_results.size()));
    const auto result = graph.Prepare(request);
    if (result.status != RenderGraphSkeletonStatus::Success) {
        return Fail("accepted graph prepare failed");
    }

    return 0;
}

int RenderCoreGraphSkeletonPreparesSubmissionBatchInDeclarationOrder() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePass pass;
    std::array<RenderGraphSkeletonPassDeclaration, 2U> declarations{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillTwoPassDeclarations(device, declarations, first_capture, second_capture)) {
        return Fail("graph skeleton setup failed");
    }

    std::array<RenderGraphSkeletonDependencyDeclaration, 1U> dependencies{};
    dependencies[0U] = RenderGraphSkeletonDependencyDeclaration{FIRST_PASS_ID, SECOND_PASS_ID};
    std::array<RenderFixturePassRequest, 2U> prepared_requests{};
    std::array<RenderFixturePassResult, 2U> pass_results{};
    RenderGraphSkeleton graph;
    const RenderGraphSkeletonRequest request = GraphRequestFrom(
        GRAPH_ID,
        pass,
        std::span<const RenderGraphSkeletonPassDeclaration>(declarations.data(), declarations.size()),
        std::span<const RenderGraphSkeletonDependencyDeclaration>(dependencies.data(), dependencies.size()),
        std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()),
        std::span<RenderFixturePassResult>(pass_results.data(), pass_results.size()));
    const auto result = graph.Prepare(request);
    if (result.status != RenderGraphSkeletonStatus::Success) {
        return Fail("graph skeleton did not prepare valid graph");
    }

    if (result.submission_batch_request.pass != &pass) {
        return Fail("graph skeleton emitted wrong pass executor");
    }

    if (result.submission_batch_request.pass_requests.size() != 2U) {
        return Fail("graph skeleton emitted wrong pass request count");
    }

    if (result.submission_batch_request.pass_requests[0U].pass_id != FIRST_PASS_ID) {
        return Fail("graph skeleton changed first pass order");
    }

    if (result.submission_batch_request.pass_requests[1U].pass_id != SECOND_PASS_ID) {
        return Fail("graph skeleton changed second pass order");
    }

    RenderSubmissionBatchFixture batch;
    const auto batch_result = batch.Execute(result.submission_batch_request);
    if (batch_result.status != RenderSubmissionBatchFixtureStatus::Success) {
        return Fail("graph skeleton emitted batch request did not execute");
    }

    const auto snapshot = graph.Snapshot();
    if (snapshot.prepared_graph_count != 1U || snapshot.graph_record_count != 1U) {
        return Fail("graph skeleton did not record prepared graph");
    }

    return 0;
}

int RenderCoreGraphSkeletonQueryReturnsGraphRecord() {
    RenderGraphSkeleton graph;
    const int prepare_result = PrepareAcceptedGraph(graph);
    if (prepare_result != 0) {
        return prepare_result;
    }

    std::array<RenderGraphSkeletonRecord, 2U> records{};
    const std::size_t copied_count = graph.QueryRecords(std::span<RenderGraphSkeletonRecord>(records.data(), records.size()));
    if (copied_count != 1U) {
        return Fail("graph skeleton query returned wrong record count");
    }

    if (records[0U].graph_id != GRAPH_ID) {
        return Fail("graph skeleton query returned wrong graph id");
    }

    if (records[0U].pass_count != 2U || records[0U].dependency_count != 1U) {
        return Fail("graph skeleton query returned wrong counts");
    }

    if (records[0U].first_pass_id != FIRST_PASS_ID || records[0U].last_pass_id != SECOND_PASS_ID) {
        return Fail("graph skeleton query returned wrong pass ids");
    }

    return 0;
}

int RenderCoreGraphSkeletonReleaseClearsGraphRecord() {
    RenderGraphSkeleton graph;
    const int prepare_result = PrepareAcceptedGraph(graph);
    if (prepare_result != 0) {
        return prepare_result;
    }

    if (graph.Release(GRAPH_ID) != RenderGraphSkeletonStatus::Success) {
        return Fail("graph skeleton release failed");
    }

    const auto snapshot = graph.Snapshot();
    if (snapshot.graph_record_count != 0U || snapshot.released_graph_count != 1U) {
        return Fail("graph skeleton release did not clear record");
    }

    std::array<RenderGraphSkeletonRecord, 1U> records{};
    if (graph.QueryRecords(std::span<RenderGraphSkeletonRecord>(records.data(), records.size())) != 0U) {
        return Fail("graph skeleton query returned released record");
    }

    return 0;
}

int RenderCoreGraphSkeletonRejectsZeroGraphIdWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePass pass;
    std::array<RenderGraphSkeletonPassDeclaration, 2U> declarations{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillTwoPassDeclarations(device, declarations, first_capture, second_capture)) {
        return Fail("zero graph setup failed");
    }

    std::array<RenderFixturePassRequest, 2U> prepared_requests{};
    FillPreparedSentinel(std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()));
    std::array<RenderFixturePassResult, 2U> pass_results{};
    RenderGraphSkeleton graph;
    const std::span<const RenderGraphSkeletonDependencyDeclaration> empty_dependencies{};
    const RenderGraphSkeletonRequest request = GraphRequestFrom(
        0U,
        pass,
        std::span<const RenderGraphSkeletonPassDeclaration>(declarations.data(), declarations.size()),
        empty_dependencies,
        std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()),
        std::span<RenderFixturePassResult>(pass_results.data(), pass_results.size()));
    const auto result = graph.Prepare(request);
    if (result.status != RenderGraphSkeletonStatus::InvalidGraphId) {
        return Fail("graph skeleton accepted zero graph id");
    }

    if (!PreparedSentinelUnchanged(std::span<const RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()))) {
        return Fail("zero graph id mutated prepared requests");
    }

    if (graph.Snapshot().graph_record_count != 0U) {
        return Fail("zero graph id mutated graph records");
    }

    return 0;
}

int RenderCoreGraphSkeletonRejectsEmptyGraphWithoutMutation() {
    RenderFixturePass pass;
    std::array<RenderFixturePassRequest, 1U> prepared_requests{};
    FillPreparedSentinel(std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()));
    std::array<RenderFixturePassResult, 1U> pass_results{};
    RenderGraphSkeleton graph;
    const std::span<const RenderGraphSkeletonPassDeclaration> empty_declarations{};
    const std::span<const RenderGraphSkeletonDependencyDeclaration> empty_dependencies{};
    const RenderGraphSkeletonRequest request = GraphRequestFrom(
        GRAPH_ID,
        pass,
        empty_declarations,
        empty_dependencies,
        std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()),
        std::span<RenderFixturePassResult>(pass_results.data(), pass_results.size()));
    const auto result = graph.Prepare(request);
    if (result.status != RenderGraphSkeletonStatus::EmptyGraph) {
        return Fail("graph skeleton accepted empty graph");
    }

    if (!PreparedSentinelUnchanged(std::span<const RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()))) {
        return Fail("empty graph mutated prepared requests");
    }

    return 0;
}

int RenderCoreGraphSkeletonRejectsDuplicateGraphIdWithoutMutation() {
    RenderGraphSkeleton graph;
    const int prepare_result = PrepareAcceptedGraph(graph);
    if (prepare_result != 0) {
        return prepare_result;
    }

    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePass pass;
    std::array<RenderGraphSkeletonPassDeclaration, 2U> declarations{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillTwoPassDeclarations(device, declarations, first_capture, second_capture)) {
        return Fail("duplicate graph setup failed");
    }

    std::array<RenderFixturePassRequest, 2U> prepared_requests{};
    FillPreparedSentinel(std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()));
    std::array<RenderFixturePassResult, 2U> pass_results{};
    const std::span<const RenderGraphSkeletonDependencyDeclaration> empty_dependencies{};
    const RenderGraphSkeletonRequest request = GraphRequestFrom(
        GRAPH_ID,
        pass,
        std::span<const RenderGraphSkeletonPassDeclaration>(declarations.data(), declarations.size()),
        empty_dependencies,
        std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()),
        std::span<RenderFixturePassResult>(pass_results.data(), pass_results.size()));
    const auto result = graph.Prepare(request);
    if (result.status != RenderGraphSkeletonStatus::DuplicateGraphId) {
        return Fail("graph skeleton accepted duplicate graph id");
    }

    if (!PreparedSentinelUnchanged(std::span<const RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()))) {
        return Fail("duplicate graph id mutated prepared requests");
    }

    if (graph.Snapshot().graph_record_count != 1U) {
        return Fail("duplicate graph id mutated graph records");
    }

    return 0;
}

int RenderCoreGraphSkeletonRejectsDuplicatePassIdWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePass pass;
    std::array<RenderGraphSkeletonPassDeclaration, 2U> declarations{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillTwoPassDeclarations(device, declarations, first_capture, second_capture)) {
        return Fail("duplicate pass setup failed");
    }

    declarations[1U].pass_id = FIRST_PASS_ID;
    declarations[1U].pass_request.pass_id = FIRST_PASS_ID;
    std::array<RenderFixturePassRequest, 2U> prepared_requests{};
    FillPreparedSentinel(std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()));
    std::array<RenderFixturePassResult, 2U> pass_results{};
    RenderGraphSkeleton graph;
    const std::span<const RenderGraphSkeletonDependencyDeclaration> empty_dependencies{};
    const RenderGraphSkeletonRequest request = GraphRequestFrom(
        GRAPH_ID,
        pass,
        std::span<const RenderGraphSkeletonPassDeclaration>(declarations.data(), declarations.size()),
        empty_dependencies,
        std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()),
        std::span<RenderFixturePassResult>(pass_results.data(), pass_results.size()));
    const auto result = graph.Prepare(request);
    if (result.status != RenderGraphSkeletonStatus::DuplicatePassId) {
        return Fail("graph skeleton accepted duplicate pass id");
    }

    if (!PreparedSentinelUnchanged(std::span<const RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()))) {
        return Fail("duplicate pass id mutated prepared requests");
    }

    return 0;
}

int RenderCoreGraphSkeletonRejectsMissingDependencyWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePass pass;
    std::array<RenderGraphSkeletonPassDeclaration, 2U> declarations{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillTwoPassDeclarations(device, declarations, first_capture, second_capture)) {
        return Fail("missing dependency setup failed");
    }

    std::array<RenderGraphSkeletonDependencyDeclaration, 1U> dependencies{};
    dependencies[0U] = RenderGraphSkeletonDependencyDeclaration{FIRST_PASS_ID, 999U};
    std::array<RenderFixturePassRequest, 2U> prepared_requests{};
    FillPreparedSentinel(std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()));
    std::array<RenderFixturePassResult, 2U> pass_results{};
    RenderGraphSkeleton graph;
    const RenderGraphSkeletonRequest request = GraphRequestFrom(
        GRAPH_ID,
        pass,
        std::span<const RenderGraphSkeletonPassDeclaration>(declarations.data(), declarations.size()),
        std::span<const RenderGraphSkeletonDependencyDeclaration>(dependencies.data(), dependencies.size()),
        std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()),
        std::span<RenderFixturePassResult>(pass_results.data(), pass_results.size()));
    const auto result = graph.Prepare(request);
    if (result.status != RenderGraphSkeletonStatus::MissingDependency) {
        return Fail("graph skeleton accepted missing dependency");
    }

    if (!PreparedSentinelUnchanged(std::span<const RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()))) {
        return Fail("missing dependency mutated prepared requests");
    }

    return 0;
}

int RenderCoreGraphSkeletonRejectsSelfDependencyWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePass pass;
    std::array<RenderGraphSkeletonPassDeclaration, 2U> declarations{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillTwoPassDeclarations(device, declarations, first_capture, second_capture)) {
        return Fail("self dependency setup failed");
    }

    std::array<RenderGraphSkeletonDependencyDeclaration, 1U> dependencies{};
    dependencies[0U] = RenderGraphSkeletonDependencyDeclaration{FIRST_PASS_ID, FIRST_PASS_ID};
    std::array<RenderFixturePassRequest, 2U> prepared_requests{};
    FillPreparedSentinel(std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()));
    std::array<RenderFixturePassResult, 2U> pass_results{};
    RenderGraphSkeleton graph;
    const RenderGraphSkeletonRequest request = GraphRequestFrom(
        GRAPH_ID,
        pass,
        std::span<const RenderGraphSkeletonPassDeclaration>(declarations.data(), declarations.size()),
        std::span<const RenderGraphSkeletonDependencyDeclaration>(dependencies.data(), dependencies.size()),
        std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()),
        std::span<RenderFixturePassResult>(pass_results.data(), pass_results.size()));
    const auto result = graph.Prepare(request);
    if (result.status != RenderGraphSkeletonStatus::SelfDependency) {
        return Fail("graph skeleton accepted self dependency");
    }

    if (!PreparedSentinelUnchanged(std::span<const RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()))) {
        return Fail("self dependency mutated prepared requests");
    }

    return 0;
}

int RenderCoreGraphSkeletonRejectsDependencyCycleWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePass pass;
    std::array<RenderGraphSkeletonPassDeclaration, 2U> declarations{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillTwoPassDeclarations(device, declarations, first_capture, second_capture)) {
        return Fail("cycle dependency setup failed");
    }

    std::array<RenderGraphSkeletonDependencyDeclaration, 2U> dependencies{};
    dependencies[0U] = RenderGraphSkeletonDependencyDeclaration{FIRST_PASS_ID, SECOND_PASS_ID};
    dependencies[1U] = RenderGraphSkeletonDependencyDeclaration{SECOND_PASS_ID, FIRST_PASS_ID};
    std::array<RenderFixturePassRequest, 2U> prepared_requests{};
    FillPreparedSentinel(std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()));
    std::array<RenderFixturePassResult, 2U> pass_results{};
    RenderGraphSkeleton graph;
    const RenderGraphSkeletonRequest request = GraphRequestFrom(
        GRAPH_ID,
        pass,
        std::span<const RenderGraphSkeletonPassDeclaration>(declarations.data(), declarations.size()),
        std::span<const RenderGraphSkeletonDependencyDeclaration>(dependencies.data(), dependencies.size()),
        std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()),
        std::span<RenderFixturePassResult>(pass_results.data(), pass_results.size()));
    const auto result = graph.Prepare(request);
    if (result.status != RenderGraphSkeletonStatus::DependencyCycle) {
        return Fail("graph skeleton accepted dependency cycle");
    }

    if (!PreparedSentinelUnchanged(std::span<const RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()))) {
        return Fail("dependency cycle mutated prepared requests");
    }

    return 0;
}

int RenderCoreGraphSkeletonRejectsPassCapacityOverflowWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePass pass;
    std::array<RenderGraphSkeletonPassDeclaration, 2U> declarations{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillTwoPassDeclarations(device, declarations, first_capture, second_capture)) {
        return Fail("pass capacity setup failed");
    }

    std::array<RenderFixturePassRequest, 2U> prepared_requests{};
    FillPreparedSentinel(std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()));
    std::array<RenderFixturePassResult, 2U> pass_results{};
    RenderGraphSkeletonDesc desc{};
    desc.pass_record_capacity = 1U;
    RenderGraphSkeleton graph(desc);
    const std::span<const RenderGraphSkeletonDependencyDeclaration> empty_dependencies{};
    const RenderGraphSkeletonRequest request = GraphRequestFrom(
        GRAPH_ID,
        pass,
        std::span<const RenderGraphSkeletonPassDeclaration>(declarations.data(), declarations.size()),
        empty_dependencies,
        std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()),
        std::span<RenderFixturePassResult>(pass_results.data(), pass_results.size()));
    const auto result = graph.Prepare(request);
    if (result.status != RenderGraphSkeletonStatus::PassCapacityExceeded) {
        return Fail("graph skeleton accepted pass capacity overflow");
    }

    if (!PreparedSentinelUnchanged(std::span<const RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()))) {
        return Fail("pass capacity overflow mutated prepared requests");
    }

    return 0;
}

int RenderCoreGraphSkeletonRejectsDependencyCapacityOverflowWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePass pass;
    std::array<RenderGraphSkeletonPassDeclaration, 2U> declarations{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillTwoPassDeclarations(device, declarations, first_capture, second_capture)) {
        return Fail("dependency capacity setup failed");
    }

    std::array<RenderGraphSkeletonDependencyDeclaration, 1U> dependencies{};
    dependencies[0U] = RenderGraphSkeletonDependencyDeclaration{FIRST_PASS_ID, SECOND_PASS_ID};
    std::array<RenderFixturePassRequest, 2U> prepared_requests{};
    FillPreparedSentinel(std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()));
    std::array<RenderFixturePassResult, 2U> pass_results{};
    RenderGraphSkeletonDesc desc{};
    desc.dependency_record_capacity = 0U;
    RenderGraphSkeleton graph(desc);
    const RenderGraphSkeletonRequest request = GraphRequestFrom(
        GRAPH_ID,
        pass,
        std::span<const RenderGraphSkeletonPassDeclaration>(declarations.data(), declarations.size()),
        std::span<const RenderGraphSkeletonDependencyDeclaration>(dependencies.data(), dependencies.size()),
        std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()),
        std::span<RenderFixturePassResult>(pass_results.data(), pass_results.size()));
    const auto result = graph.Prepare(request);
    if (result.status != RenderGraphSkeletonStatus::DependencyCapacityExceeded) {
        return Fail("graph skeleton accepted dependency capacity overflow");
    }

    if (!PreparedSentinelUnchanged(std::span<const RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()))) {
        return Fail("dependency capacity overflow mutated prepared requests");
    }

    return 0;
}

int RenderCoreGraphSkeletonRejectsInvalidPassRequestWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePass pass;
    std::array<RenderGraphSkeletonPassDeclaration, 2U> declarations{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillTwoPassDeclarations(device, declarations, first_capture, second_capture)) {
        return Fail("invalid pass setup failed");
    }

    declarations[1U].pass_request.target = RhiTextureHandle{};
    std::array<RenderFixturePassRequest, 2U> prepared_requests{};
    FillPreparedSentinel(std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()));
    std::array<RenderFixturePassResult, 2U> pass_results{};
    RenderGraphSkeleton graph;
    const std::span<const RenderGraphSkeletonDependencyDeclaration> empty_dependencies{};
    const RenderGraphSkeletonRequest request = GraphRequestFrom(
        GRAPH_ID,
        pass,
        std::span<const RenderGraphSkeletonPassDeclaration>(declarations.data(), declarations.size()),
        empty_dependencies,
        std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()),
        std::span<RenderFixturePassResult>(pass_results.data(), pass_results.size()));
    const auto result = graph.Prepare(request);
    if (result.status != RenderGraphSkeletonStatus::InvalidPassRequest) {
        return Fail("graph skeleton accepted invalid pass request");
    }

    if (result.pass_status != RenderFixturePassStatus::InvalidTarget) {
        return Fail("graph skeleton did not report invalid pass status");
    }

    if (!PreparedSentinelUnchanged(std::span<const RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()))) {
        return Fail("invalid pass request mutated prepared requests");
    }

    return 0;
}

int RenderCoreGraphSkeletonSnapshotTracksBoundedCounters() {
    RenderGraphSkeleton graph;
    const int prepare_result = PrepareAcceptedGraph(graph);
    if (prepare_result != 0) {
        return prepare_result;
    }

    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePass pass;
    std::array<RenderGraphSkeletonPassDeclaration, 2U> declarations{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillTwoPassDeclarations(device, declarations, first_capture, second_capture)) {
        return Fail("snapshot setup failed");
    }

    declarations[1U].pass_id = FIRST_PASS_ID;
    declarations[1U].pass_request.pass_id = FIRST_PASS_ID;
    std::array<RenderFixturePassRequest, 2U> prepared_requests{};
    std::array<RenderFixturePassResult, 2U> pass_results{};
    const std::span<const RenderGraphSkeletonDependencyDeclaration> empty_dependencies{};
    const RenderGraphSkeletonRequest request = GraphRequestFrom(
        NEXT_GRAPH_ID,
        pass,
        std::span<const RenderGraphSkeletonPassDeclaration>(declarations.data(), declarations.size()),
        empty_dependencies,
        std::span<RenderFixturePassRequest>(prepared_requests.data(), prepared_requests.size()),
        std::span<RenderFixturePassResult>(pass_results.data(), pass_results.size()));
    const auto duplicate_result = graph.Prepare(request);
    if (duplicate_result.status != RenderGraphSkeletonStatus::DuplicatePassId) {
        return Fail("snapshot setup did not hit duplicate pass id");
    }

    if (graph.Release(GRAPH_ID) != RenderGraphSkeletonStatus::Success) {
        return Fail("snapshot release failed");
    }

    const auto snapshot = graph.Snapshot();
    if (snapshot.prepared_graph_count != 1U || snapshot.released_graph_count != 1U) {
        return Fail("graph skeleton snapshot missed prepare or release counters");
    }

    if (snapshot.duplicate_pass_id_count != 1U || snapshot.graph_record_count != 0U) {
        return Fail("graph skeleton snapshot missed duplicate pass counter");
    }

    if (snapshot.last_status != RenderGraphSkeletonStatus::Success) {
        return Fail("graph skeleton snapshot missed last release status");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_PREPARES_BATCH) {
        return RenderCoreGraphSkeletonPreparesSubmissionBatchInDeclarationOrder();
    }

    if (name == TEST_QUERY) {
        return RenderCoreGraphSkeletonQueryReturnsGraphRecord();
    }

    if (name == TEST_RELEASE) {
        return RenderCoreGraphSkeletonReleaseClearsGraphRecord();
    }

    if (name == TEST_ZERO_GRAPH_ID) {
        return RenderCoreGraphSkeletonRejectsZeroGraphIdWithoutMutation();
    }

    if (name == TEST_EMPTY_GRAPH) {
        return RenderCoreGraphSkeletonRejectsEmptyGraphWithoutMutation();
    }

    if (name == TEST_DUPLICATE_GRAPH) {
        return RenderCoreGraphSkeletonRejectsDuplicateGraphIdWithoutMutation();
    }

    if (name == TEST_DUPLICATE_PASS) {
        return RenderCoreGraphSkeletonRejectsDuplicatePassIdWithoutMutation();
    }

    if (name == TEST_MISSING_DEPENDENCY) {
        return RenderCoreGraphSkeletonRejectsMissingDependencyWithoutMutation();
    }

    if (name == TEST_SELF_DEPENDENCY) {
        return RenderCoreGraphSkeletonRejectsSelfDependencyWithoutMutation();
    }

    if (name == TEST_DEPENDENCY_CYCLE) {
        return RenderCoreGraphSkeletonRejectsDependencyCycleWithoutMutation();
    }

    if (name == TEST_PASS_CAPACITY) {
        return RenderCoreGraphSkeletonRejectsPassCapacityOverflowWithoutMutation();
    }

    if (name == TEST_DEPENDENCY_CAPACITY) {
        return RenderCoreGraphSkeletonRejectsDependencyCapacityOverflowWithoutMutation();
    }

    if (name == TEST_INVALID_PASS) {
        return RenderCoreGraphSkeletonRejectsInvalidPassRequestWithoutMutation();
    }

    if (name == TEST_SNAPSHOT) {
        return RenderCoreGraphSkeletonSnapshotTracksBoundedCounters();
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
