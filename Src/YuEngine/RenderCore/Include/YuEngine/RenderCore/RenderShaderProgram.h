// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderShaderProgram.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderShaderProgramConstants.h"
#include "YuEngine/RenderCore/RenderShaderProgramDesc.h"
#include "YuEngine/RenderCore/RenderShaderProgramRequest.h"
#include "YuEngine/RenderCore/RenderShaderProgramResult.h"
#include "YuEngine/RenderCore/RenderShaderProgramSnapshot.h"
#include "YuEngine/RenderCore/RenderShaderProgramStatus.h"
#include "YuEngine/Rhi/RhiPipelineDesc.h"

namespace yuengine::rendercore {
/**
 * @comment Builds validated RHI pipeline descriptors from value-only shader program requests.
 */
class RenderShaderProgram final {
public:
    /**
     * @comment Constructs a RenderShaderProgram instance.
     * @param desc Input descriptor.
     */
    explicit RenderShaderProgram(const RenderShaderProgramDesc &desc=RenderShaderProgramDesc());

    /**
     * @comment Validates shader module handles and writes a caller-owned pipeline descriptor.
     * @param request Input shader program request.
     * @param out_desc Caller-owned pipeline descriptor output.
     * @return Explicit operation result.
     */
    RenderShaderProgramResult BuildPipelineDesc(
        const RenderShaderProgramRequest &request,
        yuengine::rhi::RhiPipelineDesc *out_desc);

    /**
     * @comment Returns the current shader program snapshot.
     * @return Snapshot value.
     */
    RenderShaderProgramSnapshot Snapshot() const;

    /**
     * @comment Resets bounded shader program records and counters.
     */
    void Reset();

private:
    struct Record final {
        std::uint32_t program_id = 0U;
        yuengine::rhi::RhiPipelineDesc pipeline_desc{};
    };

    RenderShaderProgramStatus ValidateRequest(const RenderShaderProgramRequest &request) const;
    bool HasRecordCapacity() const;
    bool HasProgramId(std::uint32_t program_id) const;
    yuengine::rhi::RhiPipelineDesc MakePipelineDesc(const RenderShaderProgramRequest &request) const;
    void RecordAcceptedProgram(
        const RenderShaderProgramRequest &request,
        const yuengine::rhi::RhiPipelineDesc &pipeline_desc,
        RenderShaderProgramResult *result);
    void RecordRejectedProgram(const RenderShaderProgramResult &result);

    RenderShaderProgramDesc desc_;
    RenderShaderProgramSnapshot snapshot_;
    std::array<Record, MAX_RENDER_SHADER_PROGRAM_RECORDS> records_;
};
}
