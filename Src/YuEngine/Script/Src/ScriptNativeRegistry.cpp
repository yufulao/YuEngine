// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Src/ScriptNativeRegistry.cpp

#include "YuEngine/Script/ScriptNativeRegistry.h"

using yuengine::memory::MemoryAccountingStatus;

namespace yuengine::script {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}

bool IsValueTypeValid(ScriptValueType type) {
    if (type == ScriptValueType::Bool) {
        return true;
    }

    if (type == ScriptValueType::Int32) {
        return true;
    }

    if (type == ScriptValueType::UInt32) {
        return true;
    }

    if (type == ScriptValueType::Int64) {
        return true;
    }

    if (type == ScriptValueType::UInt64) {
        return true;
    }

    return false;
}
}

ScriptNativeRegistry::ScriptNativeRegistry()
    : ScriptNativeRegistry(ScriptNativeRegistryDesc{}) {
}

ScriptNativeRegistry::ScriptNativeRegistry(ScriptNativeRegistryDesc desc)
    : bindings_{},
      snapshot_{
          ClampCapacity(desc.binding_capacity, MAX_SCRIPT_NATIVE_BINDING_COUNT),
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          ScriptStatus::Success} {
}

ScriptNativeRegistrationResult ScriptNativeRegistry::RegisterNativeCall(const ScriptNativeBinding &binding) {
    const ScriptStatus binding_status = ValidateBinding(binding);
    if (binding_status != ScriptStatus::Success) {
        return ScriptNativeRegistrationResult::Failure(RecordRegistryFailure(binding_status));
    }

    if (HasBinding(binding.call_id)) {
        return ScriptNativeRegistrationResult::Failure(RecordRegistryFailure(ScriptStatus::DuplicateCallId));
    }

    if (snapshot_.binding_count >= snapshot_.binding_capacity) {
        return ScriptNativeRegistrationResult::Failure(RecordRegistryFailure(ScriptStatus::CapacityExceeded));
    }

    bindings_[snapshot_.binding_count] = binding;
    ++snapshot_.binding_count;
    snapshot_.last_status = ScriptStatus::Success;
    return ScriptNativeRegistrationResult::Success(binding.call_id);
}

ScriptStatus ScriptNativeRegistry::Invoke(ScriptCallId call_id,
    const ScriptValue *arguments,
    std::uint32_t argument_count,
    ScriptValue *results,
    std::uint32_t result_count) {
    const ScriptNativeBinding *binding = FindBinding(call_id);
    if (binding == nullptr) {
        return RecordCallFailure(ScriptStatus::InvalidCallId);
    }

    if (binding->argument_count != argument_count) {
        return RecordCallFailure(ScriptStatus::ArgumentCountMismatch);
    }

    if (binding->result_count != result_count) {
        return RecordCallFailure(ScriptStatus::ResultCountMismatch);
    }

    if (argument_count > 0U) {
        if (arguments == nullptr) {
            return RecordCallFailure(ScriptStatus::InvalidArgumentBuffer);
        }
    }

    if (result_count > 0U) {
        if (results == nullptr) {
            return RecordCallFailure(ScriptStatus::InvalidResultBuffer);
        }
    }

    const ScriptStatus argument_status = ValidateValueTypes(arguments, binding->argument_types, argument_count);
    if (argument_status != ScriptStatus::Success) {
        return RecordCallFailure(argument_status);
    }

    const ScriptStatus result_status = ValidateResultTypes(results, binding->result_types, result_count);
    if (result_status != ScriptStatus::Success) {
        return RecordCallFailure(result_status);
    }

    const ScriptStatus call_status = binding->function(arguments, argument_count, results, result_count);
    if (call_status != ScriptStatus::Success) {
        return RecordCallFailure(call_status);
    }

    return RecordCallSuccess();
}

ScriptSnapshot ScriptNativeRegistry::Snapshot() const {
    return snapshot_;
}

ScriptStatus ScriptNativeRegistry::RecordRegistryFailure(ScriptStatus status) {
    snapshot_.last_status = status;
    return status;
}

ScriptStatus ScriptNativeRegistry::RecordCallFailure(ScriptStatus status) {
    ++snapshot_.failed_call_count;
    snapshot_.last_status = status;
    return status;
}

ScriptStatus ScriptNativeRegistry::RecordCallSuccess() {
    ++snapshot_.successful_call_count;
    snapshot_.last_status = ScriptStatus::Success;
    return ScriptStatus::Success;
}

ScriptStatus ScriptNativeRegistry::ValidateBinding(const ScriptNativeBinding &binding) const {
    if (!binding.call_id.IsValid()) {
        return ScriptStatus::InvalidCallId;
    }

    if (binding.function == nullptr) {
        return ScriptStatus::NullNativeFunction;
    }

    if (binding.argument_count > MAX_SCRIPT_ARGUMENT_COUNT) {
        return ScriptStatus::ArgumentCountMismatch;
    }

    if (binding.result_count > MAX_SCRIPT_RESULT_COUNT) {
        return ScriptStatus::ResultCountMismatch;
    }

    for (std::uint32_t index = 0U; index < binding.argument_count; ++index) {
        if (!IsValueTypeValid(binding.argument_types[index])) {
            return ScriptStatus::ArgumentTypeMismatch;
        }
    }

    for (std::uint32_t index = 0U; index < binding.result_count; ++index) {
        if (!IsValueTypeValid(binding.result_types[index])) {
            return ScriptStatus::ResultTypeMismatch;
        }
    }

    return ScriptStatus::Success;
}

ScriptStatus ScriptNativeRegistry::ValidateValueTypes(const ScriptValue *values,
    const std::array<ScriptValueType, MAX_SCRIPT_ARGUMENT_COUNT> &expected_types,
    std::uint32_t value_count) const {
    for (std::uint32_t index = 0U; index < value_count; ++index) {
        if (values[index].type != expected_types[index]) {
            return ScriptStatus::ArgumentTypeMismatch;
        }
    }

    return ScriptStatus::Success;
}

ScriptStatus ScriptNativeRegistry::ValidateResultTypes(const ScriptValue *values,
    const std::array<ScriptValueType, MAX_SCRIPT_RESULT_COUNT> &expected_types,
    std::uint32_t value_count) const {
    for (std::uint32_t index = 0U; index < value_count; ++index) {
        if (values[index].type != expected_types[index]) {
            return ScriptStatus::ResultTypeMismatch;
        }
    }

    return ScriptStatus::Success;
}

const ScriptNativeBinding* ScriptNativeRegistry::FindBinding(ScriptCallId call_id) const {
    for (std::uint32_t index = 0U; index < snapshot_.binding_count; ++index) {
        const ScriptNativeBinding &binding = bindings_[index];
        if (binding.call_id.value == call_id.value) {
            return &binding;
        }
    }

    return nullptr;
}

bool ScriptNativeRegistry::HasBinding(ScriptCallId call_id) const {
    return FindBinding(call_id) != nullptr;
}
}
