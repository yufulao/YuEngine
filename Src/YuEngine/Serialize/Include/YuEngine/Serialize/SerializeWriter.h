// 模块: YuEngine Serialize
// 文件: Src/YuEngine/Serialize/Include/YuEngine/Serialize/SerializeWriter.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/Serialize/SerializeConstants.h"
#include "YuEngine/Serialize/SerializeFieldId.h"
#include "YuEngine/Serialize/SerializeRecordId.h"
#include "YuEngine/Serialize/SerializeSnapshot.h"
#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/Serialize/SerializeTypeTag.h"

namespace yuengine::serialize {
class SerializeWriter final {
public:
    /**
     * @comment 构造 SerializeWriter 实例。
     * @param buffer 输入 buffer。
     * @param capacity 输入容量。
     */
    SerializeWriter(std::uint8_t* buffer, std::uint32_t capacity);

    /**
     * @comment 开始 serialized stream。
     * @return 显式操作状态。
     */
    SerializeStatus BeginStream();
    /**
     * @comment 开始 serialized record。
     * @param record 输入 record。
     * @return 显式操作状态。
     */
    SerializeStatus BeginRecord(SerializeRecordId record);
    /**
     * @comment 写入 uint32。
     * @param field 输入 field。
     * @param value 输入 value。
     * @return 显式操作状态。
     */
    SerializeStatus WriteUInt32(SerializeFieldId field, std::uint32_t value);
    /**
     * @comment 写入 int32。
     * @param field 输入 field。
     * @param value 输入 value。
     * @return 显式操作状态。
     */
    SerializeStatus WriteInt32(SerializeFieldId field, std::int32_t value);
    /**
     * @comment 写入 uint64。
     * @param field 输入 field。
     * @param value 输入 value。
     * @return 显式操作状态。
     */
    SerializeStatus WriteUInt64(SerializeFieldId field, std::uint64_t value);
    /**
     * @comment 写入 int64。
     * @param field 输入 field。
     * @param value 输入 value。
     * @return 显式操作状态。
     */
    SerializeStatus WriteInt64(SerializeFieldId field, std::int64_t value);
    /**
     * @comment 写入 fixed bytes。
     * @param field 输入 field。
     * @param bytes 输入 byte 数量或 byte payload。
     * @param byte_count 输入 byte 数量。
     * @return 显式操作状态。
     */
    SerializeStatus WriteFixedBytes(SerializeFieldId field, const std::uint8_t* bytes, std::uint32_t byte_count);
    /**
     * @comment 返回 writer 截断后的 byte 容量。
     * @return byte 容量。
     */
    std::uint32_t GetByteCapacity() const;
    /**
     * @comment 返回剩余可写 byte 容量。
     * @return 剩余可写 byte 容量。
     */
    std::uint32_t GetRemainingByteCapacity() const;
    /**
     * @comment 检查 writer 是否可以提交请求的 byte 数量。
     * @param byte_count 输入 byte 数量。
     * @return writer 可以提交 bytes 时返回 true，否则返回 false。
     */
    bool CanCommitByteCount(std::uint32_t byte_count) const;
    /**
     * @comment 返回当前状态快照。
     * @return 快照值。
     */
    SerializeSnapshot Snapshot() const;

private:
    SerializeStatus CommitField(
        SerializeFieldId field,
        SerializeTypeTag type,
        const std::uint8_t* payload,
        std::uint32_t byte_count);
    SerializeStatus RecordFailure(SerializeStatus status);
    void RecordSuccess();
    bool CanCommitBytes(std::uint32_t byte_count) const;
    bool HasFieldInCurrentRecord(SerializeFieldId field) const;
    void WriteUInt16At(std::uint32_t offset, std::uint16_t value);
    void WriteUInt32At(std::uint32_t offset, std::uint32_t value);
    void CopyPayload(std::uint32_t offset, const std::uint8_t* payload, std::uint32_t byte_count);

    std::uint8_t* buffer_;
    std::uint32_t capacity_;
    std::uint32_t active_record_offset_;
    std::uint32_t current_record_field_count_;
    std::array<SerializeFieldId, MAX_FIELDS_PER_RECORD> current_record_fields_;
    SerializeSnapshot snapshot_;
    bool has_stream_;
    bool has_active_record_;
};
}
