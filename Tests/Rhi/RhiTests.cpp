#include <array>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "YuEngine/Rhi/NullRhiDevice.h"
#include "YuEngine/Rhi/RhiConstants.h"

using NullRhiDevice = yuengine::rhi::NullRhiDevice;
using yuengine::rhi::RhiBackendKind;
using RhiCaptureResult = yuengine::rhi::RhiCaptureResult;
using RhiColor = yuengine::rhi::RhiColor;
using RhiColorTargetDesc = yuengine::rhi::RhiColorTargetDesc;
using RhiCommandList = yuengine::rhi::RhiCommandList;
using RhiDeviceDesc = yuengine::rhi::RhiDeviceDesc;
using yuengine::rhi::RhiFormat;
using yuengine::rhi::RhiStatus;
using RhiTextureHandle = yuengine::rhi::RhiTextureHandle;
using yuengine::rhi::MAX_COMMANDS;
using yuengine::rhi::MAX_COLOR_TARGET_EXTENT;
using yuengine::rhi::MAX_COLOR_TARGETS;
using yuengine::rhi::RGBA8_BYTES_PER_PIXEL;

namespace {
constexpr const char* TEST_CREATE_DEVICE = "RHI_CreateNullDevice_ReturnsCapabilities";
constexpr const char* TEST_UNSUPPORTED_BACKEND = "RHI_CreateDevice_RejectsUnsupportedBackend";
constexpr const char* TEST_CREATE_TARGET = "RHI_CreateTarget_ReturnsGenerationHandle";
constexpr const char* TEST_CREATE_COLOR_TARGET = "RHI_CreateColorTarget_ReturnsGenerationHandle";
constexpr const char* TEST_INVALID_DESCRIPTOR = "RHI_CreateColorTarget_RejectsInvalidDescriptor";
constexpr const char* TEST_TARGET_CAPACITY = "RHI_TargetCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_DESTROY_STALE = "RHI_DestroyTarget_InvalidatesStaleHandle";
constexpr const char* TEST_REINITIALIZE_STALE_TARGET = "RHI_Reinitialize_InvalidatesPriorTargetHandle";
constexpr const char* TEST_RECORD_CLEAR = "RHI_CommandList_RecordsClearWithinCapacity";
constexpr const char* TEST_COMMAND_CAPACITY = "RHI_CommandListCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_SUBMIT_OVERSIZE_COMMAND_LIST = "RHI_SubmitRejectsOversizedCommandListWithoutMutation";
constexpr const char* TEST_INVALID_CLEAR_TARGET = "RHI_RecordClear_RejectsInvalidTargetHandle";
constexpr const char* TEST_INCOMPLETE_SUBMIT = "RHI_SubmitRejectsIncompleteCommandListWithoutMutation";
constexpr const char* TEST_MISMATCHED_SUBMIT_TARGET = "RHI_SubmitRejectsMismatchedRecordedTargetWithoutMutation";
constexpr const char* TEST_STALE_SUBMIT_TARGET = "RHI_SubmitRejectsStaleRecordedTargetWithoutMutation";
constexpr const char* TEST_SUBMIT_EXECUTES_CLEAR = "RHI_SubmitExecutesClearIntoNullTarget";
constexpr const char* TEST_PRESENT_REQUIRES_SUBMIT = "RHI_PresentRequiresSuccessfulSubmit";
constexpr const char* TEST_PRESENT_DESTROYED_SUBMITTED_TARGET = "RHI_PresentRejectsDestroyedSubmittedTargetWithoutMutation";
constexpr const char* TEST_PRESENT_COUNTER = "RHI_ClearSubmitPresent_UpdatesPresentedCounter";
constexpr const char* TEST_CAPTURE_BEFORE_PRESENT = "RHI_CaptureBeforePresent_ReturnsExplicitStatus";
constexpr const char* TEST_CLEAR_COLOR = "RHI_ClearColor_UsesExactRgba8ByteChannels";
constexpr const char* TEST_CAPTURE_DETERMINISTIC = "RHI_CapturePresentedTarget_WritesDeterministicRgba8Bytes";
constexpr const char* TEST_CAPTURE_DESTROYED_PRESENTED_TARGET = "RHI_CaptureRejectsDestroyedPresentedTargetWithoutMutation";
constexpr const char* TEST_UNDERSIZED_CAPTURE = "RHI_CaptureRejectsUndersizedBufferWithoutWritingBytes";
constexpr const char* TEST_OVERSIZED_CAPTURE_FIXTURE = "RHI_CaptureRejectsTargetLargerThanFixtureCapWithoutWritingBytes";
constexpr const char* TEST_FRAME_NO_GROW = "RHI_FrameSubmitPresentCapture_DoesNotGrowCommandStorage";
constexpr const char* TEST_DISABLED_DIAGNOSTICS = "RHI_DisabledDiagnosticsDoesNotChangeResults";
constexpr const char* TEST_NO_FORBIDDEN_DEPENDENCY = "RHI_NoResourceFileUploadShaderUiDependency";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr const char* REINIT_TARGET_CREATION_MESSAGE = "target creation failed";
constexpr const char* REINIT_DEVICE_MESSAGE = "device reinitialize failed";
constexpr const char* REINIT_ACTIVE_TARGET_CREATION_MESSAGE = "target creation after reinitialize failed";
constexpr const char* REINIT_STALE_TARGET_ACCEPTED_MESSAGE = "stale target handle from prior initialize was accepted";
constexpr const char* REINIT_STALE_TARGET_COUNT_MESSAGE = "stale target handle changed target count";
constexpr const char* REINIT_BEGIN_FRAME_MESSAGE = "begin frame failed";
constexpr const char* REINIT_STALE_CLEAR_MESSAGE = "stale target handle was accepted for clear";
constexpr const char* REINIT_STALE_CLEAR_COUNT_MESSAGE = "stale clear changed recorded command count";
constexpr const char* REINIT_ACTIVE_TARGET_MESSAGE = "active target did not survive stale handle checks";
constexpr const char* PRESENT_TARGET_CREATION_MESSAGE = "target creation failed";
constexpr const char* PRESENT_BEGIN_FRAME_MESSAGE = "begin frame failed";
constexpr const char* PRESENT_RECORD_CLEAR_MESSAGE = "record clear failed";
constexpr const char* PRESENT_END_FRAME_MESSAGE = "end frame failed";
constexpr const char* PRESENT_SUBMIT_MESSAGE = "submit failed";
constexpr const char* PRESENT_DESTROY_SUBMITTED_MESSAGE = "destroy submitted target failed";
constexpr const char* PRESENT_DESTROYED_ACCEPTED_MESSAGE = "present accepted destroyed submitted target";
constexpr const char* PRESENT_COUNT_MUTATED_MESSAGE = "rejected present mutated present count";
constexpr const char* PRESENT_DESTROY_COUNT_MUTATED_MESSAGE = "rejected present mutated destroyed target count";
constexpr const char* CAPTURE_DESTROYED_TARGET_CREATION_MESSAGE = "target creation failed";
constexpr const char* CAPTURE_DESTROYED_CLEAR_PRESENT_MESSAGE = "clear submit present failed";
constexpr const char* CAPTURE_DESTROYED_BASELINE_MESSAGE = "baseline capture failed";
constexpr const char* CAPTURE_DESTROYED_DESTROY_MESSAGE = "destroy presented target failed";
constexpr const char* CAPTURE_DESTROYED_ACCEPTED_MESSAGE = "capture accepted destroyed presented target";
constexpr const char* CAPTURE_DESTROYED_BYTES_MESSAGE = "destroyed target capture reported bytes written";
constexpr const char* CAPTURE_DESTROYED_WRITE_MESSAGE = "destroyed target capture wrote destination bytes";
constexpr const char* CAPTURE_DESTROYED_COUNT_MESSAGE = "destroyed target capture changed capture count";
constexpr const char* CAPTURE_DESTROYED_LAST_BYTES_MESSAGE = "destroyed target capture changed last capture byte count";
constexpr const char* CAPTURE_DESTROYED_DESTROY_COUNT_MESSAGE = "destroyed target capture changed destroy count";
constexpr std::uint8_t SENTINEL_BYTE = 0xAAU;
using TestFunction = int (*)();

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

RhiColorTargetDesc SmallTargetDesc() {
    return RhiColorTargetDesc{RhiFormat::Rgba8Unorm, {2U, 2U}};
}

RhiColorTargetDesc CaptureFixtureTargetDesc() {
    return RhiColorTargetDesc{RhiFormat::Rgba8Unorm, {4U, 4U}};
}

RhiColorTargetDesc MaxTargetDesc() {
    return RhiColorTargetDesc{
        RhiFormat::Rgba8Unorm,
        {MAX_COLOR_TARGET_EXTENT, MAX_COLOR_TARGET_EXTENT}};
}

NullRhiDevice CreateInitializedDevice() {
    NullRhiDevice device;
    device.Initialize(RhiDeviceDesc{});
    return device;
}

bool CreateTarget(NullRhiDevice& device, RhiTextureHandle& outHandle) {
    return device.CreateColorTarget(SmallTargetDesc(), outHandle) == RhiStatus::Success;
}

RhiStatus ClearSubmitPresent(NullRhiDevice& device, RhiTextureHandle target, RhiColor color) {
    RhiCommandList commandList(MAX_COMMANDS);
    RhiStatus status = commandList.BeginFrame(target);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordClear(commandList, target, color);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = commandList.EndFrame();
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.Submit(commandList);
    if (status != RhiStatus::Success) {
        return status;
    }

    return device.Present();
}

bool BytesMatchColor(const std::vector<std::uint8_t>& bytes, RhiColor color) {
    for (std::size_t index = 0U; index < bytes.size(); index += RGBA8_BYTES_PER_PIXEL) {
        if (bytes[index] != color.r) {
            return false;
        }

        if (bytes[index + 1U] != color.g) {
            return false;
        }

        if (bytes[index + 2U] != color.b) {
            return false;
        }

        if (bytes[index + 3U] != color.a) {
            return false;
        }
    }

    return true;
}

int RhiCreateNullDeviceReturnsCapabilities() {
    NullRhiDevice device;
    const RhiStatus status = device.Initialize(RhiDeviceDesc{});
    if (status != RhiStatus::Success) {
        return Fail("null device did not initialize");
    }

    const auto capabilities = device.Capabilities();
    if (capabilities.backend_kind != RhiBackendKind::Null) {
        return Fail("capabilities did not report null backend");
    }

    if (capabilities.color_target_capacity != MAX_COLOR_TARGETS) {
        return Fail("capabilities reported wrong target capacity");
    }

    if (capabilities.command_list_capacity != MAX_COMMANDS) {
        return Fail("capabilities reported wrong command capacity");
    }

    if (!capabilities.supports_capture) {
        return Fail("capabilities did not report capture support");
    }

    return 0;
}

int RhiCreateDeviceRejectsUnsupportedBackend() {
    NullRhiDevice device;
    RhiDeviceDesc desc{};
    desc.backend_kind = RhiBackendKind::Unsupported;

    const RhiStatus status = device.Initialize(desc);
    if (status != RhiStatus::UnsupportedBackend) {
        return Fail("unsupported backend did not return explicit status");
    }

    if (device.Snapshot().color_target_capacity != 0U) {
        return Fail("unsupported backend mutated device capacity");
    }

    return 0;
}

int RhiCreateTargetReturnsGenerationHandle() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail("target creation failed");
    }

    if (handle.generation == 0U) {
        return Fail("target handle generation was invalid");
    }

    if (handle.slot != 0U) {
        return Fail("first target used unexpected slot");
    }

    if (device.Snapshot().created_target_count != 1U) {
        return Fail("created target count was not recorded");
    }

    return 0;
}

int RhiCreateColorTargetRejectsInvalidDescriptor() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};

    RhiColorTargetDesc unsupportedFormatDesc = SmallTargetDesc();
    unsupportedFormatDesc.format = RhiFormat::Unsupported;
    if (device.CreateColorTarget(unsupportedFormatDesc, handle) != RhiStatus::UnsupportedFormat) {
        return Fail("unsupported target format was not rejected");
    }

    RhiColorTargetDesc zeroExtentDesc = SmallTargetDesc();
    zeroExtentDesc.extent.width = 0U;
    if (device.CreateColorTarget(zeroExtentDesc, handle) != RhiStatus::InvalidDescriptor) {
        return Fail("zero extent target was not rejected");
    }

    RhiColorTargetDesc overExtentDesc = SmallTargetDesc();
    overExtentDesc.extent.width = MAX_COLOR_TARGET_EXTENT + 1U;
    if (device.CreateColorTarget(overExtentDesc, handle) != RhiStatus::InvalidDescriptor) {
        return Fail("overlarge extent target was not rejected");
    }

    if (device.Snapshot().color_target_count != 0U) {
        return Fail("invalid descriptors mutated target count");
    }

    return 0;
}

int RhiTargetCapacityOverflowDoesNotMutate() {
    NullRhiDevice device = CreateInitializedDevice();
    std::array<RhiTextureHandle, MAX_COLOR_TARGETS> handles{};
    for (std::size_t index = 0U; index < handles.size(); ++index) {
        const RhiStatus status = device.CreateColorTarget(SmallTargetDesc(), handles[index]);
        if (status != RhiStatus::Success) {
            return Fail("target creation failed before capacity");
        }
    }

    const auto beforeSnapshot = device.Snapshot();
    RhiTextureHandle overflowHandle{};
    const RhiStatus overflowStatus = device.CreateColorTarget(SmallTargetDesc(), overflowHandle);
    if (overflowStatus != RhiStatus::CapacityExceeded) {
        return Fail("target capacity overflow did not return explicit status");
    }

    const auto afterSnapshot = device.Snapshot();
    if (afterSnapshot.color_target_count != beforeSnapshot.color_target_count) {
        return Fail("target overflow changed target count");
    }

    if (afterSnapshot.created_target_count != beforeSnapshot.created_target_count) {
        return Fail("target overflow changed created count");
    }

    return 0;
}

int RhiDestroyTargetInvalidatesStaleHandle() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail("target creation failed");
    }

    if (device.DestroyTarget(handle) != RhiStatus::Success) {
        return Fail("target destroy failed");
    }

    RhiCommandList commandList(MAX_COMMANDS);
    commandList.BeginFrame(handle);
    const RhiStatus staleStatus = device.RecordClear(commandList, handle, RhiColor{1U, 2U, 3U, 4U});
    if (staleStatus != RhiStatus::InvalidHandle) {
        return Fail("stale handle did not return explicit status");
    }

    if (device.Snapshot().destroyed_target_count != 1U) {
        return Fail("destroyed target count was not recorded");
    }

    return 0;
}

int RhiReinitializeInvalidatesPriorTargetHandle() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle staleHandle{};
    if (!CreateTarget(device, staleHandle)) {
        return Fail(REINIT_TARGET_CREATION_MESSAGE);
    }

    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail(REINIT_DEVICE_MESSAGE);
    }

    RhiTextureHandle activeHandle{};
    if (!CreateTarget(device, activeHandle)) {
        return Fail(REINIT_ACTIVE_TARGET_CREATION_MESSAGE);
    }

    const auto beforeSnapshot = device.Snapshot();
    if (device.DestroyTarget(staleHandle) != RhiStatus::InvalidHandle) {
        return Fail(REINIT_STALE_TARGET_ACCEPTED_MESSAGE);
    }

    if (device.Snapshot().color_target_count != beforeSnapshot.color_target_count) {
        return Fail(REINIT_STALE_TARGET_COUNT_MESSAGE);
    }

    RhiCommandList commandList(MAX_COMMANDS);
    if (commandList.BeginFrame(activeHandle) != RhiStatus::Success) {
        return Fail(REINIT_BEGIN_FRAME_MESSAGE);
    }

    if (device.RecordClear(commandList, staleHandle, RhiColor{1U, 2U, 3U, 4U}) != RhiStatus::InvalidHandle) {
        return Fail(REINIT_STALE_CLEAR_MESSAGE);
    }

    if (device.Snapshot().recorded_command_count != beforeSnapshot.recorded_command_count) {
        return Fail(REINIT_STALE_CLEAR_COUNT_MESSAGE);
    }

    if (device.DestroyTarget(activeHandle) != RhiStatus::Success) {
        return Fail(REINIT_ACTIVE_TARGET_MESSAGE);
    }

    return 0;
}

int RhiCommandListRecordsClearWithinCapacity() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail("target creation failed");
    }

    RhiCommandList commandList(3U);
    if (commandList.BeginFrame(handle) != RhiStatus::Success) {
        return Fail("begin frame failed");
    }

    if (device.RecordClear(commandList, handle, RhiColor{1U, 2U, 3U, 4U}) != RhiStatus::Success) {
        return Fail("record clear failed");
    }

    if (commandList.EndFrame() != RhiStatus::Success) {
        return Fail("end frame failed within capacity");
    }

    if (commandList.CommandCount() != 3U) {
        return Fail("clear command list count was wrong");
    }

    return 0;
}

int RhiCommandListCapacityOverflowDoesNotMutate() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail("target creation failed");
    }

    RhiCommandList commandList(2U);
    if (commandList.BeginFrame(handle) != RhiStatus::Success) {
        return Fail("begin frame failed");
    }

    if (device.RecordClear(commandList, handle, RhiColor{1U, 2U, 3U, 4U}) != RhiStatus::Success) {
        return Fail("clear record failed");
    }

    const std::size_t countBefore = commandList.CommandCount();
    const RhiStatus endStatus = commandList.EndFrame();
    if (endStatus != RhiStatus::CapacityExceeded) {
        return Fail("command capacity overflow did not return explicit status");
    }

    if (commandList.CommandCount() != countBefore) {
        return Fail("command capacity overflow mutated command count");
    }

    return 0;
}

int RhiSubmitRejectsOversizedCommandListWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail("target creation failed");
    }

    const RhiColor initialColor{1U, 2U, 3U, 4U};
    if (ClearSubmitPresent(device, handle, initialColor) != RhiStatus::Success) {
        return Fail("initial clear submit present failed");
    }

    const auto beforeSnapshot = device.Snapshot();
    RhiCommandList oversizedCommandList(MAX_COMMANDS + 1U);
    if (oversizedCommandList.BeginFrame(handle) != RhiStatus::Success) {
        return Fail("oversized begin frame failed before submit");
    }

    if (device.RecordClear(oversizedCommandList, handle, RhiColor{9U, 8U, 7U, 6U}) != RhiStatus::Success) {
        return Fail("oversized clear record failed before submit");
    }

    if (oversizedCommandList.EndFrame() != RhiStatus::Success) {
        return Fail("oversized end frame failed before submit");
    }

    const RhiStatus submitStatus = device.Submit(oversizedCommandList);
    if (submitStatus != RhiStatus::CapacityExceeded) {
        return Fail("oversized command list submit did not return capacity status");
    }

    const auto afterSnapshot = device.Snapshot();
    if (afterSnapshot.submit_count != beforeSnapshot.submit_count) {
        return Fail("oversized command list submit mutated submit count");
    }

    std::vector<std::uint8_t> capture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    if (result.status != RhiStatus::Success) {
        return Fail("capture failed after rejected oversized submit");
    }

    if (!BytesMatchColor(capture, initialColor)) {
        return Fail("rejected oversized submit mutated target bytes");
    }

    return 0;
}

int RhiRecordClearRejectsInvalidTargetHandle() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail("target creation failed");
    }

    RhiCommandList commandList(MAX_COMMANDS);
    if (commandList.BeginFrame(handle) != RhiStatus::Success) {
        return Fail("begin frame failed");
    }

    const std::size_t countBefore = commandList.CommandCount();
    const RhiStatus status = device.RecordClear(commandList, RhiTextureHandle{99U, 1U}, RhiColor{1U, 2U, 3U, 4U});
    if (status != RhiStatus::InvalidHandle) {
        return Fail("invalid target clear did not return handle status");
    }

    if (commandList.CommandCount() != countBefore) {
        return Fail("invalid target clear mutated command list");
    }

    return 0;
}

int RhiSubmitRejectsMismatchedRecordedTargetWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle frameTarget{};
    RhiTextureHandle otherTarget{};
    if (!CreateTarget(device, frameTarget)) {
        return Fail("frame target creation failed");
    }

    if (!CreateTarget(device, otherTarget)) {
        return Fail("other target creation failed");
    }

    const RhiColor otherInitialColor{4U, 3U, 2U, 1U};
    if (ClearSubmitPresent(device, otherTarget, otherInitialColor) != RhiStatus::Success) {
        return Fail("initial other target clear submit present failed");
    }

    const auto beforeSnapshot = device.Snapshot();
    RhiCommandList commandList(MAX_COMMANDS);
    if (commandList.BeginFrame(frameTarget) != RhiStatus::Success) {
        return Fail("begin frame failed");
    }

    if (device.RecordClear(commandList, otherTarget, RhiColor{9U, 8U, 7U, 6U}) != RhiStatus::Success) {
        return Fail("mismatched clear record failed before submit");
    }

    if (commandList.EndFrame() != RhiStatus::Success) {
        return Fail("end frame failed");
    }

    const RhiStatus submitStatus = device.Submit(commandList);
    if (submitStatus != RhiStatus::InvalidHandle) {
        return Fail("mismatched recorded target did not return explicit status");
    }

    const auto afterSnapshot = device.Snapshot();
    if (afterSnapshot.submit_count != beforeSnapshot.submit_count) {
        return Fail("mismatched target submit mutated submit count");
    }

    std::vector<std::uint8_t> capture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    if (result.status != RhiStatus::Success) {
        return Fail("capture failed after rejected mismatched submit");
    }

    if (!BytesMatchColor(capture, otherInitialColor)) {
        return Fail("rejected mismatched submit mutated recorded target bytes");
    }

    return 0;
}

int RhiSubmitRejectsStaleRecordedTargetWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle originalTarget{};
    if (!CreateTarget(device, originalTarget)) {
        return Fail("original target creation failed");
    }

    if (device.DestroyTarget(originalTarget) != RhiStatus::Success) {
        return Fail("destroying original target failed");
    }

    RhiTextureHandle frameTarget{};
    if (!CreateTarget(device, frameTarget)) {
        return Fail("replacement frame target creation failed");
    }

    if (frameTarget.slot != originalTarget.slot) {
        return Fail("replacement target did not reuse slot for stale generation test");
    }

    if (frameTarget.generation == originalTarget.generation) {
        return Fail("replacement target did not advance generation");
    }

    const RhiColor frameInitialColor{5U, 6U, 7U, 8U};
    if (ClearSubmitPresent(device, frameTarget, frameInitialColor) != RhiStatus::Success) {
        return Fail("initial frame target clear submit present failed");
    }

    const auto beforeSnapshot = device.Snapshot();
    RhiCommandList commandList(MAX_COMMANDS);
    if (commandList.BeginFrame(frameTarget) != RhiStatus::Success) {
        return Fail("begin frame failed");
    }

    if (commandList.RecordClear(originalTarget, RhiColor{9U, 8U, 7U, 6U}) != RhiStatus::Success) {
        return Fail("stale generation clear record failed before submit");
    }

    if (commandList.EndFrame() != RhiStatus::Success) {
        return Fail("end frame failed");
    }

    const RhiStatus submitStatus = device.Submit(commandList);
    if (submitStatus != RhiStatus::InvalidHandle) {
        return Fail("stale recorded target did not return explicit status");
    }

    const auto afterSnapshot = device.Snapshot();
    if (afterSnapshot.submit_count != beforeSnapshot.submit_count) {
        return Fail("stale target submit mutated submit count");
    }

    if (afterSnapshot.color_target_count != beforeSnapshot.color_target_count) {
        return Fail("stale target submit mutated target count");
    }

    std::vector<std::uint8_t> capture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    if (result.status != RhiStatus::Success) {
        return Fail("capture failed after rejected stale generation submit");
    }

    if (!BytesMatchColor(capture, frameInitialColor)) {
        return Fail("rejected stale generation submit mutated frame target bytes");
    }

    return 0;
}

int RhiSubmitRejectsIncompleteCommandListWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail("target creation failed");
    }

    RhiCommandList commandList(MAX_COMMANDS);
    commandList.BeginFrame(handle);
    device.RecordClear(commandList, handle, RhiColor{9U, 8U, 7U, 6U});

    const auto beforeSnapshot = device.Snapshot();
    const RhiStatus status = device.Submit(commandList);
    if (status != RhiStatus::InvalidLifecycle) {
        return Fail("incomplete command list did not return lifecycle status");
    }

    const auto afterSnapshot = device.Snapshot();
    if (afterSnapshot.submit_count != beforeSnapshot.submit_count) {
        return Fail("incomplete submit changed submit count");
    }

    if (device.Present() != RhiStatus::InvalidLifecycle) {
        return Fail("present succeeded after rejected submit");
    }

    return 0;
}

int RhiSubmitExecutesClearIntoNullTarget() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail("target creation failed");
    }

    const RhiColor color{11U, 12U, 13U, 14U};
    if (ClearSubmitPresent(device, handle, color) != RhiStatus::Success) {
        return Fail("clear submit present failed");
    }

    std::vector<std::uint8_t> capture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    if (result.status != RhiStatus::Success) {
        return Fail("capture failed after clear submit present");
    }

    if (!BytesMatchColor(capture, color)) {
        return Fail("submit did not execute clear into null target");
    }

    return 0;
}

int RhiPresentRequiresSuccessfulSubmit() {
    NullRhiDevice device = CreateInitializedDevice();
    if (device.Present() != RhiStatus::InvalidLifecycle) {
        return Fail("present without submit did not return lifecycle status");
    }

    if (device.Snapshot().present_count != 0U) {
        return Fail("present without submit changed present count");
    }

    return 0;
}

int RhiPresentRejectsDestroyedSubmittedTargetWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail(PRESENT_TARGET_CREATION_MESSAGE);
    }

    RhiCommandList commandList(MAX_COMMANDS);
    if (commandList.BeginFrame(handle) != RhiStatus::Success) {
        return Fail(PRESENT_BEGIN_FRAME_MESSAGE);
    }

    if (device.RecordClear(commandList, handle, RhiColor{1U, 2U, 3U, 4U}) != RhiStatus::Success) {
        return Fail(PRESENT_RECORD_CLEAR_MESSAGE);
    }

    if (commandList.EndFrame() != RhiStatus::Success) {
        return Fail(PRESENT_END_FRAME_MESSAGE);
    }

    if (device.Submit(commandList) != RhiStatus::Success) {
        return Fail(PRESENT_SUBMIT_MESSAGE);
    }

    const auto beforeSnapshot = device.Snapshot();
    if (device.DestroyTarget(handle) != RhiStatus::Success) {
        return Fail(PRESENT_DESTROY_SUBMITTED_MESSAGE);
    }

    const auto afterDestroySnapshot = device.Snapshot();
    if (device.Present() != RhiStatus::InvalidHandle) {
        return Fail(PRESENT_DESTROYED_ACCEPTED_MESSAGE);
    }

    const auto afterPresentSnapshot = device.Snapshot();
    if (afterPresentSnapshot.present_count != beforeSnapshot.present_count) {
        return Fail(PRESENT_COUNT_MUTATED_MESSAGE);
    }

    if (afterPresentSnapshot.destroyed_target_count != afterDestroySnapshot.destroyed_target_count) {
        return Fail(PRESENT_DESTROY_COUNT_MUTATED_MESSAGE);
    }

    return 0;
}

int RhiClearSubmitPresentUpdatesPresentedCounter() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail("target creation failed");
    }

    const RhiStatus status = ClearSubmitPresent(device, handle, RhiColor{1U, 2U, 3U, 4U});
    if (status != RhiStatus::Success) {
        return Fail("clear submit present did not succeed");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.submit_count != 1U) {
        return Fail("submit count was not updated");
    }

    if (snapshot.present_count != 1U) {
        return Fail("present count was not updated");
    }

    return 0;
}

int RhiCaptureBeforePresentReturnsExplicitStatus() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail("target creation failed");
    }

    std::vector<std::uint8_t> capture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    if (result.status != RhiStatus::InvalidLifecycle) {
        return Fail("capture before present did not return lifecycle status");
    }

    if (result.bytes_written != 0U) {
        return Fail("capture before present wrote bytes");
    }

    return 0;
}

int RhiClearColorUsesExactRgba8ByteChannels() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail("target creation failed");
    }

    const RhiColor color{1U, 2U, 253U, 255U};
    if (ClearSubmitPresent(device, handle, color) != RhiStatus::Success) {
        return Fail("clear submit present failed");
    }

    std::vector<std::uint8_t> capture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    if (result.status != RhiStatus::Success) {
        return Fail("capture failed");
    }

    if (!BytesMatchColor(capture, color)) {
        return Fail("capture bytes did not exactly match RGBA8 clear channels");
    }

    return 0;
}

int RhiCapturePresentedTargetWritesDeterministicRgba8Bytes() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    const RhiStatus createStatus = device.CreateColorTarget(CaptureFixtureTargetDesc(), handle);
    if (createStatus != RhiStatus::Success) {
        return Fail("target creation failed");
    }

    const RhiColor color{4U, 5U, 6U, 7U};
    if (ClearSubmitPresent(device, handle, color) != RhiStatus::Success) {
        return Fail("clear submit present failed");
    }

    std::vector<std::uint8_t> firstCapture(4U * 4U * RGBA8_BYTES_PER_PIXEL);
    std::vector<std::uint8_t> secondCapture(4U * 4U * RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult firstResult = device.CapturePresentedTarget(std::span<std::uint8_t>(firstCapture.data(), firstCapture.size()));
    const RhiCaptureResult secondResult = device.CapturePresentedTarget(std::span<std::uint8_t>(secondCapture.data(), secondCapture.size()));
    if (firstResult.status != RhiStatus::Success) {
        return Fail("first capture failed");
    }

    if (secondResult.status != RhiStatus::Success) {
        return Fail("second capture failed");
    }

    if (firstCapture != secondCapture) {
        return Fail("capture bytes were not deterministic");
    }

    if (!BytesMatchColor(firstCapture, color)) {
        return Fail("deterministic capture did not match clear color");
    }

    return 0;
}

int RhiCaptureRejectsDestroyedPresentedTargetWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail(CAPTURE_DESTROYED_TARGET_CREATION_MESSAGE);
    }

    if (ClearSubmitPresent(device, handle, RhiColor{1U, 2U, 3U, 4U}) != RhiStatus::Success) {
        return Fail(CAPTURE_DESTROYED_CLEAR_PRESENT_MESSAGE);
    }

    std::vector<std::uint8_t> acceptedCapture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    if (device.CapturePresentedTarget(std::span<std::uint8_t>(acceptedCapture.data(), acceptedCapture.size())).status != RhiStatus::Success) {
        return Fail(CAPTURE_DESTROYED_BASELINE_MESSAGE);
    }

    const auto beforeDestroySnapshot = device.Snapshot();
    if (device.DestroyTarget(handle) != RhiStatus::Success) {
        return Fail(CAPTURE_DESTROYED_DESTROY_MESSAGE);
    }

    const auto afterDestroySnapshot = device.Snapshot();
    std::vector<std::uint8_t> rejectedCapture(2U * 2U * RGBA8_BYTES_PER_PIXEL, SENTINEL_BYTE);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(rejectedCapture.data(), rejectedCapture.size()));
    if (result.status != RhiStatus::InvalidHandle) {
        return Fail(CAPTURE_DESTROYED_ACCEPTED_MESSAGE);
    }

    if (result.bytes_written != 0U) {
        return Fail(CAPTURE_DESTROYED_BYTES_MESSAGE);
    }

    for (const std::uint8_t byte : rejectedCapture) {
        if (byte != SENTINEL_BYTE) {
            return Fail(CAPTURE_DESTROYED_WRITE_MESSAGE);
        }
    }

    const auto afterCaptureSnapshot = device.Snapshot();
    if (afterCaptureSnapshot.capture_count != beforeDestroySnapshot.capture_count) {
        return Fail(CAPTURE_DESTROYED_COUNT_MESSAGE);
    }

    if (afterCaptureSnapshot.last_capture_bytes_written != beforeDestroySnapshot.last_capture_bytes_written) {
        return Fail(CAPTURE_DESTROYED_LAST_BYTES_MESSAGE);
    }

    if (afterCaptureSnapshot.destroyed_target_count != afterDestroySnapshot.destroyed_target_count) {
        return Fail(CAPTURE_DESTROYED_DESTROY_COUNT_MESSAGE);
    }

    return 0;
}

int RhiCaptureRejectsUndersizedBufferWithoutWritingBytes() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail("target creation failed");
    }

    if (ClearSubmitPresent(device, handle, RhiColor{1U, 1U, 1U, 1U}) != RhiStatus::Success) {
        return Fail("clear submit present failed");
    }

    std::vector<std::uint8_t> destination((2U * 2U * RGBA8_BYTES_PER_PIXEL) - 1U, SENTINEL_BYTE);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(destination.data(), destination.size()));
    if (result.status != RhiStatus::CapacityExceeded) {
        return Fail("undersized capture did not return capacity status");
    }

    if (result.bytes_written != 0U) {
        return Fail("undersized capture reported nonzero bytes written");
    }

    for (const std::uint8_t byte : destination) {
        if (byte != SENTINEL_BYTE) {
            return Fail("undersized capture mutated destination bytes");
        }
    }

    if (device.Snapshot().last_capture_bytes_written != 0U) {
        return Fail("undersized capture did not record zero bytes written");
    }

    return 0;
}

int RhiCaptureRejectsTargetLargerThanFixtureCapWithoutWritingBytes() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (device.CreateColorTarget(MaxTargetDesc(), handle) != RhiStatus::Success) {
        return Fail("max target creation failed");
    }

    if (ClearSubmitPresent(device, handle, RhiColor{1U, 2U, 3U, 4U}) != RhiStatus::Success) {
        return Fail("clear submit present failed");
    }

    const std::size_t fullTargetBytes = static_cast<std::size_t>(MAX_COLOR_TARGET_EXTENT) *
        static_cast<std::size_t>(MAX_COLOR_TARGET_EXTENT) * RGBA8_BYTES_PER_PIXEL;
    std::vector<std::uint8_t> destination(fullTargetBytes, SENTINEL_BYTE);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(destination.data(), destination.size()));
    if (result.status != RhiStatus::CapacityExceeded) {
        return Fail("oversized capture fixture did not return capacity status");
    }

    if (result.bytes_written != 0U) {
        return Fail("oversized capture fixture reported bytes written");
    }

    for (const std::uint8_t byte : destination) {
        if (byte != SENTINEL_BYTE) {
            return Fail("oversized capture fixture mutated destination bytes");
        }
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.last_capture_bytes_written != 0U) {
        return Fail("oversized capture fixture did not record zero bytes written");
    }

    if (snapshot.capture_count != 0U) {
        return Fail("oversized capture fixture incremented capture count");
    }

    return 0;
}

int RhiFrameSubmitPresentCaptureDoesNotGrowCommandStorage() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail("target creation failed");
    }

    RhiCommandList commandList(MAX_COMMANDS);
    const std::size_t capacityBefore = commandList.Capacity();
    commandList.BeginFrame(handle);
    device.RecordClear(commandList, handle, RhiColor{1U, 2U, 3U, 4U});
    commandList.EndFrame();
    device.Submit(commandList);
    device.Present();

    std::vector<std::uint8_t> capture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));

    const auto snapshot = device.Snapshot();
    if (commandList.Capacity() != capacityBefore) {
        return Fail("command list capacity changed during frame fixture");
    }

    if (snapshot.command_storage_capacity_before_frame != snapshot.command_storage_capacity_after_last_frame) {
        return Fail("device snapshot recorded command storage growth");
    }

    if (snapshot.command_storage_capacity_before_frame != capacityBefore) {
        return Fail("device snapshot recorded wrong command storage capacity");
    }

    return 0;
}

int RhiDisabledDiagnosticsDoesNotChangeResults() {
    NullRhiDevice enabledLikeDevice = CreateInitializedDevice();
    NullRhiDevice disabledLikeDevice = CreateInitializedDevice();
    RhiTextureHandle enabledHandle{};
    RhiTextureHandle disabledHandle{};
    enabledLikeDevice.CreateColorTarget(SmallTargetDesc(), enabledHandle);
    disabledLikeDevice.CreateColorTarget(SmallTargetDesc(), disabledHandle);

    const RhiColor color{3U, 4U, 5U, 6U};
    const RhiStatus enabledStatus = ClearSubmitPresent(enabledLikeDevice, enabledHandle, color);
    const RhiStatus disabledStatus = ClearSubmitPresent(disabledLikeDevice, disabledHandle, color);
    if (enabledStatus != disabledStatus) {
        return Fail("disabled diagnostics fixture changed status");
    }

    std::vector<std::uint8_t> enabledCapture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    std::vector<std::uint8_t> disabledCapture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    enabledLikeDevice.CapturePresentedTarget(std::span<std::uint8_t>(enabledCapture.data(), enabledCapture.size()));
    disabledLikeDevice.CapturePresentedTarget(std::span<std::uint8_t>(disabledCapture.data(), disabledCapture.size()));
    if (enabledCapture != disabledCapture) {
        return Fail("disabled diagnostics fixture changed capture bytes");
    }

    if (enabledLikeDevice.Snapshot().present_count != disabledLikeDevice.Snapshot().present_count) {
        return Fail("disabled diagnostics fixture changed present count");
    }

    return 0;
}

int RhiNoResourceFileUploadShaderUiDependency() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail("minimal rhi target path failed");
    }

    if (ClearSubmitPresent(device, handle, RhiColor{0U, 0U, 0U, 255U}) != RhiStatus::Success) {
        return Fail("minimal rhi frame path failed");
    }

    if (device.Capabilities().backend_kind != RhiBackendKind::Null) {
        return Fail("rhi fixture left null backend scope");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> testRegistry{
        {TEST_CREATE_DEVICE, RhiCreateNullDeviceReturnsCapabilities},
        {TEST_UNSUPPORTED_BACKEND, RhiCreateDeviceRejectsUnsupportedBackend},
        {TEST_CREATE_TARGET, RhiCreateTargetReturnsGenerationHandle},
        {TEST_CREATE_COLOR_TARGET, RhiCreateTargetReturnsGenerationHandle},
        {TEST_INVALID_DESCRIPTOR, RhiCreateColorTargetRejectsInvalidDescriptor},
        {TEST_TARGET_CAPACITY, RhiTargetCapacityOverflowDoesNotMutate},
        {TEST_DESTROY_STALE, RhiDestroyTargetInvalidatesStaleHandle},
        {TEST_REINITIALIZE_STALE_TARGET, RhiReinitializeInvalidatesPriorTargetHandle},
        {TEST_RECORD_CLEAR, RhiCommandListRecordsClearWithinCapacity},
        {TEST_COMMAND_CAPACITY, RhiCommandListCapacityOverflowDoesNotMutate},
        {TEST_SUBMIT_OVERSIZE_COMMAND_LIST, RhiSubmitRejectsOversizedCommandListWithoutMutation},
        {TEST_INVALID_CLEAR_TARGET, RhiRecordClearRejectsInvalidTargetHandle},
        {TEST_MISMATCHED_SUBMIT_TARGET, RhiSubmitRejectsMismatchedRecordedTargetWithoutMutation},
        {TEST_STALE_SUBMIT_TARGET, RhiSubmitRejectsStaleRecordedTargetWithoutMutation},
        {TEST_INCOMPLETE_SUBMIT, RhiSubmitRejectsIncompleteCommandListWithoutMutation},
        {TEST_SUBMIT_EXECUTES_CLEAR, RhiSubmitExecutesClearIntoNullTarget},
        {TEST_PRESENT_REQUIRES_SUBMIT, RhiPresentRequiresSuccessfulSubmit},
        {TEST_PRESENT_DESTROYED_SUBMITTED_TARGET, RhiPresentRejectsDestroyedSubmittedTargetWithoutMutation},
        {TEST_PRESENT_COUNTER, RhiClearSubmitPresentUpdatesPresentedCounter},
        {TEST_CAPTURE_BEFORE_PRESENT, RhiCaptureBeforePresentReturnsExplicitStatus},
        {TEST_CLEAR_COLOR, RhiClearColorUsesExactRgba8ByteChannels},
        {TEST_CAPTURE_DETERMINISTIC, RhiCapturePresentedTargetWritesDeterministicRgba8Bytes},
        {TEST_CAPTURE_DESTROYED_PRESENTED_TARGET, RhiCaptureRejectsDestroyedPresentedTargetWithoutMutation},
        {TEST_UNDERSIZED_CAPTURE, RhiCaptureRejectsUndersizedBufferWithoutWritingBytes},
        {TEST_OVERSIZED_CAPTURE_FIXTURE, RhiCaptureRejectsTargetLargerThanFixtureCapWithoutWritingBytes},
        {TEST_FRAME_NO_GROW, RhiFrameSubmitPresentCaptureDoesNotGrowCommandStorage},
        {TEST_DISABLED_DIAGNOSTICS, RhiDisabledDiagnosticsDoesNotChangeResults},
        {TEST_NO_FORBIDDEN_DEPENDENCY, RhiNoResourceFileUploadShaderUiDependency}};

    const std::string_view testName(argv[1]);
    const auto testIterator = testRegistry.find(testName);
    if (testIterator == testRegistry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return testIterator->second();
}
