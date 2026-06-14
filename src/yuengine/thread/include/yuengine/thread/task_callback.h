#pragma once

#include "yuengine/thread/task_status.h"

namespace yuengine::thread {
using TaskCallback = TaskStatus (*)(void* context);
}
