// 模块: YuEngine World
// 文件: Src/YuEngine/World/Src/WorldSceneRecordValueStreamBridge.cpp

#include "YuEngine/World/WorldSceneRecordValueStreamBridge.h"

#include <array>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Serialize/SerializeConstants.h"
#include "YuEngine/Serialize/SerializeReader.h"
#include "YuEngine/Serialize/SerializeSnapshot.h"
#include "YuEngine/Serialize/SerializeWriter.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldSceneAssemblyManifestStreamBridge.h"
#include "YuEngine/World/WorldSceneAssemblyManifestStreamConstants.h"
#include "YuEngine/World/WorldSceneAssemblyManifestStreamDesc.h"
#include "YuEngine/World/WorldSceneAssemblyManifestStreamResult.h"
#include "YuEngine/World/WorldSceneAssemblyManifestStreamStatus.h"
#include "YuEngine/World/WorldSceneObjectTransformManifestStreamBridge.h"
#include "YuEngine/World/WorldSceneObjectTransformManifestStreamConstants.h"
#include "YuEngine/World/WorldSceneObjectTransformManifestStreamDesc.h"
#include "YuEngine/World/WorldSceneObjectTransformManifestStreamResult.h"
#include "YuEngine/World/WorldSceneObjectTransformManifestStreamStatus.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::serialize::SerializeSnapshot;
using yuengine::serialize::SerializeStatus;
using yuengine::serialize::SerializeWriter;

namespace yuengine::world {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity) {
    if (requested_capacity > MAX_WORLD_OBJECT_COUNT) {
        return MAX_WORLD_OBJECT_COUNT;
    }

    return requested_capacity;
}

std::uint32_t CalculateIdentityChunkCount(std::uint32_t record_count) {
    return (record_count + WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_CHUNK_RECORD_CAPACITY - 1U) /
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_CHUNK_RECORD_CAPACITY;
}

std::uint32_t CalculateTransformChunkCount(std::uint32_t record_count) {
    return (record_count + WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_CHUNK_RECORD_CAPACITY - 1U) /
        WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_CHUNK_RECORD_CAPACITY;
}

std::uint32_t CalculateAttachmentChunkCount(std::uint32_t record_count) {
    return (record_count + WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_CHUNK_RECORD_CAPACITY - 1U) /
        WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_CHUNK_RECORD_CAPACITY;
}

std::uint32_t CalculateBindingChunkCount(std::uint32_t record_count) {
    return (record_count + WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_CHUNK_RECORD_CAPACITY - 1U) /
        WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_CHUNK_RECORD_CAPACITY;
}

std::uint32_t GetIdentityChunkRecordCount(std::uint32_t record_count, std::uint32_t chunk_index) {
    const std::uint32_t first_record_index =
        chunk_index * WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_CHUNK_RECORD_CAPACITY;
    if (record_count <= first_record_index) {
        return 0U;
    }

    const std::uint32_t remaining_record_count = record_count - first_record_index;
    if (remaining_record_count > WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_CHUNK_RECORD_CAPACITY) {
        return WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_CHUNK_RECORD_CAPACITY;
    }

    return remaining_record_count;
}

std::uint32_t GetTransformChunkRecordCount(std::uint32_t record_count, std::uint32_t chunk_index) {
    const std::uint32_t first_record_index =
        chunk_index * WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_CHUNK_RECORD_CAPACITY;
    if (record_count <= first_record_index) {
        return 0U;
    }

    const std::uint32_t remaining_record_count = record_count - first_record_index;
    if (remaining_record_count > WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_CHUNK_RECORD_CAPACITY) {
        return WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_CHUNK_RECORD_CAPACITY;
    }

    return remaining_record_count;
}

std::uint32_t GetAttachmentChunkRecordCount(std::uint32_t record_count, std::uint32_t chunk_index) {
    const std::uint32_t first_record_index =
        chunk_index * WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_CHUNK_RECORD_CAPACITY;
    if (record_count <= first_record_index) {
        return 0U;
    }

    const std::uint32_t remaining_record_count = record_count - first_record_index;
    if (remaining_record_count > WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_CHUNK_RECORD_CAPACITY) {
        return WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_CHUNK_RECORD_CAPACITY;
    }

    return remaining_record_count;
}

std::uint32_t GetBindingChunkRecordCount(std::uint32_t record_count, std::uint32_t chunk_index) {
    const std::uint32_t first_record_index =
        chunk_index * WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_CHUNK_RECORD_CAPACITY;
    if (record_count <= first_record_index) {
        return 0U;
    }

    const std::uint32_t remaining_record_count = record_count - first_record_index;
    if (remaining_record_count > WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_CHUNK_RECORD_CAPACITY) {
        return WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_CHUNK_RECORD_CAPACITY;
    }

    return remaining_record_count;
}

std::uint32_t RequiredChunkWriteByteCount(std::uint32_t field_header_byte_count, std::uint32_t payload_byte_count) {
    return yuengine::serialize::RECORD_HEADER_BYTE_COUNT + field_header_byte_count + payload_byte_count;
}

std::uint32_t RequiredIdentityChunkPayloadByteCount(std::uint32_t chunk_record_count) {
    return chunk_record_count * WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_IDENTITY_RECORD_BYTE_COUNT;
}

std::uint32_t RequiredTransformChunkPayloadByteCount(std::uint32_t chunk_record_count) {
    return chunk_record_count * WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_TRANSFORM_RECORD_BYTE_COUNT;
}

std::uint32_t RequiredAttachmentChunkPayloadByteCount(std::uint32_t chunk_record_count) {
    return chunk_record_count * WORLD_SCENE_ASSEMBLY_MANIFEST_ATTACHMENT_RECORD_BYTE_COUNT;
}

std::uint32_t RequiredBindingChunkPayloadByteCount(std::uint32_t chunk_record_count) {
    return chunk_record_count * WORLD_SCENE_ASSEMBLY_MANIFEST_BINDING_RECORD_BYTE_COUNT;
}

std::uint32_t RequiredObjectTransformWriteByteCount(
    std::uint32_t identity_record_count,
    std::uint32_t transform_record_count) {
    std::uint32_t result = WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_METADATA_RECORD_BYTE_COUNT;

    const std::uint32_t identity_chunk_count = CalculateIdentityChunkCount(identity_record_count);
    std::uint32_t identity_chunk_index = 0U;
    while (identity_chunk_index < identity_chunk_count) {
        const std::uint32_t chunk_record_count = GetIdentityChunkRecordCount(
            identity_record_count,
            identity_chunk_index);
        const std::uint32_t payload_byte_count =
            RequiredIdentityChunkPayloadByteCount(chunk_record_count);
        result += RequiredChunkWriteByteCount(
            WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_FIXED_BYTES_FIELD_HEADER_BYTE_COUNT,
            payload_byte_count);
        ++identity_chunk_index;
    }

    const std::uint32_t transform_chunk_count = CalculateTransformChunkCount(transform_record_count);
    std::uint32_t transform_chunk_index = 0U;
    while (transform_chunk_index < transform_chunk_count) {
        const std::uint32_t chunk_record_count = GetTransformChunkRecordCount(
            transform_record_count,
            transform_chunk_index);
        const std::uint32_t payload_byte_count =
            RequiredTransformChunkPayloadByteCount(chunk_record_count);
        result += RequiredChunkWriteByteCount(
            WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_FIXED_BYTES_FIELD_HEADER_BYTE_COUNT,
            payload_byte_count);
        ++transform_chunk_index;
    }

    return result;
}

std::uint32_t RequiredAssemblyWriteByteCount(
    std::uint32_t attachment_record_count,
    std::uint32_t binding_record_count) {
    std::uint32_t result = WORLD_SCENE_ASSEMBLY_MANIFEST_METADATA_RECORD_BYTE_COUNT;

    const std::uint32_t attachment_chunk_count = CalculateAttachmentChunkCount(attachment_record_count);
    std::uint32_t attachment_chunk_index = 0U;
    while (attachment_chunk_index < attachment_chunk_count) {
        const std::uint32_t chunk_record_count = GetAttachmentChunkRecordCount(
            attachment_record_count,
            attachment_chunk_index);
        const std::uint32_t payload_byte_count =
            RequiredAttachmentChunkPayloadByteCount(chunk_record_count);
        result += RequiredChunkWriteByteCount(
            WORLD_SCENE_ASSEMBLY_MANIFEST_FIXED_BYTES_FIELD_HEADER_BYTE_COUNT,
            payload_byte_count);
        ++attachment_chunk_index;
    }

    const std::uint32_t binding_chunk_count = CalculateBindingChunkCount(binding_record_count);
    std::uint32_t binding_chunk_index = 0U;
    while (binding_chunk_index < binding_chunk_count) {
        const std::uint32_t chunk_record_count = GetBindingChunkRecordCount(
            binding_record_count,
            binding_chunk_index);
        const std::uint32_t payload_byte_count =
            RequiredBindingChunkPayloadByteCount(chunk_record_count);
        result += RequiredChunkWriteByteCount(
            WORLD_SCENE_ASSEMBLY_MANIFEST_FIXED_BYTES_FIELD_HEADER_BYTE_COUNT,
            payload_byte_count);
        ++binding_chunk_index;
    }

    return result;
}

std::uint32_t RequiredWriteByteCount(
    std::uint32_t identity_record_count,
    std::uint32_t transform_record_count,
    std::uint32_t attachment_record_count,
    std::uint32_t binding_record_count) {
    return RequiredObjectTransformWriteByteCount(identity_record_count, transform_record_count) +
        RequiredAssemblyWriteByteCount(attachment_record_count, binding_record_count);
}

std::uint32_t RequiredWriteRecordCount(
    std::uint32_t identity_record_count,
    std::uint32_t transform_record_count,
    std::uint32_t attachment_record_count,
    std::uint32_t binding_record_count) {
    return 2U +
        CalculateIdentityChunkCount(identity_record_count) +
        CalculateTransformChunkCount(transform_record_count) +
        CalculateAttachmentChunkCount(attachment_record_count) +
        CalculateBindingChunkCount(binding_record_count);
}

std::uint32_t RequiredWriteFieldCount(
    std::uint32_t identity_record_count,
    std::uint32_t transform_record_count,
    std::uint32_t attachment_record_count,
    std::uint32_t binding_record_count) {
    return WORLD_SCENE_OBJECT_TRANSFORM_MANIFEST_METADATA_FIELD_COUNT +
        WORLD_SCENE_ASSEMBLY_MANIFEST_METADATA_FIELD_COUNT +
        CalculateIdentityChunkCount(identity_record_count) +
        CalculateTransformChunkCount(transform_record_count) +
        CalculateAttachmentChunkCount(attachment_record_count) +
        CalculateBindingChunkCount(binding_record_count);
}

bool IsEmptyWriterSnapshot(const SerializeSnapshot &snapshot) {
    if (snapshot.committed_byte_count != 0U) {
        return false;
    }

    if (snapshot.record_count != 0U) {
        return false;
    }

    return snapshot.field_count == 0U;
}

SerializeStatus ValidateOpenedWriterBudget(
    const SerializeWriter &writer,
    std::uint32_t identity_record_count,
    std::uint32_t transform_record_count,
    std::uint32_t attachment_record_count,
    std::uint32_t binding_record_count) {
    const SerializeSnapshot writer_snapshot = writer.Snapshot();
    if (writer_snapshot.major_version != yuengine::serialize::STREAM_MAJOR_VERSION) {
        return SerializeStatus::InvalidHeader;
    }

    if (writer_snapshot.minor_version != yuengine::serialize::STREAM_MINOR_VERSION) {
        return SerializeStatus::InvalidHeader;
    }

    if (writer_snapshot.committed_byte_count < yuengine::serialize::STREAM_HEADER_BYTE_COUNT) {
        return SerializeStatus::InvalidHeader;
    }

    if (writer_snapshot.committed_byte_count > yuengine::serialize::MAX_STREAM_BYTE_COUNT) {
        return SerializeStatus::BufferTooSmall;
    }

    const std::uint32_t required_record_count = RequiredWriteRecordCount(
        identity_record_count,
        transform_record_count,
        attachment_record_count,
        binding_record_count);
    if (writer_snapshot.record_count > yuengine::serialize::MAX_RECORDS_PER_STREAM) {
        return SerializeStatus::RecordCapacityExceeded;
    }

    const std::uint32_t remaining_record_count =
        yuengine::serialize::MAX_RECORDS_PER_STREAM - writer_snapshot.record_count;
    if (required_record_count > remaining_record_count) {
        return SerializeStatus::RecordCapacityExceeded;
    }

    const std::uint32_t required_field_count = RequiredWriteFieldCount(
        identity_record_count,
        transform_record_count,
        attachment_record_count,
        binding_record_count);
    if (writer_snapshot.field_count > yuengine::serialize::MAX_FIELDS_PER_STREAM) {
        return SerializeStatus::FieldCapacityExceeded;
    }

    const std::uint32_t remaining_field_count =
        yuengine::serialize::MAX_FIELDS_PER_STREAM - writer_snapshot.field_count;
    if (required_field_count > remaining_field_count) {
        return SerializeStatus::FieldCapacityExceeded;
    }

    const std::uint32_t required_byte_count = RequiredWriteByteCount(
        identity_record_count,
        transform_record_count,
        attachment_record_count,
        binding_record_count);
    if (required_byte_count > writer.GetRemainingByteCapacity()) {
        return SerializeStatus::BufferTooSmall;
    }

    if (!writer.CanCommitByteCount(required_byte_count)) {
        return SerializeStatus::BufferTooSmall;
    }

    return SerializeStatus::Success;
}

SerializeStatus ValidateEmptyWriterBudget(
    const SerializeWriter &writer,
    std::uint32_t identity_record_count,
    std::uint32_t transform_record_count,
    std::uint32_t attachment_record_count,
    std::uint32_t binding_record_count) {
    const std::uint32_t required_byte_count =
        yuengine::serialize::STREAM_HEADER_BYTE_COUNT +
        RequiredWriteByteCount(
            identity_record_count,
            transform_record_count,
            attachment_record_count,
            binding_record_count);
    if (required_byte_count > writer.GetByteCapacity()) {
        return SerializeStatus::BufferTooSmall;
    }

    if (!writer.CanCommitByteCount(required_byte_count)) {
        return SerializeStatus::BufferTooSmall;
    }

    return SerializeStatus::Success;
}

SerializeStatus ValidateWriteBudget(
    const SerializeWriter &writer,
    std::uint32_t identity_record_count,
    std::uint32_t transform_record_count,
    std::uint32_t attachment_record_count,
    std::uint32_t binding_record_count) {
    const SerializeSnapshot writer_snapshot = writer.Snapshot();
    if (IsEmptyWriterSnapshot(writer_snapshot)) {
        return ValidateEmptyWriterBudget(
            writer,
            identity_record_count,
            transform_record_count,
            attachment_record_count,
            binding_record_count);
    }

    return ValidateOpenedWriterBudget(
        writer,
        identity_record_count,
        transform_record_count,
        attachment_record_count,
        binding_record_count);
}

WorldSceneRecordValueStreamStatus MapObjectTransformStatus(
    WorldSceneObjectTransformManifestStreamStatus status) {
    switch (status) {
        case WorldSceneObjectTransformManifestStreamStatus::Success:
            return WorldSceneRecordValueStreamStatus::Success;
        case WorldSceneObjectTransformManifestStreamStatus::InvalidWriter:
            return WorldSceneRecordValueStreamStatus::InvalidWriter;
        case WorldSceneObjectTransformManifestStreamStatus::InvalidReader:
            return WorldSceneRecordValueStreamStatus::InvalidReader;
        case WorldSceneObjectTransformManifestStreamStatus::InvalidIdentityInput:
            return WorldSceneRecordValueStreamStatus::InvalidIdentityInput;
        case WorldSceneObjectTransformManifestStreamStatus::InvalidTransformInput:
            return WorldSceneRecordValueStreamStatus::InvalidTransformInput;
        case WorldSceneObjectTransformManifestStreamStatus::InvalidIdentityOutput:
            return WorldSceneRecordValueStreamStatus::InvalidIdentityOutput;
        case WorldSceneObjectTransformManifestStreamStatus::InvalidTransformOutput:
            return WorldSceneRecordValueStreamStatus::InvalidTransformOutput;
        case WorldSceneObjectTransformManifestStreamStatus::InvalidIdentityOutputCount:
            return WorldSceneRecordValueStreamStatus::InvalidIdentityOutputCount;
        case WorldSceneObjectTransformManifestStreamStatus::InvalidTransformOutputCount:
            return WorldSceneRecordValueStreamStatus::InvalidTransformOutputCount;
        case WorldSceneObjectTransformManifestStreamStatus::InputCountExceeded:
            return WorldSceneRecordValueStreamStatus::InputCountExceeded;
        case WorldSceneObjectTransformManifestStreamStatus::OutputCapacityExceeded:
            return WorldSceneRecordValueStreamStatus::OutputCapacityExceeded;
        case WorldSceneObjectTransformManifestStreamStatus::UnsupportedVersion:
            return WorldSceneRecordValueStreamStatus::UnsupportedVersion;
        case WorldSceneObjectTransformManifestStreamStatus::InvalidWorldObjectId:
            return WorldSceneRecordValueStreamStatus::InvalidWorldObjectId;
        case WorldSceneObjectTransformManifestStreamStatus::InvalidObjectHandle:
            return WorldSceneRecordValueStreamStatus::InvalidObjectHandle;
        case WorldSceneObjectTransformManifestStreamStatus::DuplicateIdentityWorldObjectId:
            return WorldSceneRecordValueStreamStatus::DuplicateIdentityWorldObjectId;
        case WorldSceneObjectTransformManifestStreamStatus::DuplicateIdentityObjectHandle:
            return WorldSceneRecordValueStreamStatus::DuplicateIdentityObjectHandle;
        case WorldSceneObjectTransformManifestStreamStatus::DuplicateTransformWorldObjectId:
            return WorldSceneRecordValueStreamStatus::DuplicateTransformWorldObjectId;
        case WorldSceneObjectTransformManifestStreamStatus::MissingIdentityForTransform:
            return WorldSceneRecordValueStreamStatus::MissingIdentityForTransform;
        case WorldSceneObjectTransformManifestStreamStatus::SerializeFailure:
            return WorldSceneRecordValueStreamStatus::SerializeFailure;
        default:
            break;
    }

    return WorldSceneRecordValueStreamStatus::ChildStreamFailure;
}

WorldSceneRecordValueStreamStatus MapAssemblyStatus(
    WorldSceneAssemblyManifestStreamStatus status) {
    switch (status) {
        case WorldSceneAssemblyManifestStreamStatus::Success:
            return WorldSceneRecordValueStreamStatus::Success;
        case WorldSceneAssemblyManifestStreamStatus::InvalidWriter:
            return WorldSceneRecordValueStreamStatus::InvalidWriter;
        case WorldSceneAssemblyManifestStreamStatus::InvalidReader:
            return WorldSceneRecordValueStreamStatus::InvalidReader;
        case WorldSceneAssemblyManifestStreamStatus::InvalidAttachmentInput:
            return WorldSceneRecordValueStreamStatus::InvalidAttachmentInput;
        case WorldSceneAssemblyManifestStreamStatus::InvalidBindingInput:
            return WorldSceneRecordValueStreamStatus::InvalidBindingInput;
        case WorldSceneAssemblyManifestStreamStatus::InvalidAttachmentOutput:
            return WorldSceneRecordValueStreamStatus::InvalidAttachmentOutput;
        case WorldSceneAssemblyManifestStreamStatus::InvalidBindingOutput:
            return WorldSceneRecordValueStreamStatus::InvalidBindingOutput;
        case WorldSceneAssemblyManifestStreamStatus::InvalidAttachmentOutputCount:
            return WorldSceneRecordValueStreamStatus::InvalidAttachmentOutputCount;
        case WorldSceneAssemblyManifestStreamStatus::InvalidBindingOutputCount:
            return WorldSceneRecordValueStreamStatus::InvalidBindingOutputCount;
        case WorldSceneAssemblyManifestStreamStatus::InputCountExceeded:
            return WorldSceneRecordValueStreamStatus::InputCountExceeded;
        case WorldSceneAssemblyManifestStreamStatus::OutputCapacityExceeded:
            return WorldSceneRecordValueStreamStatus::OutputCapacityExceeded;
        case WorldSceneAssemblyManifestStreamStatus::UnsupportedVersion:
            return WorldSceneRecordValueStreamStatus::UnsupportedVersion;
        case WorldSceneAssemblyManifestStreamStatus::InvalidWorldObjectId:
            return WorldSceneRecordValueStreamStatus::InvalidWorldObjectId;
        case WorldSceneAssemblyManifestStreamStatus::InvalidComponentTypeId:
            return WorldSceneRecordValueStreamStatus::InvalidComponentTypeId;
        case WorldSceneAssemblyManifestStreamStatus::InvalidComponentSlotId:
            return WorldSceneRecordValueStreamStatus::InvalidComponentSlotId;
        case WorldSceneAssemblyManifestStreamStatus::InvalidResourceHandle:
            return WorldSceneRecordValueStreamStatus::InvalidResourceHandle;
        case WorldSceneAssemblyManifestStreamStatus::InvalidResourceTypeId:
            return WorldSceneRecordValueStreamStatus::InvalidResourceTypeId;
        case WorldSceneAssemblyManifestStreamStatus::MissingAttachment:
            return WorldSceneRecordValueStreamStatus::MissingAttachment;
        case WorldSceneAssemblyManifestStreamStatus::DuplicateAttachment:
            return WorldSceneRecordValueStreamStatus::DuplicateAttachment;
        case WorldSceneAssemblyManifestStreamStatus::DuplicateBinding:
            return WorldSceneRecordValueStreamStatus::DuplicateBinding;
        case WorldSceneAssemblyManifestStreamStatus::SerializeFailure:
            return WorldSceneRecordValueStreamStatus::SerializeFailure;
        default:
            break;
    }

    return WorldSceneRecordValueStreamStatus::ChildStreamFailure;
}
}

WorldSceneRecordValueStreamBridge::WorldSceneRecordValueStreamBridge(
    WorldSceneRecordValueStreamDesc desc)
    : identity_capacity_(ClampCapacity(desc.identity_capacity)),
      transform_capacity_(ClampCapacity(desc.transform_capacity)),
      attachment_capacity_(ClampCapacity(desc.attachment_capacity)),
      binding_capacity_(ClampCapacity(desc.binding_capacity)),
      snapshot_{
          ClampCapacity(desc.identity_capacity),
          ClampCapacity(desc.transform_capacity),
          ClampCapacity(desc.attachment_capacity),
          ClampCapacity(desc.binding_capacity),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          SerializeStatus::Success,
          WorldSceneRecordValueStreamStatus::Success} {
    if (desc.identity_capacity == 0U) {
        snapshot_.last_status = WorldSceneRecordValueStreamStatus::InvalidBridgeCapacity;
        return;
    }

    if (desc.transform_capacity == 0U) {
        snapshot_.last_status = WorldSceneRecordValueStreamStatus::InvalidBridgeCapacity;
        return;
    }

    if (desc.attachment_capacity == 0U) {
        snapshot_.last_status = WorldSceneRecordValueStreamStatus::InvalidBridgeCapacity;
        return;
    }

    if (desc.binding_capacity == 0U) {
        snapshot_.last_status = WorldSceneRecordValueStreamStatus::InvalidBridgeCapacity;
        return;
    }
}

WorldSceneRecordValueStreamResult WorldSceneRecordValueStreamBridge::WriteSceneRecords(
    yuengine::serialize::SerializeWriter *writer,
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
    std::uint32_t input_transform_count,
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t input_attachment_count,
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t input_binding_count) {
    const WorldSceneRecordValueStreamStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldSceneRecordValueStreamStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (writer == nullptr) {
        return RecordFailure(WorldSceneRecordValueStreamStatus::InvalidWriter);
    }

    WorldSceneRecordValueStreamStatus status = ValidateWriteInputs(
        input_identities,
        input_identity_count,
        input_transforms,
        input_transform_count,
        input_attachments,
        input_attachment_count,
        input_bindings,
        input_binding_count);
    if (status != WorldSceneRecordValueStreamStatus::Success) {
        return RecordFailure(status);
    }

    status = ValidateIdentityRecords(input_identities, input_identity_count);
    if (status != WorldSceneRecordValueStreamStatus::Success) {
        return RecordRejectedFailure(status);
    }

    status = ValidateTransformRecords(
        input_identities,
        input_identity_count,
        input_transforms,
        input_transform_count);
    if (status != WorldSceneRecordValueStreamStatus::Success) {
        return RecordRejectedFailure(status);
    }

    status = ValidateAttachmentRecords(
        input_identities,
        input_identity_count,
        input_attachments,
        input_attachment_count);
    if (status != WorldSceneRecordValueStreamStatus::Success) {
        return RecordRejectedFailure(status);
    }

    status = ValidateBindingRecords(
        input_attachments,
        input_attachment_count,
        input_bindings,
        input_binding_count);
    if (status != WorldSceneRecordValueStreamStatus::Success) {
        return RecordRejectedFailure(status);
    }

    const SerializeStatus serialize_status = ValidateWriteBudget(
        *writer,
        input_identity_count,
        input_transform_count,
        input_attachment_count,
        input_binding_count);
    if (serialize_status != SerializeStatus::Success) {
        return RecordSerializeFailure(serialize_status);
    }

    WorldSceneObjectTransformManifestStreamDesc object_transform_desc{};
    object_transform_desc.identity_capacity = identity_capacity_;
    object_transform_desc.transform_capacity = transform_capacity_;
    WorldSceneObjectTransformManifestStreamBridge object_transform_bridge(object_transform_desc);
    const WorldSceneObjectTransformManifestStreamResult object_transform_result =
        object_transform_bridge.WriteManifest(
            writer,
            input_identities,
            input_identity_count,
            input_transforms,
            input_transform_count);
    if (!object_transform_result.Succeeded()) {
        if (object_transform_result.status == WorldSceneObjectTransformManifestStreamStatus::SerializeFailure) {
            return RecordSerializeFailure(object_transform_result.serialize_status);
        }

        return RecordFailure(MapObjectTransformStatus(object_transform_result.status));
    }

    WorldSceneAssemblyManifestStreamDesc assembly_desc{};
    assembly_desc.attachment_capacity = attachment_capacity_;
    assembly_desc.binding_capacity = binding_capacity_;
    WorldSceneAssemblyManifestStreamBridge assembly_bridge(assembly_desc);
    const WorldSceneAssemblyManifestStreamResult assembly_result = assembly_bridge.WriteManifest(
        writer,
        input_attachments,
        input_attachment_count,
        input_bindings,
        input_binding_count);
    if (!assembly_result.Succeeded()) {
        if (assembly_result.status == WorldSceneAssemblyManifestStreamStatus::SerializeFailure) {
            return RecordSerializeFailure(assembly_result.serialize_status);
        }

        return RecordFailure(MapAssemblyStatus(assembly_result.status));
    }

    WorldSceneRecordValueStreamState state{};
    state.identity_record_count = input_identity_count;
    state.transform_record_count = input_transform_count;
    state.attachment_record_count = input_attachment_count;
    state.binding_record_count = input_binding_count;
    state.committed_byte_count = writer->Snapshot().committed_byte_count;
    return RecordWriteSuccess(state);
}

WorldSceneRecordValueStreamResult WorldSceneRecordValueStreamBridge::ReadSceneRecords(
    yuengine::serialize::SerializeReader *reader,
    WorldSceneObjectTransformRestoreIdentityRecord *output_identities,
    std::uint32_t output_identity_capacity,
    std::uint32_t *out_identity_count,
    WorldSceneObjectTransformRestoreTransformRecord *output_transforms,
    std::uint32_t output_transform_capacity,
    std::uint32_t *out_transform_count,
    WorldComponentAttachmentSnapshotRecord *output_attachments,
    std::uint32_t output_attachment_capacity,
    std::uint32_t *out_attachment_count,
    WorldComponentResourceBindingSnapshotRecord *output_bindings,
    std::uint32_t output_binding_capacity,
    std::uint32_t *out_binding_count) {
    const WorldSceneRecordValueStreamStatus capacity_status = ValidateBridgeCapacity();
    if (capacity_status != WorldSceneRecordValueStreamStatus::Success) {
        return RecordFailure(capacity_status);
    }

    if (reader == nullptr) {
        return RecordFailure(WorldSceneRecordValueStreamStatus::InvalidReader);
    }

    const WorldSceneRecordValueStreamStatus output_status = ValidateReadOutputs(
        output_identities,
        out_identity_count,
        output_transforms,
        out_transform_count,
        output_attachments,
        out_attachment_count,
        output_bindings,
        out_binding_count);
    if (output_status != WorldSceneRecordValueStreamStatus::Success) {
        return RecordFailure(output_status);
    }

    std::array<WorldSceneObjectTransformRestoreIdentityRecord, MAX_WORLD_OBJECT_COUNT> decoded_identities{};
    std::array<WorldSceneObjectTransformRestoreTransformRecord, MAX_WORLD_OBJECT_COUNT> decoded_transforms{};
    std::uint32_t decoded_identity_count = 0U;
    std::uint32_t decoded_transform_count = 0U;

    WorldSceneObjectTransformManifestStreamDesc object_transform_desc{};
    object_transform_desc.identity_capacity = identity_capacity_;
    object_transform_desc.transform_capacity = transform_capacity_;
    WorldSceneObjectTransformManifestStreamBridge object_transform_bridge(object_transform_desc);
    const WorldSceneObjectTransformManifestStreamResult object_transform_result =
        object_transform_bridge.ReadManifest(
            reader,
            decoded_identities.data(),
            static_cast<std::uint32_t>(decoded_identities.size()),
            &decoded_identity_count,
            decoded_transforms.data(),
            static_cast<std::uint32_t>(decoded_transforms.size()),
            &decoded_transform_count);
    if (!object_transform_result.Succeeded()) {
        if (object_transform_result.status == WorldSceneObjectTransformManifestStreamStatus::SerializeFailure) {
            return RecordSerializeFailure(object_transform_result.serialize_status);
        }

        return RecordFailure(MapObjectTransformStatus(object_transform_result.status));
    }

    std::array<WorldComponentAttachmentSnapshotRecord, MAX_WORLD_OBJECT_COUNT> decoded_attachments{};
    std::array<WorldComponentResourceBindingSnapshotRecord, MAX_WORLD_OBJECT_COUNT> decoded_bindings{};
    std::uint32_t decoded_attachment_count = 0U;
    std::uint32_t decoded_binding_count = 0U;

    WorldSceneAssemblyManifestStreamDesc assembly_desc{};
    assembly_desc.attachment_capacity = attachment_capacity_;
    assembly_desc.binding_capacity = binding_capacity_;
    WorldSceneAssemblyManifestStreamBridge assembly_bridge(assembly_desc);
    const WorldSceneAssemblyManifestStreamResult assembly_result = assembly_bridge.ReadManifest(
        reader,
        decoded_attachments.data(),
        static_cast<std::uint32_t>(decoded_attachments.size()),
        &decoded_attachment_count,
        decoded_bindings.data(),
        static_cast<std::uint32_t>(decoded_bindings.size()),
        &decoded_binding_count);
    if (!assembly_result.Succeeded()) {
        if (assembly_result.status == WorldSceneAssemblyManifestStreamStatus::SerializeFailure) {
            return RecordSerializeFailure(assembly_result.serialize_status);
        }

        return RecordFailure(MapAssemblyStatus(assembly_result.status));
    }

    WorldSceneRecordValueStreamStatus status = ValidateAttachmentRecords(
        decoded_identities.data(),
        decoded_identity_count,
        decoded_attachments.data(),
        decoded_attachment_count);
    if (status != WorldSceneRecordValueStreamStatus::Success) {
        return RecordRejectedFailure(status);
    }

    status = ValidateBindingRecords(
        decoded_attachments.data(),
        decoded_attachment_count,
        decoded_bindings.data(),
        decoded_binding_count);
    if (status != WorldSceneRecordValueStreamStatus::Success) {
        return RecordRejectedFailure(status);
    }

    if (decoded_identity_count > output_identity_capacity) {
        return RecordFailure(WorldSceneRecordValueStreamStatus::OutputCapacityExceeded);
    }

    if (decoded_transform_count > output_transform_capacity) {
        return RecordFailure(WorldSceneRecordValueStreamStatus::OutputCapacityExceeded);
    }

    if (decoded_attachment_count > output_attachment_capacity) {
        return RecordFailure(WorldSceneRecordValueStreamStatus::OutputCapacityExceeded);
    }

    if (decoded_binding_count > output_binding_capacity) {
        return RecordFailure(WorldSceneRecordValueStreamStatus::OutputCapacityExceeded);
    }

    CopyOutputs(
        decoded_identities.data(),
        decoded_identity_count,
        output_identities,
        out_identity_count,
        decoded_transforms.data(),
        decoded_transform_count,
        output_transforms,
        out_transform_count,
        decoded_attachments.data(),
        decoded_attachment_count,
        output_attachments,
        out_attachment_count,
        decoded_bindings.data(),
        decoded_binding_count,
        output_bindings,
        out_binding_count);

    WorldSceneRecordValueStreamState state{};
    state.identity_record_count = decoded_identity_count;
    state.transform_record_count = decoded_transform_count;
    state.attachment_record_count = decoded_attachment_count;
    state.binding_record_count = decoded_binding_count;
    state.committed_byte_count = reader->Snapshot().committed_byte_count;
    return RecordReadSuccess(state);
}

WorldSceneRecordValueStreamSnapshot WorldSceneRecordValueStreamBridge::Snapshot() const {
    return snapshot_;
}

WorldSceneRecordValueStreamResult WorldSceneRecordValueStreamBridge::RecordFailure(
    WorldSceneRecordValueStreamStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    snapshot_.last_status = status;
    return WorldSceneRecordValueStreamResult::Failure(status);
}

WorldSceneRecordValueStreamResult WorldSceneRecordValueStreamBridge::RecordRejectedFailure(
    WorldSceneRecordValueStreamStatus status) {
    ++snapshot_.rejected_record_count;
    return RecordFailure(status);
}

WorldSceneRecordValueStreamResult WorldSceneRecordValueStreamBridge::RecordSerializeFailure(
    SerializeStatus status) {
    ++snapshot_.failed_operation_count;
    snapshot_.last_serialize_status = status;
    snapshot_.last_status = WorldSceneRecordValueStreamStatus::SerializeFailure;
    return WorldSceneRecordValueStreamResult::Failure(
        WorldSceneRecordValueStreamStatus::SerializeFailure,
        status);
}

WorldSceneRecordValueStreamResult WorldSceneRecordValueStreamBridge::RecordWriteSuccess(
    const WorldSceneRecordValueStreamState &state) {
    ++snapshot_.write_count;
    snapshot_.written_identity_count += state.identity_record_count;
    snapshot_.written_transform_count += state.transform_record_count;
    snapshot_.written_attachment_count += state.attachment_record_count;
    snapshot_.written_binding_count += state.binding_record_count;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    snapshot_.last_status = WorldSceneRecordValueStreamStatus::Success;
    return WorldSceneRecordValueStreamResult::Success(state);
}

WorldSceneRecordValueStreamResult WorldSceneRecordValueStreamBridge::RecordReadSuccess(
    const WorldSceneRecordValueStreamState &state) {
    ++snapshot_.read_count;
    snapshot_.read_identity_count += state.identity_record_count;
    snapshot_.read_transform_count += state.transform_record_count;
    snapshot_.read_attachment_count += state.attachment_record_count;
    snapshot_.read_binding_count += state.binding_record_count;
    snapshot_.last_serialize_status = SerializeStatus::Success;
    snapshot_.last_status = WorldSceneRecordValueStreamStatus::Success;
    return WorldSceneRecordValueStreamResult::Success(state);
}

WorldSceneRecordValueStreamStatus WorldSceneRecordValueStreamBridge::ValidateBridgeCapacity() const {
    if (identity_capacity_ == 0U) {
        return WorldSceneRecordValueStreamStatus::InvalidBridgeCapacity;
    }

    if (transform_capacity_ == 0U) {
        return WorldSceneRecordValueStreamStatus::InvalidBridgeCapacity;
    }

    if (attachment_capacity_ == 0U) {
        return WorldSceneRecordValueStreamStatus::InvalidBridgeCapacity;
    }

    if (binding_capacity_ == 0U) {
        return WorldSceneRecordValueStreamStatus::InvalidBridgeCapacity;
    }

    return WorldSceneRecordValueStreamStatus::Success;
}

WorldSceneRecordValueStreamStatus WorldSceneRecordValueStreamBridge::ValidateWriteInputs(
    const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
    std::uint32_t input_identity_count,
    const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
    std::uint32_t input_transform_count,
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t input_attachment_count,
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t input_binding_count) const {
    if (input_identity_count > identity_capacity_) {
        return WorldSceneRecordValueStreamStatus::InputCountExceeded;
    }

    if (input_transform_count > transform_capacity_) {
        return WorldSceneRecordValueStreamStatus::InputCountExceeded;
    }

    if (input_attachment_count > attachment_capacity_) {
        return WorldSceneRecordValueStreamStatus::InputCountExceeded;
    }

    if (input_binding_count > binding_capacity_) {
        return WorldSceneRecordValueStreamStatus::InputCountExceeded;
    }

    if (input_identity_count > 0U && input_identities == nullptr) {
        return WorldSceneRecordValueStreamStatus::InvalidIdentityInput;
    }

    if (input_transform_count > 0U && input_transforms == nullptr) {
        return WorldSceneRecordValueStreamStatus::InvalidTransformInput;
    }

    if (input_attachment_count > 0U && input_attachments == nullptr) {
        return WorldSceneRecordValueStreamStatus::InvalidAttachmentInput;
    }

    if (input_binding_count > 0U && input_bindings == nullptr) {
        return WorldSceneRecordValueStreamStatus::InvalidBindingInput;
    }

    return WorldSceneRecordValueStreamStatus::Success;
}

WorldSceneRecordValueStreamStatus WorldSceneRecordValueStreamBridge::ValidateReadOutputs(
    WorldSceneObjectTransformRestoreIdentityRecord *output_identities,
    std::uint32_t *out_identity_count,
    WorldSceneObjectTransformRestoreTransformRecord *output_transforms,
    std::uint32_t *out_transform_count,
    WorldComponentAttachmentSnapshotRecord *output_attachments,
    std::uint32_t *out_attachment_count,
    WorldComponentResourceBindingSnapshotRecord *output_bindings,
    std::uint32_t *out_binding_count) const {
    if (output_identities == nullptr) {
        return WorldSceneRecordValueStreamStatus::InvalidIdentityOutput;
    }

    if (out_identity_count == nullptr) {
        return WorldSceneRecordValueStreamStatus::InvalidIdentityOutputCount;
    }

    if (output_transforms == nullptr) {
        return WorldSceneRecordValueStreamStatus::InvalidTransformOutput;
    }

    if (out_transform_count == nullptr) {
        return WorldSceneRecordValueStreamStatus::InvalidTransformOutputCount;
    }

    if (output_attachments == nullptr) {
        return WorldSceneRecordValueStreamStatus::InvalidAttachmentOutput;
    }

    if (out_attachment_count == nullptr) {
        return WorldSceneRecordValueStreamStatus::InvalidAttachmentOutputCount;
    }

    if (output_bindings == nullptr) {
        return WorldSceneRecordValueStreamStatus::InvalidBindingOutput;
    }

    if (out_binding_count == nullptr) {
        return WorldSceneRecordValueStreamStatus::InvalidBindingOutputCount;
    }

    return WorldSceneRecordValueStreamStatus::Success;
}

WorldSceneRecordValueStreamStatus WorldSceneRecordValueStreamBridge::ValidateIdentityRecords(
    const WorldSceneObjectTransformRestoreIdentityRecord *records,
    std::uint32_t record_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < record_count) {
        const WorldSceneObjectTransformRestoreIdentityRecord &record = records[record_index];
        if (!record.world_object_id.IsValid()) {
            return WorldSceneRecordValueStreamStatus::InvalidWorldObjectId;
        }

        if (!record.object_handle.IsValid()) {
            return WorldSceneRecordValueStreamStatus::InvalidObjectHandle;
        }

        if (HasDuplicateIdentityWorldObjectId(records, record_index)) {
            return WorldSceneRecordValueStreamStatus::DuplicateIdentityWorldObjectId;
        }

        if (HasDuplicateIdentityObjectHandle(records, record_index)) {
            return WorldSceneRecordValueStreamStatus::DuplicateIdentityObjectHandle;
        }

        ++record_index;
    }

    return WorldSceneRecordValueStreamStatus::Success;
}

WorldSceneRecordValueStreamStatus WorldSceneRecordValueStreamBridge::ValidateTransformRecords(
    const WorldSceneObjectTransformRestoreIdentityRecord *identity_records,
    std::uint32_t identity_record_count,
    const WorldSceneObjectTransformRestoreTransformRecord *transform_records,
    std::uint32_t transform_record_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < transform_record_count) {
        const WorldSceneObjectTransformRestoreTransformRecord &record = transform_records[record_index];
        if (!record.world_object_id.IsValid()) {
            return WorldSceneRecordValueStreamStatus::InvalidWorldObjectId;
        }

        if (HasDuplicateTransformWorldObjectId(transform_records, record_index)) {
            return WorldSceneRecordValueStreamStatus::DuplicateTransformWorldObjectId;
        }

        if (!HasIdentityRecord(identity_records, identity_record_count, record.world_object_id)) {
            return WorldSceneRecordValueStreamStatus::MissingIdentityForTransform;
        }

        ++record_index;
    }

    return WorldSceneRecordValueStreamStatus::Success;
}

WorldSceneRecordValueStreamStatus WorldSceneRecordValueStreamBridge::ValidateAttachmentRecords(
    const WorldSceneObjectTransformRestoreIdentityRecord *identity_records,
    std::uint32_t identity_record_count,
    const WorldComponentAttachmentSnapshotRecord *attachment_records,
    std::uint32_t attachment_record_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < attachment_record_count) {
        const WorldComponentAttachmentSnapshotRecord &record = attachment_records[record_index];
        if (!record.world_object_id.IsValid()) {
            return WorldSceneRecordValueStreamStatus::InvalidWorldObjectId;
        }

        if (!record.component_type_id.IsValid()) {
            return WorldSceneRecordValueStreamStatus::InvalidComponentTypeId;
        }

        if (!record.component_slot_id.IsValid()) {
            return WorldSceneRecordValueStreamStatus::InvalidComponentSlotId;
        }

        if (!HasIdentityRecord(identity_records, identity_record_count, record.world_object_id)) {
            return WorldSceneRecordValueStreamStatus::MissingIdentityForAttachment;
        }

        if (HasDuplicateAttachment(attachment_records, record_index)) {
            return WorldSceneRecordValueStreamStatus::DuplicateAttachment;
        }

        ++record_index;
    }

    return WorldSceneRecordValueStreamStatus::Success;
}

WorldSceneRecordValueStreamStatus WorldSceneRecordValueStreamBridge::ValidateBindingRecords(
    const WorldComponentAttachmentSnapshotRecord *attachment_records,
    std::uint32_t attachment_record_count,
    const WorldComponentResourceBindingSnapshotRecord *binding_records,
    std::uint32_t binding_record_count) const {
    std::uint32_t record_index = 0U;
    while (record_index < binding_record_count) {
        const WorldComponentResourceBindingSnapshotRecord &record = binding_records[record_index];
        if (!record.world_object_id.IsValid()) {
            return WorldSceneRecordValueStreamStatus::InvalidWorldObjectId;
        }

        if (!record.component_type_id.IsValid()) {
            return WorldSceneRecordValueStreamStatus::InvalidComponentTypeId;
        }

        if (!record.component_slot_id.IsValid()) {
            return WorldSceneRecordValueStreamStatus::InvalidComponentSlotId;
        }

        if (!record.resource_handle.IsValid()) {
            return WorldSceneRecordValueStreamStatus::InvalidResourceHandle;
        }

        if (!record.expected_resource_type.IsValid()) {
            return WorldSceneRecordValueStreamStatus::InvalidResourceTypeId;
        }

        if (!HasAttachmentTuple(attachment_records, attachment_record_count, record)) {
            return WorldSceneRecordValueStreamStatus::MissingAttachment;
        }

        if (HasDuplicateBinding(binding_records, record_index)) {
            return WorldSceneRecordValueStreamStatus::DuplicateBinding;
        }

        ++record_index;
    }

    return WorldSceneRecordValueStreamStatus::Success;
}

bool WorldSceneRecordValueStreamBridge::HasDuplicateIdentityWorldObjectId(
    const WorldSceneObjectTransformRestoreIdentityRecord *records,
    std::uint32_t record_index) const {
    const WorldSceneObjectTransformRestoreIdentityRecord &record = records[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldSceneObjectTransformRestoreIdentityRecord &compare_record = records[compare_index];
        if (compare_record.world_object_id.value == record.world_object_id.value) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneRecordValueStreamBridge::HasDuplicateIdentityObjectHandle(
    const WorldSceneObjectTransformRestoreIdentityRecord *records,
    std::uint32_t record_index) const {
    const WorldSceneObjectTransformRestoreIdentityRecord &record = records[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldSceneObjectTransformRestoreIdentityRecord &compare_record = records[compare_index];
        if (compare_record.object_handle.slot != record.object_handle.slot) {
            ++compare_index;
            continue;
        }

        if (compare_record.object_handle.generation == record.object_handle.generation) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneRecordValueStreamBridge::HasDuplicateTransformWorldObjectId(
    const WorldSceneObjectTransformRestoreTransformRecord *records,
    std::uint32_t record_index) const {
    const WorldSceneObjectTransformRestoreTransformRecord &record = records[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldSceneObjectTransformRestoreTransformRecord &compare_record = records[compare_index];
        if (compare_record.world_object_id.value == record.world_object_id.value) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneRecordValueStreamBridge::HasDuplicateAttachment(
    const WorldComponentAttachmentSnapshotRecord *records,
    std::uint32_t record_index) const {
    const WorldComponentAttachmentSnapshotRecord &record = records[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldComponentAttachmentSnapshotRecord &compare_record = records[compare_index];
        if (compare_record.world_object_id.value != record.world_object_id.value) {
            ++compare_index;
            continue;
        }

        if (compare_record.component_type_id.value == record.component_type_id.value) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneRecordValueStreamBridge::HasDuplicateBinding(
    const WorldComponentResourceBindingSnapshotRecord *records,
    std::uint32_t record_index) const {
    const WorldComponentResourceBindingSnapshotRecord &record = records[record_index];
    std::uint32_t compare_index = 0U;
    while (compare_index < record_index) {
        const WorldComponentResourceBindingSnapshotRecord &compare_record = records[compare_index];
        if (compare_record.world_object_id.value != record.world_object_id.value) {
            ++compare_index;
            continue;
        }

        if (compare_record.component_type_id.value != record.component_type_id.value) {
            ++compare_index;
            continue;
        }

        if (compare_record.component_slot_id.value == record.component_slot_id.value) {
            return true;
        }

        ++compare_index;
    }

    return false;
}

bool WorldSceneRecordValueStreamBridge::HasIdentityRecord(
    const WorldSceneObjectTransformRestoreIdentityRecord *records,
    std::uint32_t record_count,
    WorldObjectId world_object_id) const {
    std::uint32_t record_index = 0U;
    while (record_index < record_count) {
        const WorldSceneObjectTransformRestoreIdentityRecord &record = records[record_index];
        if (record.world_object_id.value == world_object_id.value) {
            return true;
        }

        ++record_index;
    }

    return false;
}

bool WorldSceneRecordValueStreamBridge::HasAttachmentTuple(
    const WorldComponentAttachmentSnapshotRecord *attachment_records,
    std::uint32_t attachment_record_count,
    const WorldComponentResourceBindingSnapshotRecord &binding_record) const {
    std::uint32_t attachment_index = 0U;
    while (attachment_index < attachment_record_count) {
        const WorldComponentAttachmentSnapshotRecord &attachment = attachment_records[attachment_index];
        if (attachment.world_object_id.value != binding_record.world_object_id.value) {
            ++attachment_index;
            continue;
        }

        if (attachment.component_type_id.value != binding_record.component_type_id.value) {
            ++attachment_index;
            continue;
        }

        if (attachment.component_slot_id.value == binding_record.component_slot_id.value) {
            return true;
        }

        ++attachment_index;
    }

    return false;
}

void WorldSceneRecordValueStreamBridge::CopyOutputs(
    const WorldSceneObjectTransformRestoreIdentityRecord *decoded_identities,
    std::uint32_t decoded_identity_count,
    WorldSceneObjectTransformRestoreIdentityRecord *output_identities,
    std::uint32_t *out_identity_count,
    const WorldSceneObjectTransformRestoreTransformRecord *decoded_transforms,
    std::uint32_t decoded_transform_count,
    WorldSceneObjectTransformRestoreTransformRecord *output_transforms,
    std::uint32_t *out_transform_count,
    const WorldComponentAttachmentSnapshotRecord *decoded_attachments,
    std::uint32_t decoded_attachment_count,
    WorldComponentAttachmentSnapshotRecord *output_attachments,
    std::uint32_t *out_attachment_count,
    const WorldComponentResourceBindingSnapshotRecord *decoded_bindings,
    std::uint32_t decoded_binding_count,
    WorldComponentResourceBindingSnapshotRecord *output_bindings,
    std::uint32_t *out_binding_count) const {
    std::uint32_t identity_index = 0U;
    while (identity_index < decoded_identity_count) {
        output_identities[identity_index] = decoded_identities[identity_index];
        ++identity_index;
    }

    std::uint32_t transform_index = 0U;
    while (transform_index < decoded_transform_count) {
        output_transforms[transform_index] = decoded_transforms[transform_index];
        ++transform_index;
    }

    std::uint32_t attachment_index = 0U;
    while (attachment_index < decoded_attachment_count) {
        output_attachments[attachment_index] = decoded_attachments[attachment_index];
        ++attachment_index;
    }

    std::uint32_t binding_index = 0U;
    while (binding_index < decoded_binding_count) {
        output_bindings[binding_index] = decoded_bindings[binding_index];
        ++binding_index;
    }

    *out_identity_count = decoded_identity_count;
    *out_transform_count = decoded_transform_count;
    *out_attachment_count = decoded_attachment_count;
    *out_binding_count = decoded_binding_count;
}
}
