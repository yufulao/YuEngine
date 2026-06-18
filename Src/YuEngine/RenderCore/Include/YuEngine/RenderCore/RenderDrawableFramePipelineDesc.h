// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawableFramePipelineDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/MaterialBindingFixtureDesc.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineConstants.h"
#include "YuEngine/RenderCore/RenderFixturePassDesc.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureDesc.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixtureDesc.h"

namespace yuengine::rendercore {
/**
 * @comment Describes bounded storage for drawable frame pipeline records and inner RenderCore steps.
 */
struct RenderDrawableFramePipelineDesc final {
    MaterialBindingFixtureDesc material_binding_desc{};
    RenderFixturePassDesc fixture_pass_desc{};
    RenderSubmissionBatchFixtureDesc submission_batch_desc{};
    RenderFramePacketFixtureDesc frame_packet_desc{};
    std::size_t frame_record_capacity = MAX_RENDER_DRAWABLE_FRAME_RECORDS;
};
}
