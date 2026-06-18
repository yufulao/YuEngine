// Module: Tests RenderCore
// File: Tests/RenderCore/RenderDrawableFramePipelineD3D11HardwareSmokeTests.cpp

#include "YuEngine/RenderCore/MaterialBindingFixtureStatus.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipeline.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineRequest.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineStatus.h"
#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureStatus.h"

#define main RhiD3D11HardwareSmokeMain
#include "../Rhi/RhiD3D11HardwareSmokeTests.cpp"
#undef main

using RenderDrawableFramePipeline = yuengine::rendercore::RenderDrawableFramePipeline;
using RenderDrawableFramePipelineRequest = yuengine::rendercore::RenderDrawableFramePipelineRequest;
using yuengine::rendercore::MaterialBindingFixtureStatus;
using yuengine::rendercore::RenderDrawableFramePipelineStatus;
using yuengine::rendercore::RenderFixturePassStatus;
using yuengine::rendercore::RenderFramePacketFixtureStatus;

namespace {
constexpr const char *DRAWABLE_TEST_NAME =
    "RenderCore_D3D11Hardware_DrawableFramePipelineTextureSamplingCapture";
constexpr const char *DRAWABLE_ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *DRAWABLE_ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t DRAWABLE_FRAME_ID = 1U;
constexpr std::uint32_t DRAWABLE_PASS_ID = 31U;
constexpr std::uint32_t DRAWABLE_MATERIAL_ID = 41U;

int RunD3D11DrawableFramePipelineTextureSamplingCapture() {
    WindowsPlatformWindow window;
    PlatformWindowDesc window_desc{};
    window_desc.title = "YuEngine RenderCore D3D11 Drawable Frame Pipeline Smoke";
    window_desc.client_width = SMOKE_EXTENT;
    window_desc.client_height = SMOKE_EXTENT;
    window_desc.visible = false;

    const PlatformWindowStatus window_status = window.Create(window_desc);
    if (window_status != PlatformWindowStatus::Success) {
        return Skip("rendercore d3d11 drawable frame smoke skipped because a native window could not be created");
    }

    const RhiExtent2D initial_extent{SMOKE_EXTENT, SMOKE_EXTENT};
    std::vector<std::byte> storage{};
    const RhiDeviceCreateResult create_result = CreateD3D11DeviceForWindow(window, storage, initial_extent);
    if (create_result.status == RhiStatus::UnsupportedBackend) {
        return Skip("rendercore d3d11 drawable frame smoke skipped because the backend is not compiled");
    }

    if (create_result.status == RhiStatus::MissingHardware) {
        return Skip("rendercore d3d11 drawable frame smoke skipped because a hardware D3D11 device is unavailable");
    }

    if (create_result.status != RhiStatus::Success) {
        return Fail("rendercore d3d11 drawable frame device creation failed");
    }

    if (create_result.device == nullptr) {
        return Fail("rendercore d3d11 drawable frame device creation returned null device");
    }

    IRhiDevice &device = *create_result.device;
    std::array<TexturedVertex, 3U> vertices{};
    vertices[0U].position[0U] = -1.0F;
    vertices[0U].position[1U] = -1.0F;
    vertices[0U].texcoord[0U] = 0.0F;
    vertices[0U].texcoord[1U] = 1.0F;
    vertices[1U].position[0U] = -1.0F;
    vertices[1U].position[1U] = 1.0F;
    vertices[1U].texcoord[0U] = 0.0F;
    vertices[1U].texcoord[1U] = 0.0F;
    vertices[2U].position[0U] = 1.0F;
    vertices[2U].position[1U] = -1.0F;
    vertices[2U].texcoord[0U] = 1.0F;
    vertices[2U].texcoord[1U] = 1.0F;

    const auto *vertex_byte_pointer = reinterpret_cast<const std::uint8_t *>(vertices.data());
    const std::size_t vertex_byte_count = sizeof(TexturedVertex) * vertices.size();
    const std::span<const std::uint8_t> vertex_span(vertex_byte_pointer, vertex_byte_count);
    RhiBufferDesc vertex_buffer_desc{};
    vertex_buffer_desc.usage = RhiBufferUsage::Vertex;
    vertex_buffer_desc.size_bytes = vertex_byte_count;
    RhiBufferHandle vertex_buffer{};
    RhiStatus status = device.CreateBuffer(vertex_buffer_desc, vertex_span, vertex_buffer);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame vertex buffer creation failed");
    }

    const std::array<std::uint16_t, TRIANGLE_INDEX_COUNT> indices{0U, 1U, 2U};
    const auto *index_byte_pointer = reinterpret_cast<const std::uint8_t *>(indices.data());
    const std::span<const std::uint8_t> index_span(index_byte_pointer, TRIANGLE_INDEX_BUFFER_BYTES);
    RhiBufferDesc index_buffer_desc{};
    index_buffer_desc.usage = RhiBufferUsage::Index;
    index_buffer_desc.size_bytes = TRIANGLE_INDEX_BUFFER_BYTES;
    RhiBufferHandle index_buffer{};
    status = device.CreateBuffer(index_buffer_desc, index_span, index_buffer);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame index buffer creation failed");
    }

    const std::uint8_t texture_bytes[] = {
        0U, 0U, 255U, 255U,
        0U, 0U, 255U, 255U,
        0U, 0U, 255U, 255U,
        0U, 0U, 255U, 255U};
    const std::span<const std::uint8_t> texture_span(texture_bytes, sizeof(texture_bytes));
    RhiTextureDesc texture_desc{};
    texture_desc.format = RhiFormat::Rgba8Unorm;
    texture_desc.extent = {2U, 2U};
    RhiTextureHandle texture{};
    status = device.CreateTexture(texture_desc, texture_span, texture);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame texture creation failed");
    }

    RhiSamplerDesc sampler_desc{};
    sampler_desc.linear_filter = false;
    sampler_desc.clamp_to_edge = true;
    RhiSamplerHandle sampler{};
    status = device.CreateSampler(sampler_desc, sampler);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame sampler creation failed");
    }

    const std::span<const std::uint8_t> vertex_bytecode(
        TEXTURE_SAMPLING_VERTEX_SHADER_BYTES,
        sizeof(TEXTURE_SAMPLING_VERTEX_SHADER_BYTES));
    const RhiShaderModuleDesc vertex_desc{RhiShaderStage::Vertex, vertex_bytecode};
    RhiShaderModuleHandle vertex_shader{};
    status = device.CreateShaderModule(vertex_desc, vertex_shader);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame vertex shader creation failed");
    }

    const std::span<const std::uint8_t> pixel_bytecode(
        TEXTURE_SAMPLING_PIXEL_SHADER_BYTES,
        sizeof(TEXTURE_SAMPLING_PIXEL_SHADER_BYTES));
    const RhiShaderModuleDesc pixel_desc{RhiShaderStage::Pixel, pixel_bytecode};
    RhiShaderModuleHandle pixel_shader{};
    status = device.CreateShaderModule(pixel_desc, pixel_shader);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame pixel shader creation failed");
    }

    RhiPipelineDesc pipeline_desc{};
    pipeline_desc.vertex_shader = vertex_shader;
    pipeline_desc.pixel_shader = pixel_shader;
    pipeline_desc.input_layout = TexturedTriangleInputLayoutDesc();
    RhiPipelineHandle pipeline{};
    status = device.CreatePipeline(pipeline_desc, pipeline);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame pipeline creation failed");
    }

    std::vector<std::uint8_t> capture(SMOKE_EXTENT * SMOKE_EXTENT * RGBA8_BYTES_PER_PIXEL);
    RenderDrawableFramePipeline render_pipeline;
    RenderDrawableFramePipelineRequest request{};
    request.rhi_device = &device;
    request.pipeline = pipeline;
    request.vertex_buffer = TexturedVertexBufferViewFor(vertex_buffer);
    request.index_buffer = TriangleIndexBufferViewFor(index_buffer);
    request.sampled_texture = SampledTextureBindingFor(texture);
    request.sampler = SamplerBindingFor(sampler);
    request.draw = TriangleDrawIndexedDesc();
    request.clear_color = RhiColor{0U, 0U, 0U, 255U};
    request.capture_output = std::span<std::uint8_t>(capture.data(), capture.size());
    request.capture_byte_budget = capture.size();
    request.frame_id = DRAWABLE_FRAME_ID;
    request.pass_id = DRAWABLE_PASS_ID;
    request.material_id = DRAWABLE_MATERIAL_ID;

    const auto result = render_pipeline.Execute(request);
    if (result.status != RenderDrawableFramePipelineStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame pipeline failed");
    }

    if (result.material_result.status != MaterialBindingFixtureStatus::Success ||
        result.frame_result.status != RenderFramePacketFixtureStatus::Success ||
        result.pass_result.status != RenderFixturePassStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame did not execute rendercore stack");
    }

    if (result.recorded_command_count != 9U || result.capture_bytes_written != capture.size()) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame reported wrong command or capture count");
    }

    if (!CaptureContainsTextureSampling(capture)) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame capture did not contain sampled texture");
    }

    const auto rhi_snapshot = device.Snapshot();
    if (rhi_snapshot.submitted_indexed_draw_count != 1U ||
        rhi_snapshot.submitted_sampled_texture_bind_count != 1U ||
        rhi_snapshot.submitted_sampler_bind_count != 1U ||
        rhi_snapshot.submit_count != 1U ||
        rhi_snapshot.present_count != 1U ||
        rhi_snapshot.capture_count != 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame did not update rhi counters");
    }

    const auto pipeline_snapshot = render_pipeline.Snapshot();
    if (pipeline_snapshot.completed_frame_count != 1U ||
        pipeline_snapshot.last_capture_bytes_written != capture.size()) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame did not update pipeline counters");
    }

    if (device.DestroyPipeline(pipeline) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame pipeline destroy failed");
    }

    if (device.DestroyShaderModule(pixel_shader) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame pixel shader destroy failed");
    }

    if (device.DestroyShaderModule(vertex_shader) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame vertex shader destroy failed");
    }

    if (device.DestroySampler(sampler) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame sampler destroy failed");
    }

    if (device.DestroyTexture(texture) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame texture destroy failed");
    }

    if (device.DestroyBuffer(index_buffer) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame index buffer destroy failed");
    }

    if (device.DestroyBuffer(vertex_buffer) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 drawable frame vertex buffer destroy failed");
    }

    const RhiStatus destroy_status = RhiDeviceFactory::DestroyDevice(create_result.device);
    if (destroy_status != RhiStatus::Success) {
        return Fail("rendercore d3d11 drawable frame device destroy failed");
    }

    return 0;
}

int RunDrawableNamedTest(std::string_view name) {
    if (name == DRAWABLE_TEST_NAME) {
        return RunD3D11DrawableFramePipelineTextureSamplingCapture();
    }

    return Fail(DRAWABLE_ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(DRAWABLE_ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunDrawableNamedTest(argv[1]);
}
