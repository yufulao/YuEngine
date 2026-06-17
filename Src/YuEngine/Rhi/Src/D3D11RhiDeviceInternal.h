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

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/RhiCapabilities.h"
#include "YuEngine/Rhi/RhiCaptureResult.h"
#include "YuEngine/Rhi/RhiColorTargetDesc.h"
#include "YuEngine/Rhi/RhiCommandList.h"
#include "YuEngine/Rhi/RhiConstants.h"
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
    RhiStatus RecordBindPipeline(RhiCommandList &command_list, RhiPipelineHandle handle) override;
    RhiStatus RecordBindVertexBuffer(RhiCommandList &command_list, const RhiVertexBufferView &view) override;
    RhiStatus RecordBindIndexBuffer(RhiCommandList &command_list, const RhiIndexBufferView &view) override;
    RhiStatus RecordDraw(RhiCommandList &command_list, const RhiDrawDesc &desc) override;
    RhiStatus RecordDrawIndexed(RhiCommandList &command_list, const RhiDrawIndexedDesc &desc) override;
    RhiStatus Submit(const RhiCommandList &command_list) override;
    RhiStatus Present() override;
    RhiCaptureResult CapturePresentedTarget(std::span<std::uint8_t> destination) override;
    RhiStatus CreateBuffer(
        const RhiBufferDesc &desc,
        std::span<const std::uint8_t> initial_bytes,
        RhiBufferHandle &out_handle) override;
    RhiStatus UpdateBuffer(
        RhiBufferHandle handle,
        std::span<const std::uint8_t> bytes,
        RhiFenceHandle &out_fence) override;
    RhiStatus DestroyBuffer(RhiBufferHandle handle) override;
    RhiStatus CreateTexture(
        const RhiTextureDesc &desc,
        std::span<const std::uint8_t> initial_bytes,
        RhiTextureHandle &out_handle) override;
    RhiStatus UpdateTexture(
        RhiTextureHandle handle,
        std::span<const std::uint8_t> bytes,
        RhiFenceHandle &out_fence) override;
    RhiStatus DestroyTexture(RhiTextureHandle handle) override;
    RhiStatus CreateSampler(const RhiSamplerDesc &desc, RhiSamplerHandle &out_handle) override;
    RhiStatus DestroySampler(RhiSamplerHandle handle) override;
    RhiStatus CreateShaderModule(const RhiShaderModuleDesc &desc, RhiShaderModuleHandle &out_handle) override;
    RhiStatus DestroyShaderModule(RhiShaderModuleHandle handle) override;
    RhiStatus CreatePipeline(const RhiPipelineDesc &desc, RhiPipelineHandle &out_handle) override;
    RhiStatus DestroyPipeline(RhiPipelineHandle handle) override;
    RhiCapabilities Capabilities() const override;
    RhiDeviceSnapshot Snapshot() const override;

private:
    struct D3D11BufferSlot final {
        ID3D11Buffer *buffer = nullptr;
        RhiBufferDesc desc{};
        std::uint32_t generation = 0U;
        bool is_active = false;
    };

    struct D3D11TextureSlot final {
        ID3D11Texture2D *texture = nullptr;
        RhiTextureDesc desc{};
        std::uint32_t generation = 0U;
        bool is_active = false;
    };

    struct D3D11SamplerSlot final {
        ID3D11SamplerState *sampler = nullptr;
        RhiSamplerDesc desc{};
        std::uint32_t generation = 0U;
        bool is_active = false;
    };

    struct D3D11ShaderModuleSlot final {
        ID3D11VertexShader *vertex_shader = nullptr;
        ID3D11PixelShader *pixel_shader = nullptr;
        RhiShaderStage stage = RhiShaderStage::Unsupported;
        std::array<std::uint8_t, MAX_RHI_SHADER_BYTECODE_BYTES> bytecode{};
        std::size_t bytecode_size = 0U;
        std::uint32_t generation = 0U;
        bool is_active = false;
    };

    struct D3D11PipelineSlot final {
        ID3D11InputLayout *input_layout = nullptr;
        RhiPipelineDesc desc{};
        std::uint32_t generation = 0U;
        bool is_active = false;
    };

    void ReleaseResources();
    void ReleasePrimitiveResources();
    void InitializePrimitiveSlots();
    RhiStatus RecordFailure(RhiStatus status);
    RhiStatus RecordIndexedDrawFailure(RhiStatus status);
    RhiStatus ValidateDesc(const RhiDeviceDesc &desc) const;
    RhiStatus CreateNativeObjects(const RhiDeviceDesc &desc);
    RhiStatus CreateBackbufferObjects();
    RhiStatus CreateCaptureTexture();
    RhiStatus CopyBackbufferToCaptureTexture();
    RhiStatus TranslateNativeFailure(HRESULT native_result) const;
    bool IsSwapchainTarget(RhiTextureHandle handle) const;
    bool IsBufferHandleValid(RhiBufferHandle handle) const;
    bool IsTextureHandleValid(RhiTextureHandle handle) const;
    bool IsSamplerHandleValid(RhiSamplerHandle handle) const;
    bool IsShaderModuleHandleValid(RhiShaderModuleHandle handle) const;
    bool IsPipelineHandleValid(RhiPipelineHandle handle) const;
    bool IsVertexBufferViewValid(const RhiVertexBufferView &view) const;
    bool IsIndexBufferViewValid(const RhiIndexBufferView &view) const;
    bool IsDrawDescValid(const RhiDrawDesc &desc) const;
    bool IsDrawIndexedDescValid(const RhiDrawIndexedDesc &desc) const;
    bool IsSwapchainDescValid(const RhiSwapchainDesc &desc) const;
    bool IsBufferDescValid(const RhiBufferDesc &desc, std::span<const std::uint8_t> initial_bytes) const;
    bool IsTextureDescValid(const RhiTextureDesc &desc, std::span<const std::uint8_t> initial_bytes) const;
    bool IsShaderModuleDescValid(const RhiShaderModuleDesc &desc) const;
    bool IsPipelineDescValid(const RhiPipelineDesc &desc) const;
    bool IsInputLayoutDescValid(const RhiInputLayoutDesc &desc) const;
    bool IsDrawRangeValid(const RhiVertexBufferView &view, const RhiDrawDesc &desc) const;
    bool IsIndexedDrawRangeValid(const RhiIndexBufferView &view, const RhiDrawIndexedDesc &desc) const;
    std::size_t CaptureByteCount() const;
    std::size_t TextureByteCount(const RhiTextureDesc &desc) const;
    std::size_t IndexFormatByteCount(RhiIndexFormat format) const;
    UINT NativeBindFlagsForBuffer(RhiBufferUsage usage) const;
    DXGI_FORMAT NativeIndexFormat(RhiIndexFormat format) const;
    RhiFenceHandle SignalFence(std::size_t byte_count);

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
    std::array<D3D11BufferSlot, MAX_RHI_BUFFERS> buffers_;
    std::array<D3D11TextureSlot, MAX_RHI_TEXTURES> textures_;
    std::array<D3D11SamplerSlot, MAX_RHI_SAMPLERS> samplers_;
    std::array<D3D11ShaderModuleSlot, MAX_RHI_SHADER_MODULES> shader_modules_;
    std::array<D3D11PipelineSlot, MAX_RHI_PIPELINES> pipelines_;
    std::uint32_t primitive_generation_seed_;
    std::uint32_t fence_generation_;
    bool initialized_;
    bool submitted_;
    bool presented_;
};
}
#endif
