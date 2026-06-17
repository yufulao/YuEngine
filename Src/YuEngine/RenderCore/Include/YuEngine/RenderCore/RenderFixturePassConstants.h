// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFixturePassConstants.h

#pragma once

#include <cstddef>

#include "YuEngine/Rhi/RhiConstants.h"

namespace yuengine::rendercore {
/**
 * @comment Maximum number of retained fixture pass records.
 */
constexpr std::size_t MAX_RENDER_FIXTURE_PASS_RECORDS = 8U;
/**
 * @comment Required command count for the synthetic fixture pass.
 */
constexpr std::size_t RENDER_FIXTURE_PASS_COMMAND_COUNT = 9U;
/**
 * @comment Default command capacity delegated to the public RHI command list.
 */
constexpr std::size_t DEFAULT_RENDER_FIXTURE_PASS_COMMAND_CAPACITY = yuengine::rhi::MAX_COMMANDS;
}
