// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneOneCubeCaptureRoute.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/RenderCore/RenderDrawableFramePipelineResult.h"
#include "YuEngine/RenderScene/RenderSceneCameraBindingResult.h"
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
constexpr std::size_t MAX_RENDER_SCENE_ONE_CUBE_CAPTURE_OUTPUT_PATH_BYTES = 128U;

enum class RenderSceneOneCubeCaptureStatus {
    Success,
    Fail,
    BlockedByEnv,
    InvalidArgument
};

enum class RenderSceneOneCubeCaptureMissingLayer {
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

enum class RenderSceneOneCubeCaptureOutputStatus {
    NotRequested,
    PathRecorded,
    CaptureAvailable,
    BlockedByEnv
};

struct RenderSceneOneCubeCaptureRequest final {
    std::uint32_t frame_id = 0U;
    RenderSceneCameraBindingResult camera{};
    RenderScenePrimitiveGeometryRecord cube_geometry{};
    RenderSceneRuntimeMaterialRecord material{};
    yuengine::world::WorldObjectId world_object_id{};
    yuengine::world::WorldTransformState transform{};
    yuengine::rhi::IRhiDevice *rhi_device = nullptr;
    const char *output_path = nullptr;
    std::size_t output_path_byte_count = 0U;
    std::span<std::uint8_t> capture_output{};
    std::size_t capture_byte_budget = 0U;
};

struct RenderSceneOneCubeCaptureResult final {
    RenderSceneOneCubeCaptureStatus status = RenderSceneOneCubeCaptureStatus::InvalidArgument;
    RenderSceneOneCubeCaptureMissingLayer first_missing_layer =
        RenderSceneOneCubeCaptureMissingLayer::None;
    RenderSceneOneCubeCaptureOutputStatus output_status =
        RenderSceneOneCubeCaptureOutputStatus::NotRequested;
    std::uint32_t frame_id = 0U;
    RenderSceneCameraCaptureMetadata capture{};
    RenderSceneRuntimeFrameResult frame_result{};
    RenderSceneRuntimeFrameDrawRecord draw_record{};
    yuengine::rendercore::RenderDrawableFramePipelineResult render_result{};
    char output_path[MAX_RENDER_SCENE_ONE_CUBE_CAPTURE_OUTPUT_PATH_BYTES]{};
    std::size_t output_path_byte_count = 0U;
    std::size_t capture_bytes_written = 0U;
};

class RenderSceneOneCubeCaptureRoute final {
public:
    /**
     * @comment 执行 L1-VIS-001 static one-cube runtime capture route。
     * @param request 输入 one-cube capture request。
     * @param out_result 调用方持有的输出报告。
     * @return 显式 route 状态。
     */
    RenderSceneOneCubeCaptureStatus Execute(
        const RenderSceneOneCubeCaptureRequest &request,
        RenderSceneOneCubeCaptureResult *out_result);

private:
    bool CopyOutputPath(
        const RenderSceneOneCubeCaptureRequest &request,
        RenderSceneOneCubeCaptureResult *out_result) const;
    RenderSceneOneCubeCaptureStatus FailWithLayer(
        RenderSceneOneCubeCaptureMissingLayer layer,
        RenderSceneOneCubeCaptureResult *out_result) const;
    RenderSceneOneCubeCaptureMissingLayer MapMaterialLayer(
        RenderSceneRuntimeMaterialStatus status) const;
    RenderSceneOneCubeCaptureMissingLayer MapFrameLayer(
        RenderSceneRuntimeFrameStatus status) const;
    RenderSceneOneCubeCaptureStatus MapRenderCoreResult(
        const yuengine::rendercore::RenderDrawableFramePipelineResult &render_result,
        RenderSceneOneCubeCaptureResult *out_result) const;
};
}
