// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/MaterialBindingFixtureConstants.h

#pragma once

#include <cstddef>

namespace yuengine::rendercore {
/**
 * @comment 最大数量： 保留的 material 绑定 fixture 记录.
 */
constexpr std::size_t MAX_MATERIAL_BINDING_FIXTURE_RECORDS = 8U;
/**
 * @comment 最大固定 constant payload 字节 复制 写入 一个 material 绑定 fixture 记录.
 */
constexpr std::size_t MAX_MATERIAL_BINDING_FIXTURE_CONSTANT_BYTES = 64U;
}
