// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/NullRhiDevice.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/RhiCapabilities.h"
#include "YuEngine/Rhi/RhiCaptureResult.h"
#include "YuEngine/Rhi/RhiColorTargetDesc.h"
#include "YuEngine/Rhi/RhiCommandList.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiDeviceSnapshot.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTargetSlot.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::rhi {
class NullRhiDevice final : public IRhiDevice {
public:
    /**
     * @comment Constructs a NullRhiDevice instance.
     */
    NullRhiDevice();

    /**
     * @comment Initializes the instance.
     * @param desc Input descriptor.
     * @return Explicit operation status.
     */
    RhiStatus Initialize(const RhiDeviceDesc &desc) override;
    /**
     * @comment Creates color target.
     * @param desc Input descriptor.
     * @param out_handle Output handle written on success.
     * @return Explicit operation status.
     */
    RhiStatus CreateColorTarget(const RhiColorTargetDesc &desc, RhiTextureHandle &out_handle) override;
    /**
     * @comment Returns the swapchain color target handle.
     * @param out_handle Output handle written on success.
     * @return Explicit operation status.
     */
    RhiStatus GetSwapchainColorTarget(RhiTextureHandle &out_handle) const override;
    /**
     * @comment Rejects swapchain resize for the null backend.
     * @param request Input resize request.
     * @param out_result Output resize result.
     * @return Explicit operation status.
     */
    RhiStatus ResizeSwapchain(
        const RhiSwapchainResizeRequest &request,
        RhiSwapchainResizeResult &out_result) override;
    /**
     * @comment Destroys target.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    RhiStatus DestroyTarget(RhiTextureHandle handle) override;
    /**
     * @comment Records clear.
     * @param command_list Command list updated by the function.
     * @param handle Input handle.
     * @param color Input color.
     * @return Explicit operation status.
     */
    RhiStatus RecordClear(RhiCommandList &command_list, RhiTextureHandle handle, RhiColor color) override;
    RhiStatus RecordBindPipeline(RhiCommandList &command_list, RhiPipelineHandle handle) override;
    RhiStatus RecordBindVertexBuffer(RhiCommandList &command_list, const RhiVertexBufferView &view) override;
    RhiStatus RecordBindIndexBuffer(RhiCommandList &command_list, const RhiIndexBufferView &view) override;
    RhiStatus RecordBindSampledTexture(
        RhiCommandList &command_list,
        const RhiSampledTextureBinding &binding) override;
    RhiStatus RecordBindSampler(RhiCommandList &command_list, const RhiSamplerBinding &binding) override;
    RhiStatus RecordDraw(RhiCommandList &command_list, const RhiDrawDesc &desc) override;
    RhiStatus RecordDrawIndexed(RhiCommandList &command_list, const RhiDrawIndexedDesc &desc) override;
    /**
     * @comment Submits requested work.
     * @param command_list Input command list.
     * @return Explicit operation status.
     */
    RhiStatus Submit(const RhiCommandList &command_list) override;
    /**
     * @comment Presents the submitted target.
     * @return Explicit operation status.
     */
    RhiStatus Present() override;
    /**
     * @comment Captures the presented target.
     * @param destination Input destination.
     * @return Explicit operation result.
     */
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
    RhiStatus RequestPrimitiveRetirement(
        const RhiPrimitiveRetirementRequest &request,
        RhiPrimitiveRetirementRecord &out_record) override;
    RhiStatus QueryPrimitiveRetirement(
        std::uint64_t retirement_id,
        RhiPrimitiveRetirementRecord &out_record) const override;
    RhiStatus DrainPrimitiveRetirements(
        const RhiPrimitiveRetirementDrainRequest &request,
        RhiPrimitiveRetirementDrainResult &out_result) override;
    /**
     * @comment Returns the supported capabilities.
     * @return Capability data.
     */
    RhiCapabilities Capabilities() const override;
    /**
     * @comment Returns a snapshot of the current state.
     * @return Snapshot value.
     */
    RhiDeviceSnapshot Snapshot() const override;

private:
    struct NullBufferSlot final {
        RhiBufferDesc desc{};
        std::vector<std::uint8_t> bytes{};
        std::uint32_t generation = 0U;
        bool is_active = false;
    };

    struct NullTextureSlot final {
        RhiTextureDesc desc{};
        std::vector<std::uint8_t> bytes{};
        std::uint32_t generation = 0U;
        bool is_active = false;
    };

    struct NullSamplerSlot final {
        RhiSamplerDesc desc{};
        std::uint32_t generation = 0U;
        bool is_active = false;
    };

    struct NullShaderModuleSlot final {
        RhiShaderModuleDesc desc{};
        std::vector<std::uint8_t> bytes{};
        std::uint32_t generation = 0U;
        bool is_active = false;
    };

    struct NullPipelineSlot final {
        RhiPipelineDesc desc{};
        std::uint32_t generation = 0U;
        bool is_active = false;
    };

    RhiStatus RecordFailure(RhiStatus status);
    RhiStatus RecordIndexedDrawFailure(RhiStatus status);
    RhiStatus RecordSampledTextureBindFailure(RhiStatus status);
    RhiStatus RecordSamplerBindFailure(RhiStatus status);
    bool IsTargetHandleValid(RhiTextureHandle handle) const;
    bool IsBufferHandleValid(RhiBufferHandle handle) const;
    bool IsTextureHandleValid(RhiTextureHandle handle) const;
    bool IsSamplerHandleValid(RhiSamplerHandle handle) const;
    bool IsShaderModuleHandleValid(RhiShaderModuleHandle handle) const;
    bool IsPipelineHandleValid(RhiPipelineHandle handle) const;
    bool IsRetirementRequestValid(const RhiPrimitiveRetirementRequest &request) const;
    bool IsRetirementWrongKind(const RhiPrimitiveRetirementRequest &request) const;
    bool IsRetirementDuplicate(const RhiPrimitiveRetirementRequest &request) const;
    bool IsRetirementFenceReady(RhiFenceHandle fence) const;
    void RetireBufferSlot(std::uint32_t slot);
    void RetireTextureSlot(std::uint32_t slot);
    void RetireSamplerSlot(std::uint32_t slot);
    void RetireShaderModuleSlot(std::uint32_t slot);
    void RetirePipelineSlot(std::uint32_t slot);
    void DrainRetirementRecord(RhiPrimitiveRetirementRecord &record);
    bool IsCommandTargetValidForFrame(const RhiCommandRecord &command, RhiTextureHandle frame_target) const;
    bool IsVertexBufferViewValid(const RhiVertexBufferView &view) const;
    bool IsIndexBufferViewValid(const RhiIndexBufferView &view) const;
    bool IsSampledTextureBindingValid(const RhiSampledTextureBinding &binding) const;
    bool IsSamplerBindingValid(const RhiSamplerBinding &binding) const;
    bool IsDrawDescValid(const RhiDrawDesc &desc) const;
    bool IsDrawIndexedDescValid(const RhiDrawIndexedDesc &desc) const;
    bool IsInputLayoutDescValid(const RhiInputLayoutDesc &desc) const;
    bool IsDrawRangeValid(const RhiVertexBufferView &view, const RhiDrawDesc &desc) const;
    bool IsIndexedDrawRangeValid(const RhiIndexBufferView &view, const RhiDrawIndexedDesc &desc) const;
    bool IsColorTargetDescValid(const RhiColorTargetDesc &desc) const;
    bool IsBufferDescValid(const RhiBufferDesc &desc, std::span<const std::uint8_t> initial_bytes) const;
    bool IsTextureDescValid(const RhiTextureDesc &desc, std::span<const std::uint8_t> initial_bytes) const;
    bool IsShaderModuleDescValid(const RhiShaderModuleDesc &desc) const;
    bool IsPipelineDescValid(const RhiPipelineDesc &desc) const;
    std::size_t PixelByteCount(const RhiColorTargetDesc &desc) const;
    std::size_t TextureByteCount(const RhiTextureDesc &desc) const;
    std::size_t IndexFormatByteCount(RhiIndexFormat format) const;
    RhiFenceHandle SignalFence(std::size_t byte_count);
    void ExecuteClear(RhiTextureHandle handle, RhiColor color);

    std::vector<RhiTargetSlot> targets_;
    std::vector<NullBufferSlot> buffers_;
    std::vector<NullTextureSlot> textures_;
    std::vector<NullSamplerSlot> samplers_;
    std::vector<NullShaderModuleSlot> shader_modules_;
    std::vector<NullPipelineSlot> pipelines_;
    std::vector<RhiPrimitiveRetirementRecord> primitive_retirements_;
    RhiCapabilities capabilities_;
    RhiDeviceSnapshot snapshot_;
    RhiTextureHandle submitted_handle_;
    RhiTextureHandle presented_handle_;
    std::uint32_t generation_seed_;
    std::uint32_t fence_generation_;
    std::uint64_t next_primitive_retirement_id_;
    bool is_initialized_;
    bool has_submitted_frame_;
    bool has_presented_frame_;
};
}
