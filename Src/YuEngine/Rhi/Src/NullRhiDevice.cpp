// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Src/NullRhiDevice.cpp

#include "YuEngine/Rhi/NullRhiDevice.h"

#include <algorithm>

#include "YuEngine/Rhi/RhiDeviceFactory.h"
#include "YuEngine/Rhi/RhiConstants.h"

namespace yuengine::rhi {
namespace {
constexpr std::uint32_t INVALID_GENERATION = 0U;

std::size_t InputElementFormatByteCount(RhiInputElementFormat format) {
    if (format == RhiInputElementFormat::Float32x2) {
        return sizeof(float) * 2U;
    }

    if (format == RhiInputElementFormat::Float32x3) {
        return sizeof(float) * 3U;
    }

    if (format == RhiInputElementFormat::Float32x4) {
        return sizeof(float) * 4U;
    }

    return 0U;
}
}

RhiDeviceCreateResult RhiDeviceFactory::CreateDevice(const RhiDeviceDesc &desc, NullRhiDevice *null_device) {
    const RhiStatus desc_status = ValidateDeviceDesc(desc);
    if (desc_status != RhiStatus::Success) {
        return RhiDeviceCreateResult{desc_status, nullptr, RhiCapabilities{}};
    }

    if (null_device == nullptr) {
        return RhiDeviceCreateResult{RhiStatus::InvalidDescriptor, nullptr, RhiCapabilities{}};
    }

    const RhiStatus initialize_status = null_device->Initialize(desc);
    if (initialize_status != RhiStatus::Success) {
        return RhiDeviceCreateResult{initialize_status, nullptr, RhiCapabilities{}};
    }

    return RhiDeviceCreateResult{RhiStatus::Success, null_device, null_device->Capabilities()};
}

RhiStatus RhiDeviceFactory::ValidateNativeSurfaceDesc(const RhiNativeSurfaceDesc &surface_desc) {
    if (!surface_desc.valid) {
        return RhiStatus::InvalidDescriptor;
    }

    if (surface_desc.window_value == 0U) {
        return RhiStatus::InvalidDescriptor;
    }

    return RhiStatus::Success;
}

RhiStatus RhiDeviceFactory::ValidateDeviceDesc(const RhiDeviceDesc &desc) {
    if (desc.backend_kind == RhiBackendKind::D3D11) {
        return RhiStatus::UnsupportedBackend;
    }

    if (desc.backend_kind == RhiBackendKind::Unsupported) {
        return RhiStatus::UnsupportedBackend;
    }

    if (desc.backend_kind != RhiBackendKind::Null) {
        return RhiStatus::UnsupportedBackend;
    }

    if (desc.requires_native_surface) {
        const RhiStatus surface_status = ValidateNativeSurfaceDesc(desc.native_surface);
        if (surface_status != RhiStatus::Success) {
            return surface_status;
        }

        return RhiStatus::UnsupportedBackend;
    }

    if (desc.requires_swapchain) {
        return RhiStatus::UnsupportedBackend;
    }

    return RhiStatus::Success;
}

NullRhiDevice::NullRhiDevice()
    : targets_(),
      buffers_(),
      textures_(),
      samplers_(),
      shader_modules_(),
      pipelines_(),
      capabilities_{},
      snapshot_{},
      submitted_handle_{},
      presented_handle_{},
      generation_seed_(INVALID_GENERATION),
      fence_generation_(INVALID_GENERATION),
      is_initialized_(false),
      has_submitted_frame_(false),
      has_presented_frame_(false) {
}

RhiStatus NullRhiDevice::Initialize(const RhiDeviceDesc &desc) {
    if (desc.backend_kind != RhiBackendKind::Null) {
        return RhiStatus::UnsupportedBackend;
    }

    if (desc.requires_native_surface) {
        return RhiStatus::UnsupportedBackend;
    }

    if (desc.requires_swapchain) {
        return RhiStatus::UnsupportedBackend;
    }

    if (desc.color_target_capacity == 0U) {
        return RhiStatus::InvalidDescriptor;
    }

    if (desc.color_target_capacity > MAX_COLOR_TARGETS) {
        return RhiStatus::CapacityExceeded;
    }

    if (desc.command_list_capacity == 0U) {
        return RhiStatus::InvalidDescriptor;
    }

    if (desc.command_list_capacity > MAX_COMMANDS) {
        return RhiStatus::CapacityExceeded;
    }

    ++generation_seed_;
    if (generation_seed_ == INVALID_GENERATION) {
        ++generation_seed_;
    }

    targets_.assign(desc.color_target_capacity, RhiTargetSlot{});
    for (RhiTargetSlot &target : targets_) {
        target.generation = generation_seed_;
    }

    buffers_.assign(MAX_RHI_BUFFERS, NullBufferSlot{});
    for (NullBufferSlot &buffer : buffers_) {
        buffer.generation = generation_seed_;
    }

    textures_.assign(MAX_RHI_TEXTURES, NullTextureSlot{});
    for (NullTextureSlot &texture : textures_) {
        texture.generation = generation_seed_;
    }

    samplers_.assign(MAX_RHI_SAMPLERS, NullSamplerSlot{});
    for (NullSamplerSlot &sampler : samplers_) {
        sampler.generation = generation_seed_;
    }

    shader_modules_.assign(MAX_RHI_SHADER_MODULES, NullShaderModuleSlot{});
    for (NullShaderModuleSlot &shader_module : shader_modules_) {
        shader_module.generation = generation_seed_;
    }

    pipelines_.assign(MAX_RHI_PIPELINES, NullPipelineSlot{});
    for (NullPipelineSlot &pipeline : pipelines_) {
        pipeline.generation = generation_seed_;
    }

    capabilities_ = RhiCapabilities{
        RhiBackendKind::Null,
        RhiFormat::Rgba8Unorm,
        desc.color_target_capacity,
        desc.command_list_capacity,
        MAX_COLOR_TARGET_EXTENT,
        MAX_CAPTURE_FIXTURE_EXTENT,
        true,
        false,
        false,
        false,
        true,
        MAX_RHI_BUFFERS,
        MAX_RHI_TEXTURES,
        MAX_RHI_SAMPLERS,
        MAX_RHI_SHADER_MODULES,
        MAX_RHI_PIPELINES,
        MAX_RHI_BUFFER_BYTES,
        MAX_RHI_SHADER_BYTECODE_BYTES};
    snapshot_ = RhiDeviceSnapshot{};
    snapshot_.color_target_capacity = desc.color_target_capacity;
    snapshot_.resources.buffer_capacity = MAX_RHI_BUFFERS;
    snapshot_.resources.texture_capacity = MAX_RHI_TEXTURES;
    snapshot_.resources.sampler_capacity = MAX_RHI_SAMPLERS;
    snapshot_.resources.shader_module_capacity = MAX_RHI_SHADER_MODULES;
    snapshot_.resources.pipeline_capacity = MAX_RHI_PIPELINES;
    is_initialized_ = true;
    has_submitted_frame_ = false;
    has_presented_frame_ = false;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::CreateColorTarget(const RhiColorTargetDesc &desc, RhiTextureHandle &out_handle) {
    if (!is_initialized_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (desc.format != RhiFormat::Rgba8Unorm) {
        return RecordFailure(RhiStatus::UnsupportedFormat);
    }

    if (!IsColorTargetDescValid(desc)) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    for (std::size_t index = 0U; index < targets_.size(); ++index) {
        RhiTargetSlot& slot = targets_[index];
        if (slot.is_active) {
            continue;
        }

        if (slot.generation == INVALID_GENERATION) {
            slot.generation = 1U;
        }

        slot.is_active = true;
        slot.desc = desc;
        slot.bytes.assign(PixelByteCount(desc), 0U);
        out_handle = RhiTextureHandle{static_cast<std::uint32_t>(index), slot.generation};
        ++snapshot_.color_target_count;
        ++snapshot_.created_target_count;
        return RhiStatus::Success;
    }

    return RecordFailure(RhiStatus::CapacityExceeded);
}

RhiStatus NullRhiDevice::GetSwapchainColorTarget(RhiTextureHandle &out_handle) const {
    out_handle = RhiTextureHandle{};
    return RhiStatus::UnsupportedBackend;
}

RhiStatus NullRhiDevice::DestroyTarget(RhiTextureHandle handle) {
    if (!IsTargetHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    RhiTargetSlot& slot = targets_[handle.slot];
    slot.is_active = false;
    slot.bytes.clear();
    ++slot.generation;
    --snapshot_.color_target_count;
    ++snapshot_.destroyed_target_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::RecordClear(RhiCommandList &command_list, RhiTextureHandle handle, RhiColor color) {
    if (!IsTargetHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    const RhiStatus status = command_list.RecordClear(handle, color);
    if (status != RhiStatus::Success) {
        return RecordFailure(status);
    }

    ++snapshot_.recorded_command_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::RecordBindPipeline(RhiCommandList &command_list, RhiPipelineHandle handle) {
    if (!IsPipelineHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    const RhiStatus status = command_list.RecordBindPipeline(handle);
    if (status != RhiStatus::Success) {
        return RecordFailure(status);
    }

    ++snapshot_.recorded_command_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::RecordBindVertexBuffer(RhiCommandList &command_list, const RhiVertexBufferView &view) {
    if (!IsVertexBufferViewValid(view)) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    const RhiStatus status = command_list.RecordBindVertexBuffer(view);
    if (status != RhiStatus::Success) {
        return RecordFailure(status);
    }

    ++snapshot_.recorded_command_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::RecordDraw(RhiCommandList &command_list, const RhiDrawDesc &desc) {
    if (!IsDrawDescValid(desc)) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    const RhiStatus status = command_list.RecordDraw(desc);
    if (status != RhiStatus::Success) {
        return RecordFailure(status);
    }

    ++snapshot_.recorded_command_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::Submit(const RhiCommandList &command_list) {
    if (!command_list.IsComplete()) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (command_list.Capacity() > capabilities_.command_list_capacity) {
        return RecordFailure(RhiStatus::CapacityExceeded);
    }

    const RhiTextureHandle target = command_list.TargetHandle();
    if (!IsTargetHandleValid(target)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    bool has_pipeline = false;
    bool has_vertex_buffer = false;
    RhiPipelineHandle bound_pipeline{};
    RhiVertexBufferView bound_vertex_buffer{};
    std::uint64_t submitted_draw_count = 0U;
    std::uint32_t last_draw_vertex_count = 0U;
    for (std::size_t index = 0U; index < command_list.CommandCount(); ++index) {
        const RhiCommandRecord &command = command_list.CommandAt(index);
        if (!IsCommandTargetValidForFrame(command, target)) {
            return RecordFailure(RhiStatus::InvalidHandle);
        }

        if (command.type == RhiCommandType::BindPipeline) {
            if (!IsPipelineHandleValid(command.pipeline)) {
                return RecordFailure(RhiStatus::InvalidHandle);
            }

            bound_pipeline = command.pipeline;
            has_pipeline = true;
            continue;
        }

        if (command.type == RhiCommandType::BindVertexBuffer) {
            if (!IsVertexBufferViewValid(command.vertex_buffer)) {
                return RecordFailure(RhiStatus::InvalidDescriptor);
            }

            bound_vertex_buffer = command.vertex_buffer;
            has_vertex_buffer = true;
            continue;
        }

        if (command.type == RhiCommandType::Draw) {
            if (!has_pipeline) {
                return RecordFailure(RhiStatus::InvalidLifecycle);
            }

            if (!has_vertex_buffer) {
                return RecordFailure(RhiStatus::InvalidLifecycle);
            }

            if (!IsDrawDescValid(command.draw)) {
                return RecordFailure(RhiStatus::InvalidDescriptor);
            }

            if (!IsDrawRangeValid(bound_vertex_buffer, command.draw)) {
                return RecordFailure(RhiStatus::InvalidDescriptor);
            }

            const NullPipelineSlot &pipeline = pipelines_[bound_pipeline.slot];
            if (!IsInputLayoutDescValid(pipeline.desc.input_layout)) {
                return RecordFailure(RhiStatus::InvalidDescriptor);
            }

            ++submitted_draw_count;
            last_draw_vertex_count = command.draw.vertex_count;
            continue;
        }
    }

    snapshot_.command_storage_capacity_before_frame = command_list.Capacity();
    for (std::size_t index = 0U; index < command_list.CommandCount(); ++index) {
        const RhiCommandRecord &command = command_list.CommandAt(index);
        if (command.type == RhiCommandType::ClearColor) {
            ExecuteClear(command.target, command.color);
            continue;
        }
    }

    submitted_handle_ = target;
    has_submitted_frame_ = true;
    has_presented_frame_ = false;
    ++snapshot_.submit_count;
    snapshot_.submitted_draw_count += submitted_draw_count;
    snapshot_.last_draw_vertex_count = last_draw_vertex_count;
    snapshot_.command_storage_capacity_after_last_frame = command_list.Capacity();
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::Present() {
    if (!has_submitted_frame_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (!IsTargetHandleValid(submitted_handle_)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    presented_handle_ = submitted_handle_;
    has_presented_frame_ = true;
    ++snapshot_.present_count;
    return RhiStatus::Success;
}

RhiCaptureResult NullRhiDevice::CapturePresentedTarget(std::span<std::uint8_t> destination) {
    if (!has_presented_frame_) {
        RecordFailure(RhiStatus::InvalidLifecycle);
        return RhiCaptureResult{RhiStatus::InvalidLifecycle, 0U};
    }

    if (!IsTargetHandleValid(presented_handle_)) {
        RecordFailure(RhiStatus::InvalidHandle);
        return RhiCaptureResult{RhiStatus::InvalidHandle, 0U};
    }

    const RhiTargetSlot& slot = targets_[presented_handle_.slot];
    if (slot.desc.extent.width > MAX_CAPTURE_FIXTURE_EXTENT || slot.desc.extent.height > MAX_CAPTURE_FIXTURE_EXTENT) {
        RecordFailure(RhiStatus::CapacityExceeded);
        snapshot_.last_capture_bytes_written = 0U;
        return RhiCaptureResult{RhiStatus::CapacityExceeded, 0U};
    }

    const std::size_t byte_count = slot.bytes.size();
    if (destination.size() < byte_count) {
        RecordFailure(RhiStatus::CapacityExceeded);
        snapshot_.last_capture_bytes_written = 0U;
        return RhiCaptureResult{RhiStatus::CapacityExceeded, 0U};
    }

    std::copy(slot.bytes.begin(), slot.bytes.end(), destination.begin());
    ++snapshot_.capture_count;
    snapshot_.last_capture_bytes_written = byte_count;
    return RhiCaptureResult{RhiStatus::Success, byte_count};
}

RhiStatus NullRhiDevice::CreateBuffer(
    const RhiBufferDesc &desc,
    std::span<const std::uint8_t> initial_bytes,
    RhiBufferHandle &out_handle) {
    out_handle = RhiBufferHandle{};
    if (!is_initialized_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (!IsBufferDescValid(desc, initial_bytes)) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    for (std::size_t index = 0U; index < buffers_.size(); ++index) {
        NullBufferSlot &slot = buffers_[index];
        if (slot.is_active) {
            continue;
        }

        if (slot.generation == INVALID_GENERATION) {
            slot.generation = 1U;
        }

        slot.is_active = true;
        slot.desc = desc;
        slot.bytes.assign(desc.size_bytes, 0U);
        std::copy(initial_bytes.begin(), initial_bytes.end(), slot.bytes.begin());
        out_handle = RhiBufferHandle{static_cast<std::uint32_t>(index), slot.generation};
        ++snapshot_.resources.buffer_count;
        ++snapshot_.resources.created_primitive_count;
        return RhiStatus::Success;
    }

    return RecordFailure(RhiStatus::CapacityExceeded);
}

RhiStatus NullRhiDevice::UpdateBuffer(
    RhiBufferHandle handle,
    std::span<const std::uint8_t> bytes,
    RhiFenceHandle &out_fence) {
    out_fence = RhiFenceHandle{};
    if (!IsBufferHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    if (bytes.empty()) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    NullBufferSlot &slot = buffers_[handle.slot];
    if (bytes.size() > slot.bytes.size()) {
        return RecordFailure(RhiStatus::CapacityExceeded);
    }

    std::copy(bytes.begin(), bytes.end(), slot.bytes.begin());
    out_fence = SignalFence(bytes.size());
    ++snapshot_.resources.updated_primitive_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::DestroyBuffer(RhiBufferHandle handle) {
    if (!IsBufferHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    NullBufferSlot &slot = buffers_[handle.slot];
    slot.is_active = false;
    slot.desc = RhiBufferDesc{};
    slot.bytes.clear();
    ++slot.generation;
    --snapshot_.resources.buffer_count;
    ++snapshot_.resources.destroyed_primitive_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::CreateTexture(
    const RhiTextureDesc &desc,
    std::span<const std::uint8_t> initial_bytes,
    RhiTextureHandle &out_handle) {
    out_handle = RhiTextureHandle{};
    if (!is_initialized_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (!IsTextureDescValid(desc, initial_bytes)) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    for (std::size_t index = 0U; index < textures_.size(); ++index) {
        NullTextureSlot &slot = textures_[index];
        if (slot.is_active) {
            continue;
        }

        if (slot.generation == INVALID_GENERATION) {
            slot.generation = 1U;
        }

        slot.is_active = true;
        slot.desc = desc;
        slot.bytes.assign(TextureByteCount(desc), 0U);
        std::copy(initial_bytes.begin(), initial_bytes.end(), slot.bytes.begin());
        out_handle = RhiTextureHandle{static_cast<std::uint32_t>(index), slot.generation};
        ++snapshot_.resources.texture_count;
        ++snapshot_.resources.created_primitive_count;
        return RhiStatus::Success;
    }

    return RecordFailure(RhiStatus::CapacityExceeded);
}

RhiStatus NullRhiDevice::UpdateTexture(
    RhiTextureHandle handle,
    std::span<const std::uint8_t> bytes,
    RhiFenceHandle &out_fence) {
    out_fence = RhiFenceHandle{};
    if (!IsTextureHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    NullTextureSlot &slot = textures_[handle.slot];
    if (bytes.size() != slot.bytes.size()) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    std::copy(bytes.begin(), bytes.end(), slot.bytes.begin());
    out_fence = SignalFence(bytes.size());
    ++snapshot_.resources.updated_primitive_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::DestroyTexture(RhiTextureHandle handle) {
    if (!IsTextureHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    NullTextureSlot &slot = textures_[handle.slot];
    slot.is_active = false;
    slot.desc = RhiTextureDesc{};
    slot.bytes.clear();
    ++slot.generation;
    --snapshot_.resources.texture_count;
    ++snapshot_.resources.destroyed_primitive_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::CreateSampler(const RhiSamplerDesc &desc, RhiSamplerHandle &out_handle) {
    out_handle = RhiSamplerHandle{};
    if (!is_initialized_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    for (std::size_t index = 0U; index < samplers_.size(); ++index) {
        NullSamplerSlot &slot = samplers_[index];
        if (slot.is_active) {
            continue;
        }

        if (slot.generation == INVALID_GENERATION) {
            slot.generation = 1U;
        }

        slot.is_active = true;
        slot.desc = desc;
        out_handle = RhiSamplerHandle{static_cast<std::uint32_t>(index), slot.generation};
        ++snapshot_.resources.sampler_count;
        ++snapshot_.resources.created_primitive_count;
        return RhiStatus::Success;
    }

    return RecordFailure(RhiStatus::CapacityExceeded);
}

RhiStatus NullRhiDevice::DestroySampler(RhiSamplerHandle handle) {
    if (!IsSamplerHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    NullSamplerSlot &slot = samplers_[handle.slot];
    slot.is_active = false;
    slot.desc = RhiSamplerDesc{};
    ++slot.generation;
    --snapshot_.resources.sampler_count;
    ++snapshot_.resources.destroyed_primitive_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::CreateShaderModule(const RhiShaderModuleDesc &desc, RhiShaderModuleHandle &out_handle) {
    out_handle = RhiShaderModuleHandle{};
    if (!is_initialized_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (!IsShaderModuleDescValid(desc)) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    for (std::size_t index = 0U; index < shader_modules_.size(); ++index) {
        NullShaderModuleSlot &slot = shader_modules_[index];
        if (slot.is_active) {
            continue;
        }

        if (slot.generation == INVALID_GENERATION) {
            slot.generation = 1U;
        }

        slot.is_active = true;
        slot.desc = desc;
        slot.bytes.assign(desc.bytecode.begin(), desc.bytecode.end());
        slot.desc.bytecode = std::span<const std::uint8_t>(slot.bytes.data(), slot.bytes.size());
        out_handle = RhiShaderModuleHandle{static_cast<std::uint32_t>(index), slot.generation};
        ++snapshot_.resources.shader_module_count;
        ++snapshot_.resources.created_primitive_count;
        return RhiStatus::Success;
    }

    return RecordFailure(RhiStatus::CapacityExceeded);
}

RhiStatus NullRhiDevice::DestroyShaderModule(RhiShaderModuleHandle handle) {
    if (!IsShaderModuleHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    NullShaderModuleSlot &slot = shader_modules_[handle.slot];
    slot.is_active = false;
    slot.desc = RhiShaderModuleDesc{};
    slot.bytes.clear();
    ++slot.generation;
    --snapshot_.resources.shader_module_count;
    ++snapshot_.resources.destroyed_primitive_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::CreatePipeline(const RhiPipelineDesc &desc, RhiPipelineHandle &out_handle) {
    out_handle = RhiPipelineHandle{};
    if (!is_initialized_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (!IsPipelineDescValid(desc)) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    for (std::size_t index = 0U; index < pipelines_.size(); ++index) {
        NullPipelineSlot &slot = pipelines_[index];
        if (slot.is_active) {
            continue;
        }

        if (slot.generation == INVALID_GENERATION) {
            slot.generation = 1U;
        }

        slot.is_active = true;
        slot.desc = desc;
        out_handle = RhiPipelineHandle{static_cast<std::uint32_t>(index), slot.generation};
        ++snapshot_.resources.pipeline_count;
        ++snapshot_.resources.created_primitive_count;
        return RhiStatus::Success;
    }

    return RecordFailure(RhiStatus::CapacityExceeded);
}

RhiStatus NullRhiDevice::DestroyPipeline(RhiPipelineHandle handle) {
    if (!IsPipelineHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    NullPipelineSlot &slot = pipelines_[handle.slot];
    slot.is_active = false;
    slot.desc = RhiPipelineDesc{};
    ++slot.generation;
    --snapshot_.resources.pipeline_count;
    ++snapshot_.resources.destroyed_primitive_count;
    return RhiStatus::Success;
}

RhiCapabilities NullRhiDevice::Capabilities() const {
    return capabilities_;
}

RhiDeviceSnapshot NullRhiDevice::Snapshot() const {
    return snapshot_;
}

RhiStatus NullRhiDevice::RecordFailure(RhiStatus status) {
    ++snapshot_.failed_operation_count;
    return status;
}

bool NullRhiDevice::IsTargetHandleValid(RhiTextureHandle handle) const {
    if (!is_initialized_) {
        return false;
    }

    if (handle.generation == INVALID_GENERATION) {
        return false;
    }

    if (handle.slot >= targets_.size()) {
        return false;
    }

    const RhiTargetSlot& slot = targets_[handle.slot];
    if (!slot.is_active) {
        return false;
    }

    return slot.generation == handle.generation;
}

bool NullRhiDevice::IsBufferHandleValid(RhiBufferHandle handle) const {
    if (!is_initialized_) {
        return false;
    }

    if (handle.generation == INVALID_GENERATION) {
        return false;
    }

    if (handle.slot >= buffers_.size()) {
        return false;
    }

    const NullBufferSlot &slot = buffers_[handle.slot];
    if (!slot.is_active) {
        return false;
    }

    return slot.generation == handle.generation;
}

bool NullRhiDevice::IsTextureHandleValid(RhiTextureHandle handle) const {
    if (!is_initialized_) {
        return false;
    }

    if (handle.generation == INVALID_GENERATION) {
        return false;
    }

    if (handle.slot >= textures_.size()) {
        return false;
    }

    const NullTextureSlot &slot = textures_[handle.slot];
    if (!slot.is_active) {
        return false;
    }

    return slot.generation == handle.generation;
}

bool NullRhiDevice::IsSamplerHandleValid(RhiSamplerHandle handle) const {
    if (!is_initialized_) {
        return false;
    }

    if (handle.generation == INVALID_GENERATION) {
        return false;
    }

    if (handle.slot >= samplers_.size()) {
        return false;
    }

    const NullSamplerSlot &slot = samplers_[handle.slot];
    if (!slot.is_active) {
        return false;
    }

    return slot.generation == handle.generation;
}

bool NullRhiDevice::IsShaderModuleHandleValid(RhiShaderModuleHandle handle) const {
    if (!is_initialized_) {
        return false;
    }

    if (handle.generation == INVALID_GENERATION) {
        return false;
    }

    if (handle.slot >= shader_modules_.size()) {
        return false;
    }

    const NullShaderModuleSlot &slot = shader_modules_[handle.slot];
    if (!slot.is_active) {
        return false;
    }

    return slot.generation == handle.generation;
}

bool NullRhiDevice::IsPipelineHandleValid(RhiPipelineHandle handle) const {
    if (!is_initialized_) {
        return false;
    }

    if (handle.generation == INVALID_GENERATION) {
        return false;
    }

    if (handle.slot >= pipelines_.size()) {
        return false;
    }

    const NullPipelineSlot &slot = pipelines_[handle.slot];
    if (!slot.is_active) {
        return false;
    }

    return slot.generation == handle.generation;
}

bool NullRhiDevice::IsCommandTargetValidForFrame(const RhiCommandRecord &command, RhiTextureHandle frame_target) const {
    if (command.type != RhiCommandType::ClearColor) {
        return true;
    }

    if (command.target.slot != frame_target.slot) {
        return false;
    }

    if (command.target.generation != frame_target.generation) {
        return false;
    }

    return IsTargetHandleValid(command.target);
}

bool NullRhiDevice::IsVertexBufferViewValid(const RhiVertexBufferView &view) const {
    if (!IsBufferHandleValid(view.buffer)) {
        return false;
    }

    const NullBufferSlot &slot = buffers_[view.buffer.slot];
    if (slot.desc.usage != RhiBufferUsage::Vertex) {
        return false;
    }

    if (view.stride_bytes == 0U) {
        return false;
    }

    if (view.size_bytes == 0U) {
        return false;
    }

    if (view.offset_bytes > slot.desc.size_bytes) {
        return false;
    }

    const std::size_t remaining_bytes = slot.desc.size_bytes - view.offset_bytes;
    if (view.size_bytes > remaining_bytes) {
        return false;
    }

    return true;
}

bool NullRhiDevice::IsDrawDescValid(const RhiDrawDesc &desc) const {
    if (desc.topology != RhiPrimitiveTopology::TriangleList) {
        return false;
    }

    if (desc.vertex_count == 0U) {
        return false;
    }

    return true;
}

bool NullRhiDevice::IsInputLayoutDescValid(const RhiInputLayoutDesc &desc) const {
    if (desc.element_count == 0U) {
        return false;
    }

    if (desc.element_count > MAX_RHI_INPUT_ELEMENTS) {
        return false;
    }

    if (desc.stride_bytes == 0U) {
        return false;
    }

    bool has_position = false;
    for (std::size_t index = 0U; index < desc.element_count; ++index) {
        const RhiInputElementDesc &element = desc.elements[index];
        if (element.semantic == RhiInputElementSemantic::Unsupported) {
            return false;
        }

        if (element.semantic == RhiInputElementSemantic::Position) {
            has_position = true;
        }

        const std::size_t element_bytes = InputElementFormatByteCount(element.format);
        if (element_bytes == 0U) {
            return false;
        }

        if (element.offset_bytes >= desc.stride_bytes) {
            return false;
        }

        const std::size_t remaining_bytes = desc.stride_bytes - element.offset_bytes;
        if (element_bytes > remaining_bytes) {
            return false;
        }
    }

    return has_position;
}

bool NullRhiDevice::IsDrawRangeValid(const RhiVertexBufferView &view, const RhiDrawDesc &desc) const {
    if (view.stride_bytes == 0U) {
        return false;
    }

    const std::size_t available_vertices = view.size_bytes / view.stride_bytes;
    if (desc.first_vertex > available_vertices) {
        return false;
    }

    const std::size_t remaining_vertices = available_vertices - desc.first_vertex;
    return desc.vertex_count <= remaining_vertices;
}

bool NullRhiDevice::IsColorTargetDescValid(const RhiColorTargetDesc &desc) const {
    if (desc.extent.width == 0U) {
        return false;
    }

    if (desc.extent.height == 0U) {
        return false;
    }

    if (desc.extent.width > MAX_COLOR_TARGET_EXTENT) {
        return false;
    }

    if (desc.extent.height > MAX_COLOR_TARGET_EXTENT) {
        return false;
    }

    return true;
}

bool NullRhiDevice::IsBufferDescValid(const RhiBufferDesc &desc, std::span<const std::uint8_t> initial_bytes) const {
    if (desc.usage == RhiBufferUsage::Unsupported) {
        return false;
    }

    if (desc.size_bytes == 0U) {
        return false;
    }

    if (desc.size_bytes > MAX_RHI_BUFFER_BYTES) {
        return false;
    }

    if (initial_bytes.size() > desc.size_bytes) {
        return false;
    }

    if (desc.usage == RhiBufferUsage::Constant && (desc.size_bytes % RHI_CONSTANT_BUFFER_ALIGNMENT) != 0U) {
        return false;
    }

    return true;
}

bool NullRhiDevice::IsTextureDescValid(const RhiTextureDesc &desc, std::span<const std::uint8_t> initial_bytes) const {
    if (desc.format != RhiFormat::Rgba8Unorm) {
        return false;
    }

    if (desc.extent.width == 0U) {
        return false;
    }

    if (desc.extent.height == 0U) {
        return false;
    }

    if (desc.extent.width > MAX_COLOR_TARGET_EXTENT) {
        return false;
    }

    if (desc.extent.height > MAX_COLOR_TARGET_EXTENT) {
        return false;
    }

    if (!initial_bytes.empty() && initial_bytes.size() != TextureByteCount(desc)) {
        return false;
    }

    return true;
}

bool NullRhiDevice::IsShaderModuleDescValid(const RhiShaderModuleDesc &desc) const {
    if (desc.stage == RhiShaderStage::Unsupported) {
        return false;
    }

    if (desc.bytecode.empty()) {
        return false;
    }

    if (desc.bytecode.size() > MAX_RHI_SHADER_BYTECODE_BYTES) {
        return false;
    }

    return true;
}

bool NullRhiDevice::IsPipelineDescValid(const RhiPipelineDesc &desc) const {
    if (desc.input_layout.element_count != 0U && !IsInputLayoutDescValid(desc.input_layout)) {
        return false;
    }

    if (!IsShaderModuleHandleValid(desc.vertex_shader)) {
        return false;
    }

    if (!IsShaderModuleHandleValid(desc.pixel_shader)) {
        return false;
    }

    const NullShaderModuleSlot &vertex_shader = shader_modules_[desc.vertex_shader.slot];
    if (vertex_shader.desc.stage != RhiShaderStage::Vertex) {
        return false;
    }

    const NullShaderModuleSlot &pixel_shader = shader_modules_[desc.pixel_shader.slot];
    if (pixel_shader.desc.stage != RhiShaderStage::Pixel) {
        return false;
    }

    return true;
}

std::size_t NullRhiDevice::PixelByteCount(const RhiColorTargetDesc &desc) const {
    return static_cast<std::size_t>(desc.extent.width) * static_cast<std::size_t>(desc.extent.height) * RGBA8_BYTES_PER_PIXEL;
}

std::size_t NullRhiDevice::TextureByteCount(const RhiTextureDesc &desc) const {
    return static_cast<std::size_t>(desc.extent.width) * static_cast<std::size_t>(desc.extent.height) * RGBA8_BYTES_PER_PIXEL;
}

RhiFenceHandle NullRhiDevice::SignalFence(std::size_t byte_count) {
    ++fence_generation_;
    if (fence_generation_ == INVALID_GENERATION) {
        ++fence_generation_;
    }

    ++snapshot_.resources.signaled_fence_count;
    snapshot_.resources.last_update_bytes = byte_count;
    return RhiFenceHandle{0U, fence_generation_};
}

void NullRhiDevice::ExecuteClear(RhiTextureHandle handle, RhiColor color) {
    if (!IsTargetHandleValid(handle)) {
        return;
    }

    RhiTargetSlot& slot = targets_[handle.slot];
    for (std::size_t index = 0U; index < slot.bytes.size(); index += RGBA8_BYTES_PER_PIXEL) {
        slot.bytes[index] = color.r;
        slot.bytes[index + 1U] = color.g;
        slot.bytes[index + 2U] = color.b;
        slot.bytes[index + 3U] = color.a;
    }
}
}
