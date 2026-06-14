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

    AUDIO_STATUS Initialize(const audio_device_desc_t& desc);
    AUDIO_STATUS RegisterSyntheticSource(std::span<const std::int16_t> interleavedSamples, std::size_t frameCount, audio_source_id_t& outSource);
    AUDIO_STATUS StartVoice(audio_source_id_t source, std::uint32_t gainQ15, audio_voice_handle_t& outVoice);
    AUDIO_STATUS StopVoice(audio_voice_handle_t handle);
    audio_mix_result_t Mix(std::span<std::int16_t> outputSamples, std::size_t requestedFrames);
    audio_capabilities_t Capabilities() const;
    audio_device_snapshot_t Snapshot() const;

private:
    AUDIO_STATUS RecordFailure(AUDIO_STATUS status);
    bool IsDeviceFormatSupported(const audio_device_desc_t& desc) const;
    bool IsSourceValid(audio_source_id_t source) const;
    bool IsVoiceHandleValid(audio_voice_handle_t handle) const;
    std::int16_t ReadSourceSample(const audio_voice_slot_t& voice, std::size_t channel) const;
    std::int32_t ScaleSample(std::int16_t sample, std::uint32_t gainQ15) const;
    std::int16_t SaturateToS16(std::int64_t sample) const;
    void StopVoiceSlot(audio_voice_slot_t& voice);

    std::vector<audio_source_slot_t> _sources;
    std::vector<audio_voice_slot_t> _voices;
    audio_capabilities_t _capabilities;
    audio_device_snapshot_t _snapshot;
    std::uint32_t _generationSeed;
    bool _isInitialized;
};
}
