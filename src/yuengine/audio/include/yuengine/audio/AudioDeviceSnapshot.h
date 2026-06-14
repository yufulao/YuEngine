#pragma once

#include <cstddef>
#include <cstdint>

#include "yuengine/audio/AudioAccountingStatus.h"

namespace yuengine::audio {
struct AudioDeviceSnapshot final {
    std::size_t SourceCapacity = 0U;
    std::size_t VoiceCapacity = 0U;
    std::size_t SourceCount = 0U;
    std::size_t ActiveVoiceCount = 0U;
    std::size_t VoiceStorageCapacityBeforeMix = 0U;
    std::size_t VoiceStorageCapacityAfterLastMix = 0U;
    std::uint64_t RegisteredSourceCount = 0U;
    std::uint64_t StartedVoiceCount = 0U;
    std::uint64_t StoppedVoiceCount = 0U;
    std::uint64_t MixedFrameCount = 0U;
    std::uint64_t OutputSampleWriteCount = 0U;
    std::uint64_t FailedOperationCount = 0U;
    std::size_t LastFramesWritten = 0U;
    AudioAccountingStatus AllocationAccountingStatus = AudioAccountingStatus::DeferredUntilYuMemoryIntegration;
};
}
