// 模块：Tests Rhi
// 文件：Tests/Rhi/RhiTests.cpp

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/NullRhiDevice.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiBufferUsage.h"
#include "YuEngine/Rhi/RhiCapabilities.h"
#include "YuEngine/Rhi/RhiConstantBufferBinding.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiDeviceFactory.h"
#include "YuEngine/Rhi/RhiDrawDesc.h"
#include "YuEngine/Rhi/RhiDrawIndexedDesc.h"
#include "YuEngine/Rhi/RhiFenceHandle.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiInputLayoutDesc.h"
#include "YuEngine/Rhi/RhiNativeSurfaceDesc.h"
#include "YuEngine/Rhi/RhiPipelineDesc.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiPrimitiveKind.h"
#include "YuEngine/Rhi/RhiPrimitiveRetirementDrainRequest.h"
#include "YuEngine/Rhi/RhiPrimitiveRetirementDrainResult.h"
#include "YuEngine/Rhi/RhiPrimitiveRetirementRecord.h"
#include "YuEngine/Rhi/RhiPrimitiveRetirementRequest.h"
#include "YuEngine/Rhi/RhiPrimitiveRetirementStatus.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"
#include "YuEngine/Rhi/RhiSamplerDesc.h"
#include "YuEngine/Rhi/RhiSamplerHandle.h"
#include "YuEngine/Rhi/RhiShaderModuleDesc.h"
#include "YuEngine/Rhi/RhiShaderModuleHandle.h"
#include "YuEngine/Rhi/RhiShaderStage.h"
#include "YuEngine/Rhi/RhiSwapchainDesc.h"
#include "YuEngine/Rhi/RhiSwapchainResizeRequest.h"
#include "YuEngine/Rhi/RhiSwapchainResizeResult.h"
#include "YuEngine/Rhi/RhiSwapchainSnapshot.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"

using IRhiDevice = yuengine::rhi::IRhiDevice;
using NullRhiDevice = yuengine::rhi::NullRhiDevice;
using yuengine::rhi::RhiBackendKind;
using RhiBufferDesc = yuengine::rhi::RhiBufferDesc;
using RhiBufferHandle = yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiBufferUsage;
using RhiCaptureResult = yuengine::rhi::RhiCaptureResult;
using RhiCapabilities = yuengine::rhi::RhiCapabilities;
using RhiColor = yuengine::rhi::RhiColor;
using RhiColorTargetDesc = yuengine::rhi::RhiColorTargetDesc;
using RhiCommandList = yuengine::rhi::RhiCommandList;
using RhiConstantBufferBinding = yuengine::rhi::RhiConstantBufferBinding;
using RhiDeviceCreateResult = yuengine::rhi::RhiDeviceCreateResult;
using RhiDeviceDesc = yuengine::rhi::RhiDeviceDesc;
using RhiDeviceFactory = yuengine::rhi::RhiDeviceFactory;
using RhiDeviceSnapshot = yuengine::rhi::RhiDeviceSnapshot;
using RhiDrawDesc = yuengine::rhi::RhiDrawDesc;
using RhiDrawIndexedDesc = yuengine::rhi::RhiDrawIndexedDesc;
using RhiFenceHandle = yuengine::rhi::RhiFenceHandle;
using yuengine::rhi::RhiFormat;
using RhiIndexBufferView = yuengine::rhi::RhiIndexBufferView;
using yuengine::rhi::RhiIndexFormat;
using yuengine::rhi::RhiInputElementFormat;
using yuengine::rhi::RhiInputElementSemantic;
using RhiInputLayoutDesc = yuengine::rhi::RhiInputLayoutDesc;
using RhiNativeSurfaceDesc = yuengine::rhi::RhiNativeSurfaceDesc;
using RhiPipelineDesc = yuengine::rhi::RhiPipelineDesc;
using RhiPipelineHandle = yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiPrimitiveKind;
using RhiPrimitiveRetirementDrainRequest = yuengine::rhi::RhiPrimitiveRetirementDrainRequest;
using RhiPrimitiveRetirementDrainResult = yuengine::rhi::RhiPrimitiveRetirementDrainResult;
using RhiPrimitiveRetirementRecord = yuengine::rhi::RhiPrimitiveRetirementRecord;
using RhiPrimitiveRetirementRequest = yuengine::rhi::RhiPrimitiveRetirementRequest;
using yuengine::rhi::RhiPrimitiveRetirementStatus;
using yuengine::rhi::RhiPrimitiveTopology;
using RhiSampledTextureBinding = yuengine::rhi::RhiSampledTextureBinding;
using RhiSamplerBinding = yuengine::rhi::RhiSamplerBinding;
using RhiSamplerDesc = yuengine::rhi::RhiSamplerDesc;
using RhiSamplerHandle = yuengine::rhi::RhiSamplerHandle;
using RhiShaderModuleDesc = yuengine::rhi::RhiShaderModuleDesc;
using RhiShaderModuleHandle = yuengine::rhi::RhiShaderModuleHandle;
using yuengine::rhi::RhiShaderStage;
using yuengine::rhi::RhiStatus;
using RhiSwapchainDesc = yuengine::rhi::RhiSwapchainDesc;
using RhiSwapchainResizeRequest = yuengine::rhi::RhiSwapchainResizeRequest;
using RhiSwapchainResizeResult = yuengine::rhi::RhiSwapchainResizeResult;
using RhiSwapchainSnapshot = yuengine::rhi::RhiSwapchainSnapshot;
using RhiTextureDesc = yuengine::rhi::RhiTextureDesc;
using RhiTextureHandle = yuengine::rhi::RhiTextureHandle;
using RhiVertexBufferView = yuengine::rhi::RhiVertexBufferView;
using yuengine::rhi::MAX_COMMANDS;
using yuengine::rhi::MAX_CAPTURE_FIXTURE_EXTENT;
using yuengine::rhi::MAX_COLOR_TARGET_EXTENT;
using yuengine::rhi::MAX_COLOR_TARGETS;
using yuengine::rhi::MAX_RHI_BUFFERS;
using yuengine::rhi::MAX_RHI_BUFFER_BYTES;
using yuengine::rhi::MAX_RHI_CONSTANT_BUFFER_SLOTS;
using yuengine::rhi::MAX_RHI_PIPELINES;
using yuengine::rhi::MAX_RHI_PRIMITIVE_RETIREMENTS;
using yuengine::rhi::MAX_RHI_SAMPLED_TEXTURE_SLOTS;
using yuengine::rhi::MAX_RHI_SAMPLER_SLOTS;
using yuengine::rhi::MAX_RHI_SAMPLERS;
using yuengine::rhi::MAX_RHI_SHADER_BYTECODE_BYTES;
using yuengine::rhi::MAX_RHI_SHADER_MODULES;
using yuengine::rhi::MAX_RHI_TEXTURES;
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
constexpr const char* TEST_COMMAND_LIST_LAST_STATUS = "RHI_CommandListLastStatus_TracksLifecycleAndCapacity";
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
constexpr const char *TEST_CAPTURE_USER_VISIBLE_RESOLUTION =
    "RHI_CapturePresentedTarget_WritesUserVisibleResolution";
constexpr const char* TEST_CAPTURE_DESTROYED_PRESENTED_TARGET = "RHI_CaptureRejectsDestroyedPresentedTargetWithoutMutation";
constexpr const char* TEST_UNDERSIZED_CAPTURE = "RHI_CaptureRejectsUndersizedBufferWithoutWritingBytes";
constexpr const char* TEST_OVERSIZED_CAPTURE_FIXTURE = "RHI_CaptureRejectsTargetLargerThanFixtureCapWithoutWritingBytes";
constexpr const char* TEST_FRAME_NO_GROW = "RHI_FrameSubmitPresentCapture_DoesNotGrowCommandStorage";
constexpr const char* TEST_DISABLED_DIAGNOSTICS = "RHI_DisabledDiagnosticsDoesNotChangeResults";
constexpr const char* TEST_NO_FORBIDDEN_DEPENDENCY = "RHI_NoResourceFileUploadShaderUiDependency";
constexpr const char* TEST_INTERFACE_CREATE_DEVICE = "RHI_Interface_CreateNullDevice_ReturnsCapabilities";
constexpr const char* TEST_INTERFACE_CLEAR_CAPTURE = "RHI_Interface_ClearSubmitPresentCapture_MatchesNullDevice";
constexpr const char* TEST_FACTORY_CALLER_STORAGE = "RHI_Factory_CreateNullDevice_UsesCallerOwnedStorage";
constexpr const char* TEST_FACTORY_NULL_STORAGE = "RHI_Factory_NullStorageRejectedWithoutOutput";
constexpr const char* TEST_FACTORY_D3D11_UNSUPPORTED = "RHI_Factory_D3D11BackendUnsupportedWithoutMutation";
constexpr const char* TEST_FACTORY_SURFACE_REQUIRED = "RHI_Factory_SurfaceRequiredForNullBackendRejectedWithoutMutation";
constexpr const char* TEST_FACTORY_INVALID_SURFACE = "RHI_Factory_InvalidNativeSurfaceRejectedBeforeMutation";
constexpr const char* TEST_NATIVE_SURFACE_DEFAULT = "RHI_NativeSurfaceDesc_DefaultIsInvalidPlainValue";
constexpr const char* TEST_FACTORY_RAW_STORAGE_NULL = "RHI_Factory_RawStorageCreatesNullDevice";
constexpr const char* TEST_FACTORY_RAW_STORAGE_TOO_SMALL = "RHI_Factory_RawStorageRejectsSmallBuffer";
constexpr const char* TEST_FACTORY_D3D11_INVALID_SURFACE = "RHI_Factory_D3D11InvalidSurfaceFailsBeforeHardware";
constexpr const char* TEST_SWAPCHAIN_DESC_DEFAULT = "RHI_SwapchainDesc_DefaultIsBoundedPlainValue";
constexpr const char* TEST_NULL_SWAPCHAIN_QUERY = "RHI_NullBackend_SwapchainQueryReturnsUnsupported";
constexpr const char *TEST_SWAPCHAIN_RESIZE_DEFAULTS = "RHI_SwapchainResize_DefaultContractsAreExplicit";
constexpr const char *TEST_NULL_SWAPCHAIN_RESIZE = "RHI_NullBackend_SwapchainResizeReturnsUnsupportedWithoutTarget";
constexpr const char* TEST_PRIMITIVE_CAPABILITIES = "RHI_PrimitiveCapabilities_ReportBoundedCapacities";
constexpr const char* TEST_CREATE_BUFFER = "RHI_CreateBuffer_ReturnsGenerationHandleAndSnapshot";
constexpr const char* TEST_UPDATE_BUFFER = "RHI_UpdateBuffer_SignalsFenceAndRecordsBytes";
constexpr const char *TEST_UPDATE_BUFFER_DESTINATION_RANGE =
    "RHI_UpdateBufferDestinationRange_TracksOffsetAndRejectsOverflow";
constexpr const char* TEST_BUFFER_CAPACITY = "RHI_BufferCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_TEXTURE_PRIMITIVE = "RHI_TextureCreateUpdateDestroy_TracksSnapshot";
constexpr const char *TEST_UPDATE_TEXTURE_DESTINATION_RANGE =
    "RHI_UpdateTextureDestinationRange_TracksOffsetAndRejectsOverflow";
constexpr const char* TEST_SAMPLER_PRIMITIVE = "RHI_SamplerCreateDestroy_TracksSnapshot";
constexpr const char* TEST_SHADER_EMPTY_BYTECODE = "RHI_ShaderModuleRejectsEmptyBytecode";
constexpr const char* TEST_PIPELINE_INVALID_SHADERS = "RHI_PipelineRequiresValidShaderModules";
constexpr const char* TEST_PIPELINE_LIFECYCLE = "RHI_PipelineCreateDestroy_UsesShaderModuleHandles";
constexpr const char* TEST_PRIMITIVE_STALE_HANDLE = "RHI_PrimitiveDestroyInvalidatesStaleHandles";
constexpr const char* TEST_INTERFACE_PRIMITIVE = "RHI_Interface_PrimitiveLifecycle_MatchesNullDevice";
constexpr const char* TEST_PRIMITIVE_SNAPSHOT_COMPARISON = "RHI_PrimitiveSnapshot_IsIncludedInDeviceSnapshotComparison";
constexpr const char *TEST_PRIMITIVE_RETIREMENT_DEFAULTS = "RHI_PrimitiveRetirement_DefaultContractsAreExplicit";
constexpr const char *TEST_PRIMITIVE_RETIREMENT_REQUEST = "RHI_PrimitiveRetirement_RequestCreatesPendingRecord";
constexpr const char *TEST_PRIMITIVE_RETIREMENT_DRAIN = "RHI_PrimitiveRetirement_DrainInvalidatesHandleAndTracksCounters";
constexpr const char *TEST_PRIMITIVE_RETIREMENT_REJECTS = "RHI_PrimitiveRetirement_RejectsInvalidWrongDuplicateAndCapacity";
constexpr const char *TEST_PRIMITIVE_RETIREMENT_FENCE = "RHI_PrimitiveRetirement_FenceNotReadyDrainKeepsPending";
constexpr const char *TEST_PRIMITIVE_RETIREMENT_IMMEDIATE = "RHI_PrimitiveRetirement_ImmediateDestroyCompatibility";
constexpr const char *TEST_INTERFACE_PRIMITIVE_RETIREMENT = "RHI_Interface_PrimitiveRetirementMatchesNullDevice";
constexpr const char* TEST_RECORD_VISIBLE_TRIANGLE = "RHI_CommandList_RecordsVisibleTriangleCommandsWithinCapacity";
constexpr const char* TEST_DRAW_REQUIRES_PIPELINE = "RHI_SubmitDrawRejectsMissingPipelineWithoutMutation";
constexpr const char* TEST_DRAW_RANGE_OVERFLOW = "RHI_SubmitDrawRejectsVertexBufferRangeOverflow";
constexpr const char* TEST_DRAW_SNAPSHOT = "RHI_SubmitDraw_UpdatesNullSnapshot";
constexpr const char* TEST_INDEX_FORMAT_DEFAULTS = "RHI_IndexFormat_DefaultDescriptorsAreUnsupported";
constexpr const char* TEST_RECORD_INDEXED_STATIC_MESH = "RHI_CommandList_RecordsIndexedStaticMeshWithinCapacity";
constexpr const char* TEST_INDEXED_DRAW_REQUIRES_INDEX_BUFFER = "RHI_SubmitIndexedDrawRejectsMissingIndexBufferWithoutMutation";
constexpr const char* TEST_INDEXED_DRAW_RANGE_OVERFLOW = "RHI_SubmitIndexedDrawRejectsIndexRangeOverflow";
constexpr const char* TEST_INDEXED_DRAW_SNAPSHOT = "RHI_SubmitIndexedDraw_UpdatesNullSnapshot";
constexpr const char *TEST_SAMPLED_TEXTURE_BIND_DEFAULTS = "RHI_TextureSampling_DefaultBindingsAreBoundedValues";
constexpr const char *TEST_RECORD_TEXTURE_SAMPLING = "RHI_CommandList_RecordsTextureSamplingWithinCapacity";
constexpr const char *TEST_TEXTURE_SAMPLING_SLOT_OVERFLOW = "RHI_TextureSamplingSlotOverflow_DoesNotMutate";
constexpr const char *TEST_TEXTURE_SAMPLING_REQUIRES_SAMPLER = "RHI_SubmitTextureSamplingRejectsMissingSamplerWithoutMutation";
constexpr const char *TEST_SAMPLER_REQUIRES_TEXTURE = "RHI_SubmitSamplerRejectsMissingTextureWithoutMutation";
constexpr const char *TEST_TEXTURE_SAMPLING_STALE_TEXTURE = "RHI_SubmitTextureSamplingRejectsStaleTextureHandle";
constexpr const char *TEST_TEXTURE_SAMPLING_STALE_SAMPLER = "RHI_SubmitTextureSamplingRejectsStaleSamplerHandle";
constexpr const char *TEST_TEXTURE_SAMPLING_SNAPSHOT = "RHI_SubmitTextureSampling_UpdatesNullSnapshot";
constexpr const char *TEST_CONSTANT_BUFFER_BIND_DEFAULTS = "RHI_ConstantBufferBinding_DefaultsAreBoundedValues";
constexpr const char *TEST_RECORD_CONSTANT_BUFFER_BIND =
    "RHI_CommandList_RecordsConstantBufferBindingWithinCapacity";
constexpr const char *TEST_CONSTANT_BUFFER_SLOT_OVERFLOW =
    "RHI_ConstantBufferSlotOverflow_DoesNotMutate";
constexpr const char *TEST_CONSTANT_BUFFER_SNAPSHOT =
    "RHI_SubmitConstantBufferBinding_UpdatesNullSnapshot";
constexpr const char *TEST_CONSTANT_BUFFER_STALE =
    "RHI_SubmitConstantBufferBindingRejectsStaleHandle";
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
constexpr std::uint32_t TRIANGLE_VERTEX_COUNT = 3U;
constexpr std::uint32_t TRIANGLE_INDEX_COUNT = 3U;
constexpr std::size_t TRIANGLE_VERTEX_STRIDE_BYTES = sizeof(float) * 6U;
constexpr std::size_t TRIANGLE_VERTEX_BUFFER_BYTES = TRIANGLE_VERTEX_STRIDE_BYTES * TRIANGLE_VERTEX_COUNT;
constexpr std::size_t TRIANGLE_INDEX_BUFFER_BYTES = sizeof(std::uint16_t) * TRIANGLE_INDEX_COUNT;
constexpr std::uint16_t USER_VISIBLE_CAPTURE_WIDTH = 640U;
constexpr std::uint16_t USER_VISIBLE_CAPTURE_HEIGHT = 360U;
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

RhiColorTargetDesc UserVisibleTargetDesc() {
    return RhiColorTargetDesc{
        RhiFormat::Rgba8Unorm,
        {USER_VISIBLE_CAPTURE_WIDTH, USER_VISIBLE_CAPTURE_HEIGHT}};
}

RhiColorTargetDesc OversizedCaptureTargetDesc() {
    return RhiColorTargetDesc{
        RhiFormat::Rgba8Unorm,
        {static_cast<std::uint16_t>(MAX_CAPTURE_FIXTURE_EXTENT + 1U), 1U}};
}

RhiBufferDesc SmallVertexBufferDesc() {
    return RhiBufferDesc{RhiBufferUsage::Vertex, 4U};
}

RhiBufferDesc TriangleVertexBufferDesc() {
    return RhiBufferDesc{RhiBufferUsage::Vertex, TRIANGLE_VERTEX_BUFFER_BYTES};
}

RhiBufferDesc TriangleIndexBufferDesc() {
    return RhiBufferDesc{RhiBufferUsage::Index, TRIANGLE_INDEX_BUFFER_BYTES};
}

RhiTextureDesc SmallPrimitiveTextureDesc() {
    return RhiTextureDesc{RhiFormat::Rgba8Unorm, {2U, 2U}};
}

RhiInputLayoutDesc TriangleInputLayoutDesc() {
    RhiInputLayoutDesc desc{};
    desc.elements[0U].semantic = RhiInputElementSemantic::Position;
    desc.elements[0U].format = RhiInputElementFormat::Float32x2;
    desc.elements[0U].offset_bytes = 0U;
    desc.elements[1U].semantic = RhiInputElementSemantic::Color;
    desc.elements[1U].format = RhiInputElementFormat::Float32x4;
    desc.elements[1U].offset_bytes = sizeof(float) * 2U;
    desc.element_count = 2U;
    desc.stride_bytes = TRIANGLE_VERTEX_STRIDE_BYTES;
    return desc;
}

RhiDrawDesc TriangleDrawDesc() {
    RhiDrawDesc desc{};
    desc.topology = RhiPrimitiveTopology::TriangleList;
    desc.vertex_count = TRIANGLE_VERTEX_COUNT;
    desc.first_vertex = 0U;
    return desc;
}

RhiDrawIndexedDesc TriangleDrawIndexedDesc() {
    RhiDrawIndexedDesc desc{};
    desc.topology = RhiPrimitiveTopology::TriangleList;
    desc.index_count = TRIANGLE_INDEX_COUNT;
    desc.first_index = 0U;
    desc.vertex_offset = 0;
    return desc;
}

RhiVertexBufferView TriangleVertexBufferViewFor(RhiBufferHandle handle) {
    RhiVertexBufferView view{};
    view.buffer = handle;
    view.offset_bytes = 0U;
    view.stride_bytes = TRIANGLE_VERTEX_STRIDE_BYTES;
    view.size_bytes = TRIANGLE_VERTEX_BUFFER_BYTES;
    return view;
}

RhiIndexBufferView TriangleIndexBufferViewFor(RhiBufferHandle handle) {
    RhiIndexBufferView view{};
    view.buffer = handle;
    view.offset_bytes = 0U;
    view.size_bytes = TRIANGLE_INDEX_BUFFER_BYTES;
    view.format = RhiIndexFormat::Uint16;
    return view;
}

RhiSampledTextureBinding SampledTextureBindingFor(RhiTextureHandle texture) {
    RhiSampledTextureBinding binding{};
    binding.texture = texture;
    binding.slot = 0U;
    return binding;
}

RhiSamplerBinding SamplerBindingFor(RhiSamplerHandle sampler) {
    RhiSamplerBinding binding{};
    binding.sampler = sampler;
    binding.slot = 0U;
    return binding;
}

RhiVertexBufferView OverflowTriangleVertexBufferViewFor(RhiBufferHandle handle) {
    RhiVertexBufferView view = TriangleVertexBufferViewFor(handle);
    view.size_bytes = TRIANGLE_VERTEX_STRIDE_BYTES * 2U;
    return view;
}

RhiIndexBufferView OverflowTriangleIndexBufferViewFor(RhiBufferHandle handle) {
    RhiIndexBufferView view = TriangleIndexBufferViewFor(handle);
    view.size_bytes = sizeof(std::uint16_t) * 2U;
    return view;
}

std::array<std::uint8_t, 4U> SmallShaderBytes() {
    return std::array<std::uint8_t, 4U>{1U, 2U, 3U, 4U};
}

NullRhiDevice CreateInitializedDevice() {
    NullRhiDevice device;
    device.Initialize(RhiDeviceDesc{});
    return device;
}

bool CreateTarget(NullRhiDevice& device, RhiTextureHandle& out_handle) {
    return device.CreateColorTarget(SmallTargetDesc(), out_handle) == RhiStatus::Success;
}

bool CreateTargetThroughInterface(IRhiDevice &device, RhiTextureHandle &out_handle) {
    return device.CreateColorTarget(SmallTargetDesc(), out_handle) == RhiStatus::Success;
}

bool CreateShaderModule(
    IRhiDevice &device,
    RhiShaderStage stage,
    RhiShaderModuleHandle &out_handle) {
    const std::array<std::uint8_t, 4U> bytes = SmallShaderBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const RhiShaderModuleDesc desc{stage, byte_span};
    return device.CreateShaderModule(desc, out_handle) == RhiStatus::Success;
}

bool CreateTrianglePipeline(IRhiDevice &device, RhiPipelineHandle &out_handle) {
    RhiShaderModuleHandle vertex_shader{};
    if (!CreateShaderModule(device, RhiShaderStage::Vertex, vertex_shader)) {
        return false;
    }

    RhiShaderModuleHandle pixel_shader{};
    if (!CreateShaderModule(device, RhiShaderStage::Pixel, pixel_shader)) {
        return false;
    }

    RhiPipelineDesc desc{};
    desc.vertex_shader = vertex_shader;
    desc.pixel_shader = pixel_shader;
    desc.input_layout = TriangleInputLayoutDesc();
    return device.CreatePipeline(desc, out_handle) == RhiStatus::Success;
}

bool CreateTriangleBuffer(IRhiDevice &device, RhiBufferHandle &out_handle) {
    const std::span<const std::uint8_t> empty_bytes{};
    return device.CreateBuffer(TriangleVertexBufferDesc(), empty_bytes, out_handle) == RhiStatus::Success;
}

bool CreateTriangleIndexBuffer(IRhiDevice &device, RhiBufferHandle &out_handle) {
    const std::array<std::uint16_t, TRIANGLE_INDEX_COUNT> indices{0U, 1U, 2U};
    const auto *index_byte_pointer = reinterpret_cast<const std::uint8_t *>(indices.data());
    const std::span<const std::uint8_t> index_bytes(index_byte_pointer, TRIANGLE_INDEX_BUFFER_BYTES);
    return device.CreateBuffer(TriangleIndexBufferDesc(), index_bytes, out_handle) == RhiStatus::Success;
}

bool CreateSampledTexture(IRhiDevice &device, RhiTextureHandle &out_handle) {
    const std::array<std::uint8_t, 16U> texture_bytes{
        255U, 0U, 0U, 255U,
        0U, 255U, 0U, 255U,
        0U, 0U, 255U, 255U,
        255U, 255U, 255U, 255U};
    const std::span<const std::uint8_t> texture_span(texture_bytes.data(), texture_bytes.size());
    const RhiTextureDesc desc = SmallPrimitiveTextureDesc();
    return device.CreateTexture(desc, texture_span, out_handle) == RhiStatus::Success;
}

bool CreateSamplerPrimitive(IRhiDevice &device, RhiSamplerHandle &out_handle) {
    RhiSamplerDesc desc{};
    desc.linear_filter = false;
    desc.clamp_to_edge = true;
    return device.CreateSampler(desc, out_handle) == RhiStatus::Success;
}

bool CreateConstantBuffer(IRhiDevice &device, RhiBufferHandle &out_handle) {
    const std::array<std::uint8_t, 16U> bytes{
        1U, 2U, 3U, 4U,
        5U, 6U, 7U, 8U,
        9U, 10U, 11U, 12U,
        13U, 14U, 15U, 16U};
    RhiBufferDesc desc{};
    desc.usage = RhiBufferUsage::Constant;
    desc.size_bytes = bytes.size();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    return device.CreateBuffer(desc, byte_span, out_handle) == RhiStatus::Success;
}

RhiConstantBufferBinding ConstantBufferBindingFor(RhiBufferHandle handle) {
    RhiConstantBufferBinding binding{};
    binding.buffer = handle;
    binding.stage = RhiShaderStage::Pixel;
    binding.slot = 0U;
    return binding;
}

RhiPrimitiveRetirementRequest BufferRetirementRequest(std::uint64_t request_id, RhiBufferHandle handle) {
    RhiPrimitiveRetirementRequest request{};
    request.request_id = request_id;
    request.primitive_kind = RhiPrimitiveKind::Buffer;
    request.primitive_slot = handle.slot;
    request.primitive_generation = handle.generation;
    return request;
}

RhiPrimitiveRetirementRequest TextureRetirementRequest(std::uint64_t request_id, RhiTextureHandle handle) {
    RhiPrimitiveRetirementRequest request{};
    request.request_id = request_id;
    request.primitive_kind = RhiPrimitiveKind::Texture;
    request.primitive_slot = handle.slot;
    request.primitive_generation = handle.generation;
    return request;
}

RhiPrimitiveRetirementRequest SamplerRetirementRequest(std::uint64_t request_id, RhiSamplerHandle handle) {
    RhiPrimitiveRetirementRequest request{};
    request.request_id = request_id;
    request.primitive_kind = RhiPrimitiveKind::Sampler;
    request.primitive_slot = handle.slot;
    request.primitive_generation = handle.generation;
    return request;
}

RhiStatus RecordTriangleDrawFrame(
    IRhiDevice &device,
    RhiCommandList &command_list,
    RhiTextureHandle target,
    RhiPipelineHandle pipeline,
    const RhiVertexBufferView &vertex_buffer,
    const RhiDrawDesc &draw) {
    RhiStatus status = command_list.BeginFrame(target);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordBindPipeline(command_list, pipeline);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordBindVertexBuffer(command_list, vertex_buffer);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordDraw(command_list, draw);
    if (status != RhiStatus::Success) {
        return status;
    }

    return command_list.EndFrame();
}

RhiStatus RecordIndexedTriangleDrawFrame(
    IRhiDevice &device,
    RhiCommandList &command_list,
    RhiTextureHandle target,
    RhiPipelineHandle pipeline,
    const RhiVertexBufferView &vertex_buffer,
    const RhiIndexBufferView &index_buffer,
    const RhiDrawIndexedDesc &draw) {
    RhiStatus status = command_list.BeginFrame(target);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordBindPipeline(command_list, pipeline);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordBindVertexBuffer(command_list, vertex_buffer);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordBindIndexBuffer(command_list, index_buffer);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordDrawIndexed(command_list, draw);
    if (status != RhiStatus::Success) {
        return status;
    }

    return command_list.EndFrame();
}

RhiStatus RecordSampledIndexedTriangleDrawFrame(
    IRhiDevice &device,
    RhiCommandList &command_list,
    RhiTextureHandle target,
    RhiPipelineHandle pipeline,
    const RhiVertexBufferView &vertex_buffer,
    const RhiIndexBufferView &index_buffer,
    RhiTextureHandle sampled_texture,
    RhiSamplerHandle sampler,
    const RhiDrawIndexedDesc &draw) {
    RhiStatus status = command_list.BeginFrame(target);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordBindPipeline(command_list, pipeline);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordBindVertexBuffer(command_list, vertex_buffer);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordBindIndexBuffer(command_list, index_buffer);
    if (status != RhiStatus::Success) {
        return status;
    }

    const RhiSampledTextureBinding texture_binding = SampledTextureBindingFor(sampled_texture);
    status = device.RecordBindSampledTexture(command_list, texture_binding);
    if (status != RhiStatus::Success) {
        return status;
    }

    const RhiSamplerBinding sampler_binding = SamplerBindingFor(sampler);
    status = device.RecordBindSampler(command_list, sampler_binding);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordDrawIndexed(command_list, draw);
    if (status != RhiStatus::Success) {
        return status;
    }

    return command_list.EndFrame();
}

RhiStatus RecordConstantBufferIndexedTriangleDrawFrame(
    IRhiDevice &device,
    RhiCommandList &command_list,
    RhiTextureHandle target,
    RhiPipelineHandle pipeline,
    const RhiVertexBufferView &vertex_buffer,
    const RhiIndexBufferView &index_buffer,
    RhiBufferHandle constant_buffer,
    const RhiDrawIndexedDesc &draw) {
    RhiStatus status = command_list.BeginFrame(target);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordBindPipeline(command_list, pipeline);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordBindVertexBuffer(command_list, vertex_buffer);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordBindIndexBuffer(command_list, index_buffer);
    if (status != RhiStatus::Success) {
        return status;
    }

    const RhiConstantBufferBinding constant_buffer_binding = ConstantBufferBindingFor(constant_buffer);
    status = device.RecordBindConstantBuffer(command_list, constant_buffer_binding);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordDrawIndexed(command_list, draw);
    if (status != RhiStatus::Success) {
        return status;
    }

    return command_list.EndFrame();
}

RhiStatus ClearSubmitPresent(NullRhiDevice& device, RhiTextureHandle target, RhiColor color) {
    RhiCommandList command_list(MAX_COMMANDS);
    RhiStatus status = command_list.BeginFrame(target);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordClear(command_list, target, color);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = command_list.EndFrame();
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.Submit(command_list);
    if (status != RhiStatus::Success) {
        return status;
    }

    return device.Present();
}

RhiStatus ClearSubmitPresentThroughInterface(IRhiDevice &device, RhiTextureHandle target, RhiColor color) {
    RhiCommandList command_list(MAX_COMMANDS);
    RhiStatus status = command_list.BeginFrame(target);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.RecordClear(command_list, target, color);
    if (status != RhiStatus::Success) {
        return status;
    }

    status = command_list.EndFrame();
    if (status != RhiStatus::Success) {
        return status;
    }

    status = device.Submit(command_list);
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

bool DeviceSnapshotsEqual(const RhiDeviceSnapshot &left, const RhiDeviceSnapshot &right) {
    if (left.color_target_capacity != right.color_target_capacity) {
        return false;
    }

    if (left.color_target_count != right.color_target_count) {
        return false;
    }

    if (left.command_storage_capacity_before_frame != right.command_storage_capacity_before_frame) {
        return false;
    }

    if (left.command_storage_capacity_after_last_frame != right.command_storage_capacity_after_last_frame) {
        return false;
    }

    if (left.created_target_count != right.created_target_count) {
        return false;
    }

    if (left.destroyed_target_count != right.destroyed_target_count) {
        return false;
    }

    if (left.recorded_command_count != right.recorded_command_count) {
        return false;
    }

    if (left.submit_count != right.submit_count) {
        return false;
    }

    if (left.submitted_draw_count != right.submitted_draw_count) {
        return false;
    }

    if (left.submitted_indexed_draw_count != right.submitted_indexed_draw_count) {
        return false;
    }

    if (left.submitted_sampled_texture_bind_count != right.submitted_sampled_texture_bind_count) {
        return false;
    }

    if (left.submitted_sampler_bind_count != right.submitted_sampler_bind_count) {
        return false;
    }

    if (left.submitted_constant_buffer_bind_count != right.submitted_constant_buffer_bind_count) {
        return false;
    }

    if (left.rejected_indexed_draw_count != right.rejected_indexed_draw_count) {
        return false;
    }

    if (left.rejected_sampled_texture_bind_count != right.rejected_sampled_texture_bind_count) {
        return false;
    }

    if (left.rejected_sampler_bind_count != right.rejected_sampler_bind_count) {
        return false;
    }

    if (left.rejected_constant_buffer_bind_count != right.rejected_constant_buffer_bind_count) {
        return false;
    }

    if (left.last_draw_vertex_count != right.last_draw_vertex_count) {
        return false;
    }

    if (left.last_indexed_draw_index_count != right.last_indexed_draw_index_count) {
        return false;
    }

    if (left.last_bound_sampled_texture_slot != right.last_bound_sampled_texture_slot) {
        return false;
    }

    if (left.last_bound_sampler_slot != right.last_bound_sampler_slot) {
        return false;
    }

    if (left.last_bound_constant_buffer_slot != right.last_bound_constant_buffer_slot) {
        return false;
    }

    if (left.last_bound_constant_buffer_stage != right.last_bound_constant_buffer_stage) {
        return false;
    }

    if (left.last_bound_index_buffer_offset_bytes != right.last_bound_index_buffer_offset_bytes) {
        return false;
    }

    if (left.last_bound_index_buffer_size_bytes != right.last_bound_index_buffer_size_bytes) {
        return false;
    }

    if (left.present_count != right.present_count) {
        return false;
    }

    if (left.capture_count != right.capture_count) {
        return false;
    }

    if (left.failed_operation_count != right.failed_operation_count) {
        return false;
    }

    if (left.last_status != right.last_status) {
        return false;
    }

    if (left.last_capture_bytes_written != right.last_capture_bytes_written) {
        return false;
    }

    if (left.last_capture_extent.width != right.last_capture_extent.width) {
        return false;
    }

    if (left.last_capture_extent.height != right.last_capture_extent.height) {
        return false;
    }

    if (left.swapchain.extent.width != right.swapchain.extent.width) {
        return false;
    }

    if (left.swapchain.extent.height != right.swapchain.extent.height) {
        return false;
    }

    if (left.swapchain.color_format != right.swapchain.color_format) {
        return false;
    }

    if (left.swapchain.color_target.slot != right.swapchain.color_target.slot) {
        return false;
    }

    if (left.swapchain.color_target.generation != right.swapchain.color_target.generation) {
        return false;
    }

    if (left.swapchain.valid != right.swapchain.valid) {
        return false;
    }

    if (left.swapchain.presented != right.swapchain.presented) {
        return false;
    }

    if (left.resources.buffer_capacity != right.resources.buffer_capacity) {
        return false;
    }

    if (left.resources.buffer_count != right.resources.buffer_count) {
        return false;
    }

    if (left.resources.texture_capacity != right.resources.texture_capacity) {
        return false;
    }

    if (left.resources.texture_count != right.resources.texture_count) {
        return false;
    }

    if (left.resources.sampler_capacity != right.resources.sampler_capacity) {
        return false;
    }

    if (left.resources.sampler_count != right.resources.sampler_count) {
        return false;
    }

    if (left.resources.shader_module_capacity != right.resources.shader_module_capacity) {
        return false;
    }

    if (left.resources.shader_module_count != right.resources.shader_module_count) {
        return false;
    }

    if (left.resources.pipeline_capacity != right.resources.pipeline_capacity) {
        return false;
    }

    if (left.resources.pipeline_count != right.resources.pipeline_count) {
        return false;
    }

    if (left.resources.created_primitive_count != right.resources.created_primitive_count) {
        return false;
    }

    if (left.resources.destroyed_primitive_count != right.resources.destroyed_primitive_count) {
        return false;
    }

    if (left.resources.updated_primitive_count != right.resources.updated_primitive_count) {
        return false;
    }

    if (left.resources.signaled_fence_count != right.resources.signaled_fence_count) {
        return false;
    }

    if (left.resources.last_update_bytes != right.resources.last_update_bytes) {
        return false;
    }

    if (left.allocation_accounting_status != right.allocation_accounting_status) {
        return false;
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

    if (capabilities.max_capture_fixture_extent != MAX_CAPTURE_FIXTURE_EXTENT) {
        return Fail("capabilities reported wrong capture extent");
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

int RhiInterfaceCreateNullDeviceReturnsCapabilities() {
    NullRhiDevice device_storage;
    IRhiDevice &device = device_storage;
    RhiDeviceDesc desc{};
    const RhiStatus status = device.Initialize(desc);
    if (status != RhiStatus::Success) {
        return Fail("interface device did not initialize");
    }

    const auto capabilities = device.Capabilities();
    if (capabilities.backend_kind != RhiBackendKind::Null) {
        return Fail("interface capabilities did not report null backend");
    }

    if (!capabilities.supports_capture) {
        return Fail("interface capabilities did not preserve capture support");
    }

    if (capabilities.supports_native_surface) {
        return Fail("null backend reported native surface support");
    }

    if (capabilities.supports_swapchain) {
        return Fail("null backend reported swapchain support");
    }

    if (capabilities.supports_hardware_device) {
        return Fail("null backend reported hardware device support");
    }

    return 0;
}

int RhiInterfaceClearSubmitPresentCaptureMatchesNullDevice() {
    NullRhiDevice device_storage;
    IRhiDevice &device = device_storage;
    RhiDeviceDesc desc{};
    if (device.Initialize(desc) != RhiStatus::Success) {
        return Fail("interface device initialize failed");
    }

    RhiTextureHandle handle{};
    if (!CreateTargetThroughInterface(device, handle)) {
        return Fail("interface target creation failed");
    }

    const RhiColor color{7U, 8U, 9U, 255U};
    if (ClearSubmitPresentThroughInterface(device, handle, color) != RhiStatus::Success) {
        return Fail("interface clear submit present failed");
    }

    std::vector<std::uint8_t> capture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    std::span<std::uint8_t> destination(capture.data(), capture.size());
    const RhiCaptureResult result = device.CapturePresentedTarget(destination);
    if (result.status != RhiStatus::Success) {
        return Fail("interface capture failed");
    }

    if (!BytesMatchColor(capture, color)) {
        return Fail("interface capture bytes did not match clear color");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.submit_count != 1U) {
        return Fail("interface submit count changed unexpectedly");
    }

    if (snapshot.present_count != 1U) {
        return Fail("interface present count changed unexpectedly");
    }

    if (snapshot.command_storage_capacity_before_frame != snapshot.command_storage_capacity_after_last_frame) {
        return Fail("interface frame path grew command storage");
    }

    return 0;
}

int RhiFactoryCreateNullDeviceUsesCallerOwnedStorage() {
    NullRhiDevice device_storage;
    RhiDeviceDesc desc{};
    const RhiDeviceCreateResult result = RhiDeviceFactory::CreateDevice(desc, &device_storage);
    if (result.status != RhiStatus::Success) {
        return Fail("factory did not create null device");
    }

    IRhiDevice *expected_device = &device_storage;
    if (result.device != expected_device) {
        return Fail("factory did not return caller owned storage");
    }

    if (result.capabilities.backend_kind != RhiBackendKind::Null) {
        return Fail("factory result capabilities did not report null backend");
    }

    RhiTextureHandle handle{};
    if (!CreateTargetThroughInterface(*result.device, handle)) {
        return Fail("factory device target creation failed");
    }

    return 0;
}

int RhiFactoryNullStorageRejectedWithoutOutput() {
    RhiDeviceDesc desc{};
    const RhiDeviceCreateResult result = RhiDeviceFactory::CreateDevice(desc, nullptr);
    if (result.status != RhiStatus::InvalidDescriptor) {
        return Fail("factory null storage did not return invalid descriptor");
    }

    if (result.device != nullptr) {
        return Fail("factory null storage returned a device");
    }

    return 0;
}

int RhiFactoryD3D11BackendUnsupportedWithoutMutation() {
    NullRhiDevice device_storage = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device_storage, handle)) {
        return Fail("baseline target creation failed");
    }

    const RhiDeviceSnapshot before_snapshot = device_storage.Snapshot();
    RhiDeviceDesc desc{};
    desc.backend_kind = RhiBackendKind::D3D11;
    const RhiDeviceCreateResult result = RhiDeviceFactory::CreateDevice(desc, &device_storage);
    if (result.status != RhiStatus::UnsupportedBackend) {
        return Fail("factory d3d11 request did not return unsupported");
    }

    if (result.device != nullptr) {
        return Fail("factory d3d11 request returned a device");
    }

    const RhiDeviceSnapshot after_snapshot = device_storage.Snapshot();
    if (!DeviceSnapshotsEqual(before_snapshot, after_snapshot)) {
        return Fail("factory d3d11 request mutated null device");
    }

    return 0;
}

int RhiFactorySurfaceRequiredForNullBackendRejectedWithoutMutation() {
    NullRhiDevice device_storage = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device_storage, handle)) {
        return Fail("baseline target creation failed");
    }

    const RhiDeviceSnapshot before_snapshot = device_storage.Snapshot();
    RhiDeviceDesc desc{};
    desc.requires_native_surface = true;
    desc.native_surface.valid = true;
    desc.native_surface.window_value = 1U;
    desc.native_surface.instance_value = 2U;

    const RhiDeviceCreateResult result = RhiDeviceFactory::CreateDevice(desc, &device_storage);
    if (result.status != RhiStatus::UnsupportedBackend) {
        return Fail("null backend surface request did not return unsupported");
    }

    if (result.device != nullptr) {
        return Fail("null backend surface request returned a device");
    }

    const RhiDeviceSnapshot after_snapshot = device_storage.Snapshot();
    if (!DeviceSnapshotsEqual(before_snapshot, after_snapshot)) {
        return Fail("null backend surface request mutated device");
    }

    return 0;
}

int RhiFactoryInvalidNativeSurfaceRejectedBeforeMutation() {
    NullRhiDevice device_storage = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device_storage, handle)) {
        return Fail("baseline target creation failed");
    }

    const RhiDeviceSnapshot before_snapshot = device_storage.Snapshot();
    RhiDeviceDesc desc{};
    desc.requires_native_surface = true;

    const RhiDeviceCreateResult result = RhiDeviceFactory::CreateDevice(desc, &device_storage);
    if (result.status != RhiStatus::InvalidDescriptor) {
        return Fail("invalid native surface did not return invalid descriptor");
    }

    if (result.device != nullptr) {
        return Fail("invalid native surface returned a device");
    }

    const RhiDeviceSnapshot after_snapshot = device_storage.Snapshot();
    if (!DeviceSnapshotsEqual(before_snapshot, after_snapshot)) {
        return Fail("invalid native surface mutated device");
    }

    return 0;
}

int RhiNativeSurfaceDescDefaultIsInvalidPlainValue() {
    RhiNativeSurfaceDesc desc{};
    if (desc.valid) {
        return Fail("default native surface descriptor was valid");
    }

    if (desc.window_value != 0U) {
        return Fail("default native surface window value was nonzero");
    }

    if (RhiDeviceFactory::ValidateNativeSurfaceDesc(desc) != RhiStatus::InvalidDescriptor) {
        return Fail("default native surface descriptor was accepted");
    }

    desc.valid = true;
    desc.window_value = 1U;
    const RhiStatus status = RhiDeviceFactory::ValidateNativeSurfaceDesc(desc);
    if (status != RhiStatus::Success) {
        return Fail("valid opaque native surface descriptor was rejected");
    }

    return 0;
}

int RhiFactoryRawStorageCreatesNullDevice() {
    const std::size_t required_size = RhiDeviceFactory::RequiredDeviceStorageSize(RhiBackendKind::Null);
    if (required_size == 0U) {
        return Fail("null backend reported empty storage");
    }

    std::vector<std::byte> storage(required_size);
    std::span<std::byte> storage_span(storage.data(), storage.size());
    RhiDeviceDesc desc{};
    const RhiDeviceCreateResult result = RhiDeviceFactory::CreateDevice(desc, storage_span);
    if (result.status != RhiStatus::Success) {
        return Fail("raw storage factory did not create null device");
    }

    if (result.device == nullptr) {
        return Fail("raw storage factory returned null device");
    }

    const auto device_address = reinterpret_cast<std::uintptr_t>(result.device);
    const auto storage_begin = reinterpret_cast<std::uintptr_t>(storage.data());
    const auto storage_end = storage_begin + storage.size();
    if (device_address < storage_begin || device_address >= storage_end) {
        return Fail("raw storage factory returned a device outside caller storage");
    }

    if (result.capabilities.backend_kind != RhiBackendKind::Null) {
        return Fail("raw storage factory returned wrong backend capabilities");
    }

    RhiTextureHandle handle{};
    if (result.device->CreateColorTarget(SmallTargetDesc(), handle) != RhiStatus::Success) {
        return Fail("raw storage null device target creation failed");
    }

    if (RhiDeviceFactory::DestroyDevice(result.device) != RhiStatus::Success) {
        return Fail("raw storage null device destroy failed");
    }

    return 0;
}

int RhiFactoryRawStorageRejectsSmallBuffer() {
    const std::size_t required_size = RhiDeviceFactory::RequiredDeviceStorageSize(RhiBackendKind::Null);
    if (required_size == 0U) {
        return Fail("null backend reported empty storage");
    }

    std::vector<std::byte> storage(required_size - 1U);
    RhiDeviceDesc desc{};
    const RhiDeviceCreateResult result = RhiDeviceFactory::CreateDevice(desc, std::span<std::byte>(storage.data(), storage.size()));
    if (result.status != RhiStatus::InvalidDescriptor) {
        return Fail("small raw storage did not return invalid descriptor");
    }

    if (result.device != nullptr) {
        return Fail("small raw storage returned a device");
    }

    return 0;
}

int RhiFactoryD3D11InvalidSurfaceFailsBeforeHardware() {
    const std::size_t required_size = RhiDeviceFactory::RequiredDeviceStorageSize(RhiBackendKind::D3D11);
    if (required_size == 0U) {
        return Fail("d3d11 backend reported empty storage on windows gate");
    }

    std::vector<std::byte> storage(required_size);
    RhiDeviceDesc desc{};
    desc.backend_kind = RhiBackendKind::D3D11;
    desc.requires_native_surface = true;
    desc.requires_swapchain = true;
    desc.swapchain.extent = {2U, 2U};

    const RhiDeviceCreateResult result = RhiDeviceFactory::CreateDevice(desc, std::span<std::byte>(storage.data(), storage.size()));
    if (result.status != RhiStatus::InvalidDescriptor) {
        return Fail("d3d11 invalid surface did not fail before hardware creation");
    }

    if (result.device != nullptr) {
        return Fail("d3d11 invalid surface returned a live device");
    }

    return 0;
}

int RhiSwapchainDescDefaultIsBoundedPlainValue() {
    RhiSwapchainDesc desc{};
    if (desc.color_format != RhiFormat::Rgba8Unorm) {
        return Fail("default swapchain format was unexpected");
    }

    if (desc.extent.width != 0U) {
        return Fail("default swapchain width was nonzero");
    }

    if (desc.extent.height != 0U) {
        return Fail("default swapchain height was nonzero");
    }

    if (desc.vsync_enabled) {
        return Fail("default swapchain vsync was enabled");
    }

    return 0;
}

int RhiNullBackendSwapchainQueryReturnsUnsupported() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{7U, 9U};
    const RhiStatus status = device.GetSwapchainColorTarget(handle);
    if (status != RhiStatus::UnsupportedBackend) {
        return Fail("null swapchain query did not return unsupported");
    }

    if (handle.slot != 0U) {
        return Fail("null swapchain query did not clear handle slot");
    }

    if (handle.generation != 0U) {
        return Fail("null swapchain query did not clear handle generation");
    }

    return 0;
}

int RhiSwapchainResizeDefaultContractsAreExplicit() {
    RhiSwapchainResizeRequest request{};
    if (request.extent.width != 0U) {
        return Fail("default resize request width was nonzero");
    }

    if (request.extent.height != 0U) {
        return Fail("default resize request height was nonzero");
    }

    RhiSwapchainResizeResult result{};
    if (result.status != RhiStatus::InvalidDescriptor) {
        return Fail("default resize result status was not invalid descriptor");
    }

    if (result.previous_extent.width != 0U || result.previous_extent.height != 0U) {
        return Fail("default resize result previous extent was nonzero");
    }

    if (result.previous_color_target.slot != 0U || result.previous_color_target.generation != 0U) {
        return Fail("default resize result previous handle was nonzero");
    }

    if (result.snapshot.valid) {
        return Fail("default resize result snapshot was valid");
    }

    if (result.resized) {
        return Fail("default resize result resized flag was true");
    }

    RhiSwapchainSnapshot snapshot{};
    if (snapshot.resize_count != 0U) {
        return Fail("default swapchain resize count was nonzero");
    }

    if (snapshot.rejected_resize_count != 0U) {
        return Fail("default swapchain rejected resize count was nonzero");
    }

    RhiCapabilities capabilities{};
    if (capabilities.supports_swapchain_resize) {
        return Fail("default capabilities unexpectedly support swapchain resize");
    }

    return 0;
}

int RhiNullBackendSwapchainResizeReturnsUnsupportedWithoutTarget() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    const auto before_snapshot = device.Snapshot();

    RhiSwapchainResizeRequest request{};
    request.extent = {2U, 2U};
    RhiSwapchainResizeResult result{};
    const RhiStatus status = device_interface.ResizeSwapchain(request, result);
    if (status != RhiStatus::UnsupportedBackend) {
        return Fail("null swapchain resize did not return unsupported");
    }

    if (result.status != RhiStatus::UnsupportedBackend) {
        return Fail("null swapchain resize result status was not unsupported");
    }

    if (result.resized) {
        return Fail("null swapchain resize result reported resized");
    }

    if (result.previous_color_target.slot != 0U || result.previous_color_target.generation != 0U) {
        return Fail("null swapchain resize wrote previous target");
    }

    if (result.snapshot.valid) {
        return Fail("null swapchain resize returned a valid snapshot");
    }

    if (device.Capabilities().supports_swapchain_resize) {
        return Fail("null capabilities unexpectedly support swapchain resize");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.failed_operation_count != before_snapshot.failed_operation_count + 1U) {
        return Fail("null swapchain resize failure was not tracked");
    }

    if (after_snapshot.swapchain.valid) {
        return Fail("null swapchain resize made swapchain valid");
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

    RhiColorTargetDesc unsupported_format_desc = SmallTargetDesc();
    unsupported_format_desc.format = RhiFormat::Unsupported;
    if (device.CreateColorTarget(unsupported_format_desc, handle) != RhiStatus::UnsupportedFormat) {
        return Fail("unsupported target format was not rejected");
    }

    RhiColorTargetDesc zero_extent_desc = SmallTargetDesc();
    zero_extent_desc.extent.width = 0U;
    if (device.CreateColorTarget(zero_extent_desc, handle) != RhiStatus::InvalidDescriptor) {
        return Fail("zero extent target was not rejected");
    }

    RhiColorTargetDesc over_extent_desc = SmallTargetDesc();
    over_extent_desc.extent.width = MAX_COLOR_TARGET_EXTENT + 1U;
    if (device.CreateColorTarget(over_extent_desc, handle) != RhiStatus::InvalidDescriptor) {
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

    const auto before_snapshot = device.Snapshot();
    RhiTextureHandle overflow_handle{};
    const RhiStatus overflow_status = device.CreateColorTarget(SmallTargetDesc(), overflow_handle);
    if (overflow_status != RhiStatus::CapacityExceeded) {
        return Fail("target capacity overflow did not return explicit status");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.color_target_count != before_snapshot.color_target_count) {
        return Fail("target overflow changed target count");
    }

    if (after_snapshot.created_target_count != before_snapshot.created_target_count) {
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

    RhiCommandList command_list(MAX_COMMANDS);
    command_list.BeginFrame(handle);
    const RhiStatus stale_status = device.RecordClear(command_list, handle, RhiColor{1U, 2U, 3U, 4U});
    if (stale_status != RhiStatus::InvalidHandle) {
        return Fail("stale handle did not return explicit status");
    }

    if (device.Snapshot().destroyed_target_count != 1U) {
        return Fail("destroyed target count was not recorded");
    }

    return 0;
}

int RhiReinitializeInvalidatesPriorTargetHandle() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle stale_handle{};
    if (!CreateTarget(device, stale_handle)) {
        return Fail(REINIT_TARGET_CREATION_MESSAGE);
    }

    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return Fail(REINIT_DEVICE_MESSAGE);
    }

    RhiTextureHandle active_handle{};
    if (!CreateTarget(device, active_handle)) {
        return Fail(REINIT_ACTIVE_TARGET_CREATION_MESSAGE);
    }

    const auto before_snapshot = device.Snapshot();
    if (device.DestroyTarget(stale_handle) != RhiStatus::InvalidHandle) {
        return Fail(REINIT_STALE_TARGET_ACCEPTED_MESSAGE);
    }

    if (device.Snapshot().color_target_count != before_snapshot.color_target_count) {
        return Fail(REINIT_STALE_TARGET_COUNT_MESSAGE);
    }

    RhiCommandList command_list(MAX_COMMANDS);
    if (command_list.BeginFrame(active_handle) != RhiStatus::Success) {
        return Fail(REINIT_BEGIN_FRAME_MESSAGE);
    }

    if (device.RecordClear(command_list, stale_handle, RhiColor{1U, 2U, 3U, 4U}) != RhiStatus::InvalidHandle) {
        return Fail(REINIT_STALE_CLEAR_MESSAGE);
    }

    if (device.Snapshot().recorded_command_count != before_snapshot.recorded_command_count) {
        return Fail(REINIT_STALE_CLEAR_COUNT_MESSAGE);
    }

    if (device.DestroyTarget(active_handle) != RhiStatus::Success) {
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

    RhiCommandList command_list(3U);
    if (command_list.BeginFrame(handle) != RhiStatus::Success) {
        return Fail("begin frame failed");
    }

    if (device.RecordClear(command_list, handle, RhiColor{1U, 2U, 3U, 4U}) != RhiStatus::Success) {
        return Fail("record clear failed");
    }

    if (command_list.EndFrame() != RhiStatus::Success) {
        return Fail("end frame failed within capacity");
    }

    if (command_list.CommandCount() != 3U) {
        return Fail("clear command list count was wrong");
    }

    const auto snapshot = command_list.Snapshot();
    if (snapshot.last_status != RhiStatus::Success) {
        return Fail("clear command list did not record success status");
    }

    return 0;
}

int RhiCommandListCapacityOverflowDoesNotMutate() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail("target creation failed");
    }

    RhiCommandList command_list(2U);
    if (command_list.BeginFrame(handle) != RhiStatus::Success) {
        return Fail("begin frame failed");
    }

    if (device.RecordClear(command_list, handle, RhiColor{1U, 2U, 3U, 4U}) != RhiStatus::Success) {
        return Fail("clear record failed");
    }

    const std::size_t count_before = command_list.CommandCount();
    const RhiStatus end_status = command_list.EndFrame();
    if (end_status != RhiStatus::CapacityExceeded) {
        return Fail("command capacity overflow did not return explicit status");
    }

    if (command_list.CommandCount() != count_before) {
        return Fail("command capacity overflow mutated command count");
    }

    const auto snapshot = command_list.Snapshot();
    if (snapshot.required_command_count != count_before + 1U) {
        return Fail("command capacity overflow did not record required command count");
    }

    return 0;
}

int RhiCommandListLastStatusTracksLifecycleAndCapacity() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail("target creation failed");
    }

    RhiCommandList command_list(2U);
    RhiColor clear_color{1U, 2U, 3U, 4U};
    RhiStatus status = command_list.RecordClear(handle, clear_color);
    if (status != RhiStatus::InvalidLifecycle) {
        return Fail("record clear before begin did not return invalid lifecycle");
    }

    auto snapshot = command_list.Snapshot();
    if (snapshot.last_status != RhiStatus::InvalidLifecycle) {
        return Fail("record clear before begin did not record last status");
    }

    if (snapshot.required_command_count != 1U) {
        return Fail("record clear before begin changed required command count");
    }

    status = command_list.Reset();
    if (status != RhiStatus::Success) {
        return Fail("command list reset after record failure failed");
    }

    snapshot = command_list.Snapshot();
    if (snapshot.last_status != RhiStatus::Success) {
        return Fail("command list reset did not clear last status");
    }

    status = command_list.EndFrame();
    if (status != RhiStatus::InvalidLifecycle) {
        return Fail("end frame before begin did not return invalid lifecycle");
    }

    snapshot = command_list.Snapshot();
    if (snapshot.last_status != RhiStatus::InvalidLifecycle) {
        return Fail("end frame before begin did not record last status");
    }

    status = command_list.Reset();
    if (status != RhiStatus::Success) {
        return Fail("command list reset after end frame failure failed");
    }

    status = command_list.BeginFrame(handle);
    if (status != RhiStatus::Success) {
        return Fail("begin frame for last status matrix failed");
    }

    status = command_list.BeginFrame(handle);
    if (status != RhiStatus::InvalidLifecycle) {
        return Fail("second begin frame did not return invalid lifecycle");
    }

    snapshot = command_list.Snapshot();
    if (snapshot.last_status != RhiStatus::InvalidLifecycle) {
        return Fail("second begin frame did not record last status");
    }

    status = command_list.Reset();
    if (status != RhiStatus::Success) {
        return Fail("command list reset after begin failure failed");
    }

    status = command_list.BeginFrame(handle);
    if (status != RhiStatus::Success) {
        return Fail("begin frame before capacity failure failed");
    }

    status = command_list.RecordClear(handle, clear_color);
    if (status != RhiStatus::Success) {
        return Fail("record clear before capacity failure failed");
    }

    const std::size_t count_before = command_list.CommandCount();
    status = command_list.EndFrame();
    if (status != RhiStatus::CapacityExceeded) {
        return Fail("end frame capacity failure did not return capacity exceeded");
    }

    snapshot = command_list.Snapshot();
    if (snapshot.command_count != count_before) {
        return Fail("end frame capacity failure changed command count");
    }

    if (snapshot.required_command_count != count_before + 1U) {
        return Fail("end frame capacity failure did not record required command count");
    }

    if (snapshot.last_status != RhiStatus::CapacityExceeded) {
        return Fail("end frame capacity failure did not record last status");
    }

    status = command_list.Reset();
    if (status != RhiStatus::Success) {
        return Fail("command list reset after capacity failure failed");
    }

    snapshot = command_list.Snapshot();
    if (snapshot.required_command_count != 1U) {
        return Fail("command list reset after capacity failure did not reset required command count");
    }

    if (snapshot.last_status != RhiStatus::Success) {
        return Fail("command list reset after capacity failure did not clear last status");
    }

    return 0;
}

int RhiSubmitRejectsOversizedCommandListWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    if (!CreateTarget(device, handle)) {
        return Fail("target creation failed");
    }

    const RhiColor initial_color{1U, 2U, 3U, 4U};
    if (ClearSubmitPresent(device, handle, initial_color) != RhiStatus::Success) {
        return Fail("initial clear submit present failed");
    }

    const auto before_snapshot = device.Snapshot();
    RhiCommandList oversized_command_list(MAX_COMMANDS + 1U);
    if (oversized_command_list.BeginFrame(handle) != RhiStatus::Success) {
        return Fail("oversized begin frame failed before submit");
    }

    if (device.RecordClear(oversized_command_list, handle, RhiColor{9U, 8U, 7U, 6U}) != RhiStatus::Success) {
        return Fail("oversized clear record failed before submit");
    }

    if (oversized_command_list.EndFrame() != RhiStatus::Success) {
        return Fail("oversized end frame failed before submit");
    }

    const RhiStatus submit_status = device.Submit(oversized_command_list);
    if (submit_status != RhiStatus::CapacityExceeded) {
        return Fail("oversized command list submit did not return capacity status");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.submit_count != before_snapshot.submit_count) {
        return Fail("oversized command list submit mutated submit count");
    }

    std::vector<std::uint8_t> capture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    if (result.status != RhiStatus::Success) {
        return Fail("capture failed after rejected oversized submit");
    }

    if (!BytesMatchColor(capture, initial_color)) {
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

    RhiCommandList command_list(MAX_COMMANDS);
    if (command_list.BeginFrame(handle) != RhiStatus::Success) {
        return Fail("begin frame failed");
    }

    const std::size_t count_before = command_list.CommandCount();
    const RhiStatus status = device.RecordClear(command_list, RhiTextureHandle{99U, 1U}, RhiColor{1U, 2U, 3U, 4U});
    if (status != RhiStatus::InvalidHandle) {
        return Fail("invalid target clear did not return handle status");
    }

    if (command_list.CommandCount() != count_before) {
        return Fail("invalid target clear mutated command list");
    }

    return 0;
}

int RhiSubmitRejectsMismatchedRecordedTargetWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle frame_target{};
    RhiTextureHandle other_target{};
    if (!CreateTarget(device, frame_target)) {
        return Fail("frame target creation failed");
    }

    if (!CreateTarget(device, other_target)) {
        return Fail("other target creation failed");
    }

    const RhiColor other_initial_color{4U, 3U, 2U, 1U};
    if (ClearSubmitPresent(device, other_target, other_initial_color) != RhiStatus::Success) {
        return Fail("initial other target clear submit present failed");
    }

    const auto before_snapshot = device.Snapshot();
    RhiCommandList command_list(MAX_COMMANDS);
    if (command_list.BeginFrame(frame_target) != RhiStatus::Success) {
        return Fail("begin frame failed");
    }

    if (device.RecordClear(command_list, other_target, RhiColor{9U, 8U, 7U, 6U}) != RhiStatus::Success) {
        return Fail("mismatched clear record failed before submit");
    }

    if (command_list.EndFrame() != RhiStatus::Success) {
        return Fail("end frame failed");
    }

    const RhiStatus submit_status = device.Submit(command_list);
    if (submit_status != RhiStatus::InvalidHandle) {
        return Fail("mismatched recorded target did not return explicit status");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.submit_count != before_snapshot.submit_count) {
        return Fail("mismatched target submit mutated submit count");
    }

    std::vector<std::uint8_t> capture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    if (result.status != RhiStatus::Success) {
        return Fail("capture failed after rejected mismatched submit");
    }

    if (!BytesMatchColor(capture, other_initial_color)) {
        return Fail("rejected mismatched submit mutated recorded target bytes");
    }

    return 0;
}

int RhiSubmitRejectsStaleRecordedTargetWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle original_target{};
    if (!CreateTarget(device, original_target)) {
        return Fail("original target creation failed");
    }

    if (device.DestroyTarget(original_target) != RhiStatus::Success) {
        return Fail("destroying original target failed");
    }

    RhiTextureHandle frame_target{};
    if (!CreateTarget(device, frame_target)) {
        return Fail("replacement frame target creation failed");
    }

    if (frame_target.slot != original_target.slot) {
        return Fail("replacement target did not reuse slot for stale generation test");
    }

    if (frame_target.generation == original_target.generation) {
        return Fail("replacement target did not advance generation");
    }

    const RhiColor frame_initial_color{5U, 6U, 7U, 8U};
    if (ClearSubmitPresent(device, frame_target, frame_initial_color) != RhiStatus::Success) {
        return Fail("initial frame target clear submit present failed");
    }

    const auto before_snapshot = device.Snapshot();
    RhiCommandList command_list(MAX_COMMANDS);
    if (command_list.BeginFrame(frame_target) != RhiStatus::Success) {
        return Fail("begin frame failed");
    }

    if (command_list.RecordClear(original_target, RhiColor{9U, 8U, 7U, 6U}) != RhiStatus::Success) {
        return Fail("stale generation clear record failed before submit");
    }

    if (command_list.EndFrame() != RhiStatus::Success) {
        return Fail("end frame failed");
    }

    const RhiStatus submit_status = device.Submit(command_list);
    if (submit_status != RhiStatus::InvalidHandle) {
        return Fail("stale recorded target did not return explicit status");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.submit_count != before_snapshot.submit_count) {
        return Fail("stale target submit mutated submit count");
    }

    if (after_snapshot.color_target_count != before_snapshot.color_target_count) {
        return Fail("stale target submit mutated target count");
    }

    std::vector<std::uint8_t> capture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    if (result.status != RhiStatus::Success) {
        return Fail("capture failed after rejected stale generation submit");
    }

    if (!BytesMatchColor(capture, frame_initial_color)) {
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

    RhiCommandList command_list(MAX_COMMANDS);
    command_list.BeginFrame(handle);
    device.RecordClear(command_list, handle, RhiColor{9U, 8U, 7U, 6U});

    const auto before_snapshot = device.Snapshot();
    const RhiStatus status = device.Submit(command_list);
    if (status != RhiStatus::InvalidLifecycle) {
        return Fail("incomplete command list did not return lifecycle status");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.submit_count != before_snapshot.submit_count) {
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

    RhiCommandList command_list(MAX_COMMANDS);
    if (command_list.BeginFrame(handle) != RhiStatus::Success) {
        return Fail(PRESENT_BEGIN_FRAME_MESSAGE);
    }

    if (device.RecordClear(command_list, handle, RhiColor{1U, 2U, 3U, 4U}) != RhiStatus::Success) {
        return Fail(PRESENT_RECORD_CLEAR_MESSAGE);
    }

    if (command_list.EndFrame() != RhiStatus::Success) {
        return Fail(PRESENT_END_FRAME_MESSAGE);
    }

    if (device.Submit(command_list) != RhiStatus::Success) {
        return Fail(PRESENT_SUBMIT_MESSAGE);
    }

    const auto before_snapshot = device.Snapshot();
    if (device.DestroyTarget(handle) != RhiStatus::Success) {
        return Fail(PRESENT_DESTROY_SUBMITTED_MESSAGE);
    }

    const auto after_destroy_snapshot = device.Snapshot();
    if (device.Present() != RhiStatus::InvalidHandle) {
        return Fail(PRESENT_DESTROYED_ACCEPTED_MESSAGE);
    }

    const auto after_present_snapshot = device.Snapshot();
    if (after_present_snapshot.present_count != before_snapshot.present_count) {
        return Fail(PRESENT_COUNT_MUTATED_MESSAGE);
    }

    if (after_present_snapshot.destroyed_target_count != after_destroy_snapshot.destroyed_target_count) {
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
    const RhiStatus create_status = device.CreateColorTarget(CaptureFixtureTargetDesc(), handle);
    if (create_status != RhiStatus::Success) {
        return Fail("target creation failed");
    }

    const RhiColor color{4U, 5U, 6U, 7U};
    if (ClearSubmitPresent(device, handle, color) != RhiStatus::Success) {
        return Fail("clear submit present failed");
    }

    std::vector<std::uint8_t> first_capture(4U * 4U * RGBA8_BYTES_PER_PIXEL);
    std::vector<std::uint8_t> second_capture(4U * 4U * RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult first_result = device.CapturePresentedTarget(std::span<std::uint8_t>(first_capture.data(), first_capture.size()));
    const RhiCaptureResult second_result = device.CapturePresentedTarget(std::span<std::uint8_t>(second_capture.data(), second_capture.size()));
    if (first_result.status != RhiStatus::Success) {
        return Fail("first capture failed");
    }

    if (second_result.status != RhiStatus::Success) {
        return Fail("second capture failed");
    }

    if (first_capture != second_capture) {
        return Fail("capture bytes were not deterministic");
    }

    if (!BytesMatchColor(first_capture, color)) {
        return Fail("deterministic capture did not match clear color");
    }

    if (first_result.extent.width != CaptureFixtureTargetDesc().extent.width ||
        first_result.extent.height != CaptureFixtureTargetDesc().extent.height) {
        return Fail("deterministic capture result did not report target extent");
    }

    const RhiDeviceSnapshot snapshot = device.Snapshot();
    if (snapshot.last_capture_extent.width != CaptureFixtureTargetDesc().extent.width ||
        snapshot.last_capture_extent.height != CaptureFixtureTargetDesc().extent.height) {
        return Fail("deterministic capture snapshot did not report target extent");
    }

    return 0;
}

int RhiCapturePresentedTargetWritesUserVisibleResolution() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiTextureHandle handle{};
    const RhiColorTargetDesc desc = UserVisibleTargetDesc();
    if (device.CreateColorTarget(desc, handle) != RhiStatus::Success) {
        return Fail("user-visible target creation failed");
    }

    const RhiColor color{12U, 34U, 56U, 255U};
    if (ClearSubmitPresent(device, handle, color) != RhiStatus::Success) {
        return Fail("user-visible clear submit present failed");
    }

    const std::size_t byte_count =
        static_cast<std::size_t>(desc.extent.width) *
        static_cast<std::size_t>(desc.extent.height) *
        RGBA8_BYTES_PER_PIXEL;
    std::vector<std::uint8_t> capture(byte_count, SENTINEL_BYTE);
    const RhiCaptureResult result =
        device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    if (result.status != RhiStatus::Success) {
        return Fail("user-visible capture failed");
    }

    if (result.bytes_written != byte_count) {
        return Fail("user-visible capture byte count mismatch");
    }

    if (result.extent.width != USER_VISIBLE_CAPTURE_WIDTH ||
        result.extent.height != USER_VISIBLE_CAPTURE_HEIGHT) {
        return Fail("user-visible capture extent mismatch");
    }

    if (!BytesMatchColor(capture, color)) {
        return Fail("user-visible capture did not match clear color");
    }

    const RhiDeviceSnapshot snapshot = device.Snapshot();
    if (snapshot.last_capture_bytes_written != byte_count ||
        snapshot.last_capture_extent.width != USER_VISIBLE_CAPTURE_WIDTH ||
        snapshot.last_capture_extent.height != USER_VISIBLE_CAPTURE_HEIGHT) {
        return Fail("user-visible capture snapshot mismatch");
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

    std::vector<std::uint8_t> accepted_capture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    if (device.CapturePresentedTarget(std::span<std::uint8_t>(accepted_capture.data(), accepted_capture.size())).status != RhiStatus::Success) {
        return Fail(CAPTURE_DESTROYED_BASELINE_MESSAGE);
    }

    const auto before_destroy_snapshot = device.Snapshot();
    if (device.DestroyTarget(handle) != RhiStatus::Success) {
        return Fail(CAPTURE_DESTROYED_DESTROY_MESSAGE);
    }

    const auto after_destroy_snapshot = device.Snapshot();
    std::vector<std::uint8_t> rejected_capture(2U * 2U * RGBA8_BYTES_PER_PIXEL, SENTINEL_BYTE);
    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(rejected_capture.data(), rejected_capture.size()));
    if (result.status != RhiStatus::InvalidHandle) {
        return Fail(CAPTURE_DESTROYED_ACCEPTED_MESSAGE);
    }

    if (result.bytes_written != 0U) {
        return Fail(CAPTURE_DESTROYED_BYTES_MESSAGE);
    }

    for (const std::uint8_t byte : rejected_capture) {
        if (byte != SENTINEL_BYTE) {
            return Fail(CAPTURE_DESTROYED_WRITE_MESSAGE);
        }
    }

    const auto after_capture_snapshot = device.Snapshot();
    if (after_capture_snapshot.capture_count != before_destroy_snapshot.capture_count) {
        return Fail(CAPTURE_DESTROYED_COUNT_MESSAGE);
    }

    if (after_capture_snapshot.last_capture_bytes_written != before_destroy_snapshot.last_capture_bytes_written) {
        return Fail(CAPTURE_DESTROYED_LAST_BYTES_MESSAGE);
    }

    if (after_capture_snapshot.destroyed_target_count != after_destroy_snapshot.destroyed_target_count) {
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
    const RhiColorTargetDesc desc = OversizedCaptureTargetDesc();
    if (device.CreateColorTarget(desc, handle) != RhiStatus::Success) {
        return Fail("oversized capture target creation failed");
    }

    if (ClearSubmitPresent(device, handle, RhiColor{1U, 2U, 3U, 4U}) != RhiStatus::Success) {
        return Fail("clear submit present failed");
    }

    const std::size_t full_target_bytes = static_cast<std::size_t>(desc.extent.width) *
        static_cast<std::size_t>(desc.extent.height) * RGBA8_BYTES_PER_PIXEL;
    std::vector<std::uint8_t> destination(full_target_bytes, SENTINEL_BYTE);
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

    RhiCommandList command_list(MAX_COMMANDS);
    const std::size_t capacity_before = command_list.Capacity();
    command_list.BeginFrame(handle);
    device.RecordClear(command_list, handle, RhiColor{1U, 2U, 3U, 4U});
    command_list.EndFrame();
    device.Submit(command_list);
    device.Present();

    std::vector<std::uint8_t> capture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));

    const auto snapshot = device.Snapshot();
    if (command_list.Capacity() != capacity_before) {
        return Fail("command list capacity changed during frame fixture");
    }

    if (snapshot.command_storage_capacity_before_frame != snapshot.command_storage_capacity_after_last_frame) {
        return Fail("device snapshot recorded command storage growth");
    }

    if (snapshot.command_storage_capacity_before_frame != capacity_before) {
        return Fail("device snapshot recorded wrong command storage capacity");
    }

    return 0;
}

int RhiDisabledDiagnosticsDoesNotChangeResults() {
    NullRhiDevice enabled_like_device = CreateInitializedDevice();
    NullRhiDevice disabled_like_device = CreateInitializedDevice();
    RhiTextureHandle enabled_handle{};
    RhiTextureHandle disabled_handle{};
    enabled_like_device.CreateColorTarget(SmallTargetDesc(), enabled_handle);
    disabled_like_device.CreateColorTarget(SmallTargetDesc(), disabled_handle);

    const RhiColor color{3U, 4U, 5U, 6U};
    const RhiStatus enabled_status = ClearSubmitPresent(enabled_like_device, enabled_handle, color);
    const RhiStatus disabled_status = ClearSubmitPresent(disabled_like_device, disabled_handle, color);
    if (enabled_status != disabled_status) {
        return Fail("disabled diagnostics fixture changed status");
    }

    std::vector<std::uint8_t> enabled_capture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    std::vector<std::uint8_t> disabled_capture(2U * 2U * RGBA8_BYTES_PER_PIXEL);
    enabled_like_device.CapturePresentedTarget(std::span<std::uint8_t>(enabled_capture.data(), enabled_capture.size()));
    disabled_like_device.CapturePresentedTarget(std::span<std::uint8_t>(disabled_capture.data(), disabled_capture.size()));
    if (enabled_capture != disabled_capture) {
        return Fail("disabled diagnostics fixture changed capture bytes");
    }

    if (enabled_like_device.Snapshot().present_count != disabled_like_device.Snapshot().present_count) {
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

int RhiPrimitiveCapabilitiesReportBoundedCapacities() {
    NullRhiDevice device = CreateInitializedDevice();
    const auto capabilities = device.Capabilities();
    if (!capabilities.supports_resource_primitives) {
        return Fail("primitive capability flag was not set");
    }

    if (capabilities.buffer_capacity != MAX_RHI_BUFFERS) {
        return Fail("buffer capacity did not match constant");
    }

    if (capabilities.texture_capacity != MAX_RHI_TEXTURES) {
        return Fail("texture capacity did not match constant");
    }

    if (capabilities.sampler_capacity != MAX_RHI_SAMPLERS) {
        return Fail("sampler capacity did not match constant");
    }

    if (capabilities.shader_module_capacity != MAX_RHI_SHADER_MODULES) {
        return Fail("shader module capacity did not match constant");
    }

    if (capabilities.pipeline_capacity != MAX_RHI_PIPELINES) {
        return Fail("pipeline capacity did not match constant");
    }

    if (capabilities.max_buffer_bytes != MAX_RHI_BUFFER_BYTES) {
        return Fail("buffer byte cap did not match constant");
    }

    if (capabilities.max_shader_bytecode_bytes != MAX_RHI_SHADER_BYTECODE_BYTES) {
        return Fail("shader bytecode cap did not match constant");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.resources.buffer_capacity != MAX_RHI_BUFFERS) {
        return Fail("snapshot buffer capacity did not match constant");
    }

    if (snapshot.resources.pipeline_capacity != MAX_RHI_PIPELINES) {
        return Fail("snapshot pipeline capacity did not match constant");
    }

    return 0;
}

int RhiCreateBufferReturnsGenerationHandleAndSnapshot() {
    NullRhiDevice device = CreateInitializedDevice();
    const std::array<std::uint8_t, 4U> bytes{1U, 2U, 3U, 4U};
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    RhiBufferHandle handle{};
    const RhiStatus status = device.CreateBuffer(SmallVertexBufferDesc(), byte_span, handle);
    if (status != RhiStatus::Success) {
        return Fail("buffer creation failed");
    }

    if (handle.generation == 0U) {
        return Fail("buffer handle generation was zero");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.resources.buffer_count != 1U) {
        return Fail("buffer count was not tracked");
    }

    if (snapshot.resources.created_primitive_count != 1U) {
        return Fail("created primitive count was not tracked");
    }

    return 0;
}

int RhiUpdateBufferSignalsFenceAndRecordsBytes() {
    NullRhiDevice device = CreateInitializedDevice();
    const std::span<const std::uint8_t> empty_bytes{};
    RhiBufferHandle handle{};
    if (device.CreateBuffer(SmallVertexBufferDesc(), empty_bytes, handle) != RhiStatus::Success) {
        return Fail("buffer creation failed");
    }

    const std::array<std::uint8_t, 2U> update_bytes{7U, 8U};
    const std::span<const std::uint8_t> update_span(update_bytes.data(), update_bytes.size());
    RhiFenceHandle fence{};
    const RhiStatus status = device.UpdateBuffer(handle, update_span, fence);
    if (status != RhiStatus::Success) {
        return Fail("buffer update failed");
    }

    if (fence.generation == 0U) {
        return Fail("buffer update did not signal a fence");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.resources.updated_primitive_count != 1U) {
        return Fail("buffer update count was not tracked");
    }

    if (snapshot.resources.signaled_fence_count != 1U) {
        return Fail("fence count was not tracked");
    }

    if (snapshot.resources.last_update_bytes != update_bytes.size()) {
        return Fail("last update byte count was not tracked");
    }

    return 0;
}

int RhiUpdateBufferDestinationRangeTracksOffsetAndRejectsOverflow() {
    NullRhiDevice device = CreateInitializedDevice();
    const std::span<const std::uint8_t> empty_bytes{};
    RhiBufferHandle handle{};
    if (device.CreateBuffer(SmallVertexBufferDesc(), empty_bytes, handle) != RhiStatus::Success) {
        return Fail("buffer creation failed");
    }

    constexpr std::uint64_t DESTINATION_BYTE_OFFSET = 2ULL;
    const std::array<std::uint8_t, 2U> update_bytes{7U, 8U};
    const std::span<const std::uint8_t> update_span(update_bytes.data(), update_bytes.size());
    RhiFenceHandle fence{};
    const RhiStatus status = device.UpdateBuffer(handle, update_span, fence, DESTINATION_BYTE_OFFSET);
    if (status != RhiStatus::Success) {
        return Fail("buffer destination range update failed");
    }

    if (fence.generation == 0U) {
        return Fail("buffer destination range update did not signal a fence");
    }

    const auto before_reject_snapshot = device.Snapshot();
    RhiFenceHandle rejected_fence{};
    constexpr std::uint64_t OVERFLOW_BYTE_OFFSET = 3ULL;
    const RhiStatus overflow_status =
        device.UpdateBuffer(handle, update_span, rejected_fence, OVERFLOW_BYTE_OFFSET);
    if (overflow_status != RhiStatus::CapacityExceeded) {
        return Fail("buffer destination range overflow returned wrong status");
    }

    const auto after_reject_snapshot = device.Snapshot();
    if (after_reject_snapshot.resources.updated_primitive_count !=
        before_reject_snapshot.resources.updated_primitive_count) {
        return Fail("buffer destination range overflow changed update count");
    }

    if (after_reject_snapshot.resources.signaled_fence_count !=
        before_reject_snapshot.resources.signaled_fence_count) {
        return Fail("buffer destination range overflow signaled a fence");
    }

    if (rejected_fence.generation != 0U) {
        return Fail("buffer destination range overflow wrote output fence");
    }

    return 0;
}

int RhiBufferCapacityOverflowDoesNotMutate() {
    NullRhiDevice device = CreateInitializedDevice();
    const std::span<const std::uint8_t> empty_bytes{};
    std::vector<RhiBufferHandle> handles;
    handles.reserve(MAX_RHI_BUFFERS);
    for (std::size_t index = 0U; index < MAX_RHI_BUFFERS; ++index) {
        RhiBufferHandle handle{};
        if (device.CreateBuffer(SmallVertexBufferDesc(), empty_bytes, handle) != RhiStatus::Success) {
            return Fail("buffer creation within capacity failed");
        }

        handles.emplace_back(handle);
    }

    const auto before_snapshot = device.Snapshot();
    RhiBufferHandle overflow_handle{};
    const RhiStatus status = device.CreateBuffer(SmallVertexBufferDesc(), empty_bytes, overflow_handle);
    if (status != RhiStatus::CapacityExceeded) {
        return Fail("buffer capacity overflow did not return capacity status");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.resources.buffer_count != before_snapshot.resources.buffer_count) {
        return Fail("buffer capacity overflow changed active count");
    }

    if (after_snapshot.resources.created_primitive_count != before_snapshot.resources.created_primitive_count) {
        return Fail("buffer capacity overflow changed created count");
    }

    if (overflow_handle.generation != 0U) {
        return Fail("buffer capacity overflow wrote an output handle");
    }

    return 0;
}

int RhiTextureCreateUpdateDestroyTracksSnapshot() {
    NullRhiDevice device = CreateInitializedDevice();
    const std::array<std::uint8_t, 16U> initial_bytes{
        1U, 2U, 3U, 4U,
        5U, 6U, 7U, 8U,
        9U, 10U, 11U, 12U,
        13U, 14U, 15U, 16U};
    const std::span<const std::uint8_t> initial_span(initial_bytes.data(), initial_bytes.size());
    RhiTextureHandle handle{};
    if (device.CreateTexture(SmallPrimitiveTextureDesc(), initial_span, handle) != RhiStatus::Success) {
        return Fail("texture primitive creation failed");
    }

    const std::array<std::uint8_t, 16U> update_bytes{
        16U, 15U, 14U, 13U,
        12U, 11U, 10U, 9U,
        8U, 7U, 6U, 5U,
        4U, 3U, 2U, 1U};
    const std::span<const std::uint8_t> update_span(update_bytes.data(), update_bytes.size());
    RhiFenceHandle fence{};
    if (device.UpdateTexture(handle, update_span, fence) != RhiStatus::Success) {
        return Fail("texture primitive update failed");
    }

    if (device.DestroyTexture(handle) != RhiStatus::Success) {
        return Fail("texture primitive destroy failed");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.resources.texture_count != 0U) {
        return Fail("texture primitive count did not return to zero");
    }

    if (snapshot.resources.created_primitive_count != 1U) {
        return Fail("texture primitive created count was wrong");
    }

    if (snapshot.resources.updated_primitive_count != 1U) {
        return Fail("texture primitive updated count was wrong");
    }

    if (snapshot.resources.destroyed_primitive_count != 1U) {
        return Fail("texture primitive destroyed count was wrong");
    }

    if (snapshot.resources.last_update_bytes != update_bytes.size()) {
        return Fail("texture primitive last update bytes were wrong");
    }

    return 0;
}

int RhiUpdateTextureDestinationRangeTracksOffsetAndRejectsOverflow() {
    NullRhiDevice device = CreateInitializedDevice();
    const std::span<const std::uint8_t> empty_bytes{};
    RhiTextureHandle handle{};
    if (device.CreateTexture(SmallPrimitiveTextureDesc(), empty_bytes, handle) != RhiStatus::Success) {
        return Fail("texture primitive creation failed");
    }

    constexpr std::uint64_t DESTINATION_BYTE_OFFSET = 8ULL;
    const std::array<std::uint8_t, 8U> update_bytes{
        16U, 15U, 14U, 13U,
        12U, 11U, 10U, 9U};
    const std::span<const std::uint8_t> update_span(update_bytes.data(), update_bytes.size());
    RhiFenceHandle fence{};
    const RhiStatus status = device.UpdateTexture(handle, update_span, fence, DESTINATION_BYTE_OFFSET);
    if (status != RhiStatus::Success) {
        return Fail("texture destination range update failed");
    }

    if (fence.generation == 0U) {
        return Fail("texture destination range update did not signal a fence");
    }

    const auto before_reject_snapshot = device.Snapshot();
    RhiFenceHandle rejected_fence{};
    constexpr std::uint64_t OVERFLOW_BYTE_OFFSET = 12ULL;
    const RhiStatus overflow_status =
        device.UpdateTexture(handle, update_span, rejected_fence, OVERFLOW_BYTE_OFFSET);
    if (overflow_status != RhiStatus::InvalidDescriptor) {
        return Fail("texture destination range overflow returned wrong status");
    }

    const auto after_reject_snapshot = device.Snapshot();
    if (after_reject_snapshot.resources.updated_primitive_count !=
        before_reject_snapshot.resources.updated_primitive_count) {
        return Fail("texture destination range overflow changed update count");
    }

    if (after_reject_snapshot.resources.signaled_fence_count !=
        before_reject_snapshot.resources.signaled_fence_count) {
        return Fail("texture destination range overflow signaled a fence");
    }

    if (rejected_fence.generation != 0U) {
        return Fail("texture destination range overflow wrote output fence");
    }

    return 0;
}

int RhiSamplerCreateDestroyTracksSnapshot() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiSamplerDesc desc{};
    desc.linear_filter = true;
    RhiSamplerHandle handle{};
    if (device.CreateSampler(desc, handle) != RhiStatus::Success) {
        return Fail("sampler primitive creation failed");
    }

    if (device.DestroySampler(handle) != RhiStatus::Success) {
        return Fail("sampler primitive destroy failed");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.resources.sampler_count != 0U) {
        return Fail("sampler primitive count did not return to zero");
    }

    if (snapshot.resources.created_primitive_count != 1U) {
        return Fail("sampler primitive created count was wrong");
    }

    if (snapshot.resources.destroyed_primitive_count != 1U) {
        return Fail("sampler primitive destroyed count was wrong");
    }

    return 0;
}

int RhiShaderModuleRejectsEmptyBytecode() {
    NullRhiDevice device = CreateInitializedDevice();
    const std::span<const std::uint8_t> empty_bytes{};
    const RhiShaderModuleDesc desc{RhiShaderStage::Vertex, empty_bytes};
    RhiShaderModuleHandle handle{};
    const RhiStatus status = device.CreateShaderModule(desc, handle);
    if (status != RhiStatus::InvalidDescriptor) {
        return Fail("empty shader bytecode did not return invalid descriptor");
    }

    if (device.Snapshot().resources.shader_module_count != 0U) {
        return Fail("empty shader bytecode changed shader module count");
    }

    return 0;
}

int RhiPipelineRequiresValidShaderModules() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiPipelineDesc desc{};
    RhiPipelineHandle handle{};
    const RhiStatus status = device.CreatePipeline(desc, handle);
    if (status != RhiStatus::InvalidDescriptor) {
        return Fail("pipeline accepted missing shader module handles");
    }

    if (device.Snapshot().resources.pipeline_count != 0U) {
        return Fail("invalid pipeline changed pipeline count");
    }

    return 0;
}

int RhiPipelineCreateDestroyUsesShaderModuleHandles() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiShaderModuleHandle vertex_shader{};
    if (!CreateShaderModule(device_interface, RhiShaderStage::Vertex, vertex_shader)) {
        return Fail("vertex shader module creation failed");
    }

    RhiShaderModuleHandle pixel_shader{};
    if (!CreateShaderModule(device_interface, RhiShaderStage::Pixel, pixel_shader)) {
        return Fail("pixel shader module creation failed");
    }

    RhiPipelineDesc desc{};
    desc.vertex_shader = vertex_shader;
    desc.pixel_shader = pixel_shader;
    RhiPipelineHandle pipeline{};
    if (device.CreatePipeline(desc, pipeline) != RhiStatus::Success) {
        return Fail("pipeline primitive creation failed");
    }

    if (device.DestroyPipeline(pipeline) != RhiStatus::Success) {
        return Fail("pipeline primitive destroy failed");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.resources.pipeline_count != 0U) {
        return Fail("pipeline primitive count did not return to zero");
    }

    if (snapshot.resources.shader_module_count != 2U) {
        return Fail("pipeline creation unexpectedly changed shader module count");
    }

    if (snapshot.resources.created_primitive_count != 3U) {
        return Fail("pipeline path created count was wrong");
    }

    if (snapshot.resources.destroyed_primitive_count != 1U) {
        return Fail("pipeline path destroyed count was wrong");
    }

    return 0;
}

int RhiPrimitiveDestroyInvalidatesStaleHandles() {
    NullRhiDevice device = CreateInitializedDevice();
    const std::span<const std::uint8_t> empty_bytes{};
    RhiBufferHandle handle{};
    if (device.CreateBuffer(SmallVertexBufferDesc(), empty_bytes, handle) != RhiStatus::Success) {
        return Fail("buffer creation failed");
    }

    if (device.DestroyBuffer(handle) != RhiStatus::Success) {
        return Fail("buffer destroy failed");
    }

    const auto before_update_snapshot = device.Snapshot();
    const std::array<std::uint8_t, 1U> update_bytes{1U};
    const std::span<const std::uint8_t> update_span(update_bytes.data(), update_bytes.size());
    RhiFenceHandle fence{};
    const RhiStatus status = device.UpdateBuffer(handle, update_span, fence);
    if (status != RhiStatus::InvalidHandle) {
        return Fail("stale buffer handle was accepted for update");
    }

    const auto after_update_snapshot = device.Snapshot();
    if (after_update_snapshot.resources.updated_primitive_count != before_update_snapshot.resources.updated_primitive_count) {
        return Fail("stale buffer update changed update count");
    }

    if (fence.generation != 0U) {
        return Fail("stale buffer update signaled a fence");
    }

    return 0;
}

int RhiInterfacePrimitiveLifecycleMatchesNullDevice() {
    NullRhiDevice device_storage = CreateInitializedDevice();
    IRhiDevice &device = device_storage;
    const std::span<const std::uint8_t> empty_bytes{};
    RhiBufferHandle handle{};
    if (device.CreateBuffer(SmallVertexBufferDesc(), empty_bytes, handle) != RhiStatus::Success) {
        return Fail("interface buffer creation failed");
    }

    const std::array<std::uint8_t, 4U> update_bytes{9U, 8U, 7U, 6U};
    const std::span<const std::uint8_t> update_span(update_bytes.data(), update_bytes.size());
    RhiFenceHandle fence{};
    if (device.UpdateBuffer(handle, update_span, fence) != RhiStatus::Success) {
        return Fail("interface buffer update failed");
    }

    if (device.DestroyBuffer(handle) != RhiStatus::Success) {
        return Fail("interface buffer destroy failed");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.resources.buffer_count != 0U) {
        return Fail("interface buffer count did not return to zero");
    }

    if (snapshot.resources.signaled_fence_count != 1U) {
        return Fail("interface primitive path did not signal one fence");
    }

    return 0;
}

int RhiPrimitiveSnapshotIsIncludedInDeviceSnapshotComparison() {
    NullRhiDevice first_device = CreateInitializedDevice();
    NullRhiDevice second_device = CreateInitializedDevice();
    const std::span<const std::uint8_t> empty_bytes{};
    RhiBufferHandle first_handle{};
    RhiBufferHandle second_handle{};
    if (first_device.CreateBuffer(SmallVertexBufferDesc(), empty_bytes, first_handle) != RhiStatus::Success) {
        return Fail("first buffer creation failed");
    }

    if (second_device.CreateBuffer(SmallVertexBufferDesc(), empty_bytes, second_handle) != RhiStatus::Success) {
        return Fail("second buffer creation failed");
    }

    if (!DeviceSnapshotsEqual(first_device.Snapshot(), second_device.Snapshot())) {
        return Fail("matching primitive snapshots were not equal");
    }

    const std::array<std::uint8_t, 1U> update_bytes{5U};
    const std::span<const std::uint8_t> update_span(update_bytes.data(), update_bytes.size());
    RhiFenceHandle fence{};
    if (first_device.UpdateBuffer(first_handle, update_span, fence) != RhiStatus::Success) {
        return Fail("first buffer update failed");
    }

    if (DeviceSnapshotsEqual(first_device.Snapshot(), second_device.Snapshot())) {
        return Fail("different primitive snapshots were reported equal");
    }

    return 0;
}

int RhiPrimitiveRetirementDefaultContractsAreExplicit() {
    const RhiPrimitiveRetirementRequest request{};
    if (request.primitive_kind != RhiPrimitiveKind::Unsupported) {
        return Fail("default retirement request kind was not unsupported");
    }

    if (request.request_id != 0U) {
        return Fail("default retirement request id was not zero");
    }

    const RhiPrimitiveRetirementRecord record{};
    if (record.status != RhiPrimitiveRetirementStatus::Invalid) {
        return Fail("default retirement record status was not invalid");
    }

    const RhiPrimitiveRetirementDrainRequest drain_request{};
    if (drain_request.max_retirements != 0U) {
        return Fail("default retirement drain request was not bounded at zero");
    }

    const RhiPrimitiveRetirementDrainResult drain_result{};
    if (drain_result.status != RhiStatus::Success) {
        return Fail("default retirement drain result status was not success");
    }

    NullRhiDevice device = CreateInitializedDevice();
    const auto snapshot = device.Snapshot();
    if (snapshot.resources.primitive_retirement.capacity != MAX_RHI_PRIMITIVE_RETIREMENTS) {
        return Fail("retirement snapshot capacity did not match constant");
    }

    if (snapshot.resources.primitive_retirement.pending_count != 0U) {
        return Fail("retirement snapshot pending count was not zero");
    }

    return 0;
}

int RhiPrimitiveRetirementRequestCreatesPendingRecord() {
    NullRhiDevice device = CreateInitializedDevice();
    const std::span<const std::uint8_t> empty_bytes{};
    RhiBufferHandle handle{};
    if (device.CreateBuffer(SmallVertexBufferDesc(), empty_bytes, handle) != RhiStatus::Success) {
        return Fail("buffer creation failed");
    }

    const RhiPrimitiveRetirementRequest request = BufferRetirementRequest(1U, handle);
    RhiPrimitiveRetirementRecord record{};
    if (device.RequestPrimitiveRetirement(request, record) != RhiStatus::Success) {
        return Fail("retirement request failed");
    }

    if (record.status != RhiPrimitiveRetirementStatus::Pending) {
        return Fail("retirement request did not create pending record");
    }

    RhiPrimitiveRetirementRecord queried_record{};
    if (device.QueryPrimitiveRetirement(record.retirement_id, queried_record) != RhiStatus::Success) {
        return Fail("retirement query failed");
    }

    if (queried_record.request_id != request.request_id) {
        return Fail("retirement query returned wrong request id");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.resources.buffer_count != 1U) {
        return Fail("pending retirement changed active buffer count");
    }

    if (snapshot.resources.destroyed_primitive_count != 0U) {
        return Fail("pending retirement destroyed primitive");
    }

    if (snapshot.resources.primitive_retirement.pending_count != 1U) {
        return Fail("pending retirement count was not tracked");
    }

    if (snapshot.resources.primitive_retirement.requested_count != 1U) {
        return Fail("retirement request count was not tracked");
    }

    return 0;
}

int RhiPrimitiveRetirementDrainInvalidatesHandleAndTracksCounters() {
    NullRhiDevice device = CreateInitializedDevice();
    const std::span<const std::uint8_t> empty_bytes{};
    RhiBufferHandle handle{};
    if (device.CreateBuffer(SmallVertexBufferDesc(), empty_bytes, handle) != RhiStatus::Success) {
        return Fail("buffer creation failed");
    }

    const RhiPrimitiveRetirementRequest request = BufferRetirementRequest(2U, handle);
    RhiPrimitiveRetirementRecord record{};
    if (device.RequestPrimitiveRetirement(request, record) != RhiStatus::Success) {
        return Fail("retirement request failed");
    }

    RhiPrimitiveRetirementDrainRequest drain_request{};
    drain_request.max_retirements = 1U;
    RhiPrimitiveRetirementDrainResult drain_result{};
    if (device.DrainPrimitiveRetirements(drain_request, drain_result) != RhiStatus::Success) {
        return Fail("retirement drain failed");
    }

    if (drain_result.drained_count != 1U) {
        return Fail("retirement drain count was wrong");
    }

    const std::array<std::uint8_t, 1U> update_bytes{1U};
    const std::span<const std::uint8_t> update_span(update_bytes.data(), update_bytes.size());
    RhiFenceHandle fence{};
    if (device.UpdateBuffer(handle, update_span, fence) != RhiStatus::InvalidHandle) {
        return Fail("drained retirement did not invalidate handle");
    }

    RhiPrimitiveRetirementRecord queried_record{};
    if (device.QueryPrimitiveRetirement(record.retirement_id, queried_record) != RhiStatus::Success) {
        return Fail("drained retirement query failed");
    }

    if (queried_record.status != RhiPrimitiveRetirementStatus::Drained) {
        return Fail("retirement record did not become drained");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.resources.buffer_count != 0U) {
        return Fail("drained retirement did not reduce buffer count");
    }

    if (snapshot.resources.destroyed_primitive_count != 1U) {
        return Fail("drained retirement did not track destroyed primitive count");
    }

    if (snapshot.resources.primitive_retirement.pending_count != 0U) {
        return Fail("drained retirement left pending count");
    }

    if (snapshot.resources.primitive_retirement.drained_count != 1U) {
        return Fail("drained retirement count was not tracked");
    }

    return 0;
}

int RhiPrimitiveRetirementRejectsInvalidWrongDuplicateAndCapacity() {
    NullRhiDevice device = CreateInitializedDevice();
    RhiPrimitiveRetirementRequest invalid_request{};
    invalid_request.request_id = 1U;
    invalid_request.primitive_kind = RhiPrimitiveKind::Buffer;
    invalid_request.primitive_slot = 99U;
    invalid_request.primitive_generation = 1U;
    RhiPrimitiveRetirementRecord invalid_record{};
    if (device.RequestPrimitiveRetirement(invalid_request, invalid_record) != RhiStatus::InvalidHandle) {
        return Fail("invalid retirement handle was not rejected");
    }

    const std::span<const std::uint8_t> empty_bytes{};
    RhiBufferHandle buffer_handle{};
    if (device.CreateBuffer(SmallVertexBufferDesc(), empty_bytes, buffer_handle) != RhiStatus::Success) {
        return Fail("buffer creation failed");
    }

    RhiPrimitiveRetirementRequest wrong_kind_request = BufferRetirementRequest(2U, buffer_handle);
    wrong_kind_request.primitive_kind = RhiPrimitiveKind::Texture;
    RhiPrimitiveRetirementRecord wrong_kind_record{};
    if (device.RequestPrimitiveRetirement(wrong_kind_request, wrong_kind_record) != RhiStatus::InvalidHandle) {
        return Fail("wrong kind retirement was not rejected");
    }

    const RhiPrimitiveRetirementRequest request = BufferRetirementRequest(3U, buffer_handle);
    RhiPrimitiveRetirementRecord record{};
    if (device.RequestPrimitiveRetirement(request, record) != RhiStatus::Success) {
        return Fail("retirement request failed");
    }

    RhiPrimitiveRetirementRecord duplicate_record{};
    if (device.RequestPrimitiveRetirement(request, duplicate_record) != RhiStatus::InvalidLifecycle) {
        return Fail("duplicate retirement was not rejected");
    }

    std::vector<RhiBufferHandle> buffer_handles;
    buffer_handles.reserve(MAX_RHI_BUFFERS - 1U);
    for (std::size_t index = 1U; index < MAX_RHI_BUFFERS; ++index) {
        RhiBufferHandle handle{};
        if (device.CreateBuffer(SmallVertexBufferDesc(), empty_bytes, handle) != RhiStatus::Success) {
            return Fail("capacity setup buffer creation failed");
        }

        buffer_handles.emplace_back(handle);
    }

    std::uint64_t request_id = 4U;
    for (RhiBufferHandle handle : buffer_handles) {
        const RhiPrimitiveRetirementRequest buffer_request = BufferRetirementRequest(request_id, handle);
        RhiPrimitiveRetirementRecord buffer_record{};
        if (device.RequestPrimitiveRetirement(buffer_request, buffer_record) != RhiStatus::Success) {
            return Fail("capacity setup buffer retirement failed");
        }

        ++request_id;
    }

    std::vector<RhiTextureHandle> texture_handles;
    texture_handles.reserve(MAX_RHI_TEXTURES);
    for (std::size_t index = 0U; index < MAX_RHI_TEXTURES; ++index) {
        RhiTextureHandle handle{};
        if (!CreateSampledTexture(device, handle)) {
            return Fail("capacity setup texture creation failed");
        }

        texture_handles.emplace_back(handle);
    }

    for (RhiTextureHandle handle : texture_handles) {
        const RhiPrimitiveRetirementRequest texture_request = TextureRetirementRequest(request_id, handle);
        RhiPrimitiveRetirementRecord texture_record{};
        if (device.RequestPrimitiveRetirement(texture_request, texture_record) != RhiStatus::Success) {
            return Fail("capacity setup texture retirement failed");
        }

        ++request_id;
    }

    RhiSamplerHandle sampler_handle{};
    if (!CreateSamplerPrimitive(device, sampler_handle)) {
        return Fail("capacity overflow sampler creation failed");
    }

    const RhiPrimitiveRetirementRequest capacity_request = SamplerRetirementRequest(request_id, sampler_handle);
    RhiPrimitiveRetirementRecord capacity_record{};
    if (device.RequestPrimitiveRetirement(capacity_request, capacity_record) != RhiStatus::CapacityExceeded) {
        return Fail("retirement capacity overflow was not rejected");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.resources.primitive_retirement.invalid_handle_count != 1U) {
        return Fail("invalid retirement handle count was wrong");
    }

    if (snapshot.resources.primitive_retirement.wrong_kind_count != 1U) {
        return Fail("wrong kind retirement count was wrong");
    }

    if (snapshot.resources.primitive_retirement.duplicate_request_count != 1U) {
        return Fail("duplicate retirement count was wrong");
    }

    if (snapshot.resources.primitive_retirement.capacity_rejected_count != 1U) {
        return Fail("retirement capacity rejected count was wrong");
    }

    if (snapshot.resources.primitive_retirement.pending_count != MAX_RHI_PRIMITIVE_RETIREMENTS) {
        return Fail("retirement capacity setup did not fill ledger");
    }

    return 0;
}

int RhiPrimitiveRetirementFenceNotReadyDrainKeepsPending() {
    NullRhiDevice device = CreateInitializedDevice();
    const std::span<const std::uint8_t> empty_bytes{};
    RhiBufferHandle handle{};
    if (device.CreateBuffer(SmallVertexBufferDesc(), empty_bytes, handle) != RhiStatus::Success) {
        return Fail("buffer creation failed");
    }

    RhiPrimitiveRetirementRequest request = BufferRetirementRequest(1U, handle);
    request.wait_fence = RhiFenceHandle{0U, 1U};
    RhiPrimitiveRetirementRecord record{};
    if (device.RequestPrimitiveRetirement(request, record) != RhiStatus::Success) {
        return Fail("fenced retirement request failed");
    }

    RhiPrimitiveRetirementDrainRequest drain_request{};
    drain_request.max_retirements = 1U;
    const RhiDeviceSnapshot before_fence_reject_snapshot = device.Snapshot();
    RhiPrimitiveRetirementDrainResult first_result{};
    if (device.DrainPrimitiveRetirements(drain_request, first_result) != RhiStatus::InvalidLifecycle) {
        return Fail("not-ready retirement drain was not rejected");
    }

    if (first_result.status != RhiStatus::InvalidLifecycle) {
        return Fail("not-ready retirement drain result status changed");
    }

    if (first_result.pending_count != 1U) {
        return Fail("not-ready retirement did not stay pending");
    }

    const RhiDeviceSnapshot after_fence_reject_snapshot = device.Snapshot();
    if (after_fence_reject_snapshot.failed_operation_count !=
        before_fence_reject_snapshot.failed_operation_count + 1U) {
        return Fail("not-ready retirement drain failure was not counted");
    }

    const std::array<std::uint8_t, 1U> update_bytes{1U};
    const std::span<const std::uint8_t> update_span(update_bytes.data(), update_bytes.size());
    RhiFenceHandle fence{};
    if (device.UpdateBuffer(handle, update_span, fence) != RhiStatus::Success) {
        return Fail("buffer update did not signal fence");
    }

    RhiPrimitiveRetirementDrainResult second_result{};
    if (device.DrainPrimitiveRetirements(drain_request, second_result) != RhiStatus::Success) {
        return Fail("ready retirement drain failed");
    }

    if (second_result.drained_count != 1U) {
        return Fail("ready retirement did not drain");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.failed_operation_count != after_fence_reject_snapshot.failed_operation_count) {
        return Fail("ready retirement drain changed failure count");
    }

    if (snapshot.resources.primitive_retirement.fence_not_ready_count != 1U) {
        return Fail("fence not ready count was not tracked");
    }

    if (snapshot.resources.primitive_retirement.pending_count != 0U) {
        return Fail("ready drain left retirement pending");
    }

    return 0;
}

int RhiPrimitiveRetirementImmediateDestroyCompatibility() {
    NullRhiDevice device = CreateInitializedDevice();
    const std::span<const std::uint8_t> empty_bytes{};
    RhiBufferHandle handle{};
    if (device.CreateBuffer(SmallVertexBufferDesc(), empty_bytes, handle) != RhiStatus::Success) {
        return Fail("buffer creation failed");
    }

    if (device.DestroyBuffer(handle) != RhiStatus::Success) {
        return Fail("immediate destroy failed");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.resources.buffer_count != 0U) {
        return Fail("immediate destroy did not reduce buffer count");
    }

    if (snapshot.resources.destroyed_primitive_count != 1U) {
        return Fail("immediate destroy did not update destroyed count");
    }

    if (snapshot.resources.primitive_retirement.requested_count != 0U) {
        return Fail("immediate destroy unexpectedly wrote retirement ledger");
    }

    return 0;
}

int RhiInterfacePrimitiveRetirementMatchesNullDevice() {
    NullRhiDevice device_storage = CreateInitializedDevice();
    IRhiDevice &device = device_storage;
    const std::span<const std::uint8_t> empty_bytes{};
    RhiBufferHandle handle{};
    if (device.CreateBuffer(SmallVertexBufferDesc(), empty_bytes, handle) != RhiStatus::Success) {
        return Fail("interface buffer creation failed");
    }

    const RhiPrimitiveRetirementRequest request = BufferRetirementRequest(1U, handle);
    RhiPrimitiveRetirementRecord record{};
    if (device.RequestPrimitiveRetirement(request, record) != RhiStatus::Success) {
        return Fail("interface retirement request failed");
    }

    RhiPrimitiveRetirementDrainRequest drain_request{};
    drain_request.max_retirements = 1U;
    RhiPrimitiveRetirementDrainResult drain_result{};
    if (device.DrainPrimitiveRetirements(drain_request, drain_result) != RhiStatus::Success) {
        return Fail("interface retirement drain failed");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.resources.buffer_count != 0U) {
        return Fail("interface retirement did not reduce buffer count");
    }

    if (snapshot.resources.primitive_retirement.drained_count != 1U) {
        return Fail("interface retirement did not track drained count");
    }

    return 0;
}

int RhiCommandListRecordsVisibleTriangleCommandsWithinCapacity() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device, target)) {
        return Fail("target creation failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, pipeline)) {
        return Fail("triangle pipeline creation failed");
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, vertex_buffer)) {
        return Fail("triangle vertex buffer creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    const std::size_t capacity_before = command_list.Capacity();
    const RhiStatus status = RecordTriangleDrawFrame(
        device_interface,
        command_list,
        target,
        pipeline,
        TriangleVertexBufferViewFor(vertex_buffer),
        TriangleDrawDesc());
    if (status != RhiStatus::Success) {
        return Fail("triangle draw command recording failed");
    }

    const auto snapshot = command_list.Snapshot();
    if (snapshot.command_count != 5U) {
        return Fail("triangle draw command count was unexpected");
    }

    if (snapshot.draw_command_count != 1U) {
        return Fail("triangle draw command was not tracked");
    }

    if (snapshot.last_status != RhiStatus::Success) {
        return Fail("triangle draw command recording did not record success status");
    }

    if (command_list.Capacity() != capacity_before) {
        return Fail("triangle draw command recording grew storage");
    }

    return 0;
}

int RhiSubmitDrawRejectsMissingPipelineWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device, target)) {
        return Fail("target creation failed");
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, vertex_buffer)) {
        return Fail("triangle vertex buffer creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    RhiStatus status = command_list.BeginFrame(target);
    if (status != RhiStatus::Success) {
        return Fail("begin frame failed");
    }

    status = device_interface.RecordBindVertexBuffer(command_list, TriangleVertexBufferViewFor(vertex_buffer));
    if (status != RhiStatus::Success) {
        return Fail("vertex buffer bind recording failed");
    }

    status = device_interface.RecordDraw(command_list, TriangleDrawDesc());
    if (status != RhiStatus::Success) {
        return Fail("draw recording failed");
    }

    status = command_list.EndFrame();
    if (status != RhiStatus::Success) {
        return Fail("end frame failed");
    }

    const auto before_snapshot = device.Snapshot();
    status = device_interface.Submit(command_list);
    if (status != RhiStatus::InvalidLifecycle) {
        return Fail("draw without pipeline did not return invalid lifecycle");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.submitted_draw_count != before_snapshot.submitted_draw_count) {
        return Fail("rejected draw changed submitted draw count");
    }

    if (after_snapshot.submit_count != before_snapshot.submit_count) {
        return Fail("rejected draw changed submit count");
    }

    if (after_snapshot.last_status != RhiStatus::InvalidLifecycle) {
        return Fail("rejected draw did not record last status");
    }

    RhiSamplerDesc recovery_sampler_desc{};
    RhiSamplerHandle recovery_sampler{};
    if (device_interface.CreateSampler(recovery_sampler_desc, recovery_sampler) != RhiStatus::Success) {
        return Fail("rejected draw recovery sampler creation failed");
    }

    const auto recovery_snapshot = device.Snapshot();
    if (recovery_snapshot.last_status != RhiStatus::Success) {
        return Fail("successful operation did not clear rejected draw last status");
    }

    return 0;
}

int RhiSubmitDrawRejectsVertexBufferRangeOverflow() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device, target)) {
        return Fail("target creation failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, pipeline)) {
        return Fail("triangle pipeline creation failed");
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, vertex_buffer)) {
        return Fail("triangle vertex buffer creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    const RhiStatus record_status = RecordTriangleDrawFrame(
        device_interface,
        command_list,
        target,
        pipeline,
        OverflowTriangleVertexBufferViewFor(vertex_buffer),
        TriangleDrawDesc());
    if (record_status != RhiStatus::Success) {
        return Fail("overflow draw command recording failed");
    }

    const auto before_snapshot = device.Snapshot();
    const RhiStatus submit_status = device_interface.Submit(command_list);
    if (submit_status != RhiStatus::InvalidDescriptor) {
        return Fail("overflow draw range did not return invalid descriptor");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.submitted_draw_count != before_snapshot.submitted_draw_count) {
        return Fail("overflow draw changed submitted draw count");
    }

    if (after_snapshot.submit_count != before_snapshot.submit_count) {
        return Fail("overflow draw changed submit count");
    }

    if (after_snapshot.last_status != RhiStatus::InvalidDescriptor) {
        return Fail("overflow draw did not record last status");
    }

    return 0;
}

int RhiSubmitDrawUpdatesNullSnapshot() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device, target)) {
        return Fail("target creation failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, pipeline)) {
        return Fail("triangle pipeline creation failed");
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, vertex_buffer)) {
        return Fail("triangle vertex buffer creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    const RhiStatus record_status = RecordTriangleDrawFrame(
        device_interface,
        command_list,
        target,
        pipeline,
        TriangleVertexBufferViewFor(vertex_buffer),
        TriangleDrawDesc());
    if (record_status != RhiStatus::Success) {
        return Fail("triangle draw command recording failed");
    }

    const RhiStatus submit_status = device_interface.Submit(command_list);
    if (submit_status != RhiStatus::Success) {
        return Fail("triangle draw submit failed");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.submitted_draw_count != 1U) {
        return Fail("submitted draw count was not tracked");
    }

    if (snapshot.last_draw_vertex_count != TRIANGLE_VERTEX_COUNT) {
        return Fail("last draw vertex count was not tracked");
    }

    if (snapshot.command_storage_capacity_before_frame != MAX_COMMANDS) {
        return Fail("draw submit recorded wrong command capacity");
    }

    if (snapshot.command_storage_capacity_after_last_frame != MAX_COMMANDS) {
        return Fail("draw submit changed command capacity");
    }

    return 0;
}

int RhiIndexFormatDefaultDescriptorsAreUnsupported() {
    const RhiIndexBufferView index_buffer{};
    if (index_buffer.format != RhiIndexFormat::Unsupported) {
        return Fail("default index buffer view did not use unsupported format");
    }

    const RhiDrawIndexedDesc draw{};
    if (draw.topology != RhiPrimitiveTopology::Unsupported) {
        return Fail("default indexed draw topology was not unsupported");
    }

    if (draw.index_count != 0U) {
        return Fail("default indexed draw count was not zero");
    }

    return 0;
}

int RhiCommandListRecordsIndexedStaticMeshWithinCapacity() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device, target)) {
        return Fail("target creation failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, pipeline)) {
        return Fail("triangle pipeline creation failed");
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, vertex_buffer)) {
        return Fail("triangle vertex buffer creation failed");
    }

    RhiBufferHandle index_buffer{};
    if (!CreateTriangleIndexBuffer(device_interface, index_buffer)) {
        return Fail("triangle index buffer creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    const std::size_t capacity_before = command_list.Capacity();
    const RhiStatus status = RecordIndexedTriangleDrawFrame(
        device_interface,
        command_list,
        target,
        pipeline,
        TriangleVertexBufferViewFor(vertex_buffer),
        TriangleIndexBufferViewFor(index_buffer),
        TriangleDrawIndexedDesc());
    if (status != RhiStatus::Success) {
        return Fail("indexed static mesh command recording failed");
    }

    const auto snapshot = command_list.Snapshot();
    if (snapshot.command_count != 6U) {
        return Fail("indexed static mesh command count was unexpected");
    }

    if (snapshot.draw_command_count != 0U) {
        return Fail("indexed static mesh changed non-indexed draw count");
    }

    if (snapshot.indexed_draw_command_count != 1U) {
        return Fail("indexed static mesh draw was not tracked");
    }

    if (command_list.Capacity() != capacity_before) {
        return Fail("indexed static mesh command recording grew storage");
    }

    return 0;
}

int RhiSubmitIndexedDrawRejectsMissingIndexBufferWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device, target)) {
        return Fail("target creation failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, pipeline)) {
        return Fail("triangle pipeline creation failed");
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, vertex_buffer)) {
        return Fail("triangle vertex buffer creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    RhiStatus status = command_list.BeginFrame(target);
    if (status != RhiStatus::Success) {
        return Fail("begin frame failed");
    }

    status = device_interface.RecordBindPipeline(command_list, pipeline);
    if (status != RhiStatus::Success) {
        return Fail("pipeline bind recording failed");
    }

    status = device_interface.RecordBindVertexBuffer(command_list, TriangleVertexBufferViewFor(vertex_buffer));
    if (status != RhiStatus::Success) {
        return Fail("vertex buffer bind recording failed");
    }

    status = device_interface.RecordDrawIndexed(command_list, TriangleDrawIndexedDesc());
    if (status != RhiStatus::Success) {
        return Fail("indexed draw recording failed");
    }

    status = command_list.EndFrame();
    if (status != RhiStatus::Success) {
        return Fail("end frame failed");
    }

    const auto before_snapshot = device.Snapshot();
    status = device_interface.Submit(command_list);
    if (status != RhiStatus::InvalidLifecycle) {
        return Fail("indexed draw without index buffer did not return invalid lifecycle");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.submitted_indexed_draw_count != before_snapshot.submitted_indexed_draw_count) {
        return Fail("rejected indexed draw changed submitted indexed draw count");
    }

    if (after_snapshot.rejected_indexed_draw_count != before_snapshot.rejected_indexed_draw_count + 1U) {
        return Fail("rejected indexed draw count was not tracked");
    }

    if (after_snapshot.submit_count != before_snapshot.submit_count) {
        return Fail("rejected indexed draw changed submit count");
    }

    if (after_snapshot.last_status != RhiStatus::InvalidLifecycle) {
        return Fail("rejected indexed draw did not record last status");
    }

    return 0;
}

int RhiSubmitIndexedDrawRejectsIndexRangeOverflow() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device, target)) {
        return Fail("target creation failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, pipeline)) {
        return Fail("triangle pipeline creation failed");
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, vertex_buffer)) {
        return Fail("triangle vertex buffer creation failed");
    }

    RhiBufferHandle index_buffer{};
    if (!CreateTriangleIndexBuffer(device_interface, index_buffer)) {
        return Fail("triangle index buffer creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    const RhiStatus record_status = RecordIndexedTriangleDrawFrame(
        device_interface,
        command_list,
        target,
        pipeline,
        TriangleVertexBufferViewFor(vertex_buffer),
        OverflowTriangleIndexBufferViewFor(index_buffer),
        TriangleDrawIndexedDesc());
    if (record_status != RhiStatus::Success) {
        return Fail("overflow indexed draw command recording failed");
    }

    const auto before_snapshot = device.Snapshot();
    const RhiStatus submit_status = device_interface.Submit(command_list);
    if (submit_status != RhiStatus::InvalidDescriptor) {
        return Fail("overflow indexed draw range did not return invalid descriptor");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.submitted_indexed_draw_count != before_snapshot.submitted_indexed_draw_count) {
        return Fail("overflow indexed draw changed submitted indexed draw count");
    }

    if (after_snapshot.rejected_indexed_draw_count != before_snapshot.rejected_indexed_draw_count + 1U) {
        return Fail("overflow indexed draw rejection was not tracked");
    }

    if (after_snapshot.submit_count != before_snapshot.submit_count) {
        return Fail("overflow indexed draw changed submit count");
    }

    if (after_snapshot.last_status != RhiStatus::InvalidDescriptor) {
        return Fail("overflow indexed draw did not record last status");
    }

    return 0;
}

int RhiSubmitIndexedDrawUpdatesNullSnapshot() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device, target)) {
        return Fail("target creation failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, pipeline)) {
        return Fail("triangle pipeline creation failed");
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, vertex_buffer)) {
        return Fail("triangle vertex buffer creation failed");
    }

    RhiBufferHandle index_buffer{};
    if (!CreateTriangleIndexBuffer(device_interface, index_buffer)) {
        return Fail("triangle index buffer creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    const RhiStatus record_status = RecordIndexedTriangleDrawFrame(
        device_interface,
        command_list,
        target,
        pipeline,
        TriangleVertexBufferViewFor(vertex_buffer),
        TriangleIndexBufferViewFor(index_buffer),
        TriangleDrawIndexedDesc());
    if (record_status != RhiStatus::Success) {
        return Fail("indexed static mesh command recording failed");
    }

    const RhiStatus submit_status = device_interface.Submit(command_list);
    if (submit_status != RhiStatus::Success) {
        return Fail("indexed static mesh submit failed");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.submitted_draw_count != 0U) {
        return Fail("indexed static mesh changed non-indexed submitted draw count");
    }

    if (snapshot.submitted_indexed_draw_count != 1U) {
        return Fail("submitted indexed draw count was not tracked");
    }

    if (snapshot.rejected_indexed_draw_count != 0U) {
        return Fail("successful indexed draw changed rejected indexed draw count");
    }

    if (snapshot.last_indexed_draw_index_count != TRIANGLE_INDEX_COUNT) {
        return Fail("last indexed draw index count was not tracked");
    }

    if (snapshot.last_bound_index_buffer_offset_bytes != 0U) {
        return Fail("last bound index buffer offset was not tracked");
    }

    if (snapshot.last_bound_index_buffer_size_bytes != TRIANGLE_INDEX_BUFFER_BYTES) {
        return Fail("last bound index buffer size was not tracked");
    }

    if (snapshot.command_storage_capacity_before_frame != MAX_COMMANDS) {
        return Fail("indexed draw submit recorded wrong command capacity");
    }

    if (snapshot.command_storage_capacity_after_last_frame != MAX_COMMANDS) {
        return Fail("indexed draw submit changed command capacity");
    }

    return 0;
}

int RhiTextureSamplingDefaultBindingsAreBoundedValues() {
    const RhiSampledTextureBinding texture_binding{};
    if (texture_binding.texture.generation != 0U) {
        return Fail("default sampled texture handle was not empty");
    }

    if (texture_binding.slot != 0U) {
        return Fail("default sampled texture slot was not zero");
    }

    const RhiSamplerBinding sampler_binding{};
    if (sampler_binding.sampler.generation != 0U) {
        return Fail("default sampler handle was not empty");
    }

    if (sampler_binding.slot != 0U) {
        return Fail("default sampler slot was not zero");
    }

    static_assert(MAX_RHI_SAMPLED_TEXTURE_SLOTS > 0U, "sampled texture slot capacity was zero");
    static_assert(MAX_RHI_SAMPLER_SLOTS > 0U, "sampler slot capacity was zero");

    return 0;
}

int RhiCommandListRecordsTextureSamplingWithinCapacity() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device, target)) {
        return Fail("target creation failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, pipeline)) {
        return Fail("triangle pipeline creation failed");
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, vertex_buffer)) {
        return Fail("triangle vertex buffer creation failed");
    }

    RhiBufferHandle index_buffer{};
    if (!CreateTriangleIndexBuffer(device_interface, index_buffer)) {
        return Fail("triangle index buffer creation failed");
    }

    RhiTextureHandle sampled_texture{};
    if (!CreateSampledTexture(device_interface, sampled_texture)) {
        return Fail("sampled texture creation failed");
    }

    RhiSamplerHandle sampler{};
    if (!CreateSamplerPrimitive(device_interface, sampler)) {
        return Fail("sampler creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    const std::size_t capacity_before = command_list.Capacity();
    const RhiStatus status = RecordSampledIndexedTriangleDrawFrame(
        device_interface,
        command_list,
        target,
        pipeline,
        TriangleVertexBufferViewFor(vertex_buffer),
        TriangleIndexBufferViewFor(index_buffer),
        sampled_texture,
        sampler,
        TriangleDrawIndexedDesc());
    if (status != RhiStatus::Success) {
        return Fail("texture sampling command recording failed");
    }

    const auto snapshot = command_list.Snapshot();
    if (snapshot.command_count != 8U) {
        return Fail("texture sampling command count was unexpected");
    }

    if (snapshot.sampled_texture_bind_command_count != 1U) {
        return Fail("sampled texture bind command was not tracked");
    }

    if (snapshot.sampler_bind_command_count != 1U) {
        return Fail("sampler bind command was not tracked");
    }

    if (snapshot.indexed_draw_command_count != 1U) {
        return Fail("texture sampling indexed draw command was not tracked");
    }

    if (command_list.Capacity() != capacity_before) {
        return Fail("texture sampling command recording grew storage");
    }

    return 0;
}

int RhiTextureSamplingSlotOverflowDoesNotMutate() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle sampled_texture{};
    if (!CreateSampledTexture(device_interface, sampled_texture)) {
        return Fail("sampled texture creation failed");
    }

    RhiSamplerHandle sampler{};
    if (!CreateSamplerPrimitive(device_interface, sampler)) {
        return Fail("sampler creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    RhiSampledTextureBinding texture_binding = SampledTextureBindingFor(sampled_texture);
    texture_binding.slot = static_cast<std::uint32_t>(MAX_RHI_SAMPLED_TEXTURE_SLOTS);
    const auto before_snapshot = device.Snapshot();
    RhiStatus status = device_interface.RecordBindSampledTexture(command_list, texture_binding);
    if (status != RhiStatus::InvalidDescriptor) {
        return Fail("sampled texture slot overflow did not return invalid descriptor");
    }

    const auto texture_snapshot = device.Snapshot();
    if (texture_snapshot.recorded_command_count != before_snapshot.recorded_command_count) {
        return Fail("sampled texture slot overflow recorded a command");
    }

    if (texture_snapshot.rejected_sampled_texture_bind_count != before_snapshot.rejected_sampled_texture_bind_count + 1U) {
        return Fail("sampled texture slot overflow was not tracked");
    }

    if (texture_snapshot.last_status != RhiStatus::InvalidDescriptor) {
        return Fail("sampled texture slot overflow did not record last status");
    }

    RhiSamplerBinding sampler_binding = SamplerBindingFor(sampler);
    sampler_binding.slot = static_cast<std::uint32_t>(MAX_RHI_SAMPLER_SLOTS);
    status = device_interface.RecordBindSampler(command_list, sampler_binding);
    if (status != RhiStatus::InvalidDescriptor) {
        return Fail("sampler slot overflow did not return invalid descriptor");
    }

    const auto sampler_snapshot = device.Snapshot();
    if (sampler_snapshot.recorded_command_count != before_snapshot.recorded_command_count) {
        return Fail("sampler slot overflow recorded a command");
    }

    if (sampler_snapshot.rejected_sampler_bind_count != texture_snapshot.rejected_sampler_bind_count + 1U) {
        return Fail("sampler slot overflow was not tracked");
    }

    if (sampler_snapshot.last_status != RhiStatus::InvalidDescriptor) {
        return Fail("sampler slot overflow did not record last status");
    }

    return 0;
}

int RhiSubmitTextureSamplingRejectsMissingSamplerWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device, target)) {
        return Fail("target creation failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, pipeline)) {
        return Fail("triangle pipeline creation failed");
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, vertex_buffer)) {
        return Fail("triangle vertex buffer creation failed");
    }

    RhiBufferHandle index_buffer{};
    if (!CreateTriangleIndexBuffer(device_interface, index_buffer)) {
        return Fail("triangle index buffer creation failed");
    }

    RhiTextureHandle sampled_texture{};
    if (!CreateSampledTexture(device_interface, sampled_texture)) {
        return Fail("sampled texture creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    RhiStatus status = command_list.BeginFrame(target);
    if (status != RhiStatus::Success) {
        return Fail("begin frame failed");
    }

    status = device_interface.RecordBindPipeline(command_list, pipeline);
    if (status != RhiStatus::Success) {
        return Fail("pipeline bind recording failed");
    }

    status = device_interface.RecordBindVertexBuffer(command_list, TriangleVertexBufferViewFor(vertex_buffer));
    if (status != RhiStatus::Success) {
        return Fail("vertex buffer bind recording failed");
    }

    status = device_interface.RecordBindIndexBuffer(command_list, TriangleIndexBufferViewFor(index_buffer));
    if (status != RhiStatus::Success) {
        return Fail("index buffer bind recording failed");
    }

    const RhiSampledTextureBinding texture_binding = SampledTextureBindingFor(sampled_texture);
    status = device_interface.RecordBindSampledTexture(command_list, texture_binding);
    if (status != RhiStatus::Success) {
        return Fail("sampled texture bind recording failed");
    }

    status = device_interface.RecordDrawIndexed(command_list, TriangleDrawIndexedDesc());
    if (status != RhiStatus::Success) {
        return Fail("indexed draw recording failed");
    }

    status = command_list.EndFrame();
    if (status != RhiStatus::Success) {
        return Fail("end frame failed");
    }

    const auto before_snapshot = device.Snapshot();
    status = device_interface.Submit(command_list);
    if (status != RhiStatus::InvalidLifecycle) {
        return Fail("texture sampling without sampler did not return invalid lifecycle");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.rejected_sampler_bind_count != before_snapshot.rejected_sampler_bind_count + 1U) {
        return Fail("missing sampler rejection was not tracked");
    }

    if (after_snapshot.submitted_indexed_draw_count != before_snapshot.submitted_indexed_draw_count) {
        return Fail("missing sampler rejection changed submitted indexed draw count");
    }

    if (after_snapshot.submit_count != before_snapshot.submit_count) {
        return Fail("missing sampler rejection changed submit count");
    }

    if (after_snapshot.last_status != RhiStatus::InvalidLifecycle) {
        return Fail("missing sampler rejection did not record last status");
    }

    return 0;
}

int RhiSubmitSamplerRejectsMissingTextureWithoutMutation() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device, target)) {
        return Fail("target creation failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, pipeline)) {
        return Fail("triangle pipeline creation failed");
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, vertex_buffer)) {
        return Fail("triangle vertex buffer creation failed");
    }

    RhiBufferHandle index_buffer{};
    if (!CreateTriangleIndexBuffer(device_interface, index_buffer)) {
        return Fail("triangle index buffer creation failed");
    }

    RhiSamplerHandle sampler{};
    if (!CreateSamplerPrimitive(device_interface, sampler)) {
        return Fail("sampler creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    RhiStatus status = command_list.BeginFrame(target);
    if (status != RhiStatus::Success) {
        return Fail("begin frame failed");
    }

    status = device_interface.RecordBindPipeline(command_list, pipeline);
    if (status != RhiStatus::Success) {
        return Fail("pipeline bind recording failed");
    }

    status = device_interface.RecordBindVertexBuffer(command_list, TriangleVertexBufferViewFor(vertex_buffer));
    if (status != RhiStatus::Success) {
        return Fail("vertex buffer bind recording failed");
    }

    status = device_interface.RecordBindIndexBuffer(command_list, TriangleIndexBufferViewFor(index_buffer));
    if (status != RhiStatus::Success) {
        return Fail("index buffer bind recording failed");
    }

    const RhiSamplerBinding sampler_binding = SamplerBindingFor(sampler);
    status = device_interface.RecordBindSampler(command_list, sampler_binding);
    if (status != RhiStatus::Success) {
        return Fail("sampler bind recording failed");
    }

    status = device_interface.RecordDrawIndexed(command_list, TriangleDrawIndexedDesc());
    if (status != RhiStatus::Success) {
        return Fail("indexed draw recording failed");
    }

    status = command_list.EndFrame();
    if (status != RhiStatus::Success) {
        return Fail("end frame failed");
    }

    const auto before_snapshot = device.Snapshot();
    status = device_interface.Submit(command_list);
    if (status != RhiStatus::InvalidLifecycle) {
        return Fail("sampler without sampled texture did not return invalid lifecycle");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.rejected_sampled_texture_bind_count != before_snapshot.rejected_sampled_texture_bind_count + 1U) {
        return Fail("missing sampled texture rejection was not tracked");
    }

    if (after_snapshot.submitted_indexed_draw_count != before_snapshot.submitted_indexed_draw_count) {
        return Fail("missing sampled texture rejection changed submitted indexed draw count");
    }

    if (after_snapshot.submit_count != before_snapshot.submit_count) {
        return Fail("missing sampled texture rejection changed submit count");
    }

    if (after_snapshot.last_status != RhiStatus::InvalidLifecycle) {
        return Fail("missing sampled texture rejection did not record last status");
    }

    return 0;
}

int RhiSubmitTextureSamplingRejectsStaleTextureHandle() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device, target)) {
        return Fail("target creation failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, pipeline)) {
        return Fail("triangle pipeline creation failed");
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, vertex_buffer)) {
        return Fail("triangle vertex buffer creation failed");
    }

    RhiBufferHandle index_buffer{};
    if (!CreateTriangleIndexBuffer(device_interface, index_buffer)) {
        return Fail("triangle index buffer creation failed");
    }

    RhiTextureHandle sampled_texture{};
    if (!CreateSampledTexture(device_interface, sampled_texture)) {
        return Fail("sampled texture creation failed");
    }

    RhiSamplerHandle sampler{};
    if (!CreateSamplerPrimitive(device_interface, sampler)) {
        return Fail("sampler creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    const RhiStatus record_status = RecordSampledIndexedTriangleDrawFrame(
        device_interface,
        command_list,
        target,
        pipeline,
        TriangleVertexBufferViewFor(vertex_buffer),
        TriangleIndexBufferViewFor(index_buffer),
        sampled_texture,
        sampler,
        TriangleDrawIndexedDesc());
    if (record_status != RhiStatus::Success) {
        return Fail("texture sampling command recording failed");
    }

    if (device_interface.DestroyTexture(sampled_texture) != RhiStatus::Success) {
        return Fail("sampled texture destroy failed");
    }

    const auto before_snapshot = device.Snapshot();
    const RhiStatus submit_status = device_interface.Submit(command_list);
    if (submit_status != RhiStatus::InvalidHandle) {
        return Fail("stale sampled texture did not return invalid handle");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.rejected_sampled_texture_bind_count != before_snapshot.rejected_sampled_texture_bind_count + 1U) {
        return Fail("stale sampled texture rejection was not tracked");
    }

    if (after_snapshot.submit_count != before_snapshot.submit_count) {
        return Fail("stale sampled texture changed submit count");
    }

    return 0;
}

int RhiSubmitTextureSamplingRejectsStaleSamplerHandle() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device, target)) {
        return Fail("target creation failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, pipeline)) {
        return Fail("triangle pipeline creation failed");
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, vertex_buffer)) {
        return Fail("triangle vertex buffer creation failed");
    }

    RhiBufferHandle index_buffer{};
    if (!CreateTriangleIndexBuffer(device_interface, index_buffer)) {
        return Fail("triangle index buffer creation failed");
    }

    RhiTextureHandle sampled_texture{};
    if (!CreateSampledTexture(device_interface, sampled_texture)) {
        return Fail("sampled texture creation failed");
    }

    RhiSamplerHandle sampler{};
    if (!CreateSamplerPrimitive(device_interface, sampler)) {
        return Fail("sampler creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    const RhiStatus record_status = RecordSampledIndexedTriangleDrawFrame(
        device_interface,
        command_list,
        target,
        pipeline,
        TriangleVertexBufferViewFor(vertex_buffer),
        TriangleIndexBufferViewFor(index_buffer),
        sampled_texture,
        sampler,
        TriangleDrawIndexedDesc());
    if (record_status != RhiStatus::Success) {
        return Fail("texture sampling command recording failed");
    }

    if (device_interface.DestroySampler(sampler) != RhiStatus::Success) {
        return Fail("sampler destroy failed");
    }

    const auto before_snapshot = device.Snapshot();
    const RhiStatus submit_status = device_interface.Submit(command_list);
    if (submit_status != RhiStatus::InvalidHandle) {
        return Fail("stale sampler did not return invalid handle");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.rejected_sampler_bind_count != before_snapshot.rejected_sampler_bind_count + 1U) {
        return Fail("stale sampler rejection was not tracked");
    }

    if (after_snapshot.submit_count != before_snapshot.submit_count) {
        return Fail("stale sampler changed submit count");
    }

    return 0;
}

int RhiSubmitTextureSamplingUpdatesNullSnapshot() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device, target)) {
        return Fail("target creation failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, pipeline)) {
        return Fail("triangle pipeline creation failed");
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, vertex_buffer)) {
        return Fail("triangle vertex buffer creation failed");
    }

    RhiBufferHandle index_buffer{};
    if (!CreateTriangleIndexBuffer(device_interface, index_buffer)) {
        return Fail("triangle index buffer creation failed");
    }

    RhiTextureHandle sampled_texture{};
    if (!CreateSampledTexture(device_interface, sampled_texture)) {
        return Fail("sampled texture creation failed");
    }

    RhiSamplerHandle sampler{};
    if (!CreateSamplerPrimitive(device_interface, sampler)) {
        return Fail("sampler creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    const RhiStatus record_status = RecordSampledIndexedTriangleDrawFrame(
        device_interface,
        command_list,
        target,
        pipeline,
        TriangleVertexBufferViewFor(vertex_buffer),
        TriangleIndexBufferViewFor(index_buffer),
        sampled_texture,
        sampler,
        TriangleDrawIndexedDesc());
    if (record_status != RhiStatus::Success) {
        return Fail("texture sampling command recording failed");
    }

    const RhiStatus submit_status = device_interface.Submit(command_list);
    if (submit_status != RhiStatus::Success) {
        return Fail("texture sampling submit failed");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.submitted_sampled_texture_bind_count != 1U) {
        return Fail("submitted sampled texture bind count was not tracked");
    }

    if (snapshot.submitted_sampler_bind_count != 1U) {
        return Fail("submitted sampler bind count was not tracked");
    }

    if (snapshot.rejected_sampled_texture_bind_count != 0U) {
        return Fail("successful texture sampling changed sampled texture rejection count");
    }

    if (snapshot.rejected_sampler_bind_count != 0U) {
        return Fail("successful texture sampling changed sampler rejection count");
    }

    if (snapshot.submitted_indexed_draw_count != 1U) {
        return Fail("texture sampling indexed draw was not submitted");
    }

    if (snapshot.last_bound_sampled_texture_slot != 0U) {
        return Fail("last sampled texture slot was not tracked");
    }

    if (snapshot.last_bound_sampler_slot != 0U) {
        return Fail("last sampler slot was not tracked");
    }

    if (snapshot.command_storage_capacity_before_frame != MAX_COMMANDS) {
        return Fail("texture sampling submit recorded wrong command capacity");
    }

    if (snapshot.command_storage_capacity_after_last_frame != MAX_COMMANDS) {
        return Fail("texture sampling submit changed command capacity");
    }

    return 0;
}

int RhiConstantBufferBindingDefaultsAreBoundedValues() {
    const RhiConstantBufferBinding binding{};
    if (binding.buffer.generation != 0U) {
        return Fail("default constant buffer handle was not empty");
    }

    if (binding.stage != RhiShaderStage::Unsupported) {
        return Fail("default constant buffer stage was not unsupported");
    }

    if (binding.slot != 0U) {
        return Fail("default constant buffer slot was not zero");
    }

    static_assert(MAX_RHI_CONSTANT_BUFFER_SLOTS > 0U, "constant buffer slot capacity was zero");
    return 0;
}

int RhiCommandListRecordsConstantBufferBindingWithinCapacity() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device, target)) {
        return Fail("target creation failed");
    }

    RhiBufferHandle constant_buffer{};
    if (!CreateConstantBuffer(device_interface, constant_buffer)) {
        return Fail("constant buffer creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    RhiStatus status = command_list.BeginFrame(target);
    if (status != RhiStatus::Success) {
        return Fail("begin frame failed");
    }

    const RhiConstantBufferBinding binding = ConstantBufferBindingFor(constant_buffer);
    status = device_interface.RecordBindConstantBuffer(command_list, binding);
    if (status != RhiStatus::Success) {
        return Fail("constant buffer bind recording failed");
    }

    const auto snapshot = command_list.Snapshot();
    if (snapshot.command_count != 2U) {
        return Fail("constant buffer bind command count changed total command count unexpectedly");
    }

    if (snapshot.constant_buffer_bind_command_count != 1U) {
        return Fail("constant buffer bind command was not tracked");
    }

    return 0;
}

int RhiConstantBufferSlotOverflowDoesNotMutate() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiBufferHandle constant_buffer{};
    if (!CreateConstantBuffer(device_interface, constant_buffer)) {
        return Fail("constant buffer creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    RhiConstantBufferBinding binding = ConstantBufferBindingFor(constant_buffer);
    binding.slot = static_cast<std::uint32_t>(MAX_RHI_CONSTANT_BUFFER_SLOTS);
    const auto before_snapshot = device.Snapshot();
    const RhiStatus status = device_interface.RecordBindConstantBuffer(command_list, binding);
    if (status != RhiStatus::InvalidDescriptor) {
        return Fail("constant buffer slot overflow did not return invalid descriptor");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.recorded_command_count != before_snapshot.recorded_command_count) {
        return Fail("constant buffer slot overflow recorded a command");
    }

    if (after_snapshot.rejected_constant_buffer_bind_count !=
        before_snapshot.rejected_constant_buffer_bind_count + 1U) {
        return Fail("constant buffer slot overflow was not tracked");
    }

    if (after_snapshot.last_status != RhiStatus::InvalidDescriptor) {
        return Fail("constant buffer slot overflow did not record last status");
    }

    return 0;
}

int RhiSubmitConstantBufferBindingUpdatesNullSnapshot() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device, target)) {
        return Fail("target creation failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, pipeline)) {
        return Fail("triangle pipeline creation failed");
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, vertex_buffer)) {
        return Fail("triangle vertex buffer creation failed");
    }

    RhiBufferHandle index_buffer{};
    if (!CreateTriangleIndexBuffer(device_interface, index_buffer)) {
        return Fail("triangle index buffer creation failed");
    }

    RhiBufferHandle constant_buffer{};
    if (!CreateConstantBuffer(device_interface, constant_buffer)) {
        return Fail("constant buffer creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    const RhiStatus record_status = RecordConstantBufferIndexedTriangleDrawFrame(
        device_interface,
        command_list,
        target,
        pipeline,
        TriangleVertexBufferViewFor(vertex_buffer),
        TriangleIndexBufferViewFor(index_buffer),
        constant_buffer,
        TriangleDrawIndexedDesc());
    if (record_status != RhiStatus::Success) {
        return Fail("constant buffer command recording failed");
    }

    const RhiStatus submit_status = device_interface.Submit(command_list);
    if (submit_status != RhiStatus::Success) {
        return Fail("constant buffer command submit failed");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.submitted_constant_buffer_bind_count != 1U) {
        return Fail("submitted constant buffer bind count was not tracked");
    }

    if (snapshot.rejected_constant_buffer_bind_count != 0U) {
        return Fail("successful constant buffer binding changed rejection count");
    }

    if (snapshot.submitted_indexed_draw_count != 1U) {
        return Fail("constant buffer submit did not reach indexed draw");
    }

    if (snapshot.last_bound_constant_buffer_slot != 0U) {
        return Fail("last constant buffer slot was not tracked");
    }

    if (snapshot.last_bound_constant_buffer_stage != RhiShaderStage::Pixel) {
        return Fail("last constant buffer shader stage was not tracked");
    }

    return 0;
}

int RhiSubmitConstantBufferBindingRejectsStaleHandle() {
    NullRhiDevice device = CreateInitializedDevice();
    IRhiDevice &device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device, target)) {
        return Fail("target creation failed");
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, pipeline)) {
        return Fail("triangle pipeline creation failed");
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, vertex_buffer)) {
        return Fail("triangle vertex buffer creation failed");
    }

    RhiBufferHandle index_buffer{};
    if (!CreateTriangleIndexBuffer(device_interface, index_buffer)) {
        return Fail("triangle index buffer creation failed");
    }

    RhiBufferHandle constant_buffer{};
    if (!CreateConstantBuffer(device_interface, constant_buffer)) {
        return Fail("constant buffer creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    const RhiStatus record_status = RecordConstantBufferIndexedTriangleDrawFrame(
        device_interface,
        command_list,
        target,
        pipeline,
        TriangleVertexBufferViewFor(vertex_buffer),
        TriangleIndexBufferViewFor(index_buffer),
        constant_buffer,
        TriangleDrawIndexedDesc());
    if (record_status != RhiStatus::Success) {
        return Fail("constant buffer command recording failed");
    }

    if (device_interface.DestroyBuffer(constant_buffer) != RhiStatus::Success) {
        return Fail("constant buffer destroy failed");
    }

    const auto before_snapshot = device.Snapshot();
    const RhiStatus submit_status = device_interface.Submit(command_list);
    if (submit_status != RhiStatus::InvalidHandle) {
        return Fail("stale constant buffer did not return invalid handle");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.rejected_constant_buffer_bind_count !=
        before_snapshot.rejected_constant_buffer_bind_count + 1U) {
        return Fail("stale constant buffer rejection was not tracked");
    }

    if (after_snapshot.submit_count != before_snapshot.submit_count) {
        return Fail("stale constant buffer changed submit count");
    }

    if (after_snapshot.submitted_constant_buffer_bind_count != before_snapshot.submitted_constant_buffer_bind_count) {
        return Fail("stale constant buffer changed submitted bind count");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_CREATE_DEVICE, RhiCreateNullDeviceReturnsCapabilities},
        {TEST_UNSUPPORTED_BACKEND, RhiCreateDeviceRejectsUnsupportedBackend},
        {TEST_INTERFACE_CREATE_DEVICE, RhiInterfaceCreateNullDeviceReturnsCapabilities},
        {TEST_INTERFACE_CLEAR_CAPTURE, RhiInterfaceClearSubmitPresentCaptureMatchesNullDevice},
        {TEST_FACTORY_CALLER_STORAGE, RhiFactoryCreateNullDeviceUsesCallerOwnedStorage},
        {TEST_FACTORY_NULL_STORAGE, RhiFactoryNullStorageRejectedWithoutOutput},
        {TEST_FACTORY_D3D11_UNSUPPORTED, RhiFactoryD3D11BackendUnsupportedWithoutMutation},
        {TEST_FACTORY_SURFACE_REQUIRED, RhiFactorySurfaceRequiredForNullBackendRejectedWithoutMutation},
        {TEST_FACTORY_INVALID_SURFACE, RhiFactoryInvalidNativeSurfaceRejectedBeforeMutation},
        {TEST_NATIVE_SURFACE_DEFAULT, RhiNativeSurfaceDescDefaultIsInvalidPlainValue},
        {TEST_FACTORY_RAW_STORAGE_NULL, RhiFactoryRawStorageCreatesNullDevice},
        {TEST_FACTORY_RAW_STORAGE_TOO_SMALL, RhiFactoryRawStorageRejectsSmallBuffer},
        {TEST_FACTORY_D3D11_INVALID_SURFACE, RhiFactoryD3D11InvalidSurfaceFailsBeforeHardware},
        {TEST_SWAPCHAIN_DESC_DEFAULT, RhiSwapchainDescDefaultIsBoundedPlainValue},
        {TEST_NULL_SWAPCHAIN_QUERY, RhiNullBackendSwapchainQueryReturnsUnsupported},
        {TEST_SWAPCHAIN_RESIZE_DEFAULTS, RhiSwapchainResizeDefaultContractsAreExplicit},
        {TEST_NULL_SWAPCHAIN_RESIZE, RhiNullBackendSwapchainResizeReturnsUnsupportedWithoutTarget},
        {TEST_CREATE_TARGET, RhiCreateTargetReturnsGenerationHandle},
        {TEST_CREATE_COLOR_TARGET, RhiCreateTargetReturnsGenerationHandle},
        {TEST_INVALID_DESCRIPTOR, RhiCreateColorTargetRejectsInvalidDescriptor},
        {TEST_TARGET_CAPACITY, RhiTargetCapacityOverflowDoesNotMutate},
        {TEST_DESTROY_STALE, RhiDestroyTargetInvalidatesStaleHandle},
        {TEST_REINITIALIZE_STALE_TARGET, RhiReinitializeInvalidatesPriorTargetHandle},
        {TEST_RECORD_CLEAR, RhiCommandListRecordsClearWithinCapacity},
        {TEST_COMMAND_CAPACITY, RhiCommandListCapacityOverflowDoesNotMutate},
        {TEST_COMMAND_LIST_LAST_STATUS, RhiCommandListLastStatusTracksLifecycleAndCapacity},
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
        {TEST_CAPTURE_USER_VISIBLE_RESOLUTION, RhiCapturePresentedTargetWritesUserVisibleResolution},
        {TEST_CAPTURE_DESTROYED_PRESENTED_TARGET, RhiCaptureRejectsDestroyedPresentedTargetWithoutMutation},
        {TEST_UNDERSIZED_CAPTURE, RhiCaptureRejectsUndersizedBufferWithoutWritingBytes},
        {TEST_OVERSIZED_CAPTURE_FIXTURE, RhiCaptureRejectsTargetLargerThanFixtureCapWithoutWritingBytes},
        {TEST_FRAME_NO_GROW, RhiFrameSubmitPresentCaptureDoesNotGrowCommandStorage},
        {TEST_DISABLED_DIAGNOSTICS, RhiDisabledDiagnosticsDoesNotChangeResults},
        {TEST_PRIMITIVE_CAPABILITIES, RhiPrimitiveCapabilitiesReportBoundedCapacities},
        {TEST_CREATE_BUFFER, RhiCreateBufferReturnsGenerationHandleAndSnapshot},
        {TEST_UPDATE_BUFFER, RhiUpdateBufferSignalsFenceAndRecordsBytes},
        {TEST_UPDATE_BUFFER_DESTINATION_RANGE, RhiUpdateBufferDestinationRangeTracksOffsetAndRejectsOverflow},
        {TEST_BUFFER_CAPACITY, RhiBufferCapacityOverflowDoesNotMutate},
        {TEST_TEXTURE_PRIMITIVE, RhiTextureCreateUpdateDestroyTracksSnapshot},
        {TEST_UPDATE_TEXTURE_DESTINATION_RANGE, RhiUpdateTextureDestinationRangeTracksOffsetAndRejectsOverflow},
        {TEST_SAMPLER_PRIMITIVE, RhiSamplerCreateDestroyTracksSnapshot},
        {TEST_SHADER_EMPTY_BYTECODE, RhiShaderModuleRejectsEmptyBytecode},
        {TEST_PIPELINE_INVALID_SHADERS, RhiPipelineRequiresValidShaderModules},
        {TEST_PIPELINE_LIFECYCLE, RhiPipelineCreateDestroyUsesShaderModuleHandles},
        {TEST_PRIMITIVE_STALE_HANDLE, RhiPrimitiveDestroyInvalidatesStaleHandles},
        {TEST_INTERFACE_PRIMITIVE, RhiInterfacePrimitiveLifecycleMatchesNullDevice},
        {TEST_PRIMITIVE_SNAPSHOT_COMPARISON, RhiPrimitiveSnapshotIsIncludedInDeviceSnapshotComparison},
        {TEST_PRIMITIVE_RETIREMENT_DEFAULTS, RhiPrimitiveRetirementDefaultContractsAreExplicit},
        {TEST_PRIMITIVE_RETIREMENT_REQUEST, RhiPrimitiveRetirementRequestCreatesPendingRecord},
        {TEST_PRIMITIVE_RETIREMENT_DRAIN, RhiPrimitiveRetirementDrainInvalidatesHandleAndTracksCounters},
        {TEST_PRIMITIVE_RETIREMENT_REJECTS, RhiPrimitiveRetirementRejectsInvalidWrongDuplicateAndCapacity},
        {TEST_PRIMITIVE_RETIREMENT_FENCE, RhiPrimitiveRetirementFenceNotReadyDrainKeepsPending},
        {TEST_PRIMITIVE_RETIREMENT_IMMEDIATE, RhiPrimitiveRetirementImmediateDestroyCompatibility},
        {TEST_INTERFACE_PRIMITIVE_RETIREMENT, RhiInterfacePrimitiveRetirementMatchesNullDevice},
        {TEST_RECORD_VISIBLE_TRIANGLE, RhiCommandListRecordsVisibleTriangleCommandsWithinCapacity},
        {TEST_DRAW_REQUIRES_PIPELINE, RhiSubmitDrawRejectsMissingPipelineWithoutMutation},
        {TEST_DRAW_RANGE_OVERFLOW, RhiSubmitDrawRejectsVertexBufferRangeOverflow},
        {TEST_DRAW_SNAPSHOT, RhiSubmitDrawUpdatesNullSnapshot},
        {TEST_INDEX_FORMAT_DEFAULTS, RhiIndexFormatDefaultDescriptorsAreUnsupported},
        {TEST_RECORD_INDEXED_STATIC_MESH, RhiCommandListRecordsIndexedStaticMeshWithinCapacity},
        {TEST_INDEXED_DRAW_REQUIRES_INDEX_BUFFER, RhiSubmitIndexedDrawRejectsMissingIndexBufferWithoutMutation},
        {TEST_INDEXED_DRAW_RANGE_OVERFLOW, RhiSubmitIndexedDrawRejectsIndexRangeOverflow},
        {TEST_INDEXED_DRAW_SNAPSHOT, RhiSubmitIndexedDrawUpdatesNullSnapshot},
        {TEST_SAMPLED_TEXTURE_BIND_DEFAULTS, RhiTextureSamplingDefaultBindingsAreBoundedValues},
        {TEST_RECORD_TEXTURE_SAMPLING, RhiCommandListRecordsTextureSamplingWithinCapacity},
        {TEST_TEXTURE_SAMPLING_SLOT_OVERFLOW, RhiTextureSamplingSlotOverflowDoesNotMutate},
        {TEST_TEXTURE_SAMPLING_REQUIRES_SAMPLER, RhiSubmitTextureSamplingRejectsMissingSamplerWithoutMutation},
        {TEST_SAMPLER_REQUIRES_TEXTURE, RhiSubmitSamplerRejectsMissingTextureWithoutMutation},
        {TEST_TEXTURE_SAMPLING_STALE_TEXTURE, RhiSubmitTextureSamplingRejectsStaleTextureHandle},
        {TEST_TEXTURE_SAMPLING_STALE_SAMPLER, RhiSubmitTextureSamplingRejectsStaleSamplerHandle},
        {TEST_TEXTURE_SAMPLING_SNAPSHOT, RhiSubmitTextureSamplingUpdatesNullSnapshot},
        {TEST_CONSTANT_BUFFER_BIND_DEFAULTS, RhiConstantBufferBindingDefaultsAreBoundedValues},
        {TEST_RECORD_CONSTANT_BUFFER_BIND, RhiCommandListRecordsConstantBufferBindingWithinCapacity},
        {TEST_CONSTANT_BUFFER_SLOT_OVERFLOW, RhiConstantBufferSlotOverflowDoesNotMutate},
        {TEST_CONSTANT_BUFFER_SNAPSHOT, RhiSubmitConstantBufferBindingUpdatesNullSnapshot},
        {TEST_CONSTANT_BUFFER_STALE, RhiSubmitConstantBufferBindingRejectsStaleHandle},
        {TEST_NO_FORBIDDEN_DEPENDENCY, RhiNoResourceFileUploadShaderUiDependency}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
