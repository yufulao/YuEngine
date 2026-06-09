#pragma once

#include "yuengine/thread/TaskStatus.h"

namespace yuengine::thread
{
using TaskCallback = TaskStatus (*)(void* context);
}
