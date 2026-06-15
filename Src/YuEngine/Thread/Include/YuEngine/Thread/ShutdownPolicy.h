// Module: YuEngine Thread
// File: Src/YuEngine/Thread/Include/YuEngine/Thread/ShutdownPolicy.h

#pragma once

namespace yuengine::thread {
enum class ShutdownPolicy {
    DrainQueued,
    CancelQueued
};
}
