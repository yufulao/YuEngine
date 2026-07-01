// 模块: YuEngine AudioResource
// 文件: Src/YuEngine/AudioResource/Src/AudioResourcePcmPacketImportBridge.cpp

#include "YuEngine/AudioResource/AudioResourcePcmPacketImportBridge.h"

#include <cstddef>
#include <cstdint>

#include "YuEngine/Audio/AudioConstants.h"
#include "YuEngine/Audio/AudioSampleFormat.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodeResultClass.h"

namespace yuengine::audioresource {
namespace {
AudioResourcePcmPacketImportBridgeDesc NormalizeDesc(AudioResourcePcmPacketImportBridgeDesc desc) {
    if (desc.import_capacity > MAX_AUDIO_RESOURCE_PCM_PACKET_IMPORT_RECORDS) {
        desc.import_capacity = MAX_AUDIO_RESOURCE_PCM_PACKET_IMPORT_RECORDS;
    }

    return desc;
}

bool ResourceHandlesMatch(
    yuengine::resource::ResourceHandle left,
    yuengine::resource::ResourceHandle right) {
    if (left.slot != right.slot) {
        return false;
    }

    return left.generation == right.generation;
}

std::uint32_t ToU32ByteCount(std::size_t byte_count) {
    return static_cast<std::uint32_t>(byte_count);
}
}

AudioResourcePcmPacketImportBridge::AudioResourcePcmPacketImportBridge(
    AudioResourcePcmPacketImportBridgeDesc desc)
    : slots_{},
      snapshot_{} {
    const AudioResourcePcmPacketImportBridgeDesc normalized_desc = NormalizeDesc(desc);
    snapshot_.import_capacity = normalized_desc.import_capacity;
    if (normalized_desc.import_capacity == 0U) {
        snapshot_.last_status = AudioResourcePcmPacketImportStatus::InvalidBridgeCapacity;
    }
}

AudioResourcePcmPacketImportStatus AudioResourcePcmPacketImportBridge::ImportPcmPacket(
    const yuengine::resource::ResourceDecodeResultRecord &decode_result,
    const AudioResourcePcmPacketImportRequest &request,
    AudioResourcePcmPacketImportHandle *out_handle,
    yuengine::audio::AudioPcmSamplePacketRequest *out_packet_request) {
    if (out_handle != nullptr) {
        *out_handle = AudioResourcePcmPacketImportHandle{};
    }

    if (out_packet_request != nullptr) {
        *out_packet_request = yuengine::audio::AudioPcmSamplePacketRequest{};
    }

    AudioResourcePcmPacketImportStatus status = ValidateBridgeCapacity();
    if (status != AudioResourcePcmPacketImportStatus::Success) {
        return RecordRejectedImport(status, request);
    }

    if (out_handle == nullptr) {
        return RecordRejectedImport(AudioResourcePcmPacketImportStatus::InvalidArgument, request);
    }

    if (out_packet_request == nullptr) {
        return RecordRejectedImport(AudioResourcePcmPacketImportStatus::InvalidArgument, request);
    }

    status = ValidateRequest(request);
    if (status != AudioResourcePcmPacketImportStatus::Success) {
        return RecordRejectedImport(status, request);
    }

    status = ValidatePacketRequest(request.packet_request);
    if (status != AudioResourcePcmPacketImportStatus::Success) {
        return RecordRejectedImport(status, request);
    }

    status = ValidateDecodeResult(decode_result, request);
    if (status != AudioResourcePcmPacketImportStatus::Success) {
        return RecordRejectedImport(status, request);
    }

    if (HasImportId(request.import_id)) {
        return RecordRejectedImport(AudioResourcePcmPacketImportStatus::DuplicateImportId, request);
    }

    if (HasPacketId(request.packet_request.packet_id)) {
        return RecordRejectedImport(AudioResourcePcmPacketImportStatus::DuplicatePacketId, request);
    }

    if (snapshot_.active_import_count >= snapshot_.import_capacity) {
        return RecordRejectedImport(AudioResourcePcmPacketImportStatus::CapacityExceeded, request);
    }

    AudioResourcePcmPacketImportSlot *slot = FindFreeSlot();
    if (slot == nullptr) {
        return RecordRejectedImport(AudioResourcePcmPacketImportStatus::CapacityExceeded, request);
    }

    RecordImportedPacket(slot, decode_result, request, out_handle, out_packet_request);
    return AudioResourcePcmPacketImportStatus::Success;
}

AudioResourcePcmPacketImportStatus AudioResourcePcmPacketImportBridge::QueryPcmPacketImport(
    AudioResourcePcmPacketImportHandle handle,
    AudioResourcePcmPacketImportRecord *out_record) {
    if (out_record != nullptr) {
        *out_record = AudioResourcePcmPacketImportRecord{};
    }

    AudioResourcePcmPacketImportStatus status = ValidateBridgeCapacity();
    if (status != AudioResourcePcmPacketImportStatus::Success) {
        return RecordRejectedHandle(status, AudioResourcePcmPacketImportOperation::Query);
    }

    if (out_record == nullptr) {
        return RecordRejectedHandle(
            AudioResourcePcmPacketImportStatus::InvalidArgument,
            AudioResourcePcmPacketImportOperation::Query);
    }

    status = ValidateHandle(handle);
    if (status != AudioResourcePcmPacketImportStatus::Success) {
        return RecordRejectedHandle(status, AudioResourcePcmPacketImportOperation::Query);
    }

    const AudioResourcePcmPacketImportSlot *slot = FindSlot(handle);
    if (slot == nullptr) {
        return RecordRejectedHandle(
            AudioResourcePcmPacketImportStatus::InvalidHandle,
            AudioResourcePcmPacketImportOperation::Query);
    }

    *out_record = slot->record;
    RecordQuerySuccess(slot->record);
    return AudioResourcePcmPacketImportStatus::Success;
}

AudioResourcePcmPacketImportStatus AudioResourcePcmPacketImportBridge::ReleasePcmPacketImport(
    AudioResourcePcmPacketImportHandle handle) {
    AudioResourcePcmPacketImportStatus status = ValidateBridgeCapacity();
    if (status != AudioResourcePcmPacketImportStatus::Success) {
        return RecordRejectedHandle(status, AudioResourcePcmPacketImportOperation::Release);
    }

    status = ValidateHandle(handle);
    if (status != AudioResourcePcmPacketImportStatus::Success) {
        return RecordRejectedHandle(status, AudioResourcePcmPacketImportOperation::Release);
    }

    AudioResourcePcmPacketImportSlot *slot = FindSlot(handle);
    if (slot == nullptr) {
        return RecordRejectedHandle(
            AudioResourcePcmPacketImportStatus::InvalidHandle,
            AudioResourcePcmPacketImportOperation::Release);
    }

    const AudioResourcePcmPacketImportRecord record = slot->record;
    slot->record = AudioResourcePcmPacketImportRecord{};
    slot->is_active = false;
    AdvanceGeneration(slot);
    --snapshot_.active_import_count;
    RecordReleaseSuccess(record);
    return AudioResourcePcmPacketImportStatus::Success;
}

AudioResourcePcmPacketImportSnapshot AudioResourcePcmPacketImportBridge::Snapshot() const {
    return snapshot_;
}

AudioResourcePcmPacketImportStatus AudioResourcePcmPacketImportBridge::ValidateBridgeCapacity() const {
    if (snapshot_.import_capacity == 0U) {
        return AudioResourcePcmPacketImportStatus::InvalidBridgeCapacity;
    }

    return AudioResourcePcmPacketImportStatus::Success;
}

AudioResourcePcmPacketImportStatus AudioResourcePcmPacketImportBridge::ValidateRequest(
    const AudioResourcePcmPacketImportRequest &request) const {
    if (request.import_id == 0U) {
        return AudioResourcePcmPacketImportStatus::InvalidImportId;
    }

    if (!request.resource.IsValid()) {
        return AudioResourcePcmPacketImportStatus::InvalidResourceHandle;
    }

    if (!request.expected_type.IsValid()) {
        return AudioResourcePcmPacketImportStatus::ResourceTypeMismatch;
    }

    if (request.payload_id == 0U) {
        return AudioResourcePcmPacketImportStatus::InvalidPayloadId;
    }

    if (request.decode_plan_id == 0U) {
        return AudioResourcePcmPacketImportStatus::InvalidDecodePlanId;
    }

    if (request.decode_result_id == 0U) {
        return AudioResourcePcmPacketImportStatus::InvalidDecodeResultId;
    }

    return AudioResourcePcmPacketImportStatus::Success;
}

AudioResourcePcmPacketImportStatus AudioResourcePcmPacketImportBridge::ValidatePacketRequest(
    const yuengine::audio::AudioPcmSamplePacketRequest &packet_request) const {
    if (packet_request.packet_id == 0U) {
        return AudioResourcePcmPacketImportStatus::InvalidPacketId;
    }

    if (packet_request.format != yuengine::audio::AudioSampleFormat::Signed16) {
        return AudioResourcePcmPacketImportStatus::UnsupportedFormat;
    }

    if (packet_request.sample_rate != yuengine::audio::SAMPLE_RATE) {
        return AudioResourcePcmPacketImportStatus::UnsupportedSampleRate;
    }

    if (packet_request.channel_count != yuengine::audio::CHANNEL_COUNT) {
        return AudioResourcePcmPacketImportStatus::UnsupportedChannelCount;
    }

    if (packet_request.frame_count == 0U) {
        return AudioResourcePcmPacketImportStatus::InvalidFrameCount;
    }

    if (packet_request.frame_count > yuengine::audio::MAX_SOURCE_FRAMES) {
        return AudioResourcePcmPacketImportStatus::InvalidFrameCount;
    }

    const std::size_t expected_sample_count = packet_request.frame_count * yuengine::audio::CHANNEL_COUNT;
    if (packet_request.interleaved_sample_count != expected_sample_count) {
        return AudioResourcePcmPacketImportStatus::SampleCountMismatch;
    }

    const std::size_t expected_byte_count = expected_sample_count * sizeof(std::int16_t);
    if (packet_request.byte_count != expected_byte_count) {
        return AudioResourcePcmPacketImportStatus::ByteCountMismatch;
    }

    return AudioResourcePcmPacketImportStatus::Success;
}

AudioResourcePcmPacketImportStatus AudioResourcePcmPacketImportBridge::ValidateDecodeResult(
    const yuengine::resource::ResourceDecodeResultRecord &decode_result,
    const AudioResourcePcmPacketImportRequest &request) const {
    if (!decode_result.is_active) {
        return AudioResourcePcmPacketImportStatus::StaleDecodeResult;
    }

    if (!ResourceHandlesMatch(decode_result.resource, request.resource)) {
        return AudioResourcePcmPacketImportStatus::InvalidResourceHandle;
    }

    if (decode_result.expected_type.value != request.expected_type.value) {
        return AudioResourcePcmPacketImportStatus::ResourceTypeMismatch;
    }

    if (decode_result.payload_id != request.payload_id) {
        return AudioResourcePcmPacketImportStatus::InvalidPayloadId;
    }

    if (decode_result.decode_plan_id != request.decode_plan_id) {
        return AudioResourcePcmPacketImportStatus::InvalidDecodePlanId;
    }

    if (decode_result.decode_result_id != request.decode_result_id) {
        return AudioResourcePcmPacketImportStatus::InvalidDecodeResultId;
    }

    if (decode_result.asset_class != yuengine::resource::ResourceDecodePlanAssetClass::Audio) {
        return AudioResourcePcmPacketImportStatus::AssetClassMismatch;
    }

    if (decode_result.result_class != yuengine::resource::ResourceDecodeResultClass::Audio) {
        return AudioResourcePcmPacketImportStatus::ResultClassMismatch;
    }

    if (decode_result.decoded_byte_count == 0U) {
        return AudioResourcePcmPacketImportStatus::DecodeResultByteCountMismatch;
    }

    if (decode_result.decoded_byte_count != ToU32ByteCount(request.packet_request.byte_count)) {
        return AudioResourcePcmPacketImportStatus::DecodeResultByteCountMismatch;
    }

    return AudioResourcePcmPacketImportStatus::Success;
}

AudioResourcePcmPacketImportStatus AudioResourcePcmPacketImportBridge::ValidateHandle(
    AudioResourcePcmPacketImportHandle handle) const {
    if (!handle.IsValid()) {
        return AudioResourcePcmPacketImportStatus::InvalidHandle;
    }

    if (handle.slot >= snapshot_.import_capacity) {
        return AudioResourcePcmPacketImportStatus::InvalidHandle;
    }

    const AudioResourcePcmPacketImportSlot &slot = slots_[handle.slot];
    if (!slot.is_active) {
        return AudioResourcePcmPacketImportStatus::GenerationMismatch;
    }

    if (slot.generation != handle.generation) {
        return AudioResourcePcmPacketImportStatus::GenerationMismatch;
    }

    return AudioResourcePcmPacketImportStatus::Success;
}

bool AudioResourcePcmPacketImportBridge::HasImportId(std::uint64_t import_id) const {
    for (std::uint32_t index = 0U; index < snapshot_.import_capacity; ++index) {
        const AudioResourcePcmPacketImportSlot &slot = slots_[index];
        if (!slot.is_active) {
            continue;
        }

        if (slot.record.import_id == import_id) {
            return true;
        }
    }

    return false;
}

bool AudioResourcePcmPacketImportBridge::HasPacketId(std::uint32_t packet_id) const {
    for (std::uint32_t index = 0U; index < snapshot_.import_capacity; ++index) {
        const AudioResourcePcmPacketImportSlot &slot = slots_[index];
        if (!slot.is_active) {
            continue;
        }

        if (slot.record.packet_request.packet_id == packet_id) {
            return true;
        }
    }

    return false;
}

AudioResourcePcmPacketImportSlot *AudioResourcePcmPacketImportBridge::FindFreeSlot() {
    for (std::uint32_t index = 0U; index < snapshot_.import_capacity; ++index) {
        AudioResourcePcmPacketImportSlot &slot = slots_[index];
        if (slot.is_active) {
            continue;
        }

        return &slot;
    }

    return nullptr;
}

AudioResourcePcmPacketImportSlot *AudioResourcePcmPacketImportBridge::FindSlot(
    AudioResourcePcmPacketImportHandle handle) {
    if (handle.slot >= snapshot_.import_capacity) {
        return nullptr;
    }

    return &slots_[handle.slot];
}

const AudioResourcePcmPacketImportSlot *AudioResourcePcmPacketImportBridge::FindSlot(
    AudioResourcePcmPacketImportHandle handle) const {
    if (handle.slot >= snapshot_.import_capacity) {
        return nullptr;
    }

    return &slots_[handle.slot];
}

void AudioResourcePcmPacketImportBridge::RecordImportedPacket(
    AudioResourcePcmPacketImportSlot *slot,
    const yuengine::resource::ResourceDecodeResultRecord &decode_result,
    const AudioResourcePcmPacketImportRequest &request,
    AudioResourcePcmPacketImportHandle *out_handle,
    yuengine::audio::AudioPcmSamplePacketRequest *out_packet_request) {
    if (slot == nullptr) {
        return;
    }

    if (out_handle == nullptr) {
        return;
    }

    if (out_packet_request == nullptr) {
        return;
    }

    slot->is_active = true;
    slot->record.handle = AudioResourcePcmPacketImportHandle{
        static_cast<std::uint32_t>(slot - slots_.data()),
        slot->generation};
    slot->record.import_id = request.import_id;
    slot->record.resource = request.resource;
    slot->record.expected_type = request.expected_type;
    slot->record.payload_id = request.payload_id;
    slot->record.decode_plan_id = request.decode_plan_id;
    slot->record.decode_result_id = request.decode_result_id;
    slot->record.asset_class = decode_result.asset_class;
    slot->record.result_class = decode_result.result_class;
    slot->record.decoded_byte_count = decode_result.decoded_byte_count;
    slot->record.packet_request = request.packet_request;
    slot->record.status = AudioResourcePcmPacketImportStatus::Success;
    slot->record.is_active = true;

    *out_handle = slot->record.handle;
    *out_packet_request = request.packet_request;
    ++snapshot_.active_import_count;
    ++snapshot_.imported_packet_count;
    snapshot_.last_required_import_count = 0U;
    snapshot_.last_import_id = request.import_id;
    snapshot_.last_decode_result_id = request.decode_result_id;
    snapshot_.last_packet_id = request.packet_request.packet_id;
    snapshot_.last_decoded_byte_count = decode_result.decoded_byte_count;
    snapshot_.last_status = AudioResourcePcmPacketImportStatus::Success;
    snapshot_.last_operation = AudioResourcePcmPacketImportOperation::Import;
}

AudioResourcePcmPacketImportStatus AudioResourcePcmPacketImportBridge::RecordRejectedImport(
    AudioResourcePcmPacketImportStatus status,
    const AudioResourcePcmPacketImportRequest &request) {
    ++snapshot_.rejected_import_count;
    snapshot_.last_import_id = request.import_id;
    snapshot_.last_decode_result_id = request.decode_result_id;
    snapshot_.last_packet_id = request.packet_request.packet_id;
    snapshot_.last_decoded_byte_count = ToU32ByteCount(request.packet_request.byte_count);
    snapshot_.last_status = status;
    snapshot_.last_operation = AudioResourcePcmPacketImportOperation::Import;
    snapshot_.last_required_import_count = 0U;

    if (status == AudioResourcePcmPacketImportStatus::DuplicateImportId) {
        ++snapshot_.duplicate_import_rejected_count;
        return status;
    }

    if (status == AudioResourcePcmPacketImportStatus::DuplicatePacketId) {
        ++snapshot_.duplicate_packet_rejected_count;
        return status;
    }

    if (status == AudioResourcePcmPacketImportStatus::CapacityExceeded) {
        const std::uint32_t required_import_count = snapshot_.active_import_count + 1U;
        ++snapshot_.capacity_rejected_count;
        snapshot_.last_required_import_count = required_import_count;
        return status;
    }

    if (status == AudioResourcePcmPacketImportStatus::StaleDecodeResult) {
        ++snapshot_.stale_import_rejected_count;
        return status;
    }

    ++snapshot_.failed_validation_count;
    return status;
}

AudioResourcePcmPacketImportStatus AudioResourcePcmPacketImportBridge::RecordRejectedHandle(
    AudioResourcePcmPacketImportStatus status,
    AudioResourcePcmPacketImportOperation operation) {
    ++snapshot_.rejected_import_count;
    snapshot_.last_status = status;
    snapshot_.last_operation = operation;
    snapshot_.last_required_import_count = 0U;

    if (status == AudioResourcePcmPacketImportStatus::InvalidHandle) {
        ++snapshot_.stale_import_rejected_count;
        return status;
    }

    if (status == AudioResourcePcmPacketImportStatus::GenerationMismatch) {
        ++snapshot_.stale_import_rejected_count;
        return status;
    }

    if (status == AudioResourcePcmPacketImportStatus::CapacityExceeded) {
        ++snapshot_.capacity_rejected_count;
        return status;
    }

    ++snapshot_.failed_validation_count;
    return status;
}

void AudioResourcePcmPacketImportBridge::RecordQuerySuccess(
    const AudioResourcePcmPacketImportRecord &record) {
    ++snapshot_.queried_import_count;
    snapshot_.last_required_import_count = 0U;
    snapshot_.last_import_id = record.import_id;
    snapshot_.last_decode_result_id = record.decode_result_id;
    snapshot_.last_packet_id = record.packet_request.packet_id;
    snapshot_.last_decoded_byte_count = record.decoded_byte_count;
    snapshot_.last_status = AudioResourcePcmPacketImportStatus::Success;
    snapshot_.last_operation = AudioResourcePcmPacketImportOperation::Query;
}

void AudioResourcePcmPacketImportBridge::RecordReleaseSuccess(
    const AudioResourcePcmPacketImportRecord &record) {
    ++snapshot_.released_import_count;
    snapshot_.last_required_import_count = 0U;
    snapshot_.last_import_id = record.import_id;
    snapshot_.last_decode_result_id = record.decode_result_id;
    snapshot_.last_packet_id = record.packet_request.packet_id;
    snapshot_.last_decoded_byte_count = record.decoded_byte_count;
    snapshot_.last_status = AudioResourcePcmPacketImportStatus::Success;
    snapshot_.last_operation = AudioResourcePcmPacketImportOperation::Release;
}

void AudioResourcePcmPacketImportBridge::AdvanceGeneration(AudioResourcePcmPacketImportSlot *slot) {
    if (slot == nullptr) {
        return;
    }

    ++slot->generation;
    if (slot->generation == INVALID_AUDIO_RESOURCE_PCM_PACKET_IMPORT_GENERATION) {
        ++slot->generation;
    }
}
}
