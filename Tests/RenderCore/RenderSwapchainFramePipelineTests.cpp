// 模块：Tests RenderCore
// 文件：Tests/RenderCore/RenderSwapchainFramePipelineTests.cpp

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>
#include <vector>

#include "YuEngine/RenderCore/RenderSwapchainFramePipeline.h"
#include "YuEngine/RenderCore/RenderSwapchainFramePipelineDesc.h"
#include "YuEngine/RenderCore/RenderSwapchainFramePipelineRequest.h"
#include "YuEngine/RenderCore/RenderSwapchainFramePipelineResult.h"
#include "YuEngine/RenderCore/RenderSwapchainFramePipelineSnapshot.h"
#include "YuEngine/RenderCore/RenderSwapchainFramePipelineStatus.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/RhiBackendKind.h"
#include "YuEngine/Rhi/RhiCaptureResult.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiDeviceSnapshot.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiSwapchainResizeRequest.h"

using RenderSwapchainFramePipeline = yuengine::rendercore::RenderSwapchainFramePipeline;
using RenderSwapchainFramePipelineDesc = yuengine::rendercore::RenderSwapchainFramePipelineDesc;
using RenderSwapchainFramePipelineRequest = yuengine::rendercore::RenderSwapchainFramePipelineRequest;
using RenderSwapchainFramePipelineSnapshot = yuengine::rendercore::RenderSwapchainFramePipelineSnapshot;
using yuengine::rendercore::RenderSwapchainFramePipelineStatus;
using yuengine::rhi::IRhiDevice;
using yuengine::rhi::RhiBackendKind;
using yuengine::rhi::RhiBlendStateDesc;
using yuengine::rhi::RhiBufferDesc;
using yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiCaptureResult;
using yuengine::rhi::RhiCapabilities;
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
using yuengine::rhi::RhiPipelineDesc;
using yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiPrimitiveRetirementDrainRequest;
using yuengine::rhi::RhiPrimitiveRetirementDrainResult;
using yuengine::rhi::RhiPrimitiveRetirementRecord;
using yuengine::rhi::RhiPrimitiveRetirementRequest;
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
constexpr const char *TEST_EXECUTES = "RenderCore_SwapchainFramePipeline_ExecutesClearSubmitPresentCapture";
constexpr const char *TEST_RESIZES = "RenderCore_SwapchainFramePipeline_ResizesBeforeSubmit";
constexpr const char *TEST_DUPLICATE = "RenderCore_SwapchainFramePipeline_RejectsDuplicateFrameIdWithoutMutation";
constexpr const char *TEST_INVALID_SWAPCHAIN = "RenderCore_SwapchainFramePipeline_RejectsInvalidSwapchainWithoutMutation";
constexpr const char *TEST_RHI_FAILURE = "RenderCore_SwapchainFramePipeline_TracksRhiFailureWithoutCapture";
constexpr const char *TEST_CAPACITY = "RenderCore_SwapchainFramePipeline_RejectsCommandCapacityWithoutRhiMutation";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t FRAME_ID = 1U;
constexpr std::uint32_t NEXT_FRAME_ID = 2U;
constexpr std::uint16_t DEFAULT_EXTENT = 2U;
constexpr std::uint16_t RESIZED_WIDTH = 3U;
constexpr std::uint16_t RESIZED_HEIGHT = 2U;
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

class FakeSwapchainRhiDevice final : public IRhiDevice {
public:
    FakeSwapchainRhiDevice() {
        ResetSwapchain(DEFAULT_EXTENT, DEFAULT_EXTENT);
    }

    RhiStatus Initialize(const RhiDeviceDesc &) override {
        ResetSwapchain(DEFAULT_EXTENT, DEFAULT_EXTENT);
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

        out_handle = swapchain_target_;
        return RhiStatus::Success;
    }

    RhiStatus ResizeSwapchain(
        const RhiSwapchainResizeRequest &request,
        RhiSwapchainResizeResult &out_result) override {
        out_result = RhiSwapchainResizeResult{};
        out_result.previous_extent = snapshot_.swapchain.extent;
        out_result.previous_color_target = swapchain_target_;
        out_result.snapshot = snapshot_.swapchain;
        if (!snapshot_.swapchain.valid) {
            out_result.status = RhiStatus::InvalidLifecycle;
            return RhiStatus::InvalidLifecycle;
        }

        if (request.extent.width == 0U || request.extent.height == 0U) {
            ++snapshot_.swapchain.rejected_resize_count;
            out_result.status = RhiStatus::InvalidDescriptor;
            out_result.snapshot = snapshot_.swapchain;
            return RhiStatus::InvalidDescriptor;
        }

        if (request.extent.width == snapshot_.swapchain.extent.width &&
            request.extent.height == snapshot_.swapchain.extent.height) {
            out_result.status = RhiStatus::Success;
            out_result.snapshot = snapshot_.swapchain;
            return RhiStatus::Success;
        }

        ++swapchain_target_.generation;
        snapshot_.swapchain.extent = request.extent;
        snapshot_.swapchain.color_target = swapchain_target_;
        ++snapshot_.swapchain.resize_count;
        out_result.status = RhiStatus::Success;
        out_result.snapshot = snapshot_.swapchain;
        out_result.resized = true;
        return RhiStatus::Success;
    }

    RhiStatus DestroyTarget(RhiTextureHandle) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus RecordClear(RhiCommandList &command_list, RhiTextureHandle handle, RhiColor color) override {
        if (!TextureHandlesMatch(handle, swapchain_target_)) {
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

    RhiStatus RecordBindPipeline(RhiCommandList &, RhiPipelineHandle) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus RecordBindVertexBuffer(RhiCommandList &, const RhiVertexBufferView &) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus RecordBindIndexBuffer(RhiCommandList &, const RhiIndexBufferView &) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus RecordBindSampledTexture(RhiCommandList &, const RhiSampledTextureBinding &) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus RecordBindSampler(RhiCommandList &, const RhiSamplerBinding &) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus RecordBindBlendState(RhiCommandList &, const RhiBlendStateDesc &) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus RecordDraw(RhiCommandList &, const RhiDrawDesc &) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus RecordDrawIndexed(RhiCommandList &, const RhiDrawIndexedDesc &) override {
        return RhiStatus::UnsupportedBackend;
    }

    RhiStatus Submit(const RhiCommandList &command_list) override {
        if (!command_list.IsComplete()) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidLifecycle;
        }

        if (!TextureHandlesMatch(command_list.TargetHandle(), swapchain_target_)) {
            ++snapshot_.failed_operation_count;
            return RhiStatus::InvalidHandle;
        }

        snapshot_.command_storage_capacity_before_frame = command_list.Capacity();
        snapshot_.command_storage_capacity_after_last_frame = command_list.Capacity();
        ++snapshot_.submit_count;
        submitted_ = true;
        return RhiStatus::Success;
    }

    RhiStatus Present() override {
        if (present_status_ != RhiStatus::Success) {
            ++snapshot_.failed_operation_count;
            return present_status_;
        }

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
            destination[index + 1U] = last_clear_color_.g;
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
        capabilities.supports_swapchain_resize = true;
        return capabilities;
    }

    RhiDeviceSnapshot Snapshot() const override {
        return snapshot_;
    }

    void SetSwapchainValid(bool value) {
        snapshot_.swapchain.valid = value;
    }

    void SetPresentStatus(RhiStatus status) {
        present_status_ = status;
    }

private:
    void ResetSwapchain(std::uint16_t width, std::uint16_t height) {
        swapchain_target_ = RhiTextureHandle{1U, 1U};
        snapshot_ = RhiDeviceSnapshot{};
        snapshot_.color_target_capacity = 1U;
        snapshot_.color_target_count = 1U;
        snapshot_.created_target_count = 1U;
        snapshot_.swapchain.valid = true;
        snapshot_.swapchain.extent.width = width;
        snapshot_.swapchain.extent.height = height;
        snapshot_.swapchain.color_format = RhiFormat::Rgba8Unorm;
        snapshot_.swapchain.color_target = swapchain_target_;
        submitted_ = false;
        presented_ = false;
        present_status_ = RhiStatus::Success;
        last_clear_color_ = RhiColor{};
    }

    RhiDeviceSnapshot snapshot_{};
    RhiTextureHandle swapchain_target_{};
    RhiColor last_clear_color_{};
    RhiStatus present_status_ = RhiStatus::Success;
    bool submitted_ = false;
    bool presented_ = false;
};

RenderSwapchainFramePipelineRequest MakeRequest(
    FakeSwapchainRhiDevice &device,
    std::vector<std::uint8_t> &capture,
    std::uint32_t frame_id=FRAME_ID) {
    RenderSwapchainFramePipelineRequest request{};
    request.rhi_device = &device;
    request.clear_color = RhiColor{8U, 16U, 24U, 255U};
    request.capture_output = std::span<std::uint8_t>(capture.data(), capture.size());
    request.capture_byte_budget = capture.size();
    request.frame_id = frame_id;
    return request;
}

bool CaptureMatchesColor(const std::vector<std::uint8_t> &capture, RhiColor color) {
    for (std::size_t index = 0U; index < capture.size(); index += RGBA8_BYTES_PER_PIXEL) {
        if (capture[index] != color.r) {
            return false;
        }

        if (capture[index + 1U] != color.g) {
            return false;
        }

        if (capture[index + 2U] != color.b) {
            return false;
        }

        if (capture[index + 3U] != color.a) {
            return false;
        }
    }

    return true;
}

int RenderCoreSwapchainFramePipelineExecutesClearSubmitPresentCapture() {
    FakeSwapchainRhiDevice device;
    std::vector<std::uint8_t> capture(CaptureByteCount(DEFAULT_EXTENT, DEFAULT_EXTENT), SENTINEL_BYTE);
    RenderSwapchainFramePipeline pipeline;
    const RenderSwapchainFramePipelineRequest request = MakeRequest(device, capture);

    const auto result = pipeline.Execute(request);
    if (result.status != RenderSwapchainFramePipelineStatus::Success) {
        return Fail("swapchain frame pipeline did not complete");
    }

    if (result.rhi_status != RhiStatus::Success || result.capture_bytes_written != capture.size()) {
        return Fail("swapchain frame pipeline reported wrong rhi result");
    }

    if (result.capture_extent.width != DEFAULT_EXTENT ||
        result.capture_extent.height != DEFAULT_EXTENT) {
        return Fail("swapchain frame pipeline did not expose capture extent");
    }

    if (!CaptureMatchesColor(capture, request.clear_color)) {
        return Fail("swapchain frame pipeline capture did not match clear color");
    }

    const RhiDeviceSnapshot rhi_snapshot = device.Snapshot();
    if (rhi_snapshot.recorded_command_count != 1U ||
        rhi_snapshot.submit_count != 1U ||
        rhi_snapshot.present_count != 1U ||
        rhi_snapshot.capture_count != 1U) {
        return Fail("swapchain frame pipeline did not drive expected rhi counters");
    }

    const RenderSwapchainFramePipelineSnapshot snapshot = pipeline.Snapshot();
    if (snapshot.completed_frame_count != 1U || snapshot.last_recorded_command_count != 3U) {
        return Fail("swapchain frame pipeline snapshot did not track completed frame");
    }

    if (snapshot.last_capture_extent.width != DEFAULT_EXTENT ||
        snapshot.last_capture_extent.height != DEFAULT_EXTENT) {
        return Fail("swapchain frame pipeline snapshot did not track capture extent");
    }

    return 0;
}

int RenderCoreSwapchainFramePipelineResizesBeforeSubmit() {
    FakeSwapchainRhiDevice device;
    std::vector<std::uint8_t> capture(CaptureByteCount(RESIZED_WIDTH, RESIZED_HEIGHT), SENTINEL_BYTE);
    RenderSwapchainFramePipeline pipeline;
    RenderSwapchainFramePipelineRequest request = MakeRequest(device, capture);
    request.resize_before_submit = true;
    request.resize_request.extent.width = RESIZED_WIDTH;
    request.resize_request.extent.height = RESIZED_HEIGHT;

    const auto result = pipeline.Execute(request);
    if (result.status != RenderSwapchainFramePipelineStatus::Success || !result.resized) {
        return Fail("swapchain frame pipeline resize frame did not complete");
    }

    if (result.swapchain_snapshot.extent.width != RESIZED_WIDTH ||
        result.swapchain_snapshot.extent.height != RESIZED_HEIGHT) {
        return Fail("swapchain frame pipeline did not expose resized extent");
    }

    if (result.capture_bytes_written != capture.size()) {
        return Fail("swapchain frame pipeline resize capture byte count mismatch");
    }

    if (result.capture_extent.width != RESIZED_WIDTH ||
        result.capture_extent.height != RESIZED_HEIGHT) {
        return Fail("swapchain frame pipeline did not expose resized capture extent");
    }

    const RenderSwapchainFramePipelineSnapshot snapshot = pipeline.Snapshot();
    if (snapshot.resize_request_count != 1U || snapshot.resized_frame_count != 1U) {
        return Fail("swapchain frame pipeline did not track resize counters");
    }

    if (snapshot.last_capture_extent.width != RESIZED_WIDTH ||
        snapshot.last_capture_extent.height != RESIZED_HEIGHT) {
        return Fail("swapchain frame pipeline snapshot did not track resized capture extent");
    }

    return 0;
}

int RenderCoreSwapchainFramePipelineRejectsDuplicateFrameIdWithoutMutation() {
    FakeSwapchainRhiDevice device;
    std::vector<std::uint8_t> first_capture(CaptureByteCount(DEFAULT_EXTENT, DEFAULT_EXTENT), SENTINEL_BYTE);
    std::vector<std::uint8_t> second_capture(CaptureByteCount(DEFAULT_EXTENT, DEFAULT_EXTENT), SENTINEL_BYTE);
    RenderSwapchainFramePipeline pipeline;
    if (pipeline.Execute(MakeRequest(device, first_capture)).status != RenderSwapchainFramePipelineStatus::Success) {
        return Fail("duplicate setup frame failed");
    }

    const RhiDeviceSnapshot before = device.Snapshot();
    const auto result = pipeline.Execute(MakeRequest(device, second_capture));
    if (result.status != RenderSwapchainFramePipelineStatus::DuplicateFrameId) {
        return Fail("swapchain frame pipeline accepted duplicate frame id");
    }

    const RhiDeviceSnapshot after = device.Snapshot();
    if (after.submit_count != before.submit_count || after.capture_count != before.capture_count) {
        return Fail("duplicate frame rejection mutated rhi state");
    }

    const RenderSwapchainFramePipelineSnapshot snapshot = pipeline.Snapshot();
    if (snapshot.duplicate_frame_id_count != 1U || snapshot.frame_record_count != 1U) {
        return Fail("duplicate frame rejection counters were wrong");
    }

    return 0;
}

int RenderCoreSwapchainFramePipelineRejectsInvalidSwapchainWithoutMutation() {
    FakeSwapchainRhiDevice device;
    device.SetSwapchainValid(false);
    std::vector<std::uint8_t> capture(CaptureByteCount(DEFAULT_EXTENT, DEFAULT_EXTENT), SENTINEL_BYTE);
    RenderSwapchainFramePipeline pipeline;

    const auto result = pipeline.Execute(MakeRequest(device, capture));
    if (result.status != RenderSwapchainFramePipelineStatus::InvalidSwapchain) {
        return Fail("swapchain frame pipeline accepted invalid swapchain");
    }

    if (device.Snapshot().submit_count != 0U || device.Snapshot().capture_count != 0U) {
        return Fail("invalid swapchain rejection mutated rhi state");
    }

    return 0;
}

int RenderCoreSwapchainFramePipelineTracksRhiFailureWithoutCapture() {
    FakeSwapchainRhiDevice device;
    device.SetPresentStatus(RhiStatus::DeviceLost);
    std::vector<std::uint8_t> capture(CaptureByteCount(DEFAULT_EXTENT, DEFAULT_EXTENT), SENTINEL_BYTE);
    RenderSwapchainFramePipeline pipeline;

    const auto result = pipeline.Execute(MakeRequest(device, capture));
    if (result.status != RenderSwapchainFramePipelineStatus::RhiFailure) {
        return Fail("swapchain frame pipeline did not report rhi failure");
    }

    if (result.rhi_status != RhiStatus::DeviceLost || result.capture_bytes_written != 0U) {
        return Fail("swapchain frame pipeline reported wrong rhi failure result");
    }

    for (std::uint8_t value : capture) {
        if (value != SENTINEL_BYTE) {
            return Fail("swapchain frame pipeline wrote capture after rhi failure");
        }
    }

    const RenderSwapchainFramePipelineSnapshot snapshot = pipeline.Snapshot();
    if (snapshot.rhi_failure_count != 1U || snapshot.frame_record_count != 1U) {
        return Fail("swapchain frame pipeline did not track rhi failure counters");
    }

    return 0;
}

int RenderCoreSwapchainFramePipelineRejectsCommandCapacityWithoutRhiMutation() {
    FakeSwapchainRhiDevice device;
    std::vector<std::uint8_t> capture(CaptureByteCount(DEFAULT_EXTENT, DEFAULT_EXTENT), SENTINEL_BYTE);
    RenderSwapchainFramePipelineDesc desc{};
    desc.command_capacity = 2U;
    RenderSwapchainFramePipeline pipeline(desc);

    const auto result = pipeline.Execute(MakeRequest(device, capture));
    if (result.status != RenderSwapchainFramePipelineStatus::CommandCapacityExceeded) {
        return Fail("swapchain frame pipeline accepted small command capacity");
    }

    const RhiDeviceSnapshot snapshot = device.Snapshot();
    if (snapshot.recorded_command_count != 0U || snapshot.submit_count != 0U) {
        return Fail("small command capacity rejection mutated rhi state");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_EXECUTES) {
        return RenderCoreSwapchainFramePipelineExecutesClearSubmitPresentCapture();
    }

    if (name == TEST_RESIZES) {
        return RenderCoreSwapchainFramePipelineResizesBeforeSubmit();
    }

    if (name == TEST_DUPLICATE) {
        return RenderCoreSwapchainFramePipelineRejectsDuplicateFrameIdWithoutMutation();
    }

    if (name == TEST_INVALID_SWAPCHAIN) {
        return RenderCoreSwapchainFramePipelineRejectsInvalidSwapchainWithoutMutation();
    }

    if (name == TEST_RHI_FAILURE) {
        return RenderCoreSwapchainFramePipelineTracksRhiFailureWithoutCapture();
    }

    if (name == TEST_CAPACITY) {
        return RenderCoreSwapchainFramePipelineRejectsCommandCapacityWithoutRhiMutation();
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
