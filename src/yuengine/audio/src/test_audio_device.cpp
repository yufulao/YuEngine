#include "yuengine/audio/test_audio_device.h"

#include <algorithm>

#include "yuengine/audio/audio_constants.h"

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

AudioStatus TestAudioDevice::Initialize(const audio_device_desc_t& desc) {
    if (desc.BackendKind != AudioBackendKind::Test) {
        return AudioStatus::UnsupportedBackend;
    }

    if (!IsDeviceFormatSupported(desc)) {
        return AudioStatus::UnsupportedFormat;
    }

    if (desc.SourceCapacity == 0U) {
        return AudioStatus::InvalidDescriptor;
    }

    if (desc.SourceCapacity > MAX_SOURCES) {
        return AudioStatus::CapacityExceeded;
    }

    if (desc.VoiceCapacity == 0U) {
        return AudioStatus::InvalidDescriptor;
    }

    if (desc.VoiceCapacity > MAX_VOICES) {
        return AudioStatus::CapacityExceeded;
    }

    ++_generationSeed;
    if (_generationSeed == INVALID_GENERATION) {
        ++_generationSeed;
    }

    _sources.assign(desc.SourceCapacity, audio_source_slot_t{});
    _voices.clear();
    _voices.reserve(MAX_VOICES);
    _voices.resize(desc.VoiceCapacity);
    for (audio_source_slot_t& source : _sources) {
        source.Generation = _generationSeed;
    }

    for (audio_voice_slot_t& voice : _voices) {
        voice.Generation = _generationSeed;
    }

    _capabilities = audio_capabilities_t{
        AudioBackendKind::Test,
        AudioSampleFormat::S16,
        SAMPLE_RATE,
        CHANNEL_COUNT,
        desc.SourceCapacity,
        desc.VoiceCapacity,
        MAX_SOURCE_FRAMES,
        MAX_OUTPUT_FRAMES,
        true};
    _snapshot = audio_device_snapshot_t{};
    _snapshot.SourceCapacity = desc.SourceCapacity;
    _snapshot.VoiceCapacity = desc.VoiceCapacity;
    _isInitialized = true;
    return AudioStatus::Success;
}

AudioStatus TestAudioDevice::RegisterSyntheticSource(std::span<const std::int16_t> interleavedSamples, std::size_t frameCount, audio_source_id_t& outSource) {
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
        audio_source_slot_t& slot = _sources[index];
        if (slot.IsActive) {
            continue;
        }

        slot.IsActive = true;
        slot.FrameCount = frameCount;
        slot.Samples.assign(interleavedSamples.begin(), interleavedSamples.begin() + requiredSamples);
        outSource = audio_source_id_t{static_cast<std::uint32_t>(index), slot.Generation};
        ++_snapshot.SourceCount;
        ++_snapshot.RegisteredSourceCount;
        return AudioStatus::Success;
    }

    return RecordFailure(AudioStatus::CapacityExceeded);
}

AudioStatus TestAudioDevice::StartVoice(audio_source_id_t source, std::uint32_t gainQ15, audio_voice_handle_t& outVoice) {
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
        audio_voice_slot_t& voice = _voices[index];
        if (voice.IsActive) {
            continue;
        }

        voice.IsActive = true;
        voice.Source = source;
        voice.CursorFrame = 0U;
        voice.GainQ15 = gainQ15;
        outVoice = audio_voice_handle_t{static_cast<std::uint32_t>(index), voice.Generation};
        ++_snapshot.ActiveVoiceCount;
        ++_snapshot.StartedVoiceCount;
        return AudioStatus::Success;
    }

    return RecordFailure(AudioStatus::CapacityExceeded);
}

AudioStatus TestAudioDevice::StopVoice(audio_voice_handle_t handle) {
    if (!_isInitialized) {
        return AudioStatus::InvalidDescriptor;
    }

    if (!IsVoiceHandleValid(handle)) {
        return RecordFailure(AudioStatus::InvalidHandle);
    }

    StopVoiceSlot(_voices[handle.Slot]);
    return AudioStatus::Success;
}

audio_mix_result_t TestAudioDevice::Mix(std::span<std::int16_t> outputSamples, std::size_t requestedFrames) {
    if (!_isInitialized) {
        return audio_mix_result_t{AudioStatus::InvalidDescriptor, 0U};
    }

    if (requestedFrames > MAX_OUTPUT_FRAMES) {
        RecordFailure(AudioStatus::CapacityExceeded);
        _snapshot.LastFramesWritten = 0U;
        return audio_mix_result_t{AudioStatus::CapacityExceeded, 0U};
    }

    const std::size_t requiredSamples = requestedFrames * CHANNEL_COUNT;
    if (outputSamples.size() < requiredSamples) {
        RecordFailure(AudioStatus::CapacityExceeded);
        _snapshot.LastFramesWritten = 0U;
        return audio_mix_result_t{AudioStatus::CapacityExceeded, 0U};
    }

    _snapshot.VoiceStorageCapacityBeforeMix = _voices.capacity();
    for (std::size_t frame = 0U; frame < requestedFrames; ++frame) {
        std::int64_t leftSample = 0;
        std::int64_t rightSample = 0;

        for (audio_voice_slot_t& voice : _voices) {
            if (!voice.IsActive) {
                continue;
            }

            if (!IsSourceValid(voice.Source)) {
                StopVoiceSlot(voice);
                continue;
            }

            const audio_source_slot_t& source = _sources[voice.Source.Slot];
            if (voice.CursorFrame >= source.FrameCount) {
                StopVoiceSlot(voice);
                continue;
            }

            leftSample += ScaleSample(ReadSourceSample(voice, 0U), voice.GainQ15);
            rightSample += ScaleSample(ReadSourceSample(voice, 1U), voice.GainQ15);
            ++voice.CursorFrame;

            if (voice.CursorFrame >= source.FrameCount) {
                StopVoiceSlot(voice);
                continue;
            }
        }

        outputSamples[(frame * CHANNEL_COUNT)] = SaturateToS16(leftSample);
        outputSamples[(frame * CHANNEL_COUNT) + 1U] = SaturateToS16(rightSample);
    }

    _snapshot.MixedFrameCount += requestedFrames;
    _snapshot.OutputSampleWriteCount += requiredSamples;
    _snapshot.LastFramesWritten = requestedFrames;
    _snapshot.VoiceStorageCapacityAfterLastMix = _voices.capacity();
    return audio_mix_result_t{AudioStatus::Success, requestedFrames};
}

audio_capabilities_t TestAudioDevice::Capabilities() const {
    return _capabilities;
}

audio_device_snapshot_t TestAudioDevice::Snapshot() const {
    return _snapshot;
}

AudioStatus TestAudioDevice::RecordFailure(AudioStatus status) {
    ++_snapshot.FailedOperationCount;
    return status;
}

bool TestAudioDevice::IsDeviceFormatSupported(const audio_device_desc_t& desc) const {
    if (desc.Format != AudioSampleFormat::S16) {
        return false;
    }

    if (desc.SampleRate != SAMPLE_RATE) {
        return false;
    }

    if (desc.ChannelCount != CHANNEL_COUNT) {
        return false;
    }

    return true;
}

bool TestAudioDevice::IsSourceValid(audio_source_id_t source) const {
    if (source.Generation == INVALID_GENERATION) {
        return false;
    }

    if (source.Slot >= _sources.size()) {
        return false;
    }

    const audio_source_slot_t& slot = _sources[source.Slot];
    if (!slot.IsActive) {
        return false;
    }

    return slot.Generation == source.Generation;
}

bool TestAudioDevice::IsVoiceHandleValid(audio_voice_handle_t handle) const {
    if (handle.Generation == INVALID_GENERATION) {
        return false;
    }

    if (handle.Slot >= _voices.size()) {
        return false;
    }

    const audio_voice_slot_t& voice = _voices[handle.Slot];
    if (!voice.IsActive) {
        return false;
    }

    return voice.Generation == handle.Generation;
}

std::int16_t TestAudioDevice::ReadSourceSample(const audio_voice_slot_t& voice, std::size_t channel) const {
    const audio_source_slot_t& source = _sources[voice.Source.Slot];
    return source.Samples[(voice.CursorFrame * CHANNEL_COUNT) + channel];
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

void TestAudioDevice::StopVoiceSlot(audio_voice_slot_t& voice) {
    voice.IsActive = false;
    voice.CursorFrame = 0U;
    ++voice.Generation;
    --_snapshot.ActiveVoiceCount;
    ++_snapshot.StoppedVoiceCount;
}
}
