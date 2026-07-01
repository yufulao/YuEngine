// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Src/TestAudioDevice.cpp

#include "YuEngine/Audio/TestAudioDevice.h"

#include <algorithm>

#include "YuEngine/Audio/AudioConstants.h"

namespace yuengine::audio {
namespace {
constexpr std::uint32_t INVALID_GENERATION = 0U;

void ClearDeviceRequiredCounts(AudioDeviceSnapshot &snapshot) {
    snapshot.last_required_source_count = 0U;
    snapshot.last_required_voice_count = 0U;
}

void ClearPcmSamplePacketRequiredCounts(AudioPcmSamplePacketSnapshot &snapshot) {
    snapshot.last_required_packet_count = 0U;
}

void ClearPcmStreamQueueRequiredCounts(AudioPcmStreamQueueSnapshot &snapshot) {
    snapshot.last_required_queue_count = 0U;
    snapshot.last_required_output_chunk_count = 0U;
}
}

TestAudioDevice::TestAudioDevice()
    : sources_(),
      voices_(),
      pcm_sample_packets_(),
      pcm_stream_queues_(),
      capabilities_{},
      snapshot_{},
      pcm_sample_packet_snapshot_{},
      pcm_stream_queue_snapshot_{},
      generation_seed_(INVALID_GENERATION),
      is_initialized_(false) {
}

AudioStatus TestAudioDevice::Initialize(const AudioDeviceDesc& desc) {
    if (desc.backend_kind != AudioBackendKind::Test) {
        return RecordFailure(AudioStatus::UnsupportedBackend);
    }

    if (!IsDeviceFormatSupported(desc)) {
        return RecordFailure(AudioStatus::UnsupportedFormat);
    }

    if (desc.source_capacity == 0U) {
        return RecordFailure(AudioStatus::InvalidDescriptor);
    }

    if (desc.source_capacity > MAX_SOURCES) {
        return RecordFailure(AudioStatus::CapacityExceeded);
    }

    if (desc.voice_capacity == 0U) {
        return RecordFailure(AudioStatus::InvalidDescriptor);
    }

    if (desc.voice_capacity > MAX_VOICES) {
        return RecordFailure(AudioStatus::CapacityExceeded);
    }

    ++generation_seed_;
    if (generation_seed_ == INVALID_GENERATION) {
        ++generation_seed_;
    }

    sources_.assign(desc.source_capacity, AudioSourceSlot{});
    voices_.clear();
    voices_.reserve(MAX_VOICES);
    voices_.resize(desc.voice_capacity);
    pcm_sample_packets_.assign(MAX_PCM_SAMPLE_PACKETS, AudioPcmSamplePacketSlot{});
    pcm_stream_queues_.assign(MAX_PCM_STREAM_QUEUES, AudioPcmStreamQueueSlot{});
    for (AudioSourceSlot& source : sources_) {
        source.generation = generation_seed_;
    }

    for (AudioVoiceSlot& voice : voices_) {
        voice.generation = generation_seed_;
    }

    for (AudioPcmSamplePacketSlot& packet : pcm_sample_packets_) {
        packet.generation = generation_seed_;
    }

    for (AudioPcmStreamQueueSlot& queue : pcm_stream_queues_) {
        queue.generation = generation_seed_;
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
    RecordSuccess();
    pcm_sample_packet_snapshot_ = AudioPcmSamplePacketSnapshot{};
    pcm_sample_packet_snapshot_.packet_capacity = pcm_sample_packets_.size();
    pcm_sample_packet_snapshot_.last_status = AudioStatus::Success;
    pcm_stream_queue_snapshot_ = AudioPcmStreamQueueSnapshot{};
    pcm_stream_queue_snapshot_.queue_capacity = pcm_stream_queues_.size();
    pcm_stream_queue_snapshot_.last_status = AudioStatus::Success;
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
        RecordSuccess();
        return AudioStatus::Success;
    }

    const std::size_t required_source_count = snapshot_.source_count + 1U;
    const AudioStatus status = RecordFailure(AudioStatus::CapacityExceeded);
    snapshot_.last_required_source_count = required_source_count;
    return status;
}

AudioStatus TestAudioDevice::CreatePcmSamplePacket(const AudioPcmSamplePacketRequest& request, AudioPcmSamplePacketHandle& out_packet) {
    if (!is_initialized_) {
        return AudioStatus::InvalidDescriptor;
    }

    const AudioStatus request_status = ValidatePcmSamplePacketRequest(request);
    if (request_status != AudioStatus::Success) {
        return RecordPcmSamplePacketFailure(request_status, AudioPcmSamplePacketOperation::Create);
    }

    if (HasActivePcmSamplePacketId(request.packet_id)) {
        ++pcm_sample_packet_snapshot_.duplicate_packet_rejected_count;
        return RecordPcmSamplePacketFailure(AudioStatus::InvalidDescriptor, AudioPcmSamplePacketOperation::Create);
    }

    for (std::size_t index = 0U; index < pcm_sample_packets_.size(); ++index) {
        AudioPcmSamplePacketSlot& slot = pcm_sample_packets_[index];
        if (slot.is_active) {
            continue;
        }

        slot.is_active = true;
        slot.record = AudioPcmSamplePacketRecord{
            AudioPcmSamplePacketHandle{static_cast<std::uint32_t>(index), slot.generation},
            request.packet_id,
            request.format,
            request.sample_rate,
            request.channel_count,
            request.frame_count,
            request.interleaved_sample_count,
            request.byte_count,
            true};
        out_packet = slot.record.handle;
        ++pcm_sample_packet_snapshot_.active_packet_count;
        ++pcm_sample_packet_snapshot_.created_packet_count;
        SetPcmSamplePacketLastStatus(AudioStatus::Success, AudioPcmSamplePacketOperation::Create);
        RecordSuccess();
        return AudioStatus::Success;
    }

    ++pcm_sample_packet_snapshot_.capacity_rejected_count;
    const std::size_t required_packet_count = pcm_sample_packet_snapshot_.active_packet_count + 1U;
    const AudioStatus status =
        RecordPcmSamplePacketFailure(AudioStatus::CapacityExceeded, AudioPcmSamplePacketOperation::Create);
    pcm_sample_packet_snapshot_.last_required_packet_count = required_packet_count;
    return status;
}

AudioStatus TestAudioDevice::QueryPcmSamplePacket(AudioPcmSamplePacketHandle packet, AudioPcmSamplePacketRecord& out_record) {
    if (!is_initialized_) {
        return AudioStatus::InvalidDescriptor;
    }

    if (!IsPcmSamplePacketHandleValid(packet)) {
        ++pcm_sample_packet_snapshot_.stale_packet_rejected_count;
        return RecordPcmSamplePacketFailure(AudioStatus::InvalidHandle, AudioPcmSamplePacketOperation::Query);
    }

    const AudioPcmSamplePacketSlot& slot = pcm_sample_packets_[packet.slot];
    out_record = slot.record;
    ++pcm_sample_packet_snapshot_.queried_packet_count;
    SetPcmSamplePacketLastStatus(AudioStatus::Success, AudioPcmSamplePacketOperation::Query);
    RecordSuccess();
    return AudioStatus::Success;
}

AudioStatus TestAudioDevice::ReleasePcmSamplePacket(AudioPcmSamplePacketHandle packet) {
    if (!is_initialized_) {
        return AudioStatus::InvalidDescriptor;
    }

    if (!IsPcmSamplePacketHandleValid(packet)) {
        ++pcm_sample_packet_snapshot_.stale_packet_rejected_count;
        return RecordPcmSamplePacketFailure(AudioStatus::InvalidHandle, AudioPcmSamplePacketOperation::Release);
    }

    AudioPcmSamplePacketSlot& slot = pcm_sample_packets_[packet.slot];
    slot.is_active = false;
    slot.record = AudioPcmSamplePacketRecord{};
    ++slot.generation;
    if (slot.generation == INVALID_GENERATION) {
        ++slot.generation;
    }

    --pcm_sample_packet_snapshot_.active_packet_count;
    ++pcm_sample_packet_snapshot_.released_packet_count;
    SetPcmSamplePacketLastStatus(AudioStatus::Success, AudioPcmSamplePacketOperation::Release);
    RecordSuccess();
    return AudioStatus::Success;
}

AudioStatus TestAudioDevice::CreatePcmStreamQueue(const AudioPcmStreamQueueRequest& request, AudioPcmStreamQueueHandle& out_queue) {
    if (!is_initialized_) {
        return AudioStatus::InvalidDescriptor;
    }

    pcm_stream_queue_snapshot_.last_queue_id = request.queue_id;
    pcm_stream_queue_snapshot_.last_packet_id = request.expected_packet_id;
    pcm_stream_queue_snapshot_.last_frame_count = request.frame_count;

    if (request.queue_id == 0U) {
        return RecordPcmStreamQueueFailure(AudioStatus::InvalidDescriptor, AudioPcmStreamQueueOperation::Create);
    }

    if (!IsPcmSamplePacketHandleValid(request.packet)) {
        ++pcm_stream_queue_snapshot_.packet_rejected_count;
        return RecordPcmStreamQueueFailure(AudioStatus::InvalidHandle, AudioPcmStreamQueueOperation::Create);
    }

    const AudioPcmSamplePacketSlot& packet_slot = pcm_sample_packets_[request.packet.slot];
    const AudioPcmSamplePacketRecord& packet_record = packet_slot.record;
    if (request.expected_packet_id != packet_record.packet_id) {
        ++pcm_stream_queue_snapshot_.packet_rejected_count;
        return RecordPcmStreamQueueFailure(AudioStatus::InvalidDescriptor, AudioPcmStreamQueueOperation::Create);
    }

    if (request.format != packet_record.format) {
        ++pcm_stream_queue_snapshot_.packet_rejected_count;
        return RecordPcmStreamQueueFailure(AudioStatus::UnsupportedFormat, AudioPcmStreamQueueOperation::Create);
    }

    if (request.sample_rate != packet_record.sample_rate) {
        ++pcm_stream_queue_snapshot_.packet_rejected_count;
        return RecordPcmStreamQueueFailure(AudioStatus::UnsupportedFormat, AudioPcmStreamQueueOperation::Create);
    }

    if (request.channel_count != packet_record.channel_count) {
        ++pcm_stream_queue_snapshot_.packet_rejected_count;
        return RecordPcmStreamQueueFailure(AudioStatus::UnsupportedFormat, AudioPcmStreamQueueOperation::Create);
    }

    if (request.frame_count == 0U) {
        ++pcm_stream_queue_snapshot_.range_rejected_count;
        return RecordPcmStreamQueueFailure(AudioStatus::InvalidDescriptor, AudioPcmStreamQueueOperation::Create);
    }

    if (request.first_frame > packet_record.frame_count) {
        ++pcm_stream_queue_snapshot_.range_rejected_count;
        return RecordPcmStreamQueueFailure(AudioStatus::InvalidDescriptor, AudioPcmStreamQueueOperation::Create);
    }

    const std::size_t available_frame_count = packet_record.frame_count - request.first_frame;
    if (request.frame_count > available_frame_count) {
        ++pcm_stream_queue_snapshot_.range_rejected_count;
        return RecordPcmStreamQueueFailure(AudioStatus::InvalidDescriptor, AudioPcmStreamQueueOperation::Create);
    }

    const std::size_t expected_sample_count = request.frame_count * packet_record.channel_count;
    if (request.interleaved_sample_count != expected_sample_count) {
        ++pcm_stream_queue_snapshot_.sample_count_rejected_count;
        return RecordPcmStreamQueueFailure(AudioStatus::InvalidDescriptor, AudioPcmStreamQueueOperation::Create);
    }

    const std::size_t expected_byte_count = expected_sample_count * sizeof(std::int16_t);
    if (request.byte_count != expected_byte_count) {
        ++pcm_stream_queue_snapshot_.byte_count_rejected_count;
        return RecordPcmStreamQueueFailure(AudioStatus::InvalidDescriptor, AudioPcmStreamQueueOperation::Create);
    }

    if (request.chunk_frame_count == 0U) {
        ++pcm_stream_queue_snapshot_.chunk_rejected_count;
        return RecordPcmStreamQueueFailure(AudioStatus::InvalidDescriptor, AudioPcmStreamQueueOperation::Create);
    }

    if (request.chunk_frame_count > MAX_PCM_STREAM_CHUNK_FRAMES) {
        ++pcm_stream_queue_snapshot_.chunk_rejected_count;
        return RecordPcmStreamQueueFailure(AudioStatus::CapacityExceeded, AudioPcmStreamQueueOperation::Create);
    }

    if (HasActivePcmStreamQueueId(request.queue_id)) {
        ++pcm_stream_queue_snapshot_.duplicate_queue_rejected_count;
        return RecordPcmStreamQueueFailure(AudioStatus::InvalidDescriptor, AudioPcmStreamQueueOperation::Create);
    }

    for (std::size_t index = 0U; index < pcm_stream_queues_.size(); ++index) {
        AudioPcmStreamQueueSlot& slot = pcm_stream_queues_[index];
        if (slot.is_active) {
            continue;
        }

        slot.is_active = true;
        slot.record = AudioPcmStreamQueueRecord{
            AudioPcmStreamQueueHandle{static_cast<std::uint32_t>(index), slot.generation},
            request.queue_id,
            request.packet,
            packet_record.packet_id,
            packet_record.format,
            packet_record.sample_rate,
            packet_record.channel_count,
            request.first_frame,
            request.frame_count,
            0U,
            request.frame_count,
            request.interleaved_sample_count,
            request.byte_count,
            request.chunk_frame_count,
            true};
        out_queue = slot.record.handle;
        ++pcm_stream_queue_snapshot_.active_queue_count;
        ++pcm_stream_queue_snapshot_.created_queue_count;
        SetPcmStreamQueueLastStatus(AudioStatus::Success, AudioPcmStreamQueueOperation::Create);
        RecordSuccess();
        return AudioStatus::Success;
    }

    ++pcm_stream_queue_snapshot_.capacity_rejected_count;
    const std::size_t required_queue_count = pcm_stream_queue_snapshot_.active_queue_count + 1U;
    const AudioStatus status =
        RecordPcmStreamQueueFailure(AudioStatus::CapacityExceeded, AudioPcmStreamQueueOperation::Create);
    pcm_stream_queue_snapshot_.last_required_queue_count = required_queue_count;
    return status;
}

AudioStatus TestAudioDevice::QueryPcmStreamQueue(AudioPcmStreamQueueHandle queue, AudioPcmStreamQueueRecord& out_record) {
    if (!is_initialized_) {
        return AudioStatus::InvalidDescriptor;
    }

    if (!IsPcmStreamQueueHandleValid(queue)) {
        ++pcm_stream_queue_snapshot_.stale_queue_rejected_count;
        return RecordPcmStreamQueueFailure(AudioStatus::InvalidHandle, AudioPcmStreamQueueOperation::Query);
    }

    const AudioPcmStreamQueueSlot& slot = pcm_stream_queues_[queue.slot];
    out_record = slot.record;
    pcm_stream_queue_snapshot_.last_queue_id = slot.record.queue_id;
    pcm_stream_queue_snapshot_.last_packet_id = slot.record.packet_id;
    pcm_stream_queue_snapshot_.last_frame_count = slot.record.frame_count;
    ++pcm_stream_queue_snapshot_.queried_queue_count;
    SetPcmStreamQueueLastStatus(AudioStatus::Success, AudioPcmStreamQueueOperation::Query);
    RecordSuccess();
    return AudioStatus::Success;
}

AudioStatus TestAudioDevice::DrainPcmStreamQueue(AudioPcmStreamQueueHandle queue, std::span<AudioPcmStreamQueueChunk> out_chunks, std::size_t& out_chunk_count) {
    if (!is_initialized_) {
        return AudioStatus::InvalidDescriptor;
    }

    out_chunk_count = 0U;
    if (!IsPcmStreamQueueHandleValid(queue)) {
        ++pcm_stream_queue_snapshot_.stale_queue_rejected_count;
        return RecordPcmStreamQueueFailure(AudioStatus::InvalidHandle, AudioPcmStreamQueueOperation::Drain);
    }

    AudioPcmStreamQueueSlot& slot = pcm_stream_queues_[queue.slot];
    AudioPcmStreamQueueRecord& record = slot.record;
    pcm_stream_queue_snapshot_.last_queue_id = record.queue_id;
    pcm_stream_queue_snapshot_.last_packet_id = record.packet_id;
    pcm_stream_queue_snapshot_.last_frame_count = record.remaining_frame_count;
    if (record.remaining_frame_count == 0U) {
        SetPcmStreamQueueLastStatus(AudioStatus::Success, AudioPcmStreamQueueOperation::Drain);
        RecordSuccess();
        return AudioStatus::Success;
    }

    const std::size_t chunk_frame_count = record.chunk_frame_count;
    const std::size_t required_chunk_count = (record.remaining_frame_count + chunk_frame_count - 1U) / chunk_frame_count;
    if (out_chunks.size() < required_chunk_count) {
        ++pcm_stream_queue_snapshot_.output_capacity_rejected_count;
        const AudioStatus status =
            RecordPcmStreamQueueFailure(AudioStatus::CapacityExceeded, AudioPcmStreamQueueOperation::Drain);
        pcm_stream_queue_snapshot_.last_required_output_chunk_count = required_chunk_count;
        return status;
    }

    const std::size_t drain_start_frame = record.first_frame + record.drained_frame_count;
    std::size_t remaining_frame_count = record.remaining_frame_count;
    std::size_t written_frame_count = 0U;
    for (std::size_t chunk_index = 0U; chunk_index < required_chunk_count; ++chunk_index) {
        const std::size_t frame_count = std::min(chunk_frame_count, remaining_frame_count);
        const std::size_t first_frame = drain_start_frame + written_frame_count;
        const std::size_t sample_count = frame_count * record.channel_count;
        const std::size_t byte_count = sample_count * sizeof(std::int16_t);
        const std::size_t first_sample = first_frame * record.channel_count;
        const bool is_final_chunk = chunk_index + 1U == required_chunk_count;
        out_chunks[chunk_index] = AudioPcmStreamQueueChunk{
            record.handle,
            record.packet,
            record.queue_id,
            record.packet_id,
            chunk_index,
            first_frame,
            frame_count,
            first_sample,
            sample_count,
            byte_count,
            is_final_chunk};
        remaining_frame_count -= frame_count;
        written_frame_count += frame_count;
    }

    record.drained_frame_count += written_frame_count;
    record.remaining_frame_count = remaining_frame_count;
    out_chunk_count = required_chunk_count;
    pcm_stream_queue_snapshot_.drained_descriptor_count += required_chunk_count;
    pcm_stream_queue_snapshot_.drained_frame_count += written_frame_count;
    pcm_stream_queue_snapshot_.last_frame_count = written_frame_count;
    SetPcmStreamQueueLastStatus(AudioStatus::Success, AudioPcmStreamQueueOperation::Drain);
    RecordSuccess();
    return AudioStatus::Success;
}

AudioStatus TestAudioDevice::ReleasePcmStreamQueue(AudioPcmStreamQueueHandle queue) {
    if (!is_initialized_) {
        return AudioStatus::InvalidDescriptor;
    }

    if (!IsPcmStreamQueueHandleValid(queue)) {
        ++pcm_stream_queue_snapshot_.stale_queue_rejected_count;
        return RecordPcmStreamQueueFailure(AudioStatus::InvalidHandle, AudioPcmStreamQueueOperation::Release);
    }

    AudioPcmStreamQueueSlot& slot = pcm_stream_queues_[queue.slot];
    pcm_stream_queue_snapshot_.last_queue_id = slot.record.queue_id;
    pcm_stream_queue_snapshot_.last_packet_id = slot.record.packet_id;
    pcm_stream_queue_snapshot_.last_frame_count = slot.record.remaining_frame_count;
    slot.is_active = false;
    slot.record = AudioPcmStreamQueueRecord{};
    ++slot.generation;
    if (slot.generation == INVALID_GENERATION) {
        ++slot.generation;
    }

    --pcm_stream_queue_snapshot_.active_queue_count;
    ++pcm_stream_queue_snapshot_.released_queue_count;
    SetPcmStreamQueueLastStatus(AudioStatus::Success, AudioPcmStreamQueueOperation::Release);
    RecordSuccess();
    return AudioStatus::Success;
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
        RecordSuccess();
        return AudioStatus::Success;
    }

    const std::size_t required_voice_count = snapshot_.active_voice_count + 1U;
    const AudioStatus status = RecordFailure(AudioStatus::CapacityExceeded);
    snapshot_.last_required_voice_count = required_voice_count;
    return status;
}

AudioStatus TestAudioDevice::StopVoice(AudioVoiceHandle handle) {
    if (!is_initialized_) {
        return AudioStatus::InvalidDescriptor;
    }

    if (!IsVoiceHandleValid(handle)) {
        return RecordFailure(AudioStatus::InvalidHandle);
    }

    StopVoiceSlot(voices_[handle.slot]);
    RecordSuccess();
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
    RecordSuccess();
    return AudioMixResult{AudioStatus::Success, requested_frames};
}

AudioCapabilities TestAudioDevice::Capabilities() const {
    return capabilities_;
}

AudioDeviceSnapshot TestAudioDevice::Snapshot() const {
    return snapshot_;
}

AudioPcmSamplePacketSnapshot TestAudioDevice::PcmSamplePacketSnapshot() const {
    return pcm_sample_packet_snapshot_;
}

AudioPcmStreamQueueSnapshot TestAudioDevice::PcmStreamQueueSnapshot() const {
    return pcm_stream_queue_snapshot_;
}

AudioStatus TestAudioDevice::RecordFailure(AudioStatus status) {
    ClearDeviceRequiredCounts(snapshot_);
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    return status;
}

void TestAudioDevice::RecordSuccess() {
    ClearDeviceRequiredCounts(snapshot_);
    snapshot_.last_status = AudioStatus::Success;
}

AudioStatus TestAudioDevice::RecordPcmSamplePacketFailure(AudioStatus status, AudioPcmSamplePacketOperation operation) {
    ++pcm_sample_packet_snapshot_.rejected_packet_count;
    SetPcmSamplePacketLastStatus(status, operation);
    return RecordFailure(status);
}

AudioStatus TestAudioDevice::RecordPcmStreamQueueFailure(AudioStatus status, AudioPcmStreamQueueOperation operation) {
    ++pcm_stream_queue_snapshot_.rejected_queue_count;
    SetPcmStreamQueueLastStatus(status, operation);
    return RecordFailure(status);
}

AudioStatus TestAudioDevice::ValidatePcmSamplePacketRequest(const AudioPcmSamplePacketRequest& request) const {
    if (request.packet_id == 0U) {
        return AudioStatus::InvalidDescriptor;
    }

    if (request.format != AudioSampleFormat::Signed16) {
        return AudioStatus::UnsupportedFormat;
    }

    if (request.sample_rate != SAMPLE_RATE) {
        return AudioStatus::UnsupportedFormat;
    }

    if (request.channel_count != CHANNEL_COUNT) {
        return AudioStatus::UnsupportedFormat;
    }

    if (request.frame_count == 0U) {
        return AudioStatus::InvalidDescriptor;
    }

    if (request.frame_count > MAX_SOURCE_FRAMES) {
        return AudioStatus::InvalidDescriptor;
    }

    const std::size_t expected_sample_count = request.frame_count * CHANNEL_COUNT;
    if (request.interleaved_sample_count != expected_sample_count) {
        return AudioStatus::InvalidDescriptor;
    }

    const std::size_t expected_byte_count = expected_sample_count * sizeof(std::int16_t);
    if (request.byte_count != expected_byte_count) {
        return AudioStatus::InvalidDescriptor;
    }

    return AudioStatus::Success;
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

bool TestAudioDevice::IsPcmSamplePacketHandleValid(AudioPcmSamplePacketHandle packet) const {
    if (packet.generation == INVALID_GENERATION) {
        return false;
    }

    if (packet.slot >= pcm_sample_packets_.size()) {
        return false;
    }

    const AudioPcmSamplePacketSlot& slot = pcm_sample_packets_[packet.slot];
    if (!slot.is_active) {
        return false;
    }

    return slot.generation == packet.generation;
}

bool TestAudioDevice::IsPcmStreamQueueHandleValid(AudioPcmStreamQueueHandle queue) const {
    if (queue.generation == INVALID_GENERATION) {
        return false;
    }

    if (queue.slot >= pcm_stream_queues_.size()) {
        return false;
    }

    const AudioPcmStreamQueueSlot& slot = pcm_stream_queues_[queue.slot];
    if (!slot.is_active) {
        return false;
    }

    return slot.generation == queue.generation;
}

bool TestAudioDevice::HasActivePcmSamplePacketId(std::uint32_t packet_id) const {
    for (const AudioPcmSamplePacketSlot& slot : pcm_sample_packets_) {
        if (!slot.is_active) {
            continue;
        }

        if (slot.record.packet_id != packet_id) {
            continue;
        }

        return true;
    }

    return false;
}

bool TestAudioDevice::HasActivePcmStreamQueueId(std::uint32_t queue_id) const {
    for (const AudioPcmStreamQueueSlot& slot : pcm_stream_queues_) {
        if (!slot.is_active) {
            continue;
        }

        if (slot.record.queue_id != queue_id) {
            continue;
        }

        return true;
    }

    return false;
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

void TestAudioDevice::SetPcmSamplePacketLastStatus(AudioStatus status, AudioPcmSamplePacketOperation operation) {
    ClearPcmSamplePacketRequiredCounts(pcm_sample_packet_snapshot_);
    pcm_sample_packet_snapshot_.last_status = status;
    pcm_sample_packet_snapshot_.last_operation = operation;
}

void TestAudioDevice::SetPcmStreamQueueLastStatus(AudioStatus status, AudioPcmStreamQueueOperation operation) {
    ClearPcmStreamQueueRequiredCounts(pcm_stream_queue_snapshot_);
    pcm_stream_queue_snapshot_.last_status = status;
    pcm_stream_queue_snapshot_.last_operation = operation;
}

void TestAudioDevice::StopVoiceSlot(AudioVoiceSlot& voice) {
    voice.is_active = false;
    voice.cursor_frame = 0U;
    ++voice.generation;
    --snapshot_.active_voice_count;
    ++snapshot_.stopped_voice_count;
}
}
