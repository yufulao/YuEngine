// Module: YuEngine Script
// File: Src/YuEngine/Script/Include/YuEngine/Script/ScriptValue.h

#pragma once

#include <cstdint>

#include "YuEngine/Script/ScriptValueType.h"

namespace yuengine::script {
struct ScriptValue final {
    ScriptValueType type = ScriptValueType::None;
    std::uint64_t payload = 0U;

    /**
     * @comment Creates a bool script value.
     * @param value Input value.
     * @return Script value slot.
     */
    static ScriptValue Bool(bool value) {
        if (value) {
            return ScriptValue{ScriptValueType::Bool, 1U};
        }

        return ScriptValue{ScriptValueType::Bool, 0U};
    }

    /**
     * @comment Creates a int32 script value.
     * @param value Input value.
     * @return Script value slot.
     */
    static ScriptValue Int32(std::int32_t value) {
        const std::uint32_t stored_value = static_cast<std::uint32_t>(value);
        return ScriptValue{ScriptValueType::Int32, stored_value};
    }

    /**
     * @comment Creates a uint32 script value.
     * @param value Input value.
     * @return Script value slot.
     */
    static ScriptValue UInt32(std::uint32_t value) {
        return ScriptValue{ScriptValueType::UInt32, value};
    }

    /**
     * @comment Creates a int64 script value.
     * @param value Input value.
     * @return Script value slot.
     */
    static ScriptValue Int64(std::int64_t value) {
        const std::uint64_t stored_value = static_cast<std::uint64_t>(value);
        return ScriptValue{ScriptValueType::Int64, stored_value};
    }

    /**
     * @comment Creates a uint64 script value.
     * @param value Input value.
     * @return Script value slot.
     */
    static ScriptValue UInt64(std::uint64_t value) {
        return ScriptValue{ScriptValueType::UInt64, value};
    }

    /**
     * @comment Returns the bool value.
     * @return Bool value.
     */
    bool AsBool() const {
        return payload != 0U;
    }

    /**
     * @comment Returns the int32 value.
     * @return Int32 value.
     */
    std::int32_t AsInt32() const {
        const std::uint32_t stored_value = static_cast<std::uint32_t>(payload);
        return static_cast<std::int32_t>(stored_value);
    }

    /**
     * @comment Returns the uint32 value.
     * @return UInt32 value.
     */
    std::uint32_t AsUInt32() const {
        return static_cast<std::uint32_t>(payload);
    }

    /**
     * @comment Returns the int64 value.
     * @return Int64 value.
     */
    std::int64_t AsInt64() const {
        return static_cast<std::int64_t>(payload);
    }

    /**
     * @comment Returns the uint64 value.
     * @return UInt64 value.
     */
    std::uint64_t AsUInt64() const {
        return payload;
    }
};
}
