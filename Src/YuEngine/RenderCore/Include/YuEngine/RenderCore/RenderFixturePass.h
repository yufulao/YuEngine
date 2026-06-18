// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFixturePass.h

#pragma once

#include <array>

#include "YuEngine/RenderCore/RenderFixturePassConstants.h"
#include "YuEngine/RenderCore/RenderFixturePassDesc.h"
#include "YuEngine/RenderCore/RenderFixturePassRequest.h"
#include "YuEngine/RenderCore/RenderFixturePassResult.h"
#include "YuEngine/RenderCore/RenderFixturePassSnapshot.h"
#include "YuEngine/Rhi/RhiCommandList.h"

namespace yuengine::rendercore {
/**
 * @comment 在 public RHI contract 上执行一个固定容量 synthetic RenderCore fixture pass。
 */
class RenderFixturePass final {
public:
    /**
     * @comment 构造 RenderFixturePass 实例。
     * @param desc 输入描述。
     */
    explicit RenderFixturePass(const RenderFixturePassDesc &desc=RenderFixturePassDesc());

    /**
     * @comment 执行 一个 fixture pass 请求.
     * @param request 输入 pass 请求。
     * @return 显式操作结果。
     */
    RenderFixturePassResult Execute(const RenderFixturePassRequest &request);
    /**
     * @comment 返回当前 fixture pass 快照。
     * @return 快照值。
     */
    RenderFixturePassSnapshot Snapshot() const;
    /**
     * @comment 重置固定容量 pass 记录和计数。
     */
    void Reset();

private:
    RenderFixturePassStatus ValidateRequest(const RenderFixturePassRequest &request) const;
    bool HasRecordCapacity() const;
    void RecordRejectedResult(const RenderFixturePassResult &result);
    void RecordRhiFailureResult(RenderFixturePassResult *result);
    void RecordSuccessResult(const RenderFixturePassResult &result);

    RenderFixturePassDesc desc_;
    yuengine::rhi::RhiCommandList command_list_;
    RenderFixturePassSnapshot snapshot_;
    std::array<RenderFixturePassResult, MAX_RENDER_FIXTURE_PASS_RECORDS> records_;
};
}
