// 模块: YuEngine AudioResource
// 文件: Src/YuEngine/AudioResource/Include/YuEngine/AudioResource/AudioResourcePcmPacketImportBridge.h

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
     * @comment 构造 Audio Resource PCM packet import bridge。
     * @param desc 输入 bridge descriptor。
     */
    explicit AudioResourcePcmPacketImportBridge(
        AudioResourcePcmPacketImportBridgeDesc desc=AudioResourcePcmPacketImportBridgeDesc{});

    /**
     * @comment 将一个 active Resource audio decode-result record 映射到调用方持有的 Audio PCM packet request。
     * @param decode_result 当前 active Resource decode-result metadata。
     * @param request 输入 bridge import request。
     * @param out_handle 输出 bridge handle。
     * @param out_packet_request 输出 Audio PCM packet request。
     * @return 显式操作状态。
     */
    AudioResourcePcmPacketImportStatus ImportPcmPacket(
        const yuengine::resource::ResourceDecodeResultRecord &decode_result,
        const AudioResourcePcmPacketImportRequest &request,
        AudioResourcePcmPacketImportHandle *out_handle,
        yuengine::audio::AudioPcmSamplePacketRequest *out_packet_request);
    /**
     * @comment 按 handle 查询一个 active bridge import record。
     * @param handle 输入 bridge handle。
     * @param out_record 输出 bridge record。
     * @return 显式操作状态。
     */
    AudioResourcePcmPacketImportStatus QueryPcmPacketImport(
        AudioResourcePcmPacketImportHandle handle,
        AudioResourcePcmPacketImportRecord *out_record);
    /**
     * @comment 释放一个 active bridge import record。
     * @param handle 输入 bridge handle。
     * @return 显式操作状态。
     */
    AudioResourcePcmPacketImportStatus ReleasePcmPacketImport(
        AudioResourcePcmPacketImportHandle handle);
    /**
     * @comment 返回 bridge snapshot。
     * @return 快照值。
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
