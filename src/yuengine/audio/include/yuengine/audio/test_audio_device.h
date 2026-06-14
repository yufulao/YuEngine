#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "yuengine/audio/audio_capabilities.h"
#include "yuengine/audio/audio_device_desc.h"
#include "yuengine/audio/audio_device_snapshot.h"
#include "yuengine/audio/audio_mix_result.h"
#include "yuengine/audio/audio_source_id.h"
#include "yuengine/audio/audio_source_slot.h"
#include "yuengine/audio/audio_status.h"
#include "yuengine/audio/audio_voice_handle.h"
#include "yuengine/audio/audio_voice_slot.h"

namespace yuengine::audio {
class TestAudioDevice final {
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
    std::int16_t SaturateToS16(std::int64_t sample) const;
    void StopVoiceSlot(AudioVoiceSlot& voice);

    std::vector<AudioSourceSlot> _sources;
    std::vector<AudioVoiceSlot> _voices;
    AudioCapabilities _capabilities;
    AudioDeviceSnapshot _snapshot;
    std::uint32_t _generationSeed;
    bool _isInitialized;
};
}
