// 模块：Tests RenderCore
// 文件：Tests/RenderCore/RenderDrawableFramePipelineTests.cpp

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>
#include <vector>

#include "YuEngine/RenderCore/MaterialBindingFixtureStatus.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipeline.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineDesc.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineRequest.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineSnapshot.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineStatus.h"
#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureStatus.h"
#include "YuEngine/RenderCore/RenderViewPacketStatus.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/RhiBackendKind.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiCapabilities.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiCommandList.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiDeviceSnapshot.h"
#include "YuEngine/Rhi/RhiDrawIndexedDesc.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"
#include "YuEngine/Rhi/RhiSamplerHandle.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"

using RenderDrawableFramePipeline = yuengine::rendercore::RenderDrawableFramePipeline;
using RenderDrawableFramePipelineDesc = yuengine::rendercore::RenderDrawableFramePipelineDesc;
using RenderDrawableFramePipelineRequest = yuengine::rendercore::RenderDrawableFramePipelineRequest;
using RenderDrawableFramePipelineSnapshot = yuengine::rendercore::RenderDrawableFramePipelineSnapshot;
using yuengine::rendercore::MaterialBindingFixtureStatus;
using yuengine::rendercore::RenderDrawableFramePipelineStatus;
using yuengine::rendercore::RenderFixturePassStatus;
using yuengine::rendercore::RenderFramePacketFixtureStatus;
using yuengine::rendercore::RenderViewPacketStatus;
using yuengine::rhi::IRhiDevice;
using yuengine::rhi::RhiBackendKind;
using yuengine::rhi::RhiBlendStateDesc;
using yuengine::rhi::RhiBufferDesc;
using yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiCapabilities;
using yuengine::rhi::RhiCaptureResult;
using yuengine::rhi::RhiColor;
using yuengine::rhi::RhiColorTargetDesc;
using yuengine::rhi::RhiCommandList;
using yuengine::rhi::RhiDeviceDesc;
using yuengine::rhi::RhiDeviceSnapshot;
using yuengine::rhi::RhiDrawDesc;
using yuengine::rhi::RhiDrawIndexedDesc;
using yuengine::rhi::RhiFenceHandle;
using yuengine::rhi::RhiFormat;
using yuengine::rhi::RhiIndexBufferView;
using yuengine::rhi::RhiIndexFormat;
using yuengine::rhi::RhiPipelineDesc;
using yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiPrimitiveRetirementDrainRequest;
using yuengine::rhi::RhiPrimitiveRetirementDrainResult;
using yuengine::rhi::RhiPrimitiveRetirementRecord;
using yuengine::rhi::RhiPrimitiveRetirementRequest;
using yuengine::rhi::RhiPrimitiveTopology;
using yuengine::rhi::RhiSampledTextureBinding;
using yuengine::rhi::RhiSamplerBinding;
using yuengine::rhi::RhiSamplerDesc;
using yuengine::rhi::RhiSamplerHandle;
using yuengine::rhi::RhiShaderModuleDesc;
using yuengine::rhi::RhiShaderModuleHandle;
using yuengine::rhi::RhiStatus;
using yuengine::rhi::RhiSwapchainResizeRequest;
using yuengine::rhi::RhiSwapchainResizeResult;
using yuengine::rhi::RhiTextureDesc;
using yuengine::rhi::RhiTextureHandle;
using yuengine::rhi::RhiVertexBufferView;
using yuengine::rhi::MAX_CAPTURE_FIXTURE_EXTENT;
using yuengine::rhi::MAX_COLOR_TARGET_EXTENT;
using yuengine::rhi::MAX_COMMANDS;
using yuengine::rhi::RGBA8_BYTES_PER_PIXEL;

namespace {
constexpr const char *TEST_EXECUTES = "RenderCore_DrawableFramePipeline_ExecutesMaterialFrameDrawCapture";
constexpr const char *TEST_INVALID_SWAPCHAIN = "RenderCore_DrawableFramePipeline_RejectsInvalidSwapchainWithoutMutation";
constexpr const char *TEST_INVALID_MATERIAL = "RenderCore_DrawableFramePipeline_RejectsInvalidMaterialWithoutRhiMutation";
constexpr const char *TEST_INVALID_DRAW = "RenderCore_DrawableFramePipeline_RejectsInvalidDrawThroughViewPacket";
constexpr const char *TEST_DUPLICATE_FRAME = "RenderCore_DrawableFramePipeline_RejectsDuplicateFrameThroughFramePacket";
constexpr const char *TEST_COMMAND_CAPACITY = "RenderCore_DrawableFramePipeline_PropagatesFixtureCommandCapacityFailure";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t FRAME_ID = 1U;
constexpr std::uint32_t NEXT_FRAME_ID = 2U;
constexpr std::uint32_t PASS_ID = 11U;
constexpr std::uint32_t NEXT_PASS_ID = 12U;
constexpr std::uint32_t MATERIAL_ID = 21U;
constexpr std::uint32_t NEXT_MATERIAL_ID = 22U;
constexpr std::uint16_t DEFAULT_EXTENT = 2U;
constexpr std::uint8_t SENTINEL_BYTE = 0xCCU;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool TextureHandlesMatch(RhiTextureHandle left, RhiTextureHandle right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

std::size_t CaptureByteCount(std::uint16_t width, std::uint16_t height) {
    return static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * RGBA8_BYTES_PER_PIXEL;
}

class FakeDrawableRhiDevice final : public IRhiDevice {
public:
    FakeDrawableRhiDevice() {
        ResetSwapchain();
    }

    RhiStatus Initialize(const RhiDeviceDesc &) override {
        ResetSwapchain();
        return RhiStatus::Success;
    }

    RhiStatus CreateColorTarget(const RhiColorTargetDesc &, RhiTextureHandle &out_handle) override {
        out_handle = RhiTextureHandle{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus GetSwapchainColorTarget(RhiTextureHandle &out_handle) const override {
        out_handle = RhiTextureHandle{};
        if (!snapshot_.swapchain.valid) {
            return RhiStatus::InvalidLifecycle;
        }

        out_handle = target_;
        return RhiStatus::Success;
    }

    RhiStatus ResizeSwapchain(
        const RhiSwapchainResizeRequest &,
        RhiSwapchainResizeResult &out_result) override {
        out_result = RhiSwapchainResizeResult{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus DestroyTarget(RhiTextureHandle) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus RecordClear(RhiCommandList &command_list, RhiTextureHandle handle, RhiColor color) override {
        if (!TextureHandlesMatch(handle, target_)) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidHandle;
        }

        const RhiStatus status = command_list.RecordClear(handle, color);
        if (status != RhiStatus::Success) {
            ++snapshot_.failed_operation_count;
            return status;
        }

        last_clear_color_ = color;
        ++snapshot_.recorded_command_count;
        return RhiStatus::Success;
    }

    RhiStatus RecordBindPipeline(RhiCommandList &command_list, RhiPipelineHandle handle) override {
        if (handle.generation != pipeline_.generation) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidHandle;
        }

        const RhiStatus status = command_list.RecordBindPipeline(handle);
        if (status != RhiStatus::Success) {
            ++snapshot_.failed_operation_count;
            return status;
        }

        ++snapshot_.recorded_command_count;
        return RhiStatus::Success;
    }

    RhiStatus RecordBindVertexBuffer(RhiCommandList &command_list, const RhiVertexBufferView &view) override {
        if (view.buffer.generation != vertex_buffer_.generation) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidHandle;
        }

        const RhiStatus status = command_list.RecordBindVertexBuffer(view);
        if (status != RhiStatus::Success) {
            ++snapshot_.failed_operation_count;
            return status;
        }

        ++snapshot_.recorded_command_count;
        return RhiStatus::Success;
    }

    RhiStatus RecordBindIndexBuffer(RhiCommandList &command_list, const RhiIndexBufferView &view) override {
        if (view.buffer.generation != index_buffer_.generation) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidHandle;
        }

        const RhiStatus status = command_list.RecordBindIndexBuffer(view);
        if (status != RhiStatus::Success) {
            ++snapshot_.failed_operation_count;
            return status;
        }

        ++snapshot_.recorded_command_count;
        return RhiStatus::Success;
    }

    RhiStatus RecordBindSampledTexture(
        RhiCommandList &command_list,
        const RhiSampledTextureBinding &binding) override {
        if (binding.texture.generation != texture_.generation) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidHandle;
        }

        const RhiStatus status = command_list.RecordBindSampledTexture(binding);
        if (status != RhiStatus::Success) {
            ++snapshot_.failed_operation_count;
            return status;
        }

        ++snapshot_.recorded_command_count;
        return RhiStatus::Success;
    }

    RhiStatus RecordBindSampler(RhiCommandList &command_list, const RhiSamplerBinding &binding) override {
        if (binding.sampler.generation != sampler_.generation) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidHandle;
        }

        const RhiStatus status = command_list.RecordBindSampler(binding);
        if (status != RhiStatus::Success) {
            ++snapshot_.failed_operation_count;
            return status;
        }

        ++snapshot_.recorded_command_count;
        return RhiStatus::Success;
    }

    RhiStatus RecordBindBlendState(RhiCommandList &, const RhiBlendStateDesc &) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus RecordDraw(RhiCommandList &, const RhiDrawDesc &) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus RecordDrawIndexed(RhiCommandList &command_list, const RhiDrawIndexedDesc &desc) override {
        if (desc.topology == RhiPrimitiveTopology::Unsupported) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidDescriptor;
        }

        const RhiStatus status = command_list.RecordDrawIndexed(desc);
        if (status != RhiStatus::Success) {
            ++snapshot_.failed_operation_count;
            return status;
        }

        last_draw_index_count_ = desc.index_count;
        ++snapshot_.recorded_command_count;
        return RhiStatus::Success;
    }

    RhiStatus Submit(const RhiCommandList &command_list) override {
        if (!command_list.IsComplete()) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidLifecycle;
        }

        if (!TextureHandlesMatch(command_list.TargetHandle(), target_)) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidHandle;
        }

        const auto command_snapshot = command_list.Snapshot();
        snapshot_.command_storage_capacity_before_frame = command_snapshot.capacity;
        snapshot_.command_storage_capacity_after_last_frame = command_snapshot.capacity;
        snapshot_.submitted_indexed_draw_count += command_snapshot.indexed_draw_command_count;
        snapshot_.submitted_sampled_texture_bind_count += command_snapshot.sampled_texture_bind_command_count;
        snapshot_.submitted_sampler_bind_count += command_snapshot.sampler_bind_command_count;
        snapshot_.last_indexed_draw_index_count = last_draw_index_count_;
        ++snapshot_.submit_count;
        submitted_ = true;
        return RhiStatus::Success;
    }

    RhiStatus Present() override {
        if (!submitted_) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidLifecycle;
        }

        ++snapshot_.present_count;
        snapshot_.swapchain.presented = true;
        presented_ = true;
        return RhiStatus::Success;
    }

    RhiCaptureResult CapturePresentedTarget(std::span<std::uint8_t> destination) override {
        if (!presented_) {
            ++snapshot_.failed_operation_count;
            return RhiCaptureResult{RhiStatus::InvalidLifecycle, 0U};
        }

        const std::size_t byte_count = CaptureByteCount(
            snapshot_.swapchain.extent.width,
            snapshot_.swapchain.extent.height);
        if (destination.size() < byte_count) {
            ++snapshot_.failed_operation_count;
            return RhiCaptureResult{RhiStatus::CapacityExceeded, 0U};
        }

        for (std::size_t index = 0U; index < byte_count; index += RGBA8_BYTES_PER_PIXEL) {
            destination[index] = last_clear_color_.r;
            destination[index + 1U] = 255U;
            destination[index + 2U] = last_clear_color_.b;
            destination[index + 3U] = last_clear_color_.a;
        }

        ++snapshot_.capture_count;
        snapshot_.last_capture_bytes_written = byte_count;
        snapshot_.last_capture_extent = snapshot_.swapchain.extent;
        return RhiCaptureResult{RhiStatus::Success, byte_count, snapshot_.swapchain.extent};
    }

    RhiStatus CreateBuffer(
        const RhiBufferDesc &,
        std::span<const std::uint8_t>,
        RhiBufferHandle &out_handle) override {
        out_handle = RhiBufferHandle{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus UpdateBuffer(
        RhiBufferHandle,
        std::span<const std::uint8_t>,
        RhiFenceHandle &out_fence) override {
        out_fence = RhiFenceHandle{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus DestroyBuffer(RhiBufferHandle) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus CreateTexture(
        const RhiTextureDesc &,
        std::span<const std::uint8_t>,
        RhiTextureHandle &out_handle) override {
        out_handle = RhiTextureHandle{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus UpdateTexture(
        RhiTextureHandle,
        std::span<const std::uint8_t>,
        RhiFenceHandle &out_fence) override {
        out_fence = RhiFenceHandle{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus DestroyTexture(RhiTextureHandle) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus CreateSampler(const RhiSamplerDesc &, RhiSamplerHandle &out_handle) override {
        out_handle = RhiSamplerHandle{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus DestroySampler(RhiSamplerHandle) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus CreateShaderModule(const RhiShaderModuleDesc &, RhiShaderModuleHandle &out_handle) override {
        out_handle = RhiShaderModuleHandle{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus DestroyShaderModule(RhiShaderModuleHandle) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus CreatePipeline(const RhiPipelineDesc &, RhiPipelineHandle &out_handle) override {
        out_handle = RhiPipelineHandle{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus DestroyPipeline(RhiPipelineHandle) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus RequestPrimitiveRetirement(
        const RhiPrimitiveRetirementRequest &,
        RhiPrimitiveRetirementRecord &out_record) override {
        out_record = RhiPrimitiveRetirementRecord{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus QueryPrimitiveRetirement(
        std::uint64_t,
        RhiPrimitiveRetirementRecord &out_record) const override {
        out_record = RhiPrimitiveRetirementRecord{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus DrainPrimitiveRetirements(
        const RhiPrimitiveRetirementDrainRequest &,
        RhiPrimitiveRetirementDrainResult &out_result) override {
        out_result = RhiPrimitiveRetirementDrainResult{};
        return RhiStatus::UnsupportedBackend;
    }

    RhiCapabilities Capabilities() const override {
        RhiCapabilities capabilities{};
        capabilities.backend_kind = RhiBackendKind::Null;
        capabilities.color_format = RhiFormat::Rgba8Unorm;
        capabilities.color_target_capacity = 1U;
        capabilities.command_list_capacity = MAX_COMMANDS;
        capabilities.max_color_target_extent = MAX_COLOR_TARGET_EXTENT;
        capabilities.max_capture_fixture_extent = MAX_CAPTURE_FIXTURE_EXTENT;
        capabilities.supports_capture = true;
        capabilities.supports_swapchain = true;
        return capabilities;
    }

    RhiDeviceSnapshot Snapshot() const override {
        return snapshot_;
    }

    RhiPipelineHandle Pipeline() const {
        return pipeline_;
    }

    RhiVertexBufferView VertexBuffer() const {
        return RhiVertexBufferView{vertex_buffer_, 0U, 16U, 48U};
    }

    RhiIndexBufferView IndexBuffer() const {
        return RhiIndexBufferView{index_buffer_, 0U, 6U, RhiIndexFormat::Uint16};
    }

    RhiSampledTextureBinding SampledTexture() const {
        return RhiSampledTextureBinding{texture_, 0U};
    }

    RhiSamplerBinding Sampler() const {
        return RhiSamplerBinding{sampler_, 0U};
    }

    void SetSwapchainValid(bool value) {
        snapshot_.swapchain.valid = value;
    }

private:
    void ResetSwapchain() {
        target_ = RhiTextureHandle{1U, 1U};
        pipeline_ = RhiPipelineHandle{1U, 1U};
        vertex_buffer_ = RhiBufferHandle{1U, 1U};
        index_buffer_ = RhiBufferHandle{2U, 1U};
        texture_ = RhiTextureHandle{2U, 1U};
        sampler_ = RhiSamplerHandle{1U, 1U};
        snapshot_ = RhiDeviceSnapshot{};
        snapshot_.color_target_capacity = 1U;
        snapshot_.color_target_count = 1U;
        snapshot_.created_target_count = 1U;
        snapshot_.swapchain.valid = true;
        snapshot_.swapchain.extent.width = DEFAULT_EXTENT;
        snapshot_.swapchain.extent.height = DEFAULT_EXTENT;
        snapshot_.swapchain.color_format = RhiFormat::Rgba8Unorm;
        snapshot_.swapchain.color_target = target_;
        last_clear_color_ = RhiColor{};
        last_draw_index_count_ = 0U;
        submitted_ = false;
        presented_ = false;
    }

    RhiDeviceSnapshot snapshot_{};
    RhiTextureHandle target_{};
    RhiPipelineHandle pipeline_{};
    RhiBufferHandle vertex_buffer_{};
    RhiBufferHandle index_buffer_{};
    RhiTextureHandle texture_{};
    RhiSamplerHandle sampler_{};
    RhiColor last_clear_color_{};
    std::uint32_t last_draw_index_count_ = 0U;
    bool submitted_ = false;
    bool presented_ = false;
};

RenderDrawableFramePipelineRequest MakeRequest(
    FakeDrawableRhiDevice &device,
    std::vector<std::uint8_t> &capture,
    std::uint32_t frame_id=FRAME_ID,
    std::uint32_t pass_id=PASS_ID,
    std::uint32_t material_id=MATERIAL_ID) {
    RenderDrawableFramePipelineRequest request{};
    request.rhi_device = &device;
    request.pipeline = device.Pipeline();
    request.vertex_buffer = device.VertexBuffer();
    request.index_buffer = device.IndexBuffer();
    request.sampled_texture = device.SampledTexture();
    request.sampler = device.Sampler();
    request.draw.topology = RhiPrimitiveTopology::TriangleList;
    request.draw.index_count = 3U;
    request.clear_color = RhiColor{8U, 16U, 24U, 255U};
    request.capture_output = std::span<std::uint8_t>(capture.data(), capture.size());
    request.capture_byte_budget = capture.size();
    request.frame_id = frame_id;
    request.pass_id = pass_id;
    request.material_id = material_id;
    return request;
}

bool CaptureWasWritten(const std::vector<std::uint8_t> &capture) {
    for (std::uint8_t value : capture) {
        if (value != SENTINEL_BYTE) {
            return true;
        }
    }

    return false;
}

int RenderCoreDrawableFramePipelineExecutesMaterialFrameDrawCapture() {
    FakeDrawableRhiDevice device;
    std::vector<std::uint8_t> capture(CaptureByteCount(DEFAULT_EXTENT, DEFAULT_EXTENT), SENTINEL_BYTE);
    RenderDrawableFramePipeline pipeline;

    const auto result = pipeline.Execute(MakeRequest(device, capture));
    if (result.status != RenderDrawableFramePipelineStatus::Success) {
        return Fail("drawable frame pipeline did not complete");
    }

    if (result.material_result.status != MaterialBindingFixtureStatus::Success ||
        result.view_result.status != RenderViewPacketStatus::Success ||
        result.frame_result.status != RenderFramePacketFixtureStatus::Success ||
        result.pass_result.status != RenderFixturePassStatus::Success) {
        return Fail("drawable frame pipeline did not execute rendercore stack");
    }

    if (result.recorded_command_count != 9U || result.capture_bytes_written != capture.size()) {
        return Fail("drawable frame pipeline reported wrong command or capture count");
    }

    if (result.capture_extent.width != DEFAULT_EXTENT ||
        result.capture_extent.height != DEFAULT_EXTENT ||
        result.pass_result.capture_extent.width != DEFAULT_EXTENT ||
        result.pass_result.capture_extent.height != DEFAULT_EXTENT) {
        return Fail("drawable frame pipeline did not propagate capture extent");
    }

    if (!CaptureWasWritten(capture)) {
        return Fail("drawable frame pipeline did not write capture output");
    }

    const RhiDeviceSnapshot rhi_snapshot = device.Snapshot();
    if (rhi_snapshot.submitted_indexed_draw_count != 1U ||
        rhi_snapshot.submitted_sampled_texture_bind_count != 1U ||
        rhi_snapshot.submitted_sampler_bind_count != 1U ||
        rhi_snapshot.submit_count != 1U ||
        rhi_snapshot.present_count != 1U ||
        rhi_snapshot.capture_count != 1U) {
        return Fail("drawable frame pipeline did not drive drawable rhi counters");
    }

    const RenderDrawableFramePipelineSnapshot snapshot = pipeline.Snapshot();
    if (snapshot.completed_frame_count != 1U || snapshot.last_recorded_command_count != 9U) {
        return Fail("drawable frame pipeline snapshot did not track completed frame");
    }

    if (snapshot.last_capture_extent.width != DEFAULT_EXTENT ||
        snapshot.last_capture_extent.height != DEFAULT_EXTENT) {
        return Fail("drawable frame pipeline snapshot did not track capture extent");
    }

    return 0;
}

int RenderCoreDrawableFramePipelineRejectsInvalidSwapchainWithoutMutation() {
    FakeDrawableRhiDevice device;
    device.SetSwapchainValid(false);
    std::vector<std::uint8_t> capture(CaptureByteCount(DEFAULT_EXTENT, DEFAULT_EXTENT), SENTINEL_BYTE);
    RenderDrawableFramePipeline pipeline;

    const auto result = pipeline.Execute(MakeRequest(device, capture));
    if (result.status != RenderDrawableFramePipelineStatus::InvalidSwapchain) {
        return Fail("drawable frame pipeline accepted invalid swapchain");
    }

    if (device.Snapshot().submit_count != 0U || device.Snapshot().capture_count != 0U) {
        return Fail("invalid swapchain rejection mutated rhi state");
    }

    return 0;
}

int RenderCoreDrawableFramePipelineRejectsInvalidMaterialWithoutRhiMutation() {
    FakeDrawableRhiDevice device;
    std::vector<std::uint8_t> capture(CaptureByteCount(DEFAULT_EXTENT, DEFAULT_EXTENT), SENTINEL_BYTE);
    RenderDrawableFramePipeline pipeline;
    RenderDrawableFramePipelineRequest request = MakeRequest(device, capture);
    request.pipeline = RhiPipelineHandle{};

    const auto result = pipeline.Execute(request);
    if (result.status != RenderDrawableFramePipelineStatus::MaterialBindingFailed) {
        return Fail("drawable frame pipeline accepted invalid material binding");
    }

    if (result.material_result.status != MaterialBindingFixtureStatus::InvalidPipeline) {
        return Fail("drawable frame pipeline reported wrong material failure");
    }

    if (device.Snapshot().submit_count != 0U || device.Snapshot().recorded_command_count != 0U) {
        return Fail("invalid material rejection mutated rhi state");
    }

    return 0;
}

int RenderCoreDrawableFramePipelineRejectsInvalidDrawThroughViewPacket() {
    FakeDrawableRhiDevice device;
    std::vector<std::uint8_t> capture(CaptureByteCount(DEFAULT_EXTENT, DEFAULT_EXTENT), SENTINEL_BYTE);
    RenderDrawableFramePipeline pipeline;
    RenderDrawableFramePipelineRequest request = MakeRequest(device, capture);
    request.draw.index_count = 0U;

    const auto result = pipeline.Execute(request);
    if (result.status != RenderDrawableFramePipelineStatus::ViewPacketFailed) {
        return Fail("drawable frame pipeline accepted invalid draw packet");
    }

    if (result.view_result.status != RenderViewPacketStatus::DrawFailed) {
        return Fail("drawable frame pipeline reported wrong view packet failure");
    }

    if (device.Snapshot().submit_count != 0U || device.Snapshot().recorded_command_count != 0U) {
        return Fail("invalid draw packet mutated rhi state");
    }

    const RenderDrawableFramePipelineSnapshot snapshot = pipeline.Snapshot();
    if (snapshot.view_packet_failure_count != 1U || snapshot.material_failure_count != 0U) {
        return Fail("invalid draw packet updated wrong failure counters");
    }

    return 0;
}

int RenderCoreDrawableFramePipelineRejectsDuplicateFrameThroughFramePacket() {
    FakeDrawableRhiDevice device;
    std::vector<std::uint8_t> first_capture(CaptureByteCount(DEFAULT_EXTENT, DEFAULT_EXTENT), SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CaptureByteCount(DEFAULT_EXTENT, DEFAULT_EXTENT), SENTINEL_BYTE);
    RenderDrawableFramePipeline pipeline;
    if (pipeline.Execute(MakeRequest(device, first_capture)).status != RenderDrawableFramePipelineStatus::Success) {
        return Fail("duplicate frame setup failed");
    }

    const auto result = pipeline.Execute(MakeRequest(device, second_capture, FRAME_ID, NEXT_PASS_ID, NEXT_MATERIAL_ID));
    if (result.status != RenderDrawableFramePipelineStatus::FramePacketFailed) {
        return Fail("drawable frame pipeline accepted duplicate frame");
    }

    if (result.frame_result.status != RenderFramePacketFixtureStatus::DuplicateFrameId) {
        return Fail("drawable frame pipeline did not expose duplicate frame packet failure");
    }

    const RenderDrawableFramePipelineSnapshot snapshot = pipeline.Snapshot();
    if (snapshot.frame_packet_failure_count != 1U || snapshot.completed_frame_count != 1U) {
        return Fail("duplicate frame failure counters were wrong");
    }

    return 0;
}

int RenderCoreDrawableFramePipelinePropagatesFixtureCommandCapacityFailure() {
    FakeDrawableRhiDevice device;
    std::vector<std::uint8_t> capture(CaptureByteCount(DEFAULT_EXTENT, DEFAULT_EXTENT), SENTINEL_BYTE);
    RenderDrawableFramePipelineDesc desc{};
    desc.fixture_pass_desc.command_capacity = 8U;
    RenderDrawableFramePipeline pipeline(desc);

    const auto result = pipeline.Execute(MakeRequest(device, capture, NEXT_FRAME_ID, PASS_ID, MATERIAL_ID));
    if (result.status != RenderDrawableFramePipelineStatus::FramePacketFailed) {
        return Fail("drawable frame pipeline did not propagate command capacity failure");
    }

    if (result.pass_result.status != RenderFixturePassStatus::CommandCapacityExceeded) {
        return Fail("drawable frame pipeline reported wrong pass failure");
    }

    if (device.Snapshot().submit_count != 0U) {
        return Fail("command capacity failure submitted rhi work");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_EXECUTES) {
        return RenderCoreDrawableFramePipelineExecutesMaterialFrameDrawCapture();
    }

    if (name == TEST_INVALID_SWAPCHAIN) {
        return RenderCoreDrawableFramePipelineRejectsInvalidSwapchainWithoutMutation();
    }

    if (name == TEST_INVALID_MATERIAL) {
        return RenderCoreDrawableFramePipelineRejectsInvalidMaterialWithoutRhiMutation();
    }

    if (name == TEST_INVALID_DRAW) {
        return RenderCoreDrawableFramePipelineRejectsInvalidDrawThroughViewPacket();
    }

    if (name == TEST_DUPLICATE_FRAME) {
        return RenderCoreDrawableFramePipelineRejectsDuplicateFrameThroughFramePacket();
    }

    if (name == TEST_COMMAND_CAPACITY) {
        return RenderCoreDrawableFramePipelinePropagatesFixtureCommandCapacityFailure();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
