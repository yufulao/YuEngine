// 模块: YuEngine Thread
// 文件: Src/YuEngine/Thread/Include/YuEngine/Thread/TaskCallback.h

#pragma once

#include "YuEngine/Thread/TaskStatus.h"

namespace yuengine::thread {
using TaskCallback = TaskStatus (*)(void* context);
}
