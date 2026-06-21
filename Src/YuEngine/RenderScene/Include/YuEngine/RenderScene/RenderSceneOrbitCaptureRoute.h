// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneOrbitCaptureRoute.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/RenderCore/RenderCameraPose.h"
#include "YuEngine/RenderCore/RenderCameraVector3.h"
#include "YuEngine/RenderScene/RenderSceneCameraCaptureMetadata.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeCameraRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialRecord.h"
#include "YuEngine/RenderScene/RenderSceneThreePrimitiveCaptureRoute.h"
#include "YuEngine/Rhi/IRhiDevice.h"

namespace yuengine::renderscene {
constexpr std::size_t MAX_RENDER_SCENE_ORBIT_CAPTURE_FRAME_COUNT = 8U;
constexpr std::size_t MAX_RENDER_SCENE_ORBIT_CAPTURE_OUTPUT_PATH_BYTES = 128U;

enum class RenderSceneOrbitCaptureStatus {
    Success,
    Fail,
    BlockedByEnv,
    InvalidArgument
};

enum class RenderSceneOrbitCaptureMissingLayer {
    None,
    CameraOrbit,
    Camera,
    GeometryModel,
    MaterialTextureSlots,
    ShaderPipeline,
    ScenePlacement,
    AnimationInterpolation,
    RenderSceneSubmission,
    RenderCoreRhiDrawCapture,
    RhiCaptureTarget,
    OutputPath,
    CaptureStorage
};

struct RenderSceneOrbitCaptureRequest final {
    std::uint32_t first_frame_id = 0U;
    std::uint32_t frame_count = 0U;
    RenderSceneRuntimeCameraRecord camera_template{};
    RenderSceneRuntimeMaterialRecord material{};
    std::span<const RenderSceneThreePrimitiveEntityRequest> entities{};
    yuengine::rendercore::RenderCameraVector3 target{};
    float orbit_radius = 0.0F;
    float orbit_height = 0.0F;
    bool close_orbit_loop = true;
    yuengine::rhi::IRhiDevice *rhi_device = nullptr;
    const char *output_path_prefix = nullptr;
    std::size_t output_path_prefix_byte_count = 0U;
    std::span<std::uint8_t> capture_output{};
    std::size_t capture_byte_budget_per_entity = 0U;
};

struct RenderSceneOrbitCaptureFrameReport final {
    std::uint32_t frame_index = 0U;
    std::uint32_t frame_id = 0U;
    float orbit_angle_radians = 0.0F;
    yuengine::rendercore::RenderCameraPose camera_pose{};
    yuengine::rendercore::RenderCameraVector3 target{};
    RenderSceneCameraCaptureMetadata capture{};
    RenderSceneOrbitCaptureStatus status = RenderSceneOrbitCaptureStatus::InvalidArgument;
    RenderSceneOrbitCaptureMissingLayer first_missing_layer = RenderSceneOrbitCaptureMissingLayer::None;
    RenderSceneThreePrimitiveCaptureOutputStatus output_status =
        RenderSceneThreePrimitiveCaptureOutputStatus::NotRequested;
    RenderSceneThreePrimitiveCaptureResult capture_result{};
    char output_path[MAX_RENDER_SCENE_ORBIT_CAPTURE_OUTPUT_PATH_BYTES]{};
    std::size_t output_path_byte_count = 0U;
    std::size_t capture_bytes_written = 0U;
};

struct RenderSceneOrbitCaptureResult final {
    RenderSceneOrbitCaptureStatus status = RenderSceneOrbitCaptureStatus::InvalidArgument;
    RenderSceneOrbitCaptureMissingLayer first_missing_layer = RenderSceneOrbitCaptureMissingLayer::None;
    std::uint32_t first_frame_id = 0U;
    std::uint32_t requested_frame_count = 0U;
    std::uint32_t completed_frame_count = 0U;
    yuengine::rendercore::RenderCameraVector3 target{};
    float orbit_radius = 0.0F;
    float orbit_height = 0.0F;
    std::size_t frame_capture_byte_budget = 0U;
    std::size_t capture_bytes_written = 0U;
    std::array<RenderSceneOrbitCaptureFrameReport, MAX_RENDER_SCENE_ORBIT_CAPTURE_FRAME_COUNT>
        frames{};
};

class RenderSceneOrbitCaptureRoute final {
public:
    /**
     * @comment 执行 L1-VIS-005 deterministic orbit capture sequence。
     * @param request 输入 orbit capture request。
     * @param out_result 调用方持有的输出报告。
     * @return 显式 route 状态。
     */
    RenderSceneOrbitCaptureStatus Execute(
        const RenderSceneOrbitCaptureRequest &request,
        RenderSceneOrbitCaptureResult *out_result);

private:
    bool ValidateRequest(
        const RenderSceneOrbitCaptureRequest &request,
        RenderSceneOrbitCaptureResult *out_result) const;
    bool BuildFrameOutputPath(
        const RenderSceneOrbitCaptureRequest &request,
        std::uint32_t frame_index,
        RenderSceneOrbitCaptureFrameReport *out_report) const;
    RenderSceneOrbitCaptureStatus MapFrameStatus(
        const RenderSceneThreePrimitiveCaptureResult &capture_result,
        RenderSceneOrbitCaptureFrameReport *out_report) const;
    RenderSceneOrbitCaptureMissingLayer MapMissingLayer(
        RenderSceneThreePrimitiveCaptureMissingLayer layer) const;
};
}
