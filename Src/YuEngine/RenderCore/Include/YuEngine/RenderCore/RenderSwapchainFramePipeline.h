// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSwapchainFramePipeline.h

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
 * @comment 提交 一个 固定容量 RenderCore frame 到 当前 RHI swapchain backbuffer。
 */
class RenderSwapchainFramePipeline final {
public:
    /**
     * @comment 构造 RenderSwapchainFramePipeline 实例。
     * @param desc 输入描述。
     */
    explicit RenderSwapchainFramePipeline(
        const RenderSwapchainFramePipelineDesc &desc=RenderSwapchainFramePipelineDesc());

    /**
     * @comment 对 RHI swapchain 执行一次 clear、submit、呈现 和 capture 操作。
     * @param request 调用方持有的 帧请求。
     * @return 显式操作结果。
     */
    RenderSwapchainFramePipelineResult Execute(const RenderSwapchainFramePipelineRequest &request);
    /**
     * @comment 返回当前 frame pipeline 快照。
     * @return 快照值。
     */
    RenderSwapchainFramePipelineSnapshot Snapshot() const;
    /**
     * @comment 重置固定容量 frame 记录和计数。
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
