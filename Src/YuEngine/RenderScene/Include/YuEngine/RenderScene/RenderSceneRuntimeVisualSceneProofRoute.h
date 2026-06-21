// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneRuntimeVisualSceneProofRoute.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/RenderCore/RenderCameraPose.h"
#include "YuEngine/RenderCore/RenderCameraProjectionKind.h"
#include "YuEngine/RenderScene/RenderSceneMissingLayerDiagnosticRoute.h"
#include "YuEngine/RenderScene/RenderSceneOrbitCaptureRoute.h"
#include "YuEngine/RenderScene/RenderSceneThreePrimitiveCaptureRoute.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

namespace yuengine::renderscene {
constexpr std::size_t MAX_RENDER_SCENE_RUNTIME_VISUAL_SCENE_OBJECT_NAME_BYTES =
    MAX_RENDER_SCENE_THREE_PRIMITIVE_OBJECT_NAME_BYTES;
constexpr std::size_t MAX_RENDER_SCENE_RUNTIME_VISUAL_SCENE_IMAGE_PATH_BYTES =
    MAX_RENDER_SCENE_ORBIT_CAPTURE_OUTPUT_PATH_BYTES;
constexpr std::size_t MAX_RENDER_SCENE_RUNTIME_VISUAL_SCENE_CAMERA_TWEEN_KEYFRAME_COUNT = 4U;

enum class RenderSceneRuntimeVisualSceneProofStatus {
    Success,
    Fail,
    BlockedByEnv,
    InvalidArgument
};

enum class RenderSceneRuntimeVisualSceneImageArtifactStatus {
    NotRequested,
    Written,
    Fail
};

enum class RenderSceneRuntimeVisualSceneCameraTweenEase {
    Linear,
    SmoothStep
};

struct RenderSceneRuntimeVisualSceneCameraTweenKeyframe final {
    float time_seconds = 0.0F;
    yuengine::rendercore::RenderCameraPose pose{};
    float vertical_fov_radians = 0.0F;
    RenderSceneRuntimeVisualSceneCameraTweenEase ease =
        RenderSceneRuntimeVisualSceneCameraTweenEase::Linear;
};

struct RenderSceneRuntimeVisualSceneCameraTweenFrameReport final {
    std::uint32_t frame_index = 0U;
    std::uint32_t frame_id = 0U;
    std::size_t source_keyframe_index = 0U;
    std::size_t target_keyframe_index = 0U;
    float sample_time_seconds = 0.0F;
    float linear_t = 0.0F;
    float eased_t = 0.0F;
    float vertical_fov_radians = 0.0F;
    yuengine::rendercore::RenderCameraPose camera_pose{};
};

struct RenderSceneRuntimeVisualSceneProofRequest final {
    std::uint32_t first_frame_id = 0U;
    std::uint32_t frame_count = 0U;
    yuengine::rhi::IRhiDevice *rhi_device = nullptr;
    const char *output_path_prefix = nullptr;
    std::size_t output_path_prefix_byte_count = 0U;
    bool image_artifact_requested = false;
    const char *image_output_path_prefix = nullptr;
    std::size_t image_output_path_prefix_byte_count = 0U;
    std::uint16_t target_image_artifact_width = 0U;
    std::uint16_t target_image_artifact_height = 0U;
    std::uint16_t target_capture_width = 0U;
    std::uint16_t target_capture_height = 0U;
    std::uint16_t minimum_image_artifact_width = 0U;
    std::uint16_t minimum_image_artifact_height = 0U;
    std::span<std::uint8_t> capture_output{};
    std::size_t capture_byte_budget_per_entity = 0U;
    bool target_capture_environment_available = true;
    bool close_orbit_loop = true;
    bool camera_tween_requested = false;
    bool transparent_panel_blend_requested = false;
    std::span<const RenderSceneRuntimeVisualSceneCameraTweenKeyframe> camera_tween_keyframes{};
    RenderSceneMissingLayerDiagnosticFault diagnostic_fault =
        RenderSceneMissingLayerDiagnosticFault::None;
};

struct RenderSceneRuntimeVisualSceneProofEntityReport final {
    yuengine::world::WorldObjectId world_object_id{};
    char object_name[MAX_RENDER_SCENE_RUNTIME_VISUAL_SCENE_OBJECT_NAME_BYTES]{};
    std::size_t object_name_byte_count = 0U;
    RenderScenePrimitiveGeometryKind primitive_kind = RenderScenePrimitiveGeometryKind::Cube;
    yuengine::world::WorldTransformState base_transform{};
    yuengine::world::WorldTransformState animated_transform{};
    yuengine::world::WorldTransformState render_scene_consumed_transform{};
    std::uint32_t animation_clip_id = 0U;
    std::uint32_t animation_track_id = 0U;
    float sampled_value = 0.0F;
    bool animation_sampled = false;
    bool transform_applied = false;
    bool render_scene_submitted = false;
};

struct RenderSceneRuntimeVisualSceneImageArtifactReport final {
    RenderSceneRuntimeVisualSceneImageArtifactStatus status =
        RenderSceneRuntimeVisualSceneImageArtifactStatus::NotRequested;
    std::uint32_t frame_index = 0U;
    std::uint32_t frame_id = 0U;
    std::uint16_t width = 0U;
    std::uint16_t height = 0U;
    std::size_t source_byte_count = 0U;
    std::size_t file_byte_count = 0U;
    char output_path[MAX_RENDER_SCENE_RUNTIME_VISUAL_SCENE_IMAGE_PATH_BYTES]{};
    std::size_t output_path_byte_count = 0U;
};

struct RenderSceneRuntimeVisualSceneProofResult final {
    RenderSceneRuntimeVisualSceneProofStatus status =
        RenderSceneRuntimeVisualSceneProofStatus::InvalidArgument;
    RenderSceneMissingLayerDiagnosticLayer first_missing_layer =
        RenderSceneMissingLayerDiagnosticLayer::None;
    std::uint32_t requested_frame_count = 0U;
    std::uint32_t completed_frame_count = 0U;
    std::size_t capture_bytes_written = 0U;
    std::size_t frame_capture_byte_budget = 0U;
    std::size_t image_artifact_report_count = 0U;
    std::size_t image_artifact_bytes_written = 0U;
    std::uint16_t requested_target_image_artifact_width = 0U;
    std::uint16_t requested_target_image_artifact_height = 0U;
    std::uint16_t requested_target_capture_width = 0U;
    std::uint16_t requested_target_capture_height = 0U;
    std::uint16_t requested_minimum_image_artifact_width = 0U;
    std::uint16_t requested_minimum_image_artifact_height = 0U;
    std::uint16_t available_image_artifact_width = 0U;
    std::uint16_t available_image_artifact_height = 0U;
    yuengine::rendercore::RenderCameraProjectionKind camera_projection_kind =
        yuengine::rendercore::RenderCameraProjectionKind::Perspective;
    float camera_vertical_fov_radians = 0.0F;
    float camera_aspect_ratio = 0.0F;
    float camera_orthographic_height = 0.0F;
    bool camera_perspective_projection_used = false;
    bool close_orbit_loop = true;
    bool camera_tween_used = false;
    bool transparent_panel_blend_used = false;
    bool transparent_panel_overlaps_primitive = false;
    bool transparent_panel_overlaps_background = false;
    std::uint8_t transparent_panel_alpha = 0U;
    std::size_t camera_tween_keyframe_count = 0U;
    std::size_t material_texture_slot_report_count = 0U;
    std::size_t entity_report_count = 0U;
    yuengine::rhi::RhiColor transparent_panel_source_color{};
    yuengine::rhi::RhiColor transparent_panel_background_color{};
    yuengine::rhi::RhiColor transparent_panel_primitive_color{};
    yuengine::rhi::RhiColor transparent_panel_blended_background_pixel{};
    yuengine::rhi::RhiColor transparent_panel_blended_primitive_pixel{};
    yuengine::rhi::RhiColor transparent_panel_opaque_pixel{};
    std::array<
        RenderSceneRuntimeVisualSceneImageArtifactReport,
        MAX_RENDER_SCENE_ORBIT_CAPTURE_FRAME_COUNT> image_artifact_reports{};
    std::array<
        RenderSceneRuntimeVisualSceneCameraTweenFrameReport,
        MAX_RENDER_SCENE_ORBIT_CAPTURE_FRAME_COUNT> camera_tween_frame_reports{};
    std::array<
        RenderSceneRuntimeVisualSceneProofEntityReport,
        RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT> entity_reports{};
    RenderSceneOrbitCaptureResult orbit_result{};
    RenderSceneMissingLayerDiagnosticResult diagnostic{};
};

class RenderSceneRuntimeVisualSceneProofRoute final {
public:
    /**
     * @comment 执行 L1-SAMPLE-011/012 runtime visual scene proof route。
     * @param request 输入 final scene proof request。
     * @param out_result 调用方持有的输出报告。
     * @return 显式 proof 状态。
     */
    RenderSceneRuntimeVisualSceneProofStatus Execute(
        const RenderSceneRuntimeVisualSceneProofRequest &request,
        RenderSceneRuntimeVisualSceneProofResult *out_result) const;

private:
    RenderSceneRuntimeVisualSceneProofStatus CompleteWithDiagnostic(
        RenderSceneMissingLayerDiagnosticFault fault,
        RenderSceneRuntimeVisualSceneProofResult *out_result) const;
    RenderSceneMissingLayerDiagnosticFault MapOrbitMissingLayer(
        RenderSceneOrbitCaptureMissingLayer layer) const;
    RenderSceneRuntimeVisualSceneProofStatus MapDiagnosticStatus(
        RenderSceneMissingLayerDiagnosticStatus status) const;
};
}
