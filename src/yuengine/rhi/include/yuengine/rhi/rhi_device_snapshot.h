#pragma once

#include <cstddef>
#include <cstdint>

#include "yuengine/rhi/rhi_accounting_status.h"

namespace yuengine::rhi {
struct RhiDeviceSnapshot final {
    std::size_t ColorTargetCapacity = 0U;
    std::size_t ColorTargetCount = 0U;
    std::size_t CommandStorageCapacityBeforeFrame = 0U;
    std::size_t CommandStorageCapacityAfterLastFrame = 0U;
    std::uint64_t CreatedTargetCount = 0U;
    std::uint64_t DestroyedTargetCount = 0U;
    std::uint64_t RecordedCommandCount = 0U;
    std::uint64_t SubmitCount = 0U;
    std::uint64_t PresentCount = 0U;
    std::uint64_t CaptureCount = 0U;
    std::uint64_t FailedOperationCount = 0U;
    std::size_t LastCaptureBytesWritten = 0U;
    RHI_ACCOUNTING_STATUS AllocationAccountingStatus = RHI_ACCOUNTING_STATUS::DeferredUntilYuMemoryIntegration;
};
}
