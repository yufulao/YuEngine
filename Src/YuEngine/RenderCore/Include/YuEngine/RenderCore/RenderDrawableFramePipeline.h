// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawableFramePipeline.h

#pragma once

#include <array>

#include "YuEngine/RenderCore/RenderDrawableFramePipelineConstants.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineDesc.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineRequest.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineResult.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineSnapshot.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineStatus.h"
#include "YuEngine/RenderCore/RenderFixturePass.h"
#include "YuEngine/RenderCore/RenderFramePacketFixture.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixture.h"
#include "YuEngine/RenderCore/RenderViewPacket.h"

namespace yuengine::rendercore {
/**
 * @comment 通过 material、submission、frame packet 和 RHI swapchain 路径执行一个 drawable RenderCore 帧。
 */
class RenderDrawableFramePipeline final {
public:
    /**
     * @comment 构造 RenderDrawableFramePipeline 实例。
     * @param desc 输入描述。
     */
    explicit RenderDrawableFramePipeline(
        const RenderDrawableFramePipelineDesc &desc=RenderDrawableFramePipelineDesc());

    /**
     * @comment 针对当前 RHI swapchain backbuffer 执行一个 drawable frame。
     * @param request 调用方持有的 帧请求。
     * @return 显式操作结果。
     */
    RenderDrawableFramePipelineResult Execute(const RenderDrawableFramePipelineRequest &request);
    /**
     * @comment 返回当前 drawable frame pipeline 快照。
     * @return 快照值。
     */
    RenderDrawableFramePipelineSnapshot Snapshot() const;
    /**
     * @comment 重置 固定容量 drawable frame 记录 和 inner RenderCore 状态。
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
    void RecordViewPacketFailureResult(const RenderDrawableFramePipelineResult &result);
    void RecordFrameFailureResult(const RenderDrawableFramePipelineResult &result);
    void RecordSuccessResult(const RenderDrawableFramePipelineResult &result);
    void StoreLastResult(const RenderDrawableFramePipelineResult &result);

    RenderDrawableFramePipelineDesc desc_;
    RenderViewPacket view_packet_;
    RenderFixturePass fixture_pass_;
    RenderSubmissionBatchFixture submission_batch_;
    RenderFramePacketFixture frame_packet_;
    RenderDrawableFramePipelineSnapshot snapshot_;
    std::array<Record, MAX_RENDER_DRAWABLE_FRAME_RECORDS> records_;
};
}
