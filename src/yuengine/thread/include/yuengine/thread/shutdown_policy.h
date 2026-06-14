#pragma once

namespace yuengine::thread {
enum class SHUTDOWN_POLICY {
    DrainQueued,
    CancelQueued
};
}
