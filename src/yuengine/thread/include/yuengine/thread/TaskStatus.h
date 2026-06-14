#pragma once

namespace yuengine::thread {
enum class TaskStatus {
    Created,
    Queued,
    Running,
    Completed,
    Failed,
    Canceled,
    Rejected
};
}
