// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneAssemblyManifestStreamBridge.h

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
     * @comment 构造 world scene assembly manifest stream bridge。
     * @param desc 输入 bridge descriptor。
     */
    explicit WorldSceneAssemblyManifestStreamBridge(
        WorldSceneAssemblyManifestStreamDesc desc=WorldSceneAssemblyManifestStreamDesc{});

    /**
     * @comment 将调用方持有的 sidecar records 写入 deterministic manifest stream。
     * @param writer 调用方持有的 serialize writer。
     * @param input_attachments 调用方持有的 attachment records。
     * @param input_attachment_count 输入 attachment record count。
     * @param input_bindings 调用方持有的 component-resource binding records。
     * @param input_binding_count 输入 binding record count。
     * @return 显式操作结果。
     */
    WorldSceneAssemblyManifestStreamResult WriteManifest(
        yuengine::serialize::SerializeWriter *writer,
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count,
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t input_binding_count);
    /**
     * @comment 从 deterministic manifest stream 读取调用方持有的 sidecar records。
     * @param reader 调用方持有的 serialize reader。
     * @param output_attachments 调用方持有的 attachment output records。
     * @param output_attachment_capacity Attachment 输出容量。
     * @param out_attachment_count 输出 attachment record count。
     * @param output_bindings 调用方持有的 binding output records。
     * @param output_binding_capacity Binding 输出容量。
     * @param out_binding_count 输出 binding record count。
     * @return 显式操作结果。
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
     * @comment 将调用方持有的 sidecar records 写入 deterministic manifest stream。
     * @param writer 调用方持有的 serialize writer。
     * @param input_attachments 调用方持有的 attachment records。
     * @param input_attachment_count 输入 attachment record count。
     * @param input_bindings 调用方持有的 component-resource binding records。
     * @param input_binding_count 输入 binding record count。
     * @return 显式操作结果。
     */
    WorldSceneAssemblyManifestStreamResult WriteSnapshot(
        yuengine::serialize::SerializeWriter *writer,
        const WorldComponentAttachmentSnapshotRecord *input_attachments,
        std::uint32_t input_attachment_count,
        const WorldComponentResourceBindingSnapshotRecord *input_bindings,
        std::uint32_t input_binding_count);
    /**
     * @comment 从 deterministic manifest stream 读取调用方持有的 sidecar records。
     * @param reader 调用方持有的 serialize reader。
     * @param output_attachments 调用方持有的 attachment output records。
     * @param output_attachment_capacity Attachment 输出容量。
     * @param out_attachment_count 输出 attachment record count。
     * @param output_bindings 调用方持有的 binding output records。
     * @param output_binding_capacity Binding 输出容量。
     * @param out_binding_count 输出 binding record count。
     * @return 显式操作结果。
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
     * @comment 返回当前 bridge 状态快照。
     * @return 快照值。
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
