// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformManifestStreamBridge.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldSceneObjectTransformManifestStreamDesc.h"
#include "YuEngine/World/WorldSceneObjectTransformManifestStreamResult.h"
#include "YuEngine/World/WorldSceneObjectTransformManifestStreamSnapshot.h"
#include "YuEngine/World/WorldSceneObjectTransformManifestStreamStatus.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreIdentityRecord.h"
#include "YuEngine/World/WorldSceneObjectTransformRestoreTransformRecord.h"

namespace yuengine::serialize {
class SerializeReader;
class SerializeWriter;
}

namespace yuengine::world {
class WorldSceneObjectTransformManifestStreamBridge final {
public:
    /**
     * @comment 构造 world scene object-transform manifest stream bridge。
     * @param desc 输入 bridge descriptor。
     */
    explicit WorldSceneObjectTransformManifestStreamBridge(
        WorldSceneObjectTransformManifestStreamDesc desc=
            WorldSceneObjectTransformManifestStreamDesc{});

    /**
     * @comment 将 caller-owned object-transform records 写入 deterministic manifest stream。
     * @param writer 调用方持有的 serialize writer。
     * @param input_identities 调用方持有的 identity restore records。
     * @param input_identity_count 输入 identity record count。
     * @param input_transforms 调用方持有的 transform restore records。
     * @param input_transform_count 输入 transform record count。
     * @return 显式操作结果。
     */
    WorldSceneObjectTransformManifestStreamResult WriteManifest(
        yuengine::serialize::SerializeWriter *writer,
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
        std::uint32_t input_transform_count);
    /**
     * @comment 从 deterministic manifest stream 读取 caller-owned object-transform records。
     * @param reader 调用方持有的 serialize reader。
     * @param output_identities 调用方持有的 identity output records。
     * @param output_identity_capacity Identity 输出容量。
     * @param out_identity_count 输出 identity record count。
     * @param output_transforms 调用方持有的 transform output records。
     * @param output_transform_capacity Transform 输出容量。
     * @param out_transform_count 输出 transform record count。
     * @return 显式操作结果。
     */
    WorldSceneObjectTransformManifestStreamResult ReadManifest(
        yuengine::serialize::SerializeReader *reader,
        WorldSceneObjectTransformRestoreIdentityRecord *output_identities,
        std::uint32_t output_identity_capacity,
        std::uint32_t *out_identity_count,
        WorldSceneObjectTransformRestoreTransformRecord *output_transforms,
        std::uint32_t output_transform_capacity,
        std::uint32_t *out_transform_count);
    /**
     * @comment 将 caller-owned object-transform records 写入 deterministic manifest stream。
     * @param writer 调用方持有的 serialize writer。
     * @param input_identities 调用方持有的 identity restore records。
     * @param input_identity_count 输入 identity record count。
     * @param input_transforms 调用方持有的 transform restore records。
     * @param input_transform_count 输入 transform record count。
     * @return 显式操作结果。
     */
    WorldSceneObjectTransformManifestStreamResult WriteSnapshot(
        yuengine::serialize::SerializeWriter *writer,
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
        std::uint32_t input_transform_count);
    /**
     * @comment 从 deterministic manifest stream 读取 caller-owned object-transform records。
     * @param reader 调用方持有的 serialize reader。
     * @param output_identities 调用方持有的 identity output records。
     * @param output_identity_capacity Identity 输出容量。
     * @param out_identity_count 输出 identity record count。
     * @param output_transforms 调用方持有的 transform output records。
     * @param output_transform_capacity Transform 输出容量。
     * @param out_transform_count 输出 transform record count。
     * @return 显式操作结果。
     */
    WorldSceneObjectTransformManifestStreamResult ReadSnapshot(
        yuengine::serialize::SerializeReader *reader,
        WorldSceneObjectTransformRestoreIdentityRecord *output_identities,
        std::uint32_t output_identity_capacity,
        std::uint32_t *out_identity_count,
        WorldSceneObjectTransformRestoreTransformRecord *output_transforms,
        std::uint32_t output_transform_capacity,
        std::uint32_t *out_transform_count);
    /**
     * @comment 返回当前 bridge 状态快照。
     * @return 快照值。
     */
    WorldSceneObjectTransformManifestStreamSnapshot Snapshot() const;

private:
    WorldSceneObjectTransformManifestStreamResult RecordFailure(
        WorldSceneObjectTransformManifestStreamStatus status);
    WorldSceneObjectTransformManifestStreamResult RecordRejectedFailure(
        WorldSceneObjectTransformManifestStreamStatus status);
    WorldSceneObjectTransformManifestStreamResult RecordSerializeFailure(
        yuengine::serialize::SerializeStatus status);
    WorldSceneObjectTransformManifestStreamResult RecordWriteSuccess(
        const WorldSceneObjectTransformManifestStreamState &state);
    WorldSceneObjectTransformManifestStreamResult RecordReadSuccess(
        const WorldSceneObjectTransformManifestStreamState &state);
    WorldSceneObjectTransformManifestStreamStatus ValidateBridgeCapacity() const;
    WorldSceneObjectTransformManifestStreamStatus ValidateWriteInputs(
        const WorldSceneObjectTransformRestoreIdentityRecord *input_identities,
        std::uint32_t input_identity_count,
        const WorldSceneObjectTransformRestoreTransformRecord *input_transforms,
        std::uint32_t input_transform_count) const;
    WorldSceneObjectTransformManifestStreamStatus ValidateReadOutputs(
        WorldSceneObjectTransformRestoreIdentityRecord *output_identities,
        std::uint32_t *out_identity_count,
        WorldSceneObjectTransformRestoreTransformRecord *output_transforms,
        std::uint32_t *out_transform_count) const;
    WorldSceneObjectTransformManifestStreamStatus ValidateIdentityRecords(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_count) const;
    WorldSceneObjectTransformManifestStreamStatus ValidateIdentityRecord(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_index) const;
    WorldSceneObjectTransformManifestStreamStatus ValidateTransformRecords(
        const WorldSceneObjectTransformRestoreIdentityRecord *identity_records,
        std::uint32_t identity_record_count,
        const WorldSceneObjectTransformRestoreTransformRecord *transform_records,
        std::uint32_t transform_record_count) const;
    WorldSceneObjectTransformManifestStreamStatus ValidateTransformRecord(
        const WorldSceneObjectTransformRestoreIdentityRecord *identity_records,
        std::uint32_t identity_record_count,
        const WorldSceneObjectTransformRestoreTransformRecord *transform_records,
        std::uint32_t record_index) const;
    bool HasDuplicateIdentityWorldObjectId(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_index) const;
    bool HasDuplicateIdentityObjectHandle(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_index) const;
    bool HasDuplicateTransformWorldObjectId(
        const WorldSceneObjectTransformRestoreTransformRecord *records,
        std::uint32_t record_index) const;
    bool HasIdentityRecord(
        const WorldSceneObjectTransformRestoreIdentityRecord *records,
        std::uint32_t record_count,
        WorldObjectId world_object_id) const;
    void CopyOutputs(
        const WorldSceneObjectTransformRestoreIdentityRecord *decoded_identities,
        std::uint32_t decoded_identity_count,
        WorldSceneObjectTransformRestoreIdentityRecord *output_identities,
        std::uint32_t *out_identity_count,
        const WorldSceneObjectTransformRestoreTransformRecord *decoded_transforms,
        std::uint32_t decoded_transform_count,
        WorldSceneObjectTransformRestoreTransformRecord *output_transforms,
        std::uint32_t *out_transform_count) const;

    std::uint32_t identity_capacity_;
    std::uint32_t transform_capacity_;
    WorldSceneObjectTransformManifestStreamSnapshot snapshot_;
};
}
