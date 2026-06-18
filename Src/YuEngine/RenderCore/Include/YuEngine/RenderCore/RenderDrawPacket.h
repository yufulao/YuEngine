// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawPacket.h

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
 * @comment Builds validated draw geometry fields for a RenderCore fixture pass request.
 */
class RenderDrawPacket final {
public:
    /**
     * @comment Constructs a RenderDrawPacket instance.
     * @param desc Input descriptor.
     */
    explicit RenderDrawPacket(const RenderDrawPacketDesc &desc=RenderDrawPacketDesc());

    /**
     * @comment Writes validated draw geometry into a caller-owned fixture pass request.
     * @param request Caller-owned draw packet request.
     * @param out_request Caller-owned output pass request.
     * @return Explicit operation result.
     */
    RenderDrawPacketResult BuildPassRequest(
        const RenderDrawPacketRequest &request,
        RenderFixturePassRequest *out_request);
    /**
     * @comment Returns the current draw packet snapshot.
     * @return Snapshot value.
     */
    RenderDrawPacketSnapshot Snapshot() const;
    /**
     * @comment Resets bounded draw packet records and counters.
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
