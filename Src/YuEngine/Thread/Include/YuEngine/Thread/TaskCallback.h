#pragma once

#include "YuEngine/Thread/TaskStatus.h"

namespace yuengine::thread {
using TaskCallback = TaskStatus (*)(void* context);
}
