// Module: YuEngine Script
// File: Src/YuEngine/Script/Include/YuEngine/Script/ScriptNativeRegistry.h

#pragma once

#include <array>
#include <cstddef>

#include "YuEngine/Script/ScriptConstants.h"
#include "YuEngine/Script/ScriptNativeBinding.h"
#include "YuEngine/Script/ScriptNativeRegistrationResult.h"
#include "YuEngine/Script/ScriptNativeRegistryDesc.h"
#include "YuEngine/Script/ScriptSnapshot.h"
#include "YuEngine/Script/ScriptStatus.h"
#include "YuEngine/Script/ScriptValue.h"

namespace yuengine::script {
class ScriptNativeRegistry final {
public:
    /**
     * @comment Constructs a ScriptNativeRegistry instance.
     */
    ScriptNativeRegistry();
    /**
     * @comment Constructs a ScriptNativeRegistry instance.
     * @param desc Input descriptor.
     */
    explicit ScriptNativeRegistry(ScriptNativeRegistryDesc desc);

    /**
     * @comment Registers native call binding.
     * @param binding Input native binding.
     * @return Explicit operation result.
     */
    ScriptNativeRegistrationResult RegisterNativeCall(const ScriptNativeBinding &binding);
    /**
     * @comment Invokes native call by stable call id.
     * @param call_id Input call id.
     * @param arguments Input argument slots.
     * @param argument_count Input argument count.
     * @param results Output result slots.
     * @param result_count Input result count.
     * @return Explicit operation status.
     */
    ScriptStatus Invoke(ScriptCallId call_id,
        const ScriptValue *arguments,
        std::uint32_t argument_count,
        ScriptValue *results,
        std::uint32_t result_count);
    /**
     * @comment Returns a snapshot of the current state.
     * @return Snapshot value.
     */
    ScriptSnapshot Snapshot() const;

private:
    ScriptStatus RecordRegistryFailure(ScriptStatus status);
    ScriptStatus RecordCallFailure(ScriptStatus status);
    ScriptStatus RecordCallSuccess();
    ScriptStatus ValidateBinding(const ScriptNativeBinding &binding) const;
    ScriptStatus ValidateValueTypes(const ScriptValue *values,
        const std::array<ScriptValueType, MAX_SCRIPT_ARGUMENT_COUNT> &expected_types,
        std::uint32_t value_count) const;
    ScriptStatus ValidateResultTypes(const ScriptValue *values,
        const std::array<ScriptValueType, MAX_SCRIPT_RESULT_COUNT> &expected_types,
        std::uint32_t value_count) const;
    const ScriptNativeBinding* FindBinding(ScriptCallId call_id) const;
    bool HasBinding(ScriptCallId call_id) const;

    std::array<ScriptNativeBinding, MAX_SCRIPT_NATIVE_BINDING_COUNT> bindings_;
    ScriptSnapshot snapshot_;
};
}
