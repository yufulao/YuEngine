// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderMaterialConstants.h

#pragma once

#include <cstddef>

namespace yuengine::rendercore {
/**
 * @comment Maximum number of retained render material records.
 */
constexpr std::size_t MAX_RENDER_MATERIAL_RECORDS = 16U;
/**
 * @comment Maximum fixed material constant bytes copied into one render material record.
 */
constexpr std::size_t MAX_RENDER_MATERIAL_CONSTANT_BYTES = 64U;
}
