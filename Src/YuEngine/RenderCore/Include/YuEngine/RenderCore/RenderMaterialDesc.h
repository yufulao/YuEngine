// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderMaterialDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderMaterialConstants.h"

namespace yuengine::rendercore {
/**
 * @comment Describes bounded RenderMaterial storage.
 */
struct RenderMaterialDesc final {
    std::size_t material_record_capacity = MAX_RENDER_MATERIAL_RECORDS;
};
}
