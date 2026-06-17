// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSubmissionBatchFixture.h

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
 * @comment Executes a bounded sequence of prepared RenderCore fixture pass requests.
 */
class RenderSubmissionBatchFixture final {
public:
    /**
     * @comment Constructs a RenderSubmissionBatchFixture instance.
     * @param desc Input descriptor.
     */
    explicit RenderSubmissionBatchFixture(
        const RenderSubmissionBatchFixtureDesc &desc=RenderSubmissionBatchFixtureDesc());

    /**
     * @comment Executes prepared fixture pass requests in deterministic order.
     * @param request Caller-owned submission batch request.
     * @return Explicit operation result.
     */
    RenderSubmissionBatchFixtureResult Execute(const RenderSubmissionBatchFixtureRequest &request);
    /**
     * @comment Returns the current submission batch fixture snapshot.
     * @return Snapshot value.
     */
    RenderSubmissionBatchFixtureSnapshot Snapshot() const;
    /**
     * @comment Resets bounded submission batch records and counters.
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
