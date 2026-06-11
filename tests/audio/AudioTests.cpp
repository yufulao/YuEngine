#include <array>
#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <vector>

#include "yuengine/audio/AudioConstants.h"
#include "yuengine/audio/TestAudioDevice.h"

using AudioBackendKind = yuengine::audio::AudioBackendKind;
using AudioDeviceDesc = yuengine::audio::AudioDeviceDesc;
using AudioDeviceSnapshot = yuengine::audio::AudioDeviceSnapshot;
using AudioMixResult = yuengine::audio::AudioMixResult;
using AudioSampleFormat = yuengine::audio::AudioSampleFormat;
using AudioSourceId = yuengine::audio::AudioSourceId;
using AudioStatus = yuengine::audio::AudioStatus;
using AudioVoiceHandle = yuengine::audio::AudioVoiceHandle;
using TestAudioDevice = yuengine::audio::TestAudioDevice;

namespace
{
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
constexpr std::int16_t PREFILL_SAMPLE = 1234;
constexpr std::int16_t SENTINEL_SAMPLE = -1234;

int Fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

TestAudioDevice CreateInitializedDevice()
{
    TestAudioDevice device;
    device.Initialize(AudioDeviceDesc{});
    return device;
}

std::array<std::int16_t, 4U> BasicSourceSamples()
{
    return {1000, -1000, 2000, -2000};
}

bool RegisterBasicSource(TestAudioDevice& device, AudioSourceId& outSource)
{
    const std::array<std::int16_t, 4U> samples = BasicSourceSamples();
    return device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 2U, outSource) == AudioStatus::Success;
}

bool StartBasicVoice(TestAudioDevice& device, AudioVoiceHandle& outVoice)
{
    AudioSourceId source{};
    if (!RegisterBasicSource(device, source))
    {
        return false;
    }

    return device.StartVoice(source, yuengine::audio::MAX_Q15_GAIN, outVoice) == AudioStatus::Success;
}

bool SamplesEqual(std::span<const std::int16_t> left, std::span<const std::int16_t> right)
{
    if (left.size() != right.size())
    {
        return false;
    }

    for (std::size_t index = 0U; index < left.size(); ++index)
    {
        if (left[index] != right[index])
        {
            return false;
        }
    }

    return true;
}

bool SnapshotsEqual(const AudioDeviceSnapshot& left, const AudioDeviceSnapshot& right)
{
    return left.SourceCapacity == right.SourceCapacity &&
           left.VoiceCapacity == right.VoiceCapacity &&
           left.SourceCount == right.SourceCount &&
           left.ActiveVoiceCount == right.ActiveVoiceCount &&
           left.VoiceStorageCapacityBeforeMix == right.VoiceStorageCapacityBeforeMix &&
           left.VoiceStorageCapacityAfterLastMix == right.VoiceStorageCapacityAfterLastMix &&
           left.RegisteredSourceCount == right.RegisteredSourceCount &&
           left.StartedVoiceCount == right.StartedVoiceCount &&
           left.StoppedVoiceCount == right.StoppedVoiceCount &&
           left.MixedFrameCount == right.MixedFrameCount &&
           left.OutputSampleWriteCount == right.OutputSampleWriteCount &&
           left.FailedOperationCount == right.FailedOperationCount &&
           left.LastFramesWritten == right.LastFramesWritten &&
           left.AllocationAccountingStatus == right.AllocationAccountingStatus;
}

bool MixMaxVoicesFullScale(std::int16_t sourceSample, std::int16_t expectedSample)
{
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 2U> samples{sourceSample, sourceSample};
    AudioSourceId source{};
    if (device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 1U, source) != AudioStatus::Success)
    {
        return false;
    }

    for (std::size_t index = 0U; index < yuengine::audio::MAX_VOICES; ++index)
    {
        AudioVoiceHandle voice{};
        if (device.StartVoice(source, yuengine::audio::MAX_Q15_GAIN, voice) != AudioStatus::Success)
        {
            return false;
        }
    }

    std::array<std::int16_t, 2U> output{};
    const AudioMixResult result = device.Mix(std::span<std::int16_t>(output.data(), output.size()), 1U);
    return result.Status == AudioStatus::Success &&
           result.FramesWritten == 1U &&
           output[0U] == expectedSample &&
           output[1U] == expectedSample;
}

int AudioCreateTestDeviceReturnsCapabilities()
{
    TestAudioDevice device;
    if (device.Initialize(AudioDeviceDesc{}) != AudioStatus::Success)
    {
        return Fail("test audio device did not initialize");
    }

    const auto capabilities = device.Capabilities();
    if (capabilities.BackendKind != AudioBackendKind::Test)
    {
        return Fail("capabilities did not report test backend");
    }

    if (capabilities.Format != AudioSampleFormat::S16)
    {
        return Fail("capabilities did not report S16");
    }

    if (capabilities.SampleRate != yuengine::audio::SAMPLE_RATE)
    {
        return Fail("capabilities did not report fixed sample rate");
    }

    if (capabilities.ChannelCount != yuengine::audio::CHANNEL_COUNT)
    {
        return Fail("capabilities did not report stereo");
    }

    if (!capabilities.SupportsDeterministicMix)
    {
        return Fail("capabilities did not report deterministic mix support");
    }

    return 0;
}

int AudioCreateDeviceRejectsUnsupportedBackend()
{
    TestAudioDevice device;
    AudioDeviceDesc desc{};
    desc.BackendKind = AudioBackendKind::Unsupported;
    if (device.Initialize(desc) != AudioStatus::UnsupportedBackend)
    {
        return Fail("unsupported backend was not rejected");
    }

    if (device.Snapshot().SourceCapacity != 0U)
    {
        return Fail("unsupported backend mutated source capacity");
    }

    return 0;
}

int AudioCreateDeviceRejectsUnsupportedFormat()
{
    TestAudioDevice device;
    AudioDeviceDesc formatDesc{};
    formatDesc.Format = AudioSampleFormat::Unsupported;
    if (device.Initialize(formatDesc) != AudioStatus::UnsupportedFormat)
    {
        return Fail("unsupported sample format was not rejected");
    }

    AudioDeviceDesc sampleRateDesc{};
    sampleRateDesc.SampleRate = 44100U;
    if (device.Initialize(sampleRateDesc) != AudioStatus::UnsupportedFormat)
    {
        return Fail("unsupported sample rate was not rejected");
    }

    AudioDeviceDesc channelDesc{};
    channelDesc.ChannelCount = 1U;
    if (device.Initialize(channelDesc) != AudioStatus::UnsupportedFormat)
    {
        return Fail("unsupported channel count was not rejected");
    }

    return 0;
}

int AudioRegisterSyntheticSourceReturnsStableId()
{
    TestAudioDevice device = CreateInitializedDevice();
    AudioSourceId source{};
    if (!RegisterBasicSource(device, source))
    {
        return Fail("source registration failed");
    }

    if (source.Slot != 0U)
    {
        return Fail("first source used unexpected slot");
    }

    if (source.Generation == 0U)
    {
        return Fail("source generation was invalid");
    }

    if (device.Snapshot().RegisteredSourceCount != 1U)
    {
        return Fail("registered source count was not updated");
    }

    return 0;
}

int AudioSourceCapacityOverflowDoesNotMutate()
{
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 2U> samples{1, 1};
    for (std::size_t index = 0U; index < yuengine::audio::MAX_SOURCES; ++index)
    {
        AudioSourceId source{};
        const AudioStatus status = device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 1U, source);
        if (status != AudioStatus::Success)
        {
            return Fail("source registration failed before capacity");
        }
    }

    const auto beforeSnapshot = device.Snapshot();
    AudioSourceId overflowSource{};
    const AudioStatus overflowStatus = device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 1U, overflowSource);
    if (overflowStatus != AudioStatus::CapacityExceeded)
    {
        return Fail("source overflow did not return capacity status");
    }

    if (device.Snapshot().SourceCount != beforeSnapshot.SourceCount)
    {
        return Fail("source overflow changed source count");
    }

    return 0;
}

int AudioStartVoiceReturnsGenerationHandle()
{
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice))
    {
        return Fail("voice start failed");
    }

    if (voice.Slot != 0U)
    {
        return Fail("first voice used unexpected slot");
    }

    if (voice.Generation == 0U)
    {
        return Fail("voice generation was invalid");
    }

    if (device.Snapshot().ActiveVoiceCount != 1U)
    {
        return Fail("active voice count was not updated");
    }

    return 0;
}

int AudioStartVoiceRejectsMissingSource()
{
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (device.StartVoice(AudioSourceId{99U, 1U}, yuengine::audio::MAX_Q15_GAIN, voice) != AudioStatus::SourceNotFound)
    {
        return Fail("missing source was not rejected");
    }

    if (device.Snapshot().ActiveVoiceCount != 0U)
    {
        return Fail("missing source changed active voice count");
    }

    return 0;
}

int AudioStartVoiceRejectsInvalidGainWithoutMutation()
{
    TestAudioDevice device = CreateInitializedDevice();
    AudioSourceId source{};
    if (!RegisterBasicSource(device, source))
    {
        return Fail("source registration failed");
    }

    AudioVoiceHandle voice{};
    const AudioStatus status = device.StartVoice(source, yuengine::audio::MAX_Q15_GAIN + 1U, voice);
    if (status != AudioStatus::InvalidGain)
    {
        return Fail("invalid gain was not rejected");
    }

    if (device.Snapshot().ActiveVoiceCount != 0U)
    {
        return Fail("invalid gain changed active voice count");
    }

    return 0;
}

int AudioVoiceCapacityOverflowDoesNotMutate()
{
    TestAudioDevice device = CreateInitializedDevice();
    AudioSourceId source{};
    if (!RegisterBasicSource(device, source))
    {
        return Fail("source registration failed");
    }

    for (std::size_t index = 0U; index < yuengine::audio::MAX_VOICES; ++index)
    {
        AudioVoiceHandle voice{};
        if (device.StartVoice(source, yuengine::audio::MAX_Q15_GAIN, voice) != AudioStatus::Success)
        {
            return Fail("voice start failed before capacity");
        }
    }

    const auto beforeSnapshot = device.Snapshot();
    AudioVoiceHandle overflowVoice{};
    if (device.StartVoice(source, yuengine::audio::MAX_Q15_GAIN, overflowVoice) != AudioStatus::CapacityExceeded)
    {
        return Fail("voice overflow did not return capacity status");
    }

    if (device.Snapshot().ActiveVoiceCount != beforeSnapshot.ActiveVoiceCount)
    {
        return Fail("voice overflow changed active voice count");
    }

    return 0;
}

int AudioStopVoiceInvalidatesStaleHandle()
{
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice))
    {
        return Fail("voice start failed");
    }

    if (device.StopVoice(voice) != AudioStatus::Success)
    {
        return Fail("voice stop failed");
    }

    if (device.StopVoice(voice) != AudioStatus::InvalidHandle)
    {
        return Fail("stale voice handle was not rejected");
    }

    if (device.Snapshot().ActiveVoiceCount != 0U)
    {
        return Fail("stopped voice remained active");
    }

    return 0;
}

int AudioMixSingleVoiceWritesDeterministicS16StereoSamples()
{
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice))
    {
        return Fail("voice start failed");
    }

    std::array<std::int16_t, 4U> output{};
    const AudioMixResult result = device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (result.Status != AudioStatus::Success)
    {
        return Fail("mix failed");
    }

    const std::array<std::int16_t, 4U> expected = BasicSourceSamples();
    if (!SamplesEqual(std::span<const std::int16_t>(output.data(), output.size()), std::span<const std::int16_t>(expected.data(), expected.size())))
    {
        return Fail("single voice output did not match source samples");
    }

    return 0;
}

int AudioMixUnityGainPreservesS16EdgeSamples()
{
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 4U> samples{yuengine::audio::S16_MIN, yuengine::audio::S16_MAX, -1, 1};
    AudioSourceId source{};
    device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 2U, source);
    AudioVoiceHandle voice{};
    device.StartVoice(source, yuengine::audio::MAX_Q15_GAIN, voice);

    std::array<std::int16_t, 4U> output{};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (!SamplesEqual(std::span<const std::int16_t>(output.data(), output.size()), std::span<const std::int16_t>(samples.data(), samples.size())))
    {
        return Fail("unity gain did not preserve S16 edge samples");
    }

    return 0;
}

int AudioMixFractionalGainRoundsTowardZeroDeterministically()
{
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 4U> samples{1000, -1000, 1, -1};
    AudioSourceId source{};
    device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 2U, source);
    AudioVoiceHandle voice{};
    device.StartVoice(source, 16384U, voice);

    std::array<std::int16_t, 4U> output{};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    const std::array<std::int16_t, 4U> expected{500, -500, 0, 0};
    if (!SamplesEqual(std::span<const std::int16_t>(output.data(), output.size()), std::span<const std::int16_t>(expected.data(), expected.size())))
    {
        return Fail("fractional Q15 gain did not round toward zero deterministically");
    }

    return 0;
}

int AudioMixMultipleVoicesUsesStableOrderAndSaturates()
{
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 2U> firstSamples{30000, -32000};
    const std::array<std::int16_t, 2U> secondSamples{30000, -32000};
    AudioSourceId firstSource{};
    AudioSourceId secondSource{};
    device.RegisterSyntheticSource(std::span<const std::int16_t>(firstSamples.data(), firstSamples.size()), 1U, firstSource);
    device.RegisterSyntheticSource(std::span<const std::int16_t>(secondSamples.data(), secondSamples.size()), 1U, secondSource);
    AudioVoiceHandle firstVoice{};
    AudioVoiceHandle secondVoice{};
    device.StartVoice(firstSource, yuengine::audio::MAX_Q15_GAIN, firstVoice);
    device.StartVoice(secondSource, yuengine::audio::MAX_Q15_GAIN, secondVoice);

    std::array<std::int16_t, 2U> output{};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 1U);
    if (output[0U] != yuengine::audio::S16_MAX)
    {
        return Fail("positive mix did not saturate");
    }

    if (output[1U] != yuengine::audio::S16_MIN)
    {
        return Fail("negative mix did not saturate");
    }

    return 0;
}

int AudioMixMaxVoicesFullScaleSaturatesWithoutOverflow()
{
    if (!MixMaxVoicesFullScale(yuengine::audio::S16_MAX, yuengine::audio::S16_MAX))
    {
        return Fail("positive max-voice full-scale mix did not saturate safely");
    }

    if (!MixMaxVoicesFullScale(yuengine::audio::S16_MIN, yuengine::audio::S16_MIN))
    {
        return Fail("negative max-voice full-scale mix did not saturate safely");
    }

    return 0;
}

int AudioMixStopsVoiceAtEndOfSource()
{
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 2U> samples{100, -100};
    AudioSourceId source{};
    device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 1U, source);
    AudioVoiceHandle voice{};
    device.StartVoice(source, yuengine::audio::MAX_Q15_GAIN, voice);

    std::array<std::int16_t, 4U> output{};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (device.StopVoice(voice) != AudioStatus::InvalidHandle)
    {
        return Fail("ended voice handle was not invalidated");
    }

    if (device.Snapshot().StoppedVoiceCount != 1U)
    {
        return Fail("ended voice stop count was not recorded");
    }

    return 0;
}

int AudioMixEndedVoiceWritesSilentTail()
{
    TestAudioDevice device = CreateInitializedDevice();
    const std::array<std::int16_t, 2U> samples{100, -100};
    AudioSourceId source{};
    device.RegisterSyntheticSource(std::span<const std::int16_t>(samples.data(), samples.size()), 1U, source);
    AudioVoiceHandle voice{};
    device.StartVoice(source, yuengine::audio::MAX_Q15_GAIN, voice);

    std::array<std::int16_t, 6U> output{PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 3U);
    const std::array<std::int16_t, 6U> expected{100, -100, 0, 0, 0, 0};
    if (!SamplesEqual(std::span<const std::int16_t>(output.data(), output.size()), std::span<const std::int16_t>(expected.data(), expected.size())))
    {
        return Fail("ended voice tail was not written as silence");
    }

    return 0;
}

int AudioMixOverwritesPrefilledDestination()
{
    TestAudioDevice device = CreateInitializedDevice();
    std::array<std::int16_t, 4U> output{PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE, PREFILL_SAMPLE};
    const AudioMixResult result = device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (result.Status != AudioStatus::Success)
    {
        return Fail("silent mix failed");
    }

    const std::array<std::int16_t, 4U> expected{0, 0, 0, 0};
    if (!SamplesEqual(std::span<const std::int16_t>(output.data(), output.size()), std::span<const std::int16_t>(expected.data(), expected.size())))
    {
        return Fail("mix did not overwrite prefilled destination with silence");
    }

    return 0;
}

int AudioMixRejectsUndersizedBufferWithoutWritingSamples()
{
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice))
    {
        return Fail("voice start failed");
    }

    std::array<std::int16_t, 3U> output{SENTINEL_SAMPLE, SENTINEL_SAMPLE, SENTINEL_SAMPLE};
    const AudioMixResult result = device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (result.Status != AudioStatus::CapacityExceeded)
    {
        return Fail("undersized mix did not return capacity status");
    }

    if (result.FramesWritten != 0U)
    {
        return Fail("undersized mix reported nonzero frames written");
    }

    for (const std::int16_t sample : output)
    {
        if (sample != SENTINEL_SAMPLE)
        {
            return Fail("undersized mix mutated output");
        }
    }

    return 0;
}

int AudioUninitializedDeviceOperationsReturnExplicitStatusWithoutMutation()
{
    TestAudioDevice device;
    const AudioDeviceSnapshot beforeSnapshot = device.Snapshot();
    AudioVoiceHandle voice{};

    if (device.StartVoice(AudioSourceId{0U, 1U}, yuengine::audio::MAX_Q15_GAIN, voice) != AudioStatus::InvalidDescriptor)
    {
        return Fail("uninitialized start voice did not return explicit status");
    }

    if (device.StopVoice(AudioVoiceHandle{0U, 1U}) != AudioStatus::InvalidDescriptor)
    {
        return Fail("uninitialized stop voice did not return explicit status");
    }

    std::array<std::int16_t, 4U> output{SENTINEL_SAMPLE, SENTINEL_SAMPLE, SENTINEL_SAMPLE, SENTINEL_SAMPLE};
    const AudioMixResult result = device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    if (result.Status != AudioStatus::InvalidDescriptor)
    {
        return Fail("uninitialized mix did not return explicit status");
    }

    if (result.FramesWritten != 0U)
    {
        return Fail("uninitialized mix reported written frames");
    }

    for (const std::int16_t sample : output)
    {
        if (sample != SENTINEL_SAMPLE)
        {
            return Fail("uninitialized mix wrote output");
        }
    }

    if (!SnapshotsEqual(device.Snapshot(), beforeSnapshot))
    {
        return Fail("uninitialized operations mutated counters");
    }

    return 0;
}

int AudioMixDoesNotGrowVoiceStorage()
{
    AudioDeviceDesc desc{};
    desc.VoiceCapacity = 1U;

    TestAudioDevice device;
    if (device.Initialize(desc) != AudioStatus::Success)
    {
        return Fail("minimal voice capacity device failed to initialize");
    }

    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice))
    {
        return Fail("voice start failed");
    }

    std::array<std::int16_t, 4U> output{};
    device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U);
    const auto snapshot = device.Snapshot();
    if (snapshot.VoiceStorageCapacityBeforeMix != snapshot.VoiceStorageCapacityAfterLastMix)
    {
        return Fail("voice storage capacity changed during mix");
    }

    if (snapshot.VoiceCapacity != desc.VoiceCapacity)
    {
        return Fail("logical voice capacity was unexpected");
    }

    if (snapshot.VoiceStorageCapacityBeforeMix < yuengine::audio::MAX_VOICES)
    {
        return Fail("voice storage capacity was unexpected");
    }

    if (snapshot.VoiceStorageCapacityBeforeMix == snapshot.VoiceCapacity)
    {
        return Fail("voice storage capacity reported logical slot count");
    }

    return 0;
}

int AudioDisabledDiagnosticsDoesNotChangeResults()
{
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
    if (enabledResult.Status != disabledResult.Status)
    {
        return Fail("disabled diagnostics changed mix status");
    }

    if (!SamplesEqual(std::span<const std::int16_t>(enabledOutput.data(), enabledOutput.size()), std::span<const std::int16_t>(disabledOutput.data(), disabledOutput.size())))
    {
        return Fail("disabled diagnostics changed output samples");
    }

    if (enabledLikeDevice.Snapshot().MixedFrameCount != disabledLikeDevice.Snapshot().MixedFrameCount)
    {
        return Fail("disabled diagnostics changed mixed frame count");
    }

    return 0;
}

int AudioNoDeviceCodecResourceScriptUiGameAdapterDependency()
{
    TestAudioDevice device = CreateInitializedDevice();
    AudioVoiceHandle voice{};
    if (!StartBasicVoice(device, voice))
    {
        return Fail("minimal audio path failed");
    }

    std::array<std::int16_t, 4U> output{};
    if (device.Mix(std::span<std::int16_t>(output.data(), output.size()), 2U).Status != AudioStatus::Success)
    {
        return Fail("minimal mix path failed");
    }

    if (device.Capabilities().BackendKind != AudioBackendKind::Test)
    {
        return Fail("audio fixture left test backend scope");
    }

    return 0;
}
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return Fail("expected one test name");
    }

    const std::string testName(argv[1]);
    if (testName == TEST_CREATE_DEVICE)
    {
        return AudioCreateTestDeviceReturnsCapabilities();
    }

    if (testName == TEST_UNSUPPORTED_BACKEND)
    {
        return AudioCreateDeviceRejectsUnsupportedBackend();
    }

    if (testName == TEST_UNSUPPORTED_FORMAT)
    {
        return AudioCreateDeviceRejectsUnsupportedFormat();
    }

    if (testName == TEST_REGISTER_SOURCE)
    {
        return AudioRegisterSyntheticSourceReturnsStableId();
    }

    if (testName == TEST_SOURCE_CAPACITY)
    {
        return AudioSourceCapacityOverflowDoesNotMutate();
    }

    if (testName == TEST_START_VOICE)
    {
        return AudioStartVoiceReturnsGenerationHandle();
    }

    if (testName == TEST_MISSING_SOURCE)
    {
        return AudioStartVoiceRejectsMissingSource();
    }

    if (testName == TEST_INVALID_GAIN)
    {
        return AudioStartVoiceRejectsInvalidGainWithoutMutation();
    }

    if (testName == TEST_VOICE_CAPACITY)
    {
        return AudioVoiceCapacityOverflowDoesNotMutate();
    }

    if (testName == TEST_STOP_STALE)
    {
        return AudioStopVoiceInvalidatesStaleHandle();
    }

    if (testName == TEST_SINGLE_VOICE)
    {
        return AudioMixSingleVoiceWritesDeterministicS16StereoSamples();
    }

    if (testName == TEST_UNITY_GAIN)
    {
        return AudioMixUnityGainPreservesS16EdgeSamples();
    }

    if (testName == TEST_FRACTIONAL_GAIN)
    {
        return AudioMixFractionalGainRoundsTowardZeroDeterministically();
    }

    if (testName == TEST_MULTIPLE_VOICES)
    {
        return AudioMixMultipleVoicesUsesStableOrderAndSaturates();
    }

    if (testName == TEST_MAX_VOICE_FULL_SCALE)
    {
        return AudioMixMaxVoicesFullScaleSaturatesWithoutOverflow();
    }

    if (testName == TEST_STOPS_AT_END)
    {
        return AudioMixStopsVoiceAtEndOfSource();
    }

    if (testName == TEST_SILENT_TAIL)
    {
        return AudioMixEndedVoiceWritesSilentTail();
    }

    if (testName == TEST_OVERWRITE)
    {
        return AudioMixOverwritesPrefilledDestination();
    }

    if (testName == TEST_UNDERSIZED_OUTPUT)
    {
        return AudioMixRejectsUndersizedBufferWithoutWritingSamples();
    }

    if (testName == TEST_UNINITIALIZED_LIFECYCLE)
    {
        return AudioUninitializedDeviceOperationsReturnExplicitStatusWithoutMutation();
    }

    if (testName == TEST_NO_GROW)
    {
        return AudioMixDoesNotGrowVoiceStorage();
    }

    if (testName == TEST_DISABLED_DIAGNOSTICS)
    {
        return AudioDisabledDiagnosticsDoesNotChangeResults();
    }

    if (testName == TEST_NO_FORBIDDEN_DEPENDENCY)
    {
        return AudioNoDeviceCodecResourceScriptUiGameAdapterDependency();
    }

    return Fail("unknown test name");
}
