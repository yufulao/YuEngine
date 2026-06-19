// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneRecordValueStreamBridge.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldComponentAttachmentSnapshotRecord.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotRecord.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"
#include "YuEngine/World/WorldSceneRecordValueStreamDesc.h"
#include "YuEngine/World/WorldSceneRecordValueStreamResult.h"
#include "YuEngine/World/WorldSceneRecordValueStreamSnapshot.h"
#include "YuEngine/World/WorldSceneRecordValueStreamStatus.h"

namespace yuengine::serialize {
class SerializeReader;
class SerializeWriter;
}

namespace yuengine::world {
class WorldSceneRecordValueStreamBridge final {
public:
    /**
     * @comment 构造 scene record value stream bridge。
     * @param desc 输入 bridge descriptor。
     */
    explicit WorldSceneRecordValueStreamBridge(
        WorldSceneRecordValueStreamDesc desc=WorldSceneRecordValueStreamDesc{});

    /**
     * @comment 将调用方持有的 scene assembly records 写入同一个 deterministic value stream。
     * @param writer 调用方持有的 serialize writer。
     * @param input_identities 调用方持有的 identity records。
     * @param input_identity_count 输入 identity record count。
     * @param input_transforms 调用方持有的 transform records。
     * @param input_transform_count 输入 transform record count。
     * @param input_attachments 调用方持有的 attachment records。
     * @param input_attachment_count 输入 attachment record count。
     * @param input_bindings 调用方持有的 binding records。
     * @param input_binding_count 输入 binding record count。
     * @return 显式操作结果。
     */
    WorldSceneRecordValueStreamResult WriteSceneRecords(
        yuengine::serialize::SerializeWriter *writer,
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
        std::uint32_t input_transform_count,
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count,
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t input_binding_count);

    /**
     * @comment 从同一个 deterministic value stream 读取 caller-owned scene assembly records。
     * @param reader 调用方持有的 serialize reader。
     * @param output_identities 调用方持有的 identity output records。
     * @param output_identity_capacity Identity 输出容量。
     * @param out_identity_count 输出 identity record count。
     * @param output_transforms 调用方持有的 transform output records。
     * @param output_transform_capacity Transform 输出容量。
     * @param out_transform_count 输出 transform record count。
     * @param output_attachments 调用方持有的 attachment output records。
     * @param output_attachment_capacity Attachment 输出容量。
     * @param out_attachment_count 输出 attachment record count。
     * @param output_bindings 调用方持有的 binding output records。
     * @param output_binding_capacity Binding 输出容量。
     * @param out_binding_count 输出 binding record count。
     * @return 显式操作结果。
     */
    WorldSceneRecordValueStreamResult ReadSceneRecords(
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
        std::uint32_t *out_binding_count);

    /**
     * @comment 返回当前 bridge 状态快照。
     * @return 快照值。
     */
    WorldSceneRecordValueStreamSnapshot Snapshot() const;

private:
    WorldSceneRecordValueStreamResult RecordFailure(
        WorldSceneRecordValueStreamStatus status);
    WorldSceneRecordValueStreamResult RecordRejectedFailure(
        WorldSceneRecordValueStreamStatus status);
    WorldSceneRecordValueStreamResult RecordSerializeFailure(
        yuengine::serialize::SerializeStatus status);
    WorldSceneRecordValueStreamResult RecordWriteSuccess(
        const WorldSceneRecordValueStreamState &state);
    WorldSceneRecordValueStreamResult RecordReadSuccess(
        const WorldSceneRecordValueStreamState &state);
    WorldSceneRecordValueStreamStatus ValidateBridgeCapacity() const;
    WorldSceneRecordValueStreamStatus ValidateWriteInputs(
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
        std::uint32_t input_transform_count,
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count,
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t input_binding_count) const;
    WorldSceneRecordValueStreamStatus ValidateReadOutputs(
        WorldSceneObjectTransformRestoreIdentityRecord *output_identities,
        std::uint32_t *out_identity_count,
        WorldSceneObjectTransformRestoreTransformRecord *output_transforms,
        std::uint32_t *out_transform_count,
        WorldComponentAttachmentSnapshotRecord *output_attachments,
        std::uint32_t *out_attachment_count,
        WorldComponentResourceBindingSnapshotRecord *output_bindings,
        std::uint32_t *out_binding_count) const;
    WorldSceneRecordValueStreamStatus ValidateIdentityRecords(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_count) const;
    WorldSceneRecordValueStreamStatus ValidateTransformRecords(
        const WorldSceneObjectTransformRestoreIdentityRecord *identity_records,
        std::uint32_t identity_record_count,
        const WorldSceneObjectTransformRestoreTransformRecord *transform_records,
        std::uint32_t transform_record_count) const;
    WorldSceneRecordValueStreamStatus ValidateAttachmentRecords(
        const WorldSceneObjectTransformRestoreIdentityRecord *identity_records,
        std::uint32_t identity_record_count,
        const WorldComponentAttachmentSnapshotRecord *attachment_records,
        std::uint32_t attachment_record_count) const;
    WorldSceneRecordValueStreamStatus ValidateBindingRecords(
        const WorldComponentAttachmentSnapshotRecord *attachment_records,
        std::uint32_t attachment_record_count,
        const WorldComponentResourceBindingSnapshotRecord *binding_records,
        std::uint32_t binding_record_count) const;
    bool HasDuplicateIdentityWorldObjectId(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_index) const;
    bool HasDuplicateIdentityObjectHandle(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_index) const;
    bool HasDuplicateTransformWorldObjectId(
        const WorldSceneObjectTransformRestoreTransformRecord *records,
        std::uint32_t record_index) const;
    bool HasDuplicateAttachment(
        const WorldComponentAttachmentSnapshotRecord *records,
        std::uint32_t record_index) const;
    bool HasDuplicateBinding(
        const WorldComponentResourceBindingSnapshotRecord *records,
        std::uint32_t record_index) const;
    bool HasIdentityRecord(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_count,
        WorldObjectId world_object_id) const;
    bool HasAttachmentTuple(
        const WorldComponentAttachmentSnapshotRecord *attachment_records,
        std::uint32_t attachment_record_count,
        const WorldComponentResourceBindingSnapshotRecord &binding_record) const;
    void CopyOutputs(
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
        std::uint32_t *out_binding_count) const;

    std::uint32_t identity_capacity_;
    std::uint32_t transform_capacity_;
    std::uint32_t attachment_capacity_;
    std::uint32_t binding_capacity_;
    WorldSceneRecordValueStreamSnapshot snapshot_;
};
}
