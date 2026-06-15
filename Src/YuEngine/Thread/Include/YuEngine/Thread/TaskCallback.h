// Module: YuEngine Thread
// File: Src/YuEngine/Thread/Include/YuEngine/Thread/TaskCallback.h

#pragma once

#include "YuEngine/Thread/TaskStatus.h"

namespace yuengine::thread {
using TaskCallback = TaskStatus (*)(void* context);
}
