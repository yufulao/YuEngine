// Module: Tests Rhi
// File: Tests/Rhi/RhiD3D11HardwareSmokeTests.cpp

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>
#include <vector>

#include "YuEngine/Platform/PlatformNativeSurface.h"
#include "YuEngine/Platform/PlatformWindowDesc.h"
#include "YuEngine/Platform/PlatformWindowStatus.h"
#include "YuEngine/Platform/WindowsPlatformWindow.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiBufferUsage.h"
#include "YuEngine/Rhi/RhiCaptureResult.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiCommandList.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiDeviceCreateResult.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiDeviceFactory.h"
#include "YuEngine/Rhi/RhiFenceHandle.h"
#include "YuEngine/Rhi/RhiNativeSurfaceDesc.h"
#include "YuEngine/Rhi/RhiPipelineDesc.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiSamplerDesc.h"
#include "YuEngine/Rhi/RhiSamplerHandle.h"
#include "YuEngine/Rhi/RhiShaderModuleDesc.h"
#include "YuEngine/Rhi/RhiShaderModuleHandle.h"
#include "YuEngine/Rhi/RhiShaderStage.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

using PlatformNativeSurface = yuengine::platform::PlatformNativeSurface;
using PlatformWindowDesc = yuengine::platform::PlatformWindowDesc;
using yuengine::platform::PlatformWindowStatus;
using WindowsPlatformWindow = yuengine::platform::WindowsPlatformWindow;
using yuengine::rhi::IRhiDevice;
using yuengine::rhi::MAX_COMMANDS;
using yuengine::rhi::RGBA8_BYTES_PER_PIXEL;
using yuengine::rhi::RhiBackendKind;
using yuengine::rhi::RhiBufferDesc;
using yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiBufferUsage;
using yuengine::rhi::RhiCaptureResult;
using yuengine::rhi::RhiColor;
using yuengine::rhi::RhiCommandList;
using yuengine::rhi::RhiDeviceCreateResult;
using yuengine::rhi::RhiDeviceDesc;
using yuengine::rhi::RhiDeviceFactory;
using yuengine::rhi::RhiFenceHandle;
using yuengine::rhi::RhiFormat;
using yuengine::rhi::RhiNativeSurfaceDesc;
using yuengine::rhi::RhiPipelineDesc;
using yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiSamplerDesc;
using yuengine::rhi::RhiSamplerHandle;
using yuengine::rhi::RhiShaderModuleDesc;
using yuengine::rhi::RhiShaderModuleHandle;
using yuengine::rhi::RhiShaderStage;
using yuengine::rhi::RhiStatus;
using yuengine::rhi::RhiTextureDesc;
using yuengine::rhi::RhiTextureHandle;

namespace {
constexpr const char *TEST_D3D11_CLEAR_PRESENT_CAPTURE = "RHI_D3D11Hardware_ClearPresentCaptureBytes";
constexpr const char *TEST_D3D11_PRIMITIVE_RESOURCE_PIPELINE = "RHI_D3D11Hardware_PrimitiveResourcePipelineSnapshot";
constexpr std::uint32_t SMOKE_EXTENT = 4U;
constexpr int SKIP_RETURN_CODE = 77;
constexpr std::uint8_t VERTEX_SHADER_BYTES[] = {
    0x44U, 0x58U, 0x42U, 0x43U, 0x07U, 0x36U, 0x40U, 0xB0U,
    0xF6U, 0x7FU, 0xF2U, 0x2AU, 0xA8U, 0xDEU, 0xDBU, 0x0BU,
    0x0CU, 0x69U, 0x38U, 0xF5U, 0x01U, 0x00U, 0x00U, 0x00U,
    0xDCU, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x2CU, 0x00U, 0x00U, 0x00U, 0x60U, 0x00U, 0x00U, 0x00U,
    0x94U, 0x00U, 0x00U, 0x00U, 0x49U, 0x53U, 0x47U, 0x4EU,
    0x2CU, 0x00U, 0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x08U, 0x00U, 0x00U, 0x00U, 0x20U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x06U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x53U, 0x56U, 0x5FU, 0x56U,
    0x65U, 0x72U, 0x74U, 0x65U, 0x78U, 0x49U, 0x44U, 0x00U,
    0x4FU, 0x53U, 0x47U, 0x4EU, 0x2CU, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x08U, 0x00U, 0x00U, 0x00U,
    0x20U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x0FU, 0x00U, 0x00U, 0x00U,
    0x53U, 0x56U, 0x5FU, 0x50U, 0x6FU, 0x73U, 0x69U, 0x74U,
    0x69U, 0x6FU, 0x6EU, 0x00U, 0x53U, 0x48U, 0x45U, 0x58U,
    0x40U, 0x00U, 0x00U, 0x00U, 0x50U, 0x00U, 0x01U, 0x00U,
    0x10U, 0x00U, 0x00U, 0x00U, 0x6AU, 0x08U, 0x00U, 0x01U,
    0x67U, 0x00U, 0x00U, 0x04U, 0xF2U, 0x20U, 0x10U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x36U, 0x00U, 0x00U, 0x08U, 0xF2U, 0x20U, 0x10U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x02U, 0x40U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x80U, 0x3FU,
    0x3EU, 0x00U, 0x00U, 0x01U};
constexpr std::uint8_t PIXEL_SHADER_BYTES[] = {
    0x44U, 0x58U, 0x42U, 0x43U, 0xEEU, 0x4DU, 0x0BU, 0x94U,
    0x29U, 0xEEU, 0x02U, 0x01U, 0x63U, 0xA4U, 0x5DU, 0xF3U,
    0x4FU, 0x56U, 0xE1U, 0xB1U, 0x01U, 0x00U, 0x00U, 0x00U,
    0xB4U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x2CU, 0x00U, 0x00U, 0x00U, 0x3CU, 0x00U, 0x00U, 0x00U,
    0x70U, 0x00U, 0x00U, 0x00U, 0x49U, 0x53U, 0x47U, 0x4EU,
    0x08U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x08U, 0x00U, 0x00U, 0x00U, 0x4FU, 0x53U, 0x47U, 0x4EU,
    0x2CU, 0x00U, 0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x08U, 0x00U, 0x00U, 0x00U, 0x20U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x03U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x0FU, 0x00U, 0x00U, 0x00U, 0x53U, 0x56U, 0x5FU, 0x54U,
    0x61U, 0x72U, 0x67U, 0x65U, 0x74U, 0x00U, 0xABU, 0xABU,
    0x53U, 0x48U, 0x45U, 0x58U, 0x3CU, 0x00U, 0x00U, 0x00U,
    0x50U, 0x00U, 0x00U, 0x00U, 0x0FU, 0x00U, 0x00U, 0x00U,
    0x6AU, 0x08U, 0x00U, 0x01U, 0x65U, 0x00U, 0x00U, 0x03U,
    0xF2U, 0x20U, 0x10U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x36U, 0x00U, 0x00U, 0x08U, 0xF2U, 0x20U, 0x10U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x02U, 0x40U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x80U, 0x3FU, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x80U, 0x3FU,
    0x3EU, 0x00U, 0x00U, 0x01U};

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

int Skip(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stdout);
    std::fputc('\n', stdout);
    return SKIP_RETURN_CODE;
}

RhiNativeSurfaceDesc ConvertSurface(const PlatformNativeSurface &surface) {
    return RhiNativeSurfaceDesc{surface.window_value, surface.instance_value, surface.valid};
}

bool BytesMatchColor(const std::vector<std::uint8_t> &bytes, RhiColor color) {
    for (std::size_t index = 0U; index < bytes.size(); index += RGBA8_BYTES_PER_PIXEL) {
        if (bytes[index] != color.r) {
            return false;
        }

        if (bytes[index + 1U] != color.g) {
            return false;
        }

        if (bytes[index + 2U] != color.b) {
            return false;
        }

        if (bytes[index + 3U] != color.a) {
            return false;
        }
    }

    return true;
}

RhiStatus ClearPresentCapture(IRhiDevice &device, RhiTextureHandle target, RhiColor color, std::vector<std::uint8_t> &capture) {
    RhiCommandList command_list(MAX_COMMANDS);
    RhiStatus status = command_list.BeginFrame(target);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordClear(command_list, target, color);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = command_list.EndFrame();
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.Submit(command_list);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.Present();
    if (status != RhiStatus::Success) {
        return status;
    }

    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    return result.status;
}

int RunD3D11ClearPresentCapture() {
    WindowsPlatformWindow window;
    PlatformWindowDesc window_desc{};
    window_desc.title = "YuEngine D3D11 Hardware Smoke";
    window_desc.client_width = SMOKE_EXTENT;
    window_desc.client_height = SMOKE_EXTENT;
    window_desc.visible = false;

    const PlatformWindowStatus window_status = window.Create(window_desc);
    if (window_status != PlatformWindowStatus::Success) {
        return Skip("d3d11 hardware smoke skipped because a native window could not be created");
    }

    const std::size_t storage_size = RhiDeviceFactory::RequiredDeviceStorageSize(RhiBackendKind::D3D11);
    if (storage_size == 0U) {
        return Skip("d3d11 hardware smoke skipped because the backend is not compiled");
    }

    std::vector<std::byte> storage(storage_size);
    RhiDeviceDesc device_desc{};
    device_desc.backend_kind = RhiBackendKind::D3D11;
    device_desc.native_surface = ConvertSurface(window.GetNativeSurface());
    device_desc.requires_native_surface = true;
    device_desc.requires_swapchain = true;
    device_desc.swapchain.extent = {SMOKE_EXTENT, SMOKE_EXTENT};
    device_desc.command_list_capacity = MAX_COMMANDS;

    const RhiDeviceCreateResult create_result = RhiDeviceFactory::CreateDevice(
        device_desc,
        std::span<std::byte>(storage.data(), storage.size()));
    if (create_result.status == RhiStatus::MissingHardware) {
        return Skip("d3d11 hardware smoke skipped because a hardware D3D11 device is unavailable");
    }

    if (create_result.status != RhiStatus::Success) {
        return Fail("d3d11 device creation failed");
    }

    if (create_result.device == nullptr) {
        return Fail("d3d11 device creation returned null device");
    }

    RhiTextureHandle target{};
    RhiStatus status = create_result.device->GetSwapchainColorTarget(target);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 swapchain target query failed");
    }

    const RhiColor clear_color{255U, 0U, 0U, 255U};
    std::vector<std::uint8_t> capture(SMOKE_EXTENT * SMOKE_EXTENT * RGBA8_BYTES_PER_PIXEL);
    status = ClearPresentCapture(*create_result.device, target, clear_color, capture);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 clear present capture path failed");
    }

    if (!BytesMatchColor(capture, clear_color)) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 capture bytes did not match clear color");
    }

    const RhiStatus destroy_status = RhiDeviceFactory::DestroyDevice(create_result.device);
    if (destroy_status != RhiStatus::Success) {
        return Fail("d3d11 device destroy failed");
    }

    return 0;
}

int RunD3D11PrimitiveResourcePipeline() {
    WindowsPlatformWindow window;
    PlatformWindowDesc window_desc{};
    window_desc.title = "YuEngine D3D11 Primitive Hardware Smoke";
    window_desc.client_width = SMOKE_EXTENT;
    window_desc.client_height = SMOKE_EXTENT;
    window_desc.visible = false;

    const PlatformWindowStatus window_status = window.Create(window_desc);
    if (window_status != PlatformWindowStatus::Success) {
        return Skip("d3d11 primitive smoke skipped because a native window could not be created");
    }

    const std::size_t storage_size = RhiDeviceFactory::RequiredDeviceStorageSize(RhiBackendKind::D3D11);
    if (storage_size == 0U) {
        return Skip("d3d11 primitive smoke skipped because the backend is not compiled");
    }

    std::vector<std::byte> storage(storage_size);
    RhiDeviceDesc device_desc{};
    device_desc.backend_kind = RhiBackendKind::D3D11;
    device_desc.native_surface = ConvertSurface(window.GetNativeSurface());
    device_desc.requires_native_surface = true;
    device_desc.requires_swapchain = true;
    device_desc.swapchain.extent = {SMOKE_EXTENT, SMOKE_EXTENT};
    device_desc.command_list_capacity = MAX_COMMANDS;

    const RhiDeviceCreateResult create_result = RhiDeviceFactory::CreateDevice(
        device_desc,
        std::span<std::byte>(storage.data(), storage.size()));
    if (create_result.status == RhiStatus::MissingHardware) {
        return Skip("d3d11 primitive smoke skipped because a hardware D3D11 device is unavailable");
    }

    if (create_result.status != RhiStatus::Success) {
        return Fail("d3d11 primitive device creation failed");
    }

    if (create_result.device == nullptr) {
        return Fail("d3d11 primitive device creation returned null device");
    }

    IRhiDevice &device = *create_result.device;
    const std::uint8_t buffer_bytes[] = {
        1U, 2U, 3U, 4U,
        5U, 6U, 7U, 8U,
        9U, 10U, 11U, 12U,
        13U, 14U, 15U, 16U};
    const std::span<const std::uint8_t> buffer_span(buffer_bytes, sizeof(buffer_bytes));
    RhiBufferDesc buffer_desc{};
    buffer_desc.usage = RhiBufferUsage::Vertex;
    buffer_desc.size_bytes = sizeof(buffer_bytes);
    RhiBufferHandle buffer{};
    RhiStatus status = device.CreateBuffer(buffer_desc, buffer_span, buffer);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive buffer creation failed");
    }

    const std::uint8_t buffer_update_bytes[] = {16U, 15U, 14U, 13U};
    const std::span<const std::uint8_t> buffer_update_span(buffer_update_bytes, sizeof(buffer_update_bytes));
    RhiFenceHandle buffer_fence{};
    status = device.UpdateBuffer(buffer, buffer_update_span, buffer_fence);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive buffer update failed");
    }

    if (buffer_fence.generation == 0U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive buffer update did not signal fence");
    }

    const std::uint8_t texture_bytes[] = {
        255U, 0U, 0U, 255U,
        0U, 255U, 0U, 255U,
        0U, 0U, 255U, 255U,
        255U, 255U, 255U, 255U};
    const std::span<const std::uint8_t> texture_span(texture_bytes, sizeof(texture_bytes));
    RhiTextureDesc texture_desc{};
    texture_desc.format = RhiFormat::Rgba8Unorm;
    texture_desc.extent = {2U, 2U};
    RhiTextureHandle texture{};
    status = device.CreateTexture(texture_desc, texture_span, texture);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive texture creation failed");
    }

    const std::uint8_t texture_update_bytes[] = {
        0U, 0U, 0U, 255U,
        16U, 16U, 16U, 255U,
        32U, 32U, 32U, 255U,
        48U, 48U, 48U, 255U};
    const std::span<const std::uint8_t> texture_update_span(texture_update_bytes, sizeof(texture_update_bytes));
    RhiFenceHandle texture_fence{};
    status = device.UpdateTexture(texture, texture_update_span, texture_fence);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive texture update failed");
    }

    if (texture_fence.generation == 0U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive texture update did not signal fence");
    }

    RhiSamplerDesc sampler_desc{};
    sampler_desc.linear_filter = true;
    RhiSamplerHandle sampler{};
    status = device.CreateSampler(sampler_desc, sampler);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive sampler creation failed");
    }

    const std::span<const std::uint8_t> vertex_bytecode(VERTEX_SHADER_BYTES, sizeof(VERTEX_SHADER_BYTES));
    const RhiShaderModuleDesc vertex_desc{RhiShaderStage::Vertex, vertex_bytecode};
    RhiShaderModuleHandle vertex_shader{};
    status = device.CreateShaderModule(vertex_desc, vertex_shader);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive vertex shader creation failed");
    }

    const std::span<const std::uint8_t> pixel_bytecode(PIXEL_SHADER_BYTES, sizeof(PIXEL_SHADER_BYTES));
    const RhiShaderModuleDesc pixel_desc{RhiShaderStage::Pixel, pixel_bytecode};
    RhiShaderModuleHandle pixel_shader{};
    status = device.CreateShaderModule(pixel_desc, pixel_shader);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive pixel shader creation failed");
    }

    RhiPipelineDesc pipeline_desc{};
    pipeline_desc.vertex_shader = vertex_shader;
    pipeline_desc.pixel_shader = pixel_shader;
    RhiPipelineHandle pipeline{};
    status = device.CreatePipeline(pipeline_desc, pipeline);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive pipeline creation failed");
    }

    const auto created_snapshot = device.Snapshot();
    if (created_snapshot.resources.buffer_count != 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive buffer count was not tracked");
    }

    if (created_snapshot.resources.texture_count != 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive texture count was not tracked");
    }

    if (created_snapshot.resources.sampler_count != 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive sampler count was not tracked");
    }

    if (created_snapshot.resources.shader_module_count != 2U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive shader module count was not tracked");
    }

    if (created_snapshot.resources.pipeline_count != 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive pipeline count was not tracked");
    }

    if (created_snapshot.resources.created_primitive_count != 6U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive created count was not tracked");
    }

    if (created_snapshot.resources.updated_primitive_count != 2U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive update count was not tracked");
    }

    if (created_snapshot.resources.signaled_fence_count != 2U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive fence count was not tracked");
    }

    if (created_snapshot.resources.last_update_bytes != sizeof(texture_update_bytes)) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive last update bytes were not tracked");
    }

    if (device.DestroyPipeline(pipeline) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive pipeline destroy failed");
    }

    if (device.DestroyShaderModule(pixel_shader) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive pixel shader destroy failed");
    }

    if (device.DestroyShaderModule(vertex_shader) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive vertex shader destroy failed");
    }

    if (device.DestroySampler(sampler) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive sampler destroy failed");
    }

    if (device.DestroyTexture(texture) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive texture destroy failed");
    }

    if (device.DestroyBuffer(buffer) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive buffer destroy failed");
    }

    const auto destroyed_snapshot = device.Snapshot();
    if (destroyed_snapshot.resources.destroyed_primitive_count != 6U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive destroyed count was not tracked");
    }

    if (destroyed_snapshot.resources.buffer_count != 0U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive buffer count did not return to zero");
    }

    if (destroyed_snapshot.resources.pipeline_count != 0U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive pipeline count did not return to zero");
    }

    const RhiStatus destroy_status = RhiDeviceFactory::DestroyDevice(create_result.device);
    if (destroy_status != RhiStatus::Success) {
        return Fail("d3d11 primitive device destroy failed");
    }

    return 0;
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail("expected one test name");
    }

    const std::string_view test_name(argv[1]);
    if (test_name == TEST_D3D11_CLEAR_PRESENT_CAPTURE) {
        return RunD3D11ClearPresentCapture();
    }

    if (test_name == TEST_D3D11_PRIMITIVE_RESOURCE_PIPELINE) {
        return RunD3D11PrimitiveResourcePipeline();
    }

    return Fail("unknown test name");
}
