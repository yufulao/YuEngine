// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneMissingLayerDiagnosticRoute.h

#pragma once

#include <cstddef>

namespace yuengine::renderscene {
constexpr std::size_t MAX_RENDER_SCENE_MISSING_LAYER_DIAGNOSTIC_NAME_BYTES = 64U;

enum class RenderSceneMissingLayerDiagnosticStatus {
    Success,
    Fail,
    BlockedByEnv,
    InvalidArgument
};

enum class RenderSceneMissingLayerDiagnosticFault {
    None,
    MissingCamera,
    MissingGeometryModel,
    MissingMaterialTextureSlots,
    MissingTextureResourceResolution,
    MissingSamplerBinding,
    MissingShaderPipeline,
    MissingScenePlacement,
    MissingAnimationInterpolation,
    MissingTransformApply,
    MissingRenderSceneMultiEntitySubmission,
    MissingRenderCoreRhiDrawCapture,
    MissingCameraOrbitSequencing,
    MissingOutputBounding,
    MissingCaptureTargetResolution,
    MissingCaptureOutputImage,
    MissingRhiCaptureTarget
};

enum class RenderSceneMissingLayerDiagnosticLayer {
    None,
    Camera,
    GeometryModel,
    MaterialTextureSlots,
    TextureResourceResolution,
    SamplerBinding,
    ShaderPipeline,
    ScenePlacement,
    AnimationInterpolation,
    TransformApply,
    RenderSceneMultiEntitySubmission,
    RenderCoreRhiDrawCapture,
    CameraOrbitSequencing,
    OutputBounding,
    CaptureTargetResolution,
    CaptureOutputImage,
    RhiCaptureTarget
};

struct RenderSceneMissingLayerDiagnosticRequest final {
    RenderSceneMissingLayerDiagnosticFault fault = RenderSceneMissingLayerDiagnosticFault::None;
    bool target_capture_environment_available = true;
};

struct RenderSceneMissingLayerDiagnosticResult final {
    RenderSceneMissingLayerDiagnosticStatus status =
        RenderSceneMissingLayerDiagnosticStatus::InvalidArgument;
    RenderSceneMissingLayerDiagnosticLayer first_missing_layer =
        RenderSceneMissingLayerDiagnosticLayer::None;
    RenderSceneMissingLayerDiagnosticFault fault = RenderSceneMissingLayerDiagnosticFault::None;
    bool blocked_by_environment = false;
    char diagnostic_name[MAX_RENDER_SCENE_MISSING_LAYER_DIAGNOSTIC_NAME_BYTES]{};
    std::size_t diagnostic_name_byte_count = 0U;
};

class RenderSceneMissingLayerDiagnosticRoute final {
public:
    /**
     * @comment 执行 L1-VIS-006 missing-layer diagnostic route。
     * @param request 输入 fault-injection request。
     * @param out_result 调用方持有的输出报告。
     * @return 显式 diagnostic 状态。
     */
    RenderSceneMissingLayerDiagnosticStatus Execute(
        const RenderSceneMissingLayerDiagnosticRequest &request,
        RenderSceneMissingLayerDiagnosticResult *out_result) const;

private:
    RenderSceneMissingLayerDiagnosticLayer LayerForFault(
        RenderSceneMissingLayerDiagnosticFault fault) const;
    RenderSceneMissingLayerDiagnosticStatus StatusForLayer(
        RenderSceneMissingLayerDiagnosticLayer layer) const;
    const char *NameForLayer(RenderSceneMissingLayerDiagnosticLayer layer) const;
    bool CopyDiagnosticName(
        const char *name,
        RenderSceneMissingLayerDiagnosticResult *out_result) const;
};
}
