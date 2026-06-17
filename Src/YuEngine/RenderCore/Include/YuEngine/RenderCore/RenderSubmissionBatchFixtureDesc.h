// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSubmissionBatchFixtureDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureConstants.h"

namespace yuengine::rendercore {
/**
 * @comment Configures bounded storage for a RenderCore submission batch fixture.
 */
struct RenderSubmissionBatchFixtureDesc final {
    std::size_t submission_record_capacity = MAX_RENDER_SUBMISSION_BATCH_FIXTURE_RECORDS;
};
}
