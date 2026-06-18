// 模块: YuEngine Thread
// 文件: Src/YuEngine/Thread/Include/YuEngine/Thread/ShutdownPolicy.h

#pragma once

namespace yuengine::thread {
enum class ShutdownPolicy {
    DrainQueued,
    CancelQueued
};
}
