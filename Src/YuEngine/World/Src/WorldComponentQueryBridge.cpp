// 模块: YuEngine World
// 文件: Src/YuEngine/World/Src/WorldComponentQueryBridge.cpp

#include "YuEngine/World/WorldComponentQueryBridge.h"

#include <array>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/World/WorldComponentAttachmentBridge.h"
#include "YuEngine/World/WorldConstants.h"

using yuengine::memory::MemoryAccountingStatus;

namespace yuengine::world {
WorldComponentQueryBridge::WorldComponentQueryBridge()
    : snapshot_{
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          WorldComponentQueryStatus::Success} {
}

WorldComponentQueryResult WorldComponentQueryBridge::QueryType(const WorldComponentQueryTypeDesc &desc) {
    const WorldComponentQueryStatus validate_status = ValidateTypeDesc(desc);
    if (validate_status != WorldComponentQueryStatus::Success) {
        return RecordFailureResult(validate_status);
    }

    std::array<WorldComponentAttachment, MAX_WORLD_OBJECT_COUNT> attachments{};
    const std::uint32_t attachment_count =
        desc.source_bridge->ExportAttachments(attachments.data(), MAX_WORLD_OBJECT_COUNT);

    std::uint32_t matched_record_count = 0U;
    for (std::uint32_t index = 0U; index < attachment_count; ++index) {
        const WorldComponentAttachment &attachment = attachments[index];
        if (attachment.component_type_id.value != desc.component_type_id.value) {
            continue;
        }

        ++matched_record_count;
    }

    if (matched_record_count > desc.output_capacity) {
        return RecordOverflowResult(matched_record_count, 0U);
    }

    std::uint32_t written_record_count = 0U;
    for (std::uint32_t index = 0U; index < attachment_count; ++index) {
        const WorldComponentAttachment &attachment = attachments[index];
        if (attachment.component_type_id.value != desc.component_type_id.value) {
            continue;
        }

        desc.output_world_object_ids[written_record_count] = attachment.world_object_id;
        ++written_record_count;
    }

    return RecordSuccessResult(matched_record_count, written_record_count);
}

WorldComponentQueryResult WorldComponentQueryBridge::QueryObject(const WorldComponentQueryObjectDesc &desc) {
    const WorldComponentQueryStatus validate_status = ValidateObjectDesc(desc);
    if (validate_status != WorldComponentQueryStatus::Success) {
        return RecordFailureResult(validate_status);
    }

    std::array<WorldComponentAttachment, MAX_WORLD_OBJECT_COUNT> attachments{};
    const std::uint32_t attachment_count =
        desc.source_bridge->ExportAttachments(attachments.data(), MAX_WORLD_OBJECT_COUNT);

    std::uint32_t matched_record_count = 0U;
    for (std::uint32_t index = 0U; index < attachment_count; ++index) {
        const WorldComponentAttachment &attachment = attachments[index];
        if (attachment.world_object_id.value != desc.world_object_id.value) {
            continue;
        }

        ++matched_record_count;
    }

    if (matched_record_count > desc.output_capacity) {
        return RecordOverflowResult(matched_record_count, 0U);
    }

    std::uint32_t written_record_count = 0U;
    for (std::uint32_t index = 0U; index < attachment_count; ++index) {
        const WorldComponentAttachment &attachment = attachments[index];
        if (attachment.world_object_id.value != desc.world_object_id.value) {
            continue;
        }

        desc.output_attachments[written_record_count] = attachment;
        ++written_record_count;
    }

    return RecordSuccessResult(matched_record_count, written_record_count);
}

WorldComponentQuerySnapshot WorldComponentQueryBridge::Snapshot() const {
    return snapshot_;
}

WorldComponentQueryResult WorldComponentQueryBridge::RecordSuccessResult(
    std::uint32_t matched_record_count,
    std::uint32_t written_record_count) {
    ++snapshot_.query_count;
    snapshot_.matched_record_count += matched_record_count;
    snapshot_.last_status = WorldComponentQueryStatus::Success;
    return WorldComponentQueryResult::Success(matched_record_count, written_record_count);
}

WorldComponentQueryResult WorldComponentQueryBridge::RecordFailureResult(WorldComponentQueryStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    return WorldComponentQueryResult::Failure(status);
}

WorldComponentQueryResult WorldComponentQueryBridge::RecordOverflowResult(
    std::uint32_t matched_record_count,
    std::uint32_t written_record_count) {
    ++snapshot_.query_count;
    snapshot_.matched_record_count += matched_record_count;
    ++snapshot_.overflow_rejection_count;
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = WorldComponentQueryStatus::OutputCapacityExceeded;
    return WorldComponentQueryResult::Failure(
        WorldComponentQueryStatus::OutputCapacityExceeded,
        matched_record_count,
        written_record_count);
}

WorldComponentQueryStatus WorldComponentQueryBridge::ValidateTypeDesc(
    const WorldComponentQueryTypeDesc &desc) const {
    if (desc.source_bridge == nullptr) {
        return WorldComponentQueryStatus::InvalidSourceBridge;
    }

    if (!desc.component_type_id.IsValid()) {
        return WorldComponentQueryStatus::InvalidComponentTypeId;
    }

    if ((desc.output_capacity != 0U) && (desc.output_world_object_ids == nullptr)) {
        return WorldComponentQueryStatus::InvalidOutputBuffer;
    }

    return WorldComponentQueryStatus::Success;
}

WorldComponentQueryStatus WorldComponentQueryBridge::ValidateObjectDesc(
    const WorldComponentQueryObjectDesc &desc) const {
    if (desc.source_bridge == nullptr) {
        return WorldComponentQueryStatus::InvalidSourceBridge;
    }

    if (!desc.world_object_id.IsValid()) {
        return WorldComponentQueryStatus::InvalidWorldObjectId;
    }

    if ((desc.output_capacity != 0U) && (desc.output_attachments == nullptr)) {
        return WorldComponentQueryStatus::InvalidOutputBuffer;
    }

    return WorldComponentQueryStatus::Success;
}
}
