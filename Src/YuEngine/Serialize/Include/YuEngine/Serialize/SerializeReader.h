// 模块: YuEngine Serialize
// 文件: Src/YuEngine/Serialize/Include/YuEngine/Serialize/SerializeReader.h

#pragma once

#include <cstdint>

#include "YuEngine/Serialize/SerializeFieldId.h"
#include "YuEngine/Serialize/SerializeRecordId.h"
#include "YuEngine/Serialize/SerializeSnapshot.h"
#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/Serialize/FieldLocation.h"

namespace yuengine::serialize {
class SerializeReader final {
public:
    /**
     * @comment 构造 SerializeReader 实例。
     * @param buffer 输入 buffer。
     * @param byte_count 输入 byte 数量。
     */
    SerializeReader(const std::uint8_t* buffer, std::uint32_t byte_count);

    /**
     * @comment 打开 serialized stream。
     * @return 显式操作状态。
     */
    SerializeStatus OpenStream();
    /**
     * @comment 读取 uint32。
     * @param record 输入 record。
     * @param field 输入 field。
     * @param out_value 成功时写入输出 value。
     * @return 显式操作状态。
     */
    SerializeStatus ReadUInt32(SerializeRecordId record, SerializeFieldId field, std::uint32_t& out_value);
    /**
     * @comment 读取 int32。
     * @param record 输入 record。
     * @param field 输入 field。
     * @param out_value 成功时写入输出 value。
     * @return 显式操作状态。
     */
    SerializeStatus ReadInt32(SerializeRecordId record, SerializeFieldId field, std::int32_t& out_value);
    /**
     * @comment 读取 uint64。
     * @param record 输入 record。
     * @param field 输入 field。
     * @param out_value 成功时写入输出 value。
     * @return 显式操作状态。
     */
    SerializeStatus ReadUInt64(SerializeRecordId record, SerializeFieldId field, std::uint64_t& out_value);
    /**
     * @comment 读取 int64。
     * @param record 输入 record。
     * @param field 输入 field。
     * @param out_value 成功时写入输出 value。
     * @return 显式操作状态。
     */
    SerializeStatus ReadInt64(SerializeRecordId record, SerializeFieldId field, std::int64_t& out_value);
    /**
     * @comment 读取 fixed bytes。
     * @param record 输入 record。
     * @param field 输入 field。
     * @param out_bytes 成功时写入输出 bytes。
     * @param out_capacity 成功时写入输出 capacity。
     * @param out_byte_count 成功时写入输出 byte count。
     * @return 显式操作状态。
     */
    SerializeStatus ReadFixedBytes(
        SerializeRecordId record,
        SerializeFieldId field,
        std::uint8_t* out_bytes,
        std::uint32_t out_capacity,
        std::uint32_t& out_byte_count);
    /**
     * @comment 返回当前状态快照。
     * @return 快照值。
     */
    SerializeSnapshot Snapshot() const;

private:
    SerializeStatus ValidateStream(
        std::uint32_t &out_committed_byte_count,
        std::uint32_t &out_record_count,
        std::uint32_t &out_field_count);
    SerializeStatus FindField(SerializeRecordId record, SerializeFieldId field, FieldLocation& out_location) const;
    SerializeStatus RecordFailure(SerializeStatus status);
    void RecordSuccess();
    bool CanReadBytes(std::uint32_t offset, std::uint32_t byte_count) const;
    bool IsKnownTypeTag(std::uint32_t value) const;
    bool IsDuplicateField(SerializeFieldId field, const SerializeFieldId* fields, std::uint32_t field_count) const;
    std::uint16_t ReadUInt16At(std::uint32_t offset) const;
    std::uint32_t ReadUInt32At(std::uint32_t offset) const;
    std::uint64_t ReadUInt64At(std::uint32_t offset) const;

    const std::uint8_t* buffer_;
    std::uint32_t byte_count_;
    SerializeSnapshot snapshot_;
    bool is_open_;
};
}
