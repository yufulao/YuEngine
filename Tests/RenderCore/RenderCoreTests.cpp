// Module: Tests RenderCore
// File: Tests/RenderCore/RenderCoreTests.cpp

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
using MaterialBindingFixture = yuengine::rendercore::MaterialBindingFixture;
using MaterialBindingFixtureDesc = yuengine::rendercore::MaterialBindingFixtureDesc;
using MaterialBindingFixtureRequest = yuengine::rendercore::MaterialBindingFixtureRequest;
using yuengine::rendercore::MaterialBindingFixtureStatus;
using yuengine::rendercore::MAX_MATERIAL_BINDING_FIXTURE_CONSTANT_BYTES;
using RenderFixturePassDesc = yuengine::rendercore::RenderFixturePassDesc;
using RenderFixturePassRequest = yuengine::rendercore::RenderFixturePassRequest;
using yuengine::rendercore::RenderFixturePassStatus;
using yuengine::rendercore::RENDER_FIXTURE_PASS_COMMAND_COUNT;
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
constexpr const char *TEST_EXECUTES_PASS = "RenderCore_FixturePass_ExecutesSampledIndexedPass";
constexpr const char *TEST_NULL_RHI = "RenderCore_FixturePass_RejectsNullRhiDeviceWithoutMutation";
constexpr const char *TEST_INVALID_TARGET = "RenderCore_FixturePass_RejectsInvalidTargetWithoutMutation";
constexpr const char *TEST_INVALID_PIPELINE = "RenderCore_FixturePass_RejectsInvalidPipelineWithoutMutation";
constexpr const char *TEST_MISSING_VERTEX = "RenderCore_FixturePass_RejectsMissingVertexBufferWithoutMutation";
constexpr const char *TEST_MISSING_INDEX = "RenderCore_FixturePass_RejectsMissingIndexBufferWithoutMutation";
constexpr const char *TEST_INVALID_TEXTURE = "RenderCore_FixturePass_RejectsInvalidTextureBindingWithoutMutation";
constexpr const char *TEST_INVALID_SAMPLER = "RenderCore_FixturePass_RejectsInvalidSamplerBindingWithoutMutation";
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
constexpr const char *TEST_MATERIAL_OVERSIZED_CONSTANTS = "Material_BindingFixture_RejectsOversizedConstantsWithoutMutation";
constexpr const char *TEST_MATERIAL_DUPLICATE = "Material_BindingFixture_RejectsDuplicateMaterialIdWithoutMutation";
constexpr const char *TEST_MATERIAL_CAPACITY = "Material_BindingFixture_RejectsCapacityOverflowWithoutMutation";
constexpr const char *TEST_MATERIAL_PASS_FAILURE = "Material_BindingFixture_PropagatesRenderFixturePassFailure";
constexpr const char *TEST_MATERIAL_SNAPSHOT = "Material_BindingFixture_SnapshotTracksBoundedCounters";
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

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
