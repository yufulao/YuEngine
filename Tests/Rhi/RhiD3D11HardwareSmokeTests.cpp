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
#include "YuEngine/Rhi/RhiCaptureResult.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiCommandList.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiDeviceCreateResult.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiDeviceFactory.h"
#include "YuEngine/Rhi/RhiNativeSurfaceDesc.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

using PlatformNativeSurface = yuengine::platform::PlatformNativeSurface;
using PlatformWindowDesc = yuengine::platform::PlatformWindowDesc;
using yuengine::platform::PlatformWindowStatus;
using WindowsPlatformWindow = yuengine::platform::WindowsPlatformWindow;
using yuengine::rhi::IRhiDevice;
using yuengine::rhi::MAX_COMMANDS;
using yuengine::rhi::RGBA8_BYTES_PER_PIXEL;
using yuengine::rhi::RhiBackendKind;
using yuengine::rhi::RhiCaptureResult;
using yuengine::rhi::RhiColor;
using yuengine::rhi::RhiCommandList;
using yuengine::rhi::RhiDeviceCreateResult;
using yuengine::rhi::RhiDeviceDesc;
using yuengine::rhi::RhiDeviceFactory;
using yuengine::rhi::RhiNativeSurfaceDesc;
using yuengine::rhi::RhiStatus;
using yuengine::rhi::RhiTextureHandle;

namespace {
constexpr const char *TEST_D3D11_CLEAR_PRESENT_CAPTURE = "RHI_D3D11Hardware_ClearPresentCaptureBytes";
constexpr std::uint32_t SMOKE_EXTENT = 4U;
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
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail("expected one test name");
    }

    const std::string_view test_name(argv[1]);
    if (test_name == TEST_D3D11_CLEAR_PRESENT_CAPTURE) {
        return RunD3D11ClearPresentCapture();
    }

    return Fail("unknown test name");
}
