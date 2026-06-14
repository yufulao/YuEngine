#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "YuEngine/Audio/AudioCapabilities.h"
#include "YuEngine/Audio/AudioDeviceDesc.h"
#include "YuEngine/Audio/AudioDeviceSnapshot.h"
#include "YuEngine/Audio/AudioMixResult.h"
#include "YuEngine/Audio/AudioSourceId.h"
#include "YuEngine/Audio/AudioSourceSlot.h"
#include "YuEngine/Audio/AudioStatus.h"
#include "YuEngine/Audio/AudioVoiceHandle.h"
#include "YuEngine/Audio/AudioVoiceSlot.h"

namespace yuengine::audio {
class TestAudioDevice final {
public:
    TestAudioDevice();

    AudioStatus Initialize(const AudioDeviceDesc& desc);
    AudioStatus RegisterSyntheticSource(std::span<const std::int16_t> interleaved_samples, std::size_t frame_count, AudioSourceId& out_source);
    AudioStatus StartVoice(AudioSourceId source, std::uint32_t gain_q15, AudioVoiceHandle& out_voice);
    AudioStatus StopVoice(AudioVoiceHandle handle);
    AudioMixResult Mix(std::span<std::int16_t> output_samples, std::size_t requested_frames);
    AudioCapabilities Capabilities() const;
    AudioDeviceSnapshot Snapshot() const;

private:
    AudioStatus RecordFailure(AudioStatus status);
    bool IsDeviceFormatSupported(const AudioDeviceDesc& desc) const;
    bool IsSourceValid(AudioSourceId source) const;
    bool IsVoiceHandleValid(AudioVoiceHandle handle) const;
    std::int16_t ReadSourceSample(const AudioVoiceSlot& voice, std::size_t channel) const;
    std::int32_t ScaleSample(std::int16_t sample, std::uint32_t gain_q15) const;
    std::int16_t SaturateToS16(std::int64_t sample) const;
    void StopVoiceSlot(AudioVoiceSlot& voice);

    std::vector<AudioSourceSlot> sources_;
    std::vector<AudioVoiceSlot> voices_;
    AudioCapabilities capabilities_;
    AudioDeviceSnapshot snapshot_;
    std::uint32_t generation_seed_;
    bool is_initialized_;
};
}
