// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Src/NullRhiDevice.cpp

#include "YuEngine/Rhi/NullRhiDevice.h"

#include <algorithm>
#include <array>

#include "YuEngine/Rhi/RhiDeviceFactory.h"
#include "YuEngine/Rhi/RhiConstants.h"

namespace yuengine::rhi {
namespace {
constexpr std::uint32_t INVALID_GENERATION = 0U;
using SampledTextureSlotMask = std::array<bool, MAX_RHI_SAMPLED_TEXTURE_SLOTS>;
using SamplerSlotMask = std::array<bool, MAX_RHI_SAMPLER_SLOTS>;

bool IsConstantBufferShaderStageValid(RhiShaderStage stage) {
    if (stage == RhiShaderStage::Vertex) {
        return true;
    }

    return stage == RhiShaderStage::Pixel;
}

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

bool HasSampledTextureWithoutSampler(
    const SampledTextureSlotMask &sampled_textures,
    const SamplerSlotMask &samplers) {
    for (std::size_t index = 0U; index < sampled_textures.size(); ++index) {
        if (!sampled_textures[index]) {
            continue;
        }

        if (index >= samplers.size()) {
            return true;
        }

        if (!samplers[index]) {
            return true;
        }
    }

    return false;
}

bool HasSamplerWithoutSampledTexture(
    const SampledTextureSlotMask &sampled_textures,
    const SamplerSlotMask &samplers) {
    for (std::size_t index = 0U; index < samplers.size(); ++index) {
        if (!samplers[index]) {
            continue;
        }

        if (index >= sampled_textures.size()) {
            return true;
        }

        if (!sampled_textures[index]) {
            return true;
        }
    }

    return false;
}

bool ByteRangeFits(
    std::uint64_t byte_offset,
    std::size_t byte_count,
    std::size_t total_byte_count) {
    const std::uint64_t total_bytes = static_cast<std::uint64_t>(total_byte_count);
    if (byte_offset > total_bytes) {
        return false;
    }

    const std::uint64_t remaining_bytes = total_bytes - byte_offset;
    return static_cast<std::uint64_t>(byte_count) <= remaining_bytes;
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
      primitive_retirements_(),
      capabilities_{},
      snapshot_{},
      submitted_handle_{},
      presented_handle_{},
      generation_seed_(INVALID_GENERATION),
      fence_generation_(INVALID_GENERATION),
      next_primitive_retirement_id_(1U),
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

    primitive_retirements_.assign(MAX_RHI_PRIMITIVE_RETIREMENTS, RhiPrimitiveRetirementRecord{});
    next_primitive_retirement_id_ = 1U;
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
    snapshot_.resources.primitive_retirement.capacity = MAX_RHI_PRIMITIVE_RETIREMENTS;
    snapshot_.resources.primitive_retirement.next_retirement_id = next_primitive_retirement_id_;
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

RhiStatus NullRhiDevice::ResizeSwapchain(
    const RhiSwapchainResizeRequest &request,
    RhiSwapchainResizeResult &out_result) {
    static_cast<void>(request);
    out_result = RhiSwapchainResizeResult{};
    out_result.status = RhiStatus::UnsupportedBackend;
    out_result.snapshot = snapshot_.swapchain;
    return RecordFailure(RhiStatus::UnsupportedBackend);
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

RhiStatus NullRhiDevice::RecordBindIndexBuffer(RhiCommandList &command_list, const RhiIndexBufferView &view) {
    if (!IsIndexBufferViewValid(view)) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    const RhiStatus status = command_list.RecordBindIndexBuffer(view);
    if (status != RhiStatus::Success) {
        return RecordFailure(status);
    }

    ++snapshot_.recorded_command_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::RecordBindSampledTexture(
    RhiCommandList &command_list,
    const RhiSampledTextureBinding &binding) {
    if (binding.slot >= MAX_RHI_SAMPLED_TEXTURE_SLOTS) {
        return RecordSampledTextureBindFailure(RhiStatus::InvalidDescriptor);
    }

    if (!IsTextureHandleValid(binding.texture)) {
        return RecordSampledTextureBindFailure(RhiStatus::InvalidHandle);
    }

    const RhiStatus status = command_list.RecordBindSampledTexture(binding);
    if (status != RhiStatus::Success) {
        return RecordSampledTextureBindFailure(status);
    }

    ++snapshot_.recorded_command_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::RecordBindSampler(RhiCommandList &command_list, const RhiSamplerBinding &binding) {
    if (binding.slot >= MAX_RHI_SAMPLER_SLOTS) {
        return RecordSamplerBindFailure(RhiStatus::InvalidDescriptor);
    }

    if (!IsSamplerHandleValid(binding.sampler)) {
        return RecordSamplerBindFailure(RhiStatus::InvalidHandle);
    }

    const RhiStatus status = command_list.RecordBindSampler(binding);
    if (status != RhiStatus::Success) {
        return RecordSamplerBindFailure(status);
    }

    ++snapshot_.recorded_command_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::RecordBindConstantBuffer(
    RhiCommandList &command_list,
    const RhiConstantBufferBinding &binding) {
    if (binding.slot >= MAX_RHI_CONSTANT_BUFFER_SLOTS) {
        return RecordConstantBufferBindFailure(RhiStatus::InvalidDescriptor);
    }

    if (!IsConstantBufferShaderStageValid(binding.stage)) {
        return RecordConstantBufferBindFailure(RhiStatus::InvalidDescriptor);
    }

    if (!IsBufferHandleValid(binding.buffer)) {
        return RecordConstantBufferBindFailure(RhiStatus::InvalidHandle);
    }

    const NullBufferSlot &slot = buffers_[binding.buffer.slot];
    if (slot.desc.usage != RhiBufferUsage::Constant) {
        return RecordConstantBufferBindFailure(RhiStatus::InvalidDescriptor);
    }

    const RhiStatus status = command_list.RecordBindConstantBuffer(binding);
    if (status != RhiStatus::Success) {
        return RecordConstantBufferBindFailure(status);
    }

    ++snapshot_.recorded_command_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::RecordBindBlendState(RhiCommandList &command_list, const RhiBlendStateDesc &desc) {
    if (!IsBlendStateDescValid(desc)) {
        return RecordBlendStateBindFailure(RhiStatus::InvalidDescriptor);
    }

    const RhiStatus status = command_list.RecordBindBlendState(desc);
    if (status != RhiStatus::Success) {
        return RecordBlendStateBindFailure(status);
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

RhiStatus NullRhiDevice::RecordDrawIndexed(RhiCommandList &command_list, const RhiDrawIndexedDesc &desc) {
    if (!IsDrawIndexedDescValid(desc)) {
        return RecordIndexedDrawFailure(RhiStatus::InvalidDescriptor);
    }

    const RhiStatus status = command_list.RecordDrawIndexed(desc);
    if (status != RhiStatus::Success) {
        return RecordIndexedDrawFailure(status);
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
    bool has_index_buffer = false;
    SampledTextureSlotMask has_sampled_texture{};
    SamplerSlotMask has_sampler{};
    RhiPipelineHandle bound_pipeline{};
    RhiVertexBufferView bound_vertex_buffer{};
    RhiIndexBufferView bound_index_buffer{};
    std::uint64_t submitted_draw_count = 0U;
    std::uint64_t submitted_indexed_draw_count = 0U;
    std::uint64_t submitted_sampled_texture_bind_count = 0U;
    std::uint64_t submitted_sampler_bind_count = 0U;
    std::uint64_t submitted_constant_buffer_bind_count = 0U;
    std::uint64_t submitted_blend_state_bind_count = 0U;
    std::uint32_t last_draw_vertex_count = 0U;
    std::uint32_t last_indexed_draw_index_count = 0U;
    std::uint32_t last_bound_sampled_texture_slot = 0U;
    std::uint32_t last_bound_sampler_slot = 0U;
    std::uint32_t last_bound_constant_buffer_slot = 0U;
    RhiShaderStage last_bound_constant_buffer_stage = RhiShaderStage::Unsupported;
    RhiBlendStateDesc bound_blend_state{};
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

        if (command.type == RhiCommandType::BindIndexBuffer) {
            if (!IsIndexBufferViewValid(command.index_buffer)) {
                return RecordFailure(RhiStatus::InvalidDescriptor);
            }

            bound_index_buffer = command.index_buffer;
            has_index_buffer = true;
            continue;
        }

        if (command.type == RhiCommandType::BindSampledTexture) {
            if (!IsSampledTextureBindingValid(command.sampled_texture)) {
                if (command.sampled_texture.slot >= MAX_RHI_SAMPLED_TEXTURE_SLOTS) {
                    return RecordSampledTextureBindFailure(RhiStatus::InvalidDescriptor);
                }

                return RecordSampledTextureBindFailure(RhiStatus::InvalidHandle);
            }

            const std::size_t slot = command.sampled_texture.slot;
            has_sampled_texture[slot] = true;
            ++submitted_sampled_texture_bind_count;
            last_bound_sampled_texture_slot = command.sampled_texture.slot;
            continue;
        }

        if (command.type == RhiCommandType::BindSampler) {
            if (!IsSamplerBindingValid(command.sampler)) {
                if (command.sampler.slot >= MAX_RHI_SAMPLER_SLOTS) {
                    return RecordSamplerBindFailure(RhiStatus::InvalidDescriptor);
                }

                return RecordSamplerBindFailure(RhiStatus::InvalidHandle);
            }

            const std::size_t slot = command.sampler.slot;
            has_sampler[slot] = true;
            ++submitted_sampler_bind_count;
            last_bound_sampler_slot = command.sampler.slot;
            continue;
        }

        if (command.type == RhiCommandType::BindConstantBuffer) {
            if (!IsConstantBufferBindingValid(command.constant_buffer)) {
                if (command.constant_buffer.slot >= MAX_RHI_CONSTANT_BUFFER_SLOTS) {
                    return RecordConstantBufferBindFailure(RhiStatus::InvalidDescriptor);
                }

                if (!IsConstantBufferShaderStageValid(command.constant_buffer.stage)) {
                    return RecordConstantBufferBindFailure(RhiStatus::InvalidDescriptor);
                }

                if (!IsBufferHandleValid(command.constant_buffer.buffer)) {
                    return RecordConstantBufferBindFailure(RhiStatus::InvalidHandle);
                }

                return RecordConstantBufferBindFailure(RhiStatus::InvalidDescriptor);
            }

            ++submitted_constant_buffer_bind_count;
            last_bound_constant_buffer_slot = command.constant_buffer.slot;
            last_bound_constant_buffer_stage = command.constant_buffer.stage;
            continue;
        }

        if (command.type == RhiCommandType::BindBlendState) {
            if (!IsBlendStateDescValid(command.blend_state)) {
                return RecordBlendStateBindFailure(RhiStatus::InvalidDescriptor);
            }

            bound_blend_state = command.blend_state;
            ++submitted_blend_state_bind_count;
            continue;
        }

        if (command.type == RhiCommandType::Draw) {
            if (!has_pipeline) {
                return RecordFailure(RhiStatus::InvalidLifecycle);
            }

            if (!has_vertex_buffer) {
                return RecordFailure(RhiStatus::InvalidLifecycle);
            }

            if (HasSamplerWithoutSampledTexture(has_sampled_texture, has_sampler)) {
                return RecordSampledTextureBindFailure(RhiStatus::InvalidLifecycle);
            }

            if (HasSampledTextureWithoutSampler(has_sampled_texture, has_sampler)) {
                return RecordSamplerBindFailure(RhiStatus::InvalidLifecycle);
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

        if (command.type == RhiCommandType::DrawIndexed) {
            if (!has_pipeline) {
                return RecordIndexedDrawFailure(RhiStatus::InvalidLifecycle);
            }

            if (!has_vertex_buffer) {
                return RecordIndexedDrawFailure(RhiStatus::InvalidLifecycle);
            }

            if (!has_index_buffer) {
                return RecordIndexedDrawFailure(RhiStatus::InvalidLifecycle);
            }

            if (HasSamplerWithoutSampledTexture(has_sampled_texture, has_sampler)) {
                return RecordSampledTextureBindFailure(RhiStatus::InvalidLifecycle);
            }

            if (HasSampledTextureWithoutSampler(has_sampled_texture, has_sampler)) {
                return RecordSamplerBindFailure(RhiStatus::InvalidLifecycle);
            }

            if (!IsDrawIndexedDescValid(command.draw_indexed)) {
                return RecordIndexedDrawFailure(RhiStatus::InvalidDescriptor);
            }

            if (!IsIndexedDrawRangeValid(bound_index_buffer, command.draw_indexed)) {
                return RecordIndexedDrawFailure(RhiStatus::InvalidDescriptor);
            }

            const NullPipelineSlot &pipeline = pipelines_[bound_pipeline.slot];
            if (!IsInputLayoutDescValid(pipeline.desc.input_layout)) {
                return RecordIndexedDrawFailure(RhiStatus::InvalidDescriptor);
            }

            ++submitted_indexed_draw_count;
            last_indexed_draw_index_count = command.draw_indexed.index_count;
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
    snapshot_.submitted_indexed_draw_count += submitted_indexed_draw_count;
    snapshot_.submitted_sampled_texture_bind_count += submitted_sampled_texture_bind_count;
    snapshot_.submitted_sampler_bind_count += submitted_sampler_bind_count;
    snapshot_.submitted_constant_buffer_bind_count += submitted_constant_buffer_bind_count;
    snapshot_.submitted_blend_state_bind_count += submitted_blend_state_bind_count;
    snapshot_.last_draw_vertex_count = last_draw_vertex_count;
    snapshot_.last_indexed_draw_index_count = last_indexed_draw_index_count;
    if (submitted_sampled_texture_bind_count > 0U) {
        snapshot_.last_bound_sampled_texture_slot = last_bound_sampled_texture_slot;
    }

    if (submitted_sampler_bind_count > 0U) {
        snapshot_.last_bound_sampler_slot = last_bound_sampler_slot;
    }

    if (submitted_constant_buffer_bind_count > 0U) {
        snapshot_.last_bound_constant_buffer_slot = last_bound_constant_buffer_slot;
        snapshot_.last_bound_constant_buffer_stage = last_bound_constant_buffer_stage;
    }

    if (submitted_blend_state_bind_count > 0U) {
        snapshot_.last_alpha_blend_enabled = bound_blend_state.mode == RhiBlendMode::AlphaOver;
        snapshot_.last_blend_constant_alpha = bound_blend_state.constant_alpha;
    }

    if (submitted_indexed_draw_count > 0U) {
        snapshot_.last_bound_index_buffer_offset_bytes = bound_index_buffer.offset_bytes;
        snapshot_.last_bound_index_buffer_size_bytes = bound_index_buffer.size_bytes;
    }

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

    const RhiTargetSlot &slot = targets_[presented_handle_.slot];
    if (slot.desc.extent.width > MAX_CAPTURE_FIXTURE_EXTENT || slot.desc.extent.height > MAX_CAPTURE_FIXTURE_EXTENT) {
        RecordFailure(RhiStatus::CapacityExceeded);
        snapshot_.last_capture_bytes_written = 0U;
        snapshot_.last_capture_extent = RhiExtent2D{};
        return RhiCaptureResult{RhiStatus::CapacityExceeded, 0U};
    }

    const std::size_t byte_count = slot.bytes.size();
    if (destination.size() < byte_count) {
        RecordFailure(RhiStatus::CapacityExceeded);
        snapshot_.last_capture_bytes_written = 0U;
        snapshot_.last_capture_extent = RhiExtent2D{};
        return RhiCaptureResult{RhiStatus::CapacityExceeded, 0U};
    }

    std::copy(slot.bytes.begin(), slot.bytes.end(), destination.begin());
    ++snapshot_.capture_count;
    snapshot_.last_capture_bytes_written = byte_count;
    snapshot_.last_capture_extent = slot.desc.extent;
    return RhiCaptureResult{RhiStatus::Success, byte_count, slot.desc.extent};
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
    RhiFenceHandle &out_fence,
    std::uint64_t destination_byte_offset) {
    out_fence = RhiFenceHandle{};
    if (!IsBufferHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    if (bytes.empty()) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    NullBufferSlot &slot = buffers_[handle.slot];
    if (!ByteRangeFits(destination_byte_offset, bytes.size(), slot.bytes.size())) {
        return RecordFailure(RhiStatus::CapacityExceeded);
    }

    const std::size_t destination_byte_index = static_cast<std::size_t>(destination_byte_offset);
    std::copy(bytes.begin(), bytes.end(), slot.bytes.begin() + destination_byte_index);
    out_fence = SignalFence(bytes.size());
    ++snapshot_.resources.updated_primitive_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::DestroyBuffer(RhiBufferHandle handle) {
    if (!IsBufferHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    RetireBufferSlot(handle.slot);
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
    RhiFenceHandle &out_fence,
    std::uint64_t destination_byte_offset) {
    out_fence = RhiFenceHandle{};
    if (!IsTextureHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    if (bytes.empty()) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    NullTextureSlot &slot = textures_[handle.slot];
    if (!ByteRangeFits(destination_byte_offset, bytes.size(), slot.bytes.size())) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    const std::size_t destination_byte_index = static_cast<std::size_t>(destination_byte_offset);
    std::copy(bytes.begin(), bytes.end(), slot.bytes.begin() + destination_byte_index);
    out_fence = SignalFence(bytes.size());
    ++snapshot_.resources.updated_primitive_count;
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::DestroyTexture(RhiTextureHandle handle) {
    if (!IsTextureHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    RetireTextureSlot(handle.slot);
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

    RetireSamplerSlot(handle.slot);
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

    RetireShaderModuleSlot(handle.slot);
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

    RetirePipelineSlot(handle.slot);
    return RhiStatus::Success;
}

RhiStatus NullRhiDevice::RequestPrimitiveRetirement(
    const RhiPrimitiveRetirementRequest &request,
    RhiPrimitiveRetirementRecord &out_record) {
    out_record = RhiPrimitiveRetirementRecord{};
    out_record.request_id = request.request_id;
    out_record.primitive_kind = request.primitive_kind;
    out_record.primitive_slot = request.primitive_slot;
    out_record.primitive_generation = request.primitive_generation;
    out_record.wait_fence = request.wait_fence;

    if (!is_initialized_) {
        out_record.status = RhiPrimitiveRetirementStatus::RejectedInvalidRequest;
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (request.request_id == 0U || request.primitive_kind == RhiPrimitiveKind::Unsupported) {
        out_record.status = RhiPrimitiveRetirementStatus::RejectedInvalidRequest;
        ++snapshot_.resources.primitive_retirement.rejected_count;
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    if (!IsRetirementRequestValid(request)) {
        if (IsRetirementWrongKind(request)) {
            out_record.status = RhiPrimitiveRetirementStatus::RejectedWrongKind;
            ++snapshot_.resources.primitive_retirement.wrong_kind_count;
            ++snapshot_.resources.primitive_retirement.rejected_count;
            return RecordFailure(RhiStatus::InvalidHandle);
        }

        out_record.status = RhiPrimitiveRetirementStatus::RejectedInvalidHandle;
        ++snapshot_.resources.primitive_retirement.invalid_handle_count;
        ++snapshot_.resources.primitive_retirement.rejected_count;
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    if (IsRetirementDuplicate(request)) {
        out_record.status = RhiPrimitiveRetirementStatus::RejectedDuplicate;
        ++snapshot_.resources.primitive_retirement.duplicate_request_count;
        ++snapshot_.resources.primitive_retirement.rejected_count;
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    for (RhiPrimitiveRetirementRecord &record : primitive_retirements_) {
        if (record.status != RhiPrimitiveRetirementStatus::Invalid) {
            continue;
        }

        record = out_record;
        record.retirement_id = next_primitive_retirement_id_;
        record.status = RhiPrimitiveRetirementStatus::Pending;
        out_record = record;
        ++next_primitive_retirement_id_;
        ++snapshot_.resources.primitive_retirement.pending_count;
        ++snapshot_.resources.primitive_retirement.requested_count;
        snapshot_.resources.primitive_retirement.next_retirement_id = next_primitive_retirement_id_;
        return RhiStatus::Success;
    }

    out_record.status = RhiPrimitiveRetirementStatus::RejectedCapacity;
    ++snapshot_.resources.primitive_retirement.capacity_rejected_count;
    ++snapshot_.resources.primitive_retirement.rejected_count;
    return RecordFailure(RhiStatus::CapacityExceeded);
}

RhiStatus NullRhiDevice::QueryPrimitiveRetirement(
    std::uint64_t retirement_id,
    RhiPrimitiveRetirementRecord &out_record) const {
    out_record = RhiPrimitiveRetirementRecord{};
    if (retirement_id == 0U) {
        return RhiStatus::InvalidHandle;
    }

    for (const RhiPrimitiveRetirementRecord &record : primitive_retirements_) {
        if (record.retirement_id != retirement_id) {
            continue;
        }

        if (record.status == RhiPrimitiveRetirementStatus::Invalid) {
            continue;
        }

        out_record = record;
        return RhiStatus::Success;
    }

    return RhiStatus::InvalidHandle;
}

RhiStatus NullRhiDevice::DrainPrimitiveRetirements(
    const RhiPrimitiveRetirementDrainRequest &request,
    RhiPrimitiveRetirementDrainResult &out_result) {
    out_result = RhiPrimitiveRetirementDrainResult{};
    if (!is_initialized_) {
        out_result.status = RecordFailure(RhiStatus::InvalidLifecycle);
        return out_result.status;
    }

    if (request.max_retirements == 0U) {
        out_result.status = RecordFailure(RhiStatus::InvalidDescriptor);
        return out_result.status;
    }

    for (RhiPrimitiveRetirementRecord &record : primitive_retirements_) {
        if (out_result.drained_count >= request.max_retirements) {
            break;
        }

        if (record.status != RhiPrimitiveRetirementStatus::Pending) {
            continue;
        }

        if (!IsRetirementFenceReady(record.wait_fence)) {
            ++out_result.rejected_count;
            out_result.last_rejection_status = RhiPrimitiveRetirementStatus::RejectedFenceNotReady;
            ++snapshot_.resources.primitive_retirement.fence_not_ready_count;
            ++snapshot_.resources.primitive_retirement.rejected_count;
            continue;
        }

        DrainRetirementRecord(record);
        ++out_result.drained_count;
    }

    out_result.pending_count = snapshot_.resources.primitive_retirement.pending_count;
    if (out_result.rejected_count > 0U && out_result.drained_count == 0U) {
        out_result.status = RhiStatus::InvalidLifecycle;
        return out_result.status;
    }

    out_result.status = RhiStatus::Success;
    return out_result.status;
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

RhiStatus NullRhiDevice::RecordIndexedDrawFailure(RhiStatus status) {
    ++snapshot_.rejected_indexed_draw_count;
    return RecordFailure(status);
}

RhiStatus NullRhiDevice::RecordSampledTextureBindFailure(RhiStatus status) {
    ++snapshot_.rejected_sampled_texture_bind_count;
    return RecordFailure(status);
}

RhiStatus NullRhiDevice::RecordSamplerBindFailure(RhiStatus status) {
    ++snapshot_.rejected_sampler_bind_count;
    return RecordFailure(status);
}

RhiStatus NullRhiDevice::RecordConstantBufferBindFailure(RhiStatus status) {
    ++snapshot_.rejected_constant_buffer_bind_count;
    return RecordFailure(status);
}

RhiStatus NullRhiDevice::RecordBlendStateBindFailure(RhiStatus status) {
    ++snapshot_.rejected_blend_state_bind_count;
    return RecordFailure(status);
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

bool NullRhiDevice::IsRetirementRequestValid(const RhiPrimitiveRetirementRequest &request) const {
    if (request.primitive_generation == INVALID_GENERATION) {
        return false;
    }

    if (request.primitive_kind == RhiPrimitiveKind::Buffer) {
        return IsBufferHandleValid(RhiBufferHandle{request.primitive_slot, request.primitive_generation});
    }

    if (request.primitive_kind == RhiPrimitiveKind::Texture) {
        return IsTextureHandleValid(RhiTextureHandle{request.primitive_slot, request.primitive_generation});
    }

    if (request.primitive_kind == RhiPrimitiveKind::Sampler) {
        return IsSamplerHandleValid(RhiSamplerHandle{request.primitive_slot, request.primitive_generation});
    }

    if (request.primitive_kind == RhiPrimitiveKind::ShaderModule) {
        return IsShaderModuleHandleValid(RhiShaderModuleHandle{request.primitive_slot, request.primitive_generation});
    }

    if (request.primitive_kind == RhiPrimitiveKind::Pipeline) {
        return IsPipelineHandleValid(RhiPipelineHandle{request.primitive_slot, request.primitive_generation});
    }

    if (request.primitive_kind == RhiPrimitiveKind::Fence) {
        if (request.primitive_slot != 0U) {
            return false;
        }

        return request.primitive_generation <= fence_generation_;
    }

    return false;
}

bool NullRhiDevice::IsRetirementWrongKind(const RhiPrimitiveRetirementRequest &request) const {
    if (request.primitive_kind != RhiPrimitiveKind::Buffer) {
        if (IsBufferHandleValid(RhiBufferHandle{request.primitive_slot, request.primitive_generation})) {
            return true;
        }
    }

    if (request.primitive_kind != RhiPrimitiveKind::Texture) {
        if (IsTextureHandleValid(RhiTextureHandle{request.primitive_slot, request.primitive_generation})) {
            return true;
        }
    }

    if (request.primitive_kind != RhiPrimitiveKind::Sampler) {
        if (IsSamplerHandleValid(RhiSamplerHandle{request.primitive_slot, request.primitive_generation})) {
            return true;
        }
    }

    if (request.primitive_kind != RhiPrimitiveKind::ShaderModule) {
        if (IsShaderModuleHandleValid(RhiShaderModuleHandle{request.primitive_slot, request.primitive_generation})) {
            return true;
        }
    }

    if (request.primitive_kind != RhiPrimitiveKind::Pipeline) {
        if (IsPipelineHandleValid(RhiPipelineHandle{request.primitive_slot, request.primitive_generation})) {
            return true;
        }
    }

    return false;
}

bool NullRhiDevice::IsRetirementDuplicate(const RhiPrimitiveRetirementRequest &request) const {
    for (const RhiPrimitiveRetirementRecord &record : primitive_retirements_) {
        if (record.status != RhiPrimitiveRetirementStatus::Pending) {
            continue;
        }

        if (record.primitive_kind != request.primitive_kind) {
            continue;
        }

        if (record.primitive_slot != request.primitive_slot) {
            continue;
        }

        if (record.primitive_generation != request.primitive_generation) {
            continue;
        }

        return true;
    }

    return false;
}

bool NullRhiDevice::IsRetirementFenceReady(RhiFenceHandle fence) const {
    if (fence.generation == INVALID_GENERATION) {
        return true;
    }

    if (fence.slot != 0U) {
        return false;
    }

    return fence.generation <= fence_generation_;
}

void NullRhiDevice::RetireBufferSlot(std::uint32_t slot_index) {
    NullBufferSlot &slot = buffers_[slot_index];
    slot.is_active = false;
    slot.desc = RhiBufferDesc{};
    slot.bytes.clear();
    ++slot.generation;
    --snapshot_.resources.buffer_count;
    ++snapshot_.resources.destroyed_primitive_count;
}

void NullRhiDevice::RetireTextureSlot(std::uint32_t slot_index) {
    NullTextureSlot &slot = textures_[slot_index];
    slot.is_active = false;
    slot.desc = RhiTextureDesc{};
    slot.bytes.clear();
    ++slot.generation;
    --snapshot_.resources.texture_count;
    ++snapshot_.resources.destroyed_primitive_count;
}

void NullRhiDevice::RetireSamplerSlot(std::uint32_t slot_index) {
    NullSamplerSlot &slot = samplers_[slot_index];
    slot.is_active = false;
    slot.desc = RhiSamplerDesc{};
    ++slot.generation;
    --snapshot_.resources.sampler_count;
    ++snapshot_.resources.destroyed_primitive_count;
}

void NullRhiDevice::RetireShaderModuleSlot(std::uint32_t slot_index) {
    NullShaderModuleSlot &slot = shader_modules_[slot_index];
    slot.is_active = false;
    slot.desc = RhiShaderModuleDesc{};
    slot.bytes.clear();
    ++slot.generation;
    --snapshot_.resources.shader_module_count;
    ++snapshot_.resources.destroyed_primitive_count;
}

void NullRhiDevice::RetirePipelineSlot(std::uint32_t slot_index) {
    NullPipelineSlot &slot = pipelines_[slot_index];
    slot.is_active = false;
    slot.desc = RhiPipelineDesc{};
    ++slot.generation;
    --snapshot_.resources.pipeline_count;
    ++snapshot_.resources.destroyed_primitive_count;
}

void NullRhiDevice::DrainRetirementRecord(RhiPrimitiveRetirementRecord &record) {
    if (record.primitive_kind == RhiPrimitiveKind::Buffer) {
        RetireBufferSlot(record.primitive_slot);
    }

    if (record.primitive_kind == RhiPrimitiveKind::Texture) {
        RetireTextureSlot(record.primitive_slot);
    }

    if (record.primitive_kind == RhiPrimitiveKind::Sampler) {
        RetireSamplerSlot(record.primitive_slot);
    }

    if (record.primitive_kind == RhiPrimitiveKind::ShaderModule) {
        RetireShaderModuleSlot(record.primitive_slot);
    }

    if (record.primitive_kind == RhiPrimitiveKind::Pipeline) {
        RetirePipelineSlot(record.primitive_slot);
    }

    if (record.primitive_kind == RhiPrimitiveKind::Fence) {
        ++snapshot_.resources.destroyed_primitive_count;
    }

    record.status = RhiPrimitiveRetirementStatus::Drained;
    --snapshot_.resources.primitive_retirement.pending_count;
    ++snapshot_.resources.primitive_retirement.drained_count;
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

bool NullRhiDevice::IsIndexBufferViewValid(const RhiIndexBufferView &view) const {
    if (!IsBufferHandleValid(view.buffer)) {
        return false;
    }

    const NullBufferSlot &slot = buffers_[view.buffer.slot];
    if (slot.desc.usage != RhiBufferUsage::Index) {
        return false;
    }

    const std::size_t index_bytes = IndexFormatByteCount(view.format);
    if (index_bytes == 0U) {
        return false;
    }

    if (view.size_bytes == 0U) {
        return false;
    }

    if ((view.size_bytes % index_bytes) != 0U) {
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

bool NullRhiDevice::IsSampledTextureBindingValid(const RhiSampledTextureBinding &binding) const {
    if (binding.slot >= MAX_RHI_SAMPLED_TEXTURE_SLOTS) {
        return false;
    }

    return IsTextureHandleValid(binding.texture);
}

bool NullRhiDevice::IsSamplerBindingValid(const RhiSamplerBinding &binding) const {
    if (binding.slot >= MAX_RHI_SAMPLER_SLOTS) {
        return false;
    }

    return IsSamplerHandleValid(binding.sampler);
}

bool NullRhiDevice::IsConstantBufferBindingValid(const RhiConstantBufferBinding &binding) const {
    if (binding.slot >= MAX_RHI_CONSTANT_BUFFER_SLOTS) {
        return false;
    }

    if (!IsConstantBufferShaderStageValid(binding.stage)) {
        return false;
    }

    if (!IsBufferHandleValid(binding.buffer)) {
        return false;
    }

    const NullBufferSlot &slot = buffers_[binding.buffer.slot];
    return slot.desc.usage == RhiBufferUsage::Constant;
}

bool NullRhiDevice::IsBlendStateDescValid(const RhiBlendStateDesc &desc) const {
    if (desc.mode == RhiBlendMode::Opaque) {
        return true;
    }

    return desc.mode == RhiBlendMode::AlphaOver;
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

bool NullRhiDevice::IsDrawIndexedDescValid(const RhiDrawIndexedDesc &desc) const {
    if (desc.topology != RhiPrimitiveTopology::TriangleList) {
        return false;
    }

    if (desc.index_count == 0U) {
        return false;
    }

    return true;
}

bool NullRhiDevice::IsInputLayoutDescValid(const RhiInputLayoutDesc &desc) const {
    if (desc.element_count == 0U) {
        return desc.stride_bytes == 0U;
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

bool NullRhiDevice::IsIndexedDrawRangeValid(const RhiIndexBufferView &view, const RhiDrawIndexedDesc &desc) const {
    const std::size_t index_bytes = IndexFormatByteCount(view.format);
    if (index_bytes == 0U) {
        return false;
    }

    const std::size_t available_indices = view.size_bytes / index_bytes;
    const std::size_t first_index = static_cast<std::size_t>(desc.first_index);
    if (first_index > available_indices) {
        return false;
    }

    const std::size_t remaining_indices = available_indices - first_index;
    return static_cast<std::size_t>(desc.index_count) <= remaining_indices;
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

std::size_t NullRhiDevice::IndexFormatByteCount(RhiIndexFormat format) const {
    if (format == RhiIndexFormat::Uint16) {
        return sizeof(std::uint16_t);
    }

    if (format == RhiIndexFormat::Uint32) {
        return sizeof(std::uint32_t);
    }

    return 0U;
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
