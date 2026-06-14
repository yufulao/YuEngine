#include "YuEngine/Audio/TestAudioDevice.h"

#include <algorithm>

#include "YuEngine/Audio/AudioConstants.h"

namespace yuengine::audio {
namespace {
constexpr std::uint32_t INVALID_GENERATION = 0U;
}

TestAudioDevice::TestAudioDevice()
    : _sources(),
      _voices(),
      _capabilities{},
      _snapshot{},
      _generationSeed(INVALID_GENERATION),
      _isInitialized(false) {
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

    ++_generationSeed;
    if (_generationSeed == INVALID_GENERATION) {
        ++_generationSeed;
    }

    _sources.assign(desc.source_capacity, AudioSourceSlot{});
    _voices.clear();
    _voices.reserve(MAX_VOICES);
    _voices.resize(desc.voice_capacity);
    for (AudioSourceSlot& source : _sources) {
        source.generation = _generationSeed;
    }

    for (AudioVoiceSlot& voice : _voices) {
        voice.generation = _generationSeed;
    }

    _capabilities = AudioCapabilities{
        AudioBackendKind::Test,
        AudioSampleFormat::Signed16,
        SAMPLE_RATE,
        CHANNEL_COUNT,
        desc.source_capacity,
        desc.voice_capacity,
        MAX_SOURCE_FRAMES,
        MAX_OUTPUT_FRAMES,
        true};
    _snapshot = AudioDeviceSnapshot{};
    _snapshot.source_capacity = desc.source_capacity;
    _snapshot.voice_capacity = desc.voice_capacity;
    _isInitialized = true;
    return AudioStatus::Success;
}

AudioStatus TestAudioDevice::RegisterSyntheticSource(std::span<const std::int16_t> interleavedSamples, std::size_t frameCount, AudioSourceId& outSource) {
    if (!_isInitialized) {
        return AudioStatus::InvalidDescriptor;
    }

    if (frameCount == 0U) {
        return RecordFailure(AudioStatus::InvalidDescriptor);
    }

    if (frameCount > MAX_SOURCE_FRAMES) {
        return RecordFailure(AudioStatus::InvalidDescriptor);
    }

    const std::size_t requiredSamples = frameCount * CHANNEL_COUNT;
    if (interleavedSamples.size() < requiredSamples) {
        return RecordFailure(AudioStatus::InvalidDescriptor);
    }

    for (std::size_t index = 0U; index < _sources.size(); ++index) {
        AudioSourceSlot& slot = _sources[index];
        if (slot.is_active) {
            continue;
        }

        slot.is_active = true;
        slot.frame_count = frameCount;
        slot.samples.assign(interleavedSamples.begin(), interleavedSamples.begin() + requiredSamples);
        outSource = AudioSourceId{static_cast<std::uint32_t>(index), slot.generation};
        ++_snapshot.source_count;
        ++_snapshot.registered_source_count;
        return AudioStatus::Success;
    }

    return RecordFailure(AudioStatus::CapacityExceeded);
}

AudioStatus TestAudioDevice::StartVoice(AudioSourceId source, std::uint32_t gainQ15, AudioVoiceHandle& outVoice) {
    if (!_isInitialized) {
        return AudioStatus::InvalidDescriptor;
    }

    if (gainQ15 > MAX_Q15_GAIN) {
        return RecordFailure(AudioStatus::InvalidGain);
    }

    if (!IsSourceValid(source)) {
        return RecordFailure(AudioStatus::SourceNotFound);
    }

    for (std::size_t index = 0U; index < _voices.size(); ++index) {
        AudioVoiceSlot& voice = _voices[index];
        if (voice.is_active) {
            continue;
        }

        voice.is_active = true;
        voice.source = source;
        voice.cursor_frame = 0U;
        voice.gain_q15 = gainQ15;
        outVoice = AudioVoiceHandle{static_cast<std::uint32_t>(index), voice.generation};
        ++_snapshot.active_voice_count;
        ++_snapshot.started_voice_count;
        return AudioStatus::Success;
    }

    return RecordFailure(AudioStatus::CapacityExceeded);
}

AudioStatus TestAudioDevice::StopVoice(AudioVoiceHandle handle) {
    if (!_isInitialized) {
        return AudioStatus::InvalidDescriptor;
    }

    if (!IsVoiceHandleValid(handle)) {
        return RecordFailure(AudioStatus::InvalidHandle);
    }

    StopVoiceSlot(_voices[handle.slot]);
    return AudioStatus::Success;
}

AudioMixResult TestAudioDevice::Mix(std::span<std::int16_t> outputSamples, std::size_t requestedFrames) {
    if (!_isInitialized) {
        return AudioMixResult{AudioStatus::InvalidDescriptor, 0U};
    }

    if (requestedFrames > MAX_OUTPUT_FRAMES) {
        RecordFailure(AudioStatus::CapacityExceeded);
        _snapshot.last_frames_written = 0U;
        return AudioMixResult{AudioStatus::CapacityExceeded, 0U};
    }

    const std::size_t requiredSamples = requestedFrames * CHANNEL_COUNT;
    if (outputSamples.size() < requiredSamples) {
        RecordFailure(AudioStatus::CapacityExceeded);
        _snapshot.last_frames_written = 0U;
        return AudioMixResult{AudioStatus::CapacityExceeded, 0U};
    }

    _snapshot.voice_storage_capacity_before_mix = _voices.capacity();
    for (std::size_t frame = 0U; frame < requestedFrames; ++frame) {
        std::int64_t leftSample = 0;
        std::int64_t rightSample = 0;

        for (AudioVoiceSlot& voice : _voices) {
            if (!voice.is_active) {
                continue;
            }

            if (!IsSourceValid(voice.source)) {
                StopVoiceSlot(voice);
                continue;
            }

            const AudioSourceSlot& source = _sources[voice.source.slot];
            if (voice.cursor_frame >= source.frame_count) {
                StopVoiceSlot(voice);
                continue;
            }

            leftSample += ScaleSample(ReadSourceSample(voice, 0U), voice.gain_q15);
            rightSample += ScaleSample(ReadSourceSample(voice, 1U), voice.gain_q15);
            ++voice.cursor_frame;

            if (voice.cursor_frame >= source.frame_count) {
                StopVoiceSlot(voice);
                continue;
            }
        }

        outputSamples[(frame * CHANNEL_COUNT)] = SaturateToS16(leftSample);
        outputSamples[(frame * CHANNEL_COUNT) + 1U] = SaturateToS16(rightSample);
    }

    _snapshot.mixed_frame_count += requestedFrames;
    _snapshot.output_sample_write_count += requiredSamples;
    _snapshot.last_frames_written = requestedFrames;
    _snapshot.voice_storage_capacity_after_last_mix = _voices.capacity();
    return AudioMixResult{AudioStatus::Success, requestedFrames};
}

AudioCapabilities TestAudioDevice::Capabilities() const {
    return _capabilities;
}

AudioDeviceSnapshot TestAudioDevice::Snapshot() const {
    return _snapshot;
}

AudioStatus TestAudioDevice::RecordFailure(AudioStatus status) {
    ++_snapshot.failed_operation_count;
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

    if (source.slot >= _sources.size()) {
        return false;
    }

    const AudioSourceSlot& slot = _sources[source.slot];
    if (!slot.is_active) {
        return false;
    }

    return slot.generation == source.generation;
}

bool TestAudioDevice::IsVoiceHandleValid(AudioVoiceHandle handle) const {
    if (handle.generation == INVALID_GENERATION) {
        return false;
    }

    if (handle.slot >= _voices.size()) {
        return false;
    }

    const AudioVoiceSlot& voice = _voices[handle.slot];
    if (!voice.is_active) {
        return false;
    }

    return voice.generation == handle.generation;
}

std::int16_t TestAudioDevice::ReadSourceSample(const AudioVoiceSlot& voice, std::size_t channel) const {
    const AudioSourceSlot& source = _sources[voice.source.slot];
    return source.samples[(voice.cursor_frame * CHANNEL_COUNT) + channel];
}

std::int32_t TestAudioDevice::ScaleSample(std::int16_t sample, std::uint32_t gainQ15) const {
    const std::int32_t sourceSample = static_cast<std::int32_t>(sample);
    const std::int32_t gain = static_cast<std::int32_t>(gainQ15);
    return (sourceSample * gain) / static_cast<std::int32_t>(MAX_Q15_GAIN);
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
    --_snapshot.active_voice_count;
    ++_snapshot.stopped_voice_count;
}
}
