// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderViewPacket.h

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
 * @comment Assembles one validated view-level RenderCore pass request from material and draw values.
 */
class RenderViewPacket final {
public:
    /**
     * @comment Constructs a RenderViewPacket instance.
     * @param desc Input descriptor.
     */
    explicit RenderViewPacket(const RenderViewPacketDesc &desc=RenderViewPacketDesc());

    /**
     * @comment Builds one caller-owned fixture pass request from a view packet request.
     * @param request Caller-owned view packet request.
     * @param out_request Caller-owned output pass request.
     * @return Explicit operation result.
     */
    RenderViewPacketResult BuildPassRequest(
        const RenderViewPacketRequest &request,
        RenderFixturePassRequest *out_request);
    /**
     * @comment Returns the current view packet snapshot.
     * @return Snapshot value.
     */
    RenderViewPacketSnapshot Snapshot() const;
    /**
     * @comment Resets bounded view packet records and counters.
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
