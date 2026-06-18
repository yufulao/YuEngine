// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFramePacketFixture.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderFramePacketFixtureConstants.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureDesc.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureRequest.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureResult.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureSnapshot.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 执行 一个 已准备 RenderCore submission batch inside 一个 固定容量 frame packet envelope.
 */
class RenderFramePacketFixture final {
public:
    /**
     * @comment 构造 RenderFramePacketFixture 实例。
     * @param desc 输入描述。
     */
    explicit RenderFramePacketFixture(
        const RenderFramePacketFixtureDesc &desc=RenderFramePacketFixtureDesc());

    /**
     * @comment 将准备好的 submission batch 作为确定性的 frame packet 执行。
     * @param request 调用方持有的 frame packet 请求。
     * @return 显式操作结果。
     */
    RenderFramePacketFixtureResult Execute(const RenderFramePacketFixtureRequest &request);
    /**
     * @comment 返回当前 frame packet fixture 快照。
     * @return 快照值。
     */
    RenderFramePacketFixtureSnapshot Snapshot() const;
    /**
     * @comment 重置固定容量 frame packet 记录和计数。
     */
    void Reset();

private:
    struct Record final {
        RenderFramePacketFixtureResult result{};
    };

    RenderFramePacketFixtureStatus ValidateRequest(
        const RenderFramePacketFixtureRequest &request,
        RenderFramePacketFixtureResult *result) const;
    bool HasRecordCapacity() const;
    bool HasFrameId(std::uint32_t frame_id) const;
    void RecordRejectedPacket(const RenderFramePacketFixtureResult &result);
    void RecordCompletedPacket(const RenderFramePacketFixtureResult &result);
    void RecordSubmissionBatchFailure(const RenderFramePacketFixtureResult &result);

    RenderFramePacketFixtureDesc desc_;
    RenderFramePacketFixtureSnapshot snapshot_;
    std::array<Record, MAX_RENDER_FRAME_PACKET_FIXTURE_RECORDS> records_;
};
}
