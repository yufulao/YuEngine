// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/NullRhiDevice.h

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
     * @comment 构造 NullRhiDevice 实例。
     */
    NullRhiDevice();

    /**
     * @comment 初始化实例。
     * @param desc 输入描述。
     * @return 显式操作状态。
     */
    RhiStatus Initialize(const RhiDeviceDesc &desc) override;
    /**
     * @comment 创建 颜色 target。
     * @param desc 输入描述。
     * @param out_handle 成功时写入的输出句柄。
     * @return 显式操作状态。
     */
    RhiStatus CreateColorTarget(const RhiColorTargetDesc &desc, RhiTextureHandle &out_handle) override;
    /**
     * @comment 返回交换链颜色目标句柄。
     * @param out_handle 成功时写入的输出句柄。
     * @return 显式操作状态。
     */
    RhiStatus GetSwapchainColorTarget(RhiTextureHandle &out_handle) const override;
    /**
     * @comment 拒绝 null backend 的 swapchain resize。
     * @param request 输入 resize 请求。
     * @param out_result 输出 resize 结果。
     * @return 显式操作状态。
     */
    RhiStatus ResizeSwapchain(
        const RhiSwapchainResizeRequest &request,
        RhiSwapchainResizeResult &out_result) override;
    /**
     * @comment 销毁目标。
     * @param handle 输入句柄。
     * @return 显式操作状态。
     */
    RhiStatus DestroyTarget(RhiTextureHandle handle) override;
    /**
     * @comment 记录清屏。
     * @param command_list 函数写入的命令列表。
     * @param handle 输入句柄。
     * @param color 输入颜色。
     * @return 显式操作状态。
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
     * @comment 提交 请求的 work。
     * @param command_list 输入命令列表。
     * @return 显式操作状态。
     */
    RhiStatus Submit(const RhiCommandList &command_list) override;
    /**
     * @comment 呈现 已提交目标。
     * @return 显式操作状态。
     */
    RhiStatus Present() override;
    /**
     * @comment 捕获已 呈现 的目标。
     * @param destination 输入 目标。
     * @return 显式操作结果。
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
     * @comment 返回 支持的能力。
     * @return 能力数据。
     */
    RhiCapabilities Capabilities() const override;
    /**
     * @comment 返回当前状态快照。
     * @return 快照值。
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
