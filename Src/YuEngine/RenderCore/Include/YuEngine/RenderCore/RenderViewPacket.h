// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderViewPacket.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassRequest.h"
#include "YuEngine/RenderCore/RenderViewPacketConstants.h"
#include "YuEngine/RenderCore/RenderViewPacketDesc.h"
#include "YuEngine/RenderCore/RenderViewPacketRequest.h"
#include "YuEngine/RenderCore/RenderViewPacketResult.h"
#include "YuEngine/RenderCore/RenderViewPacketSnapshot.h"
#include "YuEngine/RenderCore/RenderViewPacketStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 组装 一个 已验证 视图-level RenderCore pass 请求 从 material 和 draw 值.
 */
class RenderViewPacket final {
public:
    /**
     * @comment 构造 RenderViewPacket 实例。
     * @param desc 输入描述。
     */
    explicit RenderViewPacket(const RenderViewPacketDesc &desc=RenderViewPacketDesc());

    /**
     * @comment 构建 一个 调用方持有 fixture pass 请求 从 一个 视图 packet 请求.
     * @param request 调用方持有的 视图 packet 请求。
     * @param out_request 调用方持有的 输出 pass 请求。
     * @return 显式操作结果。
     */
    RenderViewPacketResult BuildPassRequest(
        const RenderViewPacketRequest &request,
        RenderFixturePassRequest *out_request);
    /**
     * @comment 返回当前 视图 packet 快照。
     * @return 快照值。
     */
    RenderViewPacketSnapshot Snapshot() const;
    /**
     * @comment 重置固定容量 视图 packet 记录和计数。
     */
    void Reset();

private:
    struct Record final {
        RenderViewPacketResult result{};
    };

    RenderViewPacketStatus ValidateRequest(
        const RenderViewPacketRequest &request,
        RenderViewPacketResult *result) const;
    bool HasRecordCapacity() const;
    bool HasViewId(std::uint32_t view_id) const;
    void FillPassRequest(
        const RenderViewPacketRequest &request,
        RenderFixturePassRequest *out_request) const;
    void RecordAcceptedView(RenderViewPacketResult *result);
    void RecordRejectedView(const RenderViewPacketResult &result);

    RenderViewPacketDesc desc_;
    RenderViewPacketSnapshot snapshot_;
    std::array<Record, MAX_RENDER_VIEW_PACKET_RECORDS> records_;
};
}
