// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSubmissionBatchFixture.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassRequest.h"
#include "YuEngine/RenderCore/RenderFixturePassResult.h"
#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureConstants.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureDesc.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureRequest.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureResult.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureSnapshot.h"

namespace yuengine::rendercore {
/**
 * @comment 执行 一个 固定容量 sequence 的 已准备 RenderCore fixture pass 请求s.
 */
class RenderSubmissionBatchFixture final {
public:
    /**
     * @comment 构造 RenderSubmissionBatchFixture 实例。
     * @param desc 输入描述。
     */
    explicit RenderSubmissionBatchFixture(
        const RenderSubmissionBatchFixtureDesc &desc=RenderSubmissionBatchFixtureDesc());

    /**
     * @comment 执行 已准备 fixture pass 请求s 在 确定性 order.
     * @param request 调用方持有的 submission batch 请求。
     * @return 显式操作结果。
     */
    RenderSubmissionBatchFixtureResult Execute(const RenderSubmissionBatchFixtureRequest &request);
    /**
     * @comment 返回当前 submission batch fixture 快照。
     * @return 快照值。
     */
    RenderSubmissionBatchFixtureSnapshot Snapshot() const;
    /**
     * @comment 重置固定容量 submission batch 记录和计数。
     */
    void Reset();

private:
    struct Record final {
        RenderFixturePassResult pass_result{};
        std::uint32_t material_id = 0U;
    };

    RenderSubmissionBatchFixtureStatus ValidateRequest(
        const RenderSubmissionBatchFixtureRequest &request,
        RenderSubmissionBatchFixtureResult *result) const;
    bool HasRecordCapacity(std::size_t entry_count) const;
    bool HasPassId(std::uint32_t pass_id) const;
    void RecordRejectedBatch(const RenderSubmissionBatchFixtureResult &result);
    void RecordRenderSuccess(
        const RenderFixturePassRequest &request,
        const RenderFixturePassResult &pass_result,
        std::size_t entry_index);
    void RecordRenderFailure(
        const RenderFixturePassRequest &request,
        const RenderFixturePassResult &pass_result,
        std::size_t entry_index,
        RenderSubmissionBatchFixtureResult *result);

    RenderSubmissionBatchFixtureDesc desc_;
    RenderSubmissionBatchFixtureSnapshot snapshot_;
    std::array<Record, MAX_RENDER_SUBMISSION_BATCH_FIXTURE_RECORDS> records_;
};
}
