// 模块: YuEngine Serialize
// 文件: Src/YuEngine/Serialize/Include/YuEngine/Serialize/RuntimeProfileBoundary.h

#pragma once

#include <cstdint>

#include "YuEngine/Serialize/RuntimeProfileBoundaryKind.h"

namespace yuengine::serialize {
struct RuntimeProfileBoundary final {
    std::uint32_t profile_id = 0U;
    std::uint32_t slot_id = 0U;
    RuntimeProfileBoundaryKind kind = RuntimeProfileBoundaryKind::RuntimeConfig;
    std::uint32_t caller_policy_tag = 0U;
};
}
