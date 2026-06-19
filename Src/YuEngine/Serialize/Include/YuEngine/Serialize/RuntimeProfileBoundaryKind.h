// 模块: YuEngine Serialize
// 文件: Src/YuEngine/Serialize/Include/YuEngine/Serialize/RuntimeProfileBoundaryKind.h

#pragma once

#include <cstdint>

namespace yuengine::serialize {
enum class RuntimeProfileBoundaryKind : std::uint32_t {
    RuntimeConfig = 1U,
    SaveSnapshot = 2U,
    ProfileSnapshot = 3U
};
}
