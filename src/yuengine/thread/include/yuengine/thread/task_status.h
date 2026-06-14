#pragma once

namespace yuengine::thread {
enum class TASK_STATUS {
    Created,
    Queued,
    Running,
    Completed,
    Failed,
    Canceled,
    Rejected
};
}
