// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderShaderProgram.h

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
 * @comment 构建 已验证 RHI pipeline 描述s 从 值-仅 shader program 请求s.
 */
class RenderShaderProgram final {
public:
    /**
     * @comment 构造 RenderShaderProgram 实例。
     * @param desc 输入描述。
     */
    explicit RenderShaderProgram(const RenderShaderProgramDesc &desc=RenderShaderProgramDesc());

    /**
     * @comment 验证 shader module 句柄s 和 writes 一个 调用方持有 pipeline 描述。
     * @param request 输入 shader program 请求。
     * @param out_desc 调用方持有的 pipeline 描述 output。
     * @return 显式操作结果。
     */
    RenderShaderProgramResult BuildPipelineDesc(
        const RenderShaderProgramRequest &request,
        yuengine::rhi::RhiPipelineDesc *out_desc);

    /**
     * @comment 返回当前 shader program 快照。
     * @return 快照值。
     */
    RenderShaderProgramSnapshot Snapshot() const;

    /**
     * @comment 重置固定容量 shader program 记录和计数。
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
