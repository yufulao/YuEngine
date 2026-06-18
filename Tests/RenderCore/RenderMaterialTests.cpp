// 模块：Tests RenderCore
// 文件：Tests/RenderCore/RenderMaterialTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/RenderCore/MaterialBindingFixtureRequest.h"
#include "YuEngine/RenderCore/RenderCamera.h"
#include "YuEngine/RenderCore/RenderCameraProjectionKind.h"
#include "YuEngine/RenderCore/RenderCameraShaderConstants.h"
#include "YuEngine/RenderCore/RenderCameraShaderConstantsWriter.h"
#include "YuEngine/RenderCore/RenderMaterial.h"
#include "YuEngine/RenderCore/RenderMaterialDesc.h"
#include "YuEngine/RenderCore/RenderMaterialRequest.h"
#include "YuEngine/RenderCore/RenderMaterialStatus.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"

using MaterialBindingFixtureRequest = yuengine::rendercore::MaterialBindingFixtureRequest;
using RenderCamera = yuengine::rendercore::RenderCamera;
using RenderCameraFrame = yuengine::rendercore::RenderCameraFrame;
using RenderCameraPose = yuengine::rendercore::RenderCameraPose;
using RenderCameraProjectionDesc = yuengine::rendercore::RenderCameraProjectionDesc;
using RenderCameraShaderConstants = yuengine::rendercore::RenderCameraShaderConstants;
using RenderCameraShaderConstantsWriter = yuengine::rendercore::RenderCameraShaderConstantsWriter;
using RenderMaterial = yuengine::rendercore::RenderMaterial;
using RenderMaterialDesc = yuengine::rendercore::RenderMaterialDesc;
using RenderMaterialRequest = yuengine::rendercore::RenderMaterialRequest;
using yuengine::rendercore::RenderCameraProjectionKind;
using yuengine::rendercore::RenderMaterialStatus;
using yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiSampledTextureBinding;
using yuengine::rhi::RhiSamplerBinding;
using yuengine::rhi::RhiSamplerHandle;
using yuengine::rhi::RhiTextureHandle;

namespace {
constexpr const char *TEST_BUILDS_BINDING = "RenderCore_MaterialBuildsBindingRequest";
constexpr const char *TEST_CAMERA_CONSTANTS = "RenderCore_MaterialAcceptsCameraShaderConstants";
constexpr const char *TEST_INVALID_PROGRAM = "RenderCore_MaterialRejectsInvalidProgramIdWithoutOutputMutation";
constexpr const char *TEST_OVERSIZED_CONSTANTS = "RenderCore_MaterialRejectsOversizedConstantsWithoutOutputMutation";
constexpr const char *TEST_DUPLICATE_ID = "RenderCore_MaterialRejectsDuplicateMaterialId";
constexpr const char *TEST_CAPACITY = "RenderCore_MaterialRejectsCapacityExceeded";
constexpr const char *TEST_SNAPSHOT = "RenderCore_MaterialSnapshotTracksCounters";
constexpr const char *TEST_BOUNDARY = "RenderCore_MaterialHasNarrowDependencyBoundary";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr float HALF_PI = 1.57079632679F;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

std::array<std::uint8_t, 16U> SmallConstants() {
    std::array<std::uint8_t, 16U> constants{};
    constants[0U] = 1U;
    constants[15U] = 15U;
    return constants;
}

std::array<std::uint8_t, 65U> OversizedConstants() {
    std::array<std::uint8_t, 65U> constants{};
    constants[0U] = 1U;
    return constants;
}

RenderCameraShaderConstants CameraConstants() {
    RenderCameraPose pose{};
    pose.position = {0.0F, 0.0F, -5.0F};
    pose.target = {0.0F, 0.0F, 0.0F};
    pose.up = {0.0F, 1.0F, 0.0F};

    RenderCameraProjectionDesc projection{};
    projection.kind = RenderCameraProjectionKind::Perspective;
    projection.vertical_fov_radians = HALF_PI;
    projection.aspect_ratio = 1.0F;
    projection.near_z = 0.1F;
    projection.far_z = 100.0F;

    RenderCamera camera;
    RenderCameraFrame frame{};
    camera.BuildFrame(pose, projection, &frame);

    RenderCameraShaderConstantsWriter writer;
    RenderCameraShaderConstants constants{};
    writer.WriteViewProjection(frame, &constants);
    return constants;
}

RenderMaterialRequest DefaultRequest(std::span<const std::uint8_t> constants) {
    RenderMaterialRequest request{};
    request.material_id = 20U;
    request.program_id = 10U;
    request.pipeline = RhiPipelineHandle{1U, 1U};
    request.sampled_texture = RhiSampledTextureBinding{RhiTextureHandle{2U, 1U}, 0U};
    request.sampler = RhiSamplerBinding{RhiSamplerHandle{3U, 1U}, 0U};
    request.constant_bytes = constants;
    request.pass_id = 30U;
    return request;
}

MaterialBindingFixtureRequest SentinelBindingRequest() {
    MaterialBindingFixtureRequest request{};
    request.material_id = 77U;
    request.pipeline = RhiPipelineHandle{77U, 77U};
    request.pass_id = 77U;
    return request;
}

bool BindingRequestMatchesSentinel(const MaterialBindingFixtureRequest &request) {
    if (request.material_id != 77U) {
        return false;
    }

    if (request.pipeline.slot != 77U || request.pipeline.generation != 77U) {
        return false;
    }

    return request.pass_id == 77U;
}

int RenderCoreMaterialBuildsBindingRequest() {
    const auto constants = SmallConstants();
    RenderMaterial material;
    MaterialBindingFixtureRequest binding_request{};
    const auto result = material.BuildBindingRequest(DefaultRequest(constants), &binding_request);
    if (result.status != RenderMaterialStatus::Success) {
        return Fail("render material rejected valid request");
    }

    if (binding_request.material_id != 20U || binding_request.pass_id != 30U) {
        return Fail("render material did not write material identity");
    }

    if (binding_request.pipeline.slot != 1U || binding_request.sampled_texture.texture.slot != 2U) {
        return Fail("render material did not write binding handles");
    }

    if (binding_request.constant_bytes.size() != constants.size()) {
        return Fail("render material did not write constant span");
    }

    const auto snapshot = material.Snapshot();
    if (snapshot.accepted_material_count != 1U || snapshot.last_status != RenderMaterialStatus::Success) {
        return Fail("render material snapshot missed accepted request");
    }

    return 0;
}

int RenderCoreMaterialAcceptsCameraShaderConstants() {
    const RenderCameraShaderConstants camera_constants = CameraConstants();
    const auto byte_count = camera_constants.view_projection_values.size() * sizeof(float);
    const auto *bytes = reinterpret_cast<const std::uint8_t *>(camera_constants.view_projection_values.data());
    const std::span<const std::uint8_t> constants(bytes, byte_count);

    RenderMaterial material;
    MaterialBindingFixtureRequest binding_request{};
    const auto result = material.BuildBindingRequest(DefaultRequest(constants), &binding_request);
    if (result.status != RenderMaterialStatus::Success) {
        return Fail("render material rejected camera constants");
    }

    if (binding_request.constant_bytes.size() != byte_count) {
        return Fail("render material changed camera constant byte count");
    }

    return 0;
}

int RenderCoreMaterialRejectsInvalidProgramIdWithoutOutputMutation() {
    const auto constants = SmallConstants();
    RenderMaterialRequest request = DefaultRequest(constants);
    request.program_id = 0U;

    RenderMaterial material;
    MaterialBindingFixtureRequest binding_request = SentinelBindingRequest();
    const auto result = material.BuildBindingRequest(request, &binding_request);
    if (result.status != RenderMaterialStatus::InvalidProgramId) {
        return Fail("render material accepted invalid program id");
    }

    if (!BindingRequestMatchesSentinel(binding_request)) {
        return Fail("render material mutated output after invalid program id");
    }

    return 0;
}

int RenderCoreMaterialRejectsOversizedConstantsWithoutOutputMutation() {
    const auto constants = OversizedConstants();
    RenderMaterial material;
    MaterialBindingFixtureRequest binding_request = SentinelBindingRequest();
    const auto result = material.BuildBindingRequest(DefaultRequest(constants), &binding_request);
    if (result.status != RenderMaterialStatus::OversizedConstants) {
        return Fail("render material accepted oversized constants");
    }

    if (!BindingRequestMatchesSentinel(binding_request)) {
        return Fail("render material mutated output after oversized constants");
    }

    return 0;
}

int RenderCoreMaterialRejectsDuplicateMaterialId() {
    const auto constants = SmallConstants();
    RenderMaterial material;
    MaterialBindingFixtureRequest binding_request{};
    if (material.BuildBindingRequest(DefaultRequest(constants), &binding_request).status != RenderMaterialStatus::Success) {
        return Fail("render material failed duplicate setup");
    }

    MaterialBindingFixtureRequest rejected_request = SentinelBindingRequest();
    const auto result = material.BuildBindingRequest(DefaultRequest(constants), &rejected_request);
    if (result.status != RenderMaterialStatus::DuplicateMaterialId) {
        return Fail("render material accepted duplicate material id");
    }

    if (!BindingRequestMatchesSentinel(rejected_request)) {
        return Fail("render material mutated output after duplicate id");
    }

    const auto snapshot = material.Snapshot();
    if (snapshot.duplicate_material_id_count != 1U) {
        return Fail("render material snapshot missed duplicate id");
    }

    return 0;
}

int RenderCoreMaterialRejectsCapacityExceeded() {
    RenderMaterialDesc desc{};
    desc.material_record_capacity = 1U;
    RenderMaterial material(desc);

    const auto constants = SmallConstants();
    MaterialBindingFixtureRequest binding_request{};
    if (material.BuildBindingRequest(DefaultRequest(constants), &binding_request).status != RenderMaterialStatus::Success) {
        return Fail("render material failed capacity setup");
    }

    RenderMaterialRequest second_request = DefaultRequest(constants);
    second_request.material_id = 21U;
    MaterialBindingFixtureRequest rejected_request = SentinelBindingRequest();
    const auto result = material.BuildBindingRequest(second_request, &rejected_request);
    if (result.status != RenderMaterialStatus::MaterialCapacityExceeded) {
        return Fail("render material accepted capacity overflow");
    }

    if (!BindingRequestMatchesSentinel(rejected_request)) {
        return Fail("render material mutated output after capacity overflow");
    }

    return 0;
}

int RenderCoreMaterialSnapshotTracksCounters() {
    const auto constants = SmallConstants();
    RenderMaterial material;
    MaterialBindingFixtureRequest binding_request{};
    if (material.BuildBindingRequest(DefaultRequest(constants), &binding_request).status != RenderMaterialStatus::Success) {
        return Fail("render material failed snapshot setup");
    }

    RenderMaterialRequest request = DefaultRequest(constants);
    request.material_id = 21U;
    request.pipeline = RhiPipelineHandle{};
    if (material.BuildBindingRequest(request, &binding_request).status != RenderMaterialStatus::InvalidPipeline) {
        return Fail("render material failed rejection setup");
    }

    const auto snapshot = material.Snapshot();
    if (snapshot.accepted_material_count != 1U || snapshot.failed_validation_count != 1U) {
        return Fail("render material snapshot counters were not stable");
    }

    material.Reset();
    const auto reset_snapshot = material.Snapshot();
    if (reset_snapshot.material_record_count != 0U || reset_snapshot.accepted_material_count != 0U) {
        return Fail("render material reset did not clear counters");
    }

    return 0;
}

int RenderCoreMaterialHasNarrowDependencyBoundary() {
    const auto constants = SmallConstants();
    RenderMaterial material;
    MaterialBindingFixtureRequest binding_request{};
    const auto result = material.BuildBindingRequest(DefaultRequest(constants), &binding_request);
    if (result.status != RenderMaterialStatus::Success) {
        return Fail("render material dependency smoke setup failed");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_BUILDS_BINDING) {
        return RenderCoreMaterialBuildsBindingRequest();
    }

    if (name == TEST_CAMERA_CONSTANTS) {
        return RenderCoreMaterialAcceptsCameraShaderConstants();
    }

    if (name == TEST_INVALID_PROGRAM) {
        return RenderCoreMaterialRejectsInvalidProgramIdWithoutOutputMutation();
    }

    if (name == TEST_OVERSIZED_CONSTANTS) {
        return RenderCoreMaterialRejectsOversizedConstantsWithoutOutputMutation();
    }

    if (name == TEST_DUPLICATE_ID) {
        return RenderCoreMaterialRejectsDuplicateMaterialId();
    }

    if (name == TEST_CAPACITY) {
        return RenderCoreMaterialRejectsCapacityExceeded();
    }

    if (name == TEST_SNAPSHOT) {
        return RenderCoreMaterialSnapshotTracksCounters();
    }

    if (name == TEST_BOUNDARY) {
        return RenderCoreMaterialHasNarrowDependencyBoundary();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
