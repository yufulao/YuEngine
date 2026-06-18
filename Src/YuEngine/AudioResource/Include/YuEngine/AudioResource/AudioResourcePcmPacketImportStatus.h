// 模块: YuEngine AudioResource
// 文件: Src/YuEngine/AudioResource/Include/YuEngine/AudioResource/AudioResourcePcmPacketImportStatus.h

#pragma once

namespace yuengine::audioresource {
enum class AudioResourcePcmPacketImportStatus {
    Success,
    InvalidArgument,
    InvalidBridgeCapacity,
    InvalidImportId,
    InvalidResourceHandle,
    ResourceTypeMismatch,
    InvalidPayloadId,
    InvalidDecodePlanId,
    InvalidDecodeResultId,
    StaleDecodeResult,
    AssetClassMismatch,
    ResultClassMismatch,
    DecodeResultByteCountMismatch,
    InvalidPacketId,
    UnsupportedFormat,
    UnsupportedSampleRate,
    UnsupportedChannelCount,
    InvalidFrameCount,
    SampleCountMismatch,
    ByteCountMismatch,
    DuplicateImportId,
    DuplicatePacketId,
    CapacityExceeded,
    InvalidHandle,
    GenerationMismatch
};
}
