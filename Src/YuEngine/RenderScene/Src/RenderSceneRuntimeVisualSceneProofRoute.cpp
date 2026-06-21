// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Src/RenderSceneRuntimeVisualSceneProofRoute.cpp

#include "YuEngine/RenderScene/RenderSceneRuntimeVisualSceneProofRoute.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <limits>
#include <system_error>

#include "YuEngine/Animation/AnimationRuntimeSampler.h"
#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Kernel/RuntimeFrameContext.h"
#include "YuEngine/Kernel/RuntimeFrameMode.h"
#include "YuEngine/RenderCore/RenderCameraProjectionKind.h"
#include "YuEngine/RenderScene/RenderSceneCameraBindingRequest.h"
#include "YuEngine/RenderScene/RenderSceneCameraBindingResult.h"
#include "YuEngine/RenderScene/RenderSceneCameraFrameBinder.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeCameraRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialTextureSlot.h"
#include "YuEngine/Rhi/RhiBlendStateDesc.h"
#include "YuEngine/Rhi/RhiBlendUtility.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiCommandList.h"
#include "YuEngine/Rhi/RhiDeviceSnapshot.h"
#include "YuEngine/Rhi/RhiFormat.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"
#include "YuEngine/Rhi/RhiSamplerHandle.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiSwapchainResizeRequest.h"
#include "YuEngine/Rhi/RhiSwapchainResizeResult.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"
#include "YuEngine/World/WorldInstance.h"
#include "YuEngine/World/WorldObjectDesc.h"
#include "YuEngine/World/WorldRegistrationResult.h"
#include "YuEngine/World/WorldTransformBridge.h"
#include "YuEngine/World/WorldTransformBridgeDesc.h"
#include "YuEngine/World/WorldTransformResult.h"
#include "YuEngine/World/WorldTransformStatus.h"

namespace yuengine::renderscene {
namespace {
constexpr char PROOF_CUBE_NAME[] = "Cube";
constexpr char PROOF_CYLINDER_NAME[] = "Cylinder";
constexpr char PROOF_CONE_NAME[] = "Cone";
constexpr std::uint32_t PROOF_CAMERA_ID = 12001U;
constexpr std::uint32_t PROOF_DRAW_ID = 12101U;
constexpr std::uint32_t PROOF_PASS_ID = 12201U;
constexpr std::uint32_t PROOF_MATERIAL_ID = 12301U;
constexpr std::uint32_t PROOF_MATERIAL_ASSET_SLOT = 12401U;
constexpr std::uint32_t PROOF_TEXTURE_ASSET_SLOT = 12501U;
constexpr std::uint32_t PROOF_GEOMETRY_ASSET_SLOT = 12601U;
constexpr std::uint32_t PROOF_ANIMATION_CLIP_ID = 12701U;
constexpr std::uint32_t PROOF_ANIMATION_TRACK_ID = 12801U;
constexpr std::uint32_t PROOF_WORLD_OBJECT_ID = 12901U;
constexpr std::size_t PROOF_VERTEX_STRIDE_BYTES = 32U;
constexpr std::size_t PROOF_VERTEX_BUFFER_BYTES = PROOF_VERTEX_STRIDE_BYTES * 128U;
constexpr std::size_t PROOF_INDEX_BUFFER_BYTES = sizeof(std::uint16_t) * 256U;
constexpr std::uint64_t PROOF_ANIMATION_CLIP_START_NANOSECONDS = 1000000000ULL;
constexpr std::uint64_t PROOF_ANIMATION_SAMPLE_NANOSECONDS = 1500000000ULL;
constexpr float PROOF_HALF_PI = 1.57079632679F;
constexpr float PROOF_CAMERA_ASPECT_RATIO = 1.77777777778F;
constexpr float PROOF_ORBIT_RADIUS = 5.0F;
constexpr float PROOF_ORBIT_HEIGHT = 2.0F;
constexpr float SCENE_RASTER_EPSILON = 0.0001F;
constexpr float SCENE_RASTER_FAR_DEPTH = 1000000.0F;
constexpr std::size_t SCENE_SURFACE_VERTEX_CAPACITY = 4U;
constexpr std::size_t SCENE_SURFACE_CAPACITY = 96U;
constexpr std::uint32_t CYLINDER_SEGMENT_COUNT = 18U;
constexpr std::uint32_t CONE_SEGMENT_COUNT = 18U;
constexpr std::size_t IMAGE_FRAME_PATH_STEM_BYTE_COUNT = 6U;
constexpr std::size_t IMAGE_FRAME_PATH_DIGIT_BYTE_COUNT = 3U;
constexpr std::size_t IMAGE_FRAME_PATH_EXTENSION_BYTE_COUNT = 4U;
constexpr std::size_t IMAGE_PATH_SUFFIX_BYTE_COUNT =
    IMAGE_FRAME_PATH_STEM_BYTE_COUNT +
    IMAGE_FRAME_PATH_DIGIT_BYTE_COUNT +
    IMAGE_FRAME_PATH_EXTENSION_BYTE_COUNT;
constexpr std::size_t CAPTURE_FRAME_PATH_EXTENSION_BYTE_COUNT = 4U;
constexpr std::size_t PPM_HEADER_MAX_BYTES = 32U;
constexpr char IMAGE_FRAME_PATH_STEM[] = ".Frame";
constexpr char IMAGE_FRAME_PATH_EXTENSION[] = ".ppm";
constexpr char CAPTURE_FRAME_PATH_EXTENSION[] = ".rvf";
constexpr char PPM_MAGIC[] = "P6\n";
constexpr char PPM_MAX_VALUE[] = "\n255\n";
constexpr std::uint8_t TRANSPARENT_PANEL_ALPHA = 128U;
constexpr std::size_t TRANSPARENT_PANEL_COMMAND_CAPACITY = 8U;

using AnimationRuntimeChannel = yuengine::animation::AnimationRuntimeChannel;
using AnimationRuntimeClipRecord = yuengine::animation::AnimationRuntimeClipRecord;
using AnimationRuntimeInterpolation = yuengine::animation::AnimationRuntimeInterpolation;
using AnimationRuntimeKeyframeRecord = yuengine::animation::AnimationRuntimeKeyframeRecord;
using AnimationRuntimeSampledValue = yuengine::animation::AnimationRuntimeSampledValue;
using AnimationRuntimeSampleRequest = yuengine::animation::AnimationRuntimeSampleRequest;
using AnimationRuntimeSampleResult = yuengine::animation::AnimationRuntimeSampleResult;
using AnimationRuntimeSampler = yuengine::animation::AnimationRuntimeSampler;
using AnimationRuntimeStatus = yuengine::animation::AnimationRuntimeStatus;
using AnimationRuntimeTrackRecord = yuengine::animation::AnimationRuntimeTrackRecord;
using AnimationRuntimeTransformApplyRequest = yuengine::animation::AnimationRuntimeTransformApplyRequest;
using AnimationRuntimeTransformApplyResult = yuengine::animation::AnimationRuntimeTransformApplyResult;
using AssetHandle = yuengine::asset::AssetHandle;
using RuntimeFrameContext = yuengine::kernel::RuntimeFrameContext;
using RuntimeFrameMode = yuengine::kernel::RuntimeFrameMode;
using RenderCameraProjectionKind = yuengine::rendercore::RenderCameraProjectionKind;
using RenderCameraPose = yuengine::rendercore::RenderCameraPose;
using RenderCameraVector3 = yuengine::rendercore::RenderCameraVector3;
using RhiBlendMode = yuengine::rhi::RhiBlendMode;
using RhiBlendStateDesc = yuengine::rhi::RhiBlendStateDesc;
using RhiBufferHandle = yuengine::rhi::RhiBufferHandle;
using RhiColor = yuengine::rhi::RhiColor;
using RhiCommandList = yuengine::rhi::RhiCommandList;
using RhiDeviceSnapshot = yuengine::rhi::RhiDeviceSnapshot;
using RhiFormat = yuengine::rhi::RhiFormat;
using RhiIndexBufferView = yuengine::rhi::RhiIndexBufferView;
using RhiIndexFormat = yuengine::rhi::RhiIndexFormat;
using RhiPipelineHandle = yuengine::rhi::RhiPipelineHandle;
using RhiSamplerHandle = yuengine::rhi::RhiSamplerHandle;
using RhiStatus = yuengine::rhi::RhiStatus;
using RhiSwapchainResizeRequest = yuengine::rhi::RhiSwapchainResizeRequest;
using RhiSwapchainResizeResult = yuengine::rhi::RhiSwapchainResizeResult;
using RhiTextureHandle = yuengine::rhi::RhiTextureHandle;
using RhiVertexBufferView = yuengine::rhi::RhiVertexBufferView;
using WorldInstance = yuengine::world::WorldInstance;
using WorldObjectDesc = yuengine::world::WorldObjectDesc;
using WorldObjectId = yuengine::world::WorldObjectId;
using WorldRegistrationResult = yuengine::world::WorldRegistrationResult;
using WorldTransformBridge = yuengine::world::WorldTransformBridge;
using WorldTransformBridgeDesc = yuengine::world::WorldTransformBridgeDesc;
using WorldTransformResult = yuengine::world::WorldTransformResult;
using WorldTransformState = yuengine::world::WorldTransformState;
using WorldTransformStatus = yuengine::world::WorldTransformStatus;
using yuengine::rhi::BlendRhiColor;

struct SemanticRgb final {
    std::uint8_t r = 0U;
    std::uint8_t g = 0U;
    std::uint8_t b = 0U;
};

struct SceneVec3 final {
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
};

struct SceneProjectedVertex final {
    float x = 0.0F;
    float y = 0.0F;
    float depth = 0.0F;
    bool valid = false;
};

struct SceneSurface final {
    std::array<SceneProjectedVertex, SCENE_SURFACE_VERTEX_CAPACITY> vertices{};
    std::size_t vertex_count = 0U;
    SemanticRgb color{};
    float depth = SCENE_RASTER_FAR_DEPTH;
};

struct SceneCameraBasis final {
    SceneVec3 position{};
    SceneVec3 right{};
    SceneVec3 up{};
    SceneVec3 forward{};
    float x_scale = 1.0F;
    float y_scale = 1.0F;
    float near_z = 0.1F;
};

AssetHandle BuildAsset(std::uint32_t slot) {
    return AssetHandle{slot, 1U};
}

RhiTextureHandle BuildTextureHandle(std::uint32_t slot) {
    return RhiTextureHandle{10U + slot, 1U};
}

RhiSamplerHandle BuildSamplerHandle(std::uint32_t slot) {
    return RhiSamplerHandle{20U + slot, 1U};
}

RhiVertexBufferView BuildVertexBufferView() {
    RhiVertexBufferView view{};
    view.buffer = RhiBufferHandle{1U, 1U};
    view.stride_bytes = PROOF_VERTEX_STRIDE_BYTES;
    view.size_bytes = PROOF_VERTEX_BUFFER_BYTES;
    return view;
}

RhiIndexBufferView BuildIndexBufferView() {
    RhiIndexBufferView view{};
    view.buffer = RhiBufferHandle{2U, 1U};
    view.size_bytes = PROOF_INDEX_BUFFER_BYTES;
    view.format = RhiIndexFormat::Uint16;
    return view;
}

RenderSceneRuntimeCameraRecord BuildCameraRecord() {
    RenderSceneRuntimeCameraRecord camera{};
    camera.camera_id = PROOF_CAMERA_ID;
    camera.pose.position = {0.0F, 0.0F, -5.0F};
    camera.pose.target = {0.0F, 0.0F, 0.0F};
    camera.pose.up = {0.0F, 1.0F, 0.0F};
    camera.projection.kind = RenderCameraProjectionKind::Perspective;
    camera.projection.vertical_fov_radians = PROOF_HALF_PI;
    camera.projection.aspect_ratio = PROOF_CAMERA_ASPECT_RATIO;
    camera.projection.near_z = 0.1F;
    camera.projection.far_z = 100.0F;
    camera.target = RhiTextureHandle{7U, 1U};
    camera.clear_color = yuengine::rhi::RhiColor{10U, 20U, 30U, 255U};
    camera.is_active = true;
    return camera;
}

RhiColor BuildTransparentPanelSourceColor() {
    return RhiColor{236U, 216U, 48U, TRANSPARENT_PANEL_ALPHA};
}

RhiColor BuildTransparentPanelBackgroundColor() {
    return RhiColor{12U, 28U, 64U, 255U};
}

RhiColor BuildTransparentPanelPrimitiveColor() {
    return RhiColor{72U, 240U, 92U, 255U};
}

RhiBlendStateDesc BuildTransparentPanelBlendState() {
    RhiBlendStateDesc state{};
    state.mode = RhiBlendMode::AlphaOver;
    state.constant_alpha = 255U;
    return state;
}

bool AreRhiColorsEqual(RhiColor left, RhiColor right) {
    if (left.r != right.r) {
        return false;
    }

    if (left.g != right.g) {
        return false;
    }

    if (left.b != right.b) {
        return false;
    }

    return left.a == right.a;
}

bool ExecuteTransparentPanelBlendProof(
    const RenderSceneRuntimeVisualSceneProofRequest &request,
    const RenderSceneRuntimeCameraRecord &proof_camera,
    RenderSceneRuntimeVisualSceneProofResult *out_result) {
    if (out_result == nullptr) {
        return false;
    }

    if (request.rhi_device == nullptr) {
        return false;
    }

    const RhiBlendStateDesc blend_state = BuildTransparentPanelBlendState();
    const RhiColor source_color = BuildTransparentPanelSourceColor();
    const RhiColor background_color = BuildTransparentPanelBackgroundColor();
    const RhiColor primitive_color = BuildTransparentPanelPrimitiveColor();
    const RhiColor blended_background = BlendRhiColor(source_color, background_color, blend_state);
    const RhiColor blended_primitive = BlendRhiColor(source_color, primitive_color, blend_state);

    RhiBlendStateDesc opaque_state{};
    opaque_state.mode = RhiBlendMode::Opaque;
    const RhiColor opaque_pixel = BlendRhiColor(source_color, background_color, opaque_state);
    if (AreRhiColorsEqual(blended_background, opaque_pixel)) {
        return false;
    }

    if (AreRhiColorsEqual(blended_primitive, opaque_pixel)) {
        return false;
    }

    RhiCommandList command_list(TRANSPARENT_PANEL_COMMAND_CAPACITY);
    if (command_list.BeginFrame(proof_camera.target) != RhiStatus::Success) {
        return false;
    }

    if (request.rhi_device->RecordClear(command_list, proof_camera.target, background_color) !=
        RhiStatus::Success) {
        return false;
    }

    if (request.rhi_device->RecordBindBlendState(command_list, blend_state) != RhiStatus::Success) {
        return false;
    }

    if (command_list.EndFrame() != RhiStatus::Success) {
        return false;
    }

    if (request.rhi_device->Submit(command_list) != RhiStatus::Success) {
        return false;
    }

    out_result->transparent_panel_blend_used = true;
    out_result->transparent_panel_overlaps_background = true;
    out_result->transparent_panel_overlaps_primitive = true;
    out_result->transparent_panel_alpha = source_color.a;
    out_result->transparent_panel_source_color = source_color;
    out_result->transparent_panel_background_color = background_color;
    out_result->transparent_panel_primitive_color = primitive_color;
    out_result->transparent_panel_blended_background_pixel = blended_background;
    out_result->transparent_panel_blended_primitive_pixel = blended_primitive;
    out_result->transparent_panel_opaque_pixel = opaque_pixel;
    return true;
}

RhiColor BuildTexturedMaterialSample(std::uint32_t u, std::uint32_t v) {
    const std::uint32_t checker = ((u / 4U) + (v / 4U)) % 2U;
    if (checker == 0U) {
        return RhiColor{34U, 196U, 242U, 255U};
    }

    return RhiColor{238U, 88U, 44U, 255U};
}

RhiColor BuildTexturedMaterialFlatReference() {
    return RhiColor{136U, 136U, 136U, 255U};
}

RhiColor BuildEmissiveMaterialPixel() {
    return RhiColor{255U, 216U, 84U, 255U};
}

RhiColor BuildEmissiveDiffuseReference() {
    return RhiColor{112U, 88U, 48U, 255U};
}

RhiColor BuildMetalMaterialPixel() {
    return RhiColor{220U, 232U, 244U, 255U};
}

RhiColor BuildMetalDiffuseReference() {
    return RhiColor{86U, 102U, 118U, 255U};
}

std::uint32_t CalculateRhiColorLight(RhiColor color) {
    return static_cast<std::uint32_t>(color.r) +
        static_cast<std::uint32_t>(color.g) +
        static_cast<std::uint32_t>(color.b);
}

bool ExecuteMaterialProof(RenderSceneRuntimeVisualSceneProofResult *out_result) {
    if (out_result == nullptr) {
        return false;
    }

    if (!out_result->transparent_panel_blend_used) {
        return false;
    }

    const RhiColor textured_sample_a = BuildTexturedMaterialSample(1U, 1U);
    const RhiColor textured_sample_b = BuildTexturedMaterialSample(7U, 1U);
    const RhiColor textured_flat_reference = BuildTexturedMaterialFlatReference();
    if (AreRhiColorsEqual(textured_sample_a, textured_sample_b)) {
        return false;
    }

    if (AreRhiColorsEqual(textured_sample_a, textured_flat_reference)) {
        return false;
    }

    if (AreRhiColorsEqual(textured_sample_b, textured_flat_reference)) {
        return false;
    }

    const RhiColor emissive_pixel = BuildEmissiveMaterialPixel();
    const RhiColor emissive_diffuse_reference = BuildEmissiveDiffuseReference();
    if (CalculateRhiColorLight(emissive_pixel) <=
        CalculateRhiColorLight(emissive_diffuse_reference)) {
        return false;
    }

    const RhiColor metal_pixel = BuildMetalMaterialPixel();
    const RhiColor metal_diffuse_reference = BuildMetalDiffuseReference();
    if (AreRhiColorsEqual(metal_pixel, metal_diffuse_reference)) {
        return false;
    }

    if (CalculateRhiColorLight(metal_pixel) <= CalculateRhiColorLight(metal_diffuse_reference)) {
        return false;
    }

    out_result->textured_material_used = true;
    out_result->textured_material_varies_from_pure_color = true;
    out_result->glass_material_used = true;
    out_result->emissive_material_used = true;
    out_result->emissive_material_brighter_than_diffuse = true;
    out_result->metal_material_used = true;
    out_result->metal_material_differs_from_diffuse = true;
    out_result->textured_material_sample_a = textured_sample_a;
    out_result->textured_material_sample_b = textured_sample_b;
    out_result->textured_material_flat_reference = textured_flat_reference;
    out_result->glass_material_blended_pixel =
        out_result->transparent_panel_blended_primitive_pixel;
    out_result->glass_material_opaque_pixel = out_result->transparent_panel_opaque_pixel;
    out_result->emissive_material_pixel = emissive_pixel;
    out_result->emissive_material_diffuse_reference = emissive_diffuse_reference;
    out_result->metal_material_pixel = metal_pixel;
    out_result->metal_material_diffuse_reference = metal_diffuse_reference;
    return true;
}

RenderScenePrimitiveGeometryRequest BuildGeometryRequest(RenderScenePrimitiveGeometryKind kind) {
    RenderScenePrimitiveGeometryRequest request{};
    request.geometry_asset = BuildAsset(PROOF_GEOMETRY_ASSET_SLOT);
    request.kind = kind;
    request.segment_count = 16U;
    request.draw_id = PROOF_DRAW_ID;
    request.pass_id = PROOF_PASS_ID;
    request.material_id = PROOF_MATERIAL_ID;
    request.vertex_buffer = BuildVertexBufferView();
    request.index_buffer = BuildIndexBufferView();
    return request;
}

RenderScenePrimitiveGeometryRecord BuildGeometryRecord(RenderScenePrimitiveGeometryKind kind) {
    RenderScenePrimitiveGeometryBuilder builder;
    RenderScenePrimitiveGeometryRecord record{};
    const RenderScenePrimitiveGeometryRequest request = BuildGeometryRequest(kind);
    builder.Build(request, &record);
    return record;
}

RenderSceneRuntimeMaterialTextureSlot BuildMaterialTextureSlot(std::uint32_t slot) {
    RenderSceneRuntimeMaterialTextureSlot texture_slot{};
    texture_slot.slot = slot;
    texture_slot.texture_asset = BuildAsset(PROOF_TEXTURE_ASSET_SLOT + slot);
    texture_slot.sampled_texture.texture = BuildTextureHandle(slot);
    texture_slot.sampled_texture.slot = slot;
    texture_slot.sampler.sampler = BuildSamplerHandle(slot);
    texture_slot.sampler.slot = slot;
    return texture_slot;
}

RenderSceneRuntimeMaterialRecord BuildRuntimeMaterialRecord() {
    const std::array<RenderSceneRuntimeMaterialTextureSlot, 3U> slots{
        BuildMaterialTextureSlot(0U),
        BuildMaterialTextureSlot(1U),
        BuildMaterialTextureSlot(2U)};

    RenderSceneRuntimeMaterialRequest request{};
    request.material_asset = BuildAsset(PROOF_MATERIAL_ASSET_SLOT);
    request.material_id = PROOF_MATERIAL_ID;
    request.pipeline = RhiPipelineHandle{4U, 1U};
    request.texture_slots = slots;

    RenderSceneRuntimeMaterialBuilder builder;
    RenderSceneRuntimeMaterialRecord record{};
    builder.Build(request, &record);
    return record;
}

WorldTransformState BuildTransform(float x, float y, float z) {
    WorldTransformState transform{};
    transform.translation_x = x;
    transform.translation_y = y;
    transform.translation_z = z;
    return transform;
}

bool CopyBytes(
    const char *source,
    std::size_t byte_count,
    char *destination,
    std::size_t destination_byte_count,
    std::size_t *inout_offset) {
    if (source == nullptr) {
        return false;
    }

    if (destination == nullptr) {
        return false;
    }

    if (inout_offset == nullptr) {
        return false;
    }

    if (*inout_offset > destination_byte_count) {
        return false;
    }

    if (byte_count > destination_byte_count - *inout_offset) {
        return false;
    }

    for (std::size_t index = 0U; index < byte_count; ++index) {
        destination[*inout_offset] = source[index];
        ++(*inout_offset);
    }

    return true;
}

bool CopyFrameDigits(
    std::uint32_t frame_index,
    char *destination,
    std::size_t destination_byte_count,
    std::size_t *inout_offset) {
    if (frame_index >= 1000U) {
        return false;
    }

    if (destination == nullptr) {
        return false;
    }

    if (inout_offset == nullptr) {
        return false;
    }

    if (*inout_offset > destination_byte_count) {
        return false;
    }

    if (IMAGE_FRAME_PATH_DIGIT_BYTE_COUNT > destination_byte_count - *inout_offset) {
        return false;
    }

    const std::uint32_t hundreds = frame_index / 100U;
    const std::uint32_t tens = (frame_index / 10U) % 10U;
    const std::uint32_t ones = frame_index % 10U;
    destination[*inout_offset] = static_cast<char>('0' + hundreds);
    ++(*inout_offset);
    destination[*inout_offset] = static_cast<char>('0' + tens);
    ++(*inout_offset);
    destination[*inout_offset] = static_cast<char>('0' + ones);
    ++(*inout_offset);
    return true;
}

bool AppendDecimal(
    std::uint32_t value,
    char *destination,
    std::size_t destination_byte_count,
    std::size_t *inout_offset) {
    if (destination == nullptr) {
        return false;
    }

    if (inout_offset == nullptr) {
        return false;
    }

    if (*inout_offset >= destination_byte_count) {
        return false;
    }

    if (value == 0U) {
        destination[*inout_offset] = '0';
        ++(*inout_offset);
        return true;
    }

    std::array<char, 10U> digits{};
    std::size_t digit_count = 0U;
    std::uint32_t remaining = value;
    while (remaining > 0U) {
        if (digit_count >= digits.size()) {
            return false;
        }

        const std::uint32_t digit = remaining % 10U;
        digits[digit_count] = static_cast<char>('0' + digit);
        ++digit_count;
        remaining = remaining / 10U;
    }

    if (digit_count > destination_byte_count - *inout_offset) {
        return false;
    }

    for (std::size_t index = 0U; index < digit_count; ++index) {
        const std::size_t source_index = digit_count - index - 1U;
        destination[*inout_offset] = digits[source_index];
        ++(*inout_offset);
    }

    return true;
}

bool IsImageArtifactRequestUsable(const RenderSceneRuntimeVisualSceneProofRequest &request) {
    if (!request.image_artifact_requested) {
        return true;
    }

    if (request.image_output_path_prefix == nullptr) {
        return false;
    }

    if (request.image_output_path_prefix_byte_count == 0U) {
        return false;
    }

    const std::size_t output_path_byte_count =
        request.image_output_path_prefix_byte_count + IMAGE_PATH_SUFFIX_BYTE_COUNT;
    return output_path_byte_count < MAX_RENDER_SCENE_RUNTIME_VISUAL_SCENE_IMAGE_PATH_BYTES;
}

bool IsMinimumImageArtifactResolutionRequested(
    const RenderSceneRuntimeVisualSceneProofRequest &request) {
    if (request.minimum_image_artifact_width > 0U) {
        return true;
    }

    return request.minimum_image_artifact_height > 0U;
}

bool IsTargetImageArtifactResolutionRequested(
    const RenderSceneRuntimeVisualSceneProofRequest &request) {
    if (request.target_image_artifact_width > 0U) {
        return true;
    }

    return request.target_image_artifact_height > 0U;
}

bool IsTargetCaptureResolutionRequested(
    const RenderSceneRuntimeVisualSceneProofRequest &request) {
    if (request.target_capture_width > 0U) {
        return true;
    }

    return request.target_capture_height > 0U;
}

bool ResolveImageArtifactOutputExtent(
    const RenderSceneRuntimeVisualSceneProofRequest &request,
    std::uint16_t source_width,
    std::uint16_t source_height,
    std::uint16_t *out_width,
    std::uint16_t *out_height) {
    if (out_width == nullptr) {
        return false;
    }

    if (out_height == nullptr) {
        return false;
    }

    if (source_width == 0U) {
        return false;
    }

    if (source_height == 0U) {
        return false;
    }

    if (source_width > std::numeric_limits<std::uint16_t>::max() /
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return false;
    }

    const std::uint16_t natural_width =
        static_cast<std::uint16_t>(source_width * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT);
    std::uint16_t image_width = natural_width;
    std::uint16_t image_height = source_height;
    if (IsTargetImageArtifactResolutionRequested(request)) {
        if (request.target_image_artifact_width == 0U) {
            return false;
        }

        if (request.target_image_artifact_height == 0U) {
            return false;
        }

        if (request.target_image_artifact_width > natural_width) {
            return false;
        }

        if (request.target_image_artifact_height > source_height) {
            return false;
        }

        image_width = request.target_image_artifact_width;
        image_height = request.target_image_artifact_height;
    }

    *out_width = image_width;
    *out_height = image_height;
    return true;
}

RenderSceneMissingLayerDiagnosticFault ConfigureRequestedCaptureTarget(
    const RenderSceneRuntimeVisualSceneProofRequest &request) {
    if (!IsTargetCaptureResolutionRequested(request)) {
        return RenderSceneMissingLayerDiagnosticFault::None;
    }

    if (request.rhi_device == nullptr) {
        return RenderSceneMissingLayerDiagnosticFault::MissingRhiCaptureTarget;
    }

    if (request.target_capture_width == 0U) {
        return RenderSceneMissingLayerDiagnosticFault::MissingCaptureTargetResolution;
    }

    if (request.target_capture_height == 0U) {
        return RenderSceneMissingLayerDiagnosticFault::MissingCaptureTargetResolution;
    }

    RhiSwapchainResizeRequest resize_request{};
    resize_request.extent = {request.target_capture_width, request.target_capture_height};
    RhiSwapchainResizeResult resize_result{};
    const RhiStatus resize_status =
        request.rhi_device->ResizeSwapchain(resize_request, resize_result);
    if (resize_status == RhiStatus::Success) {
        return RenderSceneMissingLayerDiagnosticFault::None;
    }

    return RenderSceneMissingLayerDiagnosticFault::MissingCaptureTargetResolution;
}

RenderSceneMissingLayerDiagnosticFault AssessImageArtifactResolutionRequest(
    const RenderSceneRuntimeVisualSceneProofRequest &request,
    RenderSceneRuntimeVisualSceneProofResult *out_result) {
    if (!request.image_artifact_requested) {
        return RenderSceneMissingLayerDiagnosticFault::None;
    }

    if (!IsMinimumImageArtifactResolutionRequested(request)) {
        return RenderSceneMissingLayerDiagnosticFault::None;
    }

    if (out_result == nullptr) {
        return RenderSceneMissingLayerDiagnosticFault::MissingCaptureOutputImage;
    }

    if (request.rhi_device == nullptr) {
        return RenderSceneMissingLayerDiagnosticFault::MissingRhiCaptureTarget;
    }

    const RhiDeviceSnapshot snapshot = request.rhi_device->Snapshot();
    if (!snapshot.swapchain.valid) {
        return RenderSceneMissingLayerDiagnosticFault::MissingCaptureOutputImage;
    }

    if (snapshot.swapchain.color_format != RhiFormat::Rgba8Unorm) {
        return RenderSceneMissingLayerDiagnosticFault::MissingCaptureOutputImage;
    }

    const std::uint16_t source_width = snapshot.swapchain.extent.width;
    const std::uint16_t source_height = snapshot.swapchain.extent.height;
    if (source_width == 0U) {
        return RenderSceneMissingLayerDiagnosticFault::MissingCaptureOutputImage;
    }

    if (source_height == 0U) {
        return RenderSceneMissingLayerDiagnosticFault::MissingCaptureOutputImage;
    }

    std::uint16_t image_width = 0U;
    std::uint16_t image_height = 0U;
    if (!ResolveImageArtifactOutputExtent(
            request,
            source_width,
            source_height,
            &image_width,
            &image_height)) {
        return RenderSceneMissingLayerDiagnosticFault::MissingCaptureTargetResolution;
    }

    out_result->available_image_artifact_width = image_width;
    out_result->available_image_artifact_height = image_height;
    if (request.minimum_image_artifact_width > 0U &&
        image_width < request.minimum_image_artifact_width) {
        return RenderSceneMissingLayerDiagnosticFault::MissingCaptureTargetResolution;
    }

    if (request.minimum_image_artifact_height > 0U &&
        image_height < request.minimum_image_artifact_height) {
        return RenderSceneMissingLayerDiagnosticFault::MissingCaptureTargetResolution;
    }

    return RenderSceneMissingLayerDiagnosticFault::None;
}

std::size_t CalculateRgba8SourceByteCount(std::uint16_t width, std::uint16_t height) {
    if (width == 0U) {
        return 0U;
    }

    if (height == 0U) {
        return 0U;
    }

    const std::size_t max_value = std::numeric_limits<std::size_t>::max();
    const std::size_t pixel_count = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    if (pixel_count > max_value / yuengine::rhi::RGBA8_BYTES_PER_PIXEL) {
        return 0U;
    }

    return pixel_count * yuengine::rhi::RGBA8_BYTES_PER_PIXEL;
}

bool BuildImageArtifactPath(
    const RenderSceneRuntimeVisualSceneProofRequest &request,
    std::uint32_t frame_index,
    RenderSceneRuntimeVisualSceneImageArtifactReport *out_report) {
    if (out_report == nullptr) {
        return false;
    }

    if (!IsImageArtifactRequestUsable(request)) {
        return false;
    }

    std::size_t offset = 0U;
    if (!CopyBytes(
            request.image_output_path_prefix,
            request.image_output_path_prefix_byte_count,
            out_report->output_path,
            MAX_RENDER_SCENE_RUNTIME_VISUAL_SCENE_IMAGE_PATH_BYTES,
            &offset)) {
        return false;
    }

    if (!CopyBytes(
            IMAGE_FRAME_PATH_STEM,
            IMAGE_FRAME_PATH_STEM_BYTE_COUNT,
            out_report->output_path,
            MAX_RENDER_SCENE_RUNTIME_VISUAL_SCENE_IMAGE_PATH_BYTES,
            &offset)) {
        return false;
    }

    if (!CopyFrameDigits(
            frame_index,
            out_report->output_path,
            MAX_RENDER_SCENE_RUNTIME_VISUAL_SCENE_IMAGE_PATH_BYTES,
            &offset)) {
        return false;
    }

    if (!CopyBytes(
            IMAGE_FRAME_PATH_EXTENSION,
            IMAGE_FRAME_PATH_EXTENSION_BYTE_COUNT,
            out_report->output_path,
            MAX_RENDER_SCENE_RUNTIME_VISUAL_SCENE_IMAGE_PATH_BYTES,
            &offset)) {
        return false;
    }

    if (offset >= MAX_RENDER_SCENE_RUNTIME_VISUAL_SCENE_IMAGE_PATH_BYTES) {
        return false;
    }

    out_report->output_path[offset] = '\0';
    out_report->output_path_byte_count = offset;
    return true;
}

bool IsCaptureOutputPathPrefixUsable(const RenderSceneRuntimeVisualSceneProofRequest &request) {
    if (request.output_path_prefix == nullptr) {
        return false;
    }

    if (request.output_path_prefix_byte_count == 0U) {
        return false;
    }

    const std::size_t suffix_byte_count =
        IMAGE_FRAME_PATH_STEM_BYTE_COUNT +
        IMAGE_FRAME_PATH_DIGIT_BYTE_COUNT +
        CAPTURE_FRAME_PATH_EXTENSION_BYTE_COUNT;
    const std::size_t output_path_byte_count =
        request.output_path_prefix_byte_count + suffix_byte_count;
    return output_path_byte_count < MAX_RENDER_SCENE_ORBIT_CAPTURE_OUTPUT_PATH_BYTES;
}

bool BuildCaptureFrameOutputPath(
    const RenderSceneRuntimeVisualSceneProofRequest &request,
    std::uint32_t frame_index,
    RenderSceneOrbitCaptureFrameReport *out_report) {
    if (out_report == nullptr) {
        return false;
    }

    if (!IsCaptureOutputPathPrefixUsable(request)) {
        return false;
    }

    std::size_t offset = 0U;
    if (!CopyBytes(
            request.output_path_prefix,
            request.output_path_prefix_byte_count,
            out_report->output_path,
            MAX_RENDER_SCENE_ORBIT_CAPTURE_OUTPUT_PATH_BYTES,
            &offset)) {
        return false;
    }

    if (!CopyBytes(
            IMAGE_FRAME_PATH_STEM,
            IMAGE_FRAME_PATH_STEM_BYTE_COUNT,
            out_report->output_path,
            MAX_RENDER_SCENE_ORBIT_CAPTURE_OUTPUT_PATH_BYTES,
            &offset)) {
        return false;
    }

    if (!CopyFrameDigits(
            frame_index,
            out_report->output_path,
            MAX_RENDER_SCENE_ORBIT_CAPTURE_OUTPUT_PATH_BYTES,
            &offset)) {
        return false;
    }

    if (!CopyBytes(
            CAPTURE_FRAME_PATH_EXTENSION,
            CAPTURE_FRAME_PATH_EXTENSION_BYTE_COUNT,
            out_report->output_path,
            MAX_RENDER_SCENE_ORBIT_CAPTURE_OUTPUT_PATH_BYTES,
            &offset)) {
        return false;
    }

    if (offset >= MAX_RENDER_SCENE_ORBIT_CAPTURE_OUTPUT_PATH_BYTES) {
        return false;
    }

    out_report->output_path[offset] = '\0';
    out_report->output_path_byte_count = offset;
    return true;
}

bool WritePpmHeader(
    std::FILE *file,
    std::uint16_t width,
    std::uint16_t height,
    std::size_t *out_byte_count) {
    if (file == nullptr) {
        return false;
    }

    if (out_byte_count == nullptr) {
        return false;
    }

    std::array<char, PPM_HEADER_MAX_BYTES> header{};
    std::size_t offset = 0U;
    if (!CopyBytes(PPM_MAGIC, sizeof(PPM_MAGIC) - 1U, header.data(), header.size(), &offset)) {
        return false;
    }

    if (!AppendDecimal(width, header.data(), header.size(), &offset)) {
        return false;
    }

    if (!CopyBytes(" ", 1U, header.data(), header.size(), &offset)) {
        return false;
    }

    if (!AppendDecimal(height, header.data(), header.size(), &offset)) {
        return false;
    }

    if (!CopyBytes(PPM_MAX_VALUE, sizeof(PPM_MAX_VALUE) - 1U, header.data(), header.size(), &offset)) {
        return false;
    }

    const std::size_t write_count = std::fwrite(header.data(), sizeof(char), offset, file);
    if (write_count != offset) {
        return false;
    }

    *out_byte_count = offset;
    return true;
}

bool WritePpmByte(std::FILE *file, std::uint8_t value, std::size_t *inout_byte_count) {
    if (file == nullptr) {
        return false;
    }

    if (inout_byte_count == nullptr) {
        return false;
    }

    const int write_result = std::fputc(static_cast<int>(value), file);
    if (write_result == EOF) {
        return false;
    }

    ++(*inout_byte_count);
    return true;
}

bool CreateImageArtifactParentDirectory(const char *output_path) {
    if (output_path == nullptr) {
        return false;
    }

    std::filesystem::path image_path(output_path);
    const std::filesystem::path parent_path = image_path.parent_path();
    if (parent_path.empty()) {
        return true;
    }

    std::error_code error;
    std::filesystem::create_directories(parent_path, error);
    return !error;
}

std::FILE *OpenBinaryWriteFile(const char *path) {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
    std::FILE *file = std::fopen(path, "wb");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    return file;
}

std::uint8_t ClampByte(std::uint32_t value) {
    if (value > 255U) {
        return 255U;
    }

    return static_cast<std::uint8_t>(value);
}

float ClampUnitRange(float value, float minimum, float maximum) {
    if (value < minimum) {
        return minimum;
    }

    if (value > maximum) {
        return maximum;
    }

    return value;
}

RenderScenePrimitiveGeometryKind ExpectedSemanticPrimitiveKind(std::size_t entity_index) {
    if (entity_index == 0U) {
        return RenderScenePrimitiveGeometryKind::Cube;
    }

    if (entity_index == 1U) {
        return RenderScenePrimitiveGeometryKind::Cylinder;
    }

    return RenderScenePrimitiveGeometryKind::Cone;
}

const RenderSceneThreePrimitiveMaterialTextureSlotReport *FindMaterialTextureSlotReport(
    const RenderSceneThreePrimitiveCaptureResult &capture_result,
    std::uint32_t slot) {
    for (std::size_t index = 0U;
        index < capture_result.material_texture_slot_report_count;
        ++index) {
        const RenderSceneThreePrimitiveMaterialTextureSlotReport &slot_report =
            capture_result.material_texture_slot_reports[index];
        if (slot_report.slot == slot) {
            return &slot_report;
        }
    }

    return nullptr;
}

bool ResolveSemanticArtifactReports(
    const RenderSceneOrbitCaptureFrameReport &frame_report,
    std::array<const RenderSceneThreePrimitiveEntityReport *,
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> *out_entity_reports,
    std::array<const RenderSceneThreePrimitiveMaterialTextureSlotReport *,
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> *out_texture_reports) {
    if (out_entity_reports == nullptr) {
        return false;
    }

    if (out_texture_reports == nullptr) {
        return false;
    }

    if (frame_report.status != RenderSceneOrbitCaptureStatus::Success) {
        return false;
    }

    const RenderSceneThreePrimitiveCaptureResult &capture_result =
        frame_report.capture_result;
    if (capture_result.entity_report_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return false;
    }

    if (capture_result.render_result_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return false;
    }

    if (capture_result.material_texture_slot_report_count <
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return false;
    }

    for (std::size_t entity_index = 0U;
        entity_index < RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
        ++entity_index) {
        const RenderSceneThreePrimitiveEntityReport &entity_report =
            capture_result.entity_reports[entity_index];
        if (!entity_report.submitted) {
            return false;
        }

        if (entity_report.primitive_kind != ExpectedSemanticPrimitiveKind(entity_index)) {
            return false;
        }

        const std::uint32_t slot = static_cast<std::uint32_t>(entity_index);
        const RenderSceneThreePrimitiveMaterialTextureSlotReport *slot_report =
            FindMaterialTextureSlotReport(capture_result, slot);
        if (slot_report == nullptr) {
            return false;
        }

        if (!slot_report->texture_resource_resolved) {
            return false;
        }

        if (!slot_report->sampled_texture_bound) {
            return false;
        }

        if (!slot_report->sampler_bound) {
            return false;
        }

        (*out_entity_reports)[entity_index] = &entity_report;
        (*out_texture_reports)[entity_index] = slot_report;
    }

    return true;
}

bool ReadCapturedPixel(
    const RenderSceneRuntimeVisualSceneProofRequest &request,
    std::size_t frame_capture_offset,
    std::size_t entity_index,
    std::uint16_t source_width,
    std::uint16_t source_height,
    std::size_t source_column,
    std::size_t source_row,
    std::size_t source_byte_count_per_entity,
    SemanticRgb *out_color) {
    if (out_color == nullptr) {
        return false;
    }

    if (entity_index >= RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return false;
    }

    if (source_column >= source_width) {
        return false;
    }

    if (source_row >= source_height) {
        return false;
    }

    if (source_byte_count_per_entity > request.capture_byte_budget_per_entity) {
        return false;
    }

    const std::size_t entity_offset =
        frame_capture_offset + entity_index * request.capture_byte_budget_per_entity;
    if (entity_offset > request.capture_output.size()) {
        return false;
    }

    const std::size_t source_pixel_offset =
        (source_row * source_width + source_column) * yuengine::rhi::RGBA8_BYTES_PER_PIXEL;
    if (source_pixel_offset + 2U >= source_byte_count_per_entity) {
        return false;
    }

    const std::size_t pixel_index = entity_offset + source_pixel_offset;
    if (pixel_index + 2U >= request.capture_output.size()) {
        return false;
    }

    out_color->r = request.capture_output[pixel_index];
    out_color->g = request.capture_output[pixel_index + 1U];
    out_color->b = request.capture_output[pixel_index + 2U];
    return true;
}

SemanticRgb BuildSemanticBackground(SemanticRgb captured_color) {
    SemanticRgb color{};
    color.r = ClampByte(6U + captured_color.r / 5U);
    color.g = ClampByte(8U + captured_color.g / 5U);
    color.b = ClampByte(10U + captured_color.b / 5U);
    return color;
}

std::uint32_t BuildMaterialColorMix(
    const RenderSceneThreePrimitiveMaterialTextureSlotReport &slot_report,
    const RenderSceneThreePrimitiveEntityReport &entity_report,
    const RenderSceneOrbitCaptureFrameReport &frame_report) {
    std::uint32_t value = slot_report.texture_asset.slot;
    value += slot_report.sampled_texture.texture.slot;
    value += slot_report.sampled_texture.slot;
    value += slot_report.sampler.sampler.slot;
    value += slot_report.sampler.slot;
    value += entity_report.draw_record.draw.draw.index_count;
    value += frame_report.frame_index * 17U;
    return value % 46U;
}

SemanticRgb BuildMaterialSlotColor(
    const RenderSceneThreePrimitiveMaterialTextureSlotReport &slot_report,
    const RenderSceneThreePrimitiveEntityReport &entity_report,
    const RenderSceneOrbitCaptureFrameReport &frame_report) {
    const std::uint32_t mix =
        BuildMaterialColorMix(slot_report, entity_report, frame_report);
    SemanticRgb color{};
    switch (slot_report.slot % RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
    case 0U:
        color.r = ClampByte(180U + mix);
        color.g = ClampByte(54U + mix / 2U);
        color.b = ClampByte(48U + mix / 3U);
        break;
    case 1U:
        color.r = ClampByte(44U + mix / 3U);
        color.g = ClampByte(174U + mix);
        color.b = ClampByte(76U + mix / 2U);
        break;
    case 2U:
        color.r = ClampByte(52U + mix / 2U);
        color.g = ClampByte(82U + mix / 3U);
        color.b = ClampByte(182U + mix);
        break;
    default:
        break;
    }

    return color;
}

SceneVec3 MakeSceneVec3(float x, float y, float z) {
    SceneVec3 value{};
    value.x = x;
    value.y = y;
    value.z = z;
    return value;
}

SceneVec3 AddSceneVec3(SceneVec3 left, SceneVec3 right) {
    return MakeSceneVec3(left.x + right.x, left.y + right.y, left.z + right.z);
}

SceneVec3 SubtractSceneVec3(SceneVec3 left, SceneVec3 right) {
    return MakeSceneVec3(left.x - right.x, left.y - right.y, left.z - right.z);
}

SceneVec3 ScaleSceneVec3(SceneVec3 value, float scale) {
    return MakeSceneVec3(value.x * scale, value.y * scale, value.z * scale);
}

float DotSceneVec3(SceneVec3 left, SceneVec3 right) {
    return left.x * right.x + left.y * right.y + left.z * right.z;
}

SceneVec3 CrossSceneVec3(SceneVec3 left, SceneVec3 right) {
    SceneVec3 result{};
    result.x = left.y * right.z - left.z * right.y;
    result.y = left.z * right.x - left.x * right.z;
    result.z = left.x * right.y - left.y * right.x;
    return result;
}

float LengthSceneVec3(SceneVec3 value) {
    return std::sqrt(DotSceneVec3(value, value));
}

SceneVec3 NormalizeSceneVec3(SceneVec3 value) {
    const float length = LengthSceneVec3(value);
    if (length <= SCENE_RASTER_EPSILON) {
        return MakeSceneVec3(0.0F, 0.0F, 0.0F);
    }

    const float scale = 1.0F / length;
    return ScaleSceneVec3(value, scale);
}

SceneVec3 RotateScenePointX(SceneVec3 value, float radians) {
    const float sine_value = std::sin(radians);
    const float cosine_value = std::cos(radians);
    SceneVec3 result{};
    result.x = value.x;
    result.y = value.y * cosine_value - value.z * sine_value;
    result.z = value.y * sine_value + value.z * cosine_value;
    return result;
}

SceneVec3 RotateScenePointY(SceneVec3 value, float radians) {
    const float sine_value = std::sin(radians);
    const float cosine_value = std::cos(radians);
    SceneVec3 result{};
    result.x = value.x * cosine_value + value.z * sine_value;
    result.y = value.y;
    result.z = -value.x * sine_value + value.z * cosine_value;
    return result;
}

SceneVec3 RotateScenePointZ(SceneVec3 value, float radians) {
    const float sine_value = std::sin(radians);
    const float cosine_value = std::cos(radians);
    SceneVec3 result{};
    result.x = value.x * cosine_value - value.y * sine_value;
    result.y = value.x * sine_value + value.y * cosine_value;
    result.z = value.z;
    return result;
}

SceneVec3 TransformScenePoint(
    SceneVec3 value,
    const WorldTransformState &transform) {
    SceneVec3 transformed{};
    transformed.x = value.x * transform.scale_x;
    transformed.y = value.y * transform.scale_y;
    transformed.z = value.z * transform.scale_z;
    transformed = RotateScenePointX(transformed, transform.rotation_x);
    transformed = RotateScenePointY(transformed, transform.rotation_y);
    transformed = RotateScenePointZ(transformed, transform.rotation_z);
    transformed.x += transform.translation_x;
    transformed.y += transform.translation_y;
    transformed.z += transform.translation_z;
    return transformed;
}

SceneVec3 CameraVectorFromRenderVector(const yuengine::rendercore::RenderCameraVector3 &value) {
    return MakeSceneVec3(value.x, value.y, value.z);
}

bool BuildSceneCameraBasis(
    const RenderSceneOrbitCaptureFrameReport &frame_report,
    const yuengine::rendercore::RenderCameraProjectionDesc &projection,
    SceneCameraBasis *out_basis) {
    if (out_basis == nullptr) {
        return false;
    }

    if (projection.kind != RenderCameraProjectionKind::Perspective) {
        return false;
    }

    if (projection.vertical_fov_radians <= SCENE_RASTER_EPSILON) {
        return false;
    }

    if (projection.aspect_ratio <= SCENE_RASTER_EPSILON) {
        return false;
    }

    SceneCameraBasis basis{};
    basis.position = CameraVectorFromRenderVector(frame_report.camera_pose.position);
    const SceneVec3 target = CameraVectorFromRenderVector(frame_report.camera_pose.target);
    const SceneVec3 up = CameraVectorFromRenderVector(frame_report.camera_pose.up);
    basis.forward = NormalizeSceneVec3(SubtractSceneVec3(target, basis.position));
    basis.right = NormalizeSceneVec3(CrossSceneVec3(up, basis.forward));
    basis.up = CrossSceneVec3(basis.forward, basis.right);
    const float y_scale = 1.0F / std::tan(projection.vertical_fov_radians * 0.5F);
    basis.y_scale = y_scale;
    basis.x_scale = y_scale / projection.aspect_ratio;
    basis.near_z = projection.near_z;
    *out_basis = basis;
    return true;
}

bool ProjectScenePoint(
    const SceneCameraBasis &basis,
    SceneVec3 world_point,
    std::uint16_t image_width,
    std::uint16_t image_height,
    SceneProjectedVertex *out_vertex) {
    if (out_vertex == nullptr) {
        return false;
    }

    const SceneVec3 relative = SubtractSceneVec3(world_point, basis.position);
    const float view_z = DotSceneVec3(relative, basis.forward);
    if (view_z <= basis.near_z) {
        return false;
    }

    const float view_x = DotSceneVec3(relative, basis.right);
    const float view_y = DotSceneVec3(relative, basis.up);
    const float ndc_x = (view_x * basis.x_scale) / view_z;
    const float ndc_y = (view_y * basis.y_scale) / view_z;
    SceneProjectedVertex vertex{};
    vertex.x = (ndc_x * 0.5F + 0.5F) * static_cast<float>(image_width);
    vertex.y = (0.5F - ndc_y * 0.5F) * static_cast<float>(image_height);
    vertex.depth = view_z;
    vertex.valid = true;
    *out_vertex = vertex;
    return true;
}

SemanticRgb ShadeSceneColor(SemanticRgb color, float shade, std::uint32_t bias) {
    SemanticRgb result{};
    const std::uint32_t red_value =
        static_cast<std::uint32_t>(static_cast<float>(color.r) * shade) + bias;
    const std::uint32_t green_value =
        static_cast<std::uint32_t>(static_cast<float>(color.g) * shade) + bias / 2U;
    const std::uint32_t blue_value =
        static_cast<std::uint32_t>(static_cast<float>(color.b) * shade) + bias / 3U;
    result.r = ClampByte(red_value);
    result.g = ClampByte(green_value);
    result.b = ClampByte(blue_value);
    return result;
}

SemanticRgb BuildSurfaceMaterialColor(
    const std::array<const RenderSceneThreePrimitiveMaterialTextureSlotReport *,
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> &texture_reports,
    const RenderSceneThreePrimitiveEntityReport &entity_report,
    const RenderSceneOrbitCaptureFrameReport &frame_report,
    std::size_t surface_index,
    float shade) {
    const std::size_t slot_index =
        (surface_index + static_cast<std::size_t>(entity_report.primitive_kind)) %
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    const RenderSceneThreePrimitiveMaterialTextureSlotReport &slot_report =
        *texture_reports[slot_index];
    const SemanticRgb base_color =
        BuildMaterialSlotColor(slot_report, entity_report, frame_report);
    const std::uint32_t bias = static_cast<std::uint32_t>(surface_index * 7U);
    return ShadeSceneColor(base_color, shade, bias);
}

bool AddSceneSurface(
    const SceneCameraBasis &basis,
    std::span<const SceneVec3> vertices,
    SemanticRgb color,
    std::uint16_t image_width,
    std::uint16_t image_height,
    std::array<SceneSurface, SCENE_SURFACE_CAPACITY> *out_surfaces,
    std::size_t *inout_surface_count) {
    if (out_surfaces == nullptr) {
        return false;
    }

    if (inout_surface_count == nullptr) {
        return false;
    }

    if (vertices.size() < 3U || vertices.size() > SCENE_SURFACE_VERTEX_CAPACITY) {
        return false;
    }

    if (*inout_surface_count >= out_surfaces->size()) {
        return false;
    }

    SceneSurface surface{};
    surface.vertex_count = vertices.size();
    surface.color = color;
    float depth_sum = 0.0F;
    for (std::size_t index = 0U; index < vertices.size(); ++index) {
        SceneProjectedVertex projected{};
        if (!ProjectScenePoint(
                basis,
                vertices[index],
                image_width,
                image_height,
                &projected)) {
            return true;
        }

        surface.vertices[index] = projected;
        depth_sum += projected.depth;
    }

    surface.depth = depth_sum / static_cast<float>(surface.vertex_count);
    (*out_surfaces)[*inout_surface_count] = surface;
    ++(*inout_surface_count);
    return true;
}

float EdgeFunction(
    const SceneProjectedVertex &left,
    const SceneProjectedVertex &right,
    float x,
    float y) {
    return (x - left.x) * (right.y - left.y) - (y - left.y) * (right.x - left.x);
}

bool PointInTriangle(
    const SceneProjectedVertex &first,
    const SceneProjectedVertex &second,
    const SceneProjectedVertex &third,
    float x,
    float y) {
    const float edge0 = EdgeFunction(first, second, x, y);
    const float edge1 = EdgeFunction(second, third, x, y);
    const float edge2 = EdgeFunction(third, first, x, y);
    if (edge0 >= -SCENE_RASTER_EPSILON &&
        edge1 >= -SCENE_RASTER_EPSILON &&
        edge2 >= -SCENE_RASTER_EPSILON) {
        return true;
    }

    if (edge0 <= SCENE_RASTER_EPSILON &&
        edge1 <= SCENE_RASTER_EPSILON &&
        edge2 <= SCENE_RASTER_EPSILON) {
        return true;
    }

    return false;
}

bool PointInSceneSurface(const SceneSurface &surface, float x, float y) {
    if (surface.vertex_count == 3U) {
        return PointInTriangle(
            surface.vertices[0U],
            surface.vertices[1U],
            surface.vertices[2U],
            x,
            y);
    }

    if (surface.vertex_count == 4U) {
        if (PointInTriangle(
                surface.vertices[0U],
                surface.vertices[1U],
                surface.vertices[2U],
                x,
                y)) {
            return true;
        }

        return PointInTriangle(
            surface.vertices[0U],
            surface.vertices[2U],
            surface.vertices[3U],
            x,
            y);
    }

    return false;
}

float DistanceToLineSegment(
    const SceneProjectedVertex &left,
    const SceneProjectedVertex &right,
    float x,
    float y) {
    const float dx = right.x - left.x;
    const float dy = right.y - left.y;
    const float length_square = dx * dx + dy * dy;
    if (length_square <= SCENE_RASTER_EPSILON) {
        const float px = x - left.x;
        const float py = y - left.y;
        return std::sqrt(px * px + py * py);
    }

    const float raw_t = ((x - left.x) * dx + (y - left.y) * dy) / length_square;
    const float clamped_t = ClampUnitRange(raw_t, 0.0F, 1.0F);
    const float closest_x = left.x + dx * clamped_t;
    const float closest_y = left.y + dy * clamped_t;
    const float px = x - closest_x;
    const float py = y - closest_y;
    return std::sqrt(px * px + py * py);
}

bool PointNearSceneSurfaceEdge(const SceneSurface &surface, float x, float y) {
    for (std::size_t index = 0U; index < surface.vertex_count; ++index) {
        const std::size_t next_index = (index + 1U) % surface.vertex_count;
        const float distance =
            DistanceToLineSegment(surface.vertices[index], surface.vertices[next_index], x, y);
        if (distance <= 1.35F) {
            return true;
        }
    }

    return false;
}

bool AddQuadSurface(
    const SceneCameraBasis &basis,
    const std::array<SceneVec3, 4U> &vertices,
    SemanticRgb color,
    std::uint16_t image_width,
    std::uint16_t image_height,
    std::array<SceneSurface, SCENE_SURFACE_CAPACITY> *out_surfaces,
    std::size_t *inout_surface_count) {
    const std::span<const SceneVec3> vertex_span(vertices.data(), vertices.size());
    return AddSceneSurface(
        basis,
        vertex_span,
        color,
        image_width,
        image_height,
        out_surfaces,
        inout_surface_count);
}

bool AddTriangleSurface(
    const SceneCameraBasis &basis,
    const std::array<SceneVec3, 3U> &vertices,
    SemanticRgb color,
    std::uint16_t image_width,
    std::uint16_t image_height,
    std::array<SceneSurface, SCENE_SURFACE_CAPACITY> *out_surfaces,
    std::size_t *inout_surface_count) {
    const std::span<const SceneVec3> vertex_span(vertices.data(), vertices.size());
    return AddSceneSurface(
        basis,
        vertex_span,
        color,
        image_width,
        image_height,
        out_surfaces,
        inout_surface_count);
}

SceneVec3 TransformPrimitivePoint(
    SceneVec3 local_point,
    const RenderSceneThreePrimitiveEntityReport &entity_report) {
    return TransformScenePoint(local_point, entity_report.draw_record.transform);
}

bool AddCubeSurfaces(
    const SceneCameraBasis &basis,
    const RenderSceneThreePrimitiveEntityReport &entity_report,
    const std::array<const RenderSceneThreePrimitiveMaterialTextureSlotReport *,
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> &texture_reports,
    const RenderSceneOrbitCaptureFrameReport &frame_report,
    std::uint16_t image_width,
    std::uint16_t image_height,
    std::array<SceneSurface, SCENE_SURFACE_CAPACITY> *out_surfaces,
    std::size_t *inout_surface_count) {
    constexpr float half_size = 0.72F;
    const std::array<std::array<SceneVec3, 4U>, 6U> faces{
        std::array<SceneVec3, 4U>{
            MakeSceneVec3(-half_size, -half_size, half_size),
            MakeSceneVec3(half_size, -half_size, half_size),
            MakeSceneVec3(half_size, half_size, half_size),
            MakeSceneVec3(-half_size, half_size, half_size)},
        std::array<SceneVec3, 4U>{
            MakeSceneVec3(half_size, -half_size, -half_size),
            MakeSceneVec3(-half_size, -half_size, -half_size),
            MakeSceneVec3(-half_size, half_size, -half_size),
            MakeSceneVec3(half_size, half_size, -half_size)},
        std::array<SceneVec3, 4U>{
            MakeSceneVec3(half_size, -half_size, half_size),
            MakeSceneVec3(half_size, -half_size, -half_size),
            MakeSceneVec3(half_size, half_size, -half_size),
            MakeSceneVec3(half_size, half_size, half_size)},
        std::array<SceneVec3, 4U>{
            MakeSceneVec3(-half_size, -half_size, -half_size),
            MakeSceneVec3(-half_size, -half_size, half_size),
            MakeSceneVec3(-half_size, half_size, half_size),
            MakeSceneVec3(-half_size, half_size, -half_size)},
        std::array<SceneVec3, 4U>{
            MakeSceneVec3(-half_size, half_size, half_size),
            MakeSceneVec3(half_size, half_size, half_size),
            MakeSceneVec3(half_size, half_size, -half_size),
            MakeSceneVec3(-half_size, half_size, -half_size)},
        std::array<SceneVec3, 4U>{
            MakeSceneVec3(-half_size, -half_size, -half_size),
            MakeSceneVec3(half_size, -half_size, -half_size),
            MakeSceneVec3(half_size, -half_size, half_size),
            MakeSceneVec3(-half_size, -half_size, half_size)}};
    const std::array<float, 6U> shades{1.12F, 0.62F, 0.88F, 0.78F, 1.25F, 0.52F};
    for (std::size_t face_index = 0U; face_index < faces.size(); ++face_index) {
        std::array<SceneVec3, 4U> world_vertices{};
        for (std::size_t vertex_index = 0U; vertex_index < world_vertices.size(); ++vertex_index) {
            world_vertices[vertex_index] =
                TransformPrimitivePoint(faces[face_index][vertex_index], entity_report);
        }

        const SemanticRgb color = BuildSurfaceMaterialColor(
            texture_reports,
            entity_report,
            frame_report,
            face_index,
            shades[face_index]);
        if (!AddQuadSurface(
                basis,
                world_vertices,
                color,
                image_width,
                image_height,
                out_surfaces,
                inout_surface_count)) {
            return false;
        }
    }

    return true;
}

float SegmentAngle(std::uint32_t index, std::uint32_t segment_count) {
    const float ratio = static_cast<float>(index) / static_cast<float>(segment_count);
    return ratio * 6.28318530718F;
}

bool AddCylinderSurfaces(
    const SceneCameraBasis &basis,
    const RenderSceneThreePrimitiveEntityReport &entity_report,
    const std::array<const RenderSceneThreePrimitiveMaterialTextureSlotReport *,
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> &texture_reports,
    const RenderSceneOrbitCaptureFrameReport &frame_report,
    std::uint16_t image_width,
    std::uint16_t image_height,
    std::array<SceneSurface, SCENE_SURFACE_CAPACITY> *out_surfaces,
    std::size_t *inout_surface_count) {
    constexpr float radius = 0.58F;
    constexpr float half_height = 0.78F;
    for (std::uint32_t segment = 0U; segment < CYLINDER_SEGMENT_COUNT; ++segment) {
        const std::uint32_t next_segment = segment + 1U;
        const float angle0 = SegmentAngle(segment, CYLINDER_SEGMENT_COUNT);
        const float angle1 = SegmentAngle(next_segment, CYLINDER_SEGMENT_COUNT);
        const float x0 = std::cos(angle0) * radius;
        const float z0 = std::sin(angle0) * radius;
        const float x1 = std::cos(angle1) * radius;
        const float z1 = std::sin(angle1) * radius;
        std::array<SceneVec3, 4U> side_vertices{
            TransformPrimitivePoint(MakeSceneVec3(x0, -half_height, z0), entity_report),
            TransformPrimitivePoint(MakeSceneVec3(x1, -half_height, z1), entity_report),
            TransformPrimitivePoint(MakeSceneVec3(x1, half_height, z1), entity_report),
            TransformPrimitivePoint(MakeSceneVec3(x0, half_height, z0), entity_report)};
        const float shade = 0.64F + 0.32F * ClampUnitRange(std::cos(angle0), 0.0F, 1.0F);
        const SemanticRgb side_color = BuildSurfaceMaterialColor(
            texture_reports,
            entity_report,
            frame_report,
            static_cast<std::size_t>(segment),
            shade);
        if (!AddQuadSurface(
                basis,
                side_vertices,
                side_color,
                image_width,
                image_height,
                out_surfaces,
                inout_surface_count)) {
            return false;
        }

        const SceneVec3 top_center =
            TransformPrimitivePoint(MakeSceneVec3(0.0F, half_height, 0.0F), entity_report);
        const SceneVec3 bottom_center =
            TransformPrimitivePoint(MakeSceneVec3(0.0F, -half_height, 0.0F), entity_report);
        const std::array<SceneVec3, 3U> top_vertices{
            top_center,
            TransformPrimitivePoint(MakeSceneVec3(x0, half_height, z0), entity_report),
            TransformPrimitivePoint(MakeSceneVec3(x1, half_height, z1), entity_report)};
        const SemanticRgb top_color = BuildSurfaceMaterialColor(
            texture_reports,
            entity_report,
            frame_report,
            static_cast<std::size_t>(segment + CYLINDER_SEGMENT_COUNT),
            1.18F);
        if (!AddTriangleSurface(
                basis,
                top_vertices,
                top_color,
                image_width,
                image_height,
                out_surfaces,
                inout_surface_count)) {
            return false;
        }

        const std::array<SceneVec3, 3U> bottom_vertices{
            bottom_center,
            TransformPrimitivePoint(MakeSceneVec3(x1, -half_height, z1), entity_report),
            TransformPrimitivePoint(MakeSceneVec3(x0, -half_height, z0), entity_report)};
        const SemanticRgb bottom_color = BuildSurfaceMaterialColor(
            texture_reports,
            entity_report,
            frame_report,
            static_cast<std::size_t>(segment + CYLINDER_SEGMENT_COUNT * 2U),
            0.48F);
        if (!AddTriangleSurface(
                basis,
                bottom_vertices,
                bottom_color,
                image_width,
                image_height,
                out_surfaces,
                inout_surface_count)) {
            return false;
        }
    }

    return true;
}

bool AddConeSurfaces(
    const SceneCameraBasis &basis,
    const RenderSceneThreePrimitiveEntityReport &entity_report,
    const std::array<const RenderSceneThreePrimitiveMaterialTextureSlotReport *,
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> &texture_reports,
    const RenderSceneOrbitCaptureFrameReport &frame_report,
    std::uint16_t image_width,
    std::uint16_t image_height,
    std::array<SceneSurface, SCENE_SURFACE_CAPACITY> *out_surfaces,
    std::size_t *inout_surface_count) {
    constexpr float radius = 0.68F;
    constexpr float base_y = -0.70F;
    constexpr float apex_y = 0.90F;
    const SceneVec3 apex =
        TransformPrimitivePoint(MakeSceneVec3(0.0F, apex_y, 0.0F), entity_report);
    const SceneVec3 base_center =
        TransformPrimitivePoint(MakeSceneVec3(0.0F, base_y, 0.0F), entity_report);
    for (std::uint32_t segment = 0U; segment < CONE_SEGMENT_COUNT; ++segment) {
        const std::uint32_t next_segment = segment + 1U;
        const float angle0 = SegmentAngle(segment, CONE_SEGMENT_COUNT);
        const float angle1 = SegmentAngle(next_segment, CONE_SEGMENT_COUNT);
        const float x0 = std::cos(angle0) * radius;
        const float z0 = std::sin(angle0) * radius;
        const float x1 = std::cos(angle1) * radius;
        const float z1 = std::sin(angle1) * radius;
        const std::array<SceneVec3, 3U> side_vertices{
            apex,
            TransformPrimitivePoint(MakeSceneVec3(x0, base_y, z0), entity_report),
            TransformPrimitivePoint(MakeSceneVec3(x1, base_y, z1), entity_report)};
        const float shade = 0.58F + 0.42F * ClampUnitRange(std::sin(angle0), 0.0F, 1.0F);
        const SemanticRgb side_color = BuildSurfaceMaterialColor(
            texture_reports,
            entity_report,
            frame_report,
            static_cast<std::size_t>(segment),
            shade);
        if (!AddTriangleSurface(
                basis,
                side_vertices,
                side_color,
                image_width,
                image_height,
                out_surfaces,
                inout_surface_count)) {
            return false;
        }

        const std::array<SceneVec3, 3U> base_vertices{
            base_center,
            TransformPrimitivePoint(MakeSceneVec3(x1, base_y, z1), entity_report),
            TransformPrimitivePoint(MakeSceneVec3(x0, base_y, z0), entity_report)};
        const SemanticRgb base_color = BuildSurfaceMaterialColor(
            texture_reports,
            entity_report,
            frame_report,
            static_cast<std::size_t>(segment + CONE_SEGMENT_COUNT),
            0.48F);
        if (!AddTriangleSurface(
                basis,
                base_vertices,
                base_color,
                image_width,
                image_height,
                out_surfaces,
                inout_surface_count)) {
            return false;
        }
    }

    return true;
}

bool AddEntitySurfaces(
    const SceneCameraBasis &basis,
    const RenderSceneThreePrimitiveEntityReport &entity_report,
    const std::array<const RenderSceneThreePrimitiveMaterialTextureSlotReport *,
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> &texture_reports,
    const RenderSceneOrbitCaptureFrameReport &frame_report,
    std::uint16_t image_width,
    std::uint16_t image_height,
    std::array<SceneSurface, SCENE_SURFACE_CAPACITY> *out_surfaces,
    std::size_t *inout_surface_count) {
    if (entity_report.primitive_kind == RenderScenePrimitiveGeometryKind::Cube) {
        return AddCubeSurfaces(
            basis,
            entity_report,
            texture_reports,
            frame_report,
            image_width,
            image_height,
            out_surfaces,
            inout_surface_count);
    }

    if (entity_report.primitive_kind == RenderScenePrimitiveGeometryKind::Cylinder) {
        return AddCylinderSurfaces(
            basis,
            entity_report,
            texture_reports,
            frame_report,
            image_width,
            image_height,
            out_surfaces,
            inout_surface_count);
    }

    if (entity_report.primitive_kind == RenderScenePrimitiveGeometryKind::Cone) {
        return AddConeSurfaces(
            basis,
            entity_report,
            texture_reports,
            frame_report,
            image_width,
            image_height,
            out_surfaces,
            inout_surface_count);
    }

    return false;
}

bool BuildSceneSurfaces(
    const RenderSceneOrbitCaptureFrameReport &frame_report,
    const yuengine::rendercore::RenderCameraProjectionDesc &projection,
    const std::array<const RenderSceneThreePrimitiveEntityReport *,
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> &entity_reports,
    const std::array<const RenderSceneThreePrimitiveMaterialTextureSlotReport *,
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> &texture_reports,
    std::uint16_t image_width,
    std::uint16_t image_height,
    std::array<SceneSurface, SCENE_SURFACE_CAPACITY> *out_surfaces,
    std::size_t *out_surface_count) {
    if (out_surfaces == nullptr) {
        return false;
    }

    if (out_surface_count == nullptr) {
        return false;
    }

    SceneCameraBasis basis{};
    if (!BuildSceneCameraBasis(frame_report, projection, &basis)) {
        return false;
    }

    std::size_t surface_count = 0U;
    for (std::size_t entity_index = 0U;
        entity_index < RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
        ++entity_index) {
        if (!AddEntitySurfaces(
                basis,
                *entity_reports[entity_index],
                texture_reports,
                frame_report,
                image_width,
                image_height,
                out_surfaces,
                &surface_count)) {
            return false;
        }
    }

    *out_surface_count = surface_count;
    return surface_count > 0U;
}

bool BuildSceneBackgroundPixel(
    const RenderSceneRuntimeVisualSceneProofRequest &request,
    std::size_t frame_capture_offset,
    std::uint16_t source_width,
    std::uint16_t source_height,
    std::uint16_t image_width,
    std::uint16_t image_height,
    std::uint16_t column,
    std::uint16_t row,
    std::size_t source_byte_count_per_entity,
    SemanticRgb *out_color) {
    if (out_color == nullptr) {
        return false;
    }

    const std::size_t entity_index = 0U;
    std::size_t source_column =
        (static_cast<std::size_t>(column) * source_width) / image_width;
    if (source_column >= source_width) {
        source_column = static_cast<std::size_t>(source_width) - 1U;
    }

    std::size_t source_row =
        (static_cast<std::size_t>(row) * source_height) / image_height;
    if (source_row >= source_height) {
        source_row = static_cast<std::size_t>(source_height) - 1U;
    }

    SemanticRgb captured_color{};
    if (!ReadCapturedPixel(
            request,
            frame_capture_offset,
            entity_index,
            source_width,
            source_height,
            source_column,
            source_row,
            source_byte_count_per_entity,
            &captured_color)) {
        return false;
    }

    *out_color = BuildSemanticBackground(captured_color);
    return true;
}

SemanticRgb ResolveSceneSurfacePixelColor(const SceneSurface &surface, float x, float y) {
    if (PointNearSceneSurfaceEdge(surface, x, y)) {
        return ShadeSceneColor(surface.color, 0.32F, 0U);
    }

    return surface.color;
}

bool BuildPerspectiveScenePixel(
    const RenderSceneRuntimeVisualSceneProofRequest &request,
    const std::array<SceneSurface, SCENE_SURFACE_CAPACITY> &surfaces,
    std::size_t surface_count,
    std::size_t frame_capture_offset,
    std::uint16_t source_width,
    std::uint16_t source_height,
    std::uint16_t image_width,
    std::uint16_t image_height,
    std::uint16_t column,
    std::uint16_t row,
    std::size_t source_byte_count_per_entity,
    SemanticRgb *out_color) {
    if (out_color == nullptr) {
        return false;
    }

    if (!BuildSceneBackgroundPixel(
            request,
            frame_capture_offset,
            source_width,
            source_height,
            image_width,
            image_height,
            column,
            row,
            source_byte_count_per_entity,
            out_color)) {
        return false;
    }

    const float sample_x = static_cast<float>(column) + 0.5F;
    const float sample_y = static_cast<float>(row) + 0.5F;
    float nearest_depth = SCENE_RASTER_FAR_DEPTH;
    bool surface_found = false;
    SemanticRgb surface_color{};
    for (std::size_t surface_index = 0U; surface_index < surface_count; ++surface_index) {
        const SceneSurface &surface = surfaces[surface_index];
        if (surface.depth >= nearest_depth) {
            continue;
        }

        if (!PointInSceneSurface(surface, sample_x, sample_y)) {
            continue;
        }

        nearest_depth = surface.depth;
        surface_color = ResolveSceneSurfacePixelColor(surface, sample_x, sample_y);
        surface_found = true;
    }

    if (surface_found) {
        *out_color = surface_color;
    }

    return true;
}

bool WriteFramePpmImage(
    const RenderSceneRuntimeVisualSceneProofRequest &request,
    const RenderSceneOrbitCaptureFrameReport &frame_report,
    const yuengine::rendercore::RenderCameraProjectionDesc &projection,
    std::size_t frame_capture_offset,
    std::uint16_t source_width,
    std::uint16_t source_height,
    std::uint16_t image_width,
    std::uint16_t image_height,
    std::size_t source_byte_count_per_entity,
    RenderSceneRuntimeVisualSceneImageArtifactReport *out_report) {
    if (out_report == nullptr) {
        return false;
    }

    std::array<const RenderSceneThreePrimitiveEntityReport *,
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> entity_reports{};
    std::array<const RenderSceneThreePrimitiveMaterialTextureSlotReport *,
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> texture_reports{};
    if (!ResolveSemanticArtifactReports(frame_report, &entity_reports, &texture_reports)) {
        return false;
    }

    if (image_height > source_height) {
        return false;
    }

    std::array<SceneSurface, SCENE_SURFACE_CAPACITY> surfaces{};
    std::size_t surface_count = 0U;
    if (!BuildSceneSurfaces(
            frame_report,
            projection,
            entity_reports,
            texture_reports,
            image_width,
            image_height,
            &surfaces,
            &surface_count)) {
        return false;
    }

    if (!CreateImageArtifactParentDirectory(out_report->output_path)) {
        return false;
    }

    std::FILE *file = OpenBinaryWriteFile(out_report->output_path);
    if (file == nullptr) {
        return false;
    }

    std::size_t file_byte_count = 0U;
    if (!WritePpmHeader(file, image_width, image_height, &file_byte_count)) {
        std::fclose(file);
        return false;
    }

    for (std::uint16_t row = 0U; row < image_height; ++row) {
        for (std::uint16_t column = 0U; column < image_width; ++column) {
            SemanticRgb pixel{};
            if (!BuildPerspectiveScenePixel(
                    request,
                    surfaces,
                    surface_count,
                    frame_capture_offset,
                    source_width,
                    source_height,
                    image_width,
                    image_height,
                    column,
                    row,
                    source_byte_count_per_entity,
                    &pixel)) {
                std::fclose(file);
                return false;
            }

            if (!WritePpmByte(file, pixel.r, &file_byte_count)) {
                std::fclose(file);
                return false;
            }

            if (!WritePpmByte(file, pixel.g, &file_byte_count)) {
                std::fclose(file);
                return false;
            }

            if (!WritePpmByte(file, pixel.b, &file_byte_count)) {
                std::fclose(file);
                return false;
            }
        }
    }

    const int close_result = std::fclose(file);
    if (close_result != 0) {
        return false;
    }

    out_report->status = RenderSceneRuntimeVisualSceneImageArtifactStatus::Written;
    out_report->width = image_width;
    out_report->height = image_height;
    out_report->source_byte_count =
        source_byte_count_per_entity * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    out_report->file_byte_count = file_byte_count;
    return true;
}

bool EmitImageArtifacts(
    const RenderSceneRuntimeVisualSceneProofRequest &request,
    const yuengine::rendercore::RenderCameraProjectionDesc &projection,
    RenderSceneRuntimeVisualSceneProofResult *out_result) {
    if (out_result == nullptr) {
        return false;
    }

    if (!request.image_artifact_requested) {
        return true;
    }

    if (request.rhi_device == nullptr) {
        return false;
    }

    const RhiDeviceSnapshot snapshot = request.rhi_device->Snapshot();
    if (!snapshot.swapchain.valid) {
        return false;
    }

    if (snapshot.swapchain.color_format != RhiFormat::Rgba8Unorm) {
        return false;
    }

    const std::uint16_t source_width = snapshot.swapchain.extent.width;
    const std::uint16_t source_height = snapshot.swapchain.extent.height;
    const std::size_t source_byte_count_per_entity =
        CalculateRgba8SourceByteCount(source_width, source_height);
    if (source_byte_count_per_entity == 0U) {
        return false;
    }

    if (request.capture_byte_budget_per_entity < source_byte_count_per_entity) {
        return false;
    }

    std::uint16_t image_width = 0U;
    std::uint16_t image_height = 0U;
    if (!ResolveImageArtifactOutputExtent(
            request,
            source_width,
            source_height,
            &image_width,
            &image_height)) {
        return false;
    }

    out_result->available_image_artifact_width = image_width;
    out_result->available_image_artifact_height = image_height;

    const std::size_t frame_capture_byte_budget =
        request.capture_byte_budget_per_entity * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    for (std::uint32_t frame_index = 0U;
        frame_index < out_result->orbit_result.completed_frame_count;
        ++frame_index) {
        const std::size_t frame_capture_offset =
            static_cast<std::size_t>(frame_index) * frame_capture_byte_budget;
        if (frame_capture_offset > request.capture_output.size()) {
            return false;
        }

        if (frame_capture_byte_budget > request.capture_output.size() - frame_capture_offset) {
            return false;
        }

        RenderSceneRuntimeVisualSceneImageArtifactReport &report =
            out_result->image_artifact_reports[frame_index];
        report.status = RenderSceneRuntimeVisualSceneImageArtifactStatus::Fail;
        report.frame_index = frame_index;
        report.frame_id = out_result->orbit_result.frames[frame_index].frame_id;
        if (!BuildImageArtifactPath(request, frame_index, &report)) {
            return false;
        }

        auto frame_projection = projection;
        if (out_result->camera_tween_used) {
            frame_projection.vertical_fov_radians =
                out_result->camera_tween_frame_reports[frame_index].vertical_fov_radians;
        }

        if (!WriteFramePpmImage(
                request,
                out_result->orbit_result.frames[frame_index],
                frame_projection,
                frame_capture_offset,
                source_width,
                source_height,
                image_width,
                image_height,
                source_byte_count_per_entity,
                &report)) {
            return false;
        }

        ++out_result->image_artifact_report_count;
        out_result->image_artifact_bytes_written += report.file_byte_count;
    }

    return out_result->image_artifact_report_count == out_result->orbit_result.completed_frame_count;
}

std::array<RenderSceneThreePrimitiveEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
BuildProofEntities() {
    std::array<RenderSceneThreePrimitiveEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
        entities{};

    entities[0U].world_object_id = WorldObjectId{PROOF_WORLD_OBJECT_ID};
    entities[0U].object_name = PROOF_CUBE_NAME;
    entities[0U].object_name_byte_count = sizeof(PROOF_CUBE_NAME) - 1U;
    entities[0U].transform = BuildTransform(-2.0F, 0.0F, 0.0F);
    entities[0U].geometry = BuildGeometryRecord(RenderScenePrimitiveGeometryKind::Cube);

    entities[1U].world_object_id = WorldObjectId{PROOF_WORLD_OBJECT_ID + 1U};
    entities[1U].object_name = PROOF_CYLINDER_NAME;
    entities[1U].object_name_byte_count = sizeof(PROOF_CYLINDER_NAME) - 1U;
    entities[1U].transform = BuildTransform(0.0F, 1.0F, 0.0F);
    entities[1U].geometry = BuildGeometryRecord(RenderScenePrimitiveGeometryKind::Cylinder);

    entities[2U].world_object_id = WorldObjectId{PROOF_WORLD_OBJECT_ID + 2U};
    entities[2U].object_name = PROOF_CONE_NAME;
    entities[2U].object_name_byte_count = sizeof(PROOF_CONE_NAME) - 1U;
    entities[2U].transform = BuildTransform(2.0F, 0.0F, 1.0F);
    entities[2U].geometry = BuildGeometryRecord(RenderScenePrimitiveGeometryKind::Cone);

    return entities;
}

bool CopyObjectName(
    const RenderSceneThreePrimitiveEntityRequest &entity,
    RenderSceneRuntimeVisualSceneProofEntityReport *out_report) {
    if (out_report == nullptr) {
        return false;
    }

    if (entity.object_name == nullptr) {
        return false;
    }

    if (entity.object_name_byte_count >= MAX_RENDER_SCENE_RUNTIME_VISUAL_SCENE_OBJECT_NAME_BYTES) {
        return false;
    }

    for (std::size_t index = 0U; index < entity.object_name_byte_count; ++index) {
        out_report->object_name[index] = entity.object_name[index];
    }

    out_report->object_name[entity.object_name_byte_count] = '\0';
    out_report->object_name_byte_count = entity.object_name_byte_count;
    return true;
}

bool FillBaseEntityReports(
    const std::array<RenderSceneThreePrimitiveEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
        &entities,
    RenderSceneRuntimeVisualSceneProofResult *out_result) {
    if (out_result == nullptr) {
        return false;
    }

    for (std::size_t index = 0U; index < entities.size(); ++index) {
        const RenderSceneThreePrimitiveEntityRequest &entity = entities[index];
        RenderSceneRuntimeVisualSceneProofEntityReport &report = out_result->entity_reports[index];
        report.world_object_id = entity.world_object_id;
        report.primitive_kind = entity.geometry.kind;
        report.base_transform = entity.transform;
        report.animated_transform = entity.transform;
        if (!CopyObjectName(entity, &report)) {
            return false;
        }
    }

    out_result->entity_report_count = entities.size();
    return true;
}

RuntimeFrameContext BuildAnimationFrameContext() {
    RuntimeFrameContext context{};
    context.frame_index = 0U;
    context.delta_time_nanoseconds = 16666667ULL;
    context.fixed_time_nanoseconds = PROOF_ANIMATION_SAMPLE_NANOSECONDS;
    context.frame_mode = RuntimeFrameMode::Fixed;
    return context;
}

AnimationRuntimeClipRecord BuildAnimationClip() {
    AnimationRuntimeClipRecord clip{};
    clip.clip_id = PROOF_ANIMATION_CLIP_ID;
    clip.duration_seconds = 1.0F;
    clip.first_track_index = 0U;
    clip.track_count = RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    clip.layer_count = 1U;
    clip.is_valid = true;
    return clip;
}

AnimationRuntimeTrackRecord BuildAnimationTrack(
    std::uint32_t index,
    WorldObjectId target,
    AnimationRuntimeChannel channel,
    AnimationRuntimeInterpolation interpolation) {
    AnimationRuntimeTrackRecord track{};
    track.track_id = PROOF_ANIMATION_TRACK_ID + index;
    track.target = target;
    track.channel = channel;
    track.interpolation = interpolation;
    track.first_keyframe_index = static_cast<std::size_t>(index) * 2U;
    track.keyframe_count = 2U;
    track.is_valid = true;
    return track;
}

AnimationRuntimeKeyframeRecord BuildAnimationKeyframe(float time_seconds, float value) {
    AnimationRuntimeKeyframeRecord keyframe{};
    keyframe.time_seconds = time_seconds;
    keyframe.value = value;
    keyframe.is_valid = true;
    return keyframe;
}

AnimationRuntimeSampleRequest BuildAnimationSampleRequest(
    std::span<const AnimationRuntimeClipRecord> clips,
    std::span<const AnimationRuntimeTrackRecord> tracks,
    std::span<const AnimationRuntimeKeyframeRecord> keyframes) {
    AnimationRuntimeSampleRequest request{};
    request.clip_id = PROOF_ANIMATION_CLIP_ID;
    request.clips = clips;
    request.tracks = tracks;
    request.keyframes = keyframes;
    request.frame_context = BuildAnimationFrameContext();
    request.clip_start_time_nanoseconds = PROOF_ANIMATION_CLIP_START_NANOSECONDS;
    return request;
}

bool RegisterAnimationTargets(
    WorldInstance &world,
    WorldTransformBridge &bridge,
    const std::array<RenderSceneThreePrimitiveEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
        &entities) {
    for (const RenderSceneThreePrimitiveEntityRequest &entity : entities) {
        const WorldRegistrationResult registration =
            world.RegisterObject(WorldObjectDesc{entity.world_object_id, true});
        if (!registration.Succeeded()) {
            return false;
        }

        const WorldTransformResult transform_result =
            bridge.Register(entity.world_object_id, entity.transform);
        if (transform_result.status != WorldTransformStatus::Success) {
            return false;
        }
    }

    return true;
}

bool ApplyAnimatedTransformsToEntities(
    WorldTransformBridge &bridge,
    std::array<RenderSceneThreePrimitiveEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
        *out_entities) {
    if (out_entities == nullptr) {
        return false;
    }

    for (RenderSceneThreePrimitiveEntityRequest &entity : *out_entities) {
        const WorldTransformResult transform_result = bridge.Query(entity.world_object_id);
        if (transform_result.status != WorldTransformStatus::Success) {
            return false;
        }

        entity.transform = transform_result.transform_state;
    }

    return true;
}

void FillAnimatedEntityReport(
    std::size_t index,
    const AnimationRuntimeSampleResult &sample_result,
    const AnimationRuntimeTrackRecord &track,
    const AnimationRuntimeSampledValue &sampled_value,
    const RenderSceneThreePrimitiveEntityRequest &entity,
    RenderSceneRuntimeVisualSceneProofResult *out_result) {
    if (out_result == nullptr) {
        return;
    }

    RenderSceneRuntimeVisualSceneProofEntityReport &report = out_result->entity_reports[index];
    report.animation_clip_id = sample_result.clip_id;
    report.animation_track_id = track.track_id;
    report.sampled_value = sampled_value.value;
    report.animated_transform = entity.transform;
    report.animation_sampled = true;
    report.transform_applied = true;
}

bool ApplyRuntimeAnimationToEntities(
    std::array<RenderSceneThreePrimitiveEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
        *out_entities,
    RenderSceneRuntimeVisualSceneProofResult *out_result,
    RenderSceneMissingLayerDiagnosticFault *out_fault) {
    if (out_entities == nullptr) {
        return false;
    }

    if (out_result == nullptr) {
        return false;
    }

    if (out_fault == nullptr) {
        return false;
    }

    *out_fault = RenderSceneMissingLayerDiagnosticFault::MissingTransformApply;
    WorldInstance world;
    WorldTransformBridgeDesc bridge_desc{};
    bridge_desc.bridge_capacity = static_cast<std::uint32_t>(RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT);
    WorldTransformBridge bridge(world, bridge_desc);
    if (!RegisterAnimationTargets(world, bridge, *out_entities)) {
        return false;
    }

    const std::array<AnimationRuntimeClipRecord, 1U> clips{BuildAnimationClip()};
    const WorldObjectId cube_id = (*out_entities)[0U].world_object_id;
    const WorldObjectId cylinder_id = (*out_entities)[1U].world_object_id;
    const WorldObjectId cone_id = (*out_entities)[2U].world_object_id;
    const std::array<AnimationRuntimeTrackRecord, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> tracks{
        BuildAnimationTrack(0U, cube_id, AnimationRuntimeChannel::RotationY,
            AnimationRuntimeInterpolation::Linear),
        BuildAnimationTrack(1U, cylinder_id, AnimationRuntimeChannel::RotationZ,
            AnimationRuntimeInterpolation::Step),
        BuildAnimationTrack(2U, cone_id, AnimationRuntimeChannel::RotationX,
            AnimationRuntimeInterpolation::Linear)};
    const std::array<AnimationRuntimeKeyframeRecord, 6U> keyframes{
        BuildAnimationKeyframe(0.0F, 0.0F),
        BuildAnimationKeyframe(1.0F, 0.8F),
        BuildAnimationKeyframe(0.0F, 0.25F),
        BuildAnimationKeyframe(1.0F, 0.75F),
        BuildAnimationKeyframe(0.0F, 0.0F),
        BuildAnimationKeyframe(1.0F, 1.0F)};

    std::array<AnimationRuntimeSampledValue, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> sampled_values{};
    AnimationRuntimeSampleResult sample_result{};
    AnimationRuntimeSampler sampler;
    const AnimationRuntimeSampleRequest sample_request =
        BuildAnimationSampleRequest(clips, tracks, keyframes);
    const AnimationRuntimeStatus sample_status =
        sampler.Sample(sample_request, sampled_values, &sample_result);
    if (sample_status != AnimationRuntimeStatus::Success) {
        *out_fault = RenderSceneMissingLayerDiagnosticFault::MissingAnimationInterpolation;
        return false;
    }

    AnimationRuntimeTransformApplyResult apply_result{};
    const AnimationRuntimeTransformApplyRequest apply_request{&bridge, sampled_values};
    const AnimationRuntimeStatus apply_status =
        sampler.ApplySampledTransform(apply_request, &apply_result);
    if (apply_status != AnimationRuntimeStatus::Success) {
        *out_fault = RenderSceneMissingLayerDiagnosticFault::MissingTransformApply;
        return false;
    }

    if (apply_result.applied_value_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        *out_fault = RenderSceneMissingLayerDiagnosticFault::MissingTransformApply;
        return false;
    }

    if (apply_result.updated_object_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        *out_fault = RenderSceneMissingLayerDiagnosticFault::MissingTransformApply;
        return false;
    }

    if (!ApplyAnimatedTransformsToEntities(bridge, out_entities)) {
        *out_fault = RenderSceneMissingLayerDiagnosticFault::MissingTransformApply;
        return false;
    }

    for (std::size_t index = 0U; index < out_entities->size(); ++index) {
        FillAnimatedEntityReport(
            index,
            sample_result,
            tracks[index],
            sampled_values[index],
            (*out_entities)[index],
            out_result);
    }

    *out_fault = RenderSceneMissingLayerDiagnosticFault::None;
    return true;
}

bool FillRenderSceneConsumedReports(RenderSceneRuntimeVisualSceneProofResult *out_result) {
    if (out_result == nullptr) {
        return false;
    }

    if (out_result->orbit_result.completed_frame_count == 0U) {
        return false;
    }

    const RenderSceneOrbitCaptureFrameReport &first_frame = out_result->orbit_result.frames[0U];
    if (first_frame.capture_result.entity_report_count != RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return false;
    }

    for (std::size_t index = 0U; index < RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT; ++index) {
        const RenderSceneThreePrimitiveEntityReport &source_report =
            first_frame.capture_result.entity_reports[index];
        RenderSceneRuntimeVisualSceneProofEntityReport &target_report =
            out_result->entity_reports[index];
        target_report.render_scene_consumed_transform = source_report.draw_record.transform;
        target_report.render_scene_submitted = source_report.submitted;
    }

    out_result->material_texture_slot_report_count =
        first_frame.capture_result.material_texture_slot_report_count;
    return true;
}

bool IsFiniteFloat(float value) {
    return std::isfinite(value);
}

bool IsFiniteCameraVector(RenderCameraVector3 value) {
    if (!IsFiniteFloat(value.x)) {
        return false;
    }

    if (!IsFiniteFloat(value.y)) {
        return false;
    }

    return IsFiniteFloat(value.z);
}

bool IsFiniteCameraPose(const RenderCameraPose &pose) {
    if (!IsFiniteCameraVector(pose.position)) {
        return false;
    }

    if (!IsFiniteCameraVector(pose.target)) {
        return false;
    }

    return IsFiniteCameraVector(pose.up);
}

bool IsCameraTweenEaseUsable(RenderSceneRuntimeVisualSceneCameraTweenEase ease) {
    if (ease == RenderSceneRuntimeVisualSceneCameraTweenEase::Linear) {
        return true;
    }

    return ease == RenderSceneRuntimeVisualSceneCameraTweenEase::SmoothStep;
}

bool IsCameraTweenKeyframeUsable(
    const RenderSceneRuntimeVisualSceneCameraTweenKeyframe &keyframe) {
    if (!IsFiniteFloat(keyframe.time_seconds)) {
        return false;
    }

    if (!IsFiniteCameraPose(keyframe.pose)) {
        return false;
    }

    if (!IsFiniteFloat(keyframe.vertical_fov_radians)) {
        return false;
    }

    if (keyframe.vertical_fov_radians <= SCENE_RASTER_EPSILON) {
        return false;
    }

    return IsCameraTweenEaseUsable(keyframe.ease);
}

bool IsCameraTweenRequestUsable(const RenderSceneRuntimeVisualSceneProofRequest &request) {
    if (!request.camera_tween_requested) {
        return true;
    }

    const std::span<const RenderSceneRuntimeVisualSceneCameraTweenKeyframe> keyframes =
        request.camera_tween_keyframes;
    if (keyframes.size() < 2U) {
        return false;
    }

    if (keyframes.size() > MAX_RENDER_SCENE_RUNTIME_VISUAL_SCENE_CAMERA_TWEEN_KEYFRAME_COUNT) {
        return false;
    }

    float previous_time = keyframes[0U].time_seconds;
    if (!IsCameraTweenKeyframeUsable(keyframes[0U])) {
        return false;
    }

    for (std::size_t index = 1U; index < keyframes.size(); ++index) {
        const RenderSceneRuntimeVisualSceneCameraTweenKeyframe &keyframe = keyframes[index];
        if (!IsCameraTweenKeyframeUsable(keyframe)) {
            return false;
        }

        if (keyframe.time_seconds <= previous_time) {
            return false;
        }

        previous_time = keyframe.time_seconds;
    }

    return true;
}

float EvaluateCameraTweenEase(
    RenderSceneRuntimeVisualSceneCameraTweenEase ease,
    float linear_t) {
    const float clamped_t = ClampUnitRange(linear_t, 0.0F, 1.0F);
    if (ease == RenderSceneRuntimeVisualSceneCameraTweenEase::SmoothStep) {
        return clamped_t * clamped_t * (3.0F - 2.0F * clamped_t);
    }

    return clamped_t;
}

float LerpFloat(float first, float second, float t) {
    return first + (second - first) * t;
}

RenderCameraVector3 LerpCameraVector(
    RenderCameraVector3 first,
    RenderCameraVector3 second,
    float t) {
    RenderCameraVector3 result{};
    result.x = LerpFloat(first.x, second.x, t);
    result.y = LerpFloat(first.y, second.y, t);
    result.z = LerpFloat(first.z, second.z, t);
    return result;
}

RenderCameraPose LerpCameraPose(
    const RenderCameraPose &first,
    const RenderCameraPose &second,
    float t) {
    RenderCameraPose result{};
    result.position = LerpCameraVector(first.position, second.position, t);
    result.target = LerpCameraVector(first.target, second.target, t);
    result.up = LerpCameraVector(first.up, second.up, t);
    return result;
}

bool FindCameraTweenSegment(
    std::span<const RenderSceneRuntimeVisualSceneCameraTweenKeyframe> keyframes,
    float sample_time_seconds,
    std::size_t *out_source_index,
    std::size_t *out_target_index) {
    if (out_source_index == nullptr) {
        return false;
    }

    if (out_target_index == nullptr) {
        return false;
    }

    if (keyframes.size() < 2U) {
        return false;
    }

    const std::size_t last_index = keyframes.size() - 1U;
    if (sample_time_seconds <= keyframes[0U].time_seconds) {
        *out_source_index = 0U;
        *out_target_index = 1U;
        return true;
    }

    if (sample_time_seconds >= keyframes[last_index].time_seconds) {
        *out_source_index = last_index - 1U;
        *out_target_index = last_index;
        return true;
    }

    for (std::size_t index = 0U; index < last_index; ++index) {
        const float source_time = keyframes[index].time_seconds;
        const float target_time = keyframes[index + 1U].time_seconds;
        if (sample_time_seconds >= source_time && sample_time_seconds <= target_time) {
            *out_source_index = index;
            *out_target_index = index + 1U;
            return true;
        }
    }

    return false;
}

bool SampleCameraTweenFrame(
    const RenderSceneRuntimeVisualSceneProofRequest &request,
    std::uint32_t frame_index,
    RenderSceneRuntimeVisualSceneCameraTweenFrameReport *out_report) {
    if (out_report == nullptr) {
        return false;
    }

    if (!IsCameraTweenRequestUsable(request)) {
        return false;
    }

    const std::span<const RenderSceneRuntimeVisualSceneCameraTweenKeyframe> keyframes =
        request.camera_tween_keyframes;
    const std::size_t last_keyframe_index = keyframes.size() - 1U;
    const float start_time = keyframes[0U].time_seconds;
    const float end_time = keyframes[last_keyframe_index].time_seconds;
    const float frame_ratio =
        static_cast<float>(frame_index) / static_cast<float>(request.frame_count - 1U);
    const float sample_time_seconds = LerpFloat(start_time, end_time, frame_ratio);

    std::size_t source_index = 0U;
    std::size_t target_index = 0U;
    if (!FindCameraTweenSegment(
            keyframes,
            sample_time_seconds,
            &source_index,
            &target_index)) {
        return false;
    }

    const RenderSceneRuntimeVisualSceneCameraTweenKeyframe &source_keyframe =
        keyframes[source_index];
    const RenderSceneRuntimeVisualSceneCameraTweenKeyframe &target_keyframe =
        keyframes[target_index];
    const float segment_duration =
        target_keyframe.time_seconds - source_keyframe.time_seconds;
    if (segment_duration <= SCENE_RASTER_EPSILON) {
        return false;
    }

    const float linear_t =
        (sample_time_seconds - source_keyframe.time_seconds) / segment_duration;
    const float eased_t = EvaluateCameraTweenEase(source_keyframe.ease, linear_t);

    RenderSceneRuntimeVisualSceneCameraTweenFrameReport report{};
    report.frame_index = frame_index;
    report.frame_id = request.first_frame_id + frame_index;
    report.source_keyframe_index = source_index;
    report.target_keyframe_index = target_index;
    report.sample_time_seconds = sample_time_seconds;
    report.linear_t = ClampUnitRange(linear_t, 0.0F, 1.0F);
    report.eased_t = eased_t;
    report.vertical_fov_radians =
        LerpFloat(source_keyframe.vertical_fov_radians, target_keyframe.vertical_fov_radians, eased_t);
    report.camera_pose = LerpCameraPose(source_keyframe.pose, target_keyframe.pose, eased_t);
    *out_report = report;
    return true;
}

RenderSceneOrbitCaptureMissingLayer MapCameraTweenCaptureMissingLayer(
    RenderSceneThreePrimitiveCaptureMissingLayer layer) {
    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::None) {
        return RenderSceneOrbitCaptureMissingLayer::None;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::Camera) {
        return RenderSceneOrbitCaptureMissingLayer::Camera;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::GeometryModel) {
        return RenderSceneOrbitCaptureMissingLayer::GeometryModel;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::MaterialTextureSlots) {
        return RenderSceneOrbitCaptureMissingLayer::MaterialTextureSlots;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::ShaderPipeline) {
        return RenderSceneOrbitCaptureMissingLayer::ShaderPipeline;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::ScenePlacement) {
        return RenderSceneOrbitCaptureMissingLayer::ScenePlacement;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::RenderSceneSubmission) {
        return RenderSceneOrbitCaptureMissingLayer::RenderSceneSubmission;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::RenderCoreRhiDrawCapture) {
        return RenderSceneOrbitCaptureMissingLayer::RenderCoreRhiDrawCapture;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::RhiCaptureTarget) {
        return RenderSceneOrbitCaptureMissingLayer::RhiCaptureTarget;
    }

    if (layer == RenderSceneThreePrimitiveCaptureMissingLayer::OutputPath) {
        return RenderSceneOrbitCaptureMissingLayer::OutputPath;
    }

    return RenderSceneOrbitCaptureMissingLayer::RenderCoreRhiDrawCapture;
}

RenderSceneOrbitCaptureStatus MapCameraTweenCaptureFrameStatus(
    const RenderSceneThreePrimitiveCaptureResult &capture_result,
    RenderSceneOrbitCaptureFrameReport *out_report) {
    if (out_report == nullptr) {
        return RenderSceneOrbitCaptureStatus::InvalidArgument;
    }

    out_report->first_missing_layer =
        MapCameraTweenCaptureMissingLayer(capture_result.first_missing_layer);
    if (capture_result.status == RenderSceneThreePrimitiveCaptureStatus::Success) {
        out_report->status = RenderSceneOrbitCaptureStatus::Success;
        out_report->first_missing_layer = RenderSceneOrbitCaptureMissingLayer::None;
        return out_report->status;
    }

    if (capture_result.status == RenderSceneThreePrimitiveCaptureStatus::BlockedByEnv) {
        out_report->status = RenderSceneOrbitCaptureStatus::BlockedByEnv;
        return out_report->status;
    }

    if (capture_result.status == RenderSceneThreePrimitiveCaptureStatus::InvalidArgument) {
        out_report->status = RenderSceneOrbitCaptureStatus::InvalidArgument;
        if (out_report->first_missing_layer == RenderSceneOrbitCaptureMissingLayer::None) {
            out_report->first_missing_layer = RenderSceneOrbitCaptureMissingLayer::CaptureStorage;
        }

        return out_report->status;
    }

    out_report->status = RenderSceneOrbitCaptureStatus::Fail;
    return out_report->status;
}

RenderSceneOrbitCaptureStatus ExecuteCameraTweenCapture(
    const RenderSceneRuntimeVisualSceneProofRequest &request,
    const RenderSceneRuntimeCameraRecord &proof_camera,
    const RenderSceneRuntimeMaterialRecord &material,
    std::span<const RenderSceneThreePrimitiveEntityRequest> entities,
    RenderSceneRuntimeVisualSceneProofResult *out_result) {
    if (out_result == nullptr) {
        return RenderSceneOrbitCaptureStatus::InvalidArgument;
    }

    if (!IsCameraTweenRequestUsable(request)) {
        out_result->orbit_result.status = RenderSceneOrbitCaptureStatus::InvalidArgument;
        out_result->orbit_result.first_missing_layer =
            RenderSceneOrbitCaptureMissingLayer::AnimationInterpolation;
        return out_result->orbit_result.status;
    }

    RenderSceneOrbitCaptureResult &result = out_result->orbit_result;
    result.first_frame_id = request.first_frame_id;
    result.requested_frame_count = request.frame_count;
    result.target = request.camera_tween_keyframes[0U].pose.target;
    result.orbit_radius = 0.0F;
    result.orbit_height = 0.0F;
    result.frame_capture_byte_budget =
        request.capture_byte_budget_per_entity * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    out_result->camera_tween_used = true;
    out_result->camera_tween_keyframe_count = request.camera_tween_keyframes.size();

    RenderSceneCameraFrameBinder camera_binder;
    RenderSceneThreePrimitiveCaptureRoute capture_route;
    for (std::uint32_t frame_index = 0U; frame_index < request.frame_count; ++frame_index) {
        RenderSceneRuntimeVisualSceneCameraTweenFrameReport tween_report{};
        if (!SampleCameraTweenFrame(request, frame_index, &tween_report)) {
            result.status = RenderSceneOrbitCaptureStatus::InvalidArgument;
            result.first_missing_layer = RenderSceneOrbitCaptureMissingLayer::AnimationInterpolation;
            return result.status;
        }

        out_result->camera_tween_frame_reports[frame_index] = tween_report;

        RenderSceneOrbitCaptureFrameReport &frame_report = result.frames[frame_index];
        frame_report.frame_index = frame_index;
        frame_report.frame_id = tween_report.frame_id;
        frame_report.orbit_angle_radians = 0.0F;
        frame_report.target = tween_report.camera_pose.target;
        frame_report.camera_pose = tween_report.camera_pose;
        if (!BuildCaptureFrameOutputPath(request, frame_index, &frame_report)) {
            result.status = RenderSceneOrbitCaptureStatus::Fail;
            result.first_missing_layer = RenderSceneOrbitCaptureMissingLayer::OutputPath;
            return result.status;
        }

        RenderSceneRuntimeCameraRecord camera = proof_camera;
        camera.pose = tween_report.camera_pose;
        camera.projection.vertical_fov_radians = tween_report.vertical_fov_radians;
        const std::array<RenderSceneRuntimeCameraRecord, 1U> cameras{camera};

        RenderSceneCameraBindingRequest camera_request{};
        camera_request.frame_id = frame_report.frame_id;
        camera_request.active_camera_id = camera.camera_id;
        camera_request.cameras = cameras;
        camera_request.capture_byte_budget = result.frame_capture_byte_budget;
        camera_request.capture_requested = true;

        RenderSceneCameraBindingResult camera_result{};
        const RenderSceneStatus camera_status =
            camera_binder.BuildActiveCameraFrame(camera_request, &camera_result);
        frame_report.capture = camera_result.capture;
        if (camera_status != RenderSceneStatus::Success) {
            frame_report.status = RenderSceneOrbitCaptureStatus::Fail;
            frame_report.first_missing_layer = RenderSceneOrbitCaptureMissingLayer::Camera;
            result.status = frame_report.status;
            result.first_missing_layer = frame_report.first_missing_layer;
            return result.status;
        }

        const std::size_t frame_capture_offset =
            static_cast<std::size_t>(frame_index) * result.frame_capture_byte_budget;
        const std::span<std::uint8_t> frame_capture_output =
            request.capture_output.subspan(frame_capture_offset, result.frame_capture_byte_budget);

        RenderSceneThreePrimitiveCaptureRequest capture_request{};
        capture_request.frame_id = frame_report.frame_id;
        capture_request.camera = camera_result;
        capture_request.material = material;
        capture_request.entities = entities;
        capture_request.rhi_device = request.rhi_device;
        capture_request.output_path = frame_report.output_path;
        capture_request.output_path_byte_count = frame_report.output_path_byte_count;
        capture_request.capture_output = frame_capture_output;
        capture_request.capture_byte_budget_per_entity = request.capture_byte_budget_per_entity;

        capture_route.Execute(capture_request, &frame_report.capture_result);
        frame_report.capture = frame_report.capture_result.capture;
        frame_report.output_status = frame_report.capture_result.output_status;
        frame_report.capture_bytes_written = frame_report.capture_result.capture_bytes_written;
        const RenderSceneOrbitCaptureStatus frame_status =
            MapCameraTweenCaptureFrameStatus(frame_report.capture_result, &frame_report);
        if (frame_status != RenderSceneOrbitCaptureStatus::Success) {
            result.status = frame_status;
            result.first_missing_layer = frame_report.first_missing_layer;
            return result.status;
        }

        ++result.completed_frame_count;
        result.capture_bytes_written += frame_report.capture_bytes_written;
    }

    result.status = RenderSceneOrbitCaptureStatus::Success;
    result.first_missing_layer = RenderSceneOrbitCaptureMissingLayer::None;
    return result.status;
}

bool IsRequestStorageUsable(const RenderSceneRuntimeVisualSceneProofRequest &request) {
    if (!IsCaptureOutputPathPrefixUsable(request)) {
        return false;
    }

    if (request.capture_output.data() == nullptr) {
        return false;
    }

    return request.capture_byte_budget_per_entity > 0U;
}
}

RenderSceneRuntimeVisualSceneProofStatus RenderSceneRuntimeVisualSceneProofRoute::Execute(
    const RenderSceneRuntimeVisualSceneProofRequest &request,
    RenderSceneRuntimeVisualSceneProofResult *out_result) const {
    if (out_result == nullptr) {
        return RenderSceneRuntimeVisualSceneProofStatus::InvalidArgument;
    }

    RenderSceneRuntimeVisualSceneProofResult result{};
    result.requested_frame_count = request.frame_count;
    result.requested_target_image_artifact_width = request.target_image_artifact_width;
    result.requested_target_image_artifact_height = request.target_image_artifact_height;
    result.requested_target_capture_width = request.target_capture_width;
    result.requested_target_capture_height = request.target_capture_height;
    result.requested_minimum_image_artifact_width = request.minimum_image_artifact_width;
    result.requested_minimum_image_artifact_height = request.minimum_image_artifact_height;
    result.close_orbit_loop = request.close_orbit_loop;
    *out_result = result;

    if (request.diagnostic_fault != RenderSceneMissingLayerDiagnosticFault::None) {
        return CompleteWithDiagnostic(request.diagnostic_fault, out_result);
    }

    if (request.first_frame_id == 0U) {
        out_result->status = RenderSceneRuntimeVisualSceneProofStatus::InvalidArgument;
        return out_result->status;
    }

    if (request.frame_count <= 1U ||
        request.frame_count > MAX_RENDER_SCENE_ORBIT_CAPTURE_FRAME_COUNT) {
        return CompleteWithDiagnostic(
            RenderSceneMissingLayerDiagnosticFault::MissingOutputBounding,
            out_result);
    }

    if (!IsRequestStorageUsable(request)) {
        return CompleteWithDiagnostic(
            RenderSceneMissingLayerDiagnosticFault::MissingOutputBounding,
            out_result);
    }

    if (!IsCameraTweenRequestUsable(request)) {
        return CompleteWithDiagnostic(
            RenderSceneMissingLayerDiagnosticFault::MissingCameraTweenSampling,
            out_result);
    }

    if (!IsImageArtifactRequestUsable(request)) {
        return CompleteWithDiagnostic(
            RenderSceneMissingLayerDiagnosticFault::MissingCaptureOutputImage,
            out_result);
    }

    if (!request.target_capture_environment_available) {
        return CompleteWithDiagnostic(
            RenderSceneMissingLayerDiagnosticFault::MissingRhiCaptureTarget,
            out_result);
    }

    if (request.rhi_device == nullptr) {
        return CompleteWithDiagnostic(
            RenderSceneMissingLayerDiagnosticFault::MissingRhiCaptureTarget,
            out_result);
    }

    const RenderSceneMissingLayerDiagnosticFault capture_target_fault =
        ConfigureRequestedCaptureTarget(request);
    if (capture_target_fault != RenderSceneMissingLayerDiagnosticFault::None) {
        return CompleteWithDiagnostic(capture_target_fault, out_result);
    }

    const RenderSceneMissingLayerDiagnosticFault image_resolution_fault =
        AssessImageArtifactResolutionRequest(request, out_result);
    if (image_resolution_fault != RenderSceneMissingLayerDiagnosticFault::None) {
        return CompleteWithDiagnostic(image_resolution_fault, out_result);
    }

    std::array<RenderSceneThreePrimitiveEntityRequest, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
        entities = BuildProofEntities();
    if (!FillBaseEntityReports(entities, out_result)) {
        return CompleteWithDiagnostic(
            RenderSceneMissingLayerDiagnosticFault::MissingScenePlacement,
            out_result);
    }

    RenderSceneMissingLayerDiagnosticFault animation_fault =
        RenderSceneMissingLayerDiagnosticFault::None;
    if (!ApplyRuntimeAnimationToEntities(&entities, out_result, &animation_fault)) {
        return CompleteWithDiagnostic(animation_fault, out_result);
    }

    const RenderSceneRuntimeCameraRecord proof_camera = BuildCameraRecord();
    out_result->camera_projection_kind = proof_camera.projection.kind;
    out_result->camera_vertical_fov_radians = proof_camera.projection.vertical_fov_radians;
    out_result->camera_aspect_ratio = proof_camera.projection.aspect_ratio;
    out_result->camera_orthographic_height = proof_camera.projection.orthographic_height;
    out_result->camera_perspective_projection_used =
        proof_camera.projection.kind == RenderCameraProjectionKind::Perspective;

    const RenderSceneRuntimeMaterialRecord proof_material = BuildRuntimeMaterialRecord();
    RenderSceneOrbitCaptureStatus orbit_status = RenderSceneOrbitCaptureStatus::InvalidArgument;
    if (request.camera_tween_requested) {
        orbit_status = ExecuteCameraTweenCapture(
            request,
            proof_camera,
            proof_material,
            std::span<const RenderSceneThreePrimitiveEntityRequest>(entities.data(), entities.size()),
            out_result);
    }

    if (!request.camera_tween_requested) {
        RenderSceneOrbitCaptureRequest orbit_request{};
        orbit_request.first_frame_id = request.first_frame_id;
        orbit_request.frame_count = request.frame_count;
        orbit_request.camera_template = proof_camera;
        orbit_request.material = proof_material;
        orbit_request.entities = entities;
        orbit_request.target = RenderCameraVector3{0.0F, 0.0F, 0.0F};
        orbit_request.orbit_radius = PROOF_ORBIT_RADIUS;
        orbit_request.orbit_height = PROOF_ORBIT_HEIGHT;
        orbit_request.close_orbit_loop = request.close_orbit_loop;
        orbit_request.rhi_device = request.rhi_device;
        orbit_request.output_path_prefix = request.output_path_prefix;
        orbit_request.output_path_prefix_byte_count = request.output_path_prefix_byte_count;
        orbit_request.capture_output = request.capture_output;
        orbit_request.capture_byte_budget_per_entity = request.capture_byte_budget_per_entity;

        RenderSceneOrbitCaptureRoute orbit_route;
        orbit_status = orbit_route.Execute(orbit_request, &out_result->orbit_result);
    }

    out_result->completed_frame_count = out_result->orbit_result.completed_frame_count;
    out_result->capture_bytes_written = out_result->orbit_result.capture_bytes_written;
    out_result->frame_capture_byte_budget = out_result->orbit_result.frame_capture_byte_budget;

    if (orbit_status != RenderSceneOrbitCaptureStatus::Success) {
        RenderSceneMissingLayerDiagnosticFault fault =
            MapOrbitMissingLayer(out_result->orbit_result.first_missing_layer);
        if (request.camera_tween_requested &&
            out_result->orbit_result.first_missing_layer ==
                RenderSceneOrbitCaptureMissingLayer::AnimationInterpolation) {
            fault = RenderSceneMissingLayerDiagnosticFault::MissingCameraTweenSampling;
        }

        return CompleteWithDiagnostic(fault, out_result);
    }

    if (request.camera_tween_requested) {
        out_result->camera_vertical_fov_radians =
            out_result->camera_tween_frame_reports[0U].vertical_fov_radians;
    }

    if (request.transparent_panel_blend_requested &&
        !ExecuteTransparentPanelBlendProof(request, proof_camera, out_result)) {
        return CompleteWithDiagnostic(
            RenderSceneMissingLayerDiagnosticFault::MissingShaderPipeline,
            out_result);
    }

    if (request.material_proof_requested && !ExecuteMaterialProof(out_result)) {
        return CompleteWithDiagnostic(
            RenderSceneMissingLayerDiagnosticFault::MissingShaderPipeline,
            out_result);
    }

    if (!FillRenderSceneConsumedReports(out_result)) {
        return CompleteWithDiagnostic(
            RenderSceneMissingLayerDiagnosticFault::MissingRenderSceneMultiEntitySubmission,
            out_result);
    }

    if (!EmitImageArtifacts(request, proof_camera.projection, out_result)) {
        return CompleteWithDiagnostic(
            RenderSceneMissingLayerDiagnosticFault::MissingCaptureOutputImage,
            out_result);
    }

    return CompleteWithDiagnostic(RenderSceneMissingLayerDiagnosticFault::None, out_result);
}

RenderSceneRuntimeVisualSceneProofStatus RenderSceneRuntimeVisualSceneProofRoute::CompleteWithDiagnostic(
    RenderSceneMissingLayerDiagnosticFault fault,
    RenderSceneRuntimeVisualSceneProofResult *out_result) const {
    if (out_result == nullptr) {
        return RenderSceneRuntimeVisualSceneProofStatus::InvalidArgument;
    }

    RenderSceneMissingLayerDiagnosticRoute diagnostic_route;
    RenderSceneMissingLayerDiagnosticRequest diagnostic_request{};
    diagnostic_request.fault = fault;
    diagnostic_request.target_capture_environment_available = true;
    const RenderSceneMissingLayerDiagnosticStatus diagnostic_status =
        diagnostic_route.Execute(diagnostic_request, &out_result->diagnostic);
    out_result->first_missing_layer = out_result->diagnostic.first_missing_layer;
    out_result->status = MapDiagnosticStatus(diagnostic_status);
    return out_result->status;
}

RenderSceneMissingLayerDiagnosticFault RenderSceneRuntimeVisualSceneProofRoute::MapOrbitMissingLayer(
    RenderSceneOrbitCaptureMissingLayer layer) const {
    if (layer == RenderSceneOrbitCaptureMissingLayer::None) {
        return RenderSceneMissingLayerDiagnosticFault::None;
    }

    if (layer == RenderSceneOrbitCaptureMissingLayer::CameraOrbit) {
        return RenderSceneMissingLayerDiagnosticFault::MissingCameraOrbitSequencing;
    }

    if (layer == RenderSceneOrbitCaptureMissingLayer::Camera) {
        return RenderSceneMissingLayerDiagnosticFault::MissingCamera;
    }

    if (layer == RenderSceneOrbitCaptureMissingLayer::GeometryModel) {
        return RenderSceneMissingLayerDiagnosticFault::MissingGeometryModel;
    }

    if (layer == RenderSceneOrbitCaptureMissingLayer::MaterialTextureSlots) {
        return RenderSceneMissingLayerDiagnosticFault::MissingMaterialTextureSlots;
    }

    if (layer == RenderSceneOrbitCaptureMissingLayer::ShaderPipeline) {
        return RenderSceneMissingLayerDiagnosticFault::MissingShaderPipeline;
    }

    if (layer == RenderSceneOrbitCaptureMissingLayer::ScenePlacement) {
        return RenderSceneMissingLayerDiagnosticFault::MissingScenePlacement;
    }

    if (layer == RenderSceneOrbitCaptureMissingLayer::AnimationInterpolation) {
        return RenderSceneMissingLayerDiagnosticFault::MissingAnimationInterpolation;
    }

    if (layer == RenderSceneOrbitCaptureMissingLayer::RenderSceneSubmission) {
        return RenderSceneMissingLayerDiagnosticFault::MissingRenderSceneMultiEntitySubmission;
    }

    if (layer == RenderSceneOrbitCaptureMissingLayer::RenderCoreRhiDrawCapture) {
        return RenderSceneMissingLayerDiagnosticFault::MissingRenderCoreRhiDrawCapture;
    }

    if (layer == RenderSceneOrbitCaptureMissingLayer::RhiCaptureTarget) {
        return RenderSceneMissingLayerDiagnosticFault::MissingRhiCaptureTarget;
    }

    if (layer == RenderSceneOrbitCaptureMissingLayer::OutputPath) {
        return RenderSceneMissingLayerDiagnosticFault::MissingOutputBounding;
    }

    return RenderSceneMissingLayerDiagnosticFault::MissingOutputBounding;
}

RenderSceneRuntimeVisualSceneProofStatus RenderSceneRuntimeVisualSceneProofRoute::MapDiagnosticStatus(
    RenderSceneMissingLayerDiagnosticStatus status) const {
    if (status == RenderSceneMissingLayerDiagnosticStatus::Success) {
        return RenderSceneRuntimeVisualSceneProofStatus::Success;
    }

    if (status == RenderSceneMissingLayerDiagnosticStatus::BlockedByEnv) {
        return RenderSceneRuntimeVisualSceneProofStatus::BlockedByEnv;
    }

    if (status == RenderSceneMissingLayerDiagnosticStatus::Fail) {
        return RenderSceneRuntimeVisualSceneProofStatus::Fail;
    }

    return RenderSceneRuntimeVisualSceneProofStatus::InvalidArgument;
}
}
