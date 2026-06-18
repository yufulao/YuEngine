// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderMaterialDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderMaterialConstants.h"

namespace yuengine::rendercore {
/**
 * @comment 描述 固定容量 RenderMaterial 存储.
 */
struct RenderMaterialDesc final {
    std::size_t material_record_capacity = MAX_RENDER_MATERIAL_RECORDS;
};
}
