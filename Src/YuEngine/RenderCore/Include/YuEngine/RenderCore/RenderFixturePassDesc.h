// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFixturePassDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderFixturePassConstants.h"

namespace yuengine::rendercore {
/**
 * @comment 配置 固定容量 存储 和 command 容量 用于 一个 fixture pass executor.
 */
struct RenderFixturePassDesc final {
    std::size_t pass_record_capacity = MAX_RENDER_FIXTURE_PASS_RECORDS;
    std::size_t command_capacity = DEFAULT_RENDER_FIXTURE_PASS_COMMAND_CAPACITY;
};
}
