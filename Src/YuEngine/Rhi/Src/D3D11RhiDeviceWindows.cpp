// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Src/D3D11RhiDeviceWindows.cpp

#include "YuEngine/Rhi/RhiDeviceFactory.h"

#include <algorithm>
#include <array>
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
constexpr std::uint32_t INVALID_GENERATION = 0U;
constexpr std::size_t NO_STORAGE_REQUIRED = 0U;
using SampledTextureSlotMask = std::array<bool, MAX_RHI_SAMPLED_TEXTURE_SLOTS>;
using SamplerSlotMask = std::array<bool, MAX_RHI_SAMPLER_SLOTS>;

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

#if defined(_WIN32)
DXGI_FORMAT NativeInputElementFormat(RhiInputElementFormat format) {
    if (format == RhiInputElementFormat::Float32x2) {
        return DXGI_FORMAT_R32G32_FLOAT;
    }

    if (format == RhiInputElementFormat::Float32x3) {
        return DXGI_FORMAT_R32G32B32_FLOAT;
    }

    if (format == RhiInputElementFormat::Float32x4) {
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    }

    return DXGI_FORMAT_UNKNOWN;
}

const char *NativeInputElementSemanticName(RhiInputElementSemantic semantic) {
    if (semantic == RhiInputElementSemantic::Position) {
        return "POSITION";
    }

    if (semantic == RhiInputElementSemantic::Color) {
        return "COLOR";
    }

    if (semantic == RhiInputElementSemantic::TexCoord) {
        return "TEXCOORD";
    }

    return "";
}
#endif

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
      buffers_{},
      textures_{},
      samplers_{},
      shader_modules_{},
      pipelines_{},
      primitive_retirements_{},
      primitive_generation_seed_(INVALID_GENERATION),
      fence_generation_(INVALID_GENERATION),
      next_primitive_retirement_id_(1U),
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

    InitializePrimitiveSlots();
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
        true,
        true,
        true,
        MAX_RHI_BUFFERS,
        MAX_RHI_TEXTURES,
        MAX_RHI_SAMPLERS,
        MAX_RHI_SHADER_MODULES,
        MAX_RHI_PIPELINES,
        MAX_RHI_BUFFER_BYTES,
        MAX_RHI_SHADER_BYTECODE_BYTES};
    snapshot_ = RhiDeviceSnapshot{};
    snapshot_.color_target_capacity = 1U;
    snapshot_.color_target_count = 1U;
    snapshot_.created_target_count = 1U;
    snapshot_.swapchain.extent = desc.swapchain.extent;
    snapshot_.swapchain.color_format = desc.swapchain.color_format;
    snapshot_.swapchain.color_target = swapchain_target_;
    snapshot_.swapchain.valid = true;
    snapshot_.resources.buffer_capacity = MAX_RHI_BUFFERS;
    snapshot_.resources.texture_capacity = MAX_RHI_TEXTURES;
    snapshot_.resources.sampler_capacity = MAX_RHI_SAMPLERS;
    snapshot_.resources.shader_module_capacity = MAX_RHI_SHADER_MODULES;
    snapshot_.resources.pipeline_capacity = MAX_RHI_PIPELINES;
    snapshot_.resources.primitive_retirement.capacity = MAX_RHI_PRIMITIVE_RETIREMENTS;
    snapshot_.resources.primitive_retirement.next_retirement_id = next_primitive_retirement_id_;
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

RhiStatus D3D11RhiDevice::ResizeSwapchain(
    const RhiSwapchainResizeRequest &request,
    RhiSwapchainResizeResult &out_result) {
    out_result = RhiSwapchainResizeResult{};
    if (!initialized_) {
        out_result.status = RhiStatus::InvalidLifecycle;
        out_result.snapshot = snapshot_.swapchain;
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    out_result.previous_extent = snapshot_.swapchain.extent;
    out_result.previous_color_target = swapchain_target_;
    out_result.snapshot = snapshot_.swapchain;
    if (!IsSwapchainResizeRequestValid(request)) {
        ++snapshot_.swapchain.rejected_resize_count;
        out_result.status = RhiStatus::InvalidDescriptor;
        out_result.snapshot = snapshot_.swapchain;
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    if (request.extent.width == swapchain_desc_.extent.width &&
        request.extent.height == swapchain_desc_.extent.height) {
        out_result.status = RhiStatus::Success;
        out_result.snapshot = snapshot_.swapchain;
        return RhiStatus::Success;
    }

    context_->OMSetRenderTargets(0U, nullptr, nullptr);
    context_->Flush();
    ReleaseBackbufferObjects();

    const UINT width = static_cast<UINT>(request.extent.width);
    const UINT height = static_cast<UINT>(request.extent.height);
    const HRESULT resize_result = swapchain_->ResizeBuffers(0U, width, height, DXGI_FORMAT_UNKNOWN, 0U);
    if (FAILED(resize_result)) {
        ++snapshot_.swapchain.rejected_resize_count;
        out_result.status = TranslateNativeFailure(resize_result);
        out_result.snapshot = snapshot_.swapchain;
        return RecordFailure(out_result.status);
    }

    swapchain_desc_.extent = request.extent;
    const RhiStatus backbuffer_status = CreateBackbufferObjects();
    if (backbuffer_status != RhiStatus::Success) {
        ++snapshot_.swapchain.rejected_resize_count;
        snapshot_.swapchain.valid = false;
        out_result.status = backbuffer_status;
        out_result.snapshot = snapshot_.swapchain;
        return RecordFailure(backbuffer_status);
    }

    const RhiStatus capture_status = CreateCaptureTexture();
    if (capture_status != RhiStatus::Success) {
        ++snapshot_.swapchain.rejected_resize_count;
        snapshot_.swapchain.valid = false;
        out_result.status = capture_status;
        out_result.snapshot = snapshot_.swapchain;
        return RecordFailure(capture_status);
    }

    ++swapchain_target_.generation;
    if (swapchain_target_.generation == INVALID_GENERATION) {
        ++swapchain_target_.generation;
    }

    snapshot_.swapchain.extent = request.extent;
    snapshot_.swapchain.color_target = swapchain_target_;
    snapshot_.swapchain.valid = true;
    snapshot_.swapchain.presented = false;
    ++snapshot_.swapchain.resize_count;
    submitted_ = false;
    presented_ = false;

    out_result.status = RhiStatus::Success;
    out_result.resized = true;
    out_result.snapshot = snapshot_.swapchain;
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

RhiStatus D3D11RhiDevice::RecordBindPipeline(RhiCommandList &command_list, RhiPipelineHandle handle) {
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

RhiStatus D3D11RhiDevice::RecordBindVertexBuffer(RhiCommandList &command_list, const RhiVertexBufferView &view) {
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

RhiStatus D3D11RhiDevice::RecordBindIndexBuffer(RhiCommandList &command_list, const RhiIndexBufferView &view) {
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

RhiStatus D3D11RhiDevice::RecordBindSampledTexture(
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

RhiStatus D3D11RhiDevice::RecordBindSampler(RhiCommandList &command_list, const RhiSamplerBinding &binding) {
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

RhiStatus D3D11RhiDevice::RecordDraw(RhiCommandList &command_list, const RhiDrawDesc &desc) {
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

RhiStatus D3D11RhiDevice::RecordDrawIndexed(RhiCommandList &command_list, const RhiDrawIndexedDesc &desc) {
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
    std::uint32_t last_draw_vertex_count = 0U;
    std::uint32_t last_indexed_draw_index_count = 0U;
    std::uint32_t last_bound_sampled_texture_slot = 0U;
    std::uint32_t last_bound_sampler_slot = 0U;
    for (std::size_t index = 0U; index < command_list.CommandCount(); ++index) {
        const RhiCommandRecord &command = command_list.CommandAt(index);
        if ((command.type == RhiCommandType::BeginFrame ||
            command.type == RhiCommandType::ClearColor ||
            command.type == RhiCommandType::EndFrame) &&
            !IsSwapchainTarget(command.target)) {
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

            const D3D11PipelineSlot &pipeline = pipelines_[bound_pipeline.slot];
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

            const D3D11PipelineSlot &pipeline = pipelines_[bound_pipeline.slot];
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
            const float color[4] = {
                static_cast<float>(command.color.r) / 255.0F,
                static_cast<float>(command.color.g) / 255.0F,
                static_cast<float>(command.color.b) / 255.0F,
                static_cast<float>(command.color.a) / 255.0F};
            context_->ClearRenderTargetView(render_target_view_, color);
            continue;
        }

        if (command.type == RhiCommandType::BindPipeline) {
            bound_pipeline = command.pipeline;
            continue;
        }

        if (command.type == RhiCommandType::BindVertexBuffer) {
            bound_vertex_buffer = command.vertex_buffer;
            continue;
        }

        if (command.type == RhiCommandType::BindIndexBuffer) {
            bound_index_buffer = command.index_buffer;
            continue;
        }

        if (command.type == RhiCommandType::BindSampledTexture) {
            D3D11TextureSlot &texture = textures_[command.sampled_texture.texture.slot];
            ID3D11ShaderResourceView *shader_resource_view = texture.shader_resource_view;
            context_->PSSetShaderResources(command.sampled_texture.slot, 1U, &shader_resource_view);
            continue;
        }

        if (command.type == RhiCommandType::BindSampler) {
            D3D11SamplerSlot &sampler = samplers_[command.sampler.sampler.slot];
            context_->PSSetSamplers(command.sampler.slot, 1U, &sampler.sampler);
            continue;
        }

        if (command.type == RhiCommandType::Draw) {
            const D3D11PipelineSlot &pipeline = pipelines_[bound_pipeline.slot];
            const D3D11ShaderModuleSlot &vertex_shader = shader_modules_[pipeline.desc.vertex_shader.slot];
            const D3D11ShaderModuleSlot &pixel_shader = shader_modules_[pipeline.desc.pixel_shader.slot];
            D3D11BufferSlot &vertex_buffer = buffers_[bound_vertex_buffer.buffer.slot];
            ID3D11RenderTargetView *target_view = render_target_view_;
            D3D11_VIEWPORT viewport{};
            viewport.Width = static_cast<float>(swapchain_desc_.extent.width);
            viewport.Height = static_cast<float>(swapchain_desc_.extent.height);
            viewport.MinDepth = 0.0F;
            viewport.MaxDepth = 1.0F;
            UINT stride = static_cast<UINT>(bound_vertex_buffer.stride_bytes);
            UINT offset = static_cast<UINT>(bound_vertex_buffer.offset_bytes);
            const UINT vertex_count = static_cast<UINT>(command.draw.vertex_count);
            const UINT first_vertex = static_cast<UINT>(command.draw.first_vertex);
            context_->OMSetRenderTargets(1U, &target_view, nullptr);
            context_->RSSetViewports(1U, &viewport);
            context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context_->IASetInputLayout(pipeline.input_layout);
            context_->IASetVertexBuffers(0U, 1U, &vertex_buffer.buffer, &stride, &offset);
            context_->VSSetShader(vertex_shader.vertex_shader, nullptr, 0U);
            context_->PSSetShader(pixel_shader.pixel_shader, nullptr, 0U);
            context_->Draw(vertex_count, first_vertex);
            continue;
        }

        if (command.type == RhiCommandType::DrawIndexed) {
            const D3D11PipelineSlot &pipeline = pipelines_[bound_pipeline.slot];
            const D3D11ShaderModuleSlot &vertex_shader = shader_modules_[pipeline.desc.vertex_shader.slot];
            const D3D11ShaderModuleSlot &pixel_shader = shader_modules_[pipeline.desc.pixel_shader.slot];
            D3D11BufferSlot &vertex_buffer = buffers_[bound_vertex_buffer.buffer.slot];
            D3D11BufferSlot &index_buffer = buffers_[bound_index_buffer.buffer.slot];
            ID3D11RenderTargetView *target_view = render_target_view_;
            D3D11_VIEWPORT viewport{};
            viewport.Width = static_cast<float>(swapchain_desc_.extent.width);
            viewport.Height = static_cast<float>(swapchain_desc_.extent.height);
            viewport.MinDepth = 0.0F;
            viewport.MaxDepth = 1.0F;
            UINT stride = static_cast<UINT>(bound_vertex_buffer.stride_bytes);
            UINT vertex_offset = static_cast<UINT>(bound_vertex_buffer.offset_bytes);
            UINT index_buffer_offset = static_cast<UINT>(bound_index_buffer.offset_bytes);
            const UINT index_count = static_cast<UINT>(command.draw_indexed.index_count);
            const UINT first_index = static_cast<UINT>(command.draw_indexed.first_index);
            const INT base_vertex = static_cast<INT>(command.draw_indexed.vertex_offset);
            context_->OMSetRenderTargets(1U, &target_view, nullptr);
            context_->RSSetViewports(1U, &viewport);
            context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context_->IASetInputLayout(pipeline.input_layout);
            context_->IASetVertexBuffers(0U, 1U, &vertex_buffer.buffer, &stride, &vertex_offset);
            context_->IASetIndexBuffer(index_buffer.buffer, NativeIndexFormat(bound_index_buffer.format), index_buffer_offset);
            context_->VSSetShader(vertex_shader.vertex_shader, nullptr, 0U);
            context_->PSSetShader(pixel_shader.pixel_shader, nullptr, 0U);
            context_->DrawIndexed(index_count, first_index, base_vertex);
            continue;
        }
    }

    submitted_ = true;
    presented_ = false;
    ++snapshot_.submit_count;
    snapshot_.submitted_draw_count += submitted_draw_count;
    snapshot_.submitted_indexed_draw_count += submitted_indexed_draw_count;
    snapshot_.submitted_sampled_texture_bind_count += submitted_sampled_texture_bind_count;
    snapshot_.submitted_sampler_bind_count += submitted_sampler_bind_count;
    snapshot_.last_draw_vertex_count = last_draw_vertex_count;
    snapshot_.last_indexed_draw_index_count = last_indexed_draw_index_count;
    if (submitted_sampled_texture_bind_count > 0U) {
        snapshot_.last_bound_sampled_texture_slot = last_bound_sampled_texture_slot;
    }

    if (submitted_sampler_bind_count > 0U) {
        snapshot_.last_bound_sampler_slot = last_bound_sampler_slot;
    }

    if (submitted_indexed_draw_count > 0U) {
        snapshot_.last_bound_index_buffer_offset_bytes = bound_index_buffer.offset_bytes;
        snapshot_.last_bound_index_buffer_size_bytes = bound_index_buffer.size_bytes;
    }

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
        snapshot_.last_capture_extent = RhiExtent2D{};
        return RhiCaptureResult{RhiStatus::CapacityExceeded, 0U};
    }

    D3D11_MAPPED_SUBRESOURCE mapped_resource{};
    const HRESULT map_result = context_->Map(capture_texture_, 0U, D3D11_MAP_READ, 0U, &mapped_resource);
    if (FAILED(map_result)) {
        const RhiStatus status = TranslateNativeFailure(map_result);
        RecordFailure(status);
        snapshot_.last_capture_bytes_written = 0U;
        snapshot_.last_capture_extent = RhiExtent2D{};
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
    snapshot_.last_capture_extent = swapchain_desc_.extent;
    return RhiCaptureResult{RhiStatus::Success, byte_count, swapchain_desc_.extent};
}

RhiStatus D3D11RhiDevice::CreateBuffer(
    const RhiBufferDesc &desc,
    std::span<const std::uint8_t> initial_bytes,
    RhiBufferHandle &out_handle) {
    out_handle = RhiBufferHandle{};
    if (!initialized_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (!IsBufferDescValid(desc, initial_bytes)) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    for (std::size_t index = 0U; index < buffers_.size(); ++index) {
        D3D11BufferSlot &slot = buffers_[index];
        if (slot.is_active) {
            continue;
        }

        D3D11_BUFFER_DESC native_desc{};
        native_desc.ByteWidth = static_cast<UINT>(desc.size_bytes);
        native_desc.Usage = D3D11_USAGE_DEFAULT;
        native_desc.BindFlags = NativeBindFlagsForBuffer(desc.usage);
        native_desc.CPUAccessFlags = 0U;
        native_desc.MiscFlags = 0U;
        native_desc.StructureByteStride = 0U;

        D3D11_SUBRESOURCE_DATA native_initial_data{};
        D3D11_SUBRESOURCE_DATA *initial_data = nullptr;
        if (!initial_bytes.empty()) {
            native_initial_data.pSysMem = initial_bytes.data();
            initial_data = &native_initial_data;
        }

        ID3D11Buffer *native_buffer = nullptr;
        const HRESULT create_result = device_->CreateBuffer(&native_desc, initial_data, &native_buffer);
        if (FAILED(create_result)) {
            return RecordFailure(TranslateNativeFailure(create_result));
        }

        if (slot.generation == INVALID_GENERATION) {
            slot.generation = 1U;
        }

        slot.buffer = native_buffer;
        slot.desc = desc;
        slot.is_active = true;
        out_handle = RhiBufferHandle{static_cast<std::uint32_t>(index), slot.generation};
        ++snapshot_.resources.buffer_count;
        ++snapshot_.resources.created_primitive_count;
        return RhiStatus::Success;
    }

    return RecordFailure(RhiStatus::CapacityExceeded);
}

RhiStatus D3D11RhiDevice::UpdateBuffer(
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

    D3D11BufferSlot &slot = buffers_[handle.slot];
    if (bytes.size() > slot.desc.size_bytes) {
        return RecordFailure(RhiStatus::CapacityExceeded);
    }

    context_->UpdateSubresource(slot.buffer, 0U, nullptr, bytes.data(), 0U, 0U);
    out_fence = SignalFence(bytes.size());
    ++snapshot_.resources.updated_primitive_count;
    return RhiStatus::Success;
}

RhiStatus D3D11RhiDevice::DestroyBuffer(RhiBufferHandle handle) {
    if (!IsBufferHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    RetireBufferSlot(handle.slot);
    return RhiStatus::Success;
}

RhiStatus D3D11RhiDevice::CreateTexture(
    const RhiTextureDesc &desc,
    std::span<const std::uint8_t> initial_bytes,
    RhiTextureHandle &out_handle) {
    out_handle = RhiTextureHandle{};
    if (!initialized_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (!IsTextureDescValid(desc, initial_bytes)) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    for (std::size_t index = 0U; index < textures_.size(); ++index) {
        D3D11TextureSlot &slot = textures_[index];
        if (slot.is_active) {
            continue;
        }

        D3D11_TEXTURE2D_DESC native_desc{};
        native_desc.Width = desc.extent.width;
        native_desc.Height = desc.extent.height;
        native_desc.MipLevels = 1U;
        native_desc.ArraySize = 1U;
        native_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        native_desc.SampleDesc.Count = 1U;
        native_desc.SampleDesc.Quality = 0U;
        native_desc.Usage = D3D11_USAGE_DEFAULT;
        native_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        native_desc.CPUAccessFlags = 0U;
        native_desc.MiscFlags = 0U;

        D3D11_SUBRESOURCE_DATA native_initial_data{};
        D3D11_SUBRESOURCE_DATA *initial_data = nullptr;
        if (!initial_bytes.empty()) {
            native_initial_data.pSysMem = initial_bytes.data();
            native_initial_data.SysMemPitch = static_cast<UINT>(desc.extent.width) * RGBA8_BYTES_PER_PIXEL;
            initial_data = &native_initial_data;
        }

        ID3D11Texture2D *native_texture = nullptr;
        const HRESULT create_result = device_->CreateTexture2D(&native_desc, initial_data, &native_texture);
        if (FAILED(create_result)) {
            return RecordFailure(TranslateNativeFailure(create_result));
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC native_view_desc{};
        native_view_desc.Format = native_desc.Format;
        native_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        native_view_desc.Texture2D.MostDetailedMip = 0U;
        native_view_desc.Texture2D.MipLevels = 1U;
        ID3D11ShaderResourceView *native_view = nullptr;
        const HRESULT view_result = device_->CreateShaderResourceView(native_texture, &native_view_desc, &native_view);
        if (FAILED(view_result)) {
            native_texture->Release();
            native_texture = nullptr;
            return RecordFailure(TranslateNativeFailure(view_result));
        }

        if (slot.generation == INVALID_GENERATION) {
            slot.generation = 1U;
        }

        slot.texture = native_texture;
        slot.shader_resource_view = native_view;
        slot.desc = desc;
        slot.is_active = true;
        out_handle = RhiTextureHandle{static_cast<std::uint32_t>(index), slot.generation};
        ++snapshot_.resources.texture_count;
        ++snapshot_.resources.created_primitive_count;
        return RhiStatus::Success;
    }

    return RecordFailure(RhiStatus::CapacityExceeded);
}

RhiStatus D3D11RhiDevice::UpdateTexture(
    RhiTextureHandle handle,
    std::span<const std::uint8_t> bytes,
    RhiFenceHandle &out_fence) {
    out_fence = RhiFenceHandle{};
    if (!IsTextureHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    D3D11TextureSlot &slot = textures_[handle.slot];
    if (bytes.size() != TextureByteCount(slot.desc)) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    const UINT row_bytes = static_cast<UINT>(slot.desc.extent.width) * RGBA8_BYTES_PER_PIXEL;
    context_->UpdateSubresource(slot.texture, 0U, nullptr, bytes.data(), row_bytes, 0U);
    out_fence = SignalFence(bytes.size());
    ++snapshot_.resources.updated_primitive_count;
    return RhiStatus::Success;
}

RhiStatus D3D11RhiDevice::DestroyTexture(RhiTextureHandle handle) {
    if (!IsTextureHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    RetireTextureSlot(handle.slot);
    return RhiStatus::Success;
}

RhiStatus D3D11RhiDevice::CreateSampler(const RhiSamplerDesc &desc, RhiSamplerHandle &out_handle) {
    out_handle = RhiSamplerHandle{};
    if (!initialized_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    for (std::size_t index = 0U; index < samplers_.size(); ++index) {
        D3D11SamplerSlot &slot = samplers_[index];
        if (slot.is_active) {
            continue;
        }

        D3D11_SAMPLER_DESC native_desc{};
        native_desc.Filter = desc.linear_filter ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;
        native_desc.AddressU = desc.clamp_to_edge ? D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP;
        native_desc.AddressV = native_desc.AddressU;
        native_desc.AddressW = native_desc.AddressU;
        native_desc.MipLODBias = 0.0F;
        native_desc.MaxAnisotropy = 1U;
        native_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        native_desc.MinLOD = 0.0F;
        native_desc.MaxLOD = D3D11_FLOAT32_MAX;

        ID3D11SamplerState *native_sampler = nullptr;
        const HRESULT create_result = device_->CreateSamplerState(&native_desc, &native_sampler);
        if (FAILED(create_result)) {
            return RecordFailure(TranslateNativeFailure(create_result));
        }

        if (slot.generation == INVALID_GENERATION) {
            slot.generation = 1U;
        }

        slot.sampler = native_sampler;
        slot.desc = desc;
        slot.is_active = true;
        out_handle = RhiSamplerHandle{static_cast<std::uint32_t>(index), slot.generation};
        ++snapshot_.resources.sampler_count;
        ++snapshot_.resources.created_primitive_count;
        return RhiStatus::Success;
    }

    return RecordFailure(RhiStatus::CapacityExceeded);
}

RhiStatus D3D11RhiDevice::DestroySampler(RhiSamplerHandle handle) {
    if (!IsSamplerHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    RetireSamplerSlot(handle.slot);
    return RhiStatus::Success;
}

RhiStatus D3D11RhiDevice::CreateShaderModule(const RhiShaderModuleDesc &desc, RhiShaderModuleHandle &out_handle) {
    out_handle = RhiShaderModuleHandle{};
    if (!initialized_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (!IsShaderModuleDescValid(desc)) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    for (std::size_t index = 0U; index < shader_modules_.size(); ++index) {
        D3D11ShaderModuleSlot &slot = shader_modules_[index];
        if (slot.is_active) {
            continue;
        }

        ID3D11VertexShader *native_vertex_shader = nullptr;
        ID3D11PixelShader *native_pixel_shader = nullptr;
        if (desc.stage == RhiShaderStage::Vertex) {
            const HRESULT create_result = device_->CreateVertexShader(
                desc.bytecode.data(),
                desc.bytecode.size(),
                nullptr,
                &native_vertex_shader);
            if (FAILED(create_result)) {
                return RecordFailure(TranslateNativeFailure(create_result));
            }
        }

        if (desc.stage == RhiShaderStage::Pixel) {
            const HRESULT create_result = device_->CreatePixelShader(
                desc.bytecode.data(),
                desc.bytecode.size(),
                nullptr,
                &native_pixel_shader);
            if (FAILED(create_result)) {
                return RecordFailure(TranslateNativeFailure(create_result));
            }
        }

        if (slot.generation == INVALID_GENERATION) {
            slot.generation = 1U;
        }

        slot.vertex_shader = native_vertex_shader;
        slot.pixel_shader = native_pixel_shader;
        slot.stage = desc.stage;
        slot.bytecode_size = desc.bytecode.size();
        std::copy(desc.bytecode.begin(), desc.bytecode.end(), slot.bytecode.begin());
        slot.is_active = true;
        out_handle = RhiShaderModuleHandle{static_cast<std::uint32_t>(index), slot.generation};
        ++snapshot_.resources.shader_module_count;
        ++snapshot_.resources.created_primitive_count;
        return RhiStatus::Success;
    }

    return RecordFailure(RhiStatus::CapacityExceeded);
}

RhiStatus D3D11RhiDevice::DestroyShaderModule(RhiShaderModuleHandle handle) {
    if (!IsShaderModuleHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    RetireShaderModuleSlot(handle.slot);
    return RhiStatus::Success;
}

RhiStatus D3D11RhiDevice::CreatePipeline(const RhiPipelineDesc &desc, RhiPipelineHandle &out_handle) {
    out_handle = RhiPipelineHandle{};
    if (!initialized_) {
        return RecordFailure(RhiStatus::InvalidLifecycle);
    }

    if (!IsPipelineDescValid(desc)) {
        return RecordFailure(RhiStatus::InvalidDescriptor);
    }

    for (std::size_t index = 0U; index < pipelines_.size(); ++index) {
        D3D11PipelineSlot &slot = pipelines_[index];
        if (slot.is_active) {
            continue;
        }

        if (slot.generation == INVALID_GENERATION) {
            slot.generation = 1U;
        }

        ID3D11InputLayout *native_input_layout = nullptr;
        if (desc.input_layout.element_count != 0U) {
            D3D11_INPUT_ELEMENT_DESC native_elements[MAX_RHI_INPUT_ELEMENTS]{};
            for (std::size_t element_index = 0U; element_index < desc.input_layout.element_count; ++element_index) {
                const RhiInputElementDesc &element = desc.input_layout.elements[element_index];
                D3D11_INPUT_ELEMENT_DESC &native_element = native_elements[element_index];
                native_element.SemanticName = NativeInputElementSemanticName(element.semantic);
                native_element.SemanticIndex = 0U;
                native_element.Format = NativeInputElementFormat(element.format);
                native_element.InputSlot = 0U;
                native_element.AlignedByteOffset = static_cast<UINT>(element.offset_bytes);
                native_element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                native_element.InstanceDataStepRate = 0U;
            }

            const D3D11ShaderModuleSlot &vertex_shader = shader_modules_[desc.vertex_shader.slot];
            const HRESULT create_result = device_->CreateInputLayout(
                native_elements,
                static_cast<UINT>(desc.input_layout.element_count),
                vertex_shader.bytecode.data(),
                vertex_shader.bytecode_size,
                &native_input_layout);
            if (FAILED(create_result)) {
                return RecordFailure(TranslateNativeFailure(create_result));
            }
        }

        slot.input_layout = native_input_layout;
        slot.desc = desc;
        slot.is_active = true;
        out_handle = RhiPipelineHandle{static_cast<std::uint32_t>(index), slot.generation};
        ++snapshot_.resources.pipeline_count;
        ++snapshot_.resources.created_primitive_count;
        return RhiStatus::Success;
    }

    return RecordFailure(RhiStatus::CapacityExceeded);
}

RhiStatus D3D11RhiDevice::DestroyPipeline(RhiPipelineHandle handle) {
    if (!IsPipelineHandleValid(handle)) {
        return RecordFailure(RhiStatus::InvalidHandle);
    }

    RetirePipelineSlot(handle.slot);
    return RhiStatus::Success;
}

RhiStatus D3D11RhiDevice::RequestPrimitiveRetirement(
    const RhiPrimitiveRetirementRequest &request,
    RhiPrimitiveRetirementRecord &out_record) {
    out_record = RhiPrimitiveRetirementRecord{};
    out_record.request_id = request.request_id;
    out_record.primitive_kind = request.primitive_kind;
    out_record.primitive_slot = request.primitive_slot;
    out_record.primitive_generation = request.primitive_generation;
    out_record.wait_fence = request.wait_fence;

    if (!initialized_) {
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

RhiStatus D3D11RhiDevice::QueryPrimitiveRetirement(
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

RhiStatus D3D11RhiDevice::DrainPrimitiveRetirements(
    const RhiPrimitiveRetirementDrainRequest &request,
    RhiPrimitiveRetirementDrainResult &out_result) {
    out_result = RhiPrimitiveRetirementDrainResult{};
    if (!initialized_) {
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

RhiCapabilities D3D11RhiDevice::Capabilities() const {
    return capabilities_;
}

RhiDeviceSnapshot D3D11RhiDevice::Snapshot() const {
    return snapshot_;
}

void D3D11RhiDevice::ReleaseResources() {
    ReleasePrimitiveResources();
    ReleaseBackbufferObjects();

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

void D3D11RhiDevice::ReleasePrimitiveResources() {
    for (D3D11PipelineSlot &pipeline : pipelines_) {
        if (pipeline.input_layout != nullptr) {
            pipeline.input_layout->Release();
            pipeline.input_layout = nullptr;
        }

        pipeline = D3D11PipelineSlot{};
    }

    for (D3D11ShaderModuleSlot &shader_module : shader_modules_) {
        if (shader_module.vertex_shader != nullptr) {
            shader_module.vertex_shader->Release();
            shader_module.vertex_shader = nullptr;
        }

        if (shader_module.pixel_shader != nullptr) {
            shader_module.pixel_shader->Release();
            shader_module.pixel_shader = nullptr;
        }

        shader_module = D3D11ShaderModuleSlot{};
    }

    for (D3D11SamplerSlot &sampler : samplers_) {
        if (sampler.sampler != nullptr) {
            sampler.sampler->Release();
            sampler.sampler = nullptr;
        }

        sampler = D3D11SamplerSlot{};
    }

    for (D3D11TextureSlot &texture : textures_) {
        if (texture.shader_resource_view != nullptr) {
            texture.shader_resource_view->Release();
            texture.shader_resource_view = nullptr;
        }

        if (texture.texture != nullptr) {
            texture.texture->Release();
            texture.texture = nullptr;
        }

        texture = D3D11TextureSlot{};
    }

    for (D3D11BufferSlot &buffer : buffers_) {
        if (buffer.buffer != nullptr) {
            buffer.buffer->Release();
            buffer.buffer = nullptr;
        }

        buffer = D3D11BufferSlot{};
    }
}

void D3D11RhiDevice::InitializePrimitiveSlots() {
    ++primitive_generation_seed_;
    if (primitive_generation_seed_ == INVALID_GENERATION) {
        ++primitive_generation_seed_;
    }

    for (D3D11BufferSlot &buffer : buffers_) {
        buffer.generation = primitive_generation_seed_;
    }

    for (D3D11TextureSlot &texture : textures_) {
        texture.generation = primitive_generation_seed_;
    }

    for (D3D11SamplerSlot &sampler : samplers_) {
        sampler.generation = primitive_generation_seed_;
    }

    for (D3D11ShaderModuleSlot &shader_module : shader_modules_) {
        shader_module.generation = primitive_generation_seed_;
    }

    for (D3D11PipelineSlot &pipeline : pipelines_) {
        pipeline.generation = primitive_generation_seed_;
    }

    primitive_retirements_.fill(RhiPrimitiveRetirementRecord{});
    next_primitive_retirement_id_ = 1U;
}

RhiStatus D3D11RhiDevice::RecordFailure(RhiStatus status) {
    ++snapshot_.failed_operation_count;
    return status;
}

RhiStatus D3D11RhiDevice::RecordIndexedDrawFailure(RhiStatus status) {
    ++snapshot_.rejected_indexed_draw_count;
    return RecordFailure(status);
}

RhiStatus D3D11RhiDevice::RecordSampledTextureBindFailure(RhiStatus status) {
    ++snapshot_.rejected_sampled_texture_bind_count;
    return RecordFailure(status);
}

RhiStatus D3D11RhiDevice::RecordSamplerBindFailure(RhiStatus status) {
    ++snapshot_.rejected_sampler_bind_count;
    return RecordFailure(status);
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
        backbuffer_->Release();
        backbuffer_ = nullptr;
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

void D3D11RhiDevice::ReleaseBackbufferObjects() {
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

bool D3D11RhiDevice::IsSwapchainResizeRequestValid(const RhiSwapchainResizeRequest &request) const {
    RhiSwapchainDesc desc = swapchain_desc_;
    desc.extent = request.extent;
    return IsSwapchainDescValid(desc);
}

bool D3D11RhiDevice::IsBufferHandleValid(RhiBufferHandle handle) const {
    if (!initialized_) {
        return false;
    }

    if (handle.generation == INVALID_GENERATION) {
        return false;
    }

    if (handle.slot >= buffers_.size()) {
        return false;
    }

    const D3D11BufferSlot &slot = buffers_[handle.slot];
    if (!slot.is_active) {
        return false;
    }

    return slot.generation == handle.generation;
}

bool D3D11RhiDevice::IsTextureHandleValid(RhiTextureHandle handle) const {
    if (!initialized_) {
        return false;
    }

    if (handle.generation == INVALID_GENERATION) {
        return false;
    }

    if (handle.slot >= textures_.size()) {
        return false;
    }

    const D3D11TextureSlot &slot = textures_[handle.slot];
    if (!slot.is_active) {
        return false;
    }

    return slot.generation == handle.generation;
}

bool D3D11RhiDevice::IsSamplerHandleValid(RhiSamplerHandle handle) const {
    if (!initialized_) {
        return false;
    }

    if (handle.generation == INVALID_GENERATION) {
        return false;
    }

    if (handle.slot >= samplers_.size()) {
        return false;
    }

    const D3D11SamplerSlot &slot = samplers_[handle.slot];
    if (!slot.is_active) {
        return false;
    }

    return slot.generation == handle.generation;
}

bool D3D11RhiDevice::IsShaderModuleHandleValid(RhiShaderModuleHandle handle) const {
    if (!initialized_) {
        return false;
    }

    if (handle.generation == INVALID_GENERATION) {
        return false;
    }

    if (handle.slot >= shader_modules_.size()) {
        return false;
    }

    const D3D11ShaderModuleSlot &slot = shader_modules_[handle.slot];
    if (!slot.is_active) {
        return false;
    }

    return slot.generation == handle.generation;
}

bool D3D11RhiDevice::IsPipelineHandleValid(RhiPipelineHandle handle) const {
    if (!initialized_) {
        return false;
    }

    if (handle.generation == INVALID_GENERATION) {
        return false;
    }

    if (handle.slot >= pipelines_.size()) {
        return false;
    }

    const D3D11PipelineSlot &slot = pipelines_[handle.slot];
    if (!slot.is_active) {
        return false;
    }

    return slot.generation == handle.generation;
}

bool D3D11RhiDevice::IsRetirementRequestValid(const RhiPrimitiveRetirementRequest &request) const {
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

bool D3D11RhiDevice::IsRetirementWrongKind(const RhiPrimitiveRetirementRequest &request) const {
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

bool D3D11RhiDevice::IsRetirementDuplicate(const RhiPrimitiveRetirementRequest &request) const {
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

bool D3D11RhiDevice::IsRetirementFenceReady(RhiFenceHandle fence) const {
    if (fence.generation == INVALID_GENERATION) {
        return true;
    }

    if (fence.slot != 0U) {
        return false;
    }

    return fence.generation <= fence_generation_;
}

void D3D11RhiDevice::RetireBufferSlot(std::uint32_t slot_index) {
    D3D11BufferSlot &slot = buffers_[slot_index];
    if (slot.buffer != nullptr) {
        slot.buffer->Release();
        slot.buffer = nullptr;
    }

    slot.desc = RhiBufferDesc{};
    slot.is_active = false;
    ++slot.generation;
    --snapshot_.resources.buffer_count;
    ++snapshot_.resources.destroyed_primitive_count;
}

void D3D11RhiDevice::RetireTextureSlot(std::uint32_t slot_index) {
    D3D11TextureSlot &slot = textures_[slot_index];
    if (slot.shader_resource_view != nullptr) {
        slot.shader_resource_view->Release();
        slot.shader_resource_view = nullptr;
    }

    if (slot.texture != nullptr) {
        slot.texture->Release();
        slot.texture = nullptr;
    }

    slot.desc = RhiTextureDesc{};
    slot.is_active = false;
    ++slot.generation;
    --snapshot_.resources.texture_count;
    ++snapshot_.resources.destroyed_primitive_count;
}

void D3D11RhiDevice::RetireSamplerSlot(std::uint32_t slot_index) {
    D3D11SamplerSlot &slot = samplers_[slot_index];
    if (slot.sampler != nullptr) {
        slot.sampler->Release();
        slot.sampler = nullptr;
    }

    slot.desc = RhiSamplerDesc{};
    slot.is_active = false;
    ++slot.generation;
    --snapshot_.resources.sampler_count;
    ++snapshot_.resources.destroyed_primitive_count;
}

void D3D11RhiDevice::RetireShaderModuleSlot(std::uint32_t slot_index) {
    D3D11ShaderModuleSlot &slot = shader_modules_[slot_index];
    if (slot.vertex_shader != nullptr) {
        slot.vertex_shader->Release();
        slot.vertex_shader = nullptr;
    }

    if (slot.pixel_shader != nullptr) {
        slot.pixel_shader->Release();
        slot.pixel_shader = nullptr;
    }

    slot.stage = RhiShaderStage::Unsupported;
    slot.bytecode.fill(0U);
    slot.bytecode_size = 0U;
    slot.is_active = false;
    ++slot.generation;
    --snapshot_.resources.shader_module_count;
    ++snapshot_.resources.destroyed_primitive_count;
}

void D3D11RhiDevice::RetirePipelineSlot(std::uint32_t slot_index) {
    D3D11PipelineSlot &slot = pipelines_[slot_index];
    if (slot.input_layout != nullptr) {
        slot.input_layout->Release();
        slot.input_layout = nullptr;
    }

    slot.desc = RhiPipelineDesc{};
    slot.is_active = false;
    ++slot.generation;
    --snapshot_.resources.pipeline_count;
    ++snapshot_.resources.destroyed_primitive_count;
}

void D3D11RhiDevice::DrainRetirementRecord(RhiPrimitiveRetirementRecord &record) {
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

bool D3D11RhiDevice::IsVertexBufferViewValid(const RhiVertexBufferView &view) const {
    if (!IsBufferHandleValid(view.buffer)) {
        return false;
    }

    const D3D11BufferSlot &slot = buffers_[view.buffer.slot];
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

bool D3D11RhiDevice::IsIndexBufferViewValid(const RhiIndexBufferView &view) const {
    if (!IsBufferHandleValid(view.buffer)) {
        return false;
    }

    const D3D11BufferSlot &slot = buffers_[view.buffer.slot];
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

bool D3D11RhiDevice::IsSampledTextureBindingValid(const RhiSampledTextureBinding &binding) const {
    if (binding.slot >= MAX_RHI_SAMPLED_TEXTURE_SLOTS) {
        return false;
    }

    return IsTextureHandleValid(binding.texture);
}

bool D3D11RhiDevice::IsSamplerBindingValid(const RhiSamplerBinding &binding) const {
    if (binding.slot >= MAX_RHI_SAMPLER_SLOTS) {
        return false;
    }

    return IsSamplerHandleValid(binding.sampler);
}

bool D3D11RhiDevice::IsDrawDescValid(const RhiDrawDesc &desc) const {
    if (desc.topology != RhiPrimitiveTopology::TriangleList) {
        return false;
    }

    if (desc.vertex_count == 0U) {
        return false;
    }

    return true;
}

bool D3D11RhiDevice::IsDrawIndexedDescValid(const RhiDrawIndexedDesc &desc) const {
    if (desc.topology != RhiPrimitiveTopology::TriangleList) {
        return false;
    }

    if (desc.index_count == 0U) {
        return false;
    }

    return true;
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

bool D3D11RhiDevice::IsBufferDescValid(const RhiBufferDesc &desc, std::span<const std::uint8_t> initial_bytes) const {
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

bool D3D11RhiDevice::IsTextureDescValid(const RhiTextureDesc &desc, std::span<const std::uint8_t> initial_bytes) const {
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

bool D3D11RhiDevice::IsShaderModuleDescValid(const RhiShaderModuleDesc &desc) const {
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

bool D3D11RhiDevice::IsPipelineDescValid(const RhiPipelineDesc &desc) const {
    if (desc.input_layout.element_count != 0U && !IsInputLayoutDescValid(desc.input_layout)) {
        return false;
    }

    if (!IsShaderModuleHandleValid(desc.vertex_shader)) {
        return false;
    }

    if (!IsShaderModuleHandleValid(desc.pixel_shader)) {
        return false;
    }

    const D3D11ShaderModuleSlot &vertex_shader = shader_modules_[desc.vertex_shader.slot];
    if (vertex_shader.stage != RhiShaderStage::Vertex) {
        return false;
    }

    const D3D11ShaderModuleSlot &pixel_shader = shader_modules_[desc.pixel_shader.slot];
    if (pixel_shader.stage != RhiShaderStage::Pixel) {
        return false;
    }

    return true;
}

bool D3D11RhiDevice::IsInputLayoutDescValid(const RhiInputLayoutDesc &desc) const {
    if (desc.element_count == 0U) {
        return false;
    }

    if (desc.element_count > MAX_RHI_INPUT_ELEMENTS) {
        return false;
    }

    if (desc.stride_bytes == 0U) {
        return false;
    }

    if (desc.stride_bytes > MAX_RHI_BUFFER_BYTES) {
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

        if (NativeInputElementFormat(element.format) == DXGI_FORMAT_UNKNOWN) {
            return false;
        }
    }

    return has_position;
}

bool D3D11RhiDevice::IsDrawRangeValid(const RhiVertexBufferView &view, const RhiDrawDesc &desc) const {
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

bool D3D11RhiDevice::IsIndexedDrawRangeValid(const RhiIndexBufferView &view, const RhiDrawIndexedDesc &desc) const {
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

std::size_t D3D11RhiDevice::CaptureByteCount() const {
    return static_cast<std::size_t>(swapchain_desc_.extent.width) *
        static_cast<std::size_t>(swapchain_desc_.extent.height) * RGBA8_BYTES_PER_PIXEL;
}

std::size_t D3D11RhiDevice::TextureByteCount(const RhiTextureDesc &desc) const {
    return static_cast<std::size_t>(desc.extent.width) *
        static_cast<std::size_t>(desc.extent.height) * RGBA8_BYTES_PER_PIXEL;
}

std::size_t D3D11RhiDevice::IndexFormatByteCount(RhiIndexFormat format) const {
    if (format == RhiIndexFormat::Uint16) {
        return sizeof(std::uint16_t);
    }

    if (format == RhiIndexFormat::Uint32) {
        return sizeof(std::uint32_t);
    }

    return 0U;
}

UINT D3D11RhiDevice::NativeBindFlagsForBuffer(RhiBufferUsage usage) const {
    if (usage == RhiBufferUsage::Vertex) {
        return D3D11_BIND_VERTEX_BUFFER;
    }

    if (usage == RhiBufferUsage::Index) {
        return D3D11_BIND_INDEX_BUFFER;
    }

    if (usage == RhiBufferUsage::Constant) {
        return D3D11_BIND_CONSTANT_BUFFER;
    }

    return 0U;
}

DXGI_FORMAT D3D11RhiDevice::NativeIndexFormat(RhiIndexFormat format) const {
    if (format == RhiIndexFormat::Uint16) {
        return DXGI_FORMAT_R16_UINT;
    }

    if (format == RhiIndexFormat::Uint32) {
        return DXGI_FORMAT_R32_UINT;
    }

    return DXGI_FORMAT_UNKNOWN;
}

RhiFenceHandle D3D11RhiDevice::SignalFence(std::size_t byte_count) {
    ++fence_generation_;
    if (fence_generation_ == INVALID_GENERATION) {
        ++fence_generation_;
    }

    ++snapshot_.resources.signaled_fence_count;
    snapshot_.resources.last_update_bytes = byte_count;
    return RhiFenceHandle{0U, fence_generation_};
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
