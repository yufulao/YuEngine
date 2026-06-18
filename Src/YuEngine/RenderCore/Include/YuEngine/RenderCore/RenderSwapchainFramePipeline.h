// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSwapchainFramePipeline.h

#pragma once

#include <array>

#include "YuEngine/RenderCore/RenderSwapchainFramePipelineConstants.h"
#include "YuEngine/RenderCore/RenderSwapchainFramePipelineDesc.h"
#include "YuEngine/RenderCore/RenderSwapchainFramePipelineRequest.h"
#include "YuEngine/RenderCore/RenderSwapchainFramePipelineResult.h"
#include "YuEngine/RenderCore/RenderSwapchainFramePipelineSnapshot.h"
#include "YuEngine/RenderCore/RenderSwapchainFramePipelineStatus.h"
#include "YuEngine/Rhi/RhiCommandList.h"

namespace yuengine::rendercore {
/**
 * @comment Submits one bounded RenderCore frame to the current RHI swapchain backbuffer.
 */
class RenderSwapchainFramePipeline final {
public:
    /**
     * @comment Constructs a RenderSwapchainFramePipeline instance.
     * @param desc Input descriptor.
     */
    explicit RenderSwapchainFramePipeline(
        const RenderSwapchainFramePipelineDesc &desc=RenderSwapchainFramePipelineDesc());

    /**
     * @comment Executes one clear, submit, present, and capture operation against an RHI swapchain.
     * @param request Caller-owned frame request.
     * @return Explicit operation result.
     */
    RenderSwapchainFramePipelineResult Execute(const RenderSwapchainFramePipelineRequest &request);
    /**
     * @comment Returns the current frame pipeline snapshot.
     * @return Snapshot value.
     */
    RenderSwapchainFramePipelineSnapshot Snapshot() const;
    /**
     * @comment Resets bounded frame records and counters.
     */
    void Reset();

private:
    struct Record final {
        RenderSwapchainFramePipelineResult result{};
    };

    RenderSwapchainFramePipelineStatus ValidateRequest(
        const RenderSwapchainFramePipelineRequest &request) const;
    bool HasFrameId(std::uint32_t frame_id) const;
    bool HasRecordCapacity() const;
    void RecordRejectedResult(const RenderSwapchainFramePipelineResult &result);
    void RecordRhiFailureResult(const RenderSwapchainFramePipelineResult &result);
    void RecordSuccessResult(const RenderSwapchainFramePipelineResult &result);
    void StoreLastResult(const RenderSwapchainFramePipelineResult &result);

    RenderSwapchainFramePipelineDesc desc_;
    yuengine::rhi::RhiCommandList command_list_;
    RenderSwapchainFramePipelineSnapshot snapshot_;
    std::array<Record, MAX_RENDER_SWAPCHAIN_FRAME_RECORDS> records_;
};
}
