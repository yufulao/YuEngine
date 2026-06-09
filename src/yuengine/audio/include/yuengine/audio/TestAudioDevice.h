#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "yuengine/audio/AudioCapabilities.h"
#include "yuengine/audio/AudioDeviceDesc.h"
#include "yuengine/audio/AudioDeviceSnapshot.h"
#include "yuengine/audio/AudioMixResult.h"
#include "yuengine/audio/AudioSourceId.h"
#include "yuengine/audio/AudioSourceSlot.h"
#include "yuengine/audio/AudioStatus.h"
#include "yuengine/audio/AudioVoiceHandle.h"
#include "yuengine/audio/AudioVoiceSlot.h"

namespace yuengine::audio
{
class TestAudioDevice final
{
public:
    TestAudioDevice();

    AudioStatus Initialize(const AudioDeviceDesc& desc);
    AudioStatus RegisterSyntheticSource(std::span<const std::int16_t> interleavedSamples, std::size_t frameCount, AudioSourceId& outSource);
    AudioStatus StartVoice(AudioSourceId source, std::uint32_t gainQ15, AudioVoiceHandle& outVoice);
    AudioStatus StopVoice(AudioVoiceHandle handle);
    AudioMixResult Mix(std::span<std::int16_t> outputSamples, std::size_t requestedFrames);
    AudioCapabilities Capabilities() const;
    AudioDeviceSnapshot Snapshot() const;

private:
    AudioStatus RecordFailure(AudioStatus status);
    bool IsDeviceFormatSupported(const AudioDeviceDesc& desc) const;
    bool IsSourceValid(AudioSourceId source) const;
    bool IsVoiceHandleValid(AudioVoiceHandle handle) const;
    std::int16_t ReadSourceSample(const AudioVoiceSlot& voice, std::size_t channel) const;
    std::int32_t ScaleSample(std::int16_t sample, std::uint32_t gainQ15) const;
    std::int16_t SaturateToS16(std::int32_t sample) const;
    void StopVoiceSlot(AudioVoiceSlot& voice);

    std::vector<AudioSourceSlot> _sources;
    std::vector<AudioVoiceSlot> _voices;
    AudioCapabilities _capabilities;
    AudioDeviceSnapshot _snapshot;
    bool _isInitialized;
};
}
