// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Src/TestAudioDevice.cpp

#include "YuEngine/Audio/TestAudioDevice.h"

#include <algorithm>

#include "YuEngine/Audio/AudioConstants.h"

namespace yuengine::audio {
namespace {
constexpr std::uint32_t INVALID_GENERATION = 0U;
}

TestAudioDevice::TestAudioDevice()
    : sources_(),
      voices_(),
      capabilities_{},
      snapshot_{},
      generation_seed_(INVALID_GENERATION),
      is_initialized_(false) {
}

AudioStatus TestAudioDevice::Initialize(const AudioDeviceDesc& desc) {
    if (desc.backend_kind != AudioBackendKind::Test) {
        return AudioStatus::UnsupportedBackend;
    }

    if (!IsDeviceFormatSupported(desc)) {
        return AudioStatus::UnsupportedFormat;
    }

    if (desc.source_capacity == 0U) {
        return AudioStatus::InvalidDescriptor;
    }

    if (desc.source_capacity > MAX_SOURCES) {
        return AudioStatus::CapacityExceeded;
    }

    if (desc.voice_capacity == 0U) {
        return AudioStatus::InvalidDescriptor;
    }

    if (desc.voice_capacity > MAX_VOICES) {
        return AudioStatus::CapacityExceeded;
    }

    ++generation_seed_;
    if (generation_seed_ == INVALID_GENERATION) {
        ++generation_seed_;
    }

    sources_.assign(desc.source_capacity, AudioSourceSlot{});
    voices_.clear();
    voices_.reserve(MAX_VOICES);
    voices_.resize(desc.voice_capacity);
    for (AudioSourceSlot& source : sources_) {
        source.generation = generation_seed_;
    }

    for (AudioVoiceSlot& voice : voices_) {
        voice.generation = generation_seed_;
    }

    capabilities_ = AudioCapabilities{
        AudioBackendKind::Test,
        AudioSampleFormat::Signed16,
        SAMPLE_RATE,
        CHANNEL_COUNT,
        desc.source_capacity,
        desc.voice_capacity,
        MAX_SOURCE_FRAMES,
        MAX_OUTPUT_FRAMES,
        true};
    snapshot_ = AudioDeviceSnapshot{};
    snapshot_.source_capacity = desc.source_capacity;
    snapshot_.voice_capacity = desc.voice_capacity;
    is_initialized_ = true;
    return AudioStatus::Success;
}

AudioStatus TestAudioDevice::RegisterSyntheticSource(std::span<const std::int16_t> interleaved_samples, std::size_t frame_count, AudioSourceId& out_source) {
    if (!is_initialized_) {
        return AudioStatus::InvalidDescriptor;
    }

    if (frame_count == 0U) {
        return RecordFailure(AudioStatus::InvalidDescriptor);
    }

    if (frame_count > MAX_SOURCE_FRAMES) {
        return RecordFailure(AudioStatus::InvalidDescriptor);
    }

    const std::size_t required_samples = frame_count * CHANNEL_COUNT;
    if (interleaved_samples.size() < required_samples) {
        return RecordFailure(AudioStatus::InvalidDescriptor);
    }

    for (std::size_t index = 0U; index < sources_.size(); ++index) {
        AudioSourceSlot& slot = sources_[index];
        if (slot.is_active) {
            continue;
        }

        slot.is_active = true;
        slot.frame_count = frame_count;
        slot.samples.assign(interleaved_samples.begin(), interleaved_samples.begin() + required_samples);
        out_source = AudioSourceId{static_cast<std::uint32_t>(index), slot.generation};
        ++snapshot_.source_count;
        ++snapshot_.registered_source_count;
        return AudioStatus::Success;
    }

    return RecordFailure(AudioStatus::CapacityExceeded);
}

AudioStatus TestAudioDevice::StartVoice(AudioSourceId source, std::uint32_t gain_q15, AudioVoiceHandle& out_voice) {
    if (!is_initialized_) {
        return AudioStatus::InvalidDescriptor;
    }

    if (gain_q15 > MAX_Q15_GAIN) {
        return RecordFailure(AudioStatus::InvalidGain);
    }

    if (!IsSourceValid(source)) {
        return RecordFailure(AudioStatus::SourceNotFound);
    }

    for (std::size_t index = 0U; index < voices_.size(); ++index) {
        AudioVoiceSlot& voice = voices_[index];
        if (voice.is_active) {
            continue;
        }

        voice.is_active = true;
        voice.source = source;
        voice.cursor_frame = 0U;
        voice.gain_q15 = gain_q15;
        out_voice = AudioVoiceHandle{static_cast<std::uint32_t>(index), voice.generation};
        ++snapshot_.active_voice_count;
        ++snapshot_.started_voice_count;
        return AudioStatus::Success;
    }

    return RecordFailure(AudioStatus::CapacityExceeded);
}

AudioStatus TestAudioDevice::StopVoice(AudioVoiceHandle handle) {
    if (!is_initialized_) {
        return AudioStatus::InvalidDescriptor;
    }

    if (!IsVoiceHandleValid(handle)) {
        return RecordFailure(AudioStatus::InvalidHandle);
    }

    StopVoiceSlot(voices_[handle.slot]);
    return AudioStatus::Success;
}

AudioMixResult TestAudioDevice::Mix(std::span<std::int16_t> output_samples, std::size_t requested_frames) {
    if (!is_initialized_) {
        return AudioMixResult{AudioStatus::InvalidDescriptor, 0U};
    }

    if (requested_frames > MAX_OUTPUT_FRAMES) {
        RecordFailure(AudioStatus::CapacityExceeded);
        snapshot_.last_frames_written = 0U;
        return AudioMixResult{AudioStatus::CapacityExceeded, 0U};
    }

    const std::size_t required_samples = requested_frames * CHANNEL_COUNT;
    if (output_samples.size() < required_samples) {
        RecordFailure(AudioStatus::CapacityExceeded);
        snapshot_.last_frames_written = 0U;
        return AudioMixResult{AudioStatus::CapacityExceeded, 0U};
    }

    snapshot_.voice_storage_capacity_before_mix = voices_.capacity();
    for (std::size_t frame = 0U; frame < requested_frames; ++frame) {
        std::int64_t left_sample = 0;
        std::int64_t right_sample = 0;

        for (AudioVoiceSlot& voice : voices_) {
            if (!voice.is_active) {
                continue;
            }

            if (!IsSourceValid(voice.source)) {
                StopVoiceSlot(voice);
                continue;
            }

            const AudioSourceSlot& source = sources_[voice.source.slot];
            if (voice.cursor_frame >= source.frame_count) {
                StopVoiceSlot(voice);
                continue;
            }

            left_sample += ScaleSample(ReadSourceSample(voice, 0U), voice.gain_q15);
            right_sample += ScaleSample(ReadSourceSample(voice, 1U), voice.gain_q15);
            ++voice.cursor_frame;

            if (voice.cursor_frame >= source.frame_count) {
                StopVoiceSlot(voice);
                continue;
            }
        }

        output_samples[(frame * CHANNEL_COUNT)] = SaturateToS16(left_sample);
        output_samples[(frame * CHANNEL_COUNT) + 1U] = SaturateToS16(right_sample);
    }

    snapshot_.mixed_frame_count += requested_frames;
    snapshot_.output_sample_write_count += required_samples;
    snapshot_.last_frames_written = requested_frames;
    snapshot_.voice_storage_capacity_after_last_mix = voices_.capacity();
    return AudioMixResult{AudioStatus::Success, requested_frames};
}

AudioCapabilities TestAudioDevice::Capabilities() const {
    return capabilities_;
}

AudioDeviceSnapshot TestAudioDevice::Snapshot() const {
    return snapshot_;
}

AudioStatus TestAudioDevice::RecordFailure(AudioStatus status) {
    ++snapshot_.failed_operation_count;
    return status;
}

bool TestAudioDevice::IsDeviceFormatSupported(const AudioDeviceDesc& desc) const {
    if (desc.format != AudioSampleFormat::Signed16) {
        return false;
    }

    if (desc.sample_rate != SAMPLE_RATE) {
        return false;
    }

    if (desc.channel_count != CHANNEL_COUNT) {
        return false;
    }

    return true;
}

bool TestAudioDevice::IsSourceValid(AudioSourceId source) const {
    if (source.generation == INVALID_GENERATION) {
        return false;
    }

    if (source.slot >= sources_.size()) {
        return false;
    }

    const AudioSourceSlot& slot = sources_[source.slot];
    if (!slot.is_active) {
        return false;
    }

    return slot.generation == source.generation;
}

bool TestAudioDevice::IsVoiceHandleValid(AudioVoiceHandle handle) const {
    if (handle.generation == INVALID_GENERATION) {
        return false;
    }

    if (handle.slot >= voices_.size()) {
        return false;
    }

    const AudioVoiceSlot& voice = voices_[handle.slot];
    if (!voice.is_active) {
        return false;
    }

    return voice.generation == handle.generation;
}

std::int16_t TestAudioDevice::ReadSourceSample(const AudioVoiceSlot& voice, std::size_t channel) const {
    const AudioSourceSlot& source = sources_[voice.source.slot];
    return source.samples[(voice.cursor_frame * CHANNEL_COUNT) + channel];
}

std::int32_t TestAudioDevice::ScaleSample(std::int16_t sample, std::uint32_t gain_q15) const {
    const std::int32_t source_sample = static_cast<std::int32_t>(sample);
    const std::int32_t gain = static_cast<std::int32_t>(gain_q15);
    return (source_sample * gain) / static_cast<std::int32_t>(MAX_Q15_GAIN);
}

std::int16_t TestAudioDevice::SaturateToS16(std::int64_t sample) const {
    if (sample > static_cast<std::int64_t>(S16_MAX)) {
        return S16_MAX;
    }

    if (sample < static_cast<std::int64_t>(S16_MIN)) {
        return S16_MIN;
    }

    return static_cast<std::int16_t>(sample);
}

void TestAudioDevice::StopVoiceSlot(AudioVoiceSlot& voice) {
    voice.is_active = false;
    voice.cursor_frame = 0U;
    ++voice.generation;
    --snapshot_.active_voice_count;
    ++snapshot_.stopped_voice_count;
}
}
