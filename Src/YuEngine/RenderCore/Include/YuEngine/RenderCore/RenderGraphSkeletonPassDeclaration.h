// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeletonPassDeclaration.h

#pragma once

#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassRequest.h"

namespace yuengine::rendercore {
/**
 * @comment 声明 一个 调用方持有 pass 值 用于 一个 RenderCore render graph skeleton。
 */
struct RenderGraphSkeletonPassDeclaration final {
    std::uint32_t pass_id = 0U;
    RenderFixturePassRequest pass_request{};
};
}
