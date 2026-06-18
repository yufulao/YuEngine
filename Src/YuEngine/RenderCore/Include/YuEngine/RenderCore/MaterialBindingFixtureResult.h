// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/MaterialBindingFixtureResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/MaterialBindingFixtureStatus.h"
#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 结果 的 一个 material 绑定 fixture 操作。
 */
struct MaterialBindingFixtureResult final {
    MaterialBindingFixtureStatus status = MaterialBindingFixtureStatus::InvalidArgument;
    RenderFixturePassStatus pass_status = RenderFixturePassStatus::InvalidArgument;
    yuengine::rhi::RhiStatus rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
    std::uint32_t material_id = 0U;
    std::uint32_t pass_id = 0U;
    std::size_t constant_byte_count = 0U;
};
}
