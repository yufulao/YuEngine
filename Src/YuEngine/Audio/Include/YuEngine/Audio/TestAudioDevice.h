// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Include/YuEngine/Audio/TestAudioDevice.h

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
    /**
     * @comment Constructs a TestAudioDevice instance.
     */
    TestAudioDevice();

    /**
     * @comment Initializes the instance.
     * @param desc Input descriptor.
     * @return Explicit operation status.
     */
    AudioStatus Initialize(const AudioDeviceDesc& desc);
    /**
     * @comment Registers synthetic source.
     * @param interleaved_samples Input interleaved samples.
     * @param frame_count Input frame count.
     * @param out_source Output source written on success.
     * @return Explicit operation status.
     */
    AudioStatus RegisterSyntheticSource(std::span<const std::int16_t> interleaved_samples, std::size_t frame_count, AudioSourceId& out_source);
    /**
     * @comment Starts voice.
     * @param source Input source.
     * @param gain_q15 Input gain q15.
     * @param out_voice Output voice written on success.
     * @return Explicit operation status.
     */
    AudioStatus StartVoice(AudioSourceId source, std::uint32_t gain_q15, AudioVoiceHandle& out_voice);
    /**
     * @comment Stops voice.
     * @param handle Input handle.
     * @return Explicit operation status.
     */
    AudioStatus StopVoice(AudioVoiceHandle handle);
    /**
     * @comment Mixes requested samples into the output buffer.
     * @param output_samples Output sample buffer updated by the function.
     * @param requested_frames Input requested frames.
     * @return Explicit operation result.
     */
    AudioMixResult Mix(std::span<std::int16_t> output_samples, std::size_t requested_frames);
    /**
     * @comment Returns the supported capabilities.
     * @return Capability data.
     */
    AudioCapabilities Capabilities() const;
    /**
     * @comment Returns a snapshot of the current state.
     * @return Snapshot value.
     */
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
