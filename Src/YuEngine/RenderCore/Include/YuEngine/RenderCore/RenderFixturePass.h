// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFixturePass.h

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
 * @comment Executes a bounded synthetic RenderCore fixture pass over public RHI contracts.
 */
class RenderFixturePass final {
public:
    /**
     * @comment Constructs a RenderFixturePass instance.
     * @param desc Input descriptor.
     */
    explicit RenderFixturePass(const RenderFixturePassDesc &desc=RenderFixturePassDesc());

    /**
     * @comment Executes one fixture pass request.
     * @param request Input pass request.
     * @return Explicit operation result.
     */
    RenderFixturePassResult Execute(const RenderFixturePassRequest &request);
    /**
     * @comment Returns the current fixture pass snapshot.
     * @return Snapshot value.
     */
    RenderFixturePassSnapshot Snapshot() const;
    /**
     * @comment Resets bounded pass records and counters.
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
