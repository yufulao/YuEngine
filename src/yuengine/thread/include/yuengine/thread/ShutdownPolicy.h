#pragma once

namespace yuengine::thread
{
enum class ShutdownPolicy
{
    DrainQueued,
    CancelQueued
};
}
