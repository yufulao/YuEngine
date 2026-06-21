// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Src/RenderSceneRuntimeVisualSceneProofRoute.cpp

#include "YuEngine/RenderScene/RenderSceneRuntimeVisualSceneProofRoute.h"

#include <array>
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
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeCameraRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialBuilder.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialTextureSlot.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiDeviceSnapshot.h"
#include "YuEngine/Rhi/RhiFormat.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"
#include "YuEngine/Rhi/RhiSamplerHandle.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
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
constexpr float PROOF_ORBIT_RADIUS = 5.0F;
constexpr float PROOF_ORBIT_HEIGHT = 2.0F;
constexpr std::size_t IMAGE_FRAME_PATH_STEM_BYTE_COUNT = 6U;
constexpr std::size_t IMAGE_FRAME_PATH_DIGIT_BYTE_COUNT = 3U;
constexpr std::size_t IMAGE_FRAME_PATH_EXTENSION_BYTE_COUNT = 4U;
constexpr std::size_t IMAGE_PATH_SUFFIX_BYTE_COUNT =
    IMAGE_FRAME_PATH_STEM_BYTE_COUNT +
    IMAGE_FRAME_PATH_DIGIT_BYTE_COUNT +
    IMAGE_FRAME_PATH_EXTENSION_BYTE_COUNT;
constexpr std::size_t PPM_HEADER_MAX_BYTES = 32U;
constexpr char IMAGE_FRAME_PATH_STEM[] = ".Frame";
constexpr char IMAGE_FRAME_PATH_EXTENSION[] = ".ppm";
constexpr char PPM_MAGIC[] = "P6\n";
constexpr char PPM_MAX_VALUE[] = "\n255\n";

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
using RhiBufferHandle = yuengine::rhi::RhiBufferHandle;
using RhiDeviceSnapshot = yuengine::rhi::RhiDeviceSnapshot;
using RhiFormat = yuengine::rhi::RhiFormat;
using RhiIndexBufferView = yuengine::rhi::RhiIndexBufferView;
using RhiIndexFormat = yuengine::rhi::RhiIndexFormat;
using RhiPipelineHandle = yuengine::rhi::RhiPipelineHandle;
using RhiSamplerHandle = yuengine::rhi::RhiSamplerHandle;
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
    camera.projection.aspect_ratio = 1.0F;
    camera.projection.near_z = 0.1F;
    camera.projection.far_z = 100.0F;
    camera.target = RhiTextureHandle{7U, 1U};
    camera.clear_color = yuengine::rhi::RhiColor{10U, 20U, 30U, 255U};
    camera.is_active = true;
    return camera;
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

bool WriteFramePpmImage(
    const RenderSceneRuntimeVisualSceneProofRequest &request,
    std::size_t frame_capture_offset,
    std::uint16_t source_width,
    std::uint16_t source_height,
    std::size_t source_byte_count_per_entity,
    RenderSceneRuntimeVisualSceneImageArtifactReport *out_report) {
    if (out_report == nullptr) {
        return false;
    }

    if (!CreateImageArtifactParentDirectory(out_report->output_path)) {
        return false;
    }

    std::FILE *file = OpenBinaryWriteFile(out_report->output_path);
    if (file == nullptr) {
        return false;
    }

    const std::uint16_t image_width =
        static_cast<std::uint16_t>(source_width * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT);
    std::size_t file_byte_count = 0U;
    if (!WritePpmHeader(file, image_width, source_height, &file_byte_count)) {
        std::fclose(file);
        return false;
    }

    for (std::uint16_t row = 0U; row < source_height; ++row) {
        for (std::size_t entity_index = 0U;
            entity_index < RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
            ++entity_index) {
            const std::size_t entity_offset =
                frame_capture_offset + entity_index * request.capture_byte_budget_per_entity;
            for (std::uint16_t column = 0U; column < source_width; ++column) {
                const std::size_t pixel_index =
                    entity_offset +
                    ((static_cast<std::size_t>(row) * source_width + column) *
                        yuengine::rhi::RGBA8_BYTES_PER_PIXEL);
                if (pixel_index + 2U >= request.capture_output.size()) {
                    std::fclose(file);
                    return false;
                }

                if (!WritePpmByte(file, request.capture_output[pixel_index], &file_byte_count)) {
                    std::fclose(file);
                    return false;
                }

                if (!WritePpmByte(file, request.capture_output[pixel_index + 1U], &file_byte_count)) {
                    std::fclose(file);
                    return false;
                }

                if (!WritePpmByte(file, request.capture_output[pixel_index + 2U], &file_byte_count)) {
                    std::fclose(file);
                    return false;
                }
            }
        }
    }

    const int close_result = std::fclose(file);
    if (close_result != 0) {
        return false;
    }

    out_report->status = RenderSceneRuntimeVisualSceneImageArtifactStatus::Written;
    out_report->width = image_width;
    out_report->height = source_height;
    out_report->source_byte_count =
        source_byte_count_per_entity * RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT;
    out_report->file_byte_count = file_byte_count;
    return true;
}

bool EmitImageArtifacts(
    const RenderSceneRuntimeVisualSceneProofRequest &request,
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

    if (source_width > std::numeric_limits<std::uint16_t>::max() /
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT) {
        return false;
    }

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

        if (!WriteFramePpmImage(
                request,
                frame_capture_offset,
                source_width,
                source_height,
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

bool IsRequestStorageUsable(const RenderSceneRuntimeVisualSceneProofRequest &request) {
    if (request.output_path_prefix == nullptr) {
        return false;
    }

    if (request.output_path_prefix_byte_count == 0U) {
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

    RenderSceneOrbitCaptureRequest orbit_request{};
    orbit_request.first_frame_id = request.first_frame_id;
    orbit_request.frame_count = request.frame_count;
    orbit_request.camera_template = BuildCameraRecord();
    orbit_request.material = BuildRuntimeMaterialRecord();
    orbit_request.entities = entities;
    orbit_request.target = yuengine::rendercore::RenderCameraVector3{0.0F, 0.0F, 0.0F};
    orbit_request.orbit_radius = PROOF_ORBIT_RADIUS;
    orbit_request.orbit_height = PROOF_ORBIT_HEIGHT;
    orbit_request.rhi_device = request.rhi_device;
    orbit_request.output_path_prefix = request.output_path_prefix;
    orbit_request.output_path_prefix_byte_count = request.output_path_prefix_byte_count;
    orbit_request.capture_output = request.capture_output;
    orbit_request.capture_byte_budget_per_entity = request.capture_byte_budget_per_entity;

    RenderSceneOrbitCaptureRoute orbit_route;
    const RenderSceneOrbitCaptureStatus orbit_status =
        orbit_route.Execute(orbit_request, &out_result->orbit_result);
    out_result->completed_frame_count = out_result->orbit_result.completed_frame_count;
    out_result->capture_bytes_written = out_result->orbit_result.capture_bytes_written;
    out_result->frame_capture_byte_budget = out_result->orbit_result.frame_capture_byte_budget;

    if (orbit_status != RenderSceneOrbitCaptureStatus::Success) {
        const RenderSceneMissingLayerDiagnosticFault fault =
            MapOrbitMissingLayer(out_result->orbit_result.first_missing_layer);
        return CompleteWithDiagnostic(fault, out_result);
    }

    if (!FillRenderSceneConsumedReports(out_result)) {
        return CompleteWithDiagnostic(
            RenderSceneMissingLayerDiagnosticFault::MissingRenderSceneMultiEntitySubmission,
            out_result);
    }

    if (!EmitImageArtifacts(request, out_result)) {
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
