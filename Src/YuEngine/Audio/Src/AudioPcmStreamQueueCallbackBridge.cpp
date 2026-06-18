// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Src/AudioPcmStreamQueueCallbackBridge.cpp

#include "YuEngine/Audio/AudioPcmStreamQueueCallbackBridge.h"

#include "YuEngine/Audio/AudioCallbackDevice.h"
#include "YuEngine/Audio/AudioCallbackSnapshot.h"
#include "YuEngine/Audio/AudioConstants.h"
#include "YuEngine/Audio/AudioPcmStreamQueueRecord.h"
#include "YuEngine/Audio/AudioSampleFormat.h"
#include "YuEngine/Audio/TestAudioDevice.h"

namespace yuengine::audio {
namespace {
AudioStatus SubmitAudioCallbackDeviceBuffer(void *context, std::span<const std::int16_t> interleaved_samples, std::size_t frame_count) {
    if (context == nullptr) {
        return AudioStatus::InvalidDescriptor;
    }

    AudioCallbackDevice *callback_device = static_cast<AudioCallbackDevice *>(context);
    return callback_device->SubmitS16Buffer(interleaved_samples, frame_count);
}

void SetBridgeResultStatus(AudioPcmStreamQueueCallbackResult *out_result, AudioStatus status) {
    if (out_result == nullptr) {
        return;
    }

    out_result->status = status;
}

bool IsRecordShapeSupported(const AudioPcmStreamQueueRecord &record, std::size_t frames_per_buffer) {
    if (!record.is_active) {
        return false;
    }

    if (record.format != AudioSampleFormat::Signed16) {
        return false;
    }

    if (record.sample_rate != SAMPLE_RATE) {
        return false;
    }

    if (record.channel_count != CHANNEL_COUNT) {
        return false;
    }

    if (frames_per_buffer == 0U) {
        return false;
    }

    if (record.chunk_frame_count != frames_per_buffer) {
        return false;
    }

    if (record.remaining_frame_count % frames_per_buffer != 0U) {
        return false;
    }

    return true;
}

bool HasSampleRange(const AudioPcmStreamQueueRecord &record, std::span<const std::int16_t> interleaved_samples) {
    const std::size_t first_frame = record.first_frame + record.drained_frame_count;
    const std::size_t first_sample = first_frame * record.channel_count;
    const std::size_t required_sample_count = record.remaining_frame_count * record.channel_count;
    if (first_sample > interleaved_samples.size()) {
        return false;
    }

    const std::size_t available_sample_count = interleaved_samples.size() - first_sample;
    if (available_sample_count < required_sample_count) {
        return false;
    }

    return true;
}
}

AudioPcmStreamQueueCallbackSubmitter CreateAudioCallbackDeviceSubmitter(AudioCallbackDevice *callback_device) {
    AudioPcmStreamQueueCallbackSubmitter submitter{};
    if (callback_device == nullptr) {
        return submitter;
    }

    const AudioCallbackSnapshot snapshot = callback_device->Snapshot();
    submitter.context = callback_device;
    submitter.submit_s16_buffer = SubmitAudioCallbackDeviceBuffer;
    submitter.frames_per_buffer = snapshot.frames_per_buffer;
    submitter.started = snapshot.initialized && snapshot.started && !snapshot.shutdown;
    return submitter;
}

AudioStatus SubmitPcmStreamQueueToCallback(TestAudioDevice *stream_device,
                                           AudioPcmStreamQueueHandle queue,
                                           const AudioPcmStreamQueueCallbackSubmitter &submitter,
                                           std::span<const std::int16_t> interleaved_samples,
                                           std::span<AudioPcmStreamQueueChunk> chunk_storage,
                                           AudioPcmStreamQueueCallbackResult *out_result) {
    if (out_result == nullptr) {
        return AudioStatus::InvalidDescriptor;
    }

    *out_result = AudioPcmStreamQueueCallbackResult{};
    out_result->queue = queue;

    if (stream_device == nullptr) {
        SetBridgeResultStatus(out_result, AudioStatus::InvalidDescriptor);
        return AudioStatus::InvalidDescriptor;
    }

    if (submitter.submit_s16_buffer == nullptr) {
        SetBridgeResultStatus(out_result, AudioStatus::InvalidDescriptor);
        return AudioStatus::InvalidDescriptor;
    }

    if (!submitter.started) {
        SetBridgeResultStatus(out_result, AudioStatus::NotStarted);
        return AudioStatus::NotStarted;
    }

    AudioPcmStreamQueueRecord record{};
    AudioStatus status = stream_device->QueryPcmStreamQueue(queue, record);
    if (status != AudioStatus::Success) {
        SetBridgeResultStatus(out_result, status);
        return status;
    }

    if (record.remaining_frame_count == 0U) {
        SetBridgeResultStatus(out_result, AudioStatus::Success);
        return AudioStatus::Success;
    }

    if (!IsRecordShapeSupported(record, submitter.frames_per_buffer)) {
        SetBridgeResultStatus(out_result, AudioStatus::InvalidDescriptor);
        return AudioStatus::InvalidDescriptor;
    }

    if (!HasSampleRange(record, interleaved_samples)) {
        SetBridgeResultStatus(out_result, AudioStatus::InvalidDescriptor);
        return AudioStatus::InvalidDescriptor;
    }

    const std::size_t required_chunk_count = record.remaining_frame_count / record.chunk_frame_count;
    if (chunk_storage.size() < required_chunk_count) {
        SetBridgeResultStatus(out_result, AudioStatus::CapacityExceeded);
        return AudioStatus::CapacityExceeded;
    }

    std::size_t chunk_count = 0U;
    status = stream_device->DrainPcmStreamQueue(queue, chunk_storage, chunk_count);
    if (status != AudioStatus::Success) {
        SetBridgeResultStatus(out_result, status);
        return status;
    }

    out_result->drained_chunk_count = chunk_count;
    for (std::size_t chunk_index = 0U; chunk_index < chunk_count; ++chunk_index) {
        const AudioPcmStreamQueueChunk &chunk = chunk_storage[chunk_index];
        const std::span<const std::int16_t> chunk_samples(interleaved_samples.data() + chunk.first_interleaved_sample, chunk.interleaved_sample_count);
        status = submitter.submit_s16_buffer(submitter.context, chunk_samples, chunk.frame_count);
        if (status != AudioStatus::Success) {
            SetBridgeResultStatus(out_result, status);
            return status;
        }

        ++out_result->submitted_chunk_count;
        out_result->submitted_frame_count += chunk.frame_count;
        out_result->submitted_sample_count += chunk.interleaved_sample_count;
        out_result->last_submitted_first_frame = chunk.first_frame;
        out_result->reached_final_chunk = chunk.is_final_chunk;
    }

    SetBridgeResultStatus(out_result, AudioStatus::Success);
    return AudioStatus::Success;
}

AudioStatus SubmitPcmStreamQueueToCallback(TestAudioDevice *stream_device,
                                           AudioPcmStreamQueueHandle queue,
                                           AudioCallbackDevice *callback_device,
                                           std::span<const std::int16_t> interleaved_samples,
                                           std::span<AudioPcmStreamQueueChunk> chunk_storage,
                                           AudioPcmStreamQueueCallbackResult *out_result) {
    if (callback_device == nullptr) {
        if (out_result != nullptr) {
            *out_result = AudioPcmStreamQueueCallbackResult{};
            out_result->queue = queue;
        }

        SetBridgeResultStatus(out_result, AudioStatus::InvalidDescriptor);
        return AudioStatus::InvalidDescriptor;
    }

    const AudioPcmStreamQueueCallbackSubmitter submitter = CreateAudioCallbackDeviceSubmitter(callback_device);
    return SubmitPcmStreamQueueToCallback(stream_device, queue, submitter, interleaved_samples, chunk_storage, out_result);
}
}
