// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/IRhiDevice.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/Rhi/RhiCapabilities.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiCaptureResult.h"
#include "YuEngine/Rhi/RhiColorTargetDesc.h"
#include "YuEngine/Rhi/RhiCommandList.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiDeviceSnapshot.h"
#include "YuEngine/Rhi/RhiDrawDesc.h"
#include "YuEngine/Rhi/RhiFenceHandle.h"
#include "YuEngine/Rhi/RhiPipelineDesc.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiSamplerDesc.h"
#include "YuEngine/Rhi/RhiSamplerHandle.h"
#include "YuEngine/Rhi/RhiShaderModuleDesc.h"
#include "YuEngine/Rhi/RhiShaderModuleHandle.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"

namespace yuengine::rhi {
class IRhiDevice {
public:
    /**
     * @comment Destroys the interface.
     */
    virtual ~IRhiDevice() = default;

    /**
     * @comment Initializes the device.
     * @param desc Input descriptor.
     * @return Explicit operation status.
     */
    virtual RhiStatus Initialize(const RhiDeviceDesc &desc) = 0;
    /**
     * @comment Creates a color target.
     * @param desc Input descriptor.
     * @param out_handle Output handle written on success.
     * @return Explicit operation status.
     */
    virtual RhiStatus CreateColorTarget(const RhiColorTargetDesc &desc, RhiTextureHandle &out_handle) = 0;
    /**
     * @comment Returns the swapchain color target handle.
     * @param out_handle Output handle written on success.
     * @return Explicit operation status.
     */
    virtual RhiStatus GetSwapchainColorTarget(RhiTextureHandle &out_handle) const = 0;
    /**
     * @comment Destroys a target.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    virtual RhiStatus DestroyTarget(RhiTextureHandle handle) = 0;
    /**
     * @comment Records a clear command.
     * @param command_list Command list updated by the function.
     * @param handle Input handle.
     * @param color Input color.
     * @return Explicit operation status.
     */
    virtual RhiStatus RecordClear(RhiCommandList &command_list, RhiTextureHandle handle, RhiColor color) = 0;
    /**
     * @comment Records pipeline binding.
     * @param command_list Command list updated by the function.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    virtual RhiStatus RecordBindPipeline(RhiCommandList &command_list, RhiPipelineHandle handle) = 0;
    /**
     * @comment Records vertex buffer binding.
     * @param command_list Command list updated by the function.
     * @param view Input view.
     * @return Explicit operation status.
     */
    virtual RhiStatus RecordBindVertexBuffer(RhiCommandList &command_list, const RhiVertexBufferView &view) = 0;
    /**
     * @comment Records draw.
     * @param command_list Command list updated by the function.
     * @param desc Input descriptor.
     * @return Explicit operation status.
     */
    virtual RhiStatus RecordDraw(RhiCommandList &command_list, const RhiDrawDesc &desc) = 0;
    /**
     * @comment Submits recorded work.
     * @param command_list Input command list.
     * @return Explicit operation status.
     */
    virtual RhiStatus Submit(const RhiCommandList &command_list) = 0;
    /**
     * @comment Presents submitted work.
     * @return Explicit operation status.
     */
    virtual RhiStatus Present() = 0;
    /**
     * @comment Captures the presented target into caller-owned storage.
     * @param destination Input destination.
     * @return Explicit operation result.
     */
    virtual RhiCaptureResult CapturePresentedTarget(std::span<std::uint8_t> destination) = 0;
    /**
     * @comment Creates a buffer primitive.
     * @param desc Input descriptor.
     * @param initial_bytes Optional caller-owned initial bytes.
     * @param out_handle Output handle written on success.
     * @return Explicit operation status.
     */
    virtual RhiStatus CreateBuffer(
        const RhiBufferDesc &desc,
        std::span<const std::uint8_t> initial_bytes,
        RhiBufferHandle &out_handle) = 0;
    /**
     * @comment Updates a buffer primitive from caller-owned bytes.
     * @param handle Input handle.
     * @param bytes Input bytes.
     * @param out_fence Output fence handle written on success.
     * @return Explicit operation status.
     */
    virtual RhiStatus UpdateBuffer(
        RhiBufferHandle handle,
        std::span<const std::uint8_t> bytes,
        RhiFenceHandle &out_fence) = 0;
    /**
     * @comment Destroys a buffer primitive.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    virtual RhiStatus DestroyBuffer(RhiBufferHandle handle) = 0;
    /**
     * @comment Creates a texture primitive.
     * @param desc Input descriptor.
     * @param initial_bytes Optional caller-owned initial bytes.
     * @param out_handle Output handle written on success.
     * @return Explicit operation status.
     */
    virtual RhiStatus CreateTexture(
        const RhiTextureDesc &desc,
        std::span<const std::uint8_t> initial_bytes,
        RhiTextureHandle &out_handle) = 0;
    /**
     * @comment Updates a texture primitive from caller-owned bytes.
     * @param handle Input handle.
     * @param bytes Input bytes.
     * @param out_fence Output fence handle written on success.
     * @return Explicit operation status.
     */
    virtual RhiStatus UpdateTexture(
        RhiTextureHandle handle,
        std::span<const std::uint8_t> bytes,
        RhiFenceHandle &out_fence) = 0;
    /**
     * @comment Destroys a texture primitive.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    virtual RhiStatus DestroyTexture(RhiTextureHandle handle) = 0;
    /**
     * @comment Creates a sampler primitive.
     * @param desc Input descriptor.
     * @param out_handle Output handle written on success.
     * @return Explicit operation status.
     */
    virtual RhiStatus CreateSampler(const RhiSamplerDesc &desc, RhiSamplerHandle &out_handle) = 0;
    /**
     * @comment Destroys a sampler primitive.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    virtual RhiStatus DestroySampler(RhiSamplerHandle handle) = 0;
    /**
     * @comment Creates a shader module from caller-owned bytecode.
     * @param desc Input descriptor.
     * @param out_handle Output handle written on success.
     * @return Explicit operation status.
     */
    virtual RhiStatus CreateShaderModule(const RhiShaderModuleDesc &desc, RhiShaderModuleHandle &out_handle) = 0;
    /**
     * @comment Destroys a shader module.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    virtual RhiStatus DestroyShaderModule(RhiShaderModuleHandle handle) = 0;
    /**
     * @comment Creates a pipeline primitive.
     * @param desc Input descriptor.
     * @param out_handle Output handle written on success.
     * @return Explicit operation status.
     */
    virtual RhiStatus CreatePipeline(const RhiPipelineDesc &desc, RhiPipelineHandle &out_handle) = 0;
    /**
     * @comment Destroys a pipeline primitive.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    virtual RhiStatus DestroyPipeline(RhiPipelineHandle handle) = 0;
    /**
     * @comment Returns supported capabilities.
     * @return Capability data.
     */
    virtual RhiCapabilities Capabilities() const = 0;
    /**
     * @comment Returns current device state.
     * @return Snapshot value.
     */
    virtual RhiDeviceSnapshot Snapshot() const = 0;
};
}
