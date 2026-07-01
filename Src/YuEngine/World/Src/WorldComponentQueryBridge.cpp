// 模块: YuEngine World
// 文件: Src/YuEngine/World/Src/WorldComponentQueryBridge.cpp

#include "YuEngine/World/WorldComponentQueryBridge.h"

#include <array>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/World/WorldComponentAttachmentBridge.h"
#include "YuEngine/World/WorldConstants.h"

using yuengine::memory::MemoryAccountingStatus;

namespace yuengine::world {
namespace {
std::uint32_t CountTypeMatches(
    const WorldComponentAttachment *attachments,
    std::uint32_t attachment_count,
    WorldComponentTypeId component_type_id) {
    std::uint32_t matched_record_count = 0U;
    for (std::uint32_t index = 0U; index < attachment_count; ++index) {
        const WorldComponentAttachment &attachment = attachments[index];
        if (attachment.component_type_id.value != component_type_id.value) {
            continue;
        }

        ++matched_record_count;
    }

    return matched_record_count;
}

std::uint32_t WriteTypeMatches(
    const WorldComponentAttachment *attachments,
    std::uint32_t attachment_count,
    WorldComponentTypeId component_type_id,
    WorldObjectId *output_world_object_ids) {
    std::uint32_t written_record_count = 0U;
    for (std::uint32_t index = 0U; index < attachment_count; ++index) {
        const WorldComponentAttachment &attachment = attachments[index];
        if (attachment.component_type_id.value != component_type_id.value) {
            continue;
        }

        output_world_object_ids[written_record_count] = attachment.world_object_id;
        ++written_record_count;
    }

    return written_record_count;
}
}

WorldComponentQueryBridge::WorldComponentQueryBridge()
    : snapshot_{
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          WorldComponentQueryKind::None,
          WorldObjectId{},
          WorldComponentTypeId{},
          0U,
          0U,
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

    const std::uint32_t matched_record_count =
        CountTypeMatches(attachments.data(), attachment_count, desc.component_type_id);
    if (matched_record_count > desc.output_capacity) {
        return RecordOverflowResult(
            WorldComponentQueryKind::Type,
            WorldObjectId{},
            desc.component_type_id,
            desc.output_capacity,
            matched_record_count,
            0U);
    }

    const std::uint32_t written_record_count = WriteTypeMatches(
        attachments.data(),
        attachment_count,
        desc.component_type_id,
        desc.output_world_object_ids);
    return RecordSuccessResult(matched_record_count, written_record_count);
}

WorldComponentQueryResult WorldComponentQueryBridge::QueryTypes(
    const WorldComponentQueryTypeDesc *descs,
    std::uint32_t desc_count) {
    if ((desc_count != 0U) && (descs == nullptr)) {
        return RecordFailureResult(WorldComponentQueryStatus::InvalidOutputBuffer);
    }

    std::uint32_t total_matched_record_count = 0U;
    bool has_overflow = false;
    WorldComponentTypeId failed_component_type_id{};
    std::uint32_t failed_output_capacity = 0U;
    for (std::uint32_t desc_index = 0U; desc_index < desc_count; ++desc_index) {
        const WorldComponentQueryTypeDesc &desc = descs[desc_index];
        const WorldComponentQueryStatus validate_status = ValidateTypeDesc(desc);
        if (validate_status != WorldComponentQueryStatus::Success) {
            return RecordFailureResult(validate_status);
        }

        std::array<WorldComponentAttachment, MAX_WORLD_OBJECT_COUNT> attachments{};
        const std::uint32_t attachment_count =
            desc.source_bridge->ExportAttachments(attachments.data(), MAX_WORLD_OBJECT_COUNT);
        const std::uint32_t matched_record_count =
            CountTypeMatches(attachments.data(), attachment_count, desc.component_type_id);
        total_matched_record_count += matched_record_count;
        if (matched_record_count > desc.output_capacity) {
            if (!has_overflow) {
                failed_component_type_id = desc.component_type_id;
                failed_output_capacity = desc.output_capacity;
            }

            has_overflow = true;
        }
    }

    if (has_overflow) {
        return RecordOverflowResult(
            WorldComponentQueryKind::Type,
            WorldObjectId{},
            failed_component_type_id,
            failed_output_capacity,
            total_matched_record_count,
            0U);
    }

    std::uint32_t total_written_record_count = 0U;
    for (std::uint32_t desc_index = 0U; desc_index < desc_count; ++desc_index) {
        const WorldComponentQueryTypeDesc &desc = descs[desc_index];
        std::array<WorldComponentAttachment, MAX_WORLD_OBJECT_COUNT> attachments{};
        const std::uint32_t attachment_count =
            desc.source_bridge->ExportAttachments(attachments.data(), MAX_WORLD_OBJECT_COUNT);
        const std::uint32_t written_record_count = WriteTypeMatches(
            attachments.data(),
            attachment_count,
            desc.component_type_id,
            desc.output_world_object_ids);
        total_written_record_count += written_record_count;
    }

    return RecordSuccessResult(total_matched_record_count, total_written_record_count);
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
        return RecordOverflowResult(
            WorldComponentQueryKind::Object,
            desc.world_object_id,
            WorldComponentTypeId{},
            desc.output_capacity,
            matched_record_count,
            0U);
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
    ClearCapacityEntry();
    ++snapshot_.query_count;
    snapshot_.matched_record_count += matched_record_count;
    snapshot_.last_status = WorldComponentQueryStatus::Success;
    return WorldComponentQueryResult::Success(matched_record_count, written_record_count);
}

WorldComponentQueryResult WorldComponentQueryBridge::RecordFailureResult(WorldComponentQueryStatus status) {
    ClearCapacityEntry();
    ++snapshot_.failed_operation_count;
    snapshot_.last_status = status;
    return WorldComponentQueryResult::Failure(status);
}

WorldComponentQueryResult WorldComponentQueryBridge::RecordOverflowResult(
    WorldComponentQueryKind query_kind,
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    std::uint32_t output_capacity,
    std::uint32_t matched_record_count,
    std::uint32_t written_record_count) {
    ++snapshot_.query_count;
    snapshot_.matched_record_count += matched_record_count;
    ++snapshot_.overflow_rejection_count;
    ++snapshot_.failed_operation_count;
    snapshot_.last_failed_query_kind = query_kind;
    snapshot_.last_failed_world_object_id = world_object_id;
    snapshot_.last_failed_component_type_id = component_type_id;
    snapshot_.last_failed_output_capacity = output_capacity;
    snapshot_.last_required_output_count = matched_record_count;
    snapshot_.last_status = WorldComponentQueryStatus::OutputCapacityExceeded;
    WorldComponentQueryResult result = WorldComponentQueryResult::Failure(
        WorldComponentQueryStatus::OutputCapacityExceeded,
        matched_record_count,
        written_record_count);
    result.failed_query_kind = query_kind;
    result.failed_world_object_id = world_object_id;
    result.failed_component_type_id = component_type_id;
    result.output_capacity = output_capacity;
    result.required_output_count = matched_record_count;
    return result;
}

void WorldComponentQueryBridge::ClearCapacityEntry() {
    snapshot_.last_failed_query_kind = WorldComponentQueryKind::None;
    snapshot_.last_failed_world_object_id = WorldObjectId{};
    snapshot_.last_failed_component_type_id = WorldComponentTypeId{};
    snapshot_.last_failed_output_capacity = 0U;
    snapshot_.last_required_output_count = 0U;
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
