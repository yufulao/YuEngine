// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneRuntimeVisualSceneProofRoute.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/RenderScene/RenderSceneMissingLayerDiagnosticRoute.h"
#include "YuEngine/RenderScene/RenderSceneOrbitCaptureRoute.h"
#include "YuEngine/RenderScene/RenderSceneThreePrimitiveCaptureRoute.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

namespace yuengine::renderscene {
constexpr std::size_t MAX_RENDER_SCENE_RUNTIME_VISUAL_SCENE_OBJECT_NAME_BYTES =
    MAX_RENDER_SCENE_THREE_PRIMITIVE_OBJECT_NAME_BYTES;

enum class RenderSceneRuntimeVisualSceneProofStatus {
    Success,
    Fail,
    BlockedByEnv,
    InvalidArgument
};

struct RenderSceneRuntimeVisualSceneProofRequest final {
    std::uint32_t first_frame_id = 0U;
    std::uint32_t frame_count = 0U;
    yuengine::rhi::IRhiDevice *rhi_device = nullptr;
    const char *output_path_prefix = nullptr;
    std::size_t output_path_prefix_byte_count = 0U;
    std::span<std::uint8_t> capture_output{};
    std::size_t capture_byte_budget_per_entity = 0U;
    bool target_capture_environment_available = true;
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

struct RenderSceneRuntimeVisualSceneProofResult final {
    RenderSceneRuntimeVisualSceneProofStatus status =
        RenderSceneRuntimeVisualSceneProofStatus::InvalidArgument;
    RenderSceneMissingLayerDiagnosticLayer first_missing_layer =
        RenderSceneMissingLayerDiagnosticLayer::None;
    std::uint32_t requested_frame_count = 0U;
    std::uint32_t completed_frame_count = 0U;
    std::size_t capture_bytes_written = 0U;
    std::size_t frame_capture_byte_budget = 0U;
    std::size_t material_texture_slot_report_count = 0U;
    std::size_t entity_report_count = 0U;
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
