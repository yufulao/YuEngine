// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Src/RenderShaderProgram.cpp

#include "YuEngine/RenderCore/RenderShaderProgram.h"

#include <cstddef>

namespace yuengine::rendercore {
namespace {
RenderShaderProgramDesc NormalizeDesc(RenderShaderProgramDesc desc) {
    if (desc.program_record_capacity > MAX_RENDER_SHADER_PROGRAM_RECORDS) {
        desc.program_record_capacity = MAX_RENDER_SHADER_PROGRAM_RECORDS;
    }

    return desc;
}

bool IsShaderHandleSet(yuengine::rhi::RhiShaderModuleHandle handle) {
    return handle.generation != 0U;
}

std::size_t GetInputElementByteSize(yuengine::rhi::RhiInputElementFormat format) {
    switch (format) {
    case yuengine::rhi::RhiInputElementFormat::Float32x2:
        return 8U;
    case yuengine::rhi::RhiInputElementFormat::Float32x3:
        return 12U;
    case yuengine::rhi::RhiInputElementFormat::Float32x4:
        return 16U;
    case yuengine::rhi::RhiInputElementFormat::Unsupported:
        break;
    default:
        break;
    }

    return 0U;
}

bool IsInputElementValid(
    const yuengine::rhi::RhiInputElementDesc &element,
    std::size_t stride_bytes) {
    if (element.semantic == yuengine::rhi::RhiInputElementSemantic::Unsupported) {
        return false;
    }

    const std::size_t element_size = GetInputElementByteSize(element.format);
    if (element_size == 0U) {
        return false;
    }

    if (stride_bytes < element_size) {
        return false;
    }

    const std::size_t max_offset = stride_bytes - element_size;
    if (element.offset_bytes > max_offset) {
        return false;
    }

    return true;
}

bool IsInputLayoutValid(const yuengine::rhi::RhiInputLayoutDesc &layout) {
    if (layout.element_count == 0U) {
        return false;
    }

    if (layout.element_count > yuengine::rhi::MAX_RHI_INPUT_ELEMENTS) {
        return false;
    }

    if (layout.stride_bytes == 0U) {
        return false;
    }

    for (std::size_t index = 0U; index < layout.element_count; ++index) {
        if (!IsInputElementValid(layout.elements[index], layout.stride_bytes)) {
            return false;
        }
    }

    return true;
}
}

RenderShaderProgram::RenderShaderProgram(const RenderShaderProgramDesc &desc)
    : desc_(NormalizeDesc(desc)) {
    Reset();
}

RenderShaderProgramResult RenderShaderProgram::BuildPipelineDesc(
    const RenderShaderProgramRequest &request,
    yuengine::rhi::RhiPipelineDesc *out_desc) {
    RenderShaderProgramResult result{};
    result.program_id = request.program_id;

    if (out_desc == nullptr) {
        result.status = RenderShaderProgramStatus::InvalidArgument;
        RecordRejectedProgram(result);
        return result;
    }

    result.status = ValidateRequest(request);
    if (result.status != RenderShaderProgramStatus::Success) {
        RecordRejectedProgram(result);
        return result;
    }

    if (HasProgramId(request.program_id)) {
        result.status = RenderShaderProgramStatus::DuplicateProgramId;
        RecordRejectedProgram(result);
        return result;
    }

    if (!HasRecordCapacity()) {
        result.status = RenderShaderProgramStatus::ProgramCapacityExceeded;
        RecordRejectedProgram(result);
        return result;
    }

    const yuengine::rhi::RhiPipelineDesc pipeline_desc = MakePipelineDesc(request);
    RecordAcceptedProgram(request, pipeline_desc, &result);
    *out_desc = pipeline_desc;
    return result;
}

RenderShaderProgramSnapshot RenderShaderProgram::Snapshot() const {
    return snapshot_;
}

void RenderShaderProgram::Reset() {
    records_ = {};
    snapshot_ = {};
    snapshot_.program_record_capacity = desc_.program_record_capacity;
}

RenderShaderProgramStatus RenderShaderProgram::ValidateRequest(
    const RenderShaderProgramRequest &request) const {
    if (request.program_id == 0U) {
        return RenderShaderProgramStatus::InvalidProgramId;
    }

    if (!IsShaderHandleSet(request.vertex_shader)) {
        return RenderShaderProgramStatus::InvalidVertexShader;
    }

    if (!IsShaderHandleSet(request.pixel_shader)) {
        return RenderShaderProgramStatus::InvalidPixelShader;
    }

    if (!IsInputLayoutValid(request.input_layout)) {
        return RenderShaderProgramStatus::InvalidInputLayout;
    }

    return RenderShaderProgramStatus::Success;
}

bool RenderShaderProgram::HasRecordCapacity() const {
    return snapshot_.program_record_count < desc_.program_record_capacity;
}

bool RenderShaderProgram::HasProgramId(std::uint32_t program_id) const {
    for (std::size_t index = 0U; index < snapshot_.program_record_count; ++index) {
        if (records_[index].program_id == program_id) {
            return true;
        }
    }

    return false;
}

yuengine::rhi::RhiPipelineDesc RenderShaderProgram::MakePipelineDesc(
    const RenderShaderProgramRequest &request) const {
    yuengine::rhi::RhiPipelineDesc pipeline_desc{};
    pipeline_desc.vertex_shader = request.vertex_shader;
    pipeline_desc.pixel_shader = request.pixel_shader;
    pipeline_desc.input_layout = request.input_layout;
    return pipeline_desc;
}

void RenderShaderProgram::RecordAcceptedProgram(
    const RenderShaderProgramRequest &request,
    const yuengine::rhi::RhiPipelineDesc &pipeline_desc,
    RenderShaderProgramResult *result) {
    if (result == nullptr) {
        return;
    }

    Record record{};
    record.program_id = request.program_id;
    record.pipeline_desc = pipeline_desc;

    if (snapshot_.program_record_count < records_.size()) {
        records_[snapshot_.program_record_count] = record;
    }

    ++snapshot_.program_record_count;
    ++snapshot_.accepted_program_count;
    snapshot_.last_program_id = request.program_id;
    snapshot_.last_status = RenderShaderProgramStatus::Success;

    result->status = RenderShaderProgramStatus::Success;
}

void RenderShaderProgram::RecordRejectedProgram(const RenderShaderProgramResult &result) {
    snapshot_.last_program_id = result.program_id;
    snapshot_.last_status = result.status;

    if (result.status == RenderShaderProgramStatus::DuplicateProgramId) {
        ++snapshot_.duplicate_program_id_count;
        return;
    }

    if (result.status == RenderShaderProgramStatus::ProgramCapacityExceeded) {
        ++snapshot_.program_capacity_rejected_count;
        return;
    }

    ++snapshot_.failed_validation_count;
}
}
