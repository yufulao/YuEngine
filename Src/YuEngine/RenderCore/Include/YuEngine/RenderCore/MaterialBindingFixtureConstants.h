// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/MaterialBindingFixtureConstants.h

#pragma once

#include <cstddef>

namespace yuengine::rendercore {
/**
 * @comment Maximum number of retained material binding fixture records.
 */
constexpr std::size_t MAX_MATERIAL_BINDING_FIXTURE_RECORDS = 8U;
/**
 * @comment Maximum fixed constant payload bytes copied into one material binding fixture record.
 */
constexpr std::size_t MAX_MATERIAL_BINDING_FIXTURE_CONSTANT_BYTES = 64U;
}
