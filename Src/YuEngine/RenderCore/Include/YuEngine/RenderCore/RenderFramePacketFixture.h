// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFramePacketFixture.h

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
 * @comment Executes one prepared RenderCore submission batch inside a bounded frame packet envelope.
 */
class RenderFramePacketFixture final {
public:
    /**
     * @comment Constructs a RenderFramePacketFixture instance.
     * @param desc Input descriptor.
     */
    explicit RenderFramePacketFixture(
        const RenderFramePacketFixtureDesc &desc=RenderFramePacketFixtureDesc());

    /**
     * @comment Executes a prepared submission batch as one deterministic frame packet.
     * @param request Caller-owned frame packet request.
     * @return Explicit operation result.
     */
    RenderFramePacketFixtureResult Execute(const RenderFramePacketFixtureRequest &request);
    /**
     * @comment Returns the current frame packet fixture snapshot.
     * @return Snapshot value.
     */
    RenderFramePacketFixtureSnapshot Snapshot() const;
    /**
     * @comment Resets bounded frame packet records and counters.
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
