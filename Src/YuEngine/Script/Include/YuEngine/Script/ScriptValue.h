// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Include/YuEngine/Script/ScriptValue.h

#pragma once

#include <cstdint>

#include "YuEngine/Script/ScriptValueType.h"

namespace yuengine::script {
struct ScriptValue final {
    ScriptValueType type = ScriptValueType::None;
    std::uint64_t payload = 0U;

    /**
     * @comment 创建 bool script value。
     * @param value 输入 value。
     * @return Script value slot 值。
     */
    static ScriptValue Bool(bool value) {
        if (value) {
            return ScriptValue{ScriptValueType::Bool, 1U};
        }

        return ScriptValue{ScriptValueType::Bool, 0U};
    }

    /**
     * @comment 创建 int32 script value。
     * @param value 输入 value。
     * @return Script value slot 值。
     */
    static ScriptValue Int32(std::int32_t value) {
        const std::uint32_t stored_value = static_cast<std::uint32_t>(value);
        return ScriptValue{ScriptValueType::Int32, stored_value};
    }

    /**
     * @comment 创建 uint32 script value。
     * @param value 输入 value。
     * @return Script value slot 值。
     */
    static ScriptValue UInt32(std::uint32_t value) {
        return ScriptValue{ScriptValueType::UInt32, value};
    }

    /**
     * @comment 创建 int64 script value。
     * @param value 输入 value。
     * @return Script value slot 值。
     */
    static ScriptValue Int64(std::int64_t value) {
        const std::uint64_t stored_value = static_cast<std::uint64_t>(value);
        return ScriptValue{ScriptValueType::Int64, stored_value};
    }

    /**
     * @comment 创建 uint64 script value。
     * @param value 输入 value。
     * @return Script value slot 值。
     */
    static ScriptValue UInt64(std::uint64_t value) {
        return ScriptValue{ScriptValueType::UInt64, value};
    }

    /**
     * @comment 返回 bool value。
     * @return Bool 值。
     */
    bool AsBool() const {
        return payload != 0U;
    }

    /**
     * @comment 返回 int32 value。
     * @return Int32 值。
     */
    std::int32_t AsInt32() const {
        const std::uint32_t stored_value = static_cast<std::uint32_t>(payload);
        return static_cast<std::int32_t>(stored_value);
    }

    /**
     * @comment 返回 uint32 value。
     * @return UInt32 值。
     */
    std::uint32_t AsUInt32() const {
        return static_cast<std::uint32_t>(payload);
    }

    /**
     * @comment 返回 int64 value。
     * @return Int64 值。
     */
    std::int64_t AsInt64() const {
        return static_cast<std::int64_t>(payload);
    }

    /**
     * @comment 返回 uint64 value。
     * @return UInt64 值。
     */
    std::uint64_t AsUInt64() const {
        return payload;
    }
};
}
