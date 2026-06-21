// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Src/RenderSceneMissingLayerDiagnosticRoute.cpp

#include "YuEngine/RenderScene/RenderSceneMissingLayerDiagnosticRoute.h"

namespace yuengine::renderscene {
namespace {
constexpr const char *DIAGNOSTIC_NAME_NONE = "None";
constexpr const char *DIAGNOSTIC_NAME_CAMERA = "Camera";
constexpr const char *DIAGNOSTIC_NAME_GEOMETRY_MODEL = "GeometryModel";
constexpr const char *DIAGNOSTIC_NAME_MATERIAL_TEXTURE_SLOTS = "MaterialTextureSlots";
constexpr const char *DIAGNOSTIC_NAME_TEXTURE_RESOURCE_RESOLUTION = "TextureResourceResolution";
constexpr const char *DIAGNOSTIC_NAME_SAMPLER_BINDING = "SamplerBinding";
constexpr const char *DIAGNOSTIC_NAME_SHADER_PIPELINE = "ShaderPipeline";
constexpr const char *DIAGNOSTIC_NAME_SCENE_PLACEMENT = "ScenePlacement";
constexpr const char *DIAGNOSTIC_NAME_ANIMATION_INTERPOLATION = "AnimationInterpolation";
constexpr const char *DIAGNOSTIC_NAME_TRANSFORM_APPLY = "TransformApply";
constexpr const char *DIAGNOSTIC_NAME_RENDER_SCENE_MULTI_ENTITY_SUBMISSION =
    "RenderSceneMultiEntitySubmission";
constexpr const char *DIAGNOSTIC_NAME_RENDER_CORE_RHI_DRAW_CAPTURE = "RenderCoreRhiDrawCapture";
constexpr const char *DIAGNOSTIC_NAME_CAMERA_ORBIT_SEQUENCING = "CameraOrbitSequencing";
constexpr const char *DIAGNOSTIC_NAME_OUTPUT_BOUNDING = "OutputBounding";
constexpr const char *DIAGNOSTIC_NAME_CAPTURE_TARGET_RESOLUTION = "CaptureTargetResolution";
constexpr const char *DIAGNOSTIC_NAME_CAPTURE_OUTPUT_IMAGE = "CaptureOutputImage";
constexpr const char *DIAGNOSTIC_NAME_RHI_CAPTURE_TARGET = "RhiCaptureTarget";
}

RenderSceneMissingLayerDiagnosticStatus RenderSceneMissingLayerDiagnosticRoute::Execute(
    const RenderSceneMissingLayerDiagnosticRequest &request,
    RenderSceneMissingLayerDiagnosticResult *out_result) const {
    if (out_result == nullptr) {
        return RenderSceneMissingLayerDiagnosticStatus::InvalidArgument;
    }

    RenderSceneMissingLayerDiagnosticResult result{};
    result.fault = request.fault;
    result.first_missing_layer = LayerForFault(request.fault);
    if (request.fault == RenderSceneMissingLayerDiagnosticFault::None &&
        !request.target_capture_environment_available) {
        result.first_missing_layer = RenderSceneMissingLayerDiagnosticLayer::RhiCaptureTarget;
    }

    result.status = StatusForLayer(result.first_missing_layer);
    result.blocked_by_environment =
        result.status == RenderSceneMissingLayerDiagnosticStatus::BlockedByEnv;
    if (!CopyDiagnosticName(NameForLayer(result.first_missing_layer), &result)) {
        result.status = RenderSceneMissingLayerDiagnosticStatus::InvalidArgument;
        *out_result = result;
        return result.status;
    }

    *out_result = result;
    return result.status;
}

RenderSceneMissingLayerDiagnosticLayer RenderSceneMissingLayerDiagnosticRoute::LayerForFault(
    RenderSceneMissingLayerDiagnosticFault fault) const {
    if (fault == RenderSceneMissingLayerDiagnosticFault::None) {
        return RenderSceneMissingLayerDiagnosticLayer::None;
    }

    if (fault == RenderSceneMissingLayerDiagnosticFault::MissingCamera) {
        return RenderSceneMissingLayerDiagnosticLayer::Camera;
    }

    if (fault == RenderSceneMissingLayerDiagnosticFault::MissingGeometryModel) {
        return RenderSceneMissingLayerDiagnosticLayer::GeometryModel;
    }

    if (fault == RenderSceneMissingLayerDiagnosticFault::MissingMaterialTextureSlots) {
        return RenderSceneMissingLayerDiagnosticLayer::MaterialTextureSlots;
    }

    if (fault == RenderSceneMissingLayerDiagnosticFault::MissingTextureResourceResolution) {
        return RenderSceneMissingLayerDiagnosticLayer::TextureResourceResolution;
    }

    if (fault == RenderSceneMissingLayerDiagnosticFault::MissingSamplerBinding) {
        return RenderSceneMissingLayerDiagnosticLayer::SamplerBinding;
    }

    if (fault == RenderSceneMissingLayerDiagnosticFault::MissingShaderPipeline) {
        return RenderSceneMissingLayerDiagnosticLayer::ShaderPipeline;
    }

    if (fault == RenderSceneMissingLayerDiagnosticFault::MissingScenePlacement) {
        return RenderSceneMissingLayerDiagnosticLayer::ScenePlacement;
    }

    if (fault == RenderSceneMissingLayerDiagnosticFault::MissingAnimationInterpolation) {
        return RenderSceneMissingLayerDiagnosticLayer::AnimationInterpolation;
    }

    if (fault == RenderSceneMissingLayerDiagnosticFault::MissingTransformApply) {
        return RenderSceneMissingLayerDiagnosticLayer::TransformApply;
    }

    if (fault == RenderSceneMissingLayerDiagnosticFault::MissingRenderSceneMultiEntitySubmission) {
        return RenderSceneMissingLayerDiagnosticLayer::RenderSceneMultiEntitySubmission;
    }

    if (fault == RenderSceneMissingLayerDiagnosticFault::MissingRenderCoreRhiDrawCapture) {
        return RenderSceneMissingLayerDiagnosticLayer::RenderCoreRhiDrawCapture;
    }

    if (fault == RenderSceneMissingLayerDiagnosticFault::MissingCameraOrbitSequencing) {
        return RenderSceneMissingLayerDiagnosticLayer::CameraOrbitSequencing;
    }

    if (fault == RenderSceneMissingLayerDiagnosticFault::MissingOutputBounding) {
        return RenderSceneMissingLayerDiagnosticLayer::OutputBounding;
    }

    if (fault == RenderSceneMissingLayerDiagnosticFault::MissingCaptureTargetResolution) {
        return RenderSceneMissingLayerDiagnosticLayer::CaptureTargetResolution;
    }

    if (fault == RenderSceneMissingLayerDiagnosticFault::MissingCaptureOutputImage) {
        return RenderSceneMissingLayerDiagnosticLayer::CaptureOutputImage;
    }

    if (fault == RenderSceneMissingLayerDiagnosticFault::MissingRhiCaptureTarget) {
        return RenderSceneMissingLayerDiagnosticLayer::RhiCaptureTarget;
    }

    return RenderSceneMissingLayerDiagnosticLayer::None;
}

RenderSceneMissingLayerDiagnosticStatus RenderSceneMissingLayerDiagnosticRoute::StatusForLayer(
    RenderSceneMissingLayerDiagnosticLayer layer) const {
    if (layer == RenderSceneMissingLayerDiagnosticLayer::None) {
        return RenderSceneMissingLayerDiagnosticStatus::Success;
    }

    if (layer == RenderSceneMissingLayerDiagnosticLayer::RhiCaptureTarget) {
        return RenderSceneMissingLayerDiagnosticStatus::BlockedByEnv;
    }

    return RenderSceneMissingLayerDiagnosticStatus::Fail;
}

const char *RenderSceneMissingLayerDiagnosticRoute::NameForLayer(
    RenderSceneMissingLayerDiagnosticLayer layer) const {
    if (layer == RenderSceneMissingLayerDiagnosticLayer::None) {
        return DIAGNOSTIC_NAME_NONE;
    }

    if (layer == RenderSceneMissingLayerDiagnosticLayer::Camera) {
        return DIAGNOSTIC_NAME_CAMERA;
    }

    if (layer == RenderSceneMissingLayerDiagnosticLayer::GeometryModel) {
        return DIAGNOSTIC_NAME_GEOMETRY_MODEL;
    }

    if (layer == RenderSceneMissingLayerDiagnosticLayer::MaterialTextureSlots) {
        return DIAGNOSTIC_NAME_MATERIAL_TEXTURE_SLOTS;
    }

    if (layer == RenderSceneMissingLayerDiagnosticLayer::TextureResourceResolution) {
        return DIAGNOSTIC_NAME_TEXTURE_RESOURCE_RESOLUTION;
    }

    if (layer == RenderSceneMissingLayerDiagnosticLayer::SamplerBinding) {
        return DIAGNOSTIC_NAME_SAMPLER_BINDING;
    }

    if (layer == RenderSceneMissingLayerDiagnosticLayer::ShaderPipeline) {
        return DIAGNOSTIC_NAME_SHADER_PIPELINE;
    }

    if (layer == RenderSceneMissingLayerDiagnosticLayer::ScenePlacement) {
        return DIAGNOSTIC_NAME_SCENE_PLACEMENT;
    }

    if (layer == RenderSceneMissingLayerDiagnosticLayer::AnimationInterpolation) {
        return DIAGNOSTIC_NAME_ANIMATION_INTERPOLATION;
    }

    if (layer == RenderSceneMissingLayerDiagnosticLayer::TransformApply) {
        return DIAGNOSTIC_NAME_TRANSFORM_APPLY;
    }

    if (layer == RenderSceneMissingLayerDiagnosticLayer::RenderSceneMultiEntitySubmission) {
        return DIAGNOSTIC_NAME_RENDER_SCENE_MULTI_ENTITY_SUBMISSION;
    }

    if (layer == RenderSceneMissingLayerDiagnosticLayer::RenderCoreRhiDrawCapture) {
        return DIAGNOSTIC_NAME_RENDER_CORE_RHI_DRAW_CAPTURE;
    }

    if (layer == RenderSceneMissingLayerDiagnosticLayer::CameraOrbitSequencing) {
        return DIAGNOSTIC_NAME_CAMERA_ORBIT_SEQUENCING;
    }

    if (layer == RenderSceneMissingLayerDiagnosticLayer::OutputBounding) {
        return DIAGNOSTIC_NAME_OUTPUT_BOUNDING;
    }

    if (layer == RenderSceneMissingLayerDiagnosticLayer::CaptureTargetResolution) {
        return DIAGNOSTIC_NAME_CAPTURE_TARGET_RESOLUTION;
    }

    if (layer == RenderSceneMissingLayerDiagnosticLayer::CaptureOutputImage) {
        return DIAGNOSTIC_NAME_CAPTURE_OUTPUT_IMAGE;
    }

    if (layer == RenderSceneMissingLayerDiagnosticLayer::RhiCaptureTarget) {
        return DIAGNOSTIC_NAME_RHI_CAPTURE_TARGET;
    }

    return DIAGNOSTIC_NAME_NONE;
}

bool RenderSceneMissingLayerDiagnosticRoute::CopyDiagnosticName(
    const char *name,
    RenderSceneMissingLayerDiagnosticResult *out_result) const {
    if (name == nullptr) {
        return false;
    }

    if (out_result == nullptr) {
        return false;
    }

    std::size_t byte_count = 0U;
    while (name[byte_count] != '\0') {
        if (byte_count + 1U >= MAX_RENDER_SCENE_MISSING_LAYER_DIAGNOSTIC_NAME_BYTES) {
            return false;
        }

        out_result->diagnostic_name[byte_count] = name[byte_count];
        ++byte_count;
    }

    out_result->diagnostic_name[byte_count] = '\0';
    out_result->diagnostic_name_byte_count = byte_count;
    return true;
}
}
