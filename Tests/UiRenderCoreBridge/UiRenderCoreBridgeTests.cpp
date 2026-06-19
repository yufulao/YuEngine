// 模块: Tests UiRenderCoreBridge
// 文件: Tests/UiRenderCoreBridge/UiRenderCoreBridgeTests.cpp

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
#include "YuEngine/RenderCore/RenderSubmissionBatchFixture.h"
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
#include "YuEngine/UiCore/UiDrawElement.h"
#include "YuEngine/UiCore/UiDrawElementType.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiRect.h"
#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridge.h"
#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeRequest.h"
#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeStatus.h"

using RenderFixturePass = yuengine::rendercore::RenderFixturePass;
using RenderFixturePassRequest = yuengine::rendercore::RenderFixturePassRequest;
using RenderFixturePassResult = yuengine::rendercore::RenderFixturePassResult;
using RenderSubmissionBatchFixture = yuengine::rendercore::RenderSubmissionBatchFixture;
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
using UiDrawElement = yuengine::uicore::UiDrawElement;
using yuengine::uicore::UiDrawElementType;
using UiNodeId = yuengine::uicore::UiNodeId;
using UiRect = yuengine::uicore::UiRect;
using UiRenderCoreBridge = yuengine::uirendercorebridge::UiRenderCoreBridge;
using UiRenderCoreBridgeRequest = yuengine::uirendercorebridge::UiRenderCoreBridgeRequest;
using yuengine::uirendercorebridge::UiRenderCoreBridgeStatus;

namespace {
constexpr const char *TEST_SUBMITS =
    "UiRenderCoreBridge_SubmitsDrawElementsThroughRenderCore";
constexpr const char *TEST_INVALID_DRAW =
    "UiRenderCoreBridge_RejectsInvalidDrawElementWithoutSubmission";
constexpr const char *TEST_SMALL_STORAGE =
    "UiRenderCoreBridge_RejectsSmallOutputStorageWithoutSubmission";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint8_t SENTINEL_BYTE = 0xA5U;
constexpr std::uint32_t TRIANGLE_VERTEX_COUNT = 3U;
constexpr std::uint32_t TRIANGLE_INDEX_COUNT = 3U;
constexpr std::size_t TRIANGLE_VERTEX_STRIDE_BYTES = sizeof(float) * 6U;
constexpr std::size_t TRIANGLE_VERTEX_BUFFER_BYTES = TRIANGLE_VERTEX_STRIDE_BYTES * TRIANGLE_VERTEX_COUNT;
constexpr std::size_t TRIANGLE_INDEX_BUFFER_BYTES = sizeof(std::uint16_t) * TRIANGLE_INDEX_COUNT;
constexpr std::size_t CAPTURE_BYTES = 16U;
constexpr std::uint32_t FIRST_MATERIAL_ID = 41U;
constexpr std::uint32_t SECOND_MATERIAL_ID = 43U;
constexpr std::uint32_t PASS_ID_BASE = 700U;

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

bool CreateTarget(IRhiDevice &device, RhiTextureHandle *out_handle) {
    if (out_handle == nullptr) {
        return false;
    }

    return device.CreateColorTarget(SmallTargetDesc(), *out_handle) == RhiStatus::Success;
}

bool CreateShaderModule(IRhiDevice &device, RhiShaderStage stage, RhiShaderModuleHandle *out_handle) {
    if (out_handle == nullptr) {
        return false;
    }

    const std::array<std::uint8_t, 4U> bytes = SmallShaderBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const RhiShaderModuleDesc desc{stage, byte_span};
    return device.CreateShaderModule(desc, *out_handle) == RhiStatus::Success;
}

bool CreateTrianglePipeline(IRhiDevice &device, RhiPipelineHandle *out_handle) {
    if (out_handle == nullptr) {
        return false;
    }

    RhiShaderModuleHandle vertex_shader{};
    if (!CreateShaderModule(device, RhiShaderStage::Vertex, &vertex_shader)) {
        return false;
    }

    RhiShaderModuleHandle pixel_shader{};
    if (!CreateShaderModule(device, RhiShaderStage::Pixel, &pixel_shader)) {
        return false;
    }

    RhiPipelineDesc desc{};
    desc.vertex_shader = vertex_shader;
    desc.pixel_shader = pixel_shader;
    desc.input_layout = TriangleInputLayoutDesc();
    return device.CreatePipeline(desc, *out_handle) == RhiStatus::Success;
}

bool CreateTriangleBuffer(IRhiDevice &device, RhiBufferHandle *out_handle) {
    if (out_handle == nullptr) {
        return false;
    }

    const std::span<const std::uint8_t> empty_bytes{};
    return device.CreateBuffer(TriangleVertexBufferDesc(), empty_bytes, *out_handle) == RhiStatus::Success;
}

bool CreateTriangleIndexBuffer(IRhiDevice &device, RhiBufferHandle *out_handle) {
    if (out_handle == nullptr) {
        return false;
    }

    const std::array<std::uint16_t, TRIANGLE_INDEX_COUNT> indices{0U, 1U, 2U};
    const auto *index_byte_pointer = reinterpret_cast<const std::uint8_t *>(indices.data());
    const std::span<const std::uint8_t> index_bytes(index_byte_pointer, TRIANGLE_INDEX_BUFFER_BYTES);
    return device.CreateBuffer(TriangleIndexBufferDesc(), index_bytes, *out_handle) == RhiStatus::Success;
}

bool CreateSampledTexture(IRhiDevice &device, RhiTextureHandle *out_handle) {
    if (out_handle == nullptr) {
        return false;
    }

    const std::array<std::uint8_t, CAPTURE_BYTES> texture_bytes{
        255U, 0U, 0U, 255U,
        0U, 255U, 0U, 255U,
        0U, 0U, 255U, 255U,
        255U, 255U, 255U, 255U};
    const std::span<const std::uint8_t> texture_span(texture_bytes.data(), texture_bytes.size());
    return device.CreateTexture(SmallTextureDesc(), texture_span, *out_handle) == RhiStatus::Success;
}

bool CreateSamplerPrimitive(IRhiDevice &device, RhiSamplerHandle *out_handle) {
    if (out_handle == nullptr) {
        return false;
    }

    RhiSamplerDesc desc{};
    desc.linear_filter = false;
    desc.clamp_to_edge = true;
    return device.CreateSampler(desc, *out_handle) == RhiStatus::Success;
}

bool FillValidTemplateRequest(
    NullRhiDevice &device,
    RenderFixturePassRequest *request,
    std::vector<std::uint8_t> *capture) {
    if (request == nullptr || capture == nullptr) {
        return false;
    }

    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device_interface, &target)) {
        return false;
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, &pipeline)) {
        return false;
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, &vertex_buffer)) {
        return false;
    }

    RhiBufferHandle index_buffer{};
    if (!CreateTriangleIndexBuffer(device_interface, &index_buffer)) {
        return false;
    }

    RhiTextureHandle sampled_texture{};
    if (!CreateSampledTexture(device_interface, &sampled_texture)) {
        return false;
    }

    RhiSamplerHandle sampler{};
    if (!CreateSamplerPrimitive(device_interface, &sampler)) {
        return false;
    }

    request->rhi_device = &device_interface;
    request->target = target;
    request->pipeline = pipeline;
    request->vertex_buffer = TriangleVertexBufferViewFor(vertex_buffer);
    request->index_buffer = TriangleIndexBufferViewFor(index_buffer);
    request->sampled_texture = SampledTextureBindingFor(sampled_texture);
    request->sampler = SamplerBindingFor(sampler);
    request->draw = TriangleDrawIndexedDesc();
    request->clear_color = RhiColor{7U, 11U, 13U, 255U};
    request->capture_output = std::span<std::uint8_t>(capture->data(), capture->size());
    request->capture_byte_budget = capture->size();
    request->pass_id = 17U;
    request->material_id = 19U;
    return true;
}

UiDrawElement MakeDrawElement(std::uint32_t node_id, UiDrawElementType type, std::uint32_t material_id) {
    UiDrawElement element{};
    element.node_id = UiNodeId{node_id};
    element.type = type;
    element.rect = UiRect{0.0F, 0.0F, 128.0F, 64.0F};
    element.clip_rect = UiRect{0.0F, 0.0F, 128.0F, 64.0F};
    element.layer = 1;
    element.sibling_order = node_id;
    element.material_key = material_id;
    element.style_key = 5U;
    element.texture_key = 7U;
    element.scissor_enabled = true;
    return element;
}

UiRenderCoreBridgeRequest BridgeRequestFrom(
    RenderFixturePass &pass,
    RenderSubmissionBatchFixture &batch,
    const std::span<const UiDrawElement> elements,
    const RenderFixturePassRequest &template_request,
    const std::span<RenderFixturePassRequest> pass_requests,
    const std::span<RenderFixturePassResult> pass_results) {
    UiRenderCoreBridgeRequest request{};
    request.pass = &pass;
    request.submission_batch = &batch;
    request.draw_elements = elements;
    request.template_pass_request = template_request;
    request.pass_requests = pass_requests;
    request.pass_results = pass_results;
    request.pass_id_base = PASS_ID_BASE;
    return request;
}

RenderFixturePassRequest SentinelPassRequest() {
    RenderFixturePassRequest request{};
    request.pass_id = 77U;
    request.material_id = 79U;
    return request;
}

bool PassRequestMatchesSentinel(const RenderFixturePassRequest &request) {
    if (request.pass_id != 77U) {
        return false;
    }

    return request.material_id == 79U;
}

bool DeviceSubmittedPasses(const RhiDeviceSnapshot &snapshot, std::uint64_t count) {
    if (snapshot.submit_count != count) {
        return false;
    }

    if (snapshot.present_count != count) {
        return false;
    }

    return snapshot.capture_count == count;
}

int UiRenderCoreBridgeSubmitsDrawElementsThroughRenderCore() {
    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest template_request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, SENTINEL_BYTE);
    if (!FillValidTemplateRequest(device, &template_request, &capture)) {
        return Fail("template request setup failed");
    }

    const std::array<UiDrawElement, 2U> elements{
        MakeDrawElement(11U, UiDrawElementType::Rect, FIRST_MATERIAL_ID),
        MakeDrawElement(13U, UiDrawElementType::TexturedQuad, SECOND_MATERIAL_ID)};
    std::array<RenderFixturePassRequest, 2U> pass_requests{};
    std::array<RenderFixturePassResult, 2U> pass_results{};
    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    UiRenderCoreBridge bridge;
    const std::span<const UiDrawElement> element_span(elements.data(), elements.size());
    const std::span<RenderFixturePassRequest> request_span(pass_requests.data(), pass_requests.size());
    const std::span<RenderFixturePassResult> result_span(pass_results.data(), pass_results.size());
    const UiRenderCoreBridgeRequest request = BridgeRequestFrom(
        pass,
        batch,
        element_span,
        template_request,
        request_span,
        result_span);

    const auto result = bridge.Submit(request);
    if (result.status != UiRenderCoreBridgeStatus::Success) {
        return Fail("bridge did not submit valid draw elements");
    }

    if (result.completed_entry_count != 2U || result.submitted_entry_count != 2U) {
        return Fail("bridge result did not report both submitted entries");
    }

    if (pass_requests[0U].pass_id != PASS_ID_BASE || pass_requests[1U].pass_id != PASS_ID_BASE + 1U) {
        return Fail("bridge did not assign deterministic pass ids");
    }

    if (pass_requests[0U].material_id != FIRST_MATERIAL_ID || pass_requests[1U].material_id != SECOND_MATERIAL_ID) {
        return Fail("bridge did not map material keys");
    }

    if (pass_requests[0U].draw.index_count != template_request.draw.index_count) {
        return Fail("bridge overwrote template draw geometry");
    }

    const auto pass_snapshot = pass.Snapshot();
    if (pass_snapshot.completed_pass_count != 2U) {
        return Fail("RenderCore fixture pass did not complete both UI draws");
    }

    const auto batch_snapshot = batch.Snapshot();
    if (batch_snapshot.completed_entry_count != 2U || batch_snapshot.last_material_id != SECOND_MATERIAL_ID) {
        return Fail("RenderCore submission batch snapshot did not track UI draws");
    }

    const RhiDeviceSnapshot device_snapshot = device.Snapshot();
    if (!DeviceSubmittedPasses(device_snapshot, 2U)) {
        return Fail("RHI null device did not receive both RenderCore submissions");
    }

    const auto bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.completed_draw_count != 2U || bridge_snapshot.last_status != UiRenderCoreBridgeStatus::Success) {
        return Fail("bridge snapshot did not track completed UI draws");
    }

    return 0;
}

int UiRenderCoreBridgeRejectsInvalidDrawElementWithoutSubmission() {
    std::array<UiDrawElement, 1U> elements{MakeDrawElement(0U, UiDrawElementType::Rect, FIRST_MATERIAL_ID)};
    std::array<RenderFixturePassRequest, 1U> pass_requests{SentinelPassRequest()};
    std::array<RenderFixturePassResult, 1U> pass_results{};
    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    UiRenderCoreBridge bridge;
    RenderFixturePassRequest template_request{};
    const std::span<const UiDrawElement> element_span(elements.data(), elements.size());
    const std::span<RenderFixturePassRequest> request_span(pass_requests.data(), pass_requests.size());
    const std::span<RenderFixturePassResult> result_span(pass_results.data(), pass_results.size());
    const UiRenderCoreBridgeRequest request = BridgeRequestFrom(
        pass,
        batch,
        element_span,
        template_request,
        request_span,
        result_span);

    const auto result = bridge.Submit(request);
    if (result.status != UiRenderCoreBridgeStatus::InvalidDrawElement) {
        return Fail("bridge accepted invalid draw element");
    }

    if (!PassRequestMatchesSentinel(pass_requests[0U])) {
        return Fail("bridge mutated pass request after invalid draw element");
    }

    if (pass.Snapshot().executed_pass_count != 0U || batch.Snapshot().submission_record_count != 0U) {
        return Fail("bridge submitted after invalid draw element");
    }

    if (bridge.Snapshot().failed_validation_count != 1U) {
        return Fail("bridge did not record invalid draw validation failure");
    }

    return 0;
}

int UiRenderCoreBridgeRejectsSmallOutputStorageWithoutSubmission() {
    const std::array<UiDrawElement, 2U> elements{
        MakeDrawElement(11U, UiDrawElementType::Rect, FIRST_MATERIAL_ID),
        MakeDrawElement(13U, UiDrawElementType::TexturedQuad, SECOND_MATERIAL_ID)};
    std::array<RenderFixturePassRequest, 1U> pass_requests{SentinelPassRequest()};
    std::array<RenderFixturePassResult, 2U> pass_results{};
    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    UiRenderCoreBridge bridge;
    RenderFixturePassRequest template_request{};
    const std::span<const UiDrawElement> element_span(elements.data(), elements.size());
    const std::span<RenderFixturePassRequest> request_span(pass_requests.data(), pass_requests.size());
    const std::span<RenderFixturePassResult> result_span(pass_results.data(), pass_results.size());
    const UiRenderCoreBridgeRequest request = BridgeRequestFrom(
        pass,
        batch,
        element_span,
        template_request,
        request_span,
        result_span);

    const auto result = bridge.Submit(request);
    if (result.status != UiRenderCoreBridgeStatus::OutputCapacityExceeded) {
        return Fail("bridge accepted undersized output storage");
    }

    if (!PassRequestMatchesSentinel(pass_requests[0U])) {
        return Fail("bridge mutated pass request after undersized output storage");
    }

    if (pass.Snapshot().executed_pass_count != 0U || batch.Snapshot().submission_record_count != 0U) {
        return Fail("bridge submitted after undersized output storage");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_SUBMITS) {
        return UiRenderCoreBridgeSubmitsDrawElementsThroughRenderCore();
    }

    if (name == TEST_INVALID_DRAW) {
        return UiRenderCoreBridgeRejectsInvalidDrawElementWithoutSubmission();
    }

    if (name == TEST_SMALL_STORAGE) {
        return UiRenderCoreBridgeRejectsSmallOutputStorageWithoutSubmission();
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
