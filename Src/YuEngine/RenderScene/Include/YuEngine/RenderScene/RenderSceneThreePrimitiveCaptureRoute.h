// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneThreePrimitiveCaptureRoute.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/RenderCore/RenderDrawableFramePipelineResult.h"
#include "YuEngine/RenderScene/RenderSceneCameraBindingResult.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryKind.h"
#include "YuEngine/RenderScene/RenderScenePrimitiveGeometryRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameDrawRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameResult.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeFrameStatus.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialStatus.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

namespace yuengine::renderscene {
constexpr std::size_t RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT = 3U;
constexpr std::size_t MAX_RENDER_SCENE_THREE_PRIMITIVE_OBJECT_NAME_BYTES = 32U;
constexpr std::size_t MAX_RENDER_SCENE_THREE_PRIMITIVE_CAPTURE_OUTPUT_PATH_BYTES = 128U;

enum class RenderSceneThreePrimitiveCaptureStatus {
    Success,
    Fail,
    BlockedByEnv,
    InvalidArgument
};

enum class RenderSceneThreePrimitiveCaptureMissingLayer {
    None,
    Camera,
    GeometryModel,
    MaterialTextureSlots,
    ShaderPipeline,
    ScenePlacement,
    RenderSceneSubmission,
    RenderCoreRhiDrawCapture,
    RhiCaptureTarget,
    OutputPath
};

enum class RenderSceneThreePrimitiveCaptureOutputStatus {
    NotRequested,
    PathRecorded,
    CaptureAvailable,
    BlockedByEnv
};

struct RenderSceneThreePrimitiveEntityRequest final {
    yuengine::world::WorldObjectId world_object_id{};
    const char *object_name = nullptr;
    std::size_t object_name_byte_count = 0U;
    yuengine::world::WorldTransformState transform{};
    RenderScenePrimitiveGeometryRecord geometry{};
    bool is_visible = true;
    bool is_active = true;
};

struct RenderSceneThreePrimitiveEntityReport final {
    yuengine::world::WorldObjectId world_object_id{};
    char object_name[MAX_RENDER_SCENE_THREE_PRIMITIVE_OBJECT_NAME_BYTES]{};
    std::size_t object_name_byte_count = 0U;
    yuengine::world::WorldTransformState transform{};
    RenderScenePrimitiveGeometryKind primitive_kind = RenderScenePrimitiveGeometryKind::Cube;
    RenderSceneRuntimeFrameDrawRecord draw_record{};
    std::uint32_t material_id = 0U;
    bool submitted = false;
};

struct RenderSceneThreePrimitiveMaterialTextureSlotReport final {
    std::uint32_t material_id = 0U;
    std::uint32_t slot = 0U;
    yuengine::asset::AssetHandle texture_asset{};
    yuengine::rhi::RhiSampledTextureBinding sampled_texture{};
    yuengine::rhi::RhiSamplerBinding sampler{};
    bool texture_resource_resolved = false;
    bool sampled_texture_bound = false;
    bool sampler_bound = false;
};

struct RenderSceneThreePrimitiveCaptureRequest final {
    std::uint32_t frame_id = 0U;
    RenderSceneCameraBindingResult camera{};
    RenderSceneRuntimeMaterialRecord material{};
    std::span<const RenderSceneThreePrimitiveEntityRequest> entities{};
    yuengine::rhi::IRhiDevice *rhi_device = nullptr;
    const char *output_path = nullptr;
    std::size_t output_path_byte_count = 0U;
    std::span<std::uint8_t> capture_output{};
    std::size_t capture_byte_budget_per_entity = 0U;
};

struct RenderSceneThreePrimitiveCaptureResult final {
    RenderSceneThreePrimitiveCaptureStatus status =
        RenderSceneThreePrimitiveCaptureStatus::InvalidArgument;
    RenderSceneThreePrimitiveCaptureMissingLayer first_missing_layer =
        RenderSceneThreePrimitiveCaptureMissingLayer::None;
    RenderSceneThreePrimitiveCaptureOutputStatus output_status =
        RenderSceneThreePrimitiveCaptureOutputStatus::NotRequested;
    std::uint32_t frame_id = 0U;
    RenderSceneCameraCaptureMetadata capture{};
    RenderSceneRuntimeFrameResult frame_result{};
    std::uint32_t shared_material_id = 0U;
    std::array<
        RenderSceneThreePrimitiveMaterialTextureSlotReport,
        MAX_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS> material_texture_slot_reports{};
    std::size_t material_texture_slot_report_count = 0U;
    std::array<RenderSceneThreePrimitiveEntityReport, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
        entity_reports{};
    std::size_t entity_report_count = 0U;
    std::array<yuengine::rendercore::RenderDrawableFramePipelineResult, RENDER_SCENE_THREE_PRIMITIVE_ENTITY_COUNT>
        render_results{};
    std::size_t render_result_count = 0U;
    char output_path[MAX_RENDER_SCENE_THREE_PRIMITIVE_CAPTURE_OUTPUT_PATH_BYTES]{};
    std::size_t output_path_byte_count = 0U;
    std::size_t capture_bytes_written = 0U;
};

class RenderSceneThreePrimitiveCaptureRoute final {
public:
    /**
     * @comment 执行 L1-VIS-002/003 three-primitive runtime visual route。
     * @param request 输入 three-primitive capture request。
     * @param out_result 调用方持有的输出报告。
     * @return 显式 route 状态。
     */
    RenderSceneThreePrimitiveCaptureStatus Execute(
        const RenderSceneThreePrimitiveCaptureRequest &request,
        RenderSceneThreePrimitiveCaptureResult *out_result);

private:
    bool CopyOutputPath(
        const RenderSceneThreePrimitiveCaptureRequest &request,
        RenderSceneThreePrimitiveCaptureResult *out_result) const;
    RenderSceneThreePrimitiveCaptureStatus FailWithLayer(
        RenderSceneThreePrimitiveCaptureMissingLayer layer,
        RenderSceneThreePrimitiveCaptureResult *out_result) const;
    RenderSceneThreePrimitiveCaptureMissingLayer MapMaterialLayer(
        RenderSceneRuntimeMaterialStatus status) const;
    RenderSceneThreePrimitiveCaptureMissingLayer MapFrameLayer(
        RenderSceneRuntimeFrameStatus status) const;
    RenderSceneThreePrimitiveCaptureStatus MapRenderCoreResult(
        const yuengine::rendercore::RenderDrawableFramePipelineResult &render_result,
        RenderSceneThreePrimitiveCaptureResult *out_result) const;
};
}
