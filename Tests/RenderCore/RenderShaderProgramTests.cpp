// 模块：Tests RenderCore
// 文件：Tests/RenderCore/RenderShaderProgramTests.cpp

#include <cstdio>
#include <string_view>

#include "YuEngine/RenderCore/RenderShaderProgram.h"
#include "YuEngine/RenderCore/RenderShaderProgramDesc.h"
#include "YuEngine/RenderCore/RenderShaderProgramRequest.h"
#include "YuEngine/RenderCore/RenderShaderProgramStatus.h"
#include "YuEngine/Rhi/RhiInputLayoutDesc.h"
#include "YuEngine/Rhi/RhiPipelineDesc.h"
#include "YuEngine/Rhi/RhiShaderModuleHandle.h"

using RenderShaderProgram = yuengine::rendercore::RenderShaderProgram;
using RenderShaderProgramDesc = yuengine::rendercore::RenderShaderProgramDesc;
using RenderShaderProgramRequest = yuengine::rendercore::RenderShaderProgramRequest;
using yuengine::rendercore::RenderShaderProgramStatus;
using yuengine::rhi::RhiInputElementDesc;
using yuengine::rhi::RhiInputElementFormat;
using yuengine::rhi::RhiInputElementSemantic;
using yuengine::rhi::RhiInputLayoutDesc;
using yuengine::rhi::RhiPipelineDesc;
using yuengine::rhi::RhiShaderModuleHandle;

namespace {
constexpr const char *TEST_BUILDS_PIPELINE_DESC = "RenderCore_ShaderProgramBuildsPipelineDesc";
constexpr const char *TEST_INVALID_VERTEX_SHADER = "RenderCore_ShaderProgramRejectsInvalidVertexShaderWithoutOutputMutation";
constexpr const char *TEST_INVALID_INPUT_LAYOUT = "RenderCore_ShaderProgramRejectsInvalidInputLayoutWithoutOutputMutation";
constexpr const char *TEST_DUPLICATE_ID = "RenderCore_ShaderProgramRejectsDuplicateProgramId";
constexpr const char *TEST_CAPACITY = "RenderCore_ShaderProgramRejectsCapacityExceeded";
constexpr const char *TEST_SNAPSHOT = "RenderCore_ShaderProgramSnapshotTracksCounters";
constexpr const char *TEST_BOUNDARY = "RenderCore_ShaderProgramHasNarrowDependencyBoundary";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

RhiShaderModuleHandle VertexShader() {
    return RhiShaderModuleHandle{1U, 1U};
}

RhiShaderModuleHandle PixelShader() {
    return RhiShaderModuleHandle{2U, 1U};
}

RhiInputLayoutDesc DefaultInputLayout() {
    RhiInputLayoutDesc layout{};
    layout.elements[0U] = RhiInputElementDesc{
        RhiInputElementSemantic::Position,
        RhiInputElementFormat::Float32x3,
        0U};
    layout.elements[1U] = RhiInputElementDesc{
        RhiInputElementSemantic::TexCoord,
        RhiInputElementFormat::Float32x2,
        12U};
    layout.element_count = 2U;
    layout.stride_bytes = 20U;
    return layout;
}

RenderShaderProgramRequest DefaultRequest() {
    RenderShaderProgramRequest request{};
    request.program_id = 10U;
    request.vertex_shader = VertexShader();
    request.pixel_shader = PixelShader();
    request.input_layout = DefaultInputLayout();
    return request;
}

RhiPipelineDesc SentinelPipelineDesc() {
    RhiPipelineDesc desc{};
    desc.vertex_shader = RhiShaderModuleHandle{77U, 77U};
    desc.pixel_shader = RhiShaderModuleHandle{88U, 88U};
    desc.input_layout.stride_bytes = 99U;
    return desc;
}

bool PipelineDescMatchesSentinel(const RhiPipelineDesc &desc) {
    if (desc.vertex_shader.slot != 77U || desc.vertex_shader.generation != 77U) {
        return false;
    }

    if (desc.pixel_shader.slot != 88U || desc.pixel_shader.generation != 88U) {
        return false;
    }

    return desc.input_layout.stride_bytes == 99U;
}

int RenderCoreShaderProgramBuildsPipelineDesc() {
    RenderShaderProgram program;
    RhiPipelineDesc pipeline_desc{};
    const auto result = program.BuildPipelineDesc(DefaultRequest(), &pipeline_desc);
    if (result.status != RenderShaderProgramStatus::Success) {
        return Fail("shader program rejected valid request");
    }

    if (pipeline_desc.vertex_shader.slot != 1U || pipeline_desc.pixel_shader.slot != 2U) {
        return Fail("shader program did not write shader handles");
    }

    if (pipeline_desc.input_layout.element_count != 2U || pipeline_desc.input_layout.stride_bytes != 20U) {
        return Fail("shader program did not write input layout");
    }

    const auto snapshot = program.Snapshot();
    if (snapshot.accepted_program_count != 1U || snapshot.last_status != RenderShaderProgramStatus::Success) {
        return Fail("shader program snapshot missed accepted program");
    }

    return 0;
}

int RenderCoreShaderProgramRejectsInvalidVertexShaderWithoutOutputMutation() {
    RenderShaderProgram program;
    RenderShaderProgramRequest request = DefaultRequest();
    request.vertex_shader = RhiShaderModuleHandle{};

    RhiPipelineDesc pipeline_desc = SentinelPipelineDesc();
    const auto result = program.BuildPipelineDesc(request, &pipeline_desc);
    if (result.status != RenderShaderProgramStatus::InvalidVertexShader) {
        return Fail("shader program accepted invalid vertex shader");
    }

    if (!PipelineDescMatchesSentinel(pipeline_desc)) {
        return Fail("shader program mutated output after invalid vertex shader");
    }

    return 0;
}

int RenderCoreShaderProgramRejectsInvalidInputLayoutWithoutOutputMutation() {
    RenderShaderProgram program;
    RenderShaderProgramRequest request = DefaultRequest();
    request.input_layout.stride_bytes = 0U;

    RhiPipelineDesc pipeline_desc = SentinelPipelineDesc();
    const auto result = program.BuildPipelineDesc(request, &pipeline_desc);
    if (result.status != RenderShaderProgramStatus::InvalidInputLayout) {
        return Fail("shader program accepted invalid input layout");
    }

    if (!PipelineDescMatchesSentinel(pipeline_desc)) {
        return Fail("shader program mutated output after invalid input layout");
    }

    return 0;
}

int RenderCoreShaderProgramRejectsDuplicateProgramId() {
    RenderShaderProgram program;
    RhiPipelineDesc pipeline_desc{};
    if (program.BuildPipelineDesc(DefaultRequest(), &pipeline_desc).status != RenderShaderProgramStatus::Success) {
        return Fail("shader program failed duplicate setup");
    }

    RhiPipelineDesc rejected_desc = SentinelPipelineDesc();
    const auto result = program.BuildPipelineDesc(DefaultRequest(), &rejected_desc);
    if (result.status != RenderShaderProgramStatus::DuplicateProgramId) {
        return Fail("shader program accepted duplicate program id");
    }

    if (!PipelineDescMatchesSentinel(rejected_desc)) {
        return Fail("shader program mutated output after duplicate id");
    }

    const auto snapshot = program.Snapshot();
    if (snapshot.duplicate_program_id_count != 1U) {
        return Fail("shader program snapshot missed duplicate id");
    }

    return 0;
}

int RenderCoreShaderProgramRejectsCapacityExceeded() {
    RenderShaderProgramDesc desc{};
    desc.program_record_capacity = 1U;
    RenderShaderProgram program(desc);

    RhiPipelineDesc pipeline_desc{};
    if (program.BuildPipelineDesc(DefaultRequest(), &pipeline_desc).status != RenderShaderProgramStatus::Success) {
        return Fail("shader program failed capacity setup");
    }

    RenderShaderProgramRequest second_request = DefaultRequest();
    second_request.program_id = 11U;
    RhiPipelineDesc rejected_desc = SentinelPipelineDesc();
    const auto result = program.BuildPipelineDesc(second_request, &rejected_desc);
    if (result.status != RenderShaderProgramStatus::ProgramCapacityExceeded) {
        return Fail("shader program accepted capacity overflow");
    }

    if (result.required_program_record_count != 2U) {
        return Fail("shader program capacity required count mismatch");
    }

    if (!PipelineDescMatchesSentinel(rejected_desc)) {
        return Fail("shader program mutated output after capacity overflow");
    }

    const auto snapshot = program.Snapshot();
    if (snapshot.last_required_program_record_count != 2U) {
        return Fail("shader program snapshot required count mismatch");
    }

    return 0;
}

int RenderCoreShaderProgramSnapshotTracksCounters() {
    RenderShaderProgram program;
    RhiPipelineDesc pipeline_desc{};
    if (program.BuildPipelineDesc(DefaultRequest(), &pipeline_desc).status != RenderShaderProgramStatus::Success) {
        return Fail("shader program failed snapshot setup");
    }

    RenderShaderProgramRequest request = DefaultRequest();
    request.program_id = 12U;
    request.pixel_shader = RhiShaderModuleHandle{};
    if (program.BuildPipelineDesc(request, &pipeline_desc).status != RenderShaderProgramStatus::InvalidPixelShader) {
        return Fail("shader program failed rejection setup");
    }

    const auto snapshot = program.Snapshot();
    if (snapshot.accepted_program_count != 1U || snapshot.failed_validation_count != 1U) {
        return Fail("shader program snapshot counters were not stable");
    }

    program.Reset();
    const auto reset_snapshot = program.Snapshot();
    if (reset_snapshot.program_record_count != 0U || reset_snapshot.accepted_program_count != 0U) {
        return Fail("shader program reset did not clear counters");
    }

    return 0;
}

int RenderCoreShaderProgramHasNarrowDependencyBoundary() {
    RenderShaderProgram program;
    RhiPipelineDesc pipeline_desc{};
    const auto result = program.BuildPipelineDesc(DefaultRequest(), &pipeline_desc);
    if (result.status != RenderShaderProgramStatus::Success) {
        return Fail("shader program dependency smoke setup failed");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_BUILDS_PIPELINE_DESC) {
        return RenderCoreShaderProgramBuildsPipelineDesc();
    }

    if (name == TEST_INVALID_VERTEX_SHADER) {
        return RenderCoreShaderProgramRejectsInvalidVertexShaderWithoutOutputMutation();
    }

    if (name == TEST_INVALID_INPUT_LAYOUT) {
        return RenderCoreShaderProgramRejectsInvalidInputLayoutWithoutOutputMutation();
    }

    if (name == TEST_DUPLICATE_ID) {
        return RenderCoreShaderProgramRejectsDuplicateProgramId();
    }

    if (name == TEST_CAPACITY) {
        return RenderCoreShaderProgramRejectsCapacityExceeded();
    }

    if (name == TEST_SNAPSHOT) {
        return RenderCoreShaderProgramSnapshotTracksCounters();
    }

    if (name == TEST_BOUNDARY) {
        return RenderCoreShaderProgramHasNarrowDependencyBoundary();
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
