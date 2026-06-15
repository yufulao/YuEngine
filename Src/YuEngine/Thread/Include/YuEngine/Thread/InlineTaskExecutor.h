// Module: YuEngine Thread
// File: Src/YuEngine/Thread/Include/YuEngine/Thread/InlineTaskExecutor.h

#pragma once

#include "YuEngine/Thread/TaskCallback.h"
#include "YuEngine/Thread/TaskStatus.h"

namespace yuengine::thread {
class InlineTaskExecutor final {
public:
    /**
     * @comment Executes queued work.
     * @param callback Input callback.
     * @param context Input context.
     * @return Explicit operation status.
     */
    TaskStatus Execute(TaskCallback callback, void* context);
};
}
