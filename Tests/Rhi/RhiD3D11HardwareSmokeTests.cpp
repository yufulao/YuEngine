// Module: Tests Rhi
// File: Tests/Rhi/RhiD3D11HardwareSmokeTests.cpp

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>
#include <vector>

#include "YuEngine/Platform/PlatformNativeSurface.h"
#include "YuEngine/Platform/PlatformWindowDesc.h"
#include "YuEngine/Platform/PlatformWindowStatus.h"
#include "YuEngine/Platform/WindowsPlatformWindow.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiBufferUsage.h"
#include "YuEngine/Rhi/RhiCaptureResult.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiCommandList.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiDeviceCreateResult.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiDeviceFactory.h"
#include "YuEngine/Rhi/RhiDrawDesc.h"
#include "YuEngine/Rhi/RhiDrawIndexedDesc.h"
#include "YuEngine/Rhi/RhiExtent2D.h"
#include "YuEngine/Rhi/RhiFenceHandle.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiInputLayoutDesc.h"
#include "YuEngine/Rhi/RhiNativeSurfaceDesc.h"
#include "YuEngine/Rhi/RhiPipelineDesc.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"
#include "YuEngine/Rhi/RhiSamplerDesc.h"
#include "YuEngine/Rhi/RhiSamplerHandle.h"
#include "YuEngine/Rhi/RhiShaderModuleDesc.h"
#include "YuEngine/Rhi/RhiShaderModuleHandle.h"
#include "YuEngine/Rhi/RhiShaderStage.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiSwapchainResizeRequest.h"
#include "YuEngine/Rhi/RhiSwapchainResizeResult.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"

using PlatformNativeSurface = yuengine::platform::PlatformNativeSurface;
using PlatformWindowDesc = yuengine::platform::PlatformWindowDesc;
using yuengine::platform::PlatformWindowStatus;
using WindowsPlatformWindow = yuengine::platform::WindowsPlatformWindow;
using yuengine::rhi::IRhiDevice;
using yuengine::rhi::MAX_COMMANDS;
using yuengine::rhi::RGBA8_BYTES_PER_PIXEL;
using yuengine::rhi::RhiBackendKind;
using yuengine::rhi::RhiBufferDesc;
using yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiBufferUsage;
using yuengine::rhi::RhiCaptureResult;
using yuengine::rhi::RhiColor;
using yuengine::rhi::RhiCommandList;
using yuengine::rhi::RhiDeviceCreateResult;
using yuengine::rhi::RhiDeviceDesc;
using yuengine::rhi::RhiDeviceFactory;
using yuengine::rhi::RhiDrawDesc;
using yuengine::rhi::RhiDrawIndexedDesc;
using yuengine::rhi::RhiExtent2D;
using yuengine::rhi::RhiFenceHandle;
using yuengine::rhi::RhiFormat;
using yuengine::rhi::RhiIndexBufferView;
using yuengine::rhi::RhiIndexFormat;
using yuengine::rhi::RhiInputElementFormat;
using yuengine::rhi::RhiInputElementSemantic;
using yuengine::rhi::RhiInputLayoutDesc;
using yuengine::rhi::RhiNativeSurfaceDesc;
using yuengine::rhi::RhiPipelineDesc;
using yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiPrimitiveTopology;
using yuengine::rhi::RhiSampledTextureBinding;
using yuengine::rhi::RhiSamplerBinding;
using yuengine::rhi::RhiSamplerDesc;
using yuengine::rhi::RhiSamplerHandle;
using yuengine::rhi::RhiShaderModuleDesc;
using yuengine::rhi::RhiShaderModuleHandle;
using yuengine::rhi::RhiShaderStage;
using yuengine::rhi::RhiStatus;
using yuengine::rhi::RhiSwapchainResizeRequest;
using yuengine::rhi::RhiSwapchainResizeResult;
using yuengine::rhi::RhiTextureDesc;
using yuengine::rhi::RhiTextureHandle;
using yuengine::rhi::RhiVertexBufferView;

namespace {
constexpr const char *TEST_D3D11_CLEAR_PRESENT_CAPTURE = "RHI_D3D11Hardware_ClearPresentCaptureBytes";
constexpr const char *TEST_D3D11_PRIMITIVE_RESOURCE_PIPELINE = "RHI_D3D11Hardware_PrimitiveResourcePipelineSnapshot";
constexpr const char *TEST_D3D11_VISIBLE_TRIANGLE = "RHI_D3D11Hardware_VisibleTriangleCaptureBytes";
constexpr const char *TEST_D3D11_INDEXED_STATIC_MESH = "RHI_D3D11Hardware_IndexedStaticMeshCaptureBytes";
constexpr const char *TEST_D3D11_TEXTURE_SAMPLING = "RHI_D3D11Hardware_TextureSamplingCaptureBytes";
constexpr const char *TEST_D3D11_SWAPCHAIN_RESIZE_GENERATION =
    "RHI_D3D11Hardware_SwapchainResizeInvalidatesOldBackbufferGeneration";
constexpr const char *TEST_D3D11_SWAPCHAIN_RESIZE_SAME_EXTENT =
    "RHI_D3D11Hardware_SwapchainResizeSameExtentNoOpKeepsGeneration";
constexpr const char *TEST_D3D11_SWAPCHAIN_RESIZE_REJECT =
    "RHI_D3D11Hardware_SwapchainResizeRejectsInvalidExtent";
constexpr std::uint32_t SMOKE_EXTENT = 4U;
constexpr std::uint32_t RESIZED_WIDTH = 2U;
constexpr std::uint32_t RESIZED_HEIGHT = 3U;
constexpr int SKIP_RETURN_CODE = 77;
constexpr std::uint32_t TRIANGLE_VERTEX_COUNT = 3U;
constexpr std::uint32_t TRIANGLE_INDEX_COUNT = 3U;
constexpr std::size_t TRIANGLE_VERTEX_STRIDE_BYTES = sizeof(float) * 6U;
constexpr std::size_t TEXTURED_VERTEX_STRIDE_BYTES = sizeof(float) * 4U;
constexpr std::size_t TEXTURED_VERTEX_BUFFER_BYTES = TEXTURED_VERTEX_STRIDE_BYTES * TRIANGLE_VERTEX_COUNT;
constexpr std::size_t TRIANGLE_INDEX_BUFFER_BYTES = sizeof(std::uint16_t) * TRIANGLE_INDEX_COUNT;
struct TriangleVertex final {
    float position[2];
    float color[4];
};
struct TexturedVertex final {
    float position[2];
    float texcoord[2];
};
constexpr std::uint8_t VERTEX_SHADER_BYTES[] = {
    0x44U, 0x58U, 0x42U, 0x43U, 0x07U, 0x36U, 0x40U, 0xB0U,
    0xF6U, 0x7FU, 0xF2U, 0x2AU, 0xA8U, 0xDEU, 0xDBU, 0x0BU,
    0x0CU, 0x69U, 0x38U, 0xF5U, 0x01U, 0x00U, 0x00U, 0x00U,
    0xDCU, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x2CU, 0x00U, 0x00U, 0x00U, 0x60U, 0x00U, 0x00U, 0x00U,
    0x94U, 0x00U, 0x00U, 0x00U, 0x49U, 0x53U, 0x47U, 0x4EU,
    0x2CU, 0x00U, 0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x08U, 0x00U, 0x00U, 0x00U, 0x20U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x06U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x53U, 0x56U, 0x5FU, 0x56U,
    0x65U, 0x72U, 0x74U, 0x65U, 0x78U, 0x49U, 0x44U, 0x00U,
    0x4FU, 0x53U, 0x47U, 0x4EU, 0x2CU, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x08U, 0x00U, 0x00U, 0x00U,
    0x20U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x0FU, 0x00U, 0x00U, 0x00U,
    0x53U, 0x56U, 0x5FU, 0x50U, 0x6FU, 0x73U, 0x69U, 0x74U,
    0x69U, 0x6FU, 0x6EU, 0x00U, 0x53U, 0x48U, 0x45U, 0x58U,
    0x40U, 0x00U, 0x00U, 0x00U, 0x50U, 0x00U, 0x01U, 0x00U,
    0x10U, 0x00U, 0x00U, 0x00U, 0x6AU, 0x08U, 0x00U, 0x01U,
    0x67U, 0x00U, 0x00U, 0x04U, 0xF2U, 0x20U, 0x10U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x36U, 0x00U, 0x00U, 0x08U, 0xF2U, 0x20U, 0x10U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x02U, 0x40U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x80U, 0x3FU,
    0x3EU, 0x00U, 0x00U, 0x01U};
constexpr std::uint8_t PIXEL_SHADER_BYTES[] = {
    0x44U, 0x58U, 0x42U, 0x43U, 0xEEU, 0x4DU, 0x0BU, 0x94U,
    0x29U, 0xEEU, 0x02U, 0x01U, 0x63U, 0xA4U, 0x5DU, 0xF3U,
    0x4FU, 0x56U, 0xE1U, 0xB1U, 0x01U, 0x00U, 0x00U, 0x00U,
    0xB4U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x2CU, 0x00U, 0x00U, 0x00U, 0x3CU, 0x00U, 0x00U, 0x00U,
    0x70U, 0x00U, 0x00U, 0x00U, 0x49U, 0x53U, 0x47U, 0x4EU,
    0x08U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x08U, 0x00U, 0x00U, 0x00U, 0x4FU, 0x53U, 0x47U, 0x4EU,
    0x2CU, 0x00U, 0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x08U, 0x00U, 0x00U, 0x00U, 0x20U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x03U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x0FU, 0x00U, 0x00U, 0x00U, 0x53U, 0x56U, 0x5FU, 0x54U,
    0x61U, 0x72U, 0x67U, 0x65U, 0x74U, 0x00U, 0xABU, 0xABU,
    0x53U, 0x48U, 0x45U, 0x58U, 0x3CU, 0x00U, 0x00U, 0x00U,
    0x50U, 0x00U, 0x00U, 0x00U, 0x0FU, 0x00U, 0x00U, 0x00U,
    0x6AU, 0x08U, 0x00U, 0x01U, 0x65U, 0x00U, 0x00U, 0x03U,
    0xF2U, 0x20U, 0x10U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x36U, 0x00U, 0x00U, 0x08U, 0xF2U, 0x20U, 0x10U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x02U, 0x40U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x80U, 0x3FU, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x80U, 0x3FU,
    0x3EU, 0x00U, 0x00U, 0x01U};
constexpr std::uint8_t TRIANGLE_VERTEX_SHADER_BYTES[] = {
    0x44U, 0x58U, 0x42U, 0x43U, 0x61U, 0x54U, 0x15U, 0x34U,
    0x22U, 0xF6U, 0x73U, 0x92U, 0x9FU, 0x1FU, 0xCEU, 0xBBU,
    0xC2U, 0x9AU, 0x71U, 0x91U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x74U, 0x02U, 0x00U, 0x00U, 0x05U, 0x00U, 0x00U, 0x00U,
    0x34U, 0x00U, 0x00U, 0x00U, 0xA0U, 0x00U, 0x00U, 0x00U,
    0xF0U, 0x00U, 0x00U, 0x00U, 0x44U, 0x01U, 0x00U, 0x00U,
    0xD8U, 0x01U, 0x00U, 0x00U, 0x52U, 0x44U, 0x45U, 0x46U,
    0x64U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x3CU, 0x00U, 0x00U, 0x00U, 0x00U, 0x05U, 0xFEU, 0xFFU,
    0x00U, 0x01U, 0x00U, 0x00U, 0x3CU, 0x00U, 0x00U, 0x00U,
    0x52U, 0x44U, 0x31U, 0x31U, 0x3CU, 0x00U, 0x00U, 0x00U,
    0x18U, 0x00U, 0x00U, 0x00U, 0x20U, 0x00U, 0x00U, 0x00U,
    0x28U, 0x00U, 0x00U, 0x00U, 0x24U, 0x00U, 0x00U, 0x00U,
    0x0CU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x4DU, 0x69U, 0x63U, 0x72U, 0x6FU, 0x73U, 0x6FU, 0x66U,
    0x74U, 0x20U, 0x28U, 0x52U, 0x29U, 0x20U, 0x48U, 0x4CU,
    0x53U, 0x4CU, 0x20U, 0x53U, 0x68U, 0x61U, 0x64U, 0x65U,
    0x72U, 0x20U, 0x43U, 0x6FU, 0x6DU, 0x70U, 0x69U, 0x6CU,
    0x65U, 0x72U, 0x20U, 0x31U, 0x30U, 0x2EU, 0x31U, 0x00U,
    0x49U, 0x53U, 0x47U, 0x4EU, 0x48U, 0x00U, 0x00U, 0x00U,
    0x02U, 0x00U, 0x00U, 0x00U, 0x08U, 0x00U, 0x00U, 0x00U,
    0x38U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x03U, 0x03U, 0x00U, 0x00U,
    0x41U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x0FU, 0x0FU, 0x00U, 0x00U,
    0x50U, 0x4FU, 0x53U, 0x49U, 0x54U, 0x49U, 0x4FU, 0x4EU,
    0x00U, 0x43U, 0x4FU, 0x4CU, 0x4FU, 0x52U, 0x00U, 0xABU,
    0x4FU, 0x53U, 0x47U, 0x4EU, 0x4CU, 0x00U, 0x00U, 0x00U,
    0x02U, 0x00U, 0x00U, 0x00U, 0x08U, 0x00U, 0x00U, 0x00U,
    0x38U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x0FU, 0x00U, 0x00U, 0x00U,
    0x44U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x0FU, 0x00U, 0x00U, 0x00U,
    0x53U, 0x56U, 0x5FU, 0x50U, 0x6FU, 0x73U, 0x69U, 0x74U,
    0x69U, 0x6FU, 0x6EU, 0x00U, 0x43U, 0x4FU, 0x4CU, 0x4FU,
    0x52U, 0x00U, 0xABU, 0xABU, 0x53U, 0x48U, 0x45U, 0x58U,
    0x8CU, 0x00U, 0x00U, 0x00U, 0x50U, 0x00U, 0x01U, 0x00U,
    0x23U, 0x00U, 0x00U, 0x00U, 0x6AU, 0x08U, 0x00U, 0x01U,
    0x5FU, 0x00U, 0x00U, 0x03U, 0x32U, 0x10U, 0x10U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x5FU, 0x00U, 0x00U, 0x03U,
    0xF2U, 0x10U, 0x10U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x67U, 0x00U, 0x00U, 0x04U, 0xF2U, 0x20U, 0x10U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x65U, 0x00U, 0x00U, 0x03U, 0xF2U, 0x20U, 0x10U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x36U, 0x00U, 0x00U, 0x05U,
    0x32U, 0x20U, 0x10U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x46U, 0x10U, 0x10U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x36U, 0x00U, 0x00U, 0x08U, 0xC2U, 0x20U, 0x10U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x02U, 0x40U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x80U, 0x3FU,
    0x36U, 0x00U, 0x00U, 0x05U, 0xF2U, 0x20U, 0x10U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x46U, 0x1EU, 0x10U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x3EU, 0x00U, 0x00U, 0x01U,
    0x53U, 0x54U, 0x41U, 0x54U, 0x94U, 0x00U, 0x00U, 0x00U,
    0x04U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x04U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U
};
constexpr std::uint8_t TRIANGLE_PIXEL_SHADER_BYTES[] = {
    0x44U, 0x58U, 0x42U, 0x43U, 0x1CU, 0xA4U, 0xEEU, 0x2AU,
    0x1EU, 0xD3U, 0xBBU, 0x57U, 0xC2U, 0x0AU, 0x8EU, 0x49U,
    0xAEU, 0x76U, 0xE9U, 0x5DU, 0x01U, 0x00U, 0x00U, 0x00U,
    0x08U, 0x02U, 0x00U, 0x00U, 0x05U, 0x00U, 0x00U, 0x00U,
    0x34U, 0x00U, 0x00U, 0x00U, 0xA0U, 0x00U, 0x00U, 0x00U,
    0xF4U, 0x00U, 0x00U, 0x00U, 0x28U, 0x01U, 0x00U, 0x00U,
    0x6CU, 0x01U, 0x00U, 0x00U, 0x52U, 0x44U, 0x45U, 0x46U,
    0x64U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x3CU, 0x00U, 0x00U, 0x00U, 0x00U, 0x05U, 0xFFU, 0xFFU,
    0x00U, 0x01U, 0x00U, 0x00U, 0x3CU, 0x00U, 0x00U, 0x00U,
    0x52U, 0x44U, 0x31U, 0x31U, 0x3CU, 0x00U, 0x00U, 0x00U,
    0x18U, 0x00U, 0x00U, 0x00U, 0x20U, 0x00U, 0x00U, 0x00U,
    0x28U, 0x00U, 0x00U, 0x00U, 0x24U, 0x00U, 0x00U, 0x00U,
    0x0CU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x4DU, 0x69U, 0x63U, 0x72U, 0x6FU, 0x73U, 0x6FU, 0x66U,
    0x74U, 0x20U, 0x28U, 0x52U, 0x29U, 0x20U, 0x48U, 0x4CU,
    0x53U, 0x4CU, 0x20U, 0x53U, 0x68U, 0x61U, 0x64U, 0x65U,
    0x72U, 0x20U, 0x43U, 0x6FU, 0x6DU, 0x70U, 0x69U, 0x6CU,
    0x65U, 0x72U, 0x20U, 0x31U, 0x30U, 0x2EU, 0x31U, 0x00U,
    0x49U, 0x53U, 0x47U, 0x4EU, 0x4CU, 0x00U, 0x00U, 0x00U,
    0x02U, 0x00U, 0x00U, 0x00U, 0x08U, 0x00U, 0x00U, 0x00U,
    0x38U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x0FU, 0x00U, 0x00U, 0x00U,
    0x44U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x0FU, 0x0FU, 0x00U, 0x00U,
    0x53U, 0x56U, 0x5FU, 0x50U, 0x6FU, 0x73U, 0x69U, 0x74U,
    0x69U, 0x6FU, 0x6EU, 0x00U, 0x43U, 0x4FU, 0x4CU, 0x4FU,
    0x52U, 0x00U, 0xABU, 0xABU, 0x4FU, 0x53U, 0x47U, 0x4EU,
    0x2CU, 0x00U, 0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x08U, 0x00U, 0x00U, 0x00U, 0x20U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x03U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x0FU, 0x00U, 0x00U, 0x00U, 0x53U, 0x56U, 0x5FU, 0x54U,
    0x61U, 0x72U, 0x67U, 0x65U, 0x74U, 0x00U, 0xABU, 0xABU,
    0x53U, 0x48U, 0x45U, 0x58U, 0x3CU, 0x00U, 0x00U, 0x00U,
    0x50U, 0x00U, 0x00U, 0x00U, 0x0FU, 0x00U, 0x00U, 0x00U,
    0x6AU, 0x08U, 0x00U, 0x01U, 0x62U, 0x10U, 0x00U, 0x03U,
    0xF2U, 0x10U, 0x10U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x65U, 0x00U, 0x00U, 0x03U, 0xF2U, 0x20U, 0x10U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x36U, 0x00U, 0x00U, 0x05U,
    0xF2U, 0x20U, 0x10U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x46U, 0x1EU, 0x10U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x3EU, 0x00U, 0x00U, 0x01U, 0x53U, 0x54U, 0x41U, 0x54U,
    0x94U, 0x00U, 0x00U, 0x00U, 0x02U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x02U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U
};
constexpr std::uint8_t TEXTURE_SAMPLING_VERTEX_SHADER_BYTES[] = {
    0x44U, 0x58U, 0x42U, 0x43U, 0x5BU, 0x3CU, 0xEDU, 0x58U,
    0x72U, 0x41U, 0x88U, 0xD3U, 0xD4U, 0x50U, 0x41U, 0x0AU,
    0x60U, 0xEDU, 0x66U, 0x53U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x38U, 0x02U, 0x00U, 0x00U, 0x05U, 0x00U, 0x00U, 0x00U,
    0x34U, 0x00U, 0x00U, 0x00U, 0x80U, 0x00U, 0x00U, 0x00U,
    0xD4U, 0x00U, 0x00U, 0x00U, 0x2CU, 0x01U, 0x00U, 0x00U,
    0xBCU, 0x01U, 0x00U, 0x00U, 0x52U, 0x44U, 0x45U, 0x46U,
    0x44U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x1CU, 0x00U, 0x00U, 0x00U, 0x00U, 0x04U, 0xFEU, 0xFFU,
    0x00U, 0x01U, 0x00U, 0x00U, 0x1CU, 0x00U, 0x00U, 0x00U,
    0x4DU, 0x69U, 0x63U, 0x72U, 0x6FU, 0x73U, 0x6FU, 0x66U,
    0x74U, 0x20U, 0x28U, 0x52U, 0x29U, 0x20U, 0x48U, 0x4CU,
    0x53U, 0x4CU, 0x20U, 0x53U, 0x68U, 0x61U, 0x64U, 0x65U,
    0x72U, 0x20U, 0x43U, 0x6FU, 0x6DU, 0x70U, 0x69U, 0x6CU,
    0x65U, 0x72U, 0x20U, 0x31U, 0x30U, 0x2EU, 0x31U, 0x00U,
    0x49U, 0x53U, 0x47U, 0x4EU, 0x4CU, 0x00U, 0x00U, 0x00U,
    0x02U, 0x00U, 0x00U, 0x00U, 0x08U, 0x00U, 0x00U, 0x00U,
    0x38U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x03U, 0x03U, 0x00U, 0x00U,
    0x41U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x03U, 0x03U, 0x00U, 0x00U,
    0x50U, 0x4FU, 0x53U, 0x49U, 0x54U, 0x49U, 0x4FU, 0x4EU,
    0x00U, 0x54U, 0x45U, 0x58U, 0x43U, 0x4FU, 0x4FU, 0x52U,
    0x44U, 0x00U, 0xABU, 0xABU, 0x4FU, 0x53U, 0x47U, 0x4EU,
    0x50U, 0x00U, 0x00U, 0x00U, 0x02U, 0x00U, 0x00U, 0x00U,
    0x08U, 0x00U, 0x00U, 0x00U, 0x38U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x03U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x0FU, 0x00U, 0x00U, 0x00U, 0x44U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x03U, 0x00U, 0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x03U, 0x0CU, 0x00U, 0x00U, 0x53U, 0x56U, 0x5FU, 0x50U,
    0x6FU, 0x73U, 0x69U, 0x74U, 0x69U, 0x6FU, 0x6EU, 0x00U,
    0x54U, 0x45U, 0x58U, 0x43U, 0x4FU, 0x4FU, 0x52U, 0x44U,
    0x00U, 0xABU, 0xABU, 0xABU, 0x53U, 0x48U, 0x44U, 0x52U,
    0x88U, 0x00U, 0x00U, 0x00U, 0x40U, 0x00U, 0x01U, 0x00U,
    0x22U, 0x00U, 0x00U, 0x00U, 0x5FU, 0x00U, 0x00U, 0x03U,
    0x32U, 0x10U, 0x10U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x5FU, 0x00U, 0x00U, 0x03U, 0x32U, 0x10U, 0x10U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x67U, 0x00U, 0x00U, 0x04U,
    0xF2U, 0x20U, 0x10U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x65U, 0x00U, 0x00U, 0x03U,
    0x32U, 0x20U, 0x10U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x36U, 0x00U, 0x00U, 0x05U, 0x32U, 0x20U, 0x10U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x46U, 0x10U, 0x10U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x36U, 0x00U, 0x00U, 0x08U,
    0xC2U, 0x20U, 0x10U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x02U, 0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x80U, 0x3FU, 0x36U, 0x00U, 0x00U, 0x05U,
    0x32U, 0x20U, 0x10U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x46U, 0x10U, 0x10U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x3EU, 0x00U, 0x00U, 0x01U, 0x53U, 0x54U, 0x41U, 0x54U,
    0x74U, 0x00U, 0x00U, 0x00U, 0x04U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x04U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x03U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U
};
constexpr std::uint8_t TEXTURE_SAMPLING_PIXEL_SHADER_BYTES[] = {
    0x44U, 0x58U, 0x42U, 0x43U, 0x06U, 0xDFU, 0x33U, 0xA5U,
    0x1FU, 0x77U, 0x8DU, 0x63U, 0xE5U, 0x67U, 0x75U, 0xA9U,
    0x9FU, 0x77U, 0x18U, 0xBCU, 0x01U, 0x00U, 0x00U, 0x00U,
    0x54U, 0x02U, 0x00U, 0x00U, 0x05U, 0x00U, 0x00U, 0x00U,
    0x34U, 0x00U, 0x00U, 0x00U, 0xE0U, 0x00U, 0x00U, 0x00U,
    0x38U, 0x01U, 0x00U, 0x00U, 0x6CU, 0x01U, 0x00U, 0x00U,
    0xD8U, 0x01U, 0x00U, 0x00U, 0x52U, 0x44U, 0x45U, 0x46U,
    0xA4U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x02U, 0x00U, 0x00U, 0x00U,
    0x1CU, 0x00U, 0x00U, 0x00U, 0x00U, 0x04U, 0xFFU, 0xFFU,
    0x00U, 0x01U, 0x00U, 0x00U, 0x7CU, 0x00U, 0x00U, 0x00U,
    0x5CU, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x6CU, 0x00U, 0x00U, 0x00U, 0x02U, 0x00U, 0x00U, 0x00U,
    0x05U, 0x00U, 0x00U, 0x00U, 0x04U, 0x00U, 0x00U, 0x00U,
    0xFFU, 0xFFU, 0xFFU, 0xFFU, 0x00U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x0DU, 0x00U, 0x00U, 0x00U,
    0x73U, 0x61U, 0x6DU, 0x70U, 0x6CU, 0x65U, 0x64U, 0x5FU,
    0x73U, 0x61U, 0x6DU, 0x70U, 0x6CU, 0x65U, 0x72U, 0x00U,
    0x73U, 0x61U, 0x6DU, 0x70U, 0x6CU, 0x65U, 0x64U, 0x5FU,
    0x74U, 0x65U, 0x78U, 0x74U, 0x75U, 0x72U, 0x65U, 0x00U,
    0x4DU, 0x69U, 0x63U, 0x72U, 0x6FU, 0x73U, 0x6FU, 0x66U,
    0x74U, 0x20U, 0x28U, 0x52U, 0x29U, 0x20U, 0x48U, 0x4CU,
    0x53U, 0x4CU, 0x20U, 0x53U, 0x68U, 0x61U, 0x64U, 0x65U,
    0x72U, 0x20U, 0x43U, 0x6FU, 0x6DU, 0x70U, 0x69U, 0x6CU,
    0x65U, 0x72U, 0x20U, 0x31U, 0x30U, 0x2EU, 0x31U, 0x00U,
    0x49U, 0x53U, 0x47U, 0x4EU, 0x50U, 0x00U, 0x00U, 0x00U,
    0x02U, 0x00U, 0x00U, 0x00U, 0x08U, 0x00U, 0x00U, 0x00U,
    0x38U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x0FU, 0x00U, 0x00U, 0x00U,
    0x44U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x03U, 0x03U, 0x00U, 0x00U,
    0x53U, 0x56U, 0x5FU, 0x50U, 0x6FU, 0x73U, 0x69U, 0x74U,
    0x69U, 0x6FU, 0x6EU, 0x00U, 0x54U, 0x45U, 0x58U, 0x43U,
    0x4FU, 0x4FU, 0x52U, 0x44U, 0x00U, 0xABU, 0xABU, 0xABU,
    0x4FU, 0x53U, 0x47U, 0x4EU, 0x2CU, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x08U, 0x00U, 0x00U, 0x00U,
    0x20U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x03U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x0FU, 0x00U, 0x00U, 0x00U,
    0x53U, 0x56U, 0x5FU, 0x54U, 0x61U, 0x72U, 0x67U, 0x65U,
    0x74U, 0x00U, 0xABU, 0xABU, 0x53U, 0x48U, 0x44U, 0x52U,
    0x64U, 0x00U, 0x00U, 0x00U, 0x40U, 0x00U, 0x00U, 0x00U,
    0x19U, 0x00U, 0x00U, 0x00U, 0x5AU, 0x00U, 0x00U, 0x03U,
    0x00U, 0x60U, 0x10U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x58U, 0x18U, 0x00U, 0x04U, 0x00U, 0x70U, 0x10U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x55U, 0x55U, 0x00U, 0x00U,
    0x62U, 0x10U, 0x00U, 0x03U, 0x32U, 0x10U, 0x10U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x65U, 0x00U, 0x00U, 0x03U,
    0xF2U, 0x20U, 0x10U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x45U, 0x00U, 0x00U, 0x09U, 0xF2U, 0x20U, 0x10U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x46U, 0x10U, 0x10U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x46U, 0x7EU, 0x10U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x60U, 0x10U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x3EU, 0x00U, 0x00U, 0x01U,
    0x53U, 0x54U, 0x41U, 0x54U, 0x74U, 0x00U, 0x00U, 0x00U,
    0x02U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x02U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x01U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x01U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U
};

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

int Skip(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stdout);
    std::fputc('\n', stdout);
    return SKIP_RETURN_CODE;
}

RhiNativeSurfaceDesc ConvertSurface(const PlatformNativeSurface &surface) {
    return RhiNativeSurfaceDesc{surface.window_value, surface.instance_value, surface.valid};
}

RhiDeviceCreateResult CreateD3D11DeviceForWindow(
    WindowsPlatformWindow &window,
    std::vector<std::byte> &storage,
    const RhiExtent2D &extent) {
    const std::size_t storage_size = RhiDeviceFactory::RequiredDeviceStorageSize(RhiBackendKind::D3D11);
    if (storage_size == 0U) {
        return RhiDeviceCreateResult{RhiStatus::UnsupportedBackend, nullptr, {}};
    }

    storage.resize(storage_size);
    RhiDeviceDesc device_desc{};
    device_desc.backend_kind = RhiBackendKind::D3D11;
    device_desc.native_surface = ConvertSurface(window.GetNativeSurface());
    device_desc.requires_native_surface = true;
    device_desc.requires_swapchain = true;
    device_desc.swapchain.extent = extent;
    device_desc.command_list_capacity = MAX_COMMANDS;

    const std::span<std::byte> storage_span(storage.data(), storage.size());
    return RhiDeviceFactory::CreateDevice(device_desc, storage_span);
}

bool BytesMatchColor(const std::vector<std::uint8_t> &bytes, RhiColor color) {
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

bool IsRedTrianglePixel(const std::vector<std::uint8_t> &bytes, std::size_t index) {
    if (bytes[index] < 200U) {
        return false;
    }

    if (bytes[index + 1U] > 40U) {
        return false;
    }

    if (bytes[index + 2U] > 40U) {
        return false;
    }

    return bytes[index + 3U] == 255U;
}

bool IsBlackBackgroundPixel(const std::vector<std::uint8_t> &bytes, std::size_t index) {
    if (bytes[index] > 40U) {
        return false;
    }

    if (bytes[index + 1U] > 40U) {
        return false;
    }

    if (bytes[index + 2U] > 40U) {
        return false;
    }

    return bytes[index + 3U] == 255U;
}

bool CaptureContainsVisibleTriangle(const std::vector<std::uint8_t> &bytes) {
    std::size_t red_count = 0U;
    std::size_t black_count = 0U;
    for (std::size_t index = 0U; index < bytes.size(); index += RGBA8_BYTES_PER_PIXEL) {
        if (IsRedTrianglePixel(bytes, index)) {
            ++red_count;
            continue;
        }

        if (IsBlackBackgroundPixel(bytes, index)) {
            ++black_count;
        }
    }

    if (red_count == 0U) {
        return false;
    }

    return black_count > 0U;
}

bool IsBlueTexturePixel(const std::vector<std::uint8_t> &bytes, std::size_t index) {
    if (bytes[index] > 40U) {
        return false;
    }

    if (bytes[index + 1U] > 40U) {
        return false;
    }

    if (bytes[index + 2U] < 200U) {
        return false;
    }

    return bytes[index + 3U] == 255U;
}

bool CaptureContainsTextureSampling(const std::vector<std::uint8_t> &bytes) {
    std::size_t blue_count = 0U;
    std::size_t black_count = 0U;
    for (std::size_t index = 0U; index < bytes.size(); index += RGBA8_BYTES_PER_PIXEL) {
        if (IsBlueTexturePixel(bytes, index)) {
            ++blue_count;
            continue;
        }

        if (IsBlackBackgroundPixel(bytes, index)) {
            ++black_count;
        }
    }

    if (blue_count == 0U) {
        return false;
    }

    return black_count > 0U;
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

RhiInputLayoutDesc TexturedTriangleInputLayoutDesc() {
    RhiInputLayoutDesc desc{};
    desc.elements[0U].semantic = RhiInputElementSemantic::Position;
    desc.elements[0U].format = RhiInputElementFormat::Float32x2;
    desc.elements[0U].offset_bytes = 0U;
    desc.elements[1U].semantic = RhiInputElementSemantic::TexCoord;
    desc.elements[1U].format = RhiInputElementFormat::Float32x2;
    desc.elements[1U].offset_bytes = sizeof(float) * 2U;
    desc.element_count = 2U;
    desc.stride_bytes = TEXTURED_VERTEX_STRIDE_BYTES;
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

RhiVertexBufferView TriangleVertexBufferViewFor(RhiBufferHandle buffer) {
    RhiVertexBufferView view{};
    view.buffer = buffer;
    view.offset_bytes = 0U;
    view.stride_bytes = TRIANGLE_VERTEX_STRIDE_BYTES;
    view.size_bytes = sizeof(TriangleVertex) * TRIANGLE_VERTEX_COUNT;
    return view;
}

RhiVertexBufferView TexturedVertexBufferViewFor(RhiBufferHandle buffer) {
    RhiVertexBufferView view{};
    view.buffer = buffer;
    view.offset_bytes = 0U;
    view.stride_bytes = TEXTURED_VERTEX_STRIDE_BYTES;
    view.size_bytes = TEXTURED_VERTEX_BUFFER_BYTES;
    return view;
}

RhiIndexBufferView TriangleIndexBufferViewFor(RhiBufferHandle buffer) {
    RhiIndexBufferView view{};
    view.buffer = buffer;
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

RhiStatus ClearPresentCapture(IRhiDevice &device, RhiTextureHandle target, RhiColor color, std::vector<std::uint8_t> &capture) {
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

    status = device.Present();
    if (status != RhiStatus::Success) {
        return status;
    }

    const RhiCaptureResult result = device.CapturePresentedTarget(std::span<std::uint8_t>(capture.data(), capture.size()));
    return result.status;
}

int RunD3D11ClearPresentCapture() {
    WindowsPlatformWindow window;
    PlatformWindowDesc window_desc{};
    window_desc.title = "YuEngine D3D11 Hardware Smoke";
    window_desc.client_width = SMOKE_EXTENT;
    window_desc.client_height = SMOKE_EXTENT;
    window_desc.visible = false;

    const PlatformWindowStatus window_status = window.Create(window_desc);
    if (window_status != PlatformWindowStatus::Success) {
        return Skip("d3d11 hardware smoke skipped because a native window could not be created");
    }

    const std::size_t storage_size = RhiDeviceFactory::RequiredDeviceStorageSize(RhiBackendKind::D3D11);
    if (storage_size == 0U) {
        return Skip("d3d11 hardware smoke skipped because the backend is not compiled");
    }

    std::vector<std::byte> storage(storage_size);
    RhiDeviceDesc device_desc{};
    device_desc.backend_kind = RhiBackendKind::D3D11;
    device_desc.native_surface = ConvertSurface(window.GetNativeSurface());
    device_desc.requires_native_surface = true;
    device_desc.requires_swapchain = true;
    device_desc.swapchain.extent = {SMOKE_EXTENT, SMOKE_EXTENT};
    device_desc.command_list_capacity = MAX_COMMANDS;

    const RhiDeviceCreateResult create_result = RhiDeviceFactory::CreateDevice(
        device_desc,
        std::span<std::byte>(storage.data(), storage.size()));
    if (create_result.status == RhiStatus::MissingHardware) {
        return Skip("d3d11 hardware smoke skipped because a hardware D3D11 device is unavailable");
    }

    if (create_result.status != RhiStatus::Success) {
        return Fail("d3d11 device creation failed");
    }

    if (create_result.device == nullptr) {
        return Fail("d3d11 device creation returned null device");
    }

    RhiTextureHandle target{};
    RhiStatus status = create_result.device->GetSwapchainColorTarget(target);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 swapchain target query failed");
    }

    const RhiColor clear_color{255U, 0U, 0U, 255U};
    std::vector<std::uint8_t> capture(SMOKE_EXTENT * SMOKE_EXTENT * RGBA8_BYTES_PER_PIXEL);
    status = ClearPresentCapture(*create_result.device, target, clear_color, capture);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 clear present capture path failed");
    }

    if (!BytesMatchColor(capture, clear_color)) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 capture bytes did not match clear color");
    }

    const RhiStatus destroy_status = RhiDeviceFactory::DestroyDevice(create_result.device);
    if (destroy_status != RhiStatus::Success) {
        return Fail("d3d11 device destroy failed");
    }

    return 0;
}

int RunD3D11SwapchainResizeInvalidatesOldBackbufferGeneration() {
    WindowsPlatformWindow window;
    PlatformWindowDesc window_desc{};
    window_desc.title = "YuEngine D3D11 Swapchain Resize Smoke";
    window_desc.client_width = SMOKE_EXTENT;
    window_desc.client_height = SMOKE_EXTENT;
    window_desc.visible = false;

    const PlatformWindowStatus window_status = window.Create(window_desc);
    if (window_status != PlatformWindowStatus::Success) {
        return Skip("d3d11 swapchain resize smoke skipped because a native window could not be created");
    }

    const RhiExtent2D initial_extent{SMOKE_EXTENT, SMOKE_EXTENT};
    std::vector<std::byte> storage{};
    const RhiDeviceCreateResult create_result = CreateD3D11DeviceForWindow(window, storage, initial_extent);
    if (create_result.status == RhiStatus::UnsupportedBackend) {
        return Skip("d3d11 swapchain resize smoke skipped because the backend is not compiled");
    }

    if (create_result.status == RhiStatus::MissingHardware) {
        return Skip("d3d11 swapchain resize smoke skipped because a hardware D3D11 device is unavailable");
    }

    if (create_result.status != RhiStatus::Success) {
        return Fail("d3d11 swapchain resize device creation failed");
    }

    if (create_result.device == nullptr) {
        return Fail("d3d11 swapchain resize device creation returned null device");
    }

    if (!create_result.capabilities.supports_swapchain_resize) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 capabilities did not report swapchain resize");
    }

    IRhiDevice &device = *create_result.device;
    RhiTextureHandle old_target{};
    RhiStatus status = device.GetSwapchainColorTarget(old_target);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 swapchain resize old target query failed");
    }

    const auto before_snapshot = device.Snapshot();
    RhiSwapchainResizeRequest request{};
    request.extent = {RESIZED_WIDTH, RESIZED_HEIGHT};
    RhiSwapchainResizeResult result{};
    status = device.ResizeSwapchain(request, result);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 swapchain resize failed");
    }

    if (!result.resized) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 swapchain resize did not report resized");
    }

    if (result.previous_extent.width != SMOKE_EXTENT || result.previous_extent.height != SMOKE_EXTENT) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 swapchain resize previous extent was wrong");
    }

    if (result.previous_color_target.slot != old_target.slot) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 swapchain resize previous target slot was wrong");
    }

    if (result.previous_color_target.generation != old_target.generation) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 swapchain resize previous target generation was wrong");
    }

    if (result.snapshot.resize_count != before_snapshot.swapchain.resize_count + 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 swapchain resize count was not tracked");
    }

    if (result.snapshot.extent.width != RESIZED_WIDTH || result.snapshot.extent.height != RESIZED_HEIGHT) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 swapchain resize snapshot extent was wrong");
    }

    RhiTextureHandle new_target{};
    status = device.GetSwapchainColorTarget(new_target);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 swapchain resize new target query failed");
    }

    if (new_target.generation == old_target.generation) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 swapchain resize did not invalidate old generation");
    }

    std::vector<std::uint8_t> capture(RESIZED_WIDTH * RESIZED_HEIGHT * RGBA8_BYTES_PER_PIXEL);
    const RhiColor stale_clear_color{1U, 2U, 3U, 255U};
    status = ClearPresentCapture(device, old_target, stale_clear_color, capture);
    if (status != RhiStatus::InvalidHandle) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 swapchain resize accepted stale target");
    }

    const RhiColor clear_color{8U, 64U, 192U, 255U};
    status = ClearPresentCapture(device, new_target, clear_color, capture);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 resized swapchain clear present capture failed");
    }

    if (!BytesMatchColor(capture, clear_color)) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 resized swapchain capture bytes did not match clear color");
    }

    const RhiStatus destroy_status = RhiDeviceFactory::DestroyDevice(create_result.device);
    if (destroy_status != RhiStatus::Success) {
        return Fail("d3d11 swapchain resize device destroy failed");
    }

    return 0;
}

int RunD3D11SwapchainResizeSameExtentNoOpKeepsGeneration() {
    WindowsPlatformWindow window;
    PlatformWindowDesc window_desc{};
    window_desc.title = "YuEngine D3D11 Swapchain Same Extent Resize Smoke";
    window_desc.client_width = SMOKE_EXTENT;
    window_desc.client_height = SMOKE_EXTENT;
    window_desc.visible = false;

    const PlatformWindowStatus window_status = window.Create(window_desc);
    if (window_status != PlatformWindowStatus::Success) {
        return Skip("d3d11 same extent resize smoke skipped because a native window could not be created");
    }

    const RhiExtent2D initial_extent{SMOKE_EXTENT, SMOKE_EXTENT};
    std::vector<std::byte> storage{};
    const RhiDeviceCreateResult create_result = CreateD3D11DeviceForWindow(window, storage, initial_extent);
    if (create_result.status == RhiStatus::UnsupportedBackend) {
        return Skip("d3d11 same extent resize smoke skipped because the backend is not compiled");
    }

    if (create_result.status == RhiStatus::MissingHardware) {
        return Skip("d3d11 same extent resize smoke skipped because a hardware D3D11 device is unavailable");
    }

    if (create_result.status != RhiStatus::Success) {
        return Fail("d3d11 same extent resize device creation failed");
    }

    if (create_result.device == nullptr) {
        return Fail("d3d11 same extent resize device creation returned null device");
    }

    IRhiDevice &device = *create_result.device;
    RhiTextureHandle old_target{};
    RhiStatus status = device.GetSwapchainColorTarget(old_target);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 same extent resize target query failed");
    }

    const auto before_snapshot = device.Snapshot();
    RhiSwapchainResizeRequest request{};
    request.extent = initial_extent;
    RhiSwapchainResizeResult result{};
    status = device.ResizeSwapchain(request, result);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 same extent resize failed");
    }

    if (result.resized) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 same extent resize reported resized");
    }

    if (result.snapshot.resize_count != before_snapshot.swapchain.resize_count) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 same extent resize changed resize count");
    }

    RhiTextureHandle new_target{};
    status = device.GetSwapchainColorTarget(new_target);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 same extent resize target requery failed");
    }

    if (new_target.generation != old_target.generation) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 same extent resize changed target generation");
    }

    const RhiStatus destroy_status = RhiDeviceFactory::DestroyDevice(create_result.device);
    if (destroy_status != RhiStatus::Success) {
        return Fail("d3d11 same extent resize device destroy failed");
    }

    return 0;
}

int RunD3D11SwapchainResizeRejectsInvalidExtent() {
    WindowsPlatformWindow window;
    PlatformWindowDesc window_desc{};
    window_desc.title = "YuEngine D3D11 Swapchain Resize Reject Smoke";
    window_desc.client_width = SMOKE_EXTENT;
    window_desc.client_height = SMOKE_EXTENT;
    window_desc.visible = false;

    const PlatformWindowStatus window_status = window.Create(window_desc);
    if (window_status != PlatformWindowStatus::Success) {
        return Skip("d3d11 resize reject smoke skipped because a native window could not be created");
    }

    const RhiExtent2D initial_extent{SMOKE_EXTENT, SMOKE_EXTENT};
    std::vector<std::byte> storage{};
    const RhiDeviceCreateResult create_result = CreateD3D11DeviceForWindow(window, storage, initial_extent);
    if (create_result.status == RhiStatus::UnsupportedBackend) {
        return Skip("d3d11 resize reject smoke skipped because the backend is not compiled");
    }

    if (create_result.status == RhiStatus::MissingHardware) {
        return Skip("d3d11 resize reject smoke skipped because a hardware D3D11 device is unavailable");
    }

    if (create_result.status != RhiStatus::Success) {
        return Fail("d3d11 resize reject device creation failed");
    }

    if (create_result.device == nullptr) {
        return Fail("d3d11 resize reject device creation returned null device");
    }

    IRhiDevice &device = *create_result.device;
    RhiTextureHandle target_before{};
    RhiStatus status = device.GetSwapchainColorTarget(target_before);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 resize reject target query failed");
    }

    const auto before_snapshot = device.Snapshot();
    RhiSwapchainResizeRequest request{};
    request.extent = {0U, SMOKE_EXTENT};
    RhiSwapchainResizeResult result{};
    status = device.ResizeSwapchain(request, result);
    if (status != RhiStatus::InvalidDescriptor) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 resize reject did not return invalid descriptor");
    }

    if (result.status != RhiStatus::InvalidDescriptor) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 resize reject result status was wrong");
    }

    if (result.resized) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 resize reject reported resized");
    }

    if (result.snapshot.rejected_resize_count != before_snapshot.swapchain.rejected_resize_count + 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 resize reject count was not tracked");
    }

    RhiTextureHandle target_after{};
    status = device.GetSwapchainColorTarget(target_after);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 resize reject target requery failed");
    }

    if (target_after.generation != target_before.generation) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 resize reject changed target generation");
    }

    const auto after_snapshot = device.Snapshot();
    if (after_snapshot.failed_operation_count != before_snapshot.failed_operation_count + 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 resize reject failure was not tracked");
    }

    const RhiStatus destroy_status = RhiDeviceFactory::DestroyDevice(create_result.device);
    if (destroy_status != RhiStatus::Success) {
        return Fail("d3d11 resize reject device destroy failed");
    }

    return 0;
}

int RunD3D11PrimitiveResourcePipeline() {
    WindowsPlatformWindow window;
    PlatformWindowDesc window_desc{};
    window_desc.title = "YuEngine D3D11 Primitive Hardware Smoke";
    window_desc.client_width = SMOKE_EXTENT;
    window_desc.client_height = SMOKE_EXTENT;
    window_desc.visible = false;

    const PlatformWindowStatus window_status = window.Create(window_desc);
    if (window_status != PlatformWindowStatus::Success) {
        return Skip("d3d11 primitive smoke skipped because a native window could not be created");
    }

    const std::size_t storage_size = RhiDeviceFactory::RequiredDeviceStorageSize(RhiBackendKind::D3D11);
    if (storage_size == 0U) {
        return Skip("d3d11 primitive smoke skipped because the backend is not compiled");
    }

    std::vector<std::byte> storage(storage_size);
    RhiDeviceDesc device_desc{};
    device_desc.backend_kind = RhiBackendKind::D3D11;
    device_desc.native_surface = ConvertSurface(window.GetNativeSurface());
    device_desc.requires_native_surface = true;
    device_desc.requires_swapchain = true;
    device_desc.swapchain.extent = {SMOKE_EXTENT, SMOKE_EXTENT};
    device_desc.command_list_capacity = MAX_COMMANDS;

    const RhiDeviceCreateResult create_result = RhiDeviceFactory::CreateDevice(
        device_desc,
        std::span<std::byte>(storage.data(), storage.size()));
    if (create_result.status == RhiStatus::MissingHardware) {
        return Skip("d3d11 primitive smoke skipped because a hardware D3D11 device is unavailable");
    }

    if (create_result.status != RhiStatus::Success) {
        return Fail("d3d11 primitive device creation failed");
    }

    if (create_result.device == nullptr) {
        return Fail("d3d11 primitive device creation returned null device");
    }

    IRhiDevice &device = *create_result.device;
    const std::uint8_t buffer_bytes[] = {
        1U, 2U, 3U, 4U,
        5U, 6U, 7U, 8U,
        9U, 10U, 11U, 12U,
        13U, 14U, 15U, 16U};
    const std::span<const std::uint8_t> buffer_span(buffer_bytes, sizeof(buffer_bytes));
    RhiBufferDesc buffer_desc{};
    buffer_desc.usage = RhiBufferUsage::Vertex;
    buffer_desc.size_bytes = sizeof(buffer_bytes);
    RhiBufferHandle buffer{};
    RhiStatus status = device.CreateBuffer(buffer_desc, buffer_span, buffer);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive buffer creation failed");
    }

    const std::uint8_t buffer_update_bytes[] = {16U, 15U, 14U, 13U};
    const std::span<const std::uint8_t> buffer_update_span(buffer_update_bytes, sizeof(buffer_update_bytes));
    RhiFenceHandle buffer_fence{};
    status = device.UpdateBuffer(buffer, buffer_update_span, buffer_fence);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive buffer update failed");
    }

    if (buffer_fence.generation == 0U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive buffer update did not signal fence");
    }

    const std::uint8_t texture_bytes[] = {
        255U, 0U, 0U, 255U,
        0U, 255U, 0U, 255U,
        0U, 0U, 255U, 255U,
        255U, 255U, 255U, 255U};
    const std::span<const std::uint8_t> texture_span(texture_bytes, sizeof(texture_bytes));
    RhiTextureDesc texture_desc{};
    texture_desc.format = RhiFormat::Rgba8Unorm;
    texture_desc.extent = {2U, 2U};
    RhiTextureHandle texture{};
    status = device.CreateTexture(texture_desc, texture_span, texture);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive texture creation failed");
    }

    const std::uint8_t texture_update_bytes[] = {
        0U, 0U, 0U, 255U,
        16U, 16U, 16U, 255U,
        32U, 32U, 32U, 255U,
        48U, 48U, 48U, 255U};
    const std::span<const std::uint8_t> texture_update_span(texture_update_bytes, sizeof(texture_update_bytes));
    RhiFenceHandle texture_fence{};
    status = device.UpdateTexture(texture, texture_update_span, texture_fence);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive texture update failed");
    }

    if (texture_fence.generation == 0U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive texture update did not signal fence");
    }

    RhiSamplerDesc sampler_desc{};
    sampler_desc.linear_filter = true;
    RhiSamplerHandle sampler{};
    status = device.CreateSampler(sampler_desc, sampler);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive sampler creation failed");
    }

    const std::span<const std::uint8_t> vertex_bytecode(VERTEX_SHADER_BYTES, sizeof(VERTEX_SHADER_BYTES));
    const RhiShaderModuleDesc vertex_desc{RhiShaderStage::Vertex, vertex_bytecode};
    RhiShaderModuleHandle vertex_shader{};
    status = device.CreateShaderModule(vertex_desc, vertex_shader);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive vertex shader creation failed");
    }

    const std::span<const std::uint8_t> pixel_bytecode(PIXEL_SHADER_BYTES, sizeof(PIXEL_SHADER_BYTES));
    const RhiShaderModuleDesc pixel_desc{RhiShaderStage::Pixel, pixel_bytecode};
    RhiShaderModuleHandle pixel_shader{};
    status = device.CreateShaderModule(pixel_desc, pixel_shader);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive pixel shader creation failed");
    }

    RhiPipelineDesc pipeline_desc{};
    pipeline_desc.vertex_shader = vertex_shader;
    pipeline_desc.pixel_shader = pixel_shader;
    RhiPipelineHandle pipeline{};
    status = device.CreatePipeline(pipeline_desc, pipeline);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive pipeline creation failed");
    }

    const auto created_snapshot = device.Snapshot();
    if (created_snapshot.resources.buffer_count != 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive buffer count was not tracked");
    }

    if (created_snapshot.resources.texture_count != 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive texture count was not tracked");
    }

    if (created_snapshot.resources.sampler_count != 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive sampler count was not tracked");
    }

    if (created_snapshot.resources.shader_module_count != 2U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive shader module count was not tracked");
    }

    if (created_snapshot.resources.pipeline_count != 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive pipeline count was not tracked");
    }

    if (created_snapshot.resources.created_primitive_count != 6U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive created count was not tracked");
    }

    if (created_snapshot.resources.updated_primitive_count != 2U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive update count was not tracked");
    }

    if (created_snapshot.resources.signaled_fence_count != 2U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive fence count was not tracked");
    }

    if (created_snapshot.resources.last_update_bytes != sizeof(texture_update_bytes)) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive last update bytes were not tracked");
    }

    if (device.DestroyPipeline(pipeline) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive pipeline destroy failed");
    }

    if (device.DestroyShaderModule(pixel_shader) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive pixel shader destroy failed");
    }

    if (device.DestroyShaderModule(vertex_shader) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive vertex shader destroy failed");
    }

    if (device.DestroySampler(sampler) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive sampler destroy failed");
    }

    if (device.DestroyTexture(texture) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive texture destroy failed");
    }

    if (device.DestroyBuffer(buffer) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive buffer destroy failed");
    }

    const auto destroyed_snapshot = device.Snapshot();
    if (destroyed_snapshot.resources.destroyed_primitive_count != 6U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive destroyed count was not tracked");
    }

    if (destroyed_snapshot.resources.buffer_count != 0U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive buffer count did not return to zero");
    }

    if (destroyed_snapshot.resources.pipeline_count != 0U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 primitive pipeline count did not return to zero");
    }

    const RhiStatus destroy_status = RhiDeviceFactory::DestroyDevice(create_result.device);
    if (destroy_status != RhiStatus::Success) {
        return Fail("d3d11 primitive device destroy failed");
    }

    return 0;
}

int RunD3D11VisibleTriangleCapture() {
    WindowsPlatformWindow window;
    PlatformWindowDesc window_desc{};
    window_desc.title = "YuEngine D3D11 Visible Triangle Smoke";
    window_desc.client_width = SMOKE_EXTENT;
    window_desc.client_height = SMOKE_EXTENT;
    window_desc.visible = false;

    const PlatformWindowStatus window_status = window.Create(window_desc);
    if (window_status != PlatformWindowStatus::Success) {
        return Skip("d3d11 visible triangle smoke skipped because a native window could not be created");
    }

    const std::size_t storage_size = RhiDeviceFactory::RequiredDeviceStorageSize(RhiBackendKind::D3D11);
    if (storage_size == 0U) {
        return Skip("d3d11 visible triangle smoke skipped because the backend is not compiled");
    }

    std::vector<std::byte> storage(storage_size);
    RhiDeviceDesc device_desc{};
    device_desc.backend_kind = RhiBackendKind::D3D11;
    device_desc.native_surface = ConvertSurface(window.GetNativeSurface());
    device_desc.requires_native_surface = true;
    device_desc.requires_swapchain = true;
    device_desc.swapchain.extent = {SMOKE_EXTENT, SMOKE_EXTENT};
    device_desc.command_list_capacity = MAX_COMMANDS;

    const RhiDeviceCreateResult create_result = RhiDeviceFactory::CreateDevice(
        device_desc,
        std::span<std::byte>(storage.data(), storage.size()));
    if (create_result.status == RhiStatus::MissingHardware) {
        return Skip("d3d11 visible triangle smoke skipped because a hardware D3D11 device is unavailable");
    }

    if (create_result.status != RhiStatus::Success) {
        return Fail("d3d11 visible triangle device creation failed");
    }

    if (create_result.device == nullptr) {
        return Fail("d3d11 visible triangle device creation returned null device");
    }

    IRhiDevice &device = *create_result.device;
    RhiTextureHandle target{};
    RhiStatus status = device.GetSwapchainColorTarget(target);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle swapchain target query failed");
    }

    std::array<TriangleVertex, 3U> vertices{};
    vertices[0U].position[0U] = -1.0F;
    vertices[0U].position[1U] = -1.0F;
    vertices[0U].color[0U] = 1.0F;
    vertices[0U].color[3U] = 1.0F;
    vertices[1U].position[0U] = -1.0F;
    vertices[1U].position[1U] = 1.0F;
    vertices[1U].color[0U] = 1.0F;
    vertices[1U].color[3U] = 1.0F;
    vertices[2U].position[0U] = 1.0F;
    vertices[2U].position[1U] = -1.0F;
    vertices[2U].color[0U] = 1.0F;
    vertices[2U].color[3U] = 1.0F;

    const auto vertex_byte_pointer = reinterpret_cast<const std::uint8_t *>(vertices.data());
    const std::size_t vertex_byte_count = sizeof(TriangleVertex) * vertices.size();
    const std::span<const std::uint8_t> vertex_span(vertex_byte_pointer, vertex_byte_count);
    RhiBufferDesc buffer_desc{};
    buffer_desc.usage = RhiBufferUsage::Vertex;
    buffer_desc.size_bytes = vertex_byte_count;
    RhiBufferHandle vertex_buffer{};
    status = device.CreateBuffer(buffer_desc, vertex_span, vertex_buffer);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle vertex buffer creation failed");
    }

    const std::span<const std::uint8_t> vertex_bytecode(
        TRIANGLE_VERTEX_SHADER_BYTES,
        sizeof(TRIANGLE_VERTEX_SHADER_BYTES));
    const RhiShaderModuleDesc vertex_desc{RhiShaderStage::Vertex, vertex_bytecode};
    RhiShaderModuleHandle vertex_shader{};
    status = device.CreateShaderModule(vertex_desc, vertex_shader);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle vertex shader creation failed");
    }

    const std::span<const std::uint8_t> pixel_bytecode(
        TRIANGLE_PIXEL_SHADER_BYTES,
        sizeof(TRIANGLE_PIXEL_SHADER_BYTES));
    const RhiShaderModuleDesc pixel_desc{RhiShaderStage::Pixel, pixel_bytecode};
    RhiShaderModuleHandle pixel_shader{};
    status = device.CreateShaderModule(pixel_desc, pixel_shader);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle pixel shader creation failed");
    }

    RhiPipelineDesc pipeline_desc{};
    pipeline_desc.vertex_shader = vertex_shader;
    pipeline_desc.pixel_shader = pixel_shader;
    pipeline_desc.input_layout = TriangleInputLayoutDesc();
    RhiPipelineHandle pipeline{};
    status = device.CreatePipeline(pipeline_desc, pipeline);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle pipeline creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    status = command_list.BeginFrame(target);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle begin frame failed");
    }

    status = device.RecordClear(command_list, target, RhiColor{0U, 0U, 0U, 255U});
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle clear recording failed");
    }

    status = device.RecordBindPipeline(command_list, pipeline);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle pipeline bind recording failed");
    }

    status = device.RecordBindVertexBuffer(command_list, TriangleVertexBufferViewFor(vertex_buffer));
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle vertex buffer bind recording failed");
    }

    status = device.RecordDraw(command_list, TriangleDrawDesc());
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle draw recording failed");
    }

    status = command_list.EndFrame();
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle end frame failed");
    }

    status = device.Submit(command_list);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle submit failed");
    }

    status = device.Present();
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle present failed");
    }

    std::vector<std::uint8_t> capture(SMOKE_EXTENT * SMOKE_EXTENT * RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult capture_result = device.CapturePresentedTarget(
        std::span<std::uint8_t>(capture.data(), capture.size()));
    if (capture_result.status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle capture failed");
    }

    if (capture_result.bytes_written != capture.size()) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle capture byte count was wrong");
    }

    if (!CaptureContainsVisibleTriangle(capture)) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle capture did not contain triangle and background pixels");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.submitted_draw_count != 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle submitted draw count was not tracked");
    }

    if (snapshot.last_draw_vertex_count != TRIANGLE_VERTEX_COUNT) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle last draw vertex count was not tracked");
    }

    if (device.DestroyPipeline(pipeline) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle pipeline destroy failed");
    }

    if (device.DestroyShaderModule(pixel_shader) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle pixel shader destroy failed");
    }

    if (device.DestroyShaderModule(vertex_shader) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle vertex shader destroy failed");
    }

    if (device.DestroyBuffer(vertex_buffer) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 visible triangle vertex buffer destroy failed");
    }

    const RhiStatus destroy_status = RhiDeviceFactory::DestroyDevice(create_result.device);
    if (destroy_status != RhiStatus::Success) {
        return Fail("d3d11 visible triangle device destroy failed");
    }

    return 0;
}

int RunD3D11IndexedStaticMeshCapture() {
    WindowsPlatformWindow window;
    PlatformWindowDesc window_desc{};
    window_desc.title = "YuEngine D3D11 Indexed Static Mesh Smoke";
    window_desc.client_width = SMOKE_EXTENT;
    window_desc.client_height = SMOKE_EXTENT;
    window_desc.visible = false;

    const PlatformWindowStatus window_status = window.Create(window_desc);
    if (window_status != PlatformWindowStatus::Success) {
        return Skip("d3d11 indexed static mesh smoke skipped because a native window could not be created");
    }

    const std::size_t storage_size = RhiDeviceFactory::RequiredDeviceStorageSize(RhiBackendKind::D3D11);
    if (storage_size == 0U) {
        return Skip("d3d11 indexed static mesh smoke skipped because the backend is not compiled");
    }

    std::vector<std::byte> storage(storage_size);
    RhiDeviceDesc device_desc{};
    device_desc.backend_kind = RhiBackendKind::D3D11;
    device_desc.native_surface = ConvertSurface(window.GetNativeSurface());
    device_desc.requires_native_surface = true;
    device_desc.requires_swapchain = true;
    device_desc.swapchain.extent = {SMOKE_EXTENT, SMOKE_EXTENT};
    device_desc.command_list_capacity = MAX_COMMANDS;

    const RhiDeviceCreateResult create_result = RhiDeviceFactory::CreateDevice(
        device_desc,
        std::span<std::byte>(storage.data(), storage.size()));
    if (create_result.status == RhiStatus::MissingHardware) {
        return Skip("d3d11 indexed static mesh smoke skipped because a hardware D3D11 device is unavailable");
    }

    if (create_result.status != RhiStatus::Success) {
        return Fail("d3d11 indexed static mesh device creation failed");
    }

    if (create_result.device == nullptr) {
        return Fail("d3d11 indexed static mesh device creation returned null device");
    }

    IRhiDevice &device = *create_result.device;
    RhiTextureHandle target{};
    RhiStatus status = device.GetSwapchainColorTarget(target);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh swapchain target query failed");
    }

    std::array<TriangleVertex, 3U> vertices{};
    vertices[0U].position[0U] = -1.0F;
    vertices[0U].position[1U] = -1.0F;
    vertices[0U].color[0U] = 1.0F;
    vertices[0U].color[3U] = 1.0F;
    vertices[1U].position[0U] = -1.0F;
    vertices[1U].position[1U] = 1.0F;
    vertices[1U].color[0U] = 1.0F;
    vertices[1U].color[3U] = 1.0F;
    vertices[2U].position[0U] = 1.0F;
    vertices[2U].position[1U] = -1.0F;
    vertices[2U].color[0U] = 1.0F;
    vertices[2U].color[3U] = 1.0F;

    const auto *vertex_byte_pointer = reinterpret_cast<const std::uint8_t *>(vertices.data());
    const std::size_t vertex_byte_count = sizeof(TriangleVertex) * vertices.size();
    const std::span<const std::uint8_t> vertex_span(vertex_byte_pointer, vertex_byte_count);
    RhiBufferDesc vertex_buffer_desc{};
    vertex_buffer_desc.usage = RhiBufferUsage::Vertex;
    vertex_buffer_desc.size_bytes = vertex_byte_count;
    RhiBufferHandle vertex_buffer{};
    status = device.CreateBuffer(vertex_buffer_desc, vertex_span, vertex_buffer);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh vertex buffer creation failed");
    }

    const std::array<std::uint16_t, TRIANGLE_INDEX_COUNT> indices{0U, 1U, 2U};
    const auto *index_byte_pointer = reinterpret_cast<const std::uint8_t *>(indices.data());
    const std::span<const std::uint8_t> index_span(index_byte_pointer, TRIANGLE_INDEX_BUFFER_BYTES);
    RhiBufferDesc index_buffer_desc{};
    index_buffer_desc.usage = RhiBufferUsage::Index;
    index_buffer_desc.size_bytes = TRIANGLE_INDEX_BUFFER_BYTES;
    RhiBufferHandle index_buffer{};
    status = device.CreateBuffer(index_buffer_desc, index_span, index_buffer);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh index buffer creation failed");
    }

    const std::span<const std::uint8_t> vertex_bytecode(
        TRIANGLE_VERTEX_SHADER_BYTES,
        sizeof(TRIANGLE_VERTEX_SHADER_BYTES));
    const RhiShaderModuleDesc vertex_desc{RhiShaderStage::Vertex, vertex_bytecode};
    RhiShaderModuleHandle vertex_shader{};
    status = device.CreateShaderModule(vertex_desc, vertex_shader);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh vertex shader creation failed");
    }

    const std::span<const std::uint8_t> pixel_bytecode(
        TRIANGLE_PIXEL_SHADER_BYTES,
        sizeof(TRIANGLE_PIXEL_SHADER_BYTES));
    const RhiShaderModuleDesc pixel_desc{RhiShaderStage::Pixel, pixel_bytecode};
    RhiShaderModuleHandle pixel_shader{};
    status = device.CreateShaderModule(pixel_desc, pixel_shader);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh pixel shader creation failed");
    }

    RhiPipelineDesc pipeline_desc{};
    pipeline_desc.vertex_shader = vertex_shader;
    pipeline_desc.pixel_shader = pixel_shader;
    pipeline_desc.input_layout = TriangleInputLayoutDesc();
    RhiPipelineHandle pipeline{};
    status = device.CreatePipeline(pipeline_desc, pipeline);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh pipeline creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    status = command_list.BeginFrame(target);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh begin frame failed");
    }

    status = device.RecordClear(command_list, target, RhiColor{0U, 0U, 0U, 255U});
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh clear recording failed");
    }

    status = device.RecordBindPipeline(command_list, pipeline);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh pipeline bind recording failed");
    }

    status = device.RecordBindVertexBuffer(command_list, TriangleVertexBufferViewFor(vertex_buffer));
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh vertex buffer bind recording failed");
    }

    status = device.RecordBindIndexBuffer(command_list, TriangleIndexBufferViewFor(index_buffer));
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh index buffer bind recording failed");
    }

    status = device.RecordDrawIndexed(command_list, TriangleDrawIndexedDesc());
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh draw recording failed");
    }

    status = command_list.EndFrame();
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh end frame failed");
    }

    status = device.Submit(command_list);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh submit failed");
    }

    status = device.Present();
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh present failed");
    }

    std::vector<std::uint8_t> capture(SMOKE_EXTENT * SMOKE_EXTENT * RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult capture_result = device.CapturePresentedTarget(
        std::span<std::uint8_t>(capture.data(), capture.size()));
    if (capture_result.status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh capture failed");
    }

    if (capture_result.bytes_written != capture.size()) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh capture byte count was wrong");
    }

    if (!CaptureContainsVisibleTriangle(capture)) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh capture did not contain triangle and background pixels");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.submitted_draw_count != 0U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh changed non-indexed draw count");
    }

    if (snapshot.submitted_indexed_draw_count != 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh submitted indexed draw count was not tracked");
    }

    if (snapshot.last_indexed_draw_index_count != TRIANGLE_INDEX_COUNT) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh last indexed draw index count was not tracked");
    }

    if (snapshot.last_bound_index_buffer_size_bytes != TRIANGLE_INDEX_BUFFER_BYTES) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh last index buffer size was not tracked");
    }

    if (device.DestroyPipeline(pipeline) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh pipeline destroy failed");
    }

    if (device.DestroyShaderModule(pixel_shader) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh pixel shader destroy failed");
    }

    if (device.DestroyShaderModule(vertex_shader) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh vertex shader destroy failed");
    }

    if (device.DestroyBuffer(index_buffer) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh index buffer destroy failed");
    }

    if (device.DestroyBuffer(vertex_buffer) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 indexed static mesh vertex buffer destroy failed");
    }

    const RhiStatus destroy_status = RhiDeviceFactory::DestroyDevice(create_result.device);
    if (destroy_status != RhiStatus::Success) {
        return Fail("d3d11 indexed static mesh device destroy failed");
    }

    return 0;
}

int RunD3D11TextureSamplingCapture() {
    WindowsPlatformWindow window;
    PlatformWindowDesc window_desc{};
    window_desc.title = "YuEngine D3D11 Texture Sampling Smoke";
    window_desc.client_width = SMOKE_EXTENT;
    window_desc.client_height = SMOKE_EXTENT;
    window_desc.visible = false;

    const PlatformWindowStatus window_status = window.Create(window_desc);
    if (window_status != PlatformWindowStatus::Success) {
        return Skip("d3d11 texture sampling smoke skipped because a native window could not be created");
    }

    const std::size_t storage_size = RhiDeviceFactory::RequiredDeviceStorageSize(RhiBackendKind::D3D11);
    if (storage_size == 0U) {
        return Skip("d3d11 texture sampling smoke skipped because the backend is not compiled");
    }

    std::vector<std::byte> storage(storage_size);
    RhiDeviceDesc device_desc{};
    device_desc.backend_kind = RhiBackendKind::D3D11;
    device_desc.native_surface = ConvertSurface(window.GetNativeSurface());
    device_desc.requires_native_surface = true;
    device_desc.requires_swapchain = true;
    device_desc.swapchain.extent = {SMOKE_EXTENT, SMOKE_EXTENT};
    device_desc.command_list_capacity = MAX_COMMANDS;

    const RhiDeviceCreateResult create_result = RhiDeviceFactory::CreateDevice(
        device_desc,
        std::span<std::byte>(storage.data(), storage.size()));
    if (create_result.status == RhiStatus::MissingHardware) {
        return Skip("d3d11 texture sampling smoke skipped because a hardware D3D11 device is unavailable");
    }

    if (create_result.status != RhiStatus::Success) {
        return Fail("d3d11 texture sampling device creation failed");
    }

    if (create_result.device == nullptr) {
        return Fail("d3d11 texture sampling device creation returned null device");
    }

    IRhiDevice &device = *create_result.device;
    RhiTextureHandle target{};
    RhiStatus status = device.GetSwapchainColorTarget(target);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling swapchain target query failed");
    }

    std::array<TexturedVertex, 3U> vertices{};
    vertices[0U].position[0U] = -1.0F;
    vertices[0U].position[1U] = -1.0F;
    vertices[0U].texcoord[0U] = 0.0F;
    vertices[0U].texcoord[1U] = 1.0F;
    vertices[1U].position[0U] = -1.0F;
    vertices[1U].position[1U] = 1.0F;
    vertices[1U].texcoord[0U] = 0.0F;
    vertices[1U].texcoord[1U] = 0.0F;
    vertices[2U].position[0U] = 1.0F;
    vertices[2U].position[1U] = -1.0F;
    vertices[2U].texcoord[0U] = 1.0F;
    vertices[2U].texcoord[1U] = 1.0F;

    const auto *vertex_byte_pointer = reinterpret_cast<const std::uint8_t *>(vertices.data());
    const std::size_t vertex_byte_count = sizeof(TexturedVertex) * vertices.size();
    const std::span<const std::uint8_t> vertex_span(vertex_byte_pointer, vertex_byte_count);
    RhiBufferDesc vertex_buffer_desc{};
    vertex_buffer_desc.usage = RhiBufferUsage::Vertex;
    vertex_buffer_desc.size_bytes = vertex_byte_count;
    RhiBufferHandle vertex_buffer{};
    status = device.CreateBuffer(vertex_buffer_desc, vertex_span, vertex_buffer);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling vertex buffer creation failed");
    }

    const std::array<std::uint16_t, TRIANGLE_INDEX_COUNT> indices{0U, 1U, 2U};
    const auto *index_byte_pointer = reinterpret_cast<const std::uint8_t *>(indices.data());
    const std::span<const std::uint8_t> index_span(index_byte_pointer, TRIANGLE_INDEX_BUFFER_BYTES);
    RhiBufferDesc index_buffer_desc{};
    index_buffer_desc.usage = RhiBufferUsage::Index;
    index_buffer_desc.size_bytes = TRIANGLE_INDEX_BUFFER_BYTES;
    RhiBufferHandle index_buffer{};
    status = device.CreateBuffer(index_buffer_desc, index_span, index_buffer);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling index buffer creation failed");
    }

    const std::uint8_t texture_bytes[] = {
        0U, 0U, 255U, 255U,
        0U, 0U, 255U, 255U,
        0U, 0U, 255U, 255U,
        0U, 0U, 255U, 255U};
    const std::span<const std::uint8_t> texture_span(texture_bytes, sizeof(texture_bytes));
    RhiTextureDesc texture_desc{};
    texture_desc.format = RhiFormat::Rgba8Unorm;
    texture_desc.extent = {2U, 2U};
    RhiTextureHandle texture{};
    status = device.CreateTexture(texture_desc, texture_span, texture);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling texture creation failed");
    }

    RhiSamplerDesc sampler_desc{};
    sampler_desc.linear_filter = false;
    sampler_desc.clamp_to_edge = true;
    RhiSamplerHandle sampler{};
    status = device.CreateSampler(sampler_desc, sampler);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling sampler creation failed");
    }

    const std::span<const std::uint8_t> vertex_bytecode(
        TEXTURE_SAMPLING_VERTEX_SHADER_BYTES,
        sizeof(TEXTURE_SAMPLING_VERTEX_SHADER_BYTES));
    const RhiShaderModuleDesc vertex_desc{RhiShaderStage::Vertex, vertex_bytecode};
    RhiShaderModuleHandle vertex_shader{};
    status = device.CreateShaderModule(vertex_desc, vertex_shader);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling vertex shader creation failed");
    }

    const std::span<const std::uint8_t> pixel_bytecode(
        TEXTURE_SAMPLING_PIXEL_SHADER_BYTES,
        sizeof(TEXTURE_SAMPLING_PIXEL_SHADER_BYTES));
    const RhiShaderModuleDesc pixel_desc{RhiShaderStage::Pixel, pixel_bytecode};
    RhiShaderModuleHandle pixel_shader{};
    status = device.CreateShaderModule(pixel_desc, pixel_shader);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling pixel shader creation failed");
    }

    RhiPipelineDesc pipeline_desc{};
    pipeline_desc.vertex_shader = vertex_shader;
    pipeline_desc.pixel_shader = pixel_shader;
    pipeline_desc.input_layout = TexturedTriangleInputLayoutDesc();
    RhiPipelineHandle pipeline{};
    status = device.CreatePipeline(pipeline_desc, pipeline);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling pipeline creation failed");
    }

    RhiCommandList command_list(MAX_COMMANDS);
    status = command_list.BeginFrame(target);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling begin frame failed");
    }

    status = device.RecordClear(command_list, target, RhiColor{0U, 0U, 0U, 255U});
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling clear recording failed");
    }

    status = device.RecordBindPipeline(command_list, pipeline);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling pipeline bind recording failed");
    }

    const RhiVertexBufferView vertex_view = TexturedVertexBufferViewFor(vertex_buffer);
    status = device.RecordBindVertexBuffer(command_list, vertex_view);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling vertex buffer bind recording failed");
    }

    const RhiIndexBufferView index_view = TriangleIndexBufferViewFor(index_buffer);
    status = device.RecordBindIndexBuffer(command_list, index_view);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling index buffer bind recording failed");
    }

    const RhiSampledTextureBinding texture_binding = SampledTextureBindingFor(texture);
    status = device.RecordBindSampledTexture(command_list, texture_binding);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling sampled texture bind recording failed");
    }

    const RhiSamplerBinding sampler_binding = SamplerBindingFor(sampler);
    status = device.RecordBindSampler(command_list, sampler_binding);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling sampler bind recording failed");
    }

    status = device.RecordDrawIndexed(command_list, TriangleDrawIndexedDesc());
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling draw recording failed");
    }

    status = command_list.EndFrame();
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling end frame failed");
    }

    status = device.Submit(command_list);
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling submit failed");
    }

    status = device.Present();
    if (status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling present failed");
    }

    std::vector<std::uint8_t> capture(SMOKE_EXTENT * SMOKE_EXTENT * RGBA8_BYTES_PER_PIXEL);
    const RhiCaptureResult capture_result = device.CapturePresentedTarget(
        std::span<std::uint8_t>(capture.data(), capture.size()));
    if (capture_result.status != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling capture failed");
    }

    if (capture_result.bytes_written != capture.size()) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling capture byte count was wrong");
    }

    if (!CaptureContainsTextureSampling(capture)) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling capture did not contain sampled texture and background pixels");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.submitted_indexed_draw_count != 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling submitted indexed draw count was not tracked");
    }

    if (snapshot.submitted_sampled_texture_bind_count != 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling sampled texture bind count was not tracked");
    }

    if (snapshot.submitted_sampler_bind_count != 1U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling sampler bind count was not tracked");
    }

    if (snapshot.last_bound_sampled_texture_slot != 0U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling sampled texture slot was not tracked");
    }

    if (snapshot.last_bound_sampler_slot != 0U) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling sampler slot was not tracked");
    }

    if (device.DestroyPipeline(pipeline) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling pipeline destroy failed");
    }

    if (device.DestroyShaderModule(pixel_shader) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling pixel shader destroy failed");
    }

    if (device.DestroyShaderModule(vertex_shader) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling vertex shader destroy failed");
    }

    if (device.DestroySampler(sampler) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling sampler destroy failed");
    }

    if (device.DestroyTexture(texture) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling texture destroy failed");
    }

    if (device.DestroyBuffer(index_buffer) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling index buffer destroy failed");
    }

    if (device.DestroyBuffer(vertex_buffer) != RhiStatus::Success) {
        static_cast<void>(RhiDeviceFactory::DestroyDevice(create_result.device));
        return Fail("d3d11 texture sampling vertex buffer destroy failed");
    }

    const RhiStatus destroy_status = RhiDeviceFactory::DestroyDevice(create_result.device);
    if (destroy_status != RhiStatus::Success) {
        return Fail("d3d11 texture sampling device destroy failed");
    }

    return 0;
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail("expected one test name");
    }

    const std::string_view test_name(argv[1]);
    if (test_name == TEST_D3D11_CLEAR_PRESENT_CAPTURE) {
        return RunD3D11ClearPresentCapture();
    }

    if (test_name == TEST_D3D11_PRIMITIVE_RESOURCE_PIPELINE) {
        return RunD3D11PrimitiveResourcePipeline();
    }

    if (test_name == TEST_D3D11_VISIBLE_TRIANGLE) {
        return RunD3D11VisibleTriangleCapture();
    }

    if (test_name == TEST_D3D11_INDEXED_STATIC_MESH) {
        return RunD3D11IndexedStaticMeshCapture();
    }

    if (test_name == TEST_D3D11_TEXTURE_SAMPLING) {
        return RunD3D11TextureSamplingCapture();
    }

    if (test_name == TEST_D3D11_SWAPCHAIN_RESIZE_GENERATION) {
        return RunD3D11SwapchainResizeInvalidatesOldBackbufferGeneration();
    }

    if (test_name == TEST_D3D11_SWAPCHAIN_RESIZE_SAME_EXTENT) {
        return RunD3D11SwapchainResizeSameExtentNoOpKeepsGeneration();
    }

    if (test_name == TEST_D3D11_SWAPCHAIN_RESIZE_REJECT) {
        return RunD3D11SwapchainResizeRejectsInvalidExtent();
    }

    return Fail("unknown test name");
}
