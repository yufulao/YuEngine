// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawableFramePipeline.h

#pragma once

#include <array>

#include "YuEngine/RenderCore/MaterialBindingFixture.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineConstants.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineDesc.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineRequest.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineResult.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineSnapshot.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineStatus.h"
#include "YuEngine/RenderCore/RenderFixturePass.h"
#include "YuEngine/RenderCore/RenderFramePacketFixture.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixture.h"

namespace yuengine::rendercore {
/**
 * @comment Executes one drawable RenderCore frame through material, submission, frame packet, and RHI swapchain paths.
 */
class RenderDrawableFramePipeline final {
public:
    /**
     * @comment Constructs a RenderDrawableFramePipeline instance.
     * @param desc Input descriptor.
     */
    explicit RenderDrawableFramePipeline(
        const RenderDrawableFramePipelineDesc &desc=RenderDrawableFramePipelineDesc());

    /**
     * @comment Executes one drawable frame against the current RHI swapchain backbuffer.
     * @param request Caller-owned frame request.
     * @return Explicit operation result.
     */
    RenderDrawableFramePipelineResult Execute(const RenderDrawableFramePipelineRequest &request);
    /**
     * @comment Returns the current drawable frame pipeline snapshot.
     * @return Snapshot value.
     */
    RenderDrawableFramePipelineSnapshot Snapshot() const;
    /**
     * @comment Resets bounded drawable frame records and inner RenderCore state.
     */
    void Reset();

private:
    struct Record final {
        RenderDrawableFramePipelineResult result{};
    };

    RenderDrawableFramePipelineStatus ValidateRequest(
        const RenderDrawableFramePipelineRequest &request) const;
    bool HasRecordCapacity() const;
    void RecordRejectedResult(const RenderDrawableFramePipelineResult &result);
    void RecordRhiFailureResult(const RenderDrawableFramePipelineResult &result);
    void RecordMaterialFailureResult(const RenderDrawableFramePipelineResult &result);
    void RecordFrameFailureResult(const RenderDrawableFramePipelineResult &result);
    void RecordSuccessResult(const RenderDrawableFramePipelineResult &result);
    void StoreLastResult(const RenderDrawableFramePipelineResult &result);

    RenderDrawableFramePipelineDesc desc_;
    MaterialBindingFixture material_binding_;
    RenderFixturePass fixture_pass_;
    RenderSubmissionBatchFixture submission_batch_;
    RenderFramePacketFixture frame_packet_;
    RenderDrawableFramePipelineSnapshot snapshot_;
    std::array<Record, MAX_RENDER_DRAWABLE_FRAME_RECORDS> records_;
};
}
