// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFixturePassConstants.h

#pragma once

#include <cstddef>

#include "YuEngine/Rhi/RhiConstants.h"

namespace yuengine::rendercore {
/**
 * @comment 最大数量： 保留的 fixture pass 记录.
 */
constexpr std::size_t MAX_RENDER_FIXTURE_PASS_RECORDS = 8U;
/**
 * @comment 所需 command 计数 用于 synthetic fixture pass。
 */
constexpr std::size_t RENDER_FIXTURE_PASS_COMMAND_COUNT = 9U;
/**
 * @comment 委托给 public RHI 命令列表的默认 command 容量。
 */
constexpr std::size_t DEFAULT_RENDER_FIXTURE_PASS_COMMAND_CAPACITY = yuengine::rhi::MAX_COMMANDS;
}
