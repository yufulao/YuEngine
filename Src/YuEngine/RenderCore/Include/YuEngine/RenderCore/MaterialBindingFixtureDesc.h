// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/MaterialBindingFixtureDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/MaterialBindingFixtureConstants.h"

namespace yuengine::rendercore {
/**
 * @comment 配置 固定容量 material 绑定 fixture 存储.
 */
struct MaterialBindingFixtureDesc final {
    std::size_t binding_record_capacity = MAX_MATERIAL_BINDING_FIXTURE_RECORDS;
};
}
