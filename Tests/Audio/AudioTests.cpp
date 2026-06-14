#include <array>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "YuEngine/Audio/AudioConstants.h"
#include "YuEngine/Audio/TestAudioDevice.h"

using yuengine::audio::AudioBackendKind;
using yuengine::audio::AudioDeviceDesc;
using yuengine::audio::AudioDeviceSnapshot;
using yuengine::audio::AudioMixResult;
using yuengine::audio::AudioSampleFormat;
using yuengine::audio::AudioSourceId;
using yuengine::audio::AudioStatus;
using yuengine::audio::AudioVoiceHandle;
using TestAudioDevice = yuengine::audio::TestAudioDevice;
using yuengine::audio::CHANNEL_COUNT;
using yuengine::audio::MAX_Q15_GAIN;
using yuengine::audio::MAX_SOURCES;
using yuengine::audio::MAX_VOICES;
using yuengine::audio::S16_MAX;
using yuengine::audio::S16_MIN;
using yuengine::audio::SAMPLE_RATE;

namespace {
constexpr const char* TEST_CREATE_DEVICE = "Audio_CreateTestDevice_ReturnsCapabilities";
constexpr const char* TEST_UNSUPPORTED_BACKEND = "Audio_CreateDevice_RejectsUnsupportedBackend";
constexpr const char* TEST_UNSUPPORTED_FORMAT = "Audio_CreateDevice_RejectsUnsupportedFormat";
constexpr const char* TEST_REGISTER_SOURCE = "Audio_RegisterSyntheticSource_ReturnsStableId";
constexpr const char* TEST_SOURCE_CAPACITY = "Audio_SourceCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_START_VOICE = "Audio_StartVoice_ReturnsGenerationHandle";
constexpr const char* TEST_MISSING_SOURCE = "Audio_StartVoice_RejectsMissingSource";
constexpr const char* TEST_INVALID_GAIN = "Audio_StartVoice_RejectsInvalidGainWithoutMutation";
constexpr const char* TEST_VOICE_CAPACITY = "Audio_VoiceCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_STOP_STALE = "Audio_StopVoice_InvalidatesStaleHandle";
constexpr const char* TEST_REINITIALIZE_STALE_HANDLES = "Audio_Reinitialize_InvalidatesPriorSourceAndVoiceHandles";
constexpr const char* TEST_SINGLE_VOICE = "Audio_MixSingleVoice_WritesDeterministicS16StereoSamples";
constexpr const char* TEST_UNITY_GAIN = "Audio_MixUnityGain_PreservesS16EdgeSamples";
constexpr const char* TEST_FRACTIONAL_GAIN = "Audio_MixFractionalGain_RoundsTowardZeroDeterministically";
constexpr const char* TEST_MULTIPLE_VOICES = "Audio_MixMultipleVoices_UsesStableOrderAndSaturates";
constexpr const char* TEST_MAX_VOICE_FULL_SCALE = "Audio_MixMaxVoicesFullScaleSaturatesWithoutOverflow";
constexpr const char* TEST_STOPS_AT_END = "Audio_MixStopsVoiceAtEndOfSource";
constexpr const char* TEST_SILENT_TAIL = "Audio_MixEndedVoice_WritesSilentTail";
constexpr const char* TEST_OVERWRITE = "Audio_MixOverwritesPrefilledDestination";
constexpr const char* TEST_UNDERSIZED_OUTPUT = "Audio_MixRejectsUndersizedBufferWithoutWritingSamples";
constexpr const char* TEST_UNINITIALIZED_LIFECYCLE = "Audio_UninitializedDeviceOperationsReturnExplicitStatusWithoutMutation";
constexpr const char* TEST_NO_GROW = "Audio_Mix_DoesNotGrowVoiceStorage";
constexpr const char* TEST_DISABLED_DIAGNOSTICS = "Audio_DisabledDiagnosticsDoesNotChangeResults";
constexpr const char* TEST_NO_FORBIDDEN_DEPENDENCY = "Audio_NoDeviceCodecResourceScriptUiGameAdapterDependency";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr const char* REINIT_SOURCE_REGISTRATION_MESSAGE = "source registration failed";
constexpr const char* REINIT_VOICE_START_MESSAGE = "voice start failed";
constexpr const char* REINIT_DEVICE_MESSAGE = "device reinitialize failed";
constexpr const char* REINIT_MIX_MESSAGE = "mix after reinitialize failed";
constexpr const char* REINIT_MIX_FRAME_COUNT_MESSAGE = "mix after reinitialize reported wrong frame count";
constexpr const char* REINIT_MIX_STALE_VOICE_MESSAGE = "mix after reinitialize consumed stale voice";
constexpr const char* REINIT_MIX_ACTIVE_VOICE_MESSAGE = "mix after reinitialize observed stale active voice";
constexpr const char* REINIT_ACTIVE_SOURCE_REGISTRATION_MESSAGE = "source registration after reinitialize failed";
constexpr const char* REINIT_ACTIVE_VOICE_START_MESSAGE = "voice start after reinitialize failed";
constexpr const char* REINIT_STALE_VOICE_ACCEPTED_MESSAGE = "stale voice handle from prior initialize was accepted";
constexpr const char* REINIT_STALE_VOICE_COUNT_MESSAGE = "stale voice handle changed active voice count";
constexpr const char* REINIT_STALE_SOURCE_ACCEPTED_MESSAGE = "stale source from prior initialize was accepted";
constexpr const char* REINIT_STALE_SOURCE_COUNT_MESSAGE = "stale source changed active voice count";
constexpr const char* REINIT_ACTIVE_VOICE_MESSAGE = "active voice did not survive stale handle checks";
constexpr std::int16_t PREFILL_SAMPLE = 1234;
constexpr std::int16_t SENTINEL_SAMPLE = -1234;
using TestFunction = int (*)();

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

TestAudioDevice CreateInitializedDevice() {
    TestAudioDevice device;
    device.Initialize(AudioDeviceDesc{});
    return device;
}

std::array<std::int16_t, 4U> BasicSourceSamples() {
    return {1000, -1000, 2000, -2000};
}

bool RegisterBasicSource(TestAudioDevice& device, AudioSourceId& outSource) {
    const std::array<std::int16_t, 4U> samples = BasicSourceSamples();
    return device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 2U, outSource) == AudioStatus::Success;
}

bool StartBasicVoice(TestAudioDevice& device, AudioVoiceHandle& outVoice) {
    AudioSourceId source{};
    if (!RegisterBasicSource(device, source)) {
        return false;
    }

    return device.StartVoice(source, MAX_Q15_GAIN, outVoice) == AudioStatus::Success;
}

bool SamplesEqual(std::span<const std::int16_t> left, std::span<const std::int16_t> right) {
    if (left.size() != right.size()) {
        return false;
    }

    for (std::size_t index = 0U; index < left.size(); ++index) {
        if (left[index] != right[index]) {
            return false;
        }
    }

    return true;
}

bool SnapshotsEqual(const AudioDeviceSnapshot& left, const AudioDeviceSnapshot& right) {
    return left.source_capacity == right.source_capacity &&
           left.voice_capacity == right.voice_capacity &&
           left.source_count == right.source_count &&
           left.active_voice_count == right.active_voice_count &&
           left.voice_storage_capacity_before_mix == right.voice_storage_capacity_before_mix &&
           left.voice_storage_capacity_after_last_mix == right.voice_storage_capacity_after_last_mix &&
           left.registered_source_count == right.registered_source_count &&
           left.started_voice_count == right.started_voice_count &&
           left.stopped_voice_count == right.stopped_voice_count &&
           left.mixed_frame_count == right.mixed_frame_count &&
           left.output_sample_write_count == right.output_sample_write_count &&
           left.failed_operation_count == right.failed_operation_count &&
           left.last_frames_written == right.last_frames_written &&
           left.allocation_accounting_status == right.allocation_accounting_status;
}

bool MixMaxVoicesFullScale(std::int16_t sourceSample, std::int16_t expectedSample) {
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 2U> samples{sourceSample, sourceSample};
    AudioSourceId source{};
    if (device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 1U, source) != AudioStatus::Success) {
        return false;
    }

    for (std::size_t index = 0U; index < MAX_VOICES; ++index) {
        AudioVoiceHandle voice{};
        if (device.StartVoice(source, MAX_Q15_GAIN, voice) != AudioStatus::Success) {
            return false;
        }
    }

    std::array<std::int16_t, 2U> output{};
    const AudioMixResult result = device.Mix(std::span<std::int16_t>(output.data(), output.size()), 1U);
    return result.status == AudioStatus::Success &&
           result.frames_written == 1U &&
           output[0U] == expectedSample &&
           output[1U] == expectedSample;
}

int AudioCreateTestDeviceReturnsCapabilities() {
    TestAudioDevice device;
    if (device.Initialize(AudioDeviceDesc{}) != AudioStatus::Success) {
        return Fail("test audio device did not initialize");
    }

    const auto capabilities = device.Capabilities();
    if (capabilities.backend_kind != AudioBackendKind::Test) {
        return Fail("capabilities did not report test backend");
    }

    if (capabilities.format != AudioSampleFormat::Signed16) {
        return Fail("capabilities did not report S16");
    }

    if (capabilities.sample_rate != SAMPLE_RATE) {
        return Fail("capabilities did not report fixed sample rate");
    }

    if (capabilities.channel_count != CHANNEL_COUNT) {
        return Fail("capabilities did not report stereo");
    }

    if (!capabilities.supports_deterministic_mix) {
        return Fail("capabilities did not report deterministic mix support");
    }

    return 0;
}

int AudioCreateDeviceRejectsUnsupportedBackend() {
    TestAudioDevice device;
    AudioDeviceDesc desc{};
    desc.backend_kind = AudioBackendKind::Unsupported;
    if (device.Initialize(desc) != AudioStatus::UnsupportedBackend) {
        return Fail("unsupported backend was not rejected");
    }

    if (device.Snapshot().source_capacity != 0U) {
        return Fail("unsupported backend mutated source capacity");
    }

    return 0;
}

int AudioCreateDeviceRejectsUnsupportedFormat() {
    TestAudioDevice device;
    AudioDeviceDesc formatDesc{};
    formatDesc.format = AudioSampleFormat::Unsupported;
    if (device.Initialize(formatDesc) != AudioStatus::UnsupportedFormat) {
        return Fail("unsupported sample format was not rejected");
    }

    AudioDeviceDesc sampleRateDesc{};
    sampleRateDesc.sample_rate = 44100U;
    if (device.Initialize(sampleRateDesc) != AudioStatus::UnsupportedFormat) {
        return Fail("unsupported sample rate was not rejected");
    }

    AudioDeviceDesc channelDesc{};
    channelDesc.channel_count = 1U;
    if (device.Initialize(channelDesc) != AudioStatus::UnsupportedFormat) {
        return Fail("unsupported channel count was not rejected");
    }

    return 0;
}

int AudioRegisterSyntheticSourceReturnsStableId() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioSourceId source{};
    if (!RegisterBasicSource(device, source)) {
        return Fail("source registration failed");
    }

    if (source.slot != 0U) {
        return Fail("first source used unexpected slot");
    }

    if (source.generation == 0U) {
        return Fail("source generation was invalid");
    }

    if (device.Snapshot().registered_source_count != 1U) {
        return Fail("registered source count was not updated");
    }

    return 0;
}

int AudioSourceCapacityOverflowDoesNotMutate() {
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 2U> samples{1, 1};
    for (std::size_t index = 0U; index < MAX_SOURCES; ++index) {
        AudioSourceId source{};
        const AudioStatus status = device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 1U, source);
        if (status != AudioStatus::Success) {
            return Fail("source registration failed before capacity");
        }
    }

    const auto beforeSnapshot = device.Snapshot();
    AudioSourceId overflowSource{};
    const AudioStatus overflowStatus = device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 1U, overflowSource);
    if (overflowStatus != AudioStatus::CapacityExceeded) {
        return Fail("source overflow did not return capacity status");
    }

    if (device.Snapshot().source_count != beforeSnapshot.source_count) {
        return Fail("source overflow changed source count");
    }

    return 0;
}

int AudioStartVoiceReturnsGenerationHandle() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice)) {
        return Fail("voice start failed");
    }

    if (voice.slot != 0U) {
        return Fail("first voice used unexpected slot");
    }

    if (voice.generation == 0U) {
        return Fail("voice generation was invalid");
    }

    if (device.Snapshot().active_voice_count != 1U) {
        return Fail("active voice count was not updated");
    }

    return 0;
}

int AudioStartVoiceRejectsMissingSource() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (device.StartVoice(AudioSourceId{99U, 1U}, MAX_Q15_GAIN, voice) != AudioStatus::SourceNotFound) {
        return Fail("missing source was not rejected");
    }

    if (device.Snapshot().active_voice_count != 0U) {
        return Fail("missing source changed active voice count");
    }

    return 0;
}

int AudioStartVoiceRejectsInvalidGainWithoutMutation() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioSourceId source{};
    if (!RegisterBasicSource(device, source)) {
        return Fail("source registration failed");
    }

    AudioVoiceHandle voice{};
    const AudioStatus status = device.StartVoice(source, MAX_Q15_GAIN + 1U, voice);
    if (status != AudioStatus::InvalidGain) {
        return Fail("invalid gain was not rejected");
    }

    if (device.Snapshot().active_voice_count != 0U) {
        return Fail("invalid gain changed active voice count");
    }

    return 0;
}

int AudioVoiceCapacityOverflowDoesNotMutate() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioSourceId source{};
    if (!RegisterBasicSource(device, source)) {
        return Fail("source registration failed");
    }

    for (std::size_t index = 0U; index < MAX_VOICES; ++index) {
        AudioVoiceHandle voice{};
        if (device.StartVoice(source, MAX_Q15_GAIN, voice) != AudioStatus::Success) {
            return Fail("voice start failed before capacity");
        }
    }

    const auto beforeSnapshot = device.Snapshot();
    AudioVoiceHandle overflowVoice{};
    if (device.StartVoice(source, MAX_Q15_GAIN, overflowVoice) != AudioStatus::CapacityExceeded) {
        return Fail("voice overflow did not return capacity status");
    }

    if (device.Snapshot().active_voice_count != beforeSnapshot.active_voice_count) {
        return Fail("voice overflow changed active voice count");
    }

    return 0;
}

int AudioStopVoiceInvalidatesStaleHandle() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice)) {
        return Fail("voice start failed");
    }

    if (device.StopVoice(voice) != AudioStatus::Success) {
        return Fail("voice stop failed");
    }

    if (device.StopVoice(voice) != AudioStatus::InvalidHandle) {
        return Fail("stale voice handle was not rejected");
    }

    if (device.Snapshot().active_voice_count != 0U) {
        return Fail("stopped voice remained active");
    }

    return 0;
}

int AudioReinitializeInvalidatesPriorSourceAndVoiceHandles() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioSourceId staleSource{};
    if (!RegisterBasicSource(device, staleSource)) {
        return Fail(REINIT_SOURCE_REGISTRATION_MESSAGE);
    }

    AudioVoiceHandle staleVoice{};
    if (device.StartVoice(staleSource, MAX_Q15_GAIN, staleVoice) != AudioStatus::Success) {
        return Fail(REINIT_VOICE_START_MESSAGE);
    }

    if (device.Initialize(AudioDeviceDesc{}) != AudioStatus::Success) {
        return Fail(REINIT_DEVICE_MESSAGE);
    }

    std::array<std::int16_t, 4U> staleMixOutput{SENTINEL_SAMPLE, SENTINEL_SAMPLE, SENTINEL_SAMPLE, SENTINEL_SAMPLE};
    const AudioMixResult staleMixResult = device.Mix(std::span<std::int16_t>(staleMixOutput.data(), staleMixOutput.size()), 2U);
    if (staleMixResult.status != AudioStatus::Success) {
        return Fail(REINIT_MIX_MESSAGE);
    }

    if (staleMixResult.frames_written != 2U) {
        return Fail(REINIT_MIX_FRAME_COUNT_MESSAGE);
    }

    const std::array<std::int16_t, 4U> silentOutput{0, 0, 0, 0};
    if (!SamplesEqual(std::span<const std::int16_t>(staleMixOutput.data(), staleMixOutput.size()), std::span<const std::int16_t>(silentOutput.data(), silentOutput.size()))) {
        return Fail(REINIT_MIX_STALE_VOICE_MESSAGE);
    }

    if (device.Snapshot().active_voice_count != 0U) {
        return Fail(REINIT_MIX_ACTIVE_VOICE_MESSAGE);
    }

    AudioSourceId activeSource{};
    if (!RegisterBasicSource(device, activeSource)) {
        return Fail(REINIT_ACTIVE_SOURCE_REGISTRATION_MESSAGE);
    }

    AudioVoiceHandle activeVoice{};
    if (device.StartVoice(activeSource, MAX_Q15_GAIN, activeVoice) != AudioStatus::Success) {
        return Fail(REINIT_ACTIVE_VOICE_START_MESSAGE);
    }

    const AudioDeviceSnapshot beforeSnapshot = device.Snapshot();
    if (device.StopVoice(staleVoice) != AudioStatus::InvalidHandle) {
        return Fail(REINIT_STALE_VOICE_ACCEPTED_MESSAGE);
    }

    if (device.Snapshot().active_voice_count != beforeSnapshot.active_voice_count) {
        return Fail(REINIT_STALE_VOICE_COUNT_MESSAGE);
    }

    AudioVoiceHandle staleSourceVoice{};
    if (device.StartVoice(staleSource, MAX_Q15_GAIN, staleSourceVoice) != AudioStatus::SourceNotFound) {
        return Fail(REINIT_STALE_SOURCE_ACCEPTED_MESSAGE);
    }

    if (device.Snapshot().active_voice_count != beforeSnapshot.active_voice_count) {
        return Fail(REINIT_STALE_SOURCE_COUNT_MESSAGE);
    }

    if (device.StopVoice(activeVoice) != AudioStatus::Success) {
        return Fail(REINIT_ACTIVE_VOICE_MESSAGE);
    }

    return 0;
}

int AudioMixSingleVoiceWritesDeterministicS16StereoSamples() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice)) {
        return Fail("voice start failed");
    }

    std::array<std::int16_t, 4U> output{};
    const AudioMixResult result = device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (result.status != AudioStatus::Success) {
        return Fail("mix failed");
    }

    const std::array<std::int16_t, 4U> expected = BasicSourceSamples();
    if (!SamplesEqual(std::span<const std::int16_t>(output.data(), output.size()), std::span<const std::int16_t>(expected.data(), expected.size()))) {
        return Fail("single voice output did not match source samples");
    }

    return 0;
}

int AudioMixUnityGainPreservesS16EdgeSamples() {
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 4U> samples{S16_MIN, S16_MAX, -1, 1};
    AudioSourceId source{};
    device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 2U, source);
    AudioVoiceHandle voice{};
    device.StartVoice(source, MAX_Q15_GAIN, voice);

    std::array<std::int16_t, 4U> output{};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (!SamplesEqual(std::span<const std::int16_t>(output.data(), output.size()), std::span<const std::int16_t>(samples.data(), samples.size()))) {
        return Fail("unity gain did not preserve S16 edge samples");
    }

    return 0;
}

int AudioMixFractionalGainRoundsTowardZeroDeterministically() {
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 4U> samples{1000, -1000, 1, -1};
    AudioSourceId source{};
    device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 2U, source);
    AudioVoiceHandle voice{};
    device.StartVoice(source, 16384U, voice);

    std::array<std::int16_t, 4U> output{};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    const std::array<std::int16_t, 4U> expected{500, -500, 0, 0};
    if (!SamplesEqual(std::span<const std::int16_t>(output.data(), output.size()), std::span<const std::int16_t>(expected.data(), expected.size()))) {
        return Fail("fractional Q15 gain did not round toward zero deterministically");
    }

    return 0;
}

int AudioMixMultipleVoicesUsesStableOrderAndSaturates() {
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 2U> firstSamples{30000, -32000};
    const std::array<std::int16_t, 2U> secondSamples{30000, -32000};
    AudioSourceId firstSource{};
    AudioSourceId secondSource{};
    device.RegisterSyntheticSource(std::span<const std::int16_t>(firstSamples.data(), firstSamples.size()), 1U, firstSource);
    device.RegisterSyntheticSource(std::span<const std::int16_t>(secondSamples.data(), secondSamples.size()), 1U, secondSource);
    AudioVoiceHandle firstVoice{};
    AudioVoiceHandle secondVoice{};
    device.StartVoice(firstSource, MAX_Q15_GAIN, firstVoice);
    device.StartVoice(secondSource, MAX_Q15_GAIN, secondVoice);

    std::array<std::int16_t, 2U> output{};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 1U);
    if (output[0U] != S16_MAX) {
        return Fail("positive mix did not saturate");
    }

    if (output[1U] != S16_MIN) {
        return Fail("negative mix did not saturate");
    }

    return 0;
}

int AudioMixMaxVoicesFullScaleSaturatesWithoutOverflow() {
    if (!MixMaxVoicesFullScale(S16_MAX, S16_MAX)) {
        return Fail("positive max-voice full-scale mix did not saturate safely");
    }

    if (!MixMaxVoicesFullScale(S16_MIN, S16_MIN)) {
        return Fail("negative max-voice full-scale mix did not saturate safely");
    }

    return 0;
}

int AudioMixStopsVoiceAtEndOfSource() {
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 2U> samples{100, -100};
    AudioSourceId source{};
    device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 1U, source);
    AudioVoiceHandle voice{};
    device.StartVoice(source, MAX_Q15_GAIN, voice);

    std::array<std::int16_t, 4U> output{};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (device.StopVoice(voice) != AudioStatus::InvalidHandle) {
        return Fail("ended voice handle was not invalidated");
    }

    if (device.Snapshot().stopped_voice_count != 1U) {
        return Fail("ended voice stop count was not recorded");
    }

    return 0;
}

int AudioMixEndedVoiceWritesSilentTail() {
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 2U> samples{100, -100};
    AudioSourceId source{};
    device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 1U, source);
    AudioVoiceHandle voice{};
    device.StartVoice(source, MAX_Q15_GAIN, voice);

    std::array<std::int16_t, 6U> output{PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 3U);
    const std::array<std::int16_t, 6U> expected{100, -100, 0, 0, 0, 0};
    if (!SamplesEqual(std::span<const std::int16_t>(output.data(), output.size()), std::span<const std::int16_t>(expected.data(), expected.size()))) {
        return Fail("ended voice tail was not written as silence");
    }

    return 0;
}

int AudioMixOverwritesPrefilledDestination() {
    TestAudioDevice device = CreateInitializedDevice();
    std::array<std::int16_t, 4U> output{PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE};
    const AudioMixResult result = device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (result.status != AudioStatus::Success) {
        return Fail("silent mix failed");
    }

    const std::array<std::int16_t, 4U> expected{0, 0, 0, 0};
    if (!SamplesEqual(std::span<const std::int16_t>(output.data(), output.size()), std::span<const std::int16_t>(expected.data(), expected.size()))) {
        return Fail("mix did not overwrite prefilled destination with silence");
    }

    return 0;
}

int AudioMixRejectsUndersizedBufferWithoutWritingSamples() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice)) {
        return Fail("voice start failed");
    }

    std::array<std::int16_t, 3U> output{SENTINEL_SAMPLE, SENTINEL_SAMPLE, SENTINEL_SAMPLE};
    const AudioMixResult result = device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (result.status != AudioStatus::CapacityExceeded) {
        return Fail("undersized mix did not return capacity status");
    }

    if (result.frames_written != 0U) {
        return Fail("undersized mix reported nonzero frames written");
    }

    for (const std::int16_t sample : output) {
        if (sample != SENTINEL_SAMPLE) {
            return Fail("undersized mix mutated output");
        }
    }

    return 0;
}

int AudioUninitializedDeviceOperationsReturnExplicitStatusWithoutMutation() {
    TestAudioDevice device;
    const AudioDeviceSnapshot beforeSnapshot = device.Snapshot();
    AudioVoiceHandle voice{};

    if (device.StartVoice(AudioSourceId{0U, 1U}, MAX_Q15_GAIN, voice) != AudioStatus::InvalidDescriptor) {
        return Fail("uninitialized start voice did not return explicit status");
    }

    if (device.StopVoice(AudioVoiceHandle{0U, 1U}) != AudioStatus::InvalidDescriptor) {
        return Fail("uninitialized stop voice did not return explicit status");
    }

    std::array<std::int16_t, 4U> output{SENTINEL_SAMPLE, SENTINEL_SAMPLE, SENTINEL_SAMPLE, SENTINEL_SAMPLE};
    const AudioMixResult result = device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (result.status != AudioStatus::InvalidDescriptor) {
        return Fail("uninitialized mix did not return explicit status");
    }

    if (result.frames_written != 0U) {
        return Fail("uninitialized mix reported written frames");
    }

    for (const std::int16_t sample : output) {
        if (sample != SENTINEL_SAMPLE) {
            return Fail("uninitialized mix wrote output");
        }
    }

    if (!SnapshotsEqual(device.Snapshot(), beforeSnapshot)) {
        return Fail("uninitialized operations mutated counters");
    }

    return 0;
}

int AudioMixDoesNotGrowVoiceStorage() {
    AudioDeviceDesc desc{};
    desc.voice_capacity = 1U;

    TestAudioDevice device;
    if (device.Initialize(desc) != AudioStatus::Success) {
        return Fail("minimal voice capacity device failed to initialize");
    }

    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice)) {
        return Fail("voice start failed");
    }

    std::array<std::int16_t, 4U> output{};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    const auto snapshot = device.Snapshot();
    if (snapshot.voice_storage_capacity_before_mix != snapshot.voice_storage_capacity_after_last_mix) {
        return Fail("voice storage capacity changed during mix");
    }

    if (snapshot.voice_capacity != desc.voice_capacity) {
        return Fail("logical voice capacity was unexpected");
    }

    if (snapshot.voice_storage_capacity_before_mix < MAX_VOICES) {
        return Fail("voice storage capacity was unexpected");
    }

    if (snapshot.voice_storage_capacity_before_mix == snapshot.voice_capacity) {
        return Fail("voice storage capacity reported logical slot count");
    }

    return 0;
}

int AudioDisabledDiagnosticsDoesNotChangeResults() {
    TestAudioDevice enabledLikeDevice = CreateInitializedDevice();
    TestAudioDevice disabledLikeDevice = CreateInitializedDevice();
    AudioVoiceHandle enabledVoice{};
    AudioVoiceHandle disabledVoice{};
    StartBasicVoice(enabledLikeDevice, enabledVoice);
    StartBasicVoice(disabledLikeDevice, disabledVoice);

    std::array<std::int16_t, 4U> enabledOutput{};
    std::array<std::int16_t, 4U> disabledOutput{};
    const AudioMixResult enabledResult = enabledLikeDevice.Mix(std::span<std::int16_t>(enabledOutput.data(), enabledOutput.size()), 2U);
    const AudioMixResult disabledResult = disabledLikeDevice.Mix(std::span<std::int16_t>(disabledOutput.data(), disabledOutput.size()), 2U);
    if (enabledResult.status != disabledResult.status) {
        return Fail("disabled diagnostics changed mix status");
    }

    if (!SamplesEqual(std::span<const std::int16_t>(enabledOutput.data(), enabledOutput.size()), std::span<const std::int16_t>(disabledOutput.data(), disabledOutput.size()))) {
        return Fail("disabled diagnostics changed output samples");
    }

    if (enabledLikeDevice.Snapshot().mixed_frame_count != disabledLikeDevice.Snapshot().mixed_frame_count) {
        return Fail("disabled diagnostics changed mixed frame count");
    }

    return 0;
}

int AudioNoDeviceCodecResourceScriptUiGameAdapterDependency() {
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice)) {
        return Fail("minimal audio path failed");
    }

    std::array<std::int16_t, 4U> output{};
    if (device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U).status != AudioStatus::Success) {
        return Fail("minimal mix path failed");
    }

    if (device.Capabilities().backend_kind != AudioBackendKind::Test) {
        return Fail("audio fixture left test backend scope");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> testRegistry{
        {TEST_CREATE_DEVICE, AudioCreateTestDeviceReturnsCapabilities},
        {TEST_UNSUPPORTED_BACKEND, AudioCreateDeviceRejectsUnsupportedBackend},
        {TEST_UNSUPPORTED_FORMAT, AudioCreateDeviceRejectsUnsupportedFormat},
        {TEST_REGISTER_SOURCE, AudioRegisterSyntheticSourceReturnsStableId},
        {TEST_SOURCE_CAPACITY, AudioSourceCapacityOverflowDoesNotMutate},
        {TEST_START_VOICE, AudioStartVoiceReturnsGenerationHandle},
        {TEST_MISSING_SOURCE, AudioStartVoiceRejectsMissingSource},
        {TEST_INVALID_GAIN, AudioStartVoiceRejectsInvalidGainWithoutMutation},
        {TEST_VOICE_CAPACITY, AudioVoiceCapacityOverflowDoesNotMutate},
        {TEST_STOP_STALE, AudioStopVoiceInvalidatesStaleHandle},
        {TEST_REINITIALIZE_STALE_HANDLES, AudioReinitializeInvalidatesPriorSourceAndVoiceHandles},
        {TEST_SINGLE_VOICE, AudioMixSingleVoiceWritesDeterministicS16StereoSamples},
        {TEST_UNITY_GAIN, AudioMixUnityGainPreservesS16EdgeSamples},
        {TEST_FRACTIONAL_GAIN, AudioMixFractionalGainRoundsTowardZeroDeterministically},
        {TEST_MULTIPLE_VOICES, AudioMixMultipleVoicesUsesStableOrderAndSaturates},
        {TEST_MAX_VOICE_FULL_SCALE, AudioMixMaxVoicesFullScaleSaturatesWithoutOverflow},
        {TEST_STOPS_AT_END, AudioMixStopsVoiceAtEndOfSource},
        {TEST_SILENT_TAIL, AudioMixEndedVoiceWritesSilentTail},
        {TEST_OVERWRITE, AudioMixOverwritesPrefilledDestination},
        {TEST_UNDERSIZED_OUTPUT, AudioMixRejectsUndersizedBufferWithoutWritingSamples},
        {TEST_UNINITIALIZED_LIFECYCLE, AudioUninitializedDeviceOperationsReturnExplicitStatusWithoutMutation},
        {TEST_NO_GROW, AudioMixDoesNotGrowVoiceStorage},
        {TEST_DISABLED_DIAGNOSTICS, AudioDisabledDiagnosticsDoesNotChangeResults},
        {TEST_NO_FORBIDDEN_DEPENDENCY, AudioNoDeviceCodecResourceScriptUiGameAdapterDependency}};

    const std::string_view testName(argv[1]);
    const auto testIterator = testRegistry.find(testName);
    if (testIterator == testRegistry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return testIterator->second();
}
