// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSubmissionBatchFixtureStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment Defines explicit RenderCore submission batch fixture status values.
 */
enum class RenderSubmissionBatchFixtureStatus {
    Success,
    InvalidArgument,
    EmptyBatch,
    InvalidRequestStorage,
    InvalidResultStorage,
    InvalidPassRequest,
    DuplicatePassId,
    BatchCapacityExceeded,
    RenderFixturePassFailed
};
}
