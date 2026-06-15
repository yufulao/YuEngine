// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneAssemblyManifestStreamBridge.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldComponentAttachmentSnapshotRecord.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotRecord.h"
#include "YuEngine/World/WorldSceneAssemblyManifestStreamDesc.h"
#include "YuEngine/World/WorldSceneAssemblyManifestStreamResult.h"
#include "YuEngine/World/WorldSceneAssemblyManifestStreamSnapshot.h"
#include "YuEngine/World/WorldSceneAssemblyManifestStreamStatus.h"

namespace yuengine::serialize {
class SerializeReader;
class SerializeWriter;
}

namespace yuengine::world {
class WorldSceneAssemblyManifestStreamBridge final {
public:
    /**
     * @comment Constructs a world scene assembly manifest stream bridge.
     * @param desc Input bridge descriptor.
     */
    explicit WorldSceneAssemblyManifestStreamBridge(
        WorldSceneAssemblyManifestStreamDesc desc=WorldSceneAssemblyManifestStreamDesc{});

    /**
     * @comment Writes caller-owned sidecar records to a deterministic manifest stream.
     * @param writer Caller-owned serialize writer.
     * @param input_attachments Caller-owned attachment records.
     * @param input_attachment_count Input attachment record count.
     * @param input_bindings Caller-owned component-resource binding records.
     * @param input_binding_count Input binding record count.
     * @return Explicit operation result.
     */
    WorldSceneAssemblyManifestStreamResult WriteManifest(
        yuengine::serialize::SerializeWriter *writer,
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count,
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t input_binding_count);
    /**
     * @comment Reads caller-owned sidecar records from a deterministic manifest stream.
     * @param reader Caller-owned serialize reader.
     * @param output_attachments Caller-owned attachment output records.
     * @param output_attachment_capacity Attachment output capacity.
     * @param out_attachment_count Output attachment record count.
     * @param output_bindings Caller-owned binding output records.
     * @param output_binding_capacity Binding output capacity.
     * @param out_binding_count Output binding record count.
     * @return Explicit operation result.
     */
    WorldSceneAssemblyManifestStreamResult ReadManifest(
        yuengine::serialize::SerializeReader *reader,
        WorldComponentAttachmentSnapshotRecord *output_attachments,
        std::uint32_t output_attachment_capacity,
        std::uint32_t *out_attachment_count,
        WorldComponentResourceBindingSnapshotRecord *output_bindings,
        std::uint32_t output_binding_capacity,
        std::uint32_t *out_binding_count);
    /**
     * @comment Writes caller-owned sidecar records to a deterministic manifest stream.
     * @param writer Caller-owned serialize writer.
     * @param input_attachments Caller-owned attachment records.
     * @param input_attachment_count Input attachment record count.
     * @param input_bindings Caller-owned component-resource binding records.
     * @param input_binding_count Input binding record count.
     * @return Explicit operation result.
     */
    WorldSceneAssemblyManifestStreamResult WriteSnapshot(
        yuengine::serialize::SerializeWriter *writer,
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count,
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t input_binding_count);
    /**
     * @comment Reads caller-owned sidecar records from a deterministic manifest stream.
     * @param reader Caller-owned serialize reader.
     * @param output_attachments Caller-owned attachment output records.
     * @param output_attachment_capacity Attachment output capacity.
     * @param out_attachment_count Output attachment record count.
     * @param output_bindings Caller-owned binding output records.
     * @param output_binding_capacity Binding output capacity.
     * @param out_binding_count Output binding record count.
     * @return Explicit operation result.
     */
    WorldSceneAssemblyManifestStreamResult ReadSnapshot(
        yuengine::serialize::SerializeReader *reader,
        WorldComponentAttachmentSnapshotRecord *output_attachments,
        std::uint32_t output_attachment_capacity,
        std::uint32_t *out_attachment_count,
        WorldComponentResourceBindingSnapshotRecord *output_bindings,
        std::uint32_t output_binding_capacity,
        std::uint32_t *out_binding_count);
    /**
     * @comment Returns a snapshot of the current bridge state.
     * @return Snapshot value.
     */
    WorldSceneAssemblyManifestStreamSnapshot Snapshot() const;

private:
    WorldSceneAssemblyManifestStreamResult RecordFailure(
        WorldSceneAssemblyManifestStreamStatus status);
    WorldSceneAssemblyManifestStreamResult RecordRejectedFailure(
        WorldSceneAssemblyManifestStreamStatus status);
    WorldSceneAssemblyManifestStreamResult RecordSerializeFailure(
        yuengine::serialize::SerializeStatus status);
    WorldSceneAssemblyManifestStreamResult RecordWriteSuccess(
        const WorldSceneAssemblyManifestStreamState &state);
    WorldSceneAssemblyManifestStreamResult RecordReadSuccess(
        const WorldSceneAssemblyManifestStreamState &state);
    WorldSceneAssemblyManifestStreamStatus ValidateBridgeCapacity() const;
    WorldSceneAssemblyManifestStreamStatus ValidateWriteInputs(
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count,
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t input_binding_count) const;
    WorldSceneAssemblyManifestStreamStatus ValidateReadOutputs(
        WorldComponentAttachmentSnapshotRecord *output_attachments,
        std::uint32_t *out_attachment_count,
        WorldComponentResourceBindingSnapshotRecord *output_bindings,
        std::uint32_t *out_binding_count) const;
    WorldSceneAssemblyManifestStreamStatus ValidateAttachmentRecords(
        const WorldComponentAttachmentSnapshotRecord *records,
        std::uint32_t record_count) const;
    WorldSceneAssemblyManifestStreamStatus ValidateAttachmentRecord(
        const WorldComponentAttachmentSnapshotRecord *records,
        std::uint32_t record_index) const;
    WorldSceneAssemblyManifestStreamStatus ValidateBindingRecords(
        const WorldComponentAttachmentSnapshotRecord *attachment_records,
        std::uint32_t attachment_record_count,
        const WorldComponentResourceBindingSnapshotRecord *binding_records,
        std::uint32_t binding_record_count) const;
    WorldSceneAssemblyManifestStreamStatus ValidateBindingRecord(
        const WorldComponentAttachmentSnapshotRecord *attachment_records,
        std::uint32_t attachment_record_count,
        const WorldComponentResourceBindingSnapshotRecord *binding_records,
        std::uint32_t record_index) const;
    bool HasDuplicateAttachment(
        const WorldComponentAttachmentSnapshotRecord *records,
        std::uint32_t record_index) const;
    bool HasDuplicateBinding(
        const WorldComponentResourceBindingSnapshotRecord *records,
        std::uint32_t record_index) const;
    bool HasAttachmentTuple(
        const WorldComponentAttachmentSnapshotRecord *attachment_records,
        std::uint32_t attachment_record_count,
        const WorldComponentResourceBindingSnapshotRecord &binding_record) const;
    void CopyOutputs(
        const WorldComponentAttachmentSnapshotRecord *decoded_attachments,
        std::uint32_t decoded_attachment_count,
        WorldComponentAttachmentSnapshotRecord *output_attachments,
        std::uint32_t *out_attachment_count,
        const WorldComponentResourceBindingSnapshotRecord *decoded_bindings,
        std::uint32_t decoded_binding_count,
        WorldComponentResourceBindingSnapshotRecord *output_bindings,
        std::uint32_t *out_binding_count) const;

    std::uint32_t attachment_capacity_;
    std::uint32_t binding_capacity_;
    WorldSceneAssemblyManifestStreamSnapshot snapshot_;
};
}
