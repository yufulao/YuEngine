// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFixturePassDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderFixturePassConstants.h"

namespace yuengine::rendercore {
/**
 * @comment Configures bounded storage and command capacity for a fixture pass executor.
 */
struct RenderFixturePassDesc final {
    std::size_t pass_record_capacity = MAX_RENDER_FIXTURE_PASS_RECORDS;
    std::size_t command_capacity = DEFAULT_RENDER_FIXTURE_PASS_COMMAND_CAPACITY;
};
}
