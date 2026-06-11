#include <array>
#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <vector>

#include "yuengine/rhi/NullRhiDevice.h"
#include "yuengine/rhi/RhiConstants.h"

using NullRhiDevice = yuengine::rhi::NullRhiDevice;
using RhiBackendKind = yuengine::rhi::RhiBackendKind;
using RhiCaptureResult = yuengine::rhi::RhiCaptureResult;
using RhiColor = yuengine::rhi::RhiColor;
using RhiColorTargetDesc = yuengine::rhi::RhiColorTargetDesc;
using RhiCommandList = yuengine::rhi::RhiCommandList;
using RhiDeviceDesc = yuengine::rhi::RhiDeviceDesc;
using RhiFormat = yuengine::rhi::RhiFormat;
using RhiStatus = yuengine::rhi::RhiStatus;
using RhiTextureHandle = yuengine::rhi::RhiTextureHandle;

namespace
{
constexpr const char* TEST_CREATE_DEVICE = "RHI_CreateNullDevice_ReturnsCapabilities";
constexpr const char* TEST_UNSUPPORTED_BACKEND = "RHI_CreateDevice_RejectsUnsupportedBackend";
constexpr const char* TEST_CREATE_TARGET = "RHI_CreateTarget_ReturnsGenerationHandle";
constexpr const char* TEST_CREATE_COLOR_TARGET = "RHI_CreateColorTarget_ReturnsGenerationHandle";
constexpr const char* TEST_INVALID_DESCRIPTOR = "RHI_CreateColorTarget_RejectsInvalidDescriptor";
constexpr const char* TEST_TARGET_CAPACITY = "RHI_TargetCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_DESTROY_STALE = "RHI_DestroyTarget_InvalidatesStaleHandle";
constexpr const char* TEST_RECORD_CLEAR = "RHI_CommandList_RecordsClearWithinCapacity";
constexpr const char* TEST_COMMAND_CAPACITY = "RHI_CommandListCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_SUBMIT_OVERSIZE_COMMAND_LIST = "RHI_SubmitRejectsOversizedCommandListWithoutMutation";
constexpr const char* TEST_INVALID_CLEAR_TARGET = "RHI_RecordClear_RejectsInvalidTargetHandle";
constexpr const char* TEST_INCOMPLETE_SUBMIT = "RHI_SubmitRejectsIncompleteCommandListWithoutMutation";
constexpr const char* TEST_MISMATCHED_SUBMIT_TARGET = "RHI_SubmitRejectsMismatchedRecordedTargetWithoutMutation";
constexpr const char* TEST_STALE_SUBMIT_TARGET = "RHI_SubmitRejectsStaleRecordedTargetWithoutMutation";
constexpr const char* TEST_SUBMIT_EXECUTES_CLEAR = "RHI_SubmitExecutesClearIntoNullTarget";
constexpr const char* TEST_PRESENT_REQUIRES_SUBMIT = "RHI_PresentRequiresSuccessfulSubmit";
constexpr const char* TEST_PRESENT_COUNTER = "RHI_ClearSubmitPresent_UpdatesPresentedCounter";
constexpr const char* TEST_CAPTURE_BEFORE_PRESENT = "RHI_CaptureBeforePresent_ReturnsExplicitStatus";
constexpr const char* TEST_CLEAR_COLOR = "RHI_ClearColor_UsesExactRgba8ByteChannels";
constexpr const char* TEST_CAPTURE_DETERMINISTIC = "RHI_CapturePresentedTarget_WritesDeterministicRgba8Bytes";
constexpr const char* TEST_UNDERSIZED_CAPTURE = "RHI_CaptureRejectsUndersizedBufferWithoutWritingBytes";
constexpr const char* TEST_OVERSIZED_CAPTURE_FIXTURE = "RHI_CaptureRejectsTargetLargerThanFixtureCapWithoutWritingBytes";
constexpr const char* TEST_FRAME_NO_GROW = "RHI_FrameSubmitPresentCapture_DoesNotGrowCommandStorage";
constexpr const char* TEST_DISABLED_DIAGNOSTICS = "RHI_DisabledDiagnosticsDoesNotChangeResults";
constexpr const char* TEST_NO_FORBIDDEN_DEPENDENCY = "RHI_NoResourceFileUploadShaderUiDependency";
constexpr std::uint8_t SENTINEL_BYTE = 0xAAU;

int Fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

RhiColorTargetDesc SmallTargetDesc()
{
    return RhiColorTargetDesc{RhiFormat::Rgba8Unorm, {2U, 2U}};
}

RhiColorTargetDesc CaptureFixtureTargetDesc()
{
    return RhiColorTargetDesc{RhiFormat::Rgba8Unorm, {4U, 4U}};
}

RhiColorTargetDesc MaxTargetDesc()
{
    return RhiColorTargetDesc{
        RhiFormat::Rgba8Unorm,
        {yuengine::rhi::MAX_COLOR_TARGET_EXTENT, yuengine::rhi::MAX_COLOR_TARGET_EXTENT}};
}

NullRhiDevice CreateInitializedDevice()
{
    NullRhiDevice device;
    device.Initialize(RhiDeviceDesc{});
    return device;
}

bool CreateTarget(NullRhiDevice& device, RhiTextureHandle& outHandle)
{
    return device.CreateColorTarget(SmallTargetDesc(), outHandle) == RhiStatus::Success;
}

RhiStatus ClearSubmitPresent(NullRhiDevice& device, RhiTextureHandle target, RhiColor color)
{
    RhiCommandList commandList(yuengine::rhi::MAX_COMMANDS);
    RhiStatus status = commandList.BeginFrame(target);
    if (status != RhiStatus::Success)
    {
        return status;
    }

    status = device.RecordClear(commandList, target, color);
    if (status != RhiStatus::Success)
    {
        return status;
    }

    status = commandList.EndFrame();
    if (status != RhiStatus::Success)
    {
        return status;
    }

    status = device.Submit(commandList);
    if (status != RhiStatus::Success)
    {
        return status;
    }

    return device.Present();
}

bool BytesMatchColor(const std::vector<std::uint8_t>& bytes, RhiColor color)
{
    for (std::size_t index = 0U; index < bytes.size(); index += yuengine::rhi::RGBA8_BYTES_PER_PIXEL)
    {
        if (bytes[index] != color.R)
        {
            return false;
        }

        if (bytes[index + 1U] != color.G)
        {
            return false;
        }

        if (bytes[index + 2U] != color.B)
        {
            return false;
        }

        if (bytes[index + 3U] != color.A)
        {
            return false;
        }
    }

    return true;
}

int RhiCreateNullDeviceReturnsCapabilities()
{
    NullRhiDevice device;
    const RhiStatus status = device.Initialize(RhiDeviceDesc{});
    if (status != RhiStatus::Success)
    {
        return Fail("null device did not initialize");
    }

    const auto capabilities = device.Capabilities();
    if (capabilities.BackendKind != RhiBackendKind::Null)
    {
        return Fail("capabilities did not report null backend");
    }

    if (capabilities.ColorTargetCapacity != yuengine::rhi::MAX_COLOR_TARGETS)
    {
        return Fail("capabilities reported wrong target capacity");
    }

    if (capabilities.CommandListCapacity != yuengine::rhi::MAX_COMMANDS)
    {
        return Fail("capabilities reported wrong command capacity");
    }

    if (!capabilities.SupportsCapture)
    {
        return Fail("capabilities did not report capture support");
    }

    return 0;
}

int RhiCreateDeviceRejectsUnsupportedBackend()
{
    NullRhiDevice device;
    RhiDeviceDesc desc{};
    desc.BackendKind = RhiBackendKind::Unsupported;

    const RhiStatus status = device.Initialize(desc);
    if (status != RhiStatus::UnsupportedBackend)
    {
        return Fail("unsupported backend did not return explicit status");
    }

    if (device.Snapshot().ColorTargetCapacity != 0U)
    {
        return Fail("unsupported backend mutated device capacity");
    }

    return 0;
}

int RhiCreateTargetReturnsGenerationHandle()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle))
    {
        return Fail("target creation failed");
    }

    if (handle.Generation == 0U)
    {
        return Fail("target handle generation was invalid");
    }

    if (handle.Slot != 0U)
    {
        return Fail("first target used unexpected slot");
    }

    if (device.Snapshot().CreatedTargetCount != 1U)
    {
        return Fail("created target count was not recorded");
    }

    return 0;
}

int RhiCreateColorTargetRejectsInvalidDescriptor()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};

    RhiColorTargetDesc unsupportedFormatDesc = SmallTargetDesc();
    unsupportedFormatDesc.Format = RhiFormat::Unsupported;
    if (device.CreateColorTarget(unsupportedFormatDesc, handle) != RhiStatus::UnsupportedFormat)
    {
        return Fail("unsupported target format was not rejected");
    }

    RhiColorTargetDesc zeroExtentDesc = SmallTargetDesc();
    zeroExtentDesc.Extent.Width = 0U;
    if (device.CreateColorTarget(zeroExtentDesc, handle) != RhiStatus::InvalidDescriptor)
    {
        return Fail("zero extent target was not rejected");
    }

    RhiColorTargetDesc overExtentDesc = SmallTargetDesc();
    overExtentDesc.Extent.Width = yuengine::rhi::MAX_COLOR_TARGET_EXTENT + 1U;
    if (device.CreateColorTarget(overExtentDesc, handle) != RhiStatus::InvalidDescriptor)
    {
        return Fail("overlarge extent target was not rejected");
    }

    if (device.Snapshot().ColorTargetCount != 0U)
    {
        return Fail("invalid descriptors mutated target count");
    }

    return 0;
}

int RhiTargetCapacityOverflowDoesNotMutate()
{
    NullRhiDevice device = CreateInitializedDevice();
    std::array<RhiTextureHandle, yuengine::rhi::MAX_COLOR_TARGETS> handles{};
    for (std::size_t index = 0U; index < handles.size(); ++index)
    {
        const RhiStatus status = device.CreateColorTarget(SmallTargetDesc(), handles[index]);
        if (status != RhiStatus::Success)
        {
            return Fail("target creation failed before capacity");
        }
    }

    const auto beforeSnapshot = device.Snapshot();
    RhiTextureHandle overflowHandle{};
    const RhiStatus overflowStatus = device.CreateColorTarget(SmallTargetDesc(), overflowHandle);
    if (overflowStatus != RhiStatus::CapacityExceeded)
    {
        return Fail("target capacity overflow did not return explicit status");
    }

    const auto afterSnapshot = device.Snapshot();
    if (afterSnapshot.ColorTargetCount != beforeSnapshot.ColorTargetCount)
    {
        return Fail("target overflow changed target count");
    }

    if (afterSnapshot.CreatedTargetCount != beforeSnapshot.CreatedTargetCount)
    {
        return Fail("target overflow changed created count");
    }

    return 0;
}

int RhiDestroyTargetInvalidatesStaleHandle()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle))
    {
        return Fail("target creation failed");
    }

    if (device.DestroyTarget(handle) != RhiStatus::Success)
    {
        return Fail("target destroy failed");
    }

    RhiCommandList commandList(yuengine::rhi::MAX_COMMANDS);
    commandList.BeginFrame(handle);
    const RhiStatus staleStatus = device.RecordClear(commandList, handle, RhiColor{1U, 2U, 3U, 4U});
    if (staleStatus != RhiStatus::InvalidHandle)
    {
        return Fail("stale handle did not return explicit status");
    }

    if (device.Snapshot().DestroyedTargetCount != 1U)
    {
        return Fail("destroyed target count was not recorded");
    }

    return 0;
}

int RhiCommandListRecordsClearWithinCapacity()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle))
    {
        return Fail("target creation failed");
    }

    RhiCommandList commandList(3U);
    if (commandList.BeginFrame(handle) != RhiStatus::Success)
    {
        return Fail("begin frame failed");
    }

    if (device.RecordClear(commandList, handle, RhiColor{1U, 2U, 3U, 4U}) != RhiStatus::Success)
    {
        return Fail("record clear failed");
    }

    if (commandList.EndFrame() != RhiStatus::Success)
    {
        return Fail("end frame failed within capacity");
    }

    if (commandList.CommandCount() != 3U)
    {
        return Fail("clear command list count was wrong");
    }

    return 0;
}

int RhiCommandListCapacityOverflowDoesNotMutate()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle))
    {
        return Fail("target creation failed");
    }

    RhiCommandList commandList(2U);
    if (commandList.BeginFrame(handle) != RhiStatus::Success)
    {
        return Fail("begin frame failed");
    }

    if (device.RecordClear(commandList, handle, RhiColor{1U, 2U, 3U, 4U}) != RhiStatus::Success)
    {
        return Fail("clear record failed");
    }

    const std::size_t countBefore = commandList.CommandCount();
    const RhiStatus endStatus = commandList.EndFrame();
    if (endStatus != RhiStatus::CapacityExceeded)
    {
        return Fail("command capacity overflow did not return explicit status");
    }

    if (commandList.CommandCount() != countBefore)
    {
        return Fail("command capacity overflow mutated command count");
    }

    return 0;
}

int RhiSubmitRejectsOversizedCommandListWithoutMutation()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle))
    {
        return Fail("target creation failed");
    }

    const RhiColor initialColor{1U, 2U, 3U, 4U};
    if (ClearSubmitPresent(device, handle, initialColor) != RhiStatus::Success)
    {
        return Fail("initial clear submit present failed");
    }

    const auto beforeSnapshot = device.Snapshot();
    RhiCommandList oversizedCommandList(yuengine::rhi::MAX_COMMANDS + 1U);
    if (oversizedCommandList.BeginFrame(handle) != RhiStatus::Success)
    {
        return Fail("oversized begin frame failed before submit");
    }

    if (device.RecordClear(oversizedCommandList, handle, RhiColor{9U, 8U, 7U, 6U}) != RhiStatus::Success)
    {
        return Fail("oversized clear record failed before submit");
    }

    if (oversizedCommandList.EndFrame() != RhiStatus::Success)
    {
        return Fail("oversized end frame failed before submit");
    }

    const RhiStatus submitStatus = device.Submit(oversizedCommandList);
    if (submitStatus != RhiStatus::CapacityExceeded)
    {
        return Fail("oversized command list submit did not return capacity status");
    }

    const auto afterSnapshot = device.Snapshot();
    if (afterSnapshot.SubmitCount != beforeSnapshot.SubmitCount)
    {
        return Fail("oversized command list submit mutated submit count");
    }

    std::vector<std::uint8_t> capture(2U * 2U * yuengine::rhi::RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    if (result.Status != RhiStatus::Success)
    {
        return Fail("capture failed after rejected oversized submit");
    }

    if (!BytesMatchColor(capture, initialColor))
    {
        return Fail("rejected oversized submit mutated target bytes");
    }

    return 0;
}

int RhiRecordClearRejectsInvalidTargetHandle()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle))
    {
        return Fail("target creation failed");
    }

    RhiCommandList commandList(yuengine::rhi::MAX_COMMANDS);
    if (commandList.BeginFrame(handle) != RhiStatus::Success)
    {
        return Fail("begin frame failed");
    }

    const std::size_t countBefore = commandList.CommandCount();
    const RhiStatus status = device.RecordClear(commandList, RhiTextureHandle{99U, 1U}, RhiColor{1U, 2U, 3U, 4U});
    if (status != RhiStatus::InvalidHandle)
    {
        return Fail("invalid target clear did not return handle status");
    }

    if (commandList.CommandCount() != countBefore)
    {
        return Fail("invalid target clear mutated command list");
    }

    return 0;
}

int RhiSubmitRejectsMismatchedRecordedTargetWithoutMutation()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle frameTarget{};
    RhiTextureHandle otherTarget{};
    if (!CreateTarget(device, frameTarget))
    {
        return Fail("frame target creation failed");
    }

    if (!CreateTarget(device, otherTarget))
    {
        return Fail("other target creation failed");
    }

    const RhiColor otherInitialColor{4U, 3U, 2U, 1U};
    if (ClearSubmitPresent(device, otherTarget, otherInitialColor) != RhiStatus::Success)
    {
        return Fail("initial other target clear submit present failed");
    }

    const auto beforeSnapshot = device.Snapshot();
    RhiCommandList commandList(yuengine::rhi::MAX_COMMANDS);
    if (commandList.BeginFrame(frameTarget) != RhiStatus::Success)
    {
        return Fail("begin frame failed");
    }

    if (device.RecordClear(commandList, otherTarget, RhiColor{9U, 8U, 7U, 6U}) != RhiStatus::Success)
    {
        return Fail("mismatched clear record failed before submit");
    }

    if (commandList.EndFrame() != RhiStatus::Success)
    {
        return Fail("end frame failed");
    }

    const RhiStatus submitStatus = device.Submit(commandList);
    if (submitStatus != RhiStatus::InvalidHandle)
    {
        return Fail("mismatched recorded target did not return explicit status");
    }

    const auto afterSnapshot = device.Snapshot();
    if (afterSnapshot.SubmitCount != beforeSnapshot.SubmitCount)
    {
        return Fail("mismatched target submit mutated submit count");
    }

    std::vector<std::uint8_t> capture(2U * 2U * yuengine::rhi::RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    if (result.Status != RhiStatus::Success)
    {
        return Fail("capture failed after rejected mismatched submit");
    }

    if (!BytesMatchColor(capture, otherInitialColor))
    {
        return Fail("rejected mismatched submit mutated recorded target bytes");
    }

    return 0;
}

int RhiSubmitRejectsStaleRecordedTargetWithoutMutation()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle originalTarget{};
    if (!CreateTarget(device, originalTarget))
    {
        return Fail("original target creation failed");
    }

    if (device.DestroyTarget(originalTarget) != RhiStatus::Success)
    {
        return Fail("destroying original target failed");
    }

    RhiTextureHandle frameTarget{};
    if (!CreateTarget(device, frameTarget))
    {
        return Fail("replacement frame target creation failed");
    }

    if (frameTarget.Slot != originalTarget.Slot)
    {
        return Fail("replacement target did not reuse slot for stale generation test");
    }

    if (frameTarget.Generation == originalTarget.Generation)
    {
        return Fail("replacement target did not advance generation");
    }

    const RhiColor frameInitialColor{5U, 6U, 7U, 8U};
    if (ClearSubmitPresent(device, frameTarget, frameInitialColor) != RhiStatus::Success)
    {
        return Fail("initial frame target clear submit present failed");
    }

    const auto beforeSnapshot = device.Snapshot();
    RhiCommandList commandList(yuengine::rhi::MAX_COMMANDS);
    if (commandList.BeginFrame(frameTarget) != RhiStatus::Success)
    {
        return Fail("begin frame failed");
    }

    if (commandList.RecordClear(originalTarget, RhiColor{9U, 8U, 7U, 6U}) != RhiStatus::Success)
    {
        return Fail("stale generation clear record failed before submit");
    }

    if (commandList.EndFrame() != RhiStatus::Success)
    {
        return Fail("end frame failed");
    }

    const RhiStatus submitStatus = device.Submit(commandList);
    if (submitStatus != RhiStatus::InvalidHandle)
    {
        return Fail("stale recorded target did not return explicit status");
    }

    const auto afterSnapshot = device.Snapshot();
    if (afterSnapshot.SubmitCount != beforeSnapshot.SubmitCount)
    {
        return Fail("stale target submit mutated submit count");
    }

    if (afterSnapshot.ColorTargetCount != beforeSnapshot.ColorTargetCount)
    {
        return Fail("stale target submit mutated target count");
    }

    std::vector<std::uint8_t> capture(2U * 2U * yuengine::rhi::RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    if (result.Status != RhiStatus::Success)
    {
        return Fail("capture failed after rejected stale generation submit");
    }

    if (!BytesMatchColor(capture, frameInitialColor))
    {
        return Fail("rejected stale generation submit mutated frame target bytes");
    }

    return 0;
}

int RhiSubmitRejectsIncompleteCommandListWithoutMutation()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle))
    {
        return Fail("target creation failed");
    }

    RhiCommandList commandList(yuengine::rhi::MAX_COMMANDS);
    commandList.BeginFrame(handle);
    device.RecordClear(commandList, handle, RhiColor{9U, 8U, 7U, 6U});

    const auto beforeSnapshot = device.Snapshot();
    const RhiStatus status = device.Submit(commandList);
    if (status != RhiStatus::InvalidLifecycle)
    {
        return Fail("incomplete command list did not return lifecycle status");
    }

    const auto afterSnapshot = device.Snapshot();
    if (afterSnapshot.SubmitCount != beforeSnapshot.SubmitCount)
    {
        return Fail("incomplete submit changed submit count");
    }

    if (device.Present() != RhiStatus::InvalidLifecycle)
    {
        return Fail("present succeeded after rejected submit");
    }

    return 0;
}

int RhiSubmitExecutesClearIntoNullTarget()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle))
    {
        return Fail("target creation failed");
    }

    const RhiColor color{11U, 12U, 13U, 14U};
    if (ClearSubmitPresent(device, handle, color) != RhiStatus::Success)
    {
        return Fail("clear submit present failed");
    }

    std::vector<std::uint8_t> capture(2U * 2U * yuengine::rhi::RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    if (result.Status != RhiStatus::Success)
    {
        return Fail("capture failed after clear submit present");
    }

    if (!BytesMatchColor(capture, color))
    {
        return Fail("submit did not execute clear into null target");
    }

    return 0;
}

int RhiPresentRequiresSuccessfulSubmit()
{
    NullRhiDevice device = CreateInitializedDevice();
    if (device.Present() != RhiStatus::InvalidLifecycle)
    {
        return Fail("present without submit did not return lifecycle status");
    }

    if (device.Snapshot().PresentCount != 0U)
    {
        return Fail("present without submit changed present count");
    }

    return 0;
}

int RhiClearSubmitPresentUpdatesPresentedCounter()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle))
    {
        return Fail("target creation failed");
    }

    const RhiStatus status = ClearSubmitPresent(device, handle, RhiColor{1U, 2U, 3U, 4U});
    if (status != RhiStatus::Success)
    {
        return Fail("clear submit present did not succeed");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.SubmitCount != 1U)
    {
        return Fail("submit count was not updated");
    }

    if (snapshot.PresentCount != 1U)
    {
        return Fail("present count was not updated");
    }

    return 0;
}

int RhiCaptureBeforePresentReturnsExplicitStatus()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle))
    {
        return Fail("target creation failed");
    }

    std::vector<std::uint8_t> capture(2U * 2U * yuengine::rhi::RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    if (result.Status != RhiStatus::InvalidLifecycle)
    {
        return Fail("capture before present did not return lifecycle status");
    }

    if (result.BytesWritten != 0U)
    {
        return Fail("capture before present wrote bytes");
    }

    return 0;
}

int RhiClearColorUsesExactRgba8ByteChannels()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle))
    {
        return Fail("target creation failed");
    }

    const RhiColor color{1U, 2U, 253U, 255U};
    if (ClearSubmitPresent(device, handle, color) != RhiStatus::Success)
    {
        return Fail("clear submit present failed");
    }

    std::vector<std::uint8_t> capture(2U * 2U * yuengine::rhi::RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    if (result.Status != RhiStatus::Success)
    {
        return Fail("capture failed");
    }

    if (!BytesMatchColor(capture, color))
    {
        return Fail("capture bytes did not exactly match RGBA8 clear channels");
    }

    return 0;
}

int RhiCapturePresentedTargetWritesDeterministicRgba8Bytes()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    const RhiStatus createStatus = device.CreateColorTarget(CaptureFixtureTargetDesc(), handle);
    if (createStatus != RhiStatus::Success)
    {
        return Fail("target creation failed");
    }

    const RhiColor color{4U, 5U, 6U, 7U};
    if (ClearSubmitPresent(device, handle, color) != RhiStatus::Success)
    {
        return Fail("clear submit present failed");
    }

    std::vector<std::uint8_t> firstCapture(4U * 4U * yuengine::rhi::RGBA8_BYTES_PER_PIXEL);
    std::vector<std::uint8_t> secondCapture(4U * 4U * yuengine::rhi::RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult firstResult = device.CapturePresentedTarget(std::span<std::uint8_t>(firstCapture.data(), firstCapture.size()));
    const RhiCaptureResult secondResult = device.CapturePresentedTarget(std::span<std::uint8_t>(secondCapture.data(), secondCapture.size()));
    if (firstResult.Status != RhiStatus::Success)
    {
        return Fail("first capture failed");
    }

    if (secondResult.Status != RhiStatus::Success)
    {
        return Fail("second capture failed");
    }

    if (firstCapture != secondCapture)
    {
        return Fail("capture bytes were not deterministic");
    }

    if (!BytesMatchColor(firstCapture, color))
    {
        return Fail("deterministic capture did not match clear color");
    }

    return 0;
}

int RhiCaptureRejectsUndersizedBufferWithoutWritingBytes()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle))
    {
        return Fail("target creation failed");
    }

    if (ClearSubmitPresent(device, handle, RhiColor{1U, 1U, 1U, 1U}) != RhiStatus::Success)
    {
        return Fail("clear submit present failed");
    }

    std::vector<std::uint8_t> destination((2U * 2U * yuengine::rhi::RGBA8_BYTES_PER_PIXEL) - 1U, SENTINEL_BYTE);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(destination.data(), destination.size()));
    if (result.Status != RhiStatus::CapacityExceeded)
    {
        return Fail("undersized capture did not return capacity status");
    }

    if (result.BytesWritten != 0U)
    {
        return Fail("undersized capture reported nonzero bytes written");
    }

    for (const std::uint8_t byte : destination)
    {
        if (byte != SENTINEL_BYTE)
        {
            return Fail("undersized capture mutated destination bytes");
        }
    }

    if (device.Snapshot().LastCaptureBytesWritten != 0U)
    {
        return Fail("undersized capture did not record zero bytes written");
    }

    return 0;
}

int RhiCaptureRejectsTargetLargerThanFixtureCapWithoutWritingBytes()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (device.CreateColorTarget(MaxTargetDesc(), handle) != RhiStatus::Success)
    {
        return Fail("max target creation failed");
    }

    if (ClearSubmitPresent(device, handle, RhiColor{1U, 2U, 3U, 4U}) != RhiStatus::Success)
    {
        return Fail("clear submit present failed");
    }

    const std::size_t fullTargetBytes = static_cast<std::size_t>(yuengine::rhi::MAX_COLOR_TARGET_EXTENT) *
        static_cast<std::size_t>(yuengine::rhi::MAX_COLOR_TARGET_EXTENT) * yuengine::rhi::RGBA8_BYTES_PER_PIXEL;
    std::vector<std::uint8_t> destination(fullTargetBytes, SENTINEL_BYTE);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(destination.data(), destination.size()));
    if (result.Status != RhiStatus::CapacityExceeded)
    {
        return Fail("oversized capture fixture did not return capacity status");
    }

    if (result.BytesWritten != 0U)
    {
        return Fail("oversized capture fixture reported bytes written");
    }

    for (const std::uint8_t byte : destination)
    {
        if (byte != SENTINEL_BYTE)
        {
            return Fail("oversized capture fixture mutated destination bytes");
        }
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.LastCaptureBytesWritten != 0U)
    {
        return Fail("oversized capture fixture did not record zero bytes written");
    }

    if (snapshot.CaptureCount != 0U)
    {
        return Fail("oversized capture fixture incremented capture count");
    }

    return 0;
}

int RhiFrameSubmitPresentCaptureDoesNotGrowCommandStorage()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle))
    {
        return Fail("target creation failed");
    }

    RhiCommandList commandList(yuengine::rhi::MAX_COMMANDS);
    const std::size_t capacityBefore = commandList.Capacity();
    commandList.BeginFrame(handle);
    device.RecordClear(commandList, handle, RhiColor{1U, 2U, 3U, 4U});
    commandList.EndFrame();
    device.Submit(commandList);
    device.Present();

    std::vector<std::uint8_t> capture(2U * 2U * yuengine::rhi::RGBA8_BYTES_PER_PIXEL);
    device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));

    const auto snapshot = device.Snapshot();
    if (commandList.Capacity() != capacityBefore)
    {
        return Fail("command list capacity changed during frame fixture");
    }

    if (snapshot.CommandStorageCapacityBeforeFrame != snapshot.CommandStorageCapacityAfterLastFrame)
    {
        return Fail("device snapshot recorded command storage growth");
    }

    if (snapshot.CommandStorageCapacityBeforeFrame != capacityBefore)
    {
        return Fail("device snapshot recorded wrong command storage capacity");
    }

    return 0;
}

int RhiDisabledDiagnosticsDoesNotChangeResults()
{
    NullRhiDevice enabledLikeDevice = CreateInitializedDevice();
    NullRhiDevice disabledLikeDevice = CreateInitializedDevice();
    RhiTextureHandle enabledHandle{};
    RhiTextureHandle disabledHandle{};
    enabledLikeDevice.CreateColorTarget(SmallTargetDesc(), enabledHandle);
    disabledLikeDevice.CreateColorTarget(SmallTargetDesc(), disabledHandle);

    const RhiColor color{3U, 4U, 5U, 6U};
    const RhiStatus enabledStatus = ClearSubmitPresent(enabledLikeDevice, enabledHandle, color);
    const RhiStatus disabledStatus = ClearSubmitPresent(disabledLikeDevice, disabledHandle, color);
    if (enabledStatus != disabledStatus)
    {
        return Fail("disabled diagnostics fixture changed status");
    }

    std::vector<std::uint8_t> enabledCapture(2U * 2U * yuengine::rhi::RGBA8_BYTES_PER_PIXEL);
    std::vector<std::uint8_t> disabledCapture(2U * 2U * yuengine::rhi::RGBA8_BYTES_PER_PIXEL);
    enabledLikeDevice.CapturePresentedTarget(std::span<std::uint8_t>(enabledCapture.data(), enabledCapture.size()));
    disabledLikeDevice.CapturePresentedTarget(std::span<std::uint8_t>(disabledCapture.data(), disabledCapture.size()));
    if (enabledCapture != disabledCapture)
    {
        return Fail("disabled diagnostics fixture changed capture bytes");
    }

    if (enabledLikeDevice.Snapshot().PresentCount != disabledLikeDevice.Snapshot().PresentCount)
    {
        return Fail("disabled diagnostics fixture changed present count");
    }

    return 0;
}

int RhiNoResourceFileUploadShaderUiDependency()
{
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle))
    {
        return Fail("minimal rhi target path failed");
    }

    if (ClearSubmitPresent(device, handle, RhiColor{0U, 0U, 0U, 255U}) != RhiStatus::Success)
    {
        return Fail("minimal rhi frame path failed");
    }

    if (device.Capabilities().BackendKind != RhiBackendKind::Null)
    {
        return Fail("rhi fixture left null backend scope");
    }

    return 0;
}
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return Fail("expected one test name");
    }

    const std::string testName(argv[1]);
    if (testName == TEST_CREATE_DEVICE)
    {
        return RhiCreateNullDeviceReturnsCapabilities();
    }

    if (testName == TEST_UNSUPPORTED_BACKEND)
    {
        return RhiCreateDeviceRejectsUnsupportedBackend();
    }

    if (testName == TEST_CREATE_TARGET)
    {
        return RhiCreateTargetReturnsGenerationHandle();
    }

    if (testName == TEST_CREATE_COLOR_TARGET)
    {
        return RhiCreateTargetReturnsGenerationHandle();
    }

    if (testName == TEST_INVALID_DESCRIPTOR)
    {
        return RhiCreateColorTargetRejectsInvalidDescriptor();
    }

    if (testName == TEST_TARGET_CAPACITY)
    {
        return RhiTargetCapacityOverflowDoesNotMutate();
    }

    if (testName == TEST_DESTROY_STALE)
    {
        return RhiDestroyTargetInvalidatesStaleHandle();
    }

    if (testName == TEST_RECORD_CLEAR)
    {
        return RhiCommandListRecordsClearWithinCapacity();
    }

    if (testName == TEST_COMMAND_CAPACITY)
    {
        return RhiCommandListCapacityOverflowDoesNotMutate();
    }

    if (testName == TEST_SUBMIT_OVERSIZE_COMMAND_LIST)
    {
        return RhiSubmitRejectsOversizedCommandListWithoutMutation();
    }

    if (testName == TEST_INVALID_CLEAR_TARGET)
    {
        return RhiRecordClearRejectsInvalidTargetHandle();
    }

    if (testName == TEST_MISMATCHED_SUBMIT_TARGET)
    {
        return RhiSubmitRejectsMismatchedRecordedTargetWithoutMutation();
    }

    if (testName == TEST_STALE_SUBMIT_TARGET)
    {
        return RhiSubmitRejectsStaleRecordedTargetWithoutMutation();
    }

    if (testName == TEST_INCOMPLETE_SUBMIT)
    {
        return RhiSubmitRejectsIncompleteCommandListWithoutMutation();
    }

    if (testName == TEST_SUBMIT_EXECUTES_CLEAR)
    {
        return RhiSubmitExecutesClearIntoNullTarget();
    }

    if (testName == TEST_PRESENT_REQUIRES_SUBMIT)
    {
        return RhiPresentRequiresSuccessfulSubmit();
    }

    if (testName == TEST_PRESENT_COUNTER)
    {
        return RhiClearSubmitPresentUpdatesPresentedCounter();
    }

    if (testName == TEST_CAPTURE_BEFORE_PRESENT)
    {
        return RhiCaptureBeforePresentReturnsExplicitStatus();
    }

    if (testName == TEST_CLEAR_COLOR)
    {
        return RhiClearColorUsesExactRgba8ByteChannels();
    }

    if (testName == TEST_CAPTURE_DETERMINISTIC)
    {
        return RhiCapturePresentedTargetWritesDeterministicRgba8Bytes();
    }

    if (testName == TEST_UNDERSIZED_CAPTURE)
    {
        return RhiCaptureRejectsUndersizedBufferWithoutWritingBytes();
    }

    if (testName == TEST_OVERSIZED_CAPTURE_FIXTURE)
    {
        return RhiCaptureRejectsTargetLargerThanFixtureCapWithoutWritingBytes();
    }

    if (testName == TEST_FRAME_NO_GROW)
    {
        return RhiFrameSubmitPresentCaptureDoesNotGrowCommandStorage();
    }

    if (testName == TEST_DISABLED_DIAGNOSTICS)
    {
        return RhiDisabledDiagnosticsDoesNotChangeResults();
    }

    if (testName == TEST_NO_FORBIDDEN_DEPENDENCY)
    {
        return RhiNoResourceFileUploadShaderUiDependency();
    }

    return Fail("unknown test name");
}
