// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Src/D3D11RhiDeviceInternal.h

#pragma once

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <d3d11.h>
#include <dxgi.h>
#include <windows.h>

#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/RhiCapabilities.h"
#include "YuEngine/Rhi/RhiCaptureResult.h"
#include "YuEngine/Rhi/RhiColorTargetDesc.h"
#include "YuEngine/Rhi/RhiCommandList.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiDeviceSnapshot.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::rhi {
class D3D11RhiDevice final : public IRhiDevice {
public:
    /**
     * @comment Constructs an empty D3D11 RHI device.
     */
    D3D11RhiDevice();
    /**
     * @comment Releases native D3D11 objects owned by the instance.
     */
    ~D3D11RhiDevice() override;

    D3D11RhiDevice(const D3D11RhiDevice &other) = delete;
    D3D11RhiDevice &operator=(const D3D11RhiDevice &other) = delete;

    RhiStatus Initialize(const RhiDeviceDesc &desc) override;
    RhiStatus CreateColorTarget(const RhiColorTargetDesc &desc, RhiTextureHandle &out_handle) override;
    RhiStatus GetSwapchainColorTarget(RhiTextureHandle &out_handle) const override;
    RhiStatus DestroyTarget(RhiTextureHandle handle) override;
    RhiStatus RecordClear(RhiCommandList &command_list, RhiTextureHandle handle, RhiColor color) override;
    RhiStatus Submit(const RhiCommandList &command_list) override;
    RhiStatus Present() override;
    RhiCaptureResult CapturePresentedTarget(std::span<std::uint8_t> destination) override;
    RhiCapabilities Capabilities() const override;
    RhiDeviceSnapshot Snapshot() const override;

private:
    void ReleaseResources();
    RhiStatus RecordFailure(RhiStatus status);
    RhiStatus ValidateDesc(const RhiDeviceDesc &desc) const;
    RhiStatus CreateNativeObjects(const RhiDeviceDesc &desc);
    RhiStatus CreateBackbufferObjects();
    RhiStatus CreateCaptureTexture();
    RhiStatus CopyBackbufferToCaptureTexture();
    RhiStatus TranslateNativeFailure(HRESULT native_result) const;
    bool IsSwapchainTarget(RhiTextureHandle handle) const;
    bool IsSwapchainDescValid(const RhiSwapchainDesc &desc) const;
    std::size_t CaptureByteCount() const;

    ID3D11Device *device_;
    ID3D11DeviceContext *context_;
    IDXGISwapChain *swapchain_;
    ID3D11Texture2D *backbuffer_;
    ID3D11RenderTargetView *render_target_view_;
    ID3D11Texture2D *capture_texture_;
    RhiCapabilities capabilities_;
    RhiDeviceSnapshot snapshot_;
    RhiSwapchainDesc swapchain_desc_;
    RhiTextureHandle swapchain_target_;
    bool initialized_;
    bool submitted_;
    bool presented_;
};
}
#endif
