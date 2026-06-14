#pragma once

#include <cstddef>
#include <cstdint>

#include "yuengine/input/input_accounting_status.h"
#include "yuengine/input/input_status.h"

namespace yuengine::input {
struct InputReplaySnapshot final {
    std::size_t DeviceCapacity = 0U;
    std::size_t ActionCapacity = 0U;
    std::size_t BindingCapacity = 0U;
    std::size_t ReplayFrameCapacity = 0U;
    std::size_t EventCapacityPerFrame = 0U;
    std::size_t ReplayStorageCapacityBeforeFrame = 0U;
    std::size_t ReplayStorageCapacityAfterLastFrame = 0U;
    std::size_t ActionCount = 0U;
    std::size_t BindingCount = 0U;
    std::size_t ChangedActionCount = 0U;
    std::uint64_t AcceptedEventCount = 0U;
    std::uint64_t RejectedEventCount = 0U;
    std::uint64_t ApplyCount = 0U;
    std::uint64_t ResetCount = 0U;
    std::uint64_t FailedOperationCount = 0U;
    InputStatus LastApplyStatus = InputStatus::Success;
    InputAccountingStatus AllocationAccountingStatus = InputAccountingStatus::DeferredUntilYuMemoryIntegration;
};
}
