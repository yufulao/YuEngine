// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Src/AudioCallbackDeviceWindows.cpp

#include "YuEngine/Audio/AudioCallbackDevice.h"

#include "AudioCallbackDeviceWindowsInternal.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <new>
#include <utility>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <audioclient.h>
#include <xaudio2.h>
#endif

namespace yuengine::audio {
namespace {
constexpr std::uint16_t BITS_PER_SAMPLE = 16U;
constexpr std::uint16_t BYTES_PER_SAMPLE = BITS_PER_SAMPLE / 8U;
constexpr std::uint64_t NO_COMPLETED_CALLBACKS = 0U;
constexpr std::size_t MAX_CALLBACK_SAMPLE_COUNT = AudioCallbackDeviceDesc::MAX_FRAMES_PER_BUFFER * CHANNEL_COUNT;
constexpr std::uint32_t CALLBACK_WAIT_POLL_MILLISECONDS = 1U;
AudioCallbackDeviceBackendTestConfig s_backend_test_config{};

AudioStatus ValidateCallbackDesc(const AudioCallbackDeviceDesc &desc) {
    if (desc.backend_kind != AudioBackendKind::Callback) {
        return AudioStatus::UnsupportedBackend;
    }

    if (desc.format != AudioSampleFormat::Signed16) {
        return AudioStatus::UnsupportedFormat;
    }

    if (desc.sample_rate != SAMPLE_RATE) {
        return AudioStatus::UnsupportedFormat;
    }

    if (desc.channel_count != CHANNEL_COUNT) {
        return AudioStatus::UnsupportedFormat;
    }

    if (desc.buffer_count < AudioCallbackDeviceDesc::MIN_BUFFER_COUNT) {
        return AudioStatus::InvalidDescriptor;
    }

    if (desc.buffer_count > AudioCallbackDeviceDesc::MAX_BUFFER_COUNT) {
        return AudioStatus::CapacityExceeded;
    }

    if (desc.frames_per_buffer < AudioCallbackDeviceDesc::MIN_FRAMES_PER_BUFFER) {
        return AudioStatus::InvalidDescriptor;
    }

    if (desc.frames_per_buffer > AudioCallbackDeviceDesc::MAX_FRAMES_PER_BUFFER) {
        return AudioStatus::CapacityExceeded;
    }

    return AudioStatus::Success;
}

std::size_t RequiredSampleCount(const AudioCallbackDeviceDesc &desc) {
    return desc.frames_per_buffer * static_cast<std::size_t>(desc.channel_count);
}

AudioCallbackDeviceBackendTestConfig CurrentBackendTestConfig() {
    return s_backend_test_config;
}

#if defined(_WIN32)
AudioStatus MapXAudio2InitializeStatus(HRESULT native_result) {
    if (native_result == XAUDIO2_E_DEVICE_INVALIDATED) {
        return AudioStatus::DeviceUnavailable;
    }

    if (native_result == AUDCLNT_E_DEVICE_INVALIDATED) {
        return AudioStatus::DeviceUnavailable;
    }

    if (native_result == AUDCLNT_E_ENDPOINT_CREATE_FAILED) {
        return AudioStatus::DeviceUnavailable;
    }

    if (native_result == AUDCLNT_E_SERVICE_NOT_RUNNING) {
        return AudioStatus::DeviceUnavailable;
    }

    if (native_result == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)) {
        return AudioStatus::DeviceUnavailable;
    }

    if (native_result == HRESULT_FROM_WIN32(ERROR_DEVICE_NOT_CONNECTED)) {
        return AudioStatus::DeviceUnavailable;
    }

    return AudioStatus::BackendError;
}
#endif

}

struct AudioCallbackBufferSlot final {
    std::array<std::int16_t, MAX_CALLBACK_SAMPLE_COUNT> samples{};
    AudioCallbackCompletion pending_completion{};
    std::uint64_t sequence = 0U;
    std::atomic_bool queued{false};
    std::atomic_bool completion_pending{false};
};

struct AudioCallbackDeviceState final {
#if defined(_WIN32)
    struct VoiceCallback final : public IXAudio2VoiceCallback {
        AudioCallbackDeviceState *owner = nullptr;

        void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 bytes_required) override {
            (void)bytes_required;
        }

        void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() override {
        }

        void STDMETHODCALLTYPE OnStreamEnd() override {
            if (owner == nullptr) {
                return;
            }

            owner->OnStreamEnd();
        }

        void STDMETHODCALLTYPE OnBufferStart(void *buffer_context) override {
            (void)buffer_context;
        }

        void STDMETHODCALLTYPE OnBufferEnd(void *buffer_context) override {
            if (owner == nullptr) {
                return;
            }

            owner->OnBufferEnd(buffer_context);
        }

        void STDMETHODCALLTYPE OnLoopEnd(void *buffer_context) override {
            (void)buffer_context;
        }

        void STDMETHODCALLTYPE OnVoiceError(void *buffer_context, HRESULT error) override {
            (void)buffer_context;
            (void)error;

            if (owner == nullptr) {
                return;
            }

            owner->OnVoiceError();
        }
    };
#endif

    AudioCallbackDeviceState() {
#if defined(_WIN32)
        callback.owner = this;
#endif
    }

    AudioCallbackDeviceDesc desc{};
    AudioCallbackSnapshot snapshot{};
    std::array<AudioCallbackBufferSlot, AudioCallbackDeviceDesc::MAX_BUFFER_COUNT> buffers{};
    std::array<AudioCallbackCompletion, AudioCallbackDeviceDesc::MAX_BUFFER_COUNT> completions{};
    std::size_t completion_count = 0U;
    mutable std::mutex mutex{};
    std::condition_variable completion_signal{};
    std::atomic<std::uint64_t> pending_shutdown_callback_count{0U};
    std::atomic<std::uint64_t> pending_failed_callback_count{0U};
    std::uint64_t next_sequence = 1U;
    bool uses_controlled_backend = false;
#if defined(_WIN32)
    IXAudio2 *engine = nullptr;
    IXAudio2MasteringVoice *mastering_voice = nullptr;
    IXAudio2SourceVoice *source_voice = nullptr;
    VoiceCallback callback{};
    bool com_initialized = false;
#endif

    void ClearSubmissionCapacityFailure() {
        snapshot.last_failed_submission_sequence = 0U;
        snapshot.last_failed_submission_buffer_capacity = 0U;
        snapshot.last_failed_submission_queued_buffer_count = 0U;
        snapshot.last_failed_submission_frame_count = 0U;
        snapshot.last_failed_submission_sample_count = 0U;
        snapshot.last_required_queued_buffer_count = 0U;
    }

    AudioStatus SetLastStatus(AudioStatus status) {
        ClearSubmissionCapacityFailure();
        snapshot.last_status = status;
        return status;
    }

    AudioStatus RecordSubmissionCapacityFailure(std::size_t frame_count, std::size_t sample_count) {
        ++snapshot.failed_submission_count;
        snapshot.last_failed_submission_sequence = next_sequence;
        snapshot.last_failed_submission_buffer_capacity = snapshot.buffer_capacity;
        snapshot.last_failed_submission_queued_buffer_count = snapshot.queued_buffer_count;
        snapshot.last_failed_submission_frame_count = frame_count;
        snapshot.last_failed_submission_sample_count = sample_count;
        snapshot.last_required_queued_buffer_count = snapshot.queued_buffer_count + 1U;
        snapshot.last_status = AudioStatus::CapacityExceeded;
        return AudioStatus::CapacityExceeded;
    }

    void MergeCallbackEventsLocked() {
        for (std::size_t index = 0U; index < desc.buffer_count; ++index) {
            AudioCallbackBufferSlot &slot = buffers[index];
            if (!slot.completion_pending.exchange(false, std::memory_order_acq_rel)) {
                continue;
            }

            slot.queued.store(false, std::memory_order_release);
            if (snapshot.queued_buffer_count > 0U) {
                --snapshot.queued_buffer_count;
            }

            if (completion_count >= completions.size()) {
                ++snapshot.failed_callback_count;
                SetLastStatus(AudioStatus::CallbackFailed);
                continue;
            }

            completions[completion_count] = slot.pending_completion;
            ++completion_count;
            ++snapshot.completed_callback_count;
            SetLastStatus(AudioStatus::Success);
        }

        const std::uint64_t shutdown_count = pending_shutdown_callback_count.exchange(0U, std::memory_order_acq_rel);
        snapshot.shutdown_callback_count += shutdown_count;

        const std::uint64_t failed_count = pending_failed_callback_count.exchange(0U, std::memory_order_acq_rel);
        if (failed_count == 0U) {
            return;
        }

        snapshot.failed_callback_count += failed_count;
        SetLastStatus(AudioStatus::CallbackFailed);
    }

    void ReleaseNativeObjects() {
#if defined(_WIN32)
        if (source_voice != nullptr) {
            source_voice->Stop(0U);
            source_voice->FlushSourceBuffers();
            source_voice->DestroyVoice();
            source_voice = nullptr;
        }

        if (mastering_voice != nullptr) {
            mastering_voice->DestroyVoice();
            mastering_voice = nullptr;
        }

        if (engine != nullptr) {
            engine->StopEngine();
            engine->Release();
            engine = nullptr;
        }

        if (com_initialized) {
            CoUninitialize();
            com_initialized = false;
        }
#endif
    }

    void OnStreamEnd() {
        pending_shutdown_callback_count.fetch_add(1U, std::memory_order_release);
    }

    void OnBufferEnd(void *buffer_context) {
        AudioCallbackBufferSlot *slot = static_cast<AudioCallbackBufferSlot *>(buffer_context);
        if (slot == nullptr) {
            OnVoiceError();
            return;
        }

        slot->pending_completion.status = AudioStatus::Success;
        slot->pending_completion.sequence = slot->sequence;
        slot->pending_completion.buffer_slot = static_cast<std::size_t>(slot - buffers.data());
        slot->pending_completion.frame_count = desc.frames_per_buffer;
        slot->completion_pending.store(true, std::memory_order_release);
    }

    void OnVoiceError() {
        pending_failed_callback_count.fetch_add(1U, std::memory_order_release);
    }
};

void SetAudioCallbackDeviceBackendTestConfig(const AudioCallbackDeviceBackendTestConfig &config) {
    s_backend_test_config = config;
}

void ClearAudioCallbackDeviceBackendTestConfig() {
    s_backend_test_config = AudioCallbackDeviceBackendTestConfig{};
}

AudioStatus RollBackFailedSubmit(AudioCallbackDeviceState *state, AudioCallbackBufferSlot *slot, AudioStatus status) {
    std::lock_guard<std::mutex> lock(state->mutex);
    slot->queued.store(false, std::memory_order_release);
    if (state->snapshot.queued_buffer_count > 0U) {
        --state->snapshot.queued_buffer_count;
    }

    if (state->snapshot.submitted_buffer_count > 0U) {
        --state->snapshot.submitted_buffer_count;
    }

    ++state->snapshot.failed_submission_count;
    return state->SetLastStatus(status);
}

AudioCallbackDevice::AudioCallbackDevice()
    : state_(nullptr) {
}

AudioCallbackDevice::~AudioCallbackDevice() {
    Shutdown();
    delete state_;
    state_ = nullptr;
}

AudioStatus AudioCallbackDevice::Initialize(const AudioCallbackDeviceDesc &desc) {
    const AudioStatus desc_status = ValidateCallbackDesc(desc);
    if (desc_status != AudioStatus::Success) {
        return desc_status;
    }

    if (state_ != nullptr) {
        if (state_->snapshot.initialized && !state_->snapshot.shutdown) {
            return state_->SetLastStatus(AudioStatus::AlreadyInitialized);
        }

        delete state_;
        state_ = nullptr;
    }

    state_ = new (std::nothrow) AudioCallbackDeviceState();
    if (state_ == nullptr) {
        return AudioStatus::AllocationFailure;
    }

    state_->desc = desc;
    state_->snapshot = AudioCallbackSnapshot{};
    state_->snapshot.buffer_capacity = desc.buffer_count;
    state_->snapshot.frames_per_buffer = desc.frames_per_buffer;
    state_->snapshot.sample_rate = desc.sample_rate;
    state_->snapshot.channel_count = desc.channel_count;
    state_->snapshot.setup_allocation_count = desc.buffer_count + 2U;

    const AudioCallbackDeviceBackendTestConfig backend_test_config = CurrentBackendTestConfig();
    if (backend_test_config.enabled) {
        state_->uses_controlled_backend = true;
        if (backend_test_config.initialize_status != AudioStatus::Success) {
            return state_->SetLastStatus(backend_test_config.initialize_status);
        }

        state_->snapshot.initialized = true;
        return state_->SetLastStatus(AudioStatus::Success);
    }

#if defined(_WIN32)
    HRESULT native_result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool owns_com_apartment = SUCCEEDED(native_result);
    if (FAILED(native_result) && native_result != RPC_E_CHANGED_MODE) {
        state_->ReleaseNativeObjects();
        state_->SetLastStatus(AudioStatus::BackendError);
        return AudioStatus::BackendError;
    }

    state_->com_initialized = owns_com_apartment;

    native_result = XAudio2Create(&state_->engine, 0U, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(native_result)) {
        const AudioStatus status = MapXAudio2InitializeStatus(native_result);
        state_->ReleaseNativeObjects();
        state_->SetLastStatus(status);
        return status;
    }

    native_result = state_->engine->CreateMasteringVoice(&state_->mastering_voice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE);
    if (FAILED(native_result)) {
        const AudioStatus status = MapXAudio2InitializeStatus(native_result);
        state_->ReleaseNativeObjects();
        state_->SetLastStatus(status);
        return status;
    }

    WAVEFORMATEX wave_format{};
    wave_format.wFormatTag = WAVE_FORMAT_PCM;
    wave_format.nChannels = desc.channel_count;
    wave_format.nSamplesPerSec = desc.sample_rate;
    wave_format.wBitsPerSample = BITS_PER_SAMPLE;
    wave_format.nBlockAlign = static_cast<WORD>(desc.channel_count * BYTES_PER_SAMPLE);
    wave_format.nAvgBytesPerSec = desc.sample_rate * wave_format.nBlockAlign;
    native_result = state_->engine->CreateSourceVoice(&state_->source_voice, &wave_format, 0U, XAUDIO2_DEFAULT_FREQ_RATIO, &state_->callback);
    if (FAILED(native_result)) {
        const AudioStatus status = MapXAudio2InitializeStatus(native_result);
        state_->ReleaseNativeObjects();
        state_->SetLastStatus(status);
        return status;
    }

    state_->snapshot.initialized = true;
    return state_->SetLastStatus(AudioStatus::Success);
#endif

#if !defined(_WIN32)
    state_->SetLastStatus(AudioStatus::UnsupportedBackend);
    return AudioStatus::UnsupportedBackend;
#endif
}

AudioStatus AudioCallbackDevice::Start() {
    if (state_ == nullptr) {
        return AudioStatus::NotInitialized;
    }

    if (!state_->snapshot.initialized) {
        return state_->SetLastStatus(AudioStatus::NotInitialized);
    }

    if (state_->snapshot.shutdown) {
        return state_->SetLastStatus(AudioStatus::ShutdownComplete);
    }

    if (state_->snapshot.started) {
        return state_->SetLastStatus(AudioStatus::AlreadyStarted);
    }

    if (state_->uses_controlled_backend) {
        const AudioCallbackDeviceBackendTestConfig backend_test_config = CurrentBackendTestConfig();
        if (backend_test_config.start_status != AudioStatus::Success) {
            return state_->SetLastStatus(backend_test_config.start_status);
        }

        state_->snapshot.started = true;
        return state_->SetLastStatus(AudioStatus::Success);
    }

#if defined(_WIN32)
    HRESULT native_result = state_->source_voice->Start(0U);
    if (FAILED(native_result)) {
        return state_->SetLastStatus(AudioStatus::DeviceStartFailed);
    }

    native_result = state_->engine->StartEngine();
    if (FAILED(native_result)) {
        state_->source_voice->Stop(0U);
        return state_->SetLastStatus(AudioStatus::DeviceStartFailed);
    }
#endif

    state_->snapshot.started = true;
    return state_->SetLastStatus(AudioStatus::Success);
}

AudioStatus AudioCallbackDevice::SubmitS16Buffer(std::span<const std::int16_t> interleaved_samples, std::size_t frame_count) {
    if (state_ == nullptr) {
        return AudioStatus::NotInitialized;
    }

    if (!state_->snapshot.initialized) {
        return state_->SetLastStatus(AudioStatus::NotInitialized);
    }

    if (!state_->snapshot.started) {
        return state_->SetLastStatus(AudioStatus::NotStarted);
    }

    if (state_->snapshot.shutdown) {
        return state_->SetLastStatus(AudioStatus::ShutdownComplete);
    }

    if (frame_count != state_->desc.frames_per_buffer) {
        return state_->SetLastStatus(AudioStatus::InvalidDescriptor);
    }

    const std::size_t required_samples = RequiredSampleCount(state_->desc);
    if (interleaved_samples.size() < required_samples) {
        return state_->SetLastStatus(AudioStatus::InvalidDescriptor);
    }

    AudioCallbackBufferSlot *selected_slot = nullptr;
    {
        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->MergeCallbackEventsLocked();
        for (std::size_t index = 0U; index < state_->desc.buffer_count; ++index) {
            AudioCallbackBufferSlot &slot = state_->buffers[index];
            if (slot.queued.load(std::memory_order_acquire)) {
                continue;
            }

            selected_slot = &slot;
            break;
        }

        if (selected_slot == nullptr) {
            return state_->RecordSubmissionCapacityFailure(frame_count, interleaved_samples.size());
        }

        std::copy(interleaved_samples.begin(), interleaved_samples.begin() + required_samples, selected_slot->samples.begin());
        selected_slot->sequence = state_->next_sequence;
        selected_slot->pending_completion = AudioCallbackCompletion{};
        selected_slot->completion_pending.store(false, std::memory_order_release);
        selected_slot->queued.store(true, std::memory_order_release);
        ++state_->next_sequence;
        ++state_->snapshot.submitted_buffer_count;
        ++state_->snapshot.queued_buffer_count;
        state_->snapshot.max_queued_buffer_count = std::max(state_->snapshot.max_queued_buffer_count, state_->snapshot.queued_buffer_count);
    }

    if (state_->uses_controlled_backend) {
        const AudioCallbackDeviceBackendTestConfig backend_test_config = CurrentBackendTestConfig();
        if (backend_test_config.submit_status != AudioStatus::Success) {
            return RollBackFailedSubmit(state_, selected_slot, backend_test_config.submit_status);
        }

        if (backend_test_config.report_callback_error_on_submit) {
            state_->OnVoiceError();
            std::lock_guard<std::mutex> lock(state_->mutex);
            state_->MergeCallbackEventsLocked();
            return state_->SetLastStatus(AudioStatus::Success);
        }

        if (backend_test_config.complete_submitted_buffer) {
            state_->OnBufferEnd(selected_slot);
        }

        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->MergeCallbackEventsLocked();
        return state_->SetLastStatus(AudioStatus::Success);
    }

#if defined(_WIN32)
    XAUDIO2_BUFFER native_buffer{};
    native_buffer.AudioBytes = static_cast<UINT32>(required_samples * sizeof(std::int16_t));
    native_buffer.pAudioData = reinterpret_cast<const BYTE *>(selected_slot->samples.data());
    native_buffer.Flags = XAUDIO2_END_OF_STREAM;
    native_buffer.pContext = selected_slot;
    const HRESULT native_result = state_->source_voice->SubmitSourceBuffer(&native_buffer);
    if (FAILED(native_result)) {
        return RollBackFailedSubmit(state_, selected_slot, AudioStatus::BufferSubmitFailed);
    }
#endif

    std::lock_guard<std::mutex> lock(state_->mutex);
    return state_->SetLastStatus(AudioStatus::Success);
}

AudioStatus AudioCallbackDevice::WaitForCompletedCallbacks(std::uint64_t target_completed_count, std::uint32_t timeout_milliseconds) {
    if (state_ == nullptr) {
        return AudioStatus::NotInitialized;
    }

    if (!state_->snapshot.initialized) {
        return state_->SetLastStatus(AudioStatus::NotInitialized);
    }

    if (target_completed_count == NO_COMPLETED_CALLBACKS) {
        return state_->SetLastStatus(AudioStatus::Success);
    }

    std::unique_lock<std::mutex> lock(state_->mutex);
    const auto timeout = std::chrono::milliseconds(timeout_milliseconds);
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (true) {
        state_->MergeCallbackEventsLocked();
        if (state_->snapshot.failed_callback_count > 0U) {
            return state_->SetLastStatus(AudioStatus::CallbackFailed);
        }

        if (state_->snapshot.completed_callback_count >= target_completed_count) {
            return state_->SetLastStatus(AudioStatus::Success);
        }

        const auto now = std::chrono::steady_clock::now();
        if (now >= deadline) {
            return state_->SetLastStatus(AudioStatus::CallbackTimeout);
        }

        auto wait_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::milliseconds(CALLBACK_WAIT_POLL_MILLISECONDS));
        const auto remaining_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(deadline - now);
        if (remaining_duration < wait_duration) {
            wait_duration = remaining_duration;
        }

        state_->completion_signal.wait_for(lock, wait_duration);
    }
}

AudioStatus AudioCallbackDevice::DrainCompletions(AudioCallbackCompletion *completions, std::size_t completion_capacity, std::size_t &out_completion_count) {
    out_completion_count = 0U;
    if (state_ == nullptr) {
        return AudioStatus::NotInitialized;
    }

    if (completion_capacity > 0U && completions == nullptr) {
        return state_->SetLastStatus(AudioStatus::InvalidDescriptor);
    }

    std::lock_guard<std::mutex> lock(state_->mutex);
    state_->MergeCallbackEventsLocked();
    const std::size_t write_count = std::min(completion_capacity, state_->completion_count);
    for (std::size_t index = 0U; index < write_count; ++index) {
        completions[index] = state_->completions[index];
    }

    out_completion_count = write_count;
    state_->snapshot.drained_completion_count += write_count;
    const std::size_t remaining_count = state_->completion_count - write_count;
    for (std::size_t index = 0U; index < remaining_count; ++index) {
        state_->completions[index] = state_->completions[index + write_count];
    }

    for (std::size_t index = remaining_count; index < state_->completion_count; ++index) {
        state_->completions[index] = AudioCallbackCompletion{};
    }

    state_->completion_count = remaining_count;
    if (state_->completion_count == 0U) {
        if (write_count == 0U) {
            state_->ClearSubmissionCapacityFailure();
            return AudioStatus::Success;
        }

        return state_->SetLastStatus(AudioStatus::Success);
    }

    return state_->SetLastStatus(AudioStatus::CapacityExceeded);
}

AudioStatus AudioCallbackDevice::Stop() {
    if (state_ == nullptr) {
        return AudioStatus::NotInitialized;
    }

    if (!state_->snapshot.initialized) {
        return state_->SetLastStatus(AudioStatus::NotInitialized);
    }

    if (!state_->snapshot.started) {
        return state_->SetLastStatus(AudioStatus::Success);
    }

#if defined(_WIN32)
    if (!state_->uses_controlled_backend) {
        state_->source_voice->Stop(0U);
        state_->engine->StopEngine();
    }
#endif

    state_->snapshot.started = false;
    return state_->SetLastStatus(AudioStatus::Success);
}

AudioStatus AudioCallbackDevice::Shutdown() {
    if (state_ == nullptr) {
        return AudioStatus::NotInitialized;
    }

    if (state_->snapshot.shutdown) {
        return state_->SetLastStatus(AudioStatus::ShutdownComplete);
    }

    state_->ReleaseNativeObjects();

    {
        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->MergeCallbackEventsLocked();
        for (AudioCallbackBufferSlot &slot : state_->buffers) {
            slot.queued.store(false, std::memory_order_release);
            slot.completion_pending.store(false, std::memory_order_release);
        }

        state_->snapshot.queued_buffer_count = 0U;
        state_->snapshot.started = false;
        state_->snapshot.shutdown = true;
    }

    state_->completion_signal.notify_all();
    return state_->SetLastStatus(AudioStatus::ShutdownComplete);
}

AudioCallbackSnapshot AudioCallbackDevice::Snapshot() const {
    if (state_ == nullptr) {
        return AudioCallbackSnapshot{};
    }

    std::lock_guard<std::mutex> lock(state_->mutex);
    state_->MergeCallbackEventsLocked();
    return state_->snapshot;
}
}
