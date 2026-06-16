// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Src/D3D11RhiDeviceWindows.cpp

#include "YuEngine/Rhi/RhiDeviceFactory.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <new>

#include "YuEngine/Rhi/NullRhiDevice.h"
#include "YuEngine/Rhi/RhiConstants.h"

#if defined(_WIN32)
#include "D3D11RhiDeviceInternal.h"
#endif

namespace yuengine::rhi {
namespace {
constexpr std::uint32_t SWAPCHAIN_TARGET_SLOT = 0U;
constexpr std::uint32_t SWAPCHAIN_TARGET_GENERATION = 1U;
constexpr std::size_t NO_STORAGE_REQUIRED = 0U;

bool IsStorageAligned(std::span<std::byte> device_storage, std::size_t alignment) {
    if (device_storage.data() == nullptr) {
        return false;
    }

    const auto address = reinterpret_cast<std::uintptr_t>(device_storage.data());
    return (address % alignment) == 0U;
}

bool IsStorageValid(std::span<std::byte> device_storage, std::size_t required_size, std::size_t required_alignment) {
    if (required_size == NO_STORAGE_REQUIRED) {
        return false;
    }

    if (device_storage.size() < required_size) {
        return false;
    }

    return IsStorageAligned(device_storage, required_alignment);
}
}

#if defined(_WIN32)
D3D11RhiDevice::D3D11RhiDevice()
    : device_(nullptr),
      context_(nullptr),
      swapchain_(nullptr),
      backbuffer_(nullptr),
      render_target_view_(nullptr),
      capture_texture_(nullptr),
      capabilities_{},
      snapshot_{},
      swapchain_desc_{},
      swapchain_target_{SWAPCHAIN_TARGET_SLOT, SWAPCHAIN_TARGET_GENERATION},
      initialized_(false),
      submitted_(false),
      presented_(false) {
}

D3D11RhiDevice::~D3D11RhiDevice() {
    ReleaseResources();
}

RhiStatus D3D11RhiDevice::Initialize(const RhiDeviceDesc &desc) {
    ReleaseResources();

    const RhiStatus desc_status = ValidateDesc(desc);
    if (desc_status != RhiStatus::Success) {
        return RecordFailure(desc_status);
    }

    const RhiStatus native_status = CreateNativeObjects(desc);
    if (native_status != RhiStatus::Success) {
        ReleaseResources();
        return RecordFailure(native_status);
    }

    const RhiStatus backbuffer_status = CreateBackbufferObjects();
    if (backbuffer_status != RhiStatus::Success) {
        ReleaseResources();
        return RecordFailure(backbuffer_status);
    }

    swapchain_desc_ = desc.swapchain;
    const RhiStatus capture_status = CreateCaptureTexture();
    if (capture_status != RhiStatus::Success) {
        ReleaseResources();
        return RecordFailure(capture_status);
    }

    capabilities_ = RhiCapabilities{
        RhiBackendKind::D3D11,
        desc.swapchain.color_format,
        1U,
        desc.command_list_capacity,
        MAX_COLOR_TARGET_EXTENT,
        MAX_CAPTURE_FIXTURE_EXTENT,
        true,
        true,
        true,
        true};
    snapshot_ = RhiDeviceSnapshot{};
    snapshot_.color_target_capacity = 1U;
    snapshot_.color_target_count = 1U;
    snapshot_.created_target_count = 1U;
    snapshot_.swapchain.extent = desc.swapchain.extent;
    snapshot_.swapchain.color_format = desc.swapchain.color_format;
    snapshot_.swapchain.color_target = swapchain_target_;
    snapshot_.swapchain.valid = true;
    initialized_ = true;
    submitted_ = false;
    presented_ = false;
    return RhiStatus::Success;
}

RhiStatus D3D11RhiDevice::CreateColorTarget(const RhiColorTargetDesc &desc, RhiTextureHandle &out_handle) {
    out_handle = RhiTextureHandle{};
    static_cast<void>(desc);
    return RecordFailure(RhiStatus::UnsupportedBackend);
}

RhiStatus D3D11RhiDevice::GetSwapchainColorTarget(RhiTextureHandle &out_handle) const {
    if (!initialized_) {
        out_handle = RhiTextureHandle{};
        return RhiStatus::InvalidLifecycle;
    }

    out_handle = swapchain_target_;
    return RhiStatus::Success;
}

RhiStatus D3D11RhiDevice::DestroyTarget(RhiTextureHandle handle) {
    if (!IsSwapchainTarget(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    return RecordFailure(RhiStatus::InvalidLifecycle);
}

RhiStatus D3D11RhiDevice::RecordClear(RhiCommandList &command_list, RhiTextureHandle handle, RhiColor color) {
    if (!IsSwapchainTarget(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    const RhiStatus record_status = command_list.RecordClear(handle, color);
    if (record_status != RhiStatus::Success) {
        return RecordFailure(record_status);
    }

    ++snapshot_.recorded_command_count;
    return RhiStatus::Success;
}

RhiStatus D3D11RhiDevice::Submit(const RhiCommandList &command_list) {
    if (!initialized_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (!command_list.IsComplete()) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (command_list.Capacity() > capabilities_.command_list_capacity) {
        return RecordFailure(RhiStatus::CapacityExceeded);
    }

    if (!IsSwapchainTarget(command_list.TargetHandle())) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    snapshot_.command_storage_capacity_before_frame = command_list.Capacity();
    for (std::size_t index = 0U; index < command_list.CommandCount(); ++index) {
        const RhiCommandRecord &command = command_list.CommandAt(index);
        if (!IsSwapchainTarget(command.target)) {
            return RecordFailure(RhiStatus::InvalidHandle);
        }

        if (command.type != RhiCommandType::ClearColor) {
            continue;
        }

        const float color[4] = {
            static_cast<float>(command.color.r) / 255.0F,
            static_cast<float>(command.color.g) / 255.0F,
            static_cast<float>(command.color.b) / 255.0F,
            static_cast<float>(command.color.a) / 255.0F};
        context_->ClearRenderTargetView(render_target_view_, color);
    }

    submitted_ = true;
    presented_ = false;
    ++snapshot_.submit_count;
    snapshot_.command_storage_capacity_after_last_frame = command_list.Capacity();
    return RhiStatus::Success;
}

RhiStatus D3D11RhiDevice::Present() {
    if (!submitted_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    const RhiStatus copy_status = CopyBackbufferToCaptureTexture();
    if (copy_status != RhiStatus::Success) {
        return RecordFailure(copy_status);
    }

    const UINT sync_interval = swapchain_desc_.vsync_enabled ? 1U : 0U;
    const HRESULT present_result = swapchain_->Present(sync_interval, 0U);
    if (FAILED(present_result)) {
        return RecordFailure(TranslateNativeFailure(present_result));
    }

    presented_ = true;
    ++snapshot_.present_count;
    snapshot_.swapchain.presented = true;
    return RhiStatus::Success;
}

RhiCaptureResult D3D11RhiDevice::CapturePresentedTarget(std::span<std::uint8_t> destination) {
    if (!presented_) {
        RecordFailure(RhiStatus::InvalidLifecycle);
        return RhiCaptureResult{RhiStatus::InvalidLifecycle, 0U};
    }

    const std::size_t byte_count = CaptureByteCount();
    if (destination.size() < byte_count) {
        RecordFailure(RhiStatus::CapacityExceeded);
        snapshot_.last_capture_bytes_written = 0U;
        return RhiCaptureResult{RhiStatus::CapacityExceeded, 0U};
    }

    D3D11_MAPPED_SUBRESOURCE mapped_resource{};
    const HRESULT map_result = context_->Map(capture_texture_, 0U, D3D11_MAP_READ, 0U, &mapped_resource);
    if (FAILED(map_result)) {
        const RhiStatus status = TranslateNativeFailure(map_result);
        RecordFailure(status);
        snapshot_.last_capture_bytes_written = 0U;
        return RhiCaptureResult{status, 0U};
    }

    const std::size_t row_bytes = static_cast<std::size_t>(swapchain_desc_.extent.width) * RGBA8_BYTES_PER_PIXEL;
    const auto *source_bytes = static_cast<const std::uint8_t *>(mapped_resource.pData);
    for (std::uint16_t row = 0U; row < swapchain_desc_.extent.height; ++row) {
        const std::size_t source_offset = static_cast<std::size_t>(mapped_resource.RowPitch) * static_cast<std::size_t>(row);
        const std::size_t destination_offset = row_bytes * static_cast<std::size_t>(row);
        std::memcpy(destination.data() + destination_offset, source_bytes + source_offset, row_bytes);
    }

    context_->Unmap(capture_texture_, 0U);
    ++snapshot_.capture_count;
    snapshot_.last_capture_bytes_written = byte_count;
    return RhiCaptureResult{RhiStatus::Success, byte_count};
}

RhiCapabilities D3D11RhiDevice::Capabilities() const {
    return capabilities_;
}

RhiDeviceSnapshot D3D11RhiDevice::Snapshot() const {
    return snapshot_;
}

void D3D11RhiDevice::ReleaseResources() {
    if (capture_texture_ != nullptr) {
        capture_texture_->Release();
        capture_texture_ = nullptr;
    }

    if (render_target_view_ != nullptr) {
        render_target_view_->Release();
        render_target_view_ = nullptr;
    }

    if (backbuffer_ != nullptr) {
        backbuffer_->Release();
        backbuffer_ = nullptr;
    }

    if (swapchain_ != nullptr) {
        swapchain_->Release();
        swapchain_ = nullptr;
    }

    if (context_ != nullptr) {
        context_->Release();
        context_ = nullptr;
    }

    if (device_ != nullptr) {
        device_->Release();
        device_ = nullptr;
    }

    capabilities_ = RhiCapabilities{};
    snapshot_ = RhiDeviceSnapshot{};
    swapchain_desc_ = RhiSwapchainDesc{};
    initialized_ = false;
    submitted_ = false;
    presented_ = false;
}

RhiStatus D3D11RhiDevice::RecordFailure(RhiStatus status) {
    ++snapshot_.failed_operation_count;
    return status;
}

RhiStatus D3D11RhiDevice::ValidateDesc(const RhiDeviceDesc &desc) const {
    if (desc.backend_kind != RhiBackendKind::D3D11) {
        return RhiStatus::UnsupportedBackend;
    }

    if (!desc.requires_native_surface) {
        return RhiStatus::InvalidDescriptor;
    }

    const RhiStatus surface_status = RhiDeviceFactory::ValidateNativeSurfaceDesc(desc.native_surface);
    if (surface_status != RhiStatus::Success) {
        return surface_status;
    }

    const HWND window_handle = reinterpret_cast<HWND>(desc.native_surface.window_value);
    if (IsWindow(window_handle) == FALSE) {
        return RhiStatus::InvalidDescriptor;
    }

    if (!desc.requires_swapchain) {
        return RhiStatus::InvalidDescriptor;
    }

    if (!IsSwapchainDescValid(desc.swapchain)) {
        return RhiStatus::InvalidDescriptor;
    }

    if (desc.command_list_capacity == 0U) {
        return RhiStatus::InvalidDescriptor;
    }

    if (desc.command_list_capacity > MAX_COMMANDS) {
        return RhiStatus::CapacityExceeded;
    }

    return RhiStatus::Success;
}

RhiStatus D3D11RhiDevice::CreateNativeObjects(const RhiDeviceDesc &desc) {
    DXGI_SWAP_CHAIN_DESC native_swapchain_desc{};
    native_swapchain_desc.BufferDesc.Width = desc.swapchain.extent.width;
    native_swapchain_desc.BufferDesc.Height = desc.swapchain.extent.height;
    native_swapchain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    native_swapchain_desc.BufferDesc.RefreshRate.Numerator = 0U;
    native_swapchain_desc.BufferDesc.RefreshRate.Denominator = 1U;
    native_swapchain_desc.SampleDesc.Count = 1U;
    native_swapchain_desc.SampleDesc.Quality = 0U;
    native_swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    native_swapchain_desc.BufferCount = 1U;
    native_swapchain_desc.OutputWindow = reinterpret_cast<HWND>(desc.native_surface.window_value);
    native_swapchain_desc.Windowed = TRUE;
    native_swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    const D3D_FEATURE_LEVEL requested_levels[] = {D3D_FEATURE_LEVEL_11_0};
    D3D_FEATURE_LEVEL accepted_level = D3D_FEATURE_LEVEL_11_0;
    const UINT create_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    const HRESULT create_result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        create_flags,
        requested_levels,
        1U,
        D3D11_SDK_VERSION,
        &native_swapchain_desc,
        &swapchain_,
        &device_,
        &accepted_level,
        &context_);
    if (FAILED(create_result)) {
        return TranslateNativeFailure(create_result);
    }

    return RhiStatus::Success;
}

RhiStatus D3D11RhiDevice::CreateBackbufferObjects() {
    const HRESULT buffer_result = swapchain_->GetBuffer(0U, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&backbuffer_));
    if (FAILED(buffer_result)) {
        return TranslateNativeFailure(buffer_result);
    }

    const HRESULT view_result = device_->CreateRenderTargetView(backbuffer_, nullptr, &render_target_view_);
    if (FAILED(view_result)) {
        return TranslateNativeFailure(view_result);
    }

    return RhiStatus::Success;
}

RhiStatus D3D11RhiDevice::CreateCaptureTexture() {
    D3D11_TEXTURE2D_DESC capture_desc{};
    capture_desc.Width = swapchain_desc_.extent.width;
    capture_desc.Height = swapchain_desc_.extent.height;
    capture_desc.MipLevels = 1U;
    capture_desc.ArraySize = 1U;
    capture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    capture_desc.SampleDesc.Count = 1U;
    capture_desc.SampleDesc.Quality = 0U;
    capture_desc.Usage = D3D11_USAGE_STAGING;
    capture_desc.BindFlags = 0U;
    capture_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    capture_desc.MiscFlags = 0U;

    const HRESULT texture_result = device_->CreateTexture2D(&capture_desc, nullptr, &capture_texture_);
    if (FAILED(texture_result)) {
        return TranslateNativeFailure(texture_result);
    }

    return RhiStatus::Success;
}

RhiStatus D3D11RhiDevice::CopyBackbufferToCaptureTexture() {
    if (backbuffer_ == nullptr) {
        return RhiStatus::InvalidLifecycle;
    }

    if (capture_texture_ == nullptr) {
        return RhiStatus::InvalidLifecycle;
    }

    context_->CopyResource(capture_texture_, backbuffer_);
    return RhiStatus::Success;
}

RhiStatus D3D11RhiDevice::TranslateNativeFailure(HRESULT native_result) const {
    if (native_result == DXGI_ERROR_DEVICE_REMOVED || native_result == DXGI_ERROR_DEVICE_RESET) {
        return RhiStatus::DeviceLost;
    }

    if (native_result == DXGI_ERROR_UNSUPPORTED || native_result == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE) {
        return RhiStatus::MissingHardware;
    }

    return RhiStatus::InvalidLifecycle;
}

bool D3D11RhiDevice::IsSwapchainTarget(RhiTextureHandle handle) const {
    if (!initialized_) {
        return false;
    }

    if (handle.slot != swapchain_target_.slot) {
        return false;
    }

    return handle.generation == swapchain_target_.generation;
}

bool D3D11RhiDevice::IsSwapchainDescValid(const RhiSwapchainDesc &desc) const {
    if (desc.color_format != RhiFormat::Rgba8Unorm) {
        return false;
    }

    if (desc.extent.width == 0U) {
        return false;
    }

    if (desc.extent.height == 0U) {
        return false;
    }

    if (desc.extent.width > MAX_CAPTURE_FIXTURE_EXTENT) {
        return false;
    }

    if (desc.extent.height > MAX_CAPTURE_FIXTURE_EXTENT) {
        return false;
    }

    return true;
}

std::size_t D3D11RhiDevice::CaptureByteCount() const {
    return static_cast<std::size_t>(swapchain_desc_.extent.width) *
        static_cast<std::size_t>(swapchain_desc_.extent.height) * RGBA8_BYTES_PER_PIXEL;
}
#endif

RhiDeviceCreateResult RhiDeviceFactory::CreateDevice(const RhiDeviceDesc &desc, std::span<std::byte> device_storage) {
    const std::size_t required_size = RequiredDeviceStorageSize(desc.backend_kind);
    const std::size_t required_alignment = RequiredDeviceStorageAlignment(desc.backend_kind);
    if (!IsStorageValid(device_storage, required_size, required_alignment)) {
        return RhiDeviceCreateResult{RhiStatus::InvalidDescriptor, nullptr, RhiCapabilities{}};
    }

    if (desc.backend_kind == RhiBackendKind::Null) {
        auto *null_device = new (device_storage.data()) NullRhiDevice();
        const RhiDeviceCreateResult result = CreateDevice(desc, null_device);
        if (result.status != RhiStatus::Success) {
            null_device->~NullRhiDevice();
        }

        return result;
    }

#if defined(_WIN32)
    if (desc.backend_kind == RhiBackendKind::D3D11) {
        auto *d3d11_device = new (device_storage.data()) D3D11RhiDevice();
        const RhiStatus initialize_status = d3d11_device->Initialize(desc);
        if (initialize_status != RhiStatus::Success) {
            d3d11_device->~D3D11RhiDevice();
            return RhiDeviceCreateResult{initialize_status, nullptr, RhiCapabilities{}};
        }

        return RhiDeviceCreateResult{RhiStatus::Success, d3d11_device, d3d11_device->Capabilities()};
    }
#endif

    return RhiDeviceCreateResult{RhiStatus::UnsupportedBackend, nullptr, RhiCapabilities{}};
}

RhiStatus RhiDeviceFactory::DestroyDevice(IRhiDevice *device) {
    if (device == nullptr) {
        return RhiStatus::InvalidDescriptor;
    }

    const RhiBackendKind backend_kind = device->Capabilities().backend_kind;
    if (backend_kind == RhiBackendKind::Null) {
        auto *null_device = static_cast<NullRhiDevice *>(device);
        null_device->~NullRhiDevice();
        return RhiStatus::Success;
    }

#if defined(_WIN32)
    if (backend_kind == RhiBackendKind::D3D11) {
        auto *d3d11_device = static_cast<D3D11RhiDevice *>(device);
        d3d11_device->~D3D11RhiDevice();
        return RhiStatus::Success;
    }
#endif

    return RhiStatus::UnsupportedBackend;
}

std::size_t RhiDeviceFactory::RequiredDeviceStorageSize(RhiBackendKind backend_kind) {
    if (backend_kind == RhiBackendKind::Null) {
        return sizeof(NullRhiDevice);
    }

#if defined(_WIN32)
    if (backend_kind == RhiBackendKind::D3D11) {
        return sizeof(D3D11RhiDevice);
    }
#endif

    return NO_STORAGE_REQUIRED;
}

std::size_t RhiDeviceFactory::RequiredDeviceStorageAlignment(RhiBackendKind backend_kind) {
    if (backend_kind == RhiBackendKind::Null) {
        return alignof(NullRhiDevice);
    }

#if defined(_WIN32)
    if (backend_kind == RhiBackendKind::D3D11) {
        return alignof(D3D11RhiDevice);
    }
#endif

    return NO_STORAGE_REQUIRED;
}
}
