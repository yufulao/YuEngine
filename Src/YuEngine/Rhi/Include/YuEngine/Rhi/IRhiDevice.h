// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/IRhiDevice.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/Rhi/RhiCapabilities.h"
#include "YuEngine/Rhi/RhiBlendStateDesc.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiCaptureResult.h"
#include "YuEngine/Rhi/RhiColorTargetDesc.h"
#include "YuEngine/Rhi/RhiCommandList.h"
#include "YuEngine/Rhi/RhiConstantBufferBinding.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiDeviceSnapshot.h"
#include "YuEngine/Rhi/RhiDrawDesc.h"
#include "YuEngine/Rhi/RhiDrawIndexedDesc.h"
#include "YuEngine/Rhi/RhiFenceHandle.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiPipelineDesc.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiPrimitiveRetirementDrainRequest.h"
#include "YuEngine/Rhi/RhiPrimitiveRetirementDrainResult.h"
#include "YuEngine/Rhi/RhiPrimitiveRetirementRecord.h"
#include "YuEngine/Rhi/RhiPrimitiveRetirementRequest.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"
#include "YuEngine/Rhi/RhiSamplerDesc.h"
#include "YuEngine/Rhi/RhiSamplerHandle.h"
#include "YuEngine/Rhi/RhiShaderModuleDesc.h"
#include "YuEngine/Rhi/RhiShaderModuleHandle.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiSwapchainResizeRequest.h"
#include "YuEngine/Rhi/RhiSwapchainResizeResult.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"

namespace yuengine::rhi {
class IRhiDevice {
public:
    /**
     * @comment 销毁接口。
     */
    virtual ~IRhiDevice() = default;

    /**
     * @comment 初始化设备。
     * @param desc 输入描述。
     * @return 显式操作状态。
     */
    virtual RhiStatus Initialize(const RhiDeviceDesc &desc) = 0;
    /**
     * @comment 创建颜色目标。
     * @param desc 输入描述。
     * @param out_handle 成功时写入的输出句柄。
     * @return 显式操作状态。
     */
    virtual RhiStatus CreateColorTarget(const RhiColorTargetDesc &desc, RhiTextureHandle &out_handle) = 0;
    /**
     * @comment 返回交换链颜色目标句柄。
     * @param out_handle 成功时写入的输出句柄。
     * @return 显式操作状态。
     */
    virtual RhiStatus GetSwapchainColorTarget(RhiTextureHandle &out_handle) const = 0;
    /**
     * @comment 调整交换链 backbuffer 大小。
     * @param request 输入 resize 请求。
     * @param out_result 输出 resize 结果。
     * @return 显式操作状态。
     */
    virtual RhiStatus ResizeSwapchain(
        const RhiSwapchainResizeRequest &request,
        RhiSwapchainResizeResult &out_result) = 0;
    /**
     * @comment 销毁目标。
     * @param handle 输入句柄。
     * @return 显式操作状态。
     */
    virtual RhiStatus DestroyTarget(RhiTextureHandle handle) = 0;
    /**
     * @comment 记录清屏命令。
     * @param command_list 函数写入的命令列表。
     * @param handle 输入句柄。
     * @param color 输入颜色。
     * @return 显式操作状态。
     */
    virtual RhiStatus RecordClear(RhiCommandList &command_list, RhiTextureHandle handle, RhiColor color) = 0;
    /**
     * @comment 记录流水线绑定。
     * @param command_list 函数写入的命令列表。
     * @param handle 输入句柄。
     * @return 显式操作状态。
     */
    virtual RhiStatus RecordBindPipeline(RhiCommandList &command_list, RhiPipelineHandle handle) = 0;
    /**
     * @comment 记录顶点缓冲绑定。
     * @param command_list 函数写入的命令列表。
     * @param view 输入视图。
     * @return 显式操作状态。
     */
    virtual RhiStatus RecordBindVertexBuffer(RhiCommandList &command_list, const RhiVertexBufferView &view) = 0;
    /**
     * @comment 记录索引缓冲绑定。
     * @param command_list 函数写入的命令列表。
     * @param view 输入视图。
     * @return 显式操作状态。
     */
    virtual RhiStatus RecordBindIndexBuffer(RhiCommandList &command_list, const RhiIndexBufferView &view) = 0;
    /**
     * @comment 记录采样纹理绑定。
     * @param command_list 函数写入的命令列表。
     * @param binding 输入绑定。
     * @return 显式操作状态。
     */
    virtual RhiStatus RecordBindSampledTexture(
        RhiCommandList &command_list,
        const RhiSampledTextureBinding &binding) = 0;
    /**
     * @comment 记录采样器绑定。
     * @param command_list 函数写入的命令列表。
     * @param binding 输入绑定。
     * @return 显式操作状态。
     */
    virtual RhiStatus RecordBindSampler(RhiCommandList &command_list, const RhiSamplerBinding &binding) = 0;
    /**
     * @comment 记录 constant buffer 绑定。
     * @param command_list 函数写入的命令列表。
     * @param binding 输入绑定。
     * @return 显式操作状态。
     */
    virtual RhiStatus RecordBindConstantBuffer(
        RhiCommandList &command_list,
        const RhiConstantBufferBinding &binding) = 0;
    /**
     * @comment 记录 blend state 绑定。
     * @param command_list 函数写入的命令列表。
     * @param desc 输入 blend state。
     * @return 显式操作状态。
     */
    virtual RhiStatus RecordBindBlendState(RhiCommandList &command_list, const RhiBlendStateDesc &desc) = 0;
    /**
     * @comment 记录绘制。
     * @param command_list 函数写入的命令列表。
     * @param desc 输入描述。
     * @return 显式操作状态。
     */
    virtual RhiStatus RecordDraw(RhiCommandList &command_list, const RhiDrawDesc &desc) = 0;
    /**
     * @comment 记录索引绘制。
     * @param command_list 函数写入的命令列表。
     * @param desc 输入描述。
     * @return 显式操作状态。
     */
    virtual RhiStatus RecordDrawIndexed(RhiCommandList &command_list, const RhiDrawIndexedDesc &desc) = 0;
    /**
     * @comment 提交已记录工作。
     * @param command_list 输入命令列表。
     * @return 显式操作状态。
     */
    virtual RhiStatus Submit(const RhiCommandList &command_list) = 0;
    /**
     * @comment 呈现 已提交工作。
     * @return 显式操作状态。
     */
    virtual RhiStatus Present() = 0;
    /**
     * @comment 将已 呈现 的目标捕获到调用方持有存储。
     * @param destination 输入 目标。
     * @return 显式操作结果。
     */
    virtual RhiCaptureResult CapturePresentedTarget(std::span<std::uint8_t> destination) = 0;
    /**
     * @comment 创建 一个 buffer primitive。
     * @param desc 输入描述。
     * @param initial_bytes 可选 调用方持有 初始 字节.
     * @param out_handle 成功时写入的输出句柄。
     * @return 显式操作状态。
     */
    virtual RhiStatus CreateBuffer(
        const RhiBufferDesc &desc,
        std::span<const std::uint8_t> initial_bytes,
        RhiBufferHandle &out_handle) = 0;
    /**
     * @comment 更新 一个 buffer primitive 从 调用方持有 字节。
     * @param handle 输入句柄。
     * @param bytes 输入字节。
     * @param out_fence 成功时写入的输出 fence 句柄。
     * @return 显式操作状态。
     */
    virtual RhiStatus UpdateBuffer(
        RhiBufferHandle handle,
        std::span<const std::uint8_t> bytes,
        RhiFenceHandle &out_fence) = 0;
    /**
     * @comment 销毁一个 buffer primitive。
     * @param handle 输入句柄。
     * @return 显式操作状态。
     */
    virtual RhiStatus DestroyBuffer(RhiBufferHandle handle) = 0;
    /**
     * @comment 创建 一个 texture primitive。
     * @param desc 输入描述。
     * @param initial_bytes 可选 调用方持有 初始 字节.
     * @param out_handle 成功时写入的输出句柄。
     * @return 显式操作状态。
     */
    virtual RhiStatus CreateTexture(
        const RhiTextureDesc &desc,
        std::span<const std::uint8_t> initial_bytes,
        RhiTextureHandle &out_handle) = 0;
    /**
     * @comment 更新 一个 texture primitive 从 调用方持有 字节。
     * @param handle 输入句柄。
     * @param bytes 输入字节。
     * @param out_fence 成功时写入的输出 fence 句柄。
     * @return 显式操作状态。
     */
    virtual RhiStatus UpdateTexture(
        RhiTextureHandle handle,
        std::span<const std::uint8_t> bytes,
        RhiFenceHandle &out_fence) = 0;
    /**
     * @comment 销毁一个 texture primitive。
     * @param handle 输入句柄。
     * @return 显式操作状态。
     */
    virtual RhiStatus DestroyTexture(RhiTextureHandle handle) = 0;
    /**
     * @comment 创建 一个 sampler primitive。
     * @param desc 输入描述。
     * @param out_handle 成功时写入的输出句柄。
     * @return 显式操作状态。
     */
    virtual RhiStatus CreateSampler(const RhiSamplerDesc &desc, RhiSamplerHandle &out_handle) = 0;
    /**
     * @comment 销毁 sampler primitive。
     * @param handle 输入句柄。
     * @return 显式操作状态。
     */
    virtual RhiStatus DestroySampler(RhiSamplerHandle handle) = 0;
    /**
     * @comment 创建 一个 shader module 从 调用方持有 bytecode。
     * @param desc 输入描述。
     * @param out_handle 成功时写入的输出句柄。
     * @return 显式操作状态。
     */
    virtual RhiStatus CreateShaderModule(const RhiShaderModuleDesc &desc, RhiShaderModuleHandle &out_handle) = 0;
    /**
     * @comment 销毁 shader module。
     * @param handle 输入句柄。
     * @return 显式操作状态。
     */
    virtual RhiStatus DestroyShaderModule(RhiShaderModuleHandle handle) = 0;
    /**
     * @comment 创建 一个 pipeline primitive。
     * @param desc 输入描述。
     * @param out_handle 成功时写入的输出句柄。
     * @return 显式操作状态。
     */
    virtual RhiStatus CreatePipeline(const RhiPipelineDesc &desc, RhiPipelineHandle &out_handle) = 0;
    /**
     * @comment 销毁 pipeline primitive。
     * @param handle 输入句柄。
     * @return 显式操作状态。
     */
    virtual RhiStatus DestroyPipeline(RhiPipelineHandle handle) = 0;
    /**
     * @comment 请求 deferred primitive retirement。
     * @param request 输入 请求。
     * @param out_record 输出记录，写入 accepted 或 rejected 状态。
     * @return 显式操作状态。
     */
    virtual RhiStatus RequestPrimitiveRetirement(
        const RhiPrimitiveRetirementRequest &request,
        RhiPrimitiveRetirementRecord &out_record) = 0;
    /**
     * @comment 查询一个 primitive retirement 记录。
     * @param retirement_id 输入 retirement id。
     * @param out_record 成功时写入的输出记录。
     * @return 显式操作状态。
     */
    virtual RhiStatus QueryPrimitiveRetirement(
        std::uint64_t retirement_id,
        RhiPrimitiveRetirementRecord &out_record) const = 0;
    /**
     * @comment 提取就绪的 primitive retirement 记录。
     * @param request 输入 drain 请求。
     * @param out_result 输出 drain 结果。
     * @return 显式操作状态。
     */
    virtual RhiStatus DrainPrimitiveRetirements(
        const RhiPrimitiveRetirementDrainRequest &request,
        RhiPrimitiveRetirementDrainResult &out_result) = 0;
    /**
     * @comment 返回 支持的能力。
     * @return 能力数据。
     */
    virtual RhiCapabilities Capabilities() const = 0;
    /**
     * @comment 返回 当前 device 状态。
     * @return 快照值。
     */
    virtual RhiDeviceSnapshot Snapshot() const = 0;
};
}
