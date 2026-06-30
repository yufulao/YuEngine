// 模块: YuEngine AudioScene
// 文件: Src/YuEngine/AudioScene/Src/AudioSceneContractQueue.cpp

#include "YuEngine/AudioScene/AudioSceneContractQueue.h"

namespace yuengine::audioscene {
AudioSceneStatus AudioSceneContractQueue::SubmitSourceUpdates(
    const AudioSceneSubmitRequest &request,
    std::span<yuengine::audio::AudioPcmStreamQueueRequest> out_requests,
    AudioSceneSubmitResult *out_result) {
    AudioSceneSubmitResult result{};
    result.frame_id = request.frame_id;

    if (out_result == nullptr) {
        return RecordFailure(AudioSceneStatus::NullPointer, result);
    }

    std::size_t active_source_count = 0U;
    std::size_t playing_source_count = 0U;
    const AudioSceneStatus validate_status =
        ValidateRequest(request, out_requests, &active_source_count, &playing_source_count);
    result.active_source_count = active_source_count;
    result.playing_source_count = playing_source_count;
    if (validate_status != AudioSceneStatus::Success) {
        result.status = validate_status;
        *out_result = result;
        return RecordFailure(validate_status, result);
    }

    std::size_t output_index = 0U;
    for (const AudioSceneSourceRecord &source : request.sources) {
        if (!source.is_active) {
            continue;
        }

        if (!IsPlayingSource(source)) {
            ++result.skipped_source_count;
            continue;
        }

        FillQueueRequest(request, source, &out_requests[output_index]);
        result.last_bus_id = source.bus_id;
        ++output_index;
    }

    result.queue_request_count = output_index;
    result.status = AudioSceneStatus::Success;
    *out_result = result;
    return RecordSuccess(result);
}

AudioSceneSnapshot AudioSceneContractQueue::Snapshot() const {
    return snapshot_;
}

AudioSceneStatus AudioSceneContractQueue::ValidateRequest(
    const AudioSceneSubmitRequest &request,
    std::span<yuengine::audio::AudioPcmStreamQueueRequest> out_requests,
    std::size_t *out_active_source_count,
    std::size_t *out_playing_source_count) const {
    if (out_active_source_count == nullptr) {
        return AudioSceneStatus::NullPointer;
    }

    if (out_playing_source_count == nullptr) {
        return AudioSceneStatus::NullPointer;
    }

    if (request.frame_id == 0U) {
        return AudioSceneStatus::InvalidFrameId;
    }

    std::size_t active_source_count = 0U;
    std::size_t playing_source_count = 0U;
    for (const AudioSceneSourceRecord &source : request.sources) {
        if (!source.is_active) {
            continue;
        }

        const AudioSceneStatus source_status = ValidateSource(source);
        if (source_status != AudioSceneStatus::Success) {
            return source_status;
        }

        ++active_source_count;
        if (IsPlayingSource(source)) {
            ++playing_source_count;
        }
    }

    if (active_source_count == 0U) {
        return AudioSceneStatus::MissingSource;
    }

    if (playing_source_count == 0U) {
        *out_active_source_count = active_source_count;
        *out_playing_source_count = 0U;
        return AudioSceneStatus::Success;
    }

    *out_active_source_count = active_source_count;
    *out_playing_source_count = playing_source_count;

    if (!request.backend_available) {
        return AudioSceneStatus::BackendUnavailable;
    }

    if (playing_source_count > out_requests.size()) {
        return AudioSceneStatus::OutputCapacityExceeded;
    }

    return AudioSceneStatus::Success;
}

AudioSceneStatus AudioSceneContractQueue::ValidateSource(const AudioSceneSourceRecord &source) const {
    if (!IsSourceIdValid(source.source_id)) {
        return AudioSceneStatus::InvalidSourceRecord;
    }

    if (!IsKnownSourceState(source.state)) {
        return AudioSceneStatus::InvalidSourceRecord;
    }

    if (source.gain < 0.0F) {
        return AudioSceneStatus::InvalidGain;
    }

    if (source.gain > AUDIO_SCENE_MAX_GAIN) {
        return AudioSceneStatus::InvalidGain;
    }

    if (!IsBusIdValid(source.bus_id)) {
        return AudioSceneStatus::InvalidBusId;
    }

    if (!IsPlayingSource(source)) {
        return AudioSceneStatus::Success;
    }

    if (!source.sound_asset.IsValid()) {
        return AudioSceneStatus::MissingSoundAsset;
    }

    if (!source.audio_ready.is_ready) {
        return AudioSceneStatus::MissingAudioReadyRecord;
    }

    if (!IsPacketHandleValid(source.packet)) {
        return AudioSceneStatus::MissingAudioPacket;
    }

    if (source.audio_ready.packet_request.frame_count == 0U) {
        return AudioSceneStatus::MissingAudioReadyRecord;
    }

    if (source.audio_ready.packet_request.byte_count == 0U) {
        return AudioSceneStatus::MissingAudioReadyRecord;
    }

    return AudioSceneStatus::Success;
}

bool AudioSceneContractQueue::IsKnownSourceState(AudioSceneSourceState state) const {
    if (state == AudioSceneSourceState::Stopped) {
        return true;
    }

    if (state == AudioSceneSourceState::Playing) {
        return true;
    }

    return state == AudioSceneSourceState::Paused;
}

bool AudioSceneContractQueue::IsBusIdValid(std::uint32_t bus_id) const {
    if (bus_id < AUDIO_SCENE_MASTER_BUS_ID) {
        return false;
    }

    return bus_id <= AUDIO_SCENE_MAX_BUS_ID;
}

bool AudioSceneContractQueue::IsPlayingSource(const AudioSceneSourceRecord &source) const {
    return source.state == AudioSceneSourceState::Playing;
}

bool AudioSceneContractQueue::IsPacketHandleValid(yuengine::audio::AudioPcmSamplePacketHandle packet) const {
    return packet.generation != 0U;
}

bool AudioSceneContractQueue::IsSourceIdValid(yuengine::audio::AudioSourceId source) const {
    if (source.generation == 0U) {
        return false;
    }

    return source.slot < AUDIO_SCENE_BUS_QUEUE_ID_STRIDE;
}

std::uint32_t AudioSceneContractQueue::BuildBusQueueId(const AudioSceneSourceRecord &source) const {
    return source.bus_id * AUDIO_SCENE_BUS_QUEUE_ID_STRIDE + source.source_id.slot;
}

void AudioSceneContractQueue::FillQueueRequest(
    const AudioSceneSubmitRequest &request,
    const AudioSceneSourceRecord &source,
    yuengine::audio::AudioPcmStreamQueueRequest *out_request) const {
    if (out_request == nullptr) {
        return;
    }

    const yuengine::audio::AudioPcmSamplePacketRequest &packet_request = source.audio_ready.packet_request;
    out_request->queue_id = BuildBusQueueId(source);
    out_request->packet = source.packet;
    out_request->expected_packet_id = packet_request.packet_id;
    out_request->format = packet_request.format;
    out_request->sample_rate = packet_request.sample_rate;
    out_request->channel_count = packet_request.channel_count;
    out_request->first_frame = 0U;
    out_request->frame_count = packet_request.frame_count;
    out_request->interleaved_sample_count = packet_request.interleaved_sample_count;
    out_request->byte_count = packet_request.byte_count;
    out_request->chunk_frame_count = request.chunk_frame_count;
}

AudioSceneStatus AudioSceneContractQueue::RecordSuccess(const AudioSceneSubmitResult &result) {
    ++snapshot_.submit_count;
    snapshot_.last_frame_id = result.frame_id;
    snapshot_.last_active_source_count = result.active_source_count;
    snapshot_.last_playing_source_count = result.playing_source_count;
    snapshot_.last_queue_request_count = result.queue_request_count;
    snapshot_.last_skipped_source_count = result.skipped_source_count;
    snapshot_.last_bus_id = result.last_bus_id;
    snapshot_.last_status = AudioSceneStatus::Success;
    return AudioSceneStatus::Success;
}

AudioSceneStatus AudioSceneContractQueue::RecordFailure(
    AudioSceneStatus status,
    const AudioSceneSubmitResult &result) {
    ++snapshot_.failed_submit_count;
    snapshot_.last_frame_id = result.frame_id;
    snapshot_.last_active_source_count = result.active_source_count;
    snapshot_.last_playing_source_count = result.playing_source_count;
    snapshot_.last_queue_request_count = result.queue_request_count;
    snapshot_.last_skipped_source_count = result.skipped_source_count;
    snapshot_.last_bus_id = result.last_bus_id;
    snapshot_.last_status = status;
    return status;
}
}
