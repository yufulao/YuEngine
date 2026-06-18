// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderMaterialConstants.h

#pragma once

#include <cstddef>

namespace yuengine::rendercore {
/**
 * @comment 最大数量： 保留的 render material 记录.
 */
constexpr std::size_t MAX_RENDER_MATERIAL_RECORDS = 16U;
/**
 * @comment 最大固定 material constant 字节 复制 写入 一个 render material 记录.
 */
constexpr std::size_t MAX_RENDER_MATERIAL_CONSTANT_BYTES = 64U;
}
