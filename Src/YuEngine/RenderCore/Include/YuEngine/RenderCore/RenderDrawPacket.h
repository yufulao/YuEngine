// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawPacket.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderDrawPacketConstants.h"
#include "YuEngine/RenderCore/RenderDrawPacketDesc.h"
#include "YuEngine/RenderCore/RenderDrawPacketRequest.h"
#include "YuEngine/RenderCore/RenderDrawPacketResult.h"
#include "YuEngine/RenderCore/RenderDrawPacketSnapshot.h"
#include "YuEngine/RenderCore/RenderDrawPacketStatus.h"
#include "YuEngine/RenderCore/RenderFixturePassRequest.h"

namespace yuengine::rendercore {
/**
 * @comment 构建 已验证 draw geometry fields 用于 一个 RenderCore fixture pass 请求.
 */
class RenderDrawPacket final {
public:
    /**
     * @comment 构造 RenderDrawPacket 实例。
     * @param desc 输入描述。
     */
    explicit RenderDrawPacket(const RenderDrawPacketDesc &desc=RenderDrawPacketDesc());

    /**
     * @comment 写入 已验证 draw geometry 写入 一个 调用方持有 fixture pass 请求。
     * @param request 调用方持有的 draw packet 请求。
     * @param out_request 调用方持有的 输出 pass 请求。
     * @return 显式操作结果。
     */
    RenderDrawPacketResult BuildPassRequest(
        const RenderDrawPacketRequest &request,
        RenderFixturePassRequest *out_request);
    /**
     * @comment 返回当前 draw packet 快照。
     * @return 快照值。
     */
    RenderDrawPacketSnapshot Snapshot() const;
    /**
     * @comment 重置固定容量 draw packet 记录和计数。
     */
    void Reset();

private:
    struct Record final {
        std::uint32_t draw_id = 0U;
        std::uint32_t pass_id = 0U;
        std::uint32_t material_id = 0U;
        yuengine::rhi::RhiVertexBufferView vertex_buffer{};
        yuengine::rhi::RhiIndexBufferView index_buffer{};
        yuengine::rhi::RhiDrawIndexedDesc draw{};
    };

    RenderDrawPacketStatus ValidateRequest(const RenderDrawPacketRequest &request) const;
    bool HasRecordCapacity() const;
    bool HasDrawId(std::uint32_t draw_id) const;
    void FillPassRequest(
        const RenderDrawPacketRequest &request,
        RenderFixturePassRequest *out_request) const;
    void RecordAcceptedDraw(
        const RenderDrawPacketRequest &request,
        RenderDrawPacketResult *result);
    void RecordRejectedDraw(const RenderDrawPacketResult &result);

    RenderDrawPacketDesc desc_;
    RenderDrawPacketSnapshot snapshot_;
    std::array<Record, MAX_RENDER_DRAW_PACKET_RECORDS> records_;
};
}
