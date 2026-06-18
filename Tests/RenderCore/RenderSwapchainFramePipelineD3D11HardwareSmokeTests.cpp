// Module: Tests RenderCore
// File: Tests/RenderCore/RenderSwapchainFramePipelineD3D11HardwareSmokeTests.cpp

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
#include "YuEngine/RenderCore/RenderSwapchainFramePipeline.h"
#include "YuEngine/RenderCore/RenderSwapchainFramePipelineRequest.h"
#include "YuEngine/RenderCore/RenderSwapchainFramePipelineStatus.h"
#include "YuEngine/Rhi/RhiBackendKind.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiDeviceCreateResult.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiDeviceFactory.h"
#include "YuEngine/Rhi/RhiExtent2D.h"
#include "YuEngine/Rhi/RhiNativeSurfaceDesc.h"
#include "YuEngine/Rhi/RhiStatus.h"

using PlatformNativeSurface = yuengine::platform::PlatformNativeSurface;
using PlatformWindowDesc = yuengine::platform::PlatformWindowDesc;
using yuengine::platform::PlatformWindowStatus;
using WindowsPlatformWindow = yuengine::platform::WindowsPlatformWindow;
using RenderSwapchainFramePipeline = yuengine::rendercore::RenderSwapchainFramePipeline;
using RenderSwapchainFramePipelineRequest = yuengine::rendercore::RenderSwapchainFramePipelineRequest;
using yuengine::rendercore::RenderSwapchainFramePipelineStatus;
using yuengine::rhi::IRhiDevice;
using yuengine::rhi::RhiBackendKind;
using yuengine::rhi::RhiColor;
using yuengine::rhi::RhiDeviceCreateResult;
using yuengine::rhi::RhiDeviceDesc;
using yuengine::rhi::RhiDeviceFactory;
using yuengine::rhi::RhiExtent2D;
using yuengine::rhi::RhiNativeSurfaceDesc;
using yuengine::rhi::RhiStatus;
using yuengine::rhi::MAX_COMMANDS;
using yuengine::rhi::RGBA8_BYTES_PER_PIXEL;

namespace {
constexpr const char *TEST_D3D11_SWAPCHAIN_FRAME =
    "RenderCore_D3D11Hardware_SwapchainFramePipelineResizeClearCapture";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint16_t SMOKE_EXTENT = 4U;
constexpr std::uint16_t RESIZED_WIDTH = 3U;
constexpr std::uint16_t RESIZED_HEIGHT = 2U;
constexpr std::uint32_t FRAME_ID = 1U;
constexpr int SKIP_RETURN_CODE = 77;

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

std::size_t CaptureByteCount(std::uint16_t width, std::uint16_t height) {
    return static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * RGBA8_BYTES_PER_PIXEL;
}

RhiDeviceCreateResult CreateD3D11DeviceForWindow(
    WindowsPlatformWindow &window,
    std::vector<std::byte> &storage,
    const RhiExtent2D &extent) {
    const std::size_t storage_size = RhiDeviceFactory::RequiredDeviceStorageSize(RhiBackendKind::D3D11);
    if (storage_size == 0U) {
        return RhiDeviceCreateResult{RhiStatus::UnsupportedBackend, nullptr, {}};
    }

    storage.resize(storage_size);
    RhiDeviceDesc device_desc{};
    device_desc.backend_kind = RhiBackendKind::D3D11;
    device_desc.native_surface = ConvertSurface(window.GetNativeSurface());
    device_desc.requires_native_surface = true;
    device_desc.requires_swapchain = true;
    device_desc.swapchain.extent = extent;
    device_desc.command_list_capacity = MAX_COMMANDS;

    const std::span<std::byte> storage_span(storage.data(), storage.size());
    return RhiDeviceFactory::CreateDevice(device_desc, storage_span);
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

int RunD3D11SwapchainFramePipelineResizeClearCapture() {
    WindowsPlatformWindow window;
    PlatformWindowDesc window_desc{};
    window_desc.title = "YuEngine RenderCore D3D11 Swapchain Frame Pipeline Smoke";
    window_desc.client_width = SMOKE_EXTENT;
    window_desc.client_height = SMOKE_EXTENT;
    window_desc.visible = false;

    const PlatformWindowStatus window_status = window.Create(window_desc);
    if (window_status != PlatformWindowStatus::Success) {
        return Skip("rendercore d3d11 swapchain frame smoke skipped because a native window could not be created");
    }

    const RhiExtent2D initial_extent{SMOKE_EXTENT, SMOKE_EXTENT};
    std::vector<std::byte> storage{};
    const RhiDeviceCreateResult create_result = CreateD3D11DeviceForWindow(window, storage, initial_extent);
    if (create_result.status == RhiStatus::UnsupportedBackend) {
        return Skip("rendercore d3d11 swapchain frame smoke skipped because the backend is not compiled");
    }

    if (create_result.status == RhiStatus::MissingHardware) {
        return Skip("rendercore d3d11 swapchain frame smoke skipped because a hardware D3D11 device is unavailable");
    }

    if (create_result.status != RhiStatus::Success) {
        return Fail("rendercore d3d11 swapchain frame device creation failed");
    }

    if (create_result.device == nullptr) {
        return Fail("rendercore d3d11 swapchain frame device creation returned null device");
    }

    IRhiDevice &device = *create_result.device;
    RenderSwapchainFramePipeline pipeline;
    const RhiColor clear_color{32U, 96U, 160U, 255U};
    std::vector<std::uint8_t> capture(CaptureByteCount(RESIZED_WIDTH, RESIZED_HEIGHT));
    RenderSwapchainFramePipelineRequest request{};
    request.rhi_device = &device;
    request.clear_color = clear_color;
    request.resize_request.extent = {RESIZED_WIDTH, RESIZED_HEIGHT};
    request.resize_before_submit = true;
    request.capture_output = std::span<std::uint8_t>(capture.data(), capture.size());
    request.capture_byte_budget = capture.size();
    request.frame_id = FRAME_ID;

    const auto result = pipeline.Execute(request);
    if (result.status != RenderSwapchainFramePipelineStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 swapchain frame pipeline failed");
    }

    if (result.rhi_status != RhiStatus::Success || result.capture_bytes_written != capture.size()) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 swapchain frame pipeline reported wrong result");
    }

    if (!result.resized) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 swapchain frame pipeline did not report resize");
    }

    if (!BytesMatchColor(capture, clear_color)) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 swapchain frame capture did not match clear color");
    }

    const auto rhi_snapshot = device.Snapshot();
    if (rhi_snapshot.swapchain.resize_count != 1U ||
        rhi_snapshot.submit_count != 1U ||
        rhi_snapshot.present_count != 1U ||
        rhi_snapshot.capture_count != 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 swapchain frame did not update rhi counters");
    }

    const auto pipeline_snapshot = pipeline.Snapshot();
    if (pipeline_snapshot.completed_frame_count != 1U ||
        pipeline_snapshot.resized_frame_count != 1U ||
        pipeline_snapshot.last_capture_bytes_written != capture.size()) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("rendercore d3d11 swapchain frame did not update pipeline counters");
    }

    const RhiStatus destroy_status = RhiDeviceFactory::DestroyDevice(create_result.device);
    if (destroy_status != RhiStatus::Success) {
        return Fail("rendercore d3d11 swapchain frame device destroy failed");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_D3D11_SWAPCHAIN_FRAME) {
        return RunD3D11SwapchainFramePipelineResizeClearCapture();
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
