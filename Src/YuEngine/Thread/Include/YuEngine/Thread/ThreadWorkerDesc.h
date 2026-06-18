// 模块: YuEngine Thread
// 文件: Src/YuEngine/Thread/Include/YuEngine/Thread/ThreadWorkerDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/Thread/ShutdownPolicy.h"

namespace yuengine::thread {
struct ThreadWorkerDesc {
    std::size_t work_capacity = 0U;
    std::size_t completion_capacity = 0U;
    ShutdownPolicy default_shutdown_policy = ShutdownPolicy::DrainQueued;
};
}
