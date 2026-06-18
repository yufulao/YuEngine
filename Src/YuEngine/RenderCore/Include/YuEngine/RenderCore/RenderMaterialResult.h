// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderMaterialResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderMaterialStatus.h"

namespace yuengine::rendercore {
/**
 * @comment Contains the result of one render material operation.
 */
struct RenderMaterialResult final {
    RenderMaterialStatus status = RenderMaterialStatus::InvalidArgument;
    std::uint32_t material_id = 0U;
    std::uint32_t program_id = 0U;
    std::uint32_t pass_id = 0U;
    std::size_t constant_byte_count = 0U;
};
}
