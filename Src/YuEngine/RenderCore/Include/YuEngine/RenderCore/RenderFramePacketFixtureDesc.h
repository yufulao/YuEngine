// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFramePacketFixtureDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderFramePacketFixtureConstants.h"

namespace yuengine::rendercore {
/**
 * @comment Configures bounded storage for a RenderCore frame packet fixture.
 */
struct RenderFramePacketFixtureDesc final {
    std::size_t frame_packet_record_capacity = MAX_RENDER_FRAME_PACKET_FIXTURE_RECORDS;
};
}
