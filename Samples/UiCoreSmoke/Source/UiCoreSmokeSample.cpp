// 模块: UiCoreSmokeSample
// 文件: Samples/UiCoreSmoke/Source/UiCoreSmokeSample.cpp

#include "UiCoreSmokeSample.h"

#include <array>
#include <cstddef>
#include <cstdint>
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
#include "YuEngine/UiCore/UiDrawElementDesc.h"
#include "YuEngine/UiCore/UiDrawElementType.h"
#include "YuEngine/UiCore/UiDrawListBuilder.h"
#include "YuEngine/UiCore/UiDrawListResult.h"
#include "YuEngine/UiCore/UiDrawListStatus.h"
#include "YuEngine/UiCore/UiLayoutContainerDesc.h"
#include "YuEngine/UiCore/UiLayoutContainerType.h"
#include "YuEngine/UiCore/UiLayoutPass.h"
#include "YuEngine/UiCore/UiLayoutPassResult.h"
#include "YuEngine/UiCore/UiNodeDesc.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiNodeTree.h"
#include "YuEngine/UiCore/UiNodeTreeDesc.h"
#include "YuEngine/UiCore/UiNodeTreeResult.h"
#include "YuEngine/UiCore/UiNodeTreeStatus.h"
#include "YuEngine/UiCore/UiRect.h"
#include "YuEngine/UiCore/UiRectTransform.h"
#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridge.h"
#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeRequest.h"
#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeResult.h"
#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeStatus.h"

namespace ui_core_smoke_sample {
namespace {
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
using RhiIndexBufferView = yuengine::rhi::RhiIndexBufferView;
using yuengine::rhi::RhiFormat;
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
using UiDrawElementDesc = yuengine::uicore::UiDrawElementDesc;
using yuengine::uicore::UiDrawElementType;
using UiDrawListBuilder = yuengine::uicore::UiDrawListBuilder;
using UiDrawListResult = yuengine::uicore::UiDrawListResult;
using yuengine::uicore::UiDrawListStatus;
using UiLayoutContainerDesc = yuengine::uicore::UiLayoutContainerDesc;
using yuengine::uicore::UiLayoutContainerType;
using UiLayoutPass = yuengine::uicore::UiLayoutPass;
using UiLayoutPassResult = yuengine::uicore::UiLayoutPassResult;
using UiNodeDesc = yuengine::uicore::UiNodeDesc;
using UiNodeId = yuengine::uicore::UiNodeId;
using UiNodeTree = yuengine::uicore::UiNodeTree;
using UiNodeTreeDesc = yuengine::uicore::UiNodeTreeDesc;
using UiNodeTreeResult = yuengine::uicore::UiNodeTreeResult;
using yuengine::uicore::UiNodeTreeStatus;
using UiRect = yuengine::uicore::UiRect;
using UiRectTransform = yuengine::uicore::UiRectTransform;
using UiRenderCoreBridge = yuengine::uirendercorebridge::UiRenderCoreBridge;
using UiRenderCoreBridgeRequest = yuengine::uirendercorebridge::UiRenderCoreBridgeRequest;
using UiRenderCoreBridgeResult = yuengine::uirendercorebridge::UiRenderCoreBridgeResult;
using yuengine::uirendercorebridge::UiRenderCoreBridgeStatus;

constexpr std::uint32_t ROOT_NODE_ID = 1U;
constexpr std::uint32_t WINDOW_NODE_ID = 2U;
constexpr std::uint32_t TITLE_NODE_ID = 3U;
constexpr std::uint32_t WINDOW_MATERIAL_ID = 101U;
constexpr std::uint32_t TITLE_MATERIAL_ID = 102U;
constexpr std::uint32_t UI_PASS_ID_BASE = 900U;
constexpr std::uint32_t TRIANGLE_VERTEX_COUNT = 3U;
constexpr std::uint32_t TRIANGLE_INDEX_COUNT = 3U;
constexpr std::size_t TRIANGLE_VERTEX_STRIDE_BYTES = sizeof(float) * 6U;
constexpr std::size_t TRIANGLE_VERTEX_BUFFER_BYTES = TRIANGLE_VERTEX_STRIDE_BYTES * TRIANGLE_VERTEX_COUNT;
constexpr std::size_t TRIANGLE_INDEX_BUFFER_BYTES = sizeof(std::uint16_t) * TRIANGLE_INDEX_COUNT;
constexpr std::size_t CAPTURE_BYTES = 16U;

struct SmokeLayout final {
    std::uint32_t node_count = 0U;
    std::uint32_t root_node_id = 0U;
    std::uint32_t window_node_id = 0U;
    std::uint32_t title_node_id = 0U;
};

bool ContainsText(std::string_view text, std::string_view expected) {
    return text.find(expected) != std::string_view::npos;
}

bool LoadSmokeLayout(std::string_view text, SmokeLayout *layout) {
    if (layout == nullptr) {
        return false;
    }

    if (text.empty()) {
        return false;
    }

    if (!ContainsText(text, "\"schema\": \"YuEngine.UI.Layout\"")) {
        return false;
    }

    if (!ContainsText(text, "\"layoutId\": \"UiCoreSmoke.SimpleWindow\"")) {
        return false;
    }

    if (!ContainsText(text, "\"rootNodeId\": 1")) {
        return false;
    }

    if (!ContainsText(text, "\"nodeId\": 1") || !ContainsText(text, "\"nodeId\": 2")) {
        return false;
    }

    if (!ContainsText(text, "\"nodeId\": 3")) {
        return false;
    }

    if (!ContainsText(text, "\"type\": \"Window\"") || !ContainsText(text, "\"type\": \"TitleBar\"")) {
        return false;
    }

    layout->node_count = 3U;
    layout->root_node_id = ROOT_NODE_ID;
    layout->window_node_id = WINDOW_NODE_ID;
    layout->title_node_id = TITLE_NODE_ID;
    return true;
}

UiNodeId NodeId(std::uint32_t value) {
    return UiNodeId{value};
}

UiRectTransform FullStretchTransform() {
    UiRectTransform transform;
    transform.anchor_min = {0.0F, 0.0F};
    transform.anchor_max = {1.0F, 1.0F};
    transform.pivot = {0.5F, 0.5F};
    return transform;
}

UiRectTransform FixedWindowTransform() {
    UiRectTransform transform;
    transform.anchor_min = {0.0F, 0.0F};
    transform.anchor_max = {1.0F, 1.0F};
    transform.pivot = {0.5F, 0.5F};
    transform.offset_min = {120.0F, 120.0F};
    transform.offset_max = {-120.0F, -160.0F};
    return transform;
}

UiRectTransform TitleBarTransform() {
    UiRectTransform transform;
    transform.anchor_min = {0.0F, 1.0F};
    transform.anchor_max = {1.0F, 1.0F};
    transform.pivot = {0.5F, 1.0F};
    transform.offset_min = {0.0F, -48.0F};
    transform.offset_max = {0.0F, 0.0F};
    return transform;
}

UiNodeDesc MakeNodeDesc(UiNodeId node_id, UiNodeId parent_id, std::uint32_t order) {
    UiNodeDesc desc;
    desc.node_id = node_id;
    desc.parent_id = parent_id;
    desc.rect_transform = FullStretchTransform();
    desc.sibling_order = order;
    desc.layer = 1;
    desc.is_visible = true;
    desc.is_enabled = true;
    desc.is_hit_testable = true;
    return desc;
}

bool CreateNode(UiNodeTree *tree, const UiNodeDesc &desc) {
    if (tree == nullptr) {
        return false;
    }

    const UiNodeTreeResult result = tree->CreateNode(desc);
    return result.Succeeded();
}

bool BuildNodeTree(const SmokeLayout &layout, UiNodeTree *tree) {
    if (tree == nullptr) {
        return false;
    }

    UiNodeDesc root_desc = MakeNodeDesc(NodeId(layout.root_node_id), UiNodeId{}, 0U);
    root_desc.layer = 0;
    if (!CreateNode(tree, root_desc)) {
        return false;
    }

    UiNodeDesc window_desc = MakeNodeDesc(NodeId(layout.window_node_id), NodeId(layout.root_node_id), 0U);
    window_desc.rect_transform = FixedWindowTransform();
    window_desc.layer = 10;
    if (!CreateNode(tree, window_desc)) {
        return false;
    }

    UiNodeDesc title_desc = MakeNodeDesc(NodeId(layout.title_node_id), NodeId(layout.window_node_id), 0U);
    title_desc.rect_transform = TitleBarTransform();
    title_desc.layer = 20;
    return CreateNode(tree, title_desc);
}

bool ApplySmokeLayout(UiNodeTree *tree, std::uint32_t *out_container_count) {
    if (tree == nullptr || out_container_count == nullptr) {
        return false;
    }

    std::array<UiLayoutContainerDesc, 2U> containers{};
    containers[0U].container_id = NodeId(ROOT_NODE_ID);
    containers[0U].type = UiLayoutContainerType::Absolute;
    containers[1U].container_id = NodeId(WINDOW_NODE_ID);
    containers[1U].type = UiLayoutContainerType::Absolute;

    UiLayoutPass pass;
    const UiLayoutPassResult result = pass.Apply(tree, containers);
    if (!result.Succeeded()) {
        return false;
    }

    if (result.container_count != containers.size() || result.arranged_node_count != 2U) {
        return false;
    }

    *out_container_count = result.container_count;
    return true;
}

bool BuildDrawElements(UiNodeTree &tree, std::array<UiDrawElement, 2U> *elements) {
    if (elements == nullptr) {
        return false;
    }

    const std::array<UiDrawElementDesc, 2U> descs{
        UiDrawElementDesc{NodeId(WINDOW_NODE_ID), UiDrawElementType::Rect, 11U, WINDOW_MATERIAL_ID, 0U, 0U, true},
        UiDrawElementDesc{NodeId(TITLE_NODE_ID), UiDrawElementType::Rect, 12U, TITLE_MATERIAL_ID, 0U, 0U, true}};

    UiDrawListResult result;
    UiDrawListBuilder builder;
    const std::span<const UiDrawElementDesc> desc_span(descs.data(), descs.size());
    const std::span<UiDrawElement> element_span(elements->data(), elements->size());
    const UiDrawListStatus status = builder.Build(tree, desc_span, element_span, &result);
    if (status != UiDrawListStatus::Success) {
        return false;
    }

    return result.element_count == elements->size();
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

bool FillRenderTemplateRequest(
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
    request->clear_color = RhiColor{9U, 23U, 41U, 255U};
    request->capture_output = std::span<std::uint8_t>(capture->data(), capture->size());
    request->capture_byte_budget = capture->size();
    request->pass_id = 1U;
    request->material_id = WINDOW_MATERIAL_ID;
    return true;
}

bool SubmitDrawElements(
    const std::array<UiDrawElement, 2U> &elements,
    UiCoreSmokeSampleResult *sample_result) {
    if (sample_result == nullptr) {
        return false;
    }

    NullRhiDevice device = CreateInitializedDevice();
    RenderFixturePassRequest template_request{};
    std::vector<std::uint8_t> capture(CAPTURE_BYTES, 0U);
    if (!FillRenderTemplateRequest(device, &template_request, &capture)) {
        sample_result->failure_stage = "render_template";
        return false;
    }

    std::array<RenderFixturePassRequest, 2U> pass_requests{};
    std::array<RenderFixturePassResult, 2U> pass_results{};
    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    UiRenderCoreBridge bridge;
    const std::span<const UiDrawElement> element_span(elements.data(), elements.size());
    const std::span<RenderFixturePassRequest> request_span(pass_requests.data(), pass_requests.size());
    const std::span<RenderFixturePassResult> result_span(pass_results.data(), pass_results.size());

    UiRenderCoreBridgeRequest request{};
    request.pass = &pass;
    request.submission_batch = &batch;
    request.draw_elements = element_span;
    request.template_pass_request = template_request;
    request.pass_requests = request_span;
    request.pass_results = result_span;
    request.pass_id_base = UI_PASS_ID_BASE;

    const UiRenderCoreBridgeResult result = bridge.Submit(request);
    if (result.status != UiRenderCoreBridgeStatus::Success) {
        sample_result->failure_stage = "render_submit";
        return false;
    }

    const RhiDeviceSnapshot device_snapshot = device.Snapshot();
    if (device_snapshot.submit_count != elements.size() || device_snapshot.present_count != elements.size()) {
        sample_result->failure_stage = "render_snapshot";
        return false;
    }

    sample_result->submitted_entry_count = result.completed_entry_count;
    sample_result->render_submit_count = device_snapshot.submit_count;
    return true;
}
}

bool RunUiCoreSmokeSample(const UiCoreSmokeSampleInput &input, UiCoreSmokeSampleResult *result) {
    if (result == nullptr) {
        return false;
    }

    *result = {};
    result->failure_stage = "layout_load";

    SmokeLayout layout{};
    if (!LoadSmokeLayout(input.layout_text, &layout)) {
        return false;
    }

    result->layout_loaded = true;
    result->layout_node_count = layout.node_count;

    UiNodeTreeDesc tree_desc;
    tree_desc.node_capacity = 8U;
    tree_desc.viewport_rect = UiRect{0.0F, 0.0F, 800.0F, 600.0F};
    UiNodeTree tree(tree_desc);
    result->failure_stage = "node_tree";
    if (!BuildNodeTree(layout, &tree)) {
        return false;
    }

    result->node_tree_built = true;

    result->failure_stage = "layout_pass";
    if (!ApplySmokeLayout(&tree, &result->layout_container_count)) {
        return false;
    }

    result->layout_passed = true;

    std::array<UiDrawElement, 2U> elements{};
    result->failure_stage = "draw_list";
    if (!BuildDrawElements(tree, &elements)) {
        return false;
    }

    result->draw_list_built = true;
    result->draw_element_count = elements.size();

    if (!SubmitDrawElements(elements, result)) {
        return false;
    }

    result->render_submitted = true;
    result->pass_reported = true;
    result->failure_stage = "success";
    return true;
}

bool BuildUiCoreSmokeValidationRoute(UiCoreSmokeValidationRoute *route) {
    if (route == nullptr) {
        return false;
    }

    route->configure_command = "cmake --preset windows-fast-gate";
    route->build_command = "cmake --build --preset windows-fast-gate --target YuUiCoreSmokeSample YuUiCoreSmokeSampleTests -- /v:minimal";
    route->test_command = "ctest --preset windows-fast-gate -R \"^UiCoreSmokeSample_\" --output-on-failure";
    route->sample_command = ".\\build\\windows-fast-gate-vs\\Debug\\YuUiCoreSmokeSample.exe --layout Samples\\UiCoreSmoke\\Layouts\\SimpleWindow.YuUILayout.json";
    route->validation_doc_path = "docs/YUENGINE_UI_STAGE1_VALIDATION.md";
    route->configure_command_available = true;
    route->build_command_available = true;
    route->test_command_available = true;
    route->sample_command_available = true;
    route->validation_doc_available = true;
    return true;
}
}
