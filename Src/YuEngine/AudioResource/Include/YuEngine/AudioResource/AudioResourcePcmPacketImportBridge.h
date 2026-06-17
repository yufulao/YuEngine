// Module: YuEngine AudioResource
// File: Src/YuEngine/AudioResource/Include/YuEngine/AudioResource/AudioResourcePcmPacketImportBridge.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/Audio/AudioPcmSamplePacketRequest.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportBridgeDesc.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportConstants.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportHandle.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportRecord.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportRequest.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportSlot.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportSnapshot.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportStatus.h"
#include "YuEngine/Resource/ResourceDecodeResultRecord.h"

namespace yuengine::audioresource {
class AudioResourcePcmPacketImportBridge final {
public:
    /**
     * @comment Constructs an Audio Resource PCM packet import bridge.
     * @param desc Input bridge descriptor.
     */
    explicit AudioResourcePcmPacketImportBridge(
        AudioResourcePcmPacketImportBridgeDesc desc=AudioResourcePcmPacketImportBridgeDesc{});

    /**
     * @comment Maps one active Resource audio decode-result record to a caller-owned Audio PCM packet request.
     * @param decode_result Active Resource decode-result metadata.
     * @param request Input bridge import request.
     * @param out_handle Output bridge handle.
     * @param out_packet_request Output Audio PCM packet request.
     * @return Explicit operation status.
     */
    AudioResourcePcmPacketImportStatus ImportPcmPacket(
        const yuengine::resource::ResourceDecodeResultRecord &decode_result,
        const AudioResourcePcmPacketImportRequest &request,
        AudioResourcePcmPacketImportHandle *out_handle,
        yuengine::audio::AudioPcmSamplePacketRequest *out_packet_request);
    /**
     * @comment Queries one active bridge import record by handle.
     * @param handle Input bridge handle.
     * @param out_record Output bridge record.
     * @return Explicit operation status.
     */
    AudioResourcePcmPacketImportStatus QueryPcmPacketImport(
        AudioResourcePcmPacketImportHandle handle,
        AudioResourcePcmPacketImportRecord *out_record);
    /**
     * @comment Releases one active bridge import record.
     * @param handle Input bridge handle.
     * @return Explicit operation status.
     */
    AudioResourcePcmPacketImportStatus ReleasePcmPacketImport(
        AudioResourcePcmPacketImportHandle handle);
    /**
     * @comment Returns the bridge snapshot.
     * @return Snapshot value.
     */
    AudioResourcePcmPacketImportSnapshot Snapshot() const;

private:
    AudioResourcePcmPacketImportStatus ValidateBridgeCapacity() const;
    AudioResourcePcmPacketImportStatus ValidateRequest(
        const AudioResourcePcmPacketImportRequest &request) const;
    AudioResourcePcmPacketImportStatus ValidatePacketRequest(
        const yuengine::audio::AudioPcmSamplePacketRequest &packet_request) const;
    AudioResourcePcmPacketImportStatus ValidateDecodeResult(
        const yuengine::resource::ResourceDecodeResultRecord &decode_result,
        const AudioResourcePcmPacketImportRequest &request) const;
    AudioResourcePcmPacketImportStatus ValidateHandle(
        AudioResourcePcmPacketImportHandle handle) const;
    bool HasImportId(std::uint64_t import_id) const;
    bool HasPacketId(std::uint32_t packet_id) const;
    AudioResourcePcmPacketImportSlot *FindFreeSlot();
    AudioResourcePcmPacketImportSlot *FindSlot(AudioResourcePcmPacketImportHandle handle);
    const AudioResourcePcmPacketImportSlot *FindSlot(AudioResourcePcmPacketImportHandle handle) const;
    void RecordImportedPacket(
        AudioResourcePcmPacketImportSlot *slot,
        const yuengine::resource::ResourceDecodeResultRecord &decode_result,
        const AudioResourcePcmPacketImportRequest &request,
        AudioResourcePcmPacketImportHandle *out_handle,
        yuengine::audio::AudioPcmSamplePacketRequest *out_packet_request);
    AudioResourcePcmPacketImportStatus RecordRejectedImport(
        AudioResourcePcmPacketImportStatus status,
        const AudioResourcePcmPacketImportRequest &request);
    AudioResourcePcmPacketImportStatus RecordRejectedHandle(
        AudioResourcePcmPacketImportStatus status,
        AudioResourcePcmPacketImportOperation operation);
    void RecordQuerySuccess(const AudioResourcePcmPacketImportRecord &record);
    void RecordReleaseSuccess(const AudioResourcePcmPacketImportRecord &record);
    void AdvanceGeneration(AudioResourcePcmPacketImportSlot *slot);

    std::array<AudioResourcePcmPacketImportSlot, MAX_AUDIO_RESOURCE_PCM_PACKET_IMPORT_RECORDS> slots_;
    AudioResourcePcmPacketImportSnapshot snapshot_;
};
}
