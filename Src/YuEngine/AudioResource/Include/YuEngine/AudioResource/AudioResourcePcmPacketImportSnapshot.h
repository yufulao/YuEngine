// 模块: YuEngine AudioResource
// 文件: Src/YuEngine/AudioResource/Include/YuEngine/AudioResource/AudioResourcePcmPacketImportSnapshot.h

#pragma once

#include <cstdint>

#include "YuEngine/AudioResource/AudioResourcePcmPacketImportOperation.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportStatus.h"

namespace yuengine::audioresource {
struct AudioResourcePcmPacketImportSnapshot final {
    std::uint32_t import_capacity = 0U;
    std::uint32_t active_import_count = 0U;
    std::uint32_t last_required_import_count = 0U;
    std::uint64_t imported_packet_count = 0U;
    std::uint64_t queried_import_count = 0U;
    std::uint64_t released_import_count = 0U;
    std::uint64_t rejected_import_count = 0U;
    std::uint64_t failed_validation_count = 0U;
    std::uint64_t duplicate_import_rejected_count = 0U;
    std::uint64_t duplicate_packet_rejected_count = 0U;
    std::uint64_t stale_import_rejected_count = 0U;
    std::uint64_t capacity_rejected_count = 0U;
    std::uint64_t last_failed_import_id = 0U;
    std::uint64_t last_failed_decode_result_id = 0U;
    std::uint32_t last_failed_packet_id = 0U;
    std::uint32_t last_failed_resource_slot = 0U;
    std::uint32_t last_failed_resource_generation = 0U;
    std::uint64_t last_import_id = 0U;
    std::uint64_t last_decode_result_id = 0U;
    std::uint32_t last_packet_id = 0U;
    std::uint32_t last_decoded_byte_count = 0U;
    AudioResourcePcmPacketImportStatus last_status = AudioResourcePcmPacketImportStatus::Success;
    AudioResourcePcmPacketImportOperation last_operation = AudioResourcePcmPacketImportOperation::None;
};
}
