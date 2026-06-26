// 模块：Tests RenderCore
// 文件：Tests/RenderCore/RenderCoreTests.cpp

#include <array>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>
#include <vector>

#include "YuEngine/RenderCore/MaterialBindingFixture.h"
#include "YuEngine/RenderCore/RenderFixturePass.h"
#include "YuEngine/RenderCore/RenderFixturePassConstants.h"
#include "YuEngine/RenderCore/RenderFixturePassDesc.h"
#include "YuEngine/RenderCore/RenderFixturePassRequest.h"
#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/RenderCore/RenderFramePacketFixture.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureDesc.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureRequest.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureResult.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureStatus.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixture.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureDesc.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureRequest.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureResult.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureStatus.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/NullRhiDevice.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiBufferUsage.h"
#include "YuEngine/Rhi/RhiColorTargetDesc.h"
#include "YuEngine/Rhi/RhiConstantBufferBinding.h"
#include "YuEngine/Rhi/RhiConstants.h"
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
using MaterialBindingFixture = yuengine::rendercore::MaterialBindingFixture;
using MaterialBindingFixtureDesc = yuengine::rendercore::MaterialBindingFixtureDesc;
using MaterialBindingFixtureRequest = yuengine::rendercore::MaterialBindingFixtureRequest;
using yuengine::rendercore::MaterialBindingFixtureStatus;
using yuengine::rendercore::MAX_MATERIAL_BINDING_FIXTURE_CONSTANT_BYTES;
using RenderFixturePassDesc = yuengine::rendercore::RenderFixturePassDesc;
using RenderFixturePassRequest = yuengine::rendercore::RenderFixturePassRequest;
using RenderFixturePassResult = yuengine::rendercore::RenderFixturePassResult;
using yuengine::rendercore::RenderFixturePassStatus;
using yuengine::rendercore::RENDER_FIXTURE_PASS_COMMAND_COUNT;
using RenderFramePacketFixture = yuengine::rendercore::RenderFramePacketFixture;
using RenderFramePacketFixtureDesc = yuengine::rendercore::RenderFramePacketFixtureDesc;
using RenderFramePacketFixtureRequest = yuengine::rendercore::RenderFramePacketFixtureRequest;
using RenderFramePacketFixtureResult = yuengine::rendercore::RenderFramePacketFixtureResult;
using yuengine::rendercore::RenderFramePacketFixtureStatus;
using RenderSubmissionBatchFixture = yuengine::rendercore::RenderSubmissionBatchFixture;
using RenderSubmissionBatchFixtureDesc = yuengine::rendercore::RenderSubmissionBatchFixtureDesc;
using RenderSubmissionBatchFixtureRequest = yuengine::rendercore::RenderSubmissionBatchFixtureRequest;
using RenderSubmissionBatchFixtureResult = yuengine::rendercore::RenderSubmissionBatchFixtureResult;
using yuengine::rendercore::RenderSubmissionBatchFixtureStatus;
using IRhiDevice = yuengine::rhi::IRhiDevice;
using NullRhiDevice = yuengine::rhi::NullRhiDevice;
using RhiBufferDesc = yuengine::rhi::RhiBufferDesc;
using RhiBufferHandle = yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiBufferUsage;
using RhiColor = yuengine::rhi::RhiColor;
using RhiColorTargetDesc = yuengine::rhi::RhiColorTargetDesc;
using RhiConstantBufferBinding = yuengine::rhi::RhiConstantBufferBinding;
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
constexpr const char *TEST_EXECUTES_PASS = "RenderCore_FixturePass_ExecutesSampledIndexedPass";
constexpr const char *TEST_NULL_RHI = "RenderCore_FixturePass_RejectsNullRhiDeviceWithoutMutation";
constexpr const char *TEST_INVALID_TARGET = "RenderCore_FixturePass_RejectsInvalidTargetWithoutMutation";
constexpr const char *TEST_INVALID_PIPELINE = "RenderCore_FixturePass_RejectsInvalidPipelineWithoutMutation";
constexpr const char *TEST_MISSING_VERTEX = "RenderCore_FixturePass_RejectsMissingVertexBufferWithoutMutation";
constexpr const char *TEST_MISSING_INDEX = "RenderCore_FixturePass_RejectsMissingIndexBufferWithoutMutation";
constexpr const char *TEST_INVALID_TEXTURE = "RenderCore_FixturePass_RejectsInvalidTextureBindingWithoutMutation";
constexpr const char *TEST_INVALID_SAMPLER = "RenderCore_FixturePass_RejectsInvalidSamplerBindingWithoutMutation";
constexpr const char *TEST_CONSTANT_BUFFER_BIND = "RenderCore_FixturePass_BindsConstantBuffer";
constexpr const char *TEST_INVALID_DRAW = "RenderCore_FixturePass_RejectsInvalidDrawWithoutMutation";
constexpr const char *TEST_SMALL_CAPTURE = "RenderCore_FixturePass_RejectsSmallCaptureStorageWithoutMutation";
constexpr const char *TEST_COMMAND_CAPACITY = "RenderCore_FixturePass_RejectsCommandCapacityWithoutRhiMutation";
constexpr const char *TEST_RECORD_CAPACITY = "RenderCore_FixturePass_RejectsPassRecordCapacityWithoutRhiMutation";
constexpr const char *TEST_RHI_FAILURE = "RenderCore_FixturePass_TracksRhiFailureWithoutWritingCapture";
constexpr const char *TEST_SNAPSHOT = "RenderCore_FixturePass_SnapshotTracksBoundedCounters";
constexpr const char *TEST_MATERIAL_BIND = "Material_BindingFixture_BindsValuesToRenderFixtureRequest";
constexpr const char *TEST_MATERIAL_INVALID_PIPELINE = "Material_BindingFixture_RejectsInvalidPipelineWithoutMutation";
constexpr const char *TEST_MATERIAL_INVALID_TEXTURE = "Material_BindingFixture_RejectsInvalidTextureBindingWithoutMutation";
constexpr const char *TEST_MATERIAL_INVALID_SAMPLER = "Material_BindingFixture_RejectsInvalidSamplerBindingWithoutMutation";
constexpr const char *TEST_MATERIAL_CONSTANT_BUFFER_BIND =
    "Material_BindingFixture_PropagatesConstantBufferBindings";
constexpr const char *TEST_MATERIAL_OVERSIZED_CONSTANTS = "Material_BindingFixture_RejectsOversizedConstantsWithoutMutation";
constexpr const char *TEST_MATERIAL_DUPLICATE = "Material_BindingFixture_RejectsDuplicateMaterialIdWithoutMutation";
constexpr const char *TEST_MATERIAL_CAPACITY = "Material_BindingFixture_RejectsCapacityOverflowWithoutMutation";
constexpr const char *TEST_MATERIAL_PASS_FAILURE = "Material_BindingFixture_PropagatesRenderFixturePassFailure";
constexpr const char *TEST_MATERIAL_SNAPSHOT = "Material_BindingFixture_SnapshotTracksBoundedCounters";
constexpr const char *TEST_BATCH_EXECUTES_MATERIAL = "RenderCore_SubmissionBatch_ExecutesMaterialPreparedRequests";
constexpr const char *TEST_BATCH_EMPTY = "RenderCore_SubmissionBatch_RejectsEmptyBatchWithoutMutation";
constexpr const char *TEST_BATCH_NULL_PASS = "RenderCore_SubmissionBatch_RejectsNullPassWithoutMutation";
constexpr const char *TEST_BATCH_INVALID_PASS_REQUEST = "RenderCore_SubmissionBatch_RejectsInvalidPassRequestWithoutMutation";
constexpr const char *TEST_BATCH_DUPLICATE_PASS_ID = "RenderCore_SubmissionBatch_RejectsDuplicatePassIdWithoutMutation";
constexpr const char *TEST_BATCH_CAPACITY = "RenderCore_SubmissionBatch_RejectsBatchCapacityWithoutMutation";
constexpr const char *TEST_BATCH_PASS_FAILURE = "RenderCore_SubmissionBatch_PropagatesRenderFixturePassFailure";
constexpr const char *TEST_BATCH_SNAPSHOT = "RenderCore_SubmissionBatch_SnapshotTracksBoundedCounters";
constexpr const char *TEST_FRAME_PACKET_EXECUTES_BATCH = "RenderCore_FramePacket_ExecutesPreparedSubmissionBatch";
constexpr const char *TEST_FRAME_PACKET_ZERO_FRAME = "RenderCore_FramePacket_RejectsZeroFrameIdWithoutMutation";
constexpr const char *TEST_FRAME_PACKET_NULL_EXECUTOR = "RenderCore_FramePacket_RejectsNullExecutorWithoutMutation";
constexpr const char *TEST_FRAME_PACKET_NULL_BATCH_REQUEST = "RenderCore_FramePacket_RejectsNullBatchRequestWithoutMutation";
constexpr const char *TEST_FRAME_PACKET_INVALID_BATCH_REQUEST = "RenderCore_FramePacket_RejectsInvalidBatchRequestWithoutMutation";
constexpr const char *TEST_FRAME_PACKET_DUPLICATE_FRAME = "RenderCore_FramePacket_RejectsDuplicateFrameIdWithoutMutation";
constexpr const char *TEST_FRAME_PACKET_CAPACITY = "RenderCore_FramePacket_RejectsPacketCapacityWithoutMutation";
constexpr const char *TEST_FRAME_PACKET_BATCH_FAILURE = "RenderCore_FramePacket_PropagatesSubmissionBatchFailure";
constexpr const char *TEST_FRAME_PACKET_SNAPSHOT = "RenderCore_FramePacket_SnapshotTracksBoundedCounters";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint8_t SENTINEL_BYTE = 0xA5U;
constexpr std::uint32_t TRIANGLE_VERTEX_COUNT = 3U;
constexpr std::uint32_t TRIANGLE_INDEX_COUNT = 3U;
constexpr std::size_t TRIANGLE_VERTEX_STRIDE_BYTES = sizeof(float) * 6U;
constexpr std::size_t TRIANGLE_VERTEX_BUFFER_BYTES = TRIANGLE_VERTEX_STRIDE_BYTES * TRIANGLE_VERTEX_COUNT;
constexpr std::size_t TRIANGLE_INDEX_BUFFER_BYTES = sizeof(std::uint16_t) * TRIANGLE_INDEX_COUNT;
constexpr std::size_t CAPTURE_BYTES = 16U;
constexpr std::uint32_t MATERIAL_ID = 41U;
constexpr std::uint32_t NEXT_MATERIAL_ID = 43U;
constexpr std::uint32_t MATERIAL_PASS_ID = 311U;
constexpr std::uint32_t FRAME_ID = 7001U;
constexpr std::uint32_t NEXT_FRAME_ID = 7003U;

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

std::array<std::uint8_t, 4U> MaterialConstantBytes() {
    return std::array<std::uint8_t, 4U>{3U, 5U, 7U, 11U};
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

bool CreateConstantBuffer(IRhiDevice &device, RhiBufferHandle &out_handle) {
    const std::array<std::uint8_t, yuengine::rhi::RHI_CONSTANT_BUFFER_ALIGNMENT> bytes{};
    RhiBufferDesc desc{};
    desc.usage = RhiBufferUsage::Constant;
    desc.size_bytes = bytes.size();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    return device.CreateBuffer(desc, byte_span, out_handle) == RhiStatus::Success;
}

RhiConstantBufferBinding ConstantBufferBindingFor(RhiBufferHandle handle) {
    RhiConstantBufferBinding binding{};
    binding.buffer = handle;
    binding.stage = RhiShaderStage::Pixel;
    binding.slot = 0U;
    return binding;
}

bool FillValidFixture(NullRhiDevice &device, RenderFixturePassRequest &request, std::vector<std::uint8_t> &capture) {
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
    request.pass_id = 17U;
    return true;
}

bool CaptureUnchanged(const std::vector<std::uint8_t> &capture) {
    for (std::uint8_t value : capture) {
        if (value != SENTINEL_BYTE) {
            return false;
        }
    }

    return true;
}

bool WorkCountersMatch(const RhiDeviceSnapshot &left, const RhiDeviceSnapshot &right) {
    if (left.recorded_command_count != right.recorded_command_count) {
        return false;
    }

    if (left.submit_count != right.submit_count) {
        return false;
    }

    if (left.submitted_constant_buffer_bind_count != right.submitted_constant_buffer_bind_count) {
        return false;
    }

    if (left.rejected_constant_buffer_bind_count != right.rejected_constant_buffer_bind_count) {
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

bool PipelineHandleMatches(RhiPipelineHandle left, RhiPipelineHandle right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

bool TextureHandleMatches(RhiTextureHandle left, RhiTextureHandle right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

bool SamplerHandleMatches(RhiSamplerHandle left, RhiSamplerHandle right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

bool SampledTextureBindingMatches(RhiSampledTextureBinding left, RhiSampledTextureBinding right) {
    if (!TextureHandleMatches(left.texture, right.texture)) {
        return false;
    }

    return left.slot == right.slot;
}

bool SamplerBindingMatches(RhiSamplerBinding left, RhiSamplerBinding right) {
    if (!SamplerHandleMatches(left.sampler, right.sampler)) {
        return false;
    }

    return left.slot == right.slot;
}

bool ConstantBufferBindingMatches(RhiConstantBufferBinding left, RhiConstantBufferBinding right) {
    if (left.buffer.slot != right.buffer.slot) {
        return false;
    }

    if (left.buffer.generation != right.buffer.generation) {
        return false;
    }

    if (left.stage != right.stage) {
        return false;
    }

    return left.slot == right.slot;
}

bool MaterialPassFieldsMatch(const RenderFixturePassRequest &left, const RenderFixturePassRequest &right) {
    if (!PipelineHandleMatches(left.pipeline, right.pipeline)) {
        return false;
    }

    if (!SampledTextureBindingMatches(left.sampled_texture, right.sampled_texture)) {
        return false;
    }

    if (!SamplerBindingMatches(left.sampler, right.sampler)) {
        return false;
    }

    if (left.material_id != right.material_id) {
        return false;
    }

    if (left.material_constant_byte_count != right.material_constant_byte_count) {
        return false;
    }

    if (left.constant_buffers.size() != right.constant_buffers.size()) {
        return false;
    }

    for (std::size_t index = 0U; index < left.constant_buffers.size(); ++index) {
        if (!ConstantBufferBindingMatches(left.constant_buffers[index], right.constant_buffers[index])) {
            return false;
        }
    }

    return left.pass_id == right.pass_id;
}

MaterialBindingFixtureRequest MaterialRequestFrom(
    const RenderFixturePassRequest &pass_request,
    std::span<const std::uint8_t> constants) {
    MaterialBindingFixtureRequest request{};
    request.material_id = MATERIAL_ID;
    request.pipeline = pass_request.pipeline;
    request.sampled_texture = pass_request.sampled_texture;
    request.sampler = pass_request.sampler;
    request.constant_bytes = constants;
    request.pass_id = MATERIAL_PASS_ID;
    return request;
}

RenderFixturePassResult SentinelPassResult() {
    RenderFixturePassResult result{};
    result.status = RenderFixturePassStatus::RhiFailure;
    result.rhi_status = RhiStatus::InvalidHandle;
    result.recorded_command_count = 97U;
    result.capture_bytes_written = 53U;
    result.pass_id = 65535U;
    return result;
}

bool RenderFixturePassResultMatches(
    const RenderFixturePassResult &left,
    const RenderFixturePassResult &right) {
    if (left.status != right.status) {
        return false;
    }

    if (left.rhi_status != right.rhi_status) {
        return false;
    }

    if (left.recorded_command_count != right.recorded_command_count) {
        return false;
    }

    if (left.capture_bytes_written != right.capture_bytes_written) {
        return false;
    }

    return left.pass_id == right.pass_id;
}

bool PassResultsUnchanged(
    const std::array<RenderFixturePassResult, 2U> &results,
    const RenderFixturePassResult &sentinel) {
    for (const RenderFixturePassResult &result : results) {
        if (!RenderFixturePassResultMatches(result, sentinel)) {
            return false;
        }
    }

    return true;
}

RenderSubmissionBatchFixtureRequest BatchRequestFrom(
    RenderFixturePass &pass,
    std::span<const RenderFixturePassRequest> pass_requests,
    std::span<RenderFixturePassResult> pass_results) {
    RenderSubmissionBatchFixtureRequest request{};
    request.pass = &pass;
    request.pass_requests = pass_requests;
    request.pass_results = pass_results;
    return request;
}

RenderSubmissionBatchFixtureRequest NullPassBatchRequestFrom(
    std::span<const RenderFixturePassRequest> pass_requests,
    std::span<RenderFixturePassResult> pass_results) {
    RenderSubmissionBatchFixtureRequest request{};
    request.pass = nullptr;
    request.pass_requests = pass_requests;
    request.pass_results = pass_results;
    return request;
}

RenderFramePacketFixtureRequest FramePacketRequestFrom(
    std::uint32_t frame_id,
    RenderSubmissionBatchFixture &submission_batch,
    const RenderSubmissionBatchFixtureRequest &batch_request) {
    RenderFramePacketFixtureRequest request{};
    request.frame_id = frame_id;
    request.submission_batch = &submission_batch;
    request.batch_request = &batch_request;
    return request;
}

RenderFramePacketFixtureRequest NullFramePacketExecutorRequestFrom(
    std::uint32_t frame_id,
    const RenderSubmissionBatchFixtureRequest &batch_request) {
    RenderFramePacketFixtureRequest request{};
    request.frame_id = frame_id;
    request.submission_batch = nullptr;
    request.batch_request = &batch_request;
    return request;
}

RenderFramePacketFixtureRequest NullBatchRequestFramePacketRequestFrom(
    std::uint32_t frame_id,
    RenderSubmissionBatchFixture &submission_batch) {
    RenderFramePacketFixtureRequest request{};
    request.frame_id = frame_id;
    request.submission_batch = &submission_batch;
    request.batch_request = nullptr;
    return request;
}

bool BindMaterialForBatch(
    MaterialBindingFixture &fixture,
    RenderFixturePassRequest &pass_request,
    std::uint32_t material_id,
    std::uint32_t pass_id) {
    const std::array<std::uint8_t, 4U> constants = MaterialConstantBytes();
    const std::span<const std::uint8_t> constant_span(constants.data(), constants.size());
    MaterialBindingFixtureRequest material_request = MaterialRequestFrom(pass_request, constant_span);
    material_request.material_id = material_id;
    material_request.pass_id = pass_id;
    const auto result = fixture.Bind(material_request, &pass_request);
    return result.status == MaterialBindingFixtureStatus::Success;
}

bool FillMaterialPreparedBatchRequest(
    NullRhiDevice &device,
    MaterialBindingFixture &fixture,
    RenderFixturePassRequest &pass_request,
    std::vector<std::uint8_t> &capture,
    std::uint32_t material_id,
    std::uint32_t pass_id) {
    if (!FillValidFixture(device, pass_request, capture)) {
        return false;
    }

    return BindMaterialForBatch(fixture, pass_request, material_id, pass_id);
}

int ExpectMaterialValidationFailure(
    MaterialBindingFixtureStatus expected_status,
    const MaterialBindingFixtureRequest &material_request,
    RenderFixturePassRequest &pass_request) {
    MaterialBindingFixture fixture;
    const RenderFixturePassRequest before = pass_request;
    const auto result = fixture.Bind(material_request, &pass_request);
    if (result.status != expected_status) {
        return Fail("material binding fixture returned unexpected validation status");
    }

    if (!MaterialPassFieldsMatch(before, pass_request)) {
        return Fail("material binding validation failure mutated pass request");
    }

    const auto snapshot = fixture.Snapshot();
    if (snapshot.binding_record_count != 0U) {
        return Fail("material binding validation failure recorded binding");
    }

    if (snapshot.failed_validation_count != 1U) {
        return Fail("material binding validation counter was not updated");
    }

    return 0;
}

int ExpectValidationFailure(RenderFixturePassStatus expected_status, RenderFixturePassRequest request, NullRhiDevice &device, std::vector<std::uint8_t> &capture) {
    RenderFixturePass pass;
    const RhiDeviceSnapshot before = device.Snapshot();
    const auto result = pass.Execute(request);
    if (result.status != expected_status) {
        return Fail("fixture pass returned unexpected validation status");
    }

    if (!CaptureUnchanged(capture)) {
        return Fail("validation failure wrote capture output");
    }

    const RhiDeviceSnapshot after = device.Snapshot();
    if (!WorkCountersMatch(before, after)) {
        return Fail("validation failure mutated RHI work counters");
    }

    const auto snapshot = pass.Snapshot();
    if (snapshot.pass_record_count != 0U) {
        return Fail("validation failure recorded pass result");
    }

    if (snapshot.failed_validation_count != 1U) {
        return Fail("validation failure counter was not updated");
    }

    return 0;
}

int RenderCoreFixturePassExecutesSampledIndexedPass() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, request, capture)) {
        return Fail("fixture setup failed");
    }

    RenderFixturePass pass;
    const auto result = pass.Execute(request);
    if (result.status != RenderFixturePassStatus::Success) {
        return Fail("fixture pass did not succeed");
    }

    if (result.capture_bytes_written != CAPTURE_BYTES) {
        return Fail("fixture pass wrote unexpected capture byte count");
    }

    if (result.recorded_command_count != RENDER_FIXTURE_PASS_COMMAND_COUNT) {
        return Fail("fixture pass recorded unexpected command count");
    }

    const auto snapshot = pass.Snapshot();
    if (snapshot.completed_pass_count != 1U) {
        return Fail("fixture pass did not update completion counter");
    }

    const RhiDeviceSnapshot rhi_snapshot = device.Snapshot();
    if (rhi_snapshot.submit_count != 1U || rhi_snapshot.present_count != 1U || rhi_snapshot.capture_count != 1U) {
        return Fail("fixture pass did not submit present and capture once");
    }

    if (rhi_snapshot.submitted_indexed_draw_count != 1U) {
        return Fail("fixture pass did not submit indexed draw");
    }

    if (rhi_snapshot.submitted_sampled_texture_bind_count != 1U || rhi_snapshot.submitted_sampler_bind_count != 1U) {
        return Fail("fixture pass did not submit texture and sampler bindings");
    }

    return 0;
}

int RenderCoreFixturePassBindsConstantBuffer() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RenderFixturePassRequest request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, request, capture)) {
        return Fail("fixture setup failed");
    }

    RhiBufferHandle constant_buffer{};
    if (!CreateConstantBuffer(device_interface, constant_buffer)) {
        return Fail("constant buffer creation failed");
    }

    const std::array<RhiConstantBufferBinding, 1U> constant_buffers{
        ConstantBufferBindingFor(constant_buffer)};
    request.constant_buffers =
        std::span<const RhiConstantBufferBinding>(constant_buffers.data(), constant_buffers.size());

    RenderFixturePass pass;
    const auto result = pass.Execute(request);
    if (result.status != RenderFixturePassStatus::Success) {
        return Fail("fixture pass with constant buffer did not succeed");
    }

    if (result.recorded_command_count != RENDER_FIXTURE_PASS_COMMAND_COUNT + 1U) {
        return Fail("fixture pass did not record constant buffer bind command");
    }

    const RhiDeviceSnapshot rhi_snapshot = device.Snapshot();
    if (rhi_snapshot.submitted_constant_buffer_bind_count != 1U) {
        return Fail("fixture pass did not submit constant buffer bind");
    }

    if (rhi_snapshot.last_bound_constant_buffer_stage != RhiShaderStage::Pixel) {
        return Fail("fixture pass did not track constant buffer shader stage");
    }

    return 0;
}

int RenderCoreFixturePassRejectsNullRhiDeviceWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, request, capture)) {
        return Fail("fixture setup failed");
    }

    request.rhi_device = nullptr;
    return ExpectValidationFailure(RenderFixturePassStatus::InvalidArgument, request, device, capture);
}

int RenderCoreFixturePassRejectsInvalidTargetWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, request, capture)) {
        return Fail("fixture setup failed");
    }

    request.target = RhiTextureHandle{};
    return ExpectValidationFailure(RenderFixturePassStatus::InvalidTarget, request, device, capture);
}

int RenderCoreFixturePassRejectsInvalidPipelineWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, request, capture)) {
        return Fail("fixture setup failed");
    }

    request.pipeline = RhiPipelineHandle{};
    return ExpectValidationFailure(RenderFixturePassStatus::InvalidPipeline, request, device, capture);
}

int RenderCoreFixturePassRejectsMissingVertexBufferWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, request, capture)) {
        return Fail("fixture setup failed");
    }

    request.vertex_buffer = RhiVertexBufferView{};
    return ExpectValidationFailure(RenderFixturePassStatus::MissingVertexBuffer, request, device, capture);
}

int RenderCoreFixturePassRejectsMissingIndexBufferWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, request, capture)) {
        return Fail("fixture setup failed");
    }

    request.index_buffer = RhiIndexBufferView{};
    return ExpectValidationFailure(RenderFixturePassStatus::MissingIndexBuffer, request, device, capture);
}

int RenderCoreFixturePassRejectsInvalidTextureBindingWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, request, capture)) {
        return Fail("fixture setup failed");
    }

    request.sampled_texture = RhiSampledTextureBinding{};
    return ExpectValidationFailure(RenderFixturePassStatus::InvalidTextureBinding, request, device, capture);
}

int RenderCoreFixturePassRejectsInvalidSamplerBindingWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, request, capture)) {
        return Fail("fixture setup failed");
    }

    request.sampler = RhiSamplerBinding{};
    return ExpectValidationFailure(RenderFixturePassStatus::InvalidSamplerBinding, request, device, capture);
}

int RenderCoreFixturePassRejectsInvalidDrawWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, request, capture)) {
        return Fail("fixture setup failed");
    }

    request.draw.index_count = 0U;
    return ExpectValidationFailure(RenderFixturePassStatus::InvalidDraw, request, device, capture);
}

int RenderCoreFixturePassRejectsSmallCaptureStorageWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, request, capture)) {
        return Fail("fixture setup failed");
    }

    request.capture_byte_budget = capture.size() + 1U;
    return ExpectValidationFailure(RenderFixturePassStatus::InsufficientCaptureStorage, request, device, capture);
}

int RenderCoreFixturePassRejectsCommandCapacityWithoutRhiMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, request, capture)) {
        return Fail("fixture setup failed");
    }

    RenderFixturePassDesc desc{};
    desc.command_capacity = RENDER_FIXTURE_PASS_COMMAND_COUNT - 1U;
    RenderFixturePass pass(desc);
    const RhiDeviceSnapshot before = device.Snapshot();
    const auto result = pass.Execute(request);
    if (result.status != RenderFixturePassStatus::CommandCapacityExceeded) {
        return Fail("fixture pass accepted small command capacity");
    }

    if (!CaptureUnchanged(capture)) {
        return Fail("command capacity rejection wrote capture output");
    }

    const RhiDeviceSnapshot after = device.Snapshot();
    if (!WorkCountersMatch(before, after)) {
        return Fail("command capacity rejection mutated RHI work counters");
    }

    const auto snapshot = pass.Snapshot();
    if (snapshot.command_capacity_rejected_count != 1U) {
        return Fail("command capacity rejection counter was not updated");
    }

    return 0;
}

int RenderCoreFixturePassRejectsPassRecordCapacityWithoutRhiMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, request, capture)) {
        return Fail("fixture setup failed");
    }

    RenderFixturePassDesc desc{};
    desc.pass_record_capacity = 1U;
    RenderFixturePass pass(desc);
    if (pass.Execute(request).status != RenderFixturePassStatus::Success) {
        return Fail("first fixture pass did not succeed");
    }

    std::fill(capture.begin(), capture.end(), SENTINEL_BYTE);
    const RhiDeviceSnapshot before = device.Snapshot();
    const auto result = pass.Execute(request);
    if (result.status != RenderFixturePassStatus::PassRecordCapacityExceeded) {
        return Fail("fixture pass accepted record capacity overflow");
    }

    if (!CaptureUnchanged(capture)) {
        return Fail("record capacity rejection wrote capture output");
    }

    const RhiDeviceSnapshot after = device.Snapshot();
    if (!WorkCountersMatch(before, after)) {
        return Fail("record capacity rejection mutated RHI work counters");
    }

    const auto snapshot = pass.Snapshot();
    if (snapshot.pass_record_capacity_rejected_count != 1U) {
        return Fail("record capacity rejection counter was not updated");
    }

    return 0;
}

int RenderCoreFixturePassTracksRhiFailureWithoutWritingCapture() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, request, capture)) {
        return Fail("fixture setup failed");
    }

    request.target = RhiTextureHandle{999U, 1U};
    RenderFixturePass pass;
    const auto result = pass.Execute(request);
    if (result.status != RenderFixturePassStatus::RhiFailure) {
        return Fail("fixture pass did not track RHI failure");
    }

    if (result.rhi_status != RhiStatus::InvalidHandle) {
        return Fail("fixture pass did not propagate RHI status");
    }

    if (!CaptureUnchanged(capture)) {
        return Fail("RHI failure wrote capture output");
    }

    const auto snapshot = pass.Snapshot();
    if (snapshot.rhi_failure_count != 1U) {
        return Fail("RHI failure counter was not updated");
    }

    if (snapshot.pass_record_count != 1U) {
        return Fail("RHI failure did not record result");
    }

    return 0;
}

int RenderCoreFixturePassSnapshotTracksBoundedCounters() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, request, capture)) {
        return Fail("fixture setup failed");
    }

    RenderFixturePass pass;
    const auto before = pass.Snapshot();
    if (before.pass_record_capacity == 0U || before.command_capacity == 0U) {
        return Fail("initial fixture pass capacities were not tracked");
    }

    if (pass.Execute(request).status != RenderFixturePassStatus::Success) {
        return Fail("fixture pass did not succeed");
    }

    const auto after = pass.Snapshot();
    if (after.executed_pass_count != 1U || after.completed_pass_count != 1U) {
        return Fail("fixture pass execution counters were not updated");
    }

    if (after.last_status != RenderFixturePassStatus::Success || after.last_pass_id != request.pass_id) {
        return Fail("fixture pass last status or pass id was unexpected");
    }

    if (after.last_recorded_command_count != RENDER_FIXTURE_PASS_COMMAND_COUNT) {
        return Fail("fixture pass last command count was unexpected");
    }

    return 0;
}

int MaterialBindingFixtureBindsValuesToRenderFixtureRequest() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest pass_request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, pass_request, capture)) {
        return Fail("fixture setup failed");
    }

    const std::array<std::uint8_t, 4U> constants = MaterialConstantBytes();
    const std::span<const std::uint8_t> constant_span(constants.data(), constants.size());
    MaterialBindingFixtureRequest material_request = MaterialRequestFrom(pass_request, constant_span);
    MaterialBindingFixture fixture;
    const auto result = fixture.Bind(material_request, &pass_request);
    if (result.status != MaterialBindingFixtureStatus::Success) {
        return Fail("material binding fixture did not bind values");
    }

    if (!PipelineHandleMatches(pass_request.pipeline, material_request.pipeline)) {
        return Fail("material binding fixture did not set pipeline");
    }

    if (!SampledTextureBindingMatches(pass_request.sampled_texture, material_request.sampled_texture)) {
        return Fail("material binding fixture did not set sampled texture");
    }

    if (!SamplerBindingMatches(pass_request.sampler, material_request.sampler)) {
        return Fail("material binding fixture did not set sampler");
    }

    if (pass_request.material_id != MATERIAL_ID || pass_request.material_constant_byte_count != constants.size()) {
        return Fail("material binding fixture did not set material metadata");
    }

    RenderFixturePass pass;
    const auto pass_result = pass.Execute(pass_request);
    if (pass_result.status != RenderFixturePassStatus::Success) {
        return Fail("material bound fixture pass did not execute");
    }

    const auto snapshot = fixture.Snapshot();
    if (snapshot.accepted_binding_count != 1U || snapshot.binding_record_count != 1U) {
        return Fail("material binding fixture did not track accepted binding");
    }

    return 0;
}

int MaterialBindingFixturePropagatesConstantBufferBindings() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RenderFixturePassRequest pass_request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, pass_request, capture)) {
        return Fail("fixture setup failed");
    }

    RhiBufferHandle constant_buffer{};
    if (!CreateConstantBuffer(device_interface, constant_buffer)) {
        return Fail("constant buffer creation failed");
    }

    const std::array<RhiConstantBufferBinding, 1U> constant_buffers{
        ConstantBufferBindingFor(constant_buffer)};
    const std::array<std::uint8_t, 4U> constants = MaterialConstantBytes();
    const std::span<const std::uint8_t> constant_span(constants.data(), constants.size());
    MaterialBindingFixtureRequest material_request = MaterialRequestFrom(pass_request, constant_span);
    material_request.constant_buffers =
        std::span<const RhiConstantBufferBinding>(constant_buffers.data(), constant_buffers.size());

    MaterialBindingFixture fixture;
    const auto result = fixture.Bind(material_request, &pass_request);
    if (result.status != MaterialBindingFixtureStatus::Success) {
        return Fail("material binding fixture did not bind constant buffer values");
    }

    if (pass_request.constant_buffers.size() != constant_buffers.size()) {
        return Fail("material binding fixture did not propagate constant buffer count");
    }

    if (!ConstantBufferBindingMatches(pass_request.constant_buffers[0U], constant_buffers[0U])) {
        return Fail("material binding fixture did not propagate constant buffer binding");
    }

    return 0;
}

int MaterialBindingFixtureRejectsInvalidPipelineWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest pass_request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, pass_request, capture)) {
        return Fail("fixture setup failed");
    }

    const std::array<std::uint8_t, 4U> constants = MaterialConstantBytes();
    const std::span<const std::uint8_t> constant_span(constants.data(), constants.size());
    MaterialBindingFixtureRequest material_request = MaterialRequestFrom(pass_request, constant_span);
    material_request.pipeline = RhiPipelineHandle{};
    return ExpectMaterialValidationFailure(MaterialBindingFixtureStatus::InvalidPipeline, material_request, pass_request);
}

int MaterialBindingFixtureRejectsInvalidTextureBindingWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest pass_request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, pass_request, capture)) {
        return Fail("fixture setup failed");
    }

    const std::array<std::uint8_t, 4U> constants = MaterialConstantBytes();
    const std::span<const std::uint8_t> constant_span(constants.data(), constants.size());
    MaterialBindingFixtureRequest material_request = MaterialRequestFrom(pass_request, constant_span);
    material_request.sampled_texture = RhiSampledTextureBinding{};
    return ExpectMaterialValidationFailure(
        MaterialBindingFixtureStatus::InvalidTextureBinding,
        material_request,
        pass_request);
}

int MaterialBindingFixtureRejectsInvalidSamplerBindingWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest pass_request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, pass_request, capture)) {
        return Fail("fixture setup failed");
    }

    const std::array<std::uint8_t, 4U> constants = MaterialConstantBytes();
    const std::span<const std::uint8_t> constant_span(constants.data(), constants.size());
    MaterialBindingFixtureRequest material_request = MaterialRequestFrom(pass_request, constant_span);
    material_request.sampler = RhiSamplerBinding{};
    return ExpectMaterialValidationFailure(
        MaterialBindingFixtureStatus::InvalidSamplerBinding,
        material_request,
        pass_request);
}

int MaterialBindingFixtureRejectsOversizedConstantsWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest pass_request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, pass_request, capture)) {
        return Fail("fixture setup failed");
    }

    std::array<std::uint8_t, MAX_MATERIAL_BINDING_FIXTURE_CONSTANT_BYTES + 1U> constants{};
    const std::span<const std::uint8_t> constant_span(constants.data(), constants.size());
    MaterialBindingFixtureRequest material_request = MaterialRequestFrom(pass_request, constant_span);
    return ExpectMaterialValidationFailure(
        MaterialBindingFixtureStatus::OversizedConstants,
        material_request,
        pass_request);
}

int MaterialBindingFixtureRejectsDuplicateMaterialIdWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest first_pass_request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, first_pass_request, capture)) {
        return Fail("fixture setup failed");
    }

    const std::array<std::uint8_t, 4U> constants = MaterialConstantBytes();
    const std::span<const std::uint8_t> constant_span(constants.data(), constants.size());
    MaterialBindingFixtureRequest first_request = MaterialRequestFrom(first_pass_request, constant_span);
    MaterialBindingFixture fixture;
    if (fixture.Bind(first_request, &first_pass_request).status != MaterialBindingFixtureStatus::Success) {
        return Fail("first material binding fixture bind failed");
    }

    RenderFixturePassRequest second_pass_request{};
    if (!FillValidFixture(device, second_pass_request, capture)) {
        return Fail("second fixture setup failed");
    }

    MaterialBindingFixtureRequest second_request = MaterialRequestFrom(second_pass_request, constant_span);
    second_request.pass_id = MATERIAL_PASS_ID + 1U;
    const RenderFixturePassRequest before = second_pass_request;
    const auto result = fixture.Bind(second_request, &second_pass_request);
    if (result.status != MaterialBindingFixtureStatus::DuplicateMaterialId) {
        return Fail("material binding fixture accepted duplicate material id");
    }

    if (!MaterialPassFieldsMatch(before, second_pass_request)) {
        return Fail("duplicate material id mutated pass request");
    }

    const auto snapshot = fixture.Snapshot();
    if (snapshot.binding_record_count != 1U || snapshot.duplicate_material_id_count != 1U) {
        return Fail("duplicate material id counters were not updated");
    }

    return 0;
}

int MaterialBindingFixtureRejectsCapacityOverflowWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest first_pass_request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, first_pass_request, capture)) {
        return Fail("fixture setup failed");
    }

    const std::array<std::uint8_t, 4U> constants = MaterialConstantBytes();
    const std::span<const std::uint8_t> constant_span(constants.data(), constants.size());
    MaterialBindingFixtureRequest first_request = MaterialRequestFrom(first_pass_request, constant_span);
    MaterialBindingFixtureDesc desc{};
    desc.binding_record_capacity = 1U;
    MaterialBindingFixture fixture(desc);
    if (fixture.Bind(first_request, &first_pass_request).status != MaterialBindingFixtureStatus::Success) {
        return Fail("first capacity material binding fixture bind failed");
    }

    RenderFixturePassRequest second_pass_request{};
    if (!FillValidFixture(device, second_pass_request, capture)) {
        return Fail("second fixture setup failed");
    }

    MaterialBindingFixtureRequest second_request = MaterialRequestFrom(second_pass_request, constant_span);
    second_request.material_id = NEXT_MATERIAL_ID;
    const RenderFixturePassRequest before = second_pass_request;
    const auto result = fixture.Bind(second_request, &second_pass_request);
    if (result.status != MaterialBindingFixtureStatus::BindingCapacityExceeded) {
        return Fail("material binding fixture accepted capacity overflow");
    }

    if (!MaterialPassFieldsMatch(before, second_pass_request)) {
        return Fail("capacity overflow mutated pass request");
    }

    const auto snapshot = fixture.Snapshot();
    if (snapshot.binding_record_count != 1U || snapshot.binding_capacity_rejected_count != 1U) {
        return Fail("capacity overflow counters were not updated");
    }

    return 0;
}

int MaterialBindingFixturePropagatesRenderFixturePassFailure() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest pass_request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, pass_request, capture)) {
        return Fail("fixture setup failed");
    }

    pass_request.target = RhiTextureHandle{};
    const std::array<std::uint8_t, 4U> constants = MaterialConstantBytes();
    const std::span<const std::uint8_t> constant_span(constants.data(), constants.size());
    MaterialBindingFixtureRequest material_request = MaterialRequestFrom(pass_request, constant_span);
    MaterialBindingFixture fixture;
    RenderFixturePass pass;
    const auto result = fixture.BindAndExecute(material_request, &pass, &pass_request);
    if (result.status != MaterialBindingFixtureStatus::RenderFixturePassFailed) {
        return Fail("material binding fixture did not propagate fixture pass failure");
    }

    if (result.pass_status != RenderFixturePassStatus::InvalidTarget) {
        return Fail("material binding fixture reported unexpected pass failure status");
    }

    const auto snapshot = fixture.Snapshot();
    if (snapshot.render_pass_failure_count != 1U || snapshot.executed_pass_count != 1U) {
        return Fail("material binding fixture did not track pass failure counters");
    }

    return 0;
}

int MaterialBindingFixtureSnapshotTracksBoundedCounters() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest pass_request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, pass_request, capture)) {
        return Fail("fixture setup failed");
    }

    const std::array<std::uint8_t, 4U> constants = MaterialConstantBytes();
    const std::span<const std::uint8_t> constant_span(constants.data(), constants.size());
    MaterialBindingFixtureRequest material_request = MaterialRequestFrom(pass_request, constant_span);
    MaterialBindingFixture fixture;
    RenderFixturePass pass;
    const auto result = fixture.BindAndExecute(material_request, &pass, &pass_request);
    if (result.status != MaterialBindingFixtureStatus::Success) {
        return Fail("material binding fixture did not execute fixture pass");
    }

    if (result.pass_status != RenderFixturePassStatus::Success || result.rhi_status != RhiStatus::Success) {
        return Fail("material binding fixture did not propagate pass success");
    }

    const auto snapshot = fixture.Snapshot();
    if (snapshot.accepted_binding_count != 1U || snapshot.binding_record_count != 1U) {
        return Fail("material binding fixture snapshot missed binding counters");
    }

    if (snapshot.executed_pass_count != 1U || snapshot.completed_pass_count != 1U) {
        return Fail("material binding fixture snapshot missed pass counters");
    }

    if (snapshot.last_material_id != MATERIAL_ID || snapshot.last_pass_id != MATERIAL_PASS_ID) {
        return Fail("material binding fixture snapshot missed last ids");
    }

    return 0;
}

int RenderCoreSubmissionBatchExecutesMaterialPreparedRequests() {
    NullRhiDevice device = CreateInitializedDevice();
    MaterialBindingFixture material_fixture;
    std::array<RenderFixturePassRequest, 2U> requests{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        requests[0U],
        first_capture,
        MATERIAL_ID,
        MATERIAL_PASS_ID)) {
        return Fail("first submission batch fixture setup failed");
    }

    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        requests[1U],
        second_capture,
        NEXT_MATERIAL_ID,
        MATERIAL_PASS_ID + 1U)) {
        return Fail("second submission batch fixture setup failed");
    }

    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    std::array<RenderFixturePassResult, 2U> results{};
    const std::span<const RenderFixturePassRequest> request_span(requests.data(), requests.size());
    const std::span<RenderFixturePassResult> result_span(results.data(), results.size());
    const RenderSubmissionBatchFixtureRequest batch_request = BatchRequestFrom(pass, request_span, result_span);
    const auto result = batch.Execute(batch_request);
    if (result.status != RenderSubmissionBatchFixtureStatus::Success) {
        return Fail("submission batch fixture did not succeed");
    }

    if (result.completed_entry_count != 2U) {
        return Fail("submission batch fixture did not complete both entries");
    }

    if (results[0U].status != RenderFixturePassStatus::Success) {
        return Fail("first submission batch pass did not succeed");
    }

    if (results[1U].status != RenderFixturePassStatus::Success) {
        return Fail("second submission batch pass did not succeed");
    }

    if (results[0U].pass_id != MATERIAL_PASS_ID || results[1U].pass_id != MATERIAL_PASS_ID + 1U) {
        return Fail("submission batch fixture wrote unexpected pass ids");
    }

    const auto snapshot = batch.Snapshot();
    if (snapshot.accepted_entry_count != 2U || snapshot.completed_entry_count != 2U) {
        return Fail("submission batch fixture did not track completed entries");
    }

    const RhiDeviceSnapshot rhi_snapshot = device.Snapshot();
    if (rhi_snapshot.submit_count != 2U || rhi_snapshot.present_count != 2U || rhi_snapshot.capture_count != 2U) {
        return Fail("submission batch fixture did not submit present and capture twice");
    }

    return 0;
}

int RenderCoreSubmissionBatchRejectsEmptyBatchWithoutMutation() {
    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    const RenderFixturePassResult sentinel = SentinelPassResult();
    std::array<RenderFixturePassResult, 2U> results{};
    results.fill(sentinel);
    const std::span<const RenderFixturePassRequest> request_span{};
    const std::span<RenderFixturePassResult> result_span(results.data(), results.size());
    const RenderSubmissionBatchFixtureRequest batch_request = BatchRequestFrom(pass, request_span, result_span);
    const auto result = batch.Execute(batch_request);
    if (result.status != RenderSubmissionBatchFixtureStatus::EmptyBatch) {
        return Fail("submission batch fixture accepted empty batch");
    }

    if (!PassResultsUnchanged(results, sentinel)) {
        return Fail("empty submission batch mutated result storage");
    }

    const auto pass_snapshot = pass.Snapshot();
    if (pass_snapshot.executed_pass_count != 0U || pass_snapshot.failed_validation_count != 0U) {
        return Fail("empty submission batch mutated fixture pass");
    }

    const auto batch_snapshot = batch.Snapshot();
    if (batch_snapshot.failed_validation_count != 1U || batch_snapshot.submission_record_count != 0U) {
        return Fail("empty submission batch counters were not updated");
    }

    return 0;
}

int RenderCoreSubmissionBatchRejectsNullPassWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    std::array<RenderFixturePassRequest, 1U> requests{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, requests[0U], capture)) {
        return Fail("submission batch fixture setup failed");
    }

    RenderSubmissionBatchFixture batch;
    const RenderFixturePassResult sentinel = SentinelPassResult();
    std::array<RenderFixturePassResult, 2U> results{};
    results.fill(sentinel);
    const RhiDeviceSnapshot before = device.Snapshot();
    const std::span<const RenderFixturePassRequest> request_span(requests.data(), requests.size());
    const std::span<RenderFixturePassResult> result_span(results.data(), results.size());
    const RenderSubmissionBatchFixtureRequest batch_request = NullPassBatchRequestFrom(request_span, result_span);
    const auto result = batch.Execute(batch_request);
    if (result.status != RenderSubmissionBatchFixtureStatus::InvalidArgument) {
        return Fail("submission batch fixture accepted null pass");
    }

    if (!PassResultsUnchanged(results, sentinel)) {
        return Fail("null pass submission batch mutated result storage");
    }

    const RhiDeviceSnapshot after = device.Snapshot();
    if (!WorkCountersMatch(before, after)) {
        return Fail("null pass submission batch mutated RHI work counters");
    }

    return 0;
}

int RenderCoreSubmissionBatchRejectsInvalidPassRequestWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    std::array<RenderFixturePassRequest, 1U> requests{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidFixture(device, requests[0U], capture)) {
        return Fail("submission batch fixture setup failed");
    }

    requests[0U].target = RhiTextureHandle{};
    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    const RenderFixturePassResult sentinel = SentinelPassResult();
    std::array<RenderFixturePassResult, 2U> results{};
    results.fill(sentinel);
    const RhiDeviceSnapshot before = device.Snapshot();
    const std::span<const RenderFixturePassRequest> request_span(requests.data(), requests.size());
    const std::span<RenderFixturePassResult> result_span(results.data(), results.size());
    const RenderSubmissionBatchFixtureRequest batch_request = BatchRequestFrom(pass, request_span, result_span);
    const auto result = batch.Execute(batch_request);
    if (result.status != RenderSubmissionBatchFixtureStatus::InvalidPassRequest) {
        return Fail("submission batch fixture accepted invalid pass request");
    }

    if (result.pass_status != RenderFixturePassStatus::InvalidTarget) {
        return Fail("submission batch fixture did not propagate invalid pass request status");
    }

    if (!PassResultsUnchanged(results, sentinel)) {
        return Fail("invalid pass request mutated result storage");
    }

    const RhiDeviceSnapshot after = device.Snapshot();
    if (!WorkCountersMatch(before, after)) {
        return Fail("invalid pass request mutated RHI work counters");
    }

    const auto pass_snapshot = pass.Snapshot();
    if (pass_snapshot.executed_pass_count != 0U || pass_snapshot.failed_validation_count != 0U) {
        return Fail("invalid pass request mutated fixture pass");
    }

    return 0;
}

int RenderCoreSubmissionBatchRejectsDuplicatePassIdWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    MaterialBindingFixture material_fixture;
    std::array<RenderFixturePassRequest, 2U> requests{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        requests[0U],
        first_capture,
        MATERIAL_ID,
        MATERIAL_PASS_ID)) {
        return Fail("first duplicate submission batch setup failed");
    }

    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        requests[1U],
        second_capture,
        NEXT_MATERIAL_ID,
        MATERIAL_PASS_ID)) {
        return Fail("second duplicate submission batch setup failed");
    }

    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    const RenderFixturePassResult sentinel = SentinelPassResult();
    std::array<RenderFixturePassResult, 2U> results{};
    results.fill(sentinel);
    const RhiDeviceSnapshot before = device.Snapshot();
    const std::span<const RenderFixturePassRequest> request_span(requests.data(), requests.size());
    const std::span<RenderFixturePassResult> result_span(results.data(), results.size());
    const RenderSubmissionBatchFixtureRequest batch_request = BatchRequestFrom(pass, request_span, result_span);
    const auto result = batch.Execute(batch_request);
    if (result.status != RenderSubmissionBatchFixtureStatus::DuplicatePassId) {
        return Fail("submission batch fixture accepted duplicate pass id");
    }

    if (!PassResultsUnchanged(results, sentinel)) {
        return Fail("duplicate pass id mutated result storage");
    }

    const RhiDeviceSnapshot after = device.Snapshot();
    if (!WorkCountersMatch(before, after)) {
        return Fail("duplicate pass id mutated RHI work counters");
    }

    const auto snapshot = batch.Snapshot();
    if (snapshot.duplicate_pass_id_count != 1U || snapshot.submission_record_count != 0U) {
        return Fail("duplicate pass id counters were not updated");
    }

    return 0;
}

int RenderCoreSubmissionBatchRejectsBatchCapacityWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    MaterialBindingFixture material_fixture;
    std::array<RenderFixturePassRequest, 2U> requests{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        requests[0U],
        first_capture,
        MATERIAL_ID,
        MATERIAL_PASS_ID)) {
        return Fail("first capacity submission batch setup failed");
    }

    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        requests[1U],
        second_capture,
        NEXT_MATERIAL_ID,
        MATERIAL_PASS_ID + 1U)) {
        return Fail("second capacity submission batch setup failed");
    }

    RenderFixturePass pass;
    RenderSubmissionBatchFixtureDesc desc{};
    desc.submission_record_capacity = 1U;
    RenderSubmissionBatchFixture batch(desc);
    const RenderFixturePassResult sentinel = SentinelPassResult();
    std::array<RenderFixturePassResult, 2U> results{};
    results.fill(sentinel);
    const RhiDeviceSnapshot before = device.Snapshot();
    const std::span<const RenderFixturePassRequest> request_span(requests.data(), requests.size());
    const std::span<RenderFixturePassResult> result_span(results.data(), results.size());
    const RenderSubmissionBatchFixtureRequest batch_request = BatchRequestFrom(pass, request_span, result_span);
    const auto result = batch.Execute(batch_request);
    if (result.status != RenderSubmissionBatchFixtureStatus::BatchCapacityExceeded) {
        return Fail("submission batch fixture accepted capacity overflow");
    }

    if (!PassResultsUnchanged(results, sentinel)) {
        return Fail("submission batch capacity overflow mutated result storage");
    }

    const RhiDeviceSnapshot after = device.Snapshot();
    if (!WorkCountersMatch(before, after)) {
        return Fail("submission batch capacity overflow mutated RHI work counters");
    }

    const auto snapshot = batch.Snapshot();
    if (snapshot.batch_capacity_rejected_count != 1U || snapshot.submission_record_count != 0U) {
        return Fail("submission batch capacity counters were not updated");
    }

    return 0;
}

int RenderCoreSubmissionBatchPropagatesRenderFixturePassFailure() {
    NullRhiDevice device = CreateInitializedDevice();
    MaterialBindingFixture material_fixture;
    std::array<RenderFixturePassRequest, 1U> requests{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        requests[0U],
        capture,
        MATERIAL_ID,
        MATERIAL_PASS_ID)) {
        return Fail("submission batch pass failure setup failed");
    }

    requests[0U].target = RhiTextureHandle{999U, 1U};
    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    std::array<RenderFixturePassResult, 1U> results{};
    const std::span<const RenderFixturePassRequest> request_span(requests.data(), requests.size());
    const std::span<RenderFixturePassResult> result_span(results.data(), results.size());
    const RenderSubmissionBatchFixtureRequest batch_request = BatchRequestFrom(pass, request_span, result_span);
    const auto result = batch.Execute(batch_request);
    if (result.status != RenderSubmissionBatchFixtureStatus::RenderFixturePassFailed) {
        return Fail("submission batch fixture did not propagate pass failure");
    }

    if (result.pass_status != RenderFixturePassStatus::RhiFailure || result.rhi_status != RhiStatus::InvalidHandle) {
        return Fail("submission batch fixture reported unexpected pass failure");
    }

    if (results[0U].status != RenderFixturePassStatus::RhiFailure) {
        return Fail("submission batch fixture did not write failing pass result");
    }

    if (!CaptureUnchanged(capture)) {
        return Fail("submission batch pass failure wrote capture output");
    }

    const auto snapshot = batch.Snapshot();
    if (snapshot.render_pass_failure_count != 1U || snapshot.submission_record_count != 1U) {
        return Fail("submission batch pass failure counters were not updated");
    }

    return 0;
}

int RenderCoreSubmissionBatchSnapshotTracksBoundedCounters() {
    NullRhiDevice device = CreateInitializedDevice();
    MaterialBindingFixture material_fixture;
    std::array<RenderFixturePassRequest, 2U> requests{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        requests[0U],
        first_capture,
        MATERIAL_ID,
        MATERIAL_PASS_ID)) {
        return Fail("first snapshot submission batch setup failed");
    }

    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        requests[1U],
        second_capture,
        NEXT_MATERIAL_ID,
        MATERIAL_PASS_ID + 1U)) {
        return Fail("second snapshot submission batch setup failed");
    }

    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    const auto before = batch.Snapshot();
    if (before.submission_record_capacity == 0U) {
        return Fail("submission batch fixture did not track initial capacity");
    }

    std::array<RenderFixturePassResult, 2U> results{};
    const std::span<const RenderFixturePassRequest> request_span(requests.data(), requests.size());
    const std::span<RenderFixturePassResult> result_span(results.data(), results.size());
    const RenderSubmissionBatchFixtureRequest batch_request = BatchRequestFrom(pass, request_span, result_span);
    const auto result = batch.Execute(batch_request);
    if (result.status != RenderSubmissionBatchFixtureStatus::Success) {
        return Fail("submission batch fixture snapshot setup did not succeed");
    }

    const auto after = batch.Snapshot();
    if (after.accepted_entry_count != 2U || after.executed_entry_count != 2U) {
        return Fail("submission batch fixture snapshot missed accepted entries");
    }

    if (after.last_entry_index != 1U || after.last_pass_id != MATERIAL_PASS_ID + 1U) {
        return Fail("submission batch fixture snapshot missed last entry");
    }

    if (after.last_material_id != NEXT_MATERIAL_ID) {
        return Fail("submission batch fixture snapshot missed last material id");
    }

    if (after.last_pass_status != RenderFixturePassStatus::Success || after.last_rhi_status != RhiStatus::Success) {
        return Fail("submission batch fixture snapshot missed last pass status");
    }

    if (after.last_recorded_command_count != RENDER_FIXTURE_PASS_COMMAND_COUNT) {
        return Fail("submission batch fixture snapshot missed command count");
    }

    return 0;
}

int RenderCoreFramePacketExecutesPreparedSubmissionBatch() {
    NullRhiDevice device = CreateInitializedDevice();
    MaterialBindingFixture material_fixture;
    std::array<RenderFixturePassRequest, 2U> requests{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        requests[0U],
        first_capture,
        MATERIAL_ID,
        MATERIAL_PASS_ID)) {
        return Fail("first frame packet setup failed");
    }

    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        requests[1U],
        second_capture,
        NEXT_MATERIAL_ID,
        MATERIAL_PASS_ID + 1U)) {
        return Fail("second frame packet setup failed");
    }

    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    RenderFramePacketFixture frame_packet;
    std::array<RenderFixturePassResult, 2U> results{};
    const std::span<const RenderFixturePassRequest> request_span(requests.data(), requests.size());
    const std::span<RenderFixturePassResult> result_span(results.data(), results.size());
    const RenderSubmissionBatchFixtureRequest batch_request = BatchRequestFrom(pass, request_span, result_span);
    const RenderFramePacketFixtureRequest frame_request = FramePacketRequestFrom(FRAME_ID, batch, batch_request);
    const auto result = frame_packet.Execute(frame_request);
    if (result.status != RenderFramePacketFixtureStatus::Success) {
        return Fail("frame packet fixture did not succeed");
    }

    if (result.batch_status != RenderSubmissionBatchFixtureStatus::Success) {
        return Fail("frame packet fixture did not report batch success");
    }

    if (result.frame_id != FRAME_ID || result.completed_entry_count != 2U || result.failed_entry_count != 0U) {
        return Fail("frame packet fixture returned unexpected result counters");
    }

    if (result.pass_id != MATERIAL_PASS_ID + 1U || result.material_id != NEXT_MATERIAL_ID) {
        return Fail("frame packet fixture missed last pass or material id");
    }

    if (results[0U].status != RenderFixturePassStatus::Success || results[1U].status != RenderFixturePassStatus::Success) {
        return Fail("frame packet fixture did not write successful pass results");
    }

    const auto snapshot = frame_packet.Snapshot();
    if (snapshot.accepted_packet_count != 1U || snapshot.completed_packet_count != 1U) {
        return Fail("frame packet fixture did not track completed packet");
    }

    if (snapshot.completed_entry_count != 2U || snapshot.frame_packet_record_count != 1U) {
        return Fail("frame packet fixture did not track completed entries");
    }

    const RhiDeviceSnapshot rhi_snapshot = device.Snapshot();
    if (rhi_snapshot.submit_count != 2U || rhi_snapshot.present_count != 2U || rhi_snapshot.capture_count != 2U) {
        return Fail("frame packet fixture did not execute batch work");
    }

    return 0;
}

int RenderCoreFramePacketRejectsZeroFrameIdWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    MaterialBindingFixture material_fixture;
    std::array<RenderFixturePassRequest, 1U> requests{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        requests[0U],
        capture,
        MATERIAL_ID,
        MATERIAL_PASS_ID)) {
        return Fail("zero frame packet setup failed");
    }

    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    RenderFramePacketFixture frame_packet;
    const RenderFixturePassResult sentinel = SentinelPassResult();
    std::array<RenderFixturePassResult, 2U> results{};
    results.fill(sentinel);
    const RhiDeviceSnapshot before = device.Snapshot();
    const std::span<const RenderFixturePassRequest> request_span(requests.data(), requests.size());
    const std::span<RenderFixturePassResult> result_span(results.data(), results.size());
    const RenderSubmissionBatchFixtureRequest batch_request = BatchRequestFrom(pass, request_span, result_span);
    const RenderFramePacketFixtureRequest frame_request = FramePacketRequestFrom(0U, batch, batch_request);
    const auto result = frame_packet.Execute(frame_request);
    if (result.status != RenderFramePacketFixtureStatus::InvalidFrameId) {
        return Fail("frame packet fixture accepted zero frame id");
    }

    if (!PassResultsUnchanged(results, sentinel)) {
        return Fail("zero frame id mutated result storage");
    }

    const RhiDeviceSnapshot after = device.Snapshot();
    if (!WorkCountersMatch(before, after)) {
        return Fail("zero frame id mutated RHI work counters");
    }

    const auto batch_snapshot = batch.Snapshot();
    if (batch_snapshot.submission_record_count != 0U || batch_snapshot.failed_validation_count != 0U) {
        return Fail("zero frame id mutated submission batch state");
    }

    const auto pass_snapshot = pass.Snapshot();
    if (pass_snapshot.executed_pass_count != 0U || pass_snapshot.failed_validation_count != 0U) {
        return Fail("zero frame id mutated fixture pass state");
    }

    const auto frame_snapshot = frame_packet.Snapshot();
    if (frame_snapshot.failed_validation_count != 1U || frame_snapshot.frame_packet_record_count != 0U) {
        return Fail("zero frame id counters were not updated");
    }

    return 0;
}

int RenderCoreFramePacketRejectsNullExecutorWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    MaterialBindingFixture material_fixture;
    std::array<RenderFixturePassRequest, 1U> requests{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        requests[0U],
        capture,
        MATERIAL_ID,
        MATERIAL_PASS_ID)) {
        return Fail("null executor frame packet setup failed");
    }

    RenderFixturePass pass;
    RenderFramePacketFixture frame_packet;
    const RenderFixturePassResult sentinel = SentinelPassResult();
    std::array<RenderFixturePassResult, 2U> results{};
    results.fill(sentinel);
    const RhiDeviceSnapshot before = device.Snapshot();
    const std::span<const RenderFixturePassRequest> request_span(requests.data(), requests.size());
    const std::span<RenderFixturePassResult> result_span(results.data(), results.size());
    const RenderSubmissionBatchFixtureRequest batch_request = BatchRequestFrom(pass, request_span, result_span);
    const RenderFramePacketFixtureRequest frame_request = NullFramePacketExecutorRequestFrom(FRAME_ID, batch_request);
    const auto result = frame_packet.Execute(frame_request);
    if (result.status != RenderFramePacketFixtureStatus::InvalidArgument) {
        return Fail("frame packet fixture accepted null executor");
    }

    if (!PassResultsUnchanged(results, sentinel)) {
        return Fail("null frame packet executor mutated result storage");
    }

    const RhiDeviceSnapshot after = device.Snapshot();
    if (!WorkCountersMatch(before, after)) {
        return Fail("null frame packet executor mutated RHI work counters");
    }

    const auto frame_snapshot = frame_packet.Snapshot();
    if (frame_snapshot.failed_validation_count != 1U || frame_snapshot.frame_packet_record_count != 0U) {
        return Fail("null frame packet executor counters were not updated");
    }

    return 0;
}

int RenderCoreFramePacketRejectsNullBatchRequestWithoutMutation() {
    RenderSubmissionBatchFixture batch;
    RenderFramePacketFixture frame_packet;
    const auto before = batch.Snapshot();
    const RenderFramePacketFixtureRequest frame_request = NullBatchRequestFramePacketRequestFrom(FRAME_ID, batch);
    const auto result = frame_packet.Execute(frame_request);
    if (result.status != RenderFramePacketFixtureStatus::InvalidBatchRequest) {
        return Fail("frame packet fixture accepted null batch request");
    }

    if (result.batch_status != RenderSubmissionBatchFixtureStatus::InvalidArgument) {
        return Fail("frame packet fixture did not report null batch request status");
    }

    const auto after = batch.Snapshot();
    if (after.submission_record_count != before.submission_record_count) {
        return Fail("null batch request mutated submission batch records");
    }

    const auto frame_snapshot = frame_packet.Snapshot();
    if (frame_snapshot.failed_validation_count != 1U || frame_snapshot.frame_packet_record_count != 0U) {
        return Fail("null batch request counters were not updated");
    }

    return 0;
}

int RenderCoreFramePacketRejectsInvalidBatchRequestWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    MaterialBindingFixture material_fixture;
    std::array<RenderFixturePassRequest, 1U> requests{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        requests[0U],
        capture,
        MATERIAL_ID,
        MATERIAL_PASS_ID)) {
        return Fail("invalid batch frame packet setup failed");
    }

    requests[0U].target = RhiTextureHandle{};
    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    RenderFramePacketFixture frame_packet;
    const RenderFixturePassResult sentinel = SentinelPassResult();
    std::array<RenderFixturePassResult, 2U> results{};
    results.fill(sentinel);
    const RhiDeviceSnapshot before = device.Snapshot();
    const std::span<const RenderFixturePassRequest> request_span(requests.data(), requests.size());
    const std::span<RenderFixturePassResult> result_span(results.data(), results.size());
    const RenderSubmissionBatchFixtureRequest batch_request = BatchRequestFrom(pass, request_span, result_span);
    const RenderFramePacketFixtureRequest frame_request = FramePacketRequestFrom(FRAME_ID, batch, batch_request);
    const auto result = frame_packet.Execute(frame_request);
    if (result.status != RenderFramePacketFixtureStatus::InvalidBatchRequest) {
        return Fail("frame packet fixture accepted invalid batch request");
    }

    if (result.batch_status != RenderSubmissionBatchFixtureStatus::InvalidPassRequest) {
        return Fail("frame packet fixture did not report invalid batch status");
    }

    if (result.pass_status != RenderFixturePassStatus::InvalidTarget) {
        return Fail("frame packet fixture did not propagate invalid pass status");
    }

    if (!PassResultsUnchanged(results, sentinel)) {
        return Fail("invalid frame packet batch mutated result storage");
    }

    const RhiDeviceSnapshot after = device.Snapshot();
    if (!WorkCountersMatch(before, after)) {
        return Fail("invalid frame packet batch mutated RHI work counters");
    }

    const auto batch_snapshot = batch.Snapshot();
    if (batch_snapshot.submission_record_count != 0U || batch_snapshot.failed_validation_count != 0U) {
        return Fail("invalid frame packet batch mutated submission batch state");
    }

    const auto pass_snapshot = pass.Snapshot();
    if (pass_snapshot.executed_pass_count != 0U || pass_snapshot.failed_validation_count != 0U) {
        return Fail("invalid frame packet batch mutated fixture pass state");
    }

    return 0;
}

int RenderCoreFramePacketRejectsDuplicateFrameIdWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    MaterialBindingFixture material_fixture;
    std::array<RenderFixturePassRequest, 1U> first_requests{};
    std::array<RenderFixturePassRequest, 1U> second_requests{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        first_requests[0U],
        first_capture,
        MATERIAL_ID,
        MATERIAL_PASS_ID)) {
        return Fail("first duplicate frame packet setup failed");
    }

    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        second_requests[0U],
        second_capture,
        NEXT_MATERIAL_ID,
        MATERIAL_PASS_ID + 1U)) {
        return Fail("second duplicate frame packet setup failed");
    }

    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    RenderFramePacketFixture frame_packet;
    std::array<RenderFixturePassResult, 2U> first_results{};
    const std::span<const RenderFixturePassRequest> first_request_span(first_requests.data(), first_requests.size());
    const std::span<RenderFixturePassResult> first_result_span(first_results.data(), first_results.size());
    const RenderSubmissionBatchFixtureRequest first_batch_request = BatchRequestFrom(pass, first_request_span, first_result_span);
    const RenderFramePacketFixtureRequest first_frame_request = FramePacketRequestFrom(FRAME_ID, batch, first_batch_request);
    if (frame_packet.Execute(first_frame_request).status != RenderFramePacketFixtureStatus::Success) {
        return Fail("duplicate frame packet setup execution failed");
    }

    const RenderFixturePassResult sentinel = SentinelPassResult();
    std::array<RenderFixturePassResult, 2U> second_results{};
    second_results.fill(sentinel);
    const RhiDeviceSnapshot before = device.Snapshot();
    const auto batch_before = batch.Snapshot();
    const std::span<const RenderFixturePassRequest> second_request_span(second_requests.data(), second_requests.size());
    const std::span<RenderFixturePassResult> second_result_span(second_results.data(), second_results.size());
    const RenderSubmissionBatchFixtureRequest second_batch_request = BatchRequestFrom(pass, second_request_span, second_result_span);
    const RenderFramePacketFixtureRequest second_frame_request = FramePacketRequestFrom(FRAME_ID, batch, second_batch_request);
    const auto result = frame_packet.Execute(second_frame_request);
    if (result.status != RenderFramePacketFixtureStatus::DuplicateFrameId) {
        return Fail("frame packet fixture accepted duplicate frame id");
    }

    if (!PassResultsUnchanged(second_results, sentinel)) {
        return Fail("duplicate frame id mutated result storage");
    }

    const RhiDeviceSnapshot after = device.Snapshot();
    if (!WorkCountersMatch(before, after)) {
        return Fail("duplicate frame id mutated RHI work counters");
    }

    const auto batch_after = batch.Snapshot();
    if (batch_after.submission_record_count != batch_before.submission_record_count) {
        return Fail("duplicate frame id mutated submission batch records");
    }

    const auto frame_snapshot = frame_packet.Snapshot();
    if (frame_snapshot.duplicate_frame_id_count != 1U || frame_snapshot.frame_packet_record_count != 1U) {
        return Fail("duplicate frame id counters were not updated");
    }

    return 0;
}

int RenderCoreFramePacketRejectsPacketCapacityWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    MaterialBindingFixture material_fixture;
    std::array<RenderFixturePassRequest, 1U> first_requests{};
    std::array<RenderFixturePassRequest, 1U> second_requests{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        first_requests[0U],
        first_capture,
        MATERIAL_ID,
        MATERIAL_PASS_ID)) {
        return Fail("first capacity frame packet setup failed");
    }

    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        second_requests[0U],
        second_capture,
        NEXT_MATERIAL_ID,
        MATERIAL_PASS_ID + 1U)) {
        return Fail("second capacity frame packet setup failed");
    }

    RenderFramePacketFixtureDesc desc{};
    desc.frame_packet_record_capacity = 1U;
    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    RenderFramePacketFixture frame_packet(desc);
    std::array<RenderFixturePassResult, 2U> first_results{};
    const std::span<const RenderFixturePassRequest> first_request_span(first_requests.data(), first_requests.size());
    const std::span<RenderFixturePassResult> first_result_span(first_results.data(), first_results.size());
    const RenderSubmissionBatchFixtureRequest first_batch_request = BatchRequestFrom(pass, first_request_span, first_result_span);
    const RenderFramePacketFixtureRequest first_frame_request = FramePacketRequestFrom(FRAME_ID, batch, first_batch_request);
    if (frame_packet.Execute(first_frame_request).status != RenderFramePacketFixtureStatus::Success) {
        return Fail("capacity frame packet setup execution failed");
    }

    const RenderFixturePassResult sentinel = SentinelPassResult();
    std::array<RenderFixturePassResult, 2U> second_results{};
    second_results.fill(sentinel);
    const RhiDeviceSnapshot before = device.Snapshot();
    const auto batch_before = batch.Snapshot();
    const std::span<const RenderFixturePassRequest> second_request_span(second_requests.data(), second_requests.size());
    const std::span<RenderFixturePassResult> second_result_span(second_results.data(), second_results.size());
    const RenderSubmissionBatchFixtureRequest second_batch_request = BatchRequestFrom(pass, second_request_span, second_result_span);
    const RenderFramePacketFixtureRequest second_frame_request = FramePacketRequestFrom(NEXT_FRAME_ID, batch, second_batch_request);
    const auto result = frame_packet.Execute(second_frame_request);
    if (result.status != RenderFramePacketFixtureStatus::PacketCapacityExceeded) {
        return Fail("frame packet fixture accepted packet capacity overflow");
    }

    if (!PassResultsUnchanged(second_results, sentinel)) {
        return Fail("frame packet capacity overflow mutated result storage");
    }

    const RhiDeviceSnapshot after = device.Snapshot();
    if (!WorkCountersMatch(before, after)) {
        return Fail("frame packet capacity overflow mutated RHI work counters");
    }

    const auto batch_after = batch.Snapshot();
    if (batch_after.submission_record_count != batch_before.submission_record_count) {
        return Fail("frame packet capacity overflow mutated submission batch records");
    }

    const auto frame_snapshot = frame_packet.Snapshot();
    if (frame_snapshot.packet_capacity_rejected_count != 1U || frame_snapshot.frame_packet_record_count != 1U) {
        return Fail("frame packet capacity counters were not updated");
    }

    return 0;
}

int RenderCoreFramePacketPropagatesSubmissionBatchFailure() {
    NullRhiDevice device = CreateInitializedDevice();
    MaterialBindingFixture material_fixture;
    std::array<RenderFixturePassRequest, 1U> requests{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        requests[0U],
        capture,
        MATERIAL_ID,
        MATERIAL_PASS_ID)) {
        return Fail("frame packet batch failure setup failed");
    }

    requests[0U].target = RhiTextureHandle{999U, 1U};
    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    RenderFramePacketFixture frame_packet;
    std::array<RenderFixturePassResult, 2U> results{};
    const std::span<const RenderFixturePassRequest> request_span(requests.data(), requests.size());
    const std::span<RenderFixturePassResult> result_span(results.data(), results.size());
    const RenderSubmissionBatchFixtureRequest batch_request = BatchRequestFrom(pass, request_span, result_span);
    const RenderFramePacketFixtureRequest frame_request = FramePacketRequestFrom(FRAME_ID, batch, batch_request);
    const auto result = frame_packet.Execute(frame_request);
    if (result.status != RenderFramePacketFixtureStatus::SubmissionBatchFailed) {
        return Fail("frame packet fixture did not propagate batch failure");
    }

    if (result.batch_status != RenderSubmissionBatchFixtureStatus::RenderFixturePassFailed) {
        return Fail("frame packet fixture reported unexpected batch failure status");
    }

    if (result.pass_status != RenderFixturePassStatus::RhiFailure || result.rhi_status != RhiStatus::InvalidHandle) {
        return Fail("frame packet fixture reported unexpected pass failure status");
    }

    if (results[0U].status != RenderFixturePassStatus::RhiFailure) {
        return Fail("frame packet fixture did not write failing pass result");
    }

    if (!CaptureUnchanged(capture)) {
        return Fail("frame packet batch failure wrote capture output");
    }

    const auto frame_snapshot = frame_packet.Snapshot();
    if (frame_snapshot.submission_batch_failure_count != 1U || frame_snapshot.frame_packet_record_count != 1U) {
        return Fail("frame packet batch failure counters were not updated");
    }

    const auto batch_snapshot = batch.Snapshot();
    if (batch_snapshot.render_pass_failure_count != 1U || batch_snapshot.submission_record_count != 1U) {
        return Fail("submission batch failure counters were not updated");
    }

    return 0;
}

int RenderCoreFramePacketSnapshotTracksBoundedCounters() {
    NullRhiDevice device = CreateInitializedDevice();
    MaterialBindingFixture material_fixture;
    std::array<RenderFixturePassRequest, 1U> first_requests{};
    std::array<RenderFixturePassRequest, 1U> second_requests{};
    std::vector<std::uint8_t> first_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        first_requests[0U],
        first_capture,
        MATERIAL_ID,
        MATERIAL_PASS_ID)) {
        return Fail("first snapshot frame packet setup failed");
    }

    if (!FillMaterialPreparedBatchRequest(
        device,
        material_fixture,
        second_requests[0U],
        second_capture,
        NEXT_MATERIAL_ID,
        MATERIAL_PASS_ID + 1U)) {
        return Fail("second snapshot frame packet setup failed");
    }

    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    RenderFramePacketFixture frame_packet;
    const auto before = frame_packet.Snapshot();
    if (before.frame_packet_record_capacity == 0U) {
        return Fail("frame packet fixture did not track initial capacity");
    }

    std::array<RenderFixturePassResult, 2U> first_results{};
    const std::span<const RenderFixturePassRequest> first_request_span(first_requests.data(), first_requests.size());
    const std::span<RenderFixturePassResult> first_result_span(first_results.data(), first_results.size());
    const RenderSubmissionBatchFixtureRequest first_batch_request = BatchRequestFrom(pass, first_request_span, first_result_span);
    const RenderFramePacketFixtureRequest first_frame_request = FramePacketRequestFrom(FRAME_ID, batch, first_batch_request);
    if (frame_packet.Execute(first_frame_request).status != RenderFramePacketFixtureStatus::Success) {
        return Fail("first snapshot frame packet execution failed");
    }

    std::array<RenderFixturePassResult, 2U> second_results{};
    const std::span<const RenderFixturePassRequest> second_request_span(second_requests.data(), second_requests.size());
    const std::span<RenderFixturePassResult> second_result_span(second_results.data(), second_results.size());
    const RenderSubmissionBatchFixtureRequest second_batch_request = BatchRequestFrom(pass, second_request_span, second_result_span);
    const RenderFramePacketFixtureRequest second_frame_request = FramePacketRequestFrom(NEXT_FRAME_ID, batch, second_batch_request);
    if (frame_packet.Execute(second_frame_request).status != RenderFramePacketFixtureStatus::Success) {
        return Fail("second snapshot frame packet execution failed");
    }

    const auto after = frame_packet.Snapshot();
    if (after.accepted_packet_count != 2U || after.completed_packet_count != 2U) {
        return Fail("frame packet snapshot missed completed packets");
    }

    if (after.completed_entry_count != 2U || after.frame_packet_record_count != 2U) {
        return Fail("frame packet snapshot missed completed entries");
    }

    if (after.last_frame_id != NEXT_FRAME_ID || after.last_pass_id != MATERIAL_PASS_ID + 1U) {
        return Fail("frame packet snapshot missed last frame or pass id");
    }

    if (after.last_material_id != NEXT_MATERIAL_ID) {
        return Fail("frame packet snapshot missed last material id");
    }

    if (after.last_status != RenderFramePacketFixtureStatus::Success) {
        return Fail("frame packet snapshot missed last frame status");
    }

    if (after.last_batch_status != RenderSubmissionBatchFixtureStatus::Success) {
        return Fail("frame packet snapshot missed last batch status");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_EXECUTES_PASS) {
        return RenderCoreFixturePassExecutesSampledIndexedPass();
    }

    if (name == TEST_NULL_RHI) {
        return RenderCoreFixturePassRejectsNullRhiDeviceWithoutMutation();
    }

    if (name == TEST_INVALID_TARGET) {
        return RenderCoreFixturePassRejectsInvalidTargetWithoutMutation();
    }

    if (name == TEST_INVALID_PIPELINE) {
        return RenderCoreFixturePassRejectsInvalidPipelineWithoutMutation();
    }

    if (name == TEST_MISSING_VERTEX) {
        return RenderCoreFixturePassRejectsMissingVertexBufferWithoutMutation();
    }

    if (name == TEST_MISSING_INDEX) {
        return RenderCoreFixturePassRejectsMissingIndexBufferWithoutMutation();
    }

    if (name == TEST_INVALID_TEXTURE) {
        return RenderCoreFixturePassRejectsInvalidTextureBindingWithoutMutation();
    }

    if (name == TEST_INVALID_SAMPLER) {
        return RenderCoreFixturePassRejectsInvalidSamplerBindingWithoutMutation();
    }

    if (name == TEST_CONSTANT_BUFFER_BIND) {
        return RenderCoreFixturePassBindsConstantBuffer();
    }

    if (name == TEST_INVALID_DRAW) {
        return RenderCoreFixturePassRejectsInvalidDrawWithoutMutation();
    }

    if (name == TEST_SMALL_CAPTURE) {
        return RenderCoreFixturePassRejectsSmallCaptureStorageWithoutMutation();
    }

    if (name == TEST_COMMAND_CAPACITY) {
        return RenderCoreFixturePassRejectsCommandCapacityWithoutRhiMutation();
    }

    if (name == TEST_RECORD_CAPACITY) {
        return RenderCoreFixturePassRejectsPassRecordCapacityWithoutRhiMutation();
    }

    if (name == TEST_RHI_FAILURE) {
        return RenderCoreFixturePassTracksRhiFailureWithoutWritingCapture();
    }

    if (name == TEST_SNAPSHOT) {
        return RenderCoreFixturePassSnapshotTracksBoundedCounters();
    }

    if (name == TEST_MATERIAL_BIND) {
        return MaterialBindingFixtureBindsValuesToRenderFixtureRequest();
    }

    if (name == TEST_MATERIAL_INVALID_PIPELINE) {
        return MaterialBindingFixtureRejectsInvalidPipelineWithoutMutation();
    }

    if (name == TEST_MATERIAL_INVALID_TEXTURE) {
        return MaterialBindingFixtureRejectsInvalidTextureBindingWithoutMutation();
    }

    if (name == TEST_MATERIAL_INVALID_SAMPLER) {
        return MaterialBindingFixtureRejectsInvalidSamplerBindingWithoutMutation();
    }

    if (name == TEST_MATERIAL_CONSTANT_BUFFER_BIND) {
        return MaterialBindingFixturePropagatesConstantBufferBindings();
    }

    if (name == TEST_MATERIAL_OVERSIZED_CONSTANTS) {
        return MaterialBindingFixtureRejectsOversizedConstantsWithoutMutation();
    }

    if (name == TEST_MATERIAL_DUPLICATE) {
        return MaterialBindingFixtureRejectsDuplicateMaterialIdWithoutMutation();
    }

    if (name == TEST_MATERIAL_CAPACITY) {
        return MaterialBindingFixtureRejectsCapacityOverflowWithoutMutation();
    }

    if (name == TEST_MATERIAL_PASS_FAILURE) {
        return MaterialBindingFixturePropagatesRenderFixturePassFailure();
    }

    if (name == TEST_MATERIAL_SNAPSHOT) {
        return MaterialBindingFixtureSnapshotTracksBoundedCounters();
    }

    if (name == TEST_BATCH_EXECUTES_MATERIAL) {
        return RenderCoreSubmissionBatchExecutesMaterialPreparedRequests();
    }

    if (name == TEST_BATCH_EMPTY) {
        return RenderCoreSubmissionBatchRejectsEmptyBatchWithoutMutation();
    }

    if (name == TEST_BATCH_NULL_PASS) {
        return RenderCoreSubmissionBatchRejectsNullPassWithoutMutation();
    }

    if (name == TEST_BATCH_INVALID_PASS_REQUEST) {
        return RenderCoreSubmissionBatchRejectsInvalidPassRequestWithoutMutation();
    }

    if (name == TEST_BATCH_DUPLICATE_PASS_ID) {
        return RenderCoreSubmissionBatchRejectsDuplicatePassIdWithoutMutation();
    }

    if (name == TEST_BATCH_CAPACITY) {
        return RenderCoreSubmissionBatchRejectsBatchCapacityWithoutMutation();
    }

    if (name == TEST_BATCH_PASS_FAILURE) {
        return RenderCoreSubmissionBatchPropagatesRenderFixturePassFailure();
    }

    if (name == TEST_BATCH_SNAPSHOT) {
        return RenderCoreSubmissionBatchSnapshotTracksBoundedCounters();
    }

    if (name == TEST_FRAME_PACKET_EXECUTES_BATCH) {
        return RenderCoreFramePacketExecutesPreparedSubmissionBatch();
    }

    if (name == TEST_FRAME_PACKET_ZERO_FRAME) {
        return RenderCoreFramePacketRejectsZeroFrameIdWithoutMutation();
    }

    if (name == TEST_FRAME_PACKET_NULL_EXECUTOR) {
        return RenderCoreFramePacketRejectsNullExecutorWithoutMutation();
    }

    if (name == TEST_FRAME_PACKET_NULL_BATCH_REQUEST) {
        return RenderCoreFramePacketRejectsNullBatchRequestWithoutMutation();
    }

    if (name == TEST_FRAME_PACKET_INVALID_BATCH_REQUEST) {
        return RenderCoreFramePacketRejectsInvalidBatchRequestWithoutMutation();
    }

    if (name == TEST_FRAME_PACKET_DUPLICATE_FRAME) {
        return RenderCoreFramePacketRejectsDuplicateFrameIdWithoutMutation();
    }

    if (name == TEST_FRAME_PACKET_CAPACITY) {
        return RenderCoreFramePacketRejectsPacketCapacityWithoutMutation();
    }

    if (name == TEST_FRAME_PACKET_BATCH_FAILURE) {
        return RenderCoreFramePacketPropagatesSubmissionBatchFailure();
    }

    if (name == TEST_FRAME_PACKET_SNAPSHOT) {
        return RenderCoreFramePacketSnapshotTracksBoundedCounters();
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
