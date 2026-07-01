// 模块：Tests Script
// 文件：Tests/Script/ScriptTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Script/ScriptConstants.h"
#include "YuEngine/Script/ScriptNativeBinding.h"
#include "YuEngine/Script/ScriptNativeRegistry.h"
#include "YuEngine/Script/ScriptRuntimePhase.h"
#include "YuEngine/Script/ScriptRuntimePhaseDispatchAdapter.h"
#include "YuEngine/Script/ScriptRuntimePhaseDispatchAdapterDesc.h"
#include "YuEngine/Script/ScriptRuntimePhaseDispatchResult.h"
#include "YuEngine/Script/ScriptRuntimePhaseDispatchSnapshot.h"
#include "YuEngine/Script/ScriptRuntimePhaseDispatchStatus.h"
#include "YuEngine/Script/ScriptRuntimePhaseTrace.h"
#include "YuEngine/Script/ScriptStatus.h"
#include "YuEngine/Script/ScriptValue.h"
#include "YuEngine/Script/ScriptValueType.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::script::MAX_SCRIPT_NATIVE_BINDING_COUNT;
using yuengine::script::ScriptCallId;
using yuengine::script::ScriptNativeBinding;
using yuengine::script::ScriptNativeFunction;
using yuengine::script::ScriptNativeRegistrationResult;
using yuengine::script::ScriptNativeRegistry;
using yuengine::script::ScriptNativeRegistryDesc;
using yuengine::script::ScriptRuntimePhase;
using yuengine::script::ScriptRuntimePhaseDispatchAdapter;
using yuengine::script::ScriptRuntimePhaseDispatchAdapterDesc;
using yuengine::script::ScriptRuntimePhaseDispatchResult;
using yuengine::script::ScriptRuntimePhaseDispatchSnapshot;
using yuengine::script::ScriptRuntimePhaseDispatchStatus;
using yuengine::script::ScriptRuntimePhaseTrace;
using yuengine::script::ScriptSnapshot;
using yuengine::script::ScriptStatus;
using yuengine::script::ScriptValue;
using yuengine::script::ScriptValueType;

namespace {
constexpr const char *TEST_REGISTER_STABLE_ID = "Script_RegisterNativeCall_ReturnsStableId";
constexpr const char *TEST_DUPLICATE = "Script_RegisterDuplicateCall_ReturnsExplicitStatus";
constexpr const char *TEST_NULL_FUNCTION = "Script_RegisterRejectsNullFunction";
constexpr const char *TEST_CAPACITY = "Script_RegistryCapacityOverflow_DoesNotMutate";
constexpr const char *TEST_INVOKE_RESULT = "Script_InvokeNativeCall_WritesResultDeterministically";
constexpr const char *TEST_UNKNOWN_CALL = "Script_InvokeRejectsUnknownCallId";
constexpr const char *TEST_ARGUMENT_COUNT = "Script_InvokeRejectsArgumentCountMismatch";
constexpr const char *TEST_ARGUMENT_TYPE = "Script_InvokeRejectsArgumentTypeMismatch";
constexpr const char *TEST_RESULT_COUNT = "Script_InvokeRejectsResultCountMismatch";
constexpr const char *TEST_NATIVE_FAILURE = "Script_NativeFailure_ReturnsExplicitStatus";
constexpr const char *TEST_CALL_PATH_CAPACITY = "Script_CallPath_DoesNotGrowRegistryStorage";
constexpr const char *TEST_NO_FORBIDDEN_DEPENDENCY = "Script_NoWorldResourcePackageOrGameAdapterDependency";
constexpr const char *TEST_NO_HIDDEN_ALLOCATION = "Script_NoHiddenAllocation_UsesYuMemorySignal";
constexpr const char *TEST_SNAPSHOT = "Script_SnapshotReportsCountsAndLastStatus";
constexpr const char *TEST_RUNTIME_PHASE_TRACE = "Script_RuntimePhaseDispatch_TraceMapsToCallIds";
constexpr const char *TEST_RUNTIME_PHASE_MISSING_CALL =
    "Script_RuntimePhaseDispatch_MissingCallReturnsExplicitStatus";
constexpr const char *TEST_RUNTIME_PHASE_INVALID_SLOTS =
    "Script_RuntimePhaseDispatch_InvalidSlotsReturnExplicitStatus";
constexpr const char *TEST_RUNTIME_PHASE_INVALID_PHASE =
    "Script_RuntimePhaseDispatch_InvalidPhaseDoesNotMutate";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr ScriptCallId CALL_ADD{1U};
constexpr ScriptCallId CALL_FAILING{2U};
constexpr ScriptCallId CALL_SECOND{3U};
constexpr ScriptCallId CALL_RUNTIME_BEGIN{11U};
constexpr ScriptCallId CALL_RUNTIME_END{12U};
constexpr ScriptCallId CALL_UNKNOWN{99U};
using TestFunction = int (*)();

int Fail(const std::string &message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

ScriptStatus AddInt32Native(const ScriptValue *arguments,
    std::uint32_t argument_count,
    ScriptValue *results,
    std::uint32_t result_count) {
    if (argument_count != 2U) {
        return ScriptStatus::ArgumentCountMismatch;
    }

    if (result_count != 1U) {
        return ScriptStatus::ResultCountMismatch;
    }

    const std::int32_t left_value = arguments[0].AsInt32();
    const std::int32_t right_value = arguments[1].AsInt32();
    const std::int32_t sum_value = left_value + right_value;
    results[0] = ScriptValue::Int32(sum_value);
    return ScriptStatus::Success;
}

ScriptStatus FailingNative(const ScriptValue *arguments,
    std::uint32_t argument_count,
    ScriptValue *results,
    std::uint32_t result_count) {
    if (argument_count != 1U) {
        return ScriptStatus::ArgumentCountMismatch;
    }

    if (result_count != 1U) {
        return ScriptStatus::ResultCountMismatch;
    }

    const std::int32_t source_value = arguments[0].AsInt32();
    results[0] = ScriptValue::Int32(source_value + 1);
    return ScriptStatus::NativeCallFailed;
}

ScriptStatus AddOneRuntimeNative(const ScriptValue *arguments,
    std::uint32_t argument_count,
    ScriptValue *results,
    std::uint32_t result_count) {
    static_cast<void>(arguments);
    if (argument_count != 0U) {
        return ScriptStatus::ArgumentCountMismatch;
    }

    if (result_count != 1U) {
        return ScriptStatus::ResultCountMismatch;
    }

    if (results == nullptr) {
        return ScriptStatus::InvalidResultBuffer;
    }

    if (results[0].type != ScriptValueType::UInt32) {
        return ScriptStatus::ResultTypeMismatch;
    }

    const std::uint32_t current_value = results[0].AsUInt32();
    results[0] = ScriptValue::UInt32(current_value + 1U);
    return ScriptStatus::Success;
}

ScriptStatus AddTenRuntimeNative(const ScriptValue *arguments,
    std::uint32_t argument_count,
    ScriptValue *results,
    std::uint32_t result_count) {
    static_cast<void>(arguments);
    if (argument_count != 0U) {
        return ScriptStatus::ArgumentCountMismatch;
    }

    if (result_count != 1U) {
        return ScriptStatus::ResultCountMismatch;
    }

    if (results == nullptr) {
        return ScriptStatus::InvalidResultBuffer;
    }

    if (results[0].type != ScriptValueType::UInt32) {
        return ScriptStatus::ResultTypeMismatch;
    }

    const std::uint32_t current_value = results[0].AsUInt32();
    results[0] = ScriptValue::UInt32(current_value + 10U);
    return ScriptStatus::Success;
}

ScriptNativeBinding MakeAddBinding(ScriptCallId call_id) {
    ScriptNativeBinding binding{};
    binding.call_id = call_id;
    binding.function = AddInt32Native;
    binding.argument_count = 2U;
    binding.argument_types[0] = ScriptValueType::Int32;
    binding.argument_types[1] = ScriptValueType::Int32;
    binding.result_count = 1U;
    binding.result_types[0] = ScriptValueType::Int32;
    return binding;
}

ScriptNativeBinding MakeFailingBinding(ScriptCallId call_id) {
    ScriptNativeBinding binding{};
    binding.call_id = call_id;
    binding.function = FailingNative;
    binding.argument_count = 1U;
    binding.argument_types[0] = ScriptValueType::Int32;
    binding.result_count = 1U;
    binding.result_types[0] = ScriptValueType::Int32;
    return binding;
}

ScriptNativeBinding MakeNullBinding(ScriptCallId call_id) {
    ScriptNativeBinding binding = MakeAddBinding(call_id);
    binding.function = nullptr;
    return binding;
}

ScriptNativeBinding MakeRuntimePhaseBinding(ScriptCallId call_id, ScriptNativeFunction function) {
    ScriptNativeBinding binding{};
    binding.call_id = call_id;
    binding.function = function;
    binding.argument_count = 0U;
    binding.result_count = 1U;
    binding.result_types[0] = ScriptValueType::UInt32;
    return binding;
}

ScriptNativeRegistrationResult RegisterAddBinding(ScriptNativeRegistry &registry) {
    const ScriptNativeBinding binding = MakeAddBinding(CALL_ADD);
    return registry.RegisterNativeCall(binding);
}

ScriptNativeRegistrationResult RegisterRuntimeBinding(
    ScriptNativeRegistry &registry,
    ScriptCallId call_id,
    ScriptNativeFunction function) {
    const ScriptNativeBinding binding = MakeRuntimePhaseBinding(call_id, function);
    return registry.RegisterNativeCall(binding);
}

std::array<ScriptValue, 2> MakeAddArguments(std::int32_t left_value, std::int32_t right_value) {
    std::array<ScriptValue, 2> arguments{};
    arguments[0] = ScriptValue::Int32(left_value);
    arguments[1] = ScriptValue::Int32(right_value);
    return arguments;
}

std::array<ScriptValue, 1> MakeSingleIntResult(std::int32_t value) {
    std::array<ScriptValue, 1> results{};
    results[0] = ScriptValue::Int32(value);
    return results;
}

std::array<ScriptValue, 1> MakeRuntimeResult(std::uint32_t value) {
    std::array<ScriptValue, 1> results{};
    results[0] = ScriptValue::UInt32(value);
    return results;
}

ScriptRuntimePhaseTrace RuntimeTrace(ScriptRuntimePhase phase, std::uint64_t frame_index) {
    ScriptRuntimePhaseTrace trace{};
    trace.phase = phase;
    trace.frame_index = frame_index;
    trace.active_object_count = 2U;
    trace.skipped_object_count = 1U;
    return trace;
}

int ScriptRegisterNativeCallReturnsStableId() {
    ScriptNativeRegistry registry;
    const ScriptNativeRegistrationResult result = RegisterAddBinding(registry);
    if (!result.Succeeded()) {
        return Fail("native binding registration failed");
    }

    if (result.call_id.value != CALL_ADD.value) {
        return Fail("registered call id was not stable");
    }

    const ScriptSnapshot snapshot = registry.Snapshot();
    if (snapshot.binding_count != 1U) {
        return Fail("registered binding count was not recorded");
    }

    if (snapshot.last_status != ScriptStatus::Success) {
        return Fail("successful registration did not record success");
    }

    return 0;
}

int ScriptRegisterDuplicateCallReturnsExplicitStatus() {
    ScriptNativeRegistry registry;
    const ScriptNativeRegistrationResult first_result = RegisterAddBinding(registry);
    if (!first_result.Succeeded()) {
        return Fail("first native binding registration failed");
    }

    const ScriptNativeBinding duplicate_binding = MakeAddBinding(CALL_ADD);
    const ScriptNativeRegistrationResult duplicate_result = registry.RegisterNativeCall(duplicate_binding);
    if (duplicate_result.status != ScriptStatus::DuplicateCallId) {
        return Fail("duplicate registration did not return explicit status");
    }

    const ScriptSnapshot snapshot = registry.Snapshot();
    if (snapshot.binding_count != 1U) {
        return Fail("duplicate registration mutated binding count");
    }

    if (snapshot.last_status != ScriptStatus::DuplicateCallId) {
        return Fail("duplicate registration did not record last status");
    }

    return 0;
}

int ScriptRegisterRejectsNullFunction() {
    ScriptNativeRegistry registry;
    const ScriptNativeBinding binding = MakeNullBinding(CALL_ADD);
    const ScriptNativeRegistrationResult result = registry.RegisterNativeCall(binding);
    if (result.status != ScriptStatus::NullNativeFunction) {
        return Fail("null native function did not return explicit status");
    }

    if (registry.Snapshot().binding_count != 0U) {
        return Fail("null native function mutated binding count");
    }

    return 0;
}

int ScriptRegistryCapacityOverflowDoesNotMutate() {
    ScriptNativeRegistryDesc desc{};
    desc.binding_capacity = 1U;
    ScriptNativeRegistry registry(desc);

    const ScriptNativeBinding first_binding = MakeAddBinding(CALL_ADD);
    const ScriptNativeRegistrationResult first_result = registry.RegisterNativeCall(first_binding);
    if (!first_result.Succeeded()) {
        return Fail("first capacity fixture registration failed");
    }

    const ScriptSnapshot before_snapshot = registry.Snapshot();
    const ScriptNativeBinding second_binding = MakeAddBinding(CALL_SECOND);
    const ScriptNativeRegistrationResult second_result = registry.RegisterNativeCall(second_binding);
    if (second_result.status != ScriptStatus::CapacityExceeded) {
        return Fail("capacity overflow did not return explicit status");
    }

    if (second_result.required_binding_count != 2U) {
        return Fail("capacity overflow did not return required binding count");
    }

    const ScriptSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.binding_count != before_snapshot.binding_count) {
        return Fail("capacity overflow mutated binding count");
    }

    if (after_snapshot.binding_capacity != before_snapshot.binding_capacity) {
        return Fail("capacity overflow mutated binding capacity");
    }

    if (after_snapshot.last_required_binding_count != 2U) {
        return Fail("capacity overflow snapshot missed required binding count");
    }

    return 0;
}

int ScriptInvokeNativeCallWritesResultDeterministically() {
    ScriptNativeRegistry registry;
    const ScriptNativeRegistrationResult registration = RegisterAddBinding(registry);
    if (!registration.Succeeded()) {
        return Fail("add binding registration failed");
    }

    std::array<ScriptValue, 2> arguments = MakeAddArguments(7, 5);
    std::array<ScriptValue, 1> results = MakeSingleIntResult(0);
    const ScriptStatus status = registry.Invoke(CALL_ADD, arguments.data(), 2U, results.data(), 1U);
    if (status != ScriptStatus::Success) {
        return Fail("native add call failed");
    }

    if (results[0].AsInt32() != 12) {
        return Fail("native add result was not deterministic");
    }

    if (registry.Snapshot().successful_call_count != 1U) {
        return Fail("successful native call count was not recorded");
    }

    return 0;
}

int ScriptInvokeRejectsUnknownCallId() {
    ScriptNativeRegistry registry;
    std::array<ScriptValue, 1> results = MakeSingleIntResult(0);
    const ScriptStatus status = registry.Invoke(CALL_UNKNOWN, nullptr, 0U, results.data(), 1U);
    if (status != ScriptStatus::InvalidCallId) {
        return Fail("unknown native call id did not return explicit status");
    }

    if (registry.Snapshot().failed_call_count != 1U) {
        return Fail("unknown native call did not record failure count");
    }

    return 0;
}

int ScriptInvokeRejectsArgumentCountMismatch() {
    ScriptNativeRegistry registry;
    const ScriptNativeRegistrationResult registration = RegisterAddBinding(registry);
    if (!registration.Succeeded()) {
        return Fail("argument count fixture registration failed");
    }

    std::array<ScriptValue, 1> arguments{};
    arguments[0] = ScriptValue::Int32(7);
    std::array<ScriptValue, 1> results = MakeSingleIntResult(0);
    const ScriptStatus status = registry.Invoke(CALL_ADD, arguments.data(), 1U, results.data(), 1U);
    if (status != ScriptStatus::ArgumentCountMismatch) {
        return Fail("argument count mismatch did not return explicit status");
    }

    if (results[0].AsInt32() != 0) {
        return Fail("argument count mismatch mutated result slot");
    }

    return 0;
}

int ScriptInvokeRejectsArgumentTypeMismatch() {
    ScriptNativeRegistry registry;
    const ScriptNativeRegistrationResult registration = RegisterAddBinding(registry);
    if (!registration.Succeeded()) {
        return Fail("argument type fixture registration failed");
    }

    std::array<ScriptValue, 2> arguments{};
    arguments[0] = ScriptValue::UInt32(7U);
    arguments[1] = ScriptValue::Int32(5);
    std::array<ScriptValue, 1> results = MakeSingleIntResult(0);
    const ScriptStatus status = registry.Invoke(CALL_ADD, arguments.data(), 2U, results.data(), 1U);
    if (status != ScriptStatus::ArgumentTypeMismatch) {
        return Fail("argument type mismatch did not return explicit status");
    }

    if (results[0].AsInt32() != 0) {
        return Fail("argument type mismatch mutated result slot");
    }

    return 0;
}

int ScriptInvokeRejectsResultCountMismatch() {
    ScriptNativeRegistry registry;
    const ScriptNativeRegistrationResult registration = RegisterAddBinding(registry);
    if (!registration.Succeeded()) {
        return Fail("result count fixture registration failed");
    }

    std::array<ScriptValue, 2> arguments = MakeAddArguments(7, 5);
    const ScriptStatus status = registry.Invoke(CALL_ADD, arguments.data(), 2U, nullptr, 0U);
    if (status != ScriptStatus::ResultCountMismatch) {
        return Fail("result count mismatch did not return explicit status");
    }

    return 0;
}

int ScriptNativeFailureReturnsExplicitStatus() {
    ScriptNativeRegistry registry;
    const ScriptNativeBinding binding = MakeFailingBinding(CALL_FAILING);
    const ScriptNativeRegistrationResult registration = registry.RegisterNativeCall(binding);
    if (!registration.Succeeded()) {
        return Fail("failing native binding registration failed");
    }

    std::array<ScriptValue, 1> arguments{};
    arguments[0] = ScriptValue::Int32(41);
    std::array<ScriptValue, 1> results = MakeSingleIntResult(0);
    const ScriptStatus status = registry.Invoke(CALL_FAILING, arguments.data(), 1U, results.data(), 1U);
    if (status != ScriptStatus::NativeCallFailed) {
        return Fail("native failure did not return explicit status");
    }

    if (results[0].AsInt32() != 42) {
        return Fail("native failure did not preserve native-written result slot");
    }

    if (registry.Snapshot().failed_call_count != 1U) {
        return Fail("native failure did not record failed call count");
    }

    return 0;
}

int ScriptCallPathDoesNotGrowRegistryStorage() {
    ScriptNativeRegistry registry;
    const ScriptNativeRegistrationResult registration = RegisterAddBinding(registry);
    if (!registration.Succeeded()) {
        return Fail("call path capacity fixture registration failed");
    }

    const ScriptSnapshot before_snapshot = registry.Snapshot();
    std::array<ScriptValue, 2> arguments = MakeAddArguments(1, 2);
    std::array<ScriptValue, 1> results = MakeSingleIntResult(0);
    const ScriptStatus first_status = registry.Invoke(CALL_ADD, arguments.data(), 2U, results.data(), 1U);
    const ScriptStatus second_status = registry.Invoke(CALL_ADD, arguments.data(), 2U, results.data(), 1U);
    if (first_status != ScriptStatus::Success) {
        return Fail("first call path fixture invoke failed");
    }

    if (second_status != ScriptStatus::Success) {
        return Fail("second call path fixture invoke failed");
    }

    const ScriptSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.binding_capacity != before_snapshot.binding_capacity) {
        return Fail("call path mutated binding capacity");
    }

    if (after_snapshot.binding_count != before_snapshot.binding_count) {
        return Fail("call path mutated binding count");
    }

    return 0;
}

int ScriptNoWorldResourcePackageOrGameAdapterDependency() {
    ScriptNativeRegistry registry;
    const ScriptSnapshot snapshot = registry.Snapshot();
    if (snapshot.binding_capacity != MAX_SCRIPT_NATIVE_BINDING_COUNT) {
        return Fail("script registry default capacity was not self-contained");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("script registry did not expose YuMemory accounting vocabulary");
    }

    return 0;
}

int ScriptNoHiddenAllocationUsesYuMemorySignal() {
    ScriptNativeRegistry registry;
    const ScriptSnapshot before_snapshot = registry.Snapshot();
    if (before_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("script registry initial accounting signal was wrong");
    }

    const ScriptNativeRegistrationResult registration = RegisterAddBinding(registry);
    if (!registration.Succeeded()) {
        return Fail("allocation signal fixture registration failed");
    }

    std::array<ScriptValue, 2> arguments = MakeAddArguments(3, 4);
    std::array<ScriptValue, 1> results = MakeSingleIntResult(0);
    const ScriptStatus status = registry.Invoke(CALL_ADD, arguments.data(), 2U, results.data(), 1U);
    if (status != ScriptStatus::Success) {
        return Fail("allocation signal fixture invoke failed");
    }

    const ScriptSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.binding_capacity != before_snapshot.binding_capacity) {
        return Fail("script registry changed binding capacity during call path");
    }

    if (after_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("script registry changed allocation accounting vocabulary");
    }

    return 0;
}

int ScriptSnapshotReportsCountsAndLastStatus() {
    ScriptNativeRegistry registry;
    const ScriptNativeRegistrationResult registration = RegisterAddBinding(registry);
    if (!registration.Succeeded()) {
        return Fail("snapshot fixture registration failed");
    }

    std::array<ScriptValue, 2> arguments = MakeAddArguments(5, 6);
    std::array<ScriptValue, 1> results = MakeSingleIntResult(0);
    const ScriptStatus success_status = registry.Invoke(CALL_ADD, arguments.data(), 2U, results.data(), 1U);
    if (success_status != ScriptStatus::Success) {
        return Fail("snapshot success invoke failed");
    }

    const ScriptStatus failure_status = registry.Invoke(CALL_UNKNOWN, nullptr, 0U, results.data(), 1U);
    if (failure_status != ScriptStatus::InvalidCallId) {
        return Fail("snapshot failure invoke did not return explicit status");
    }

    const ScriptSnapshot snapshot = registry.Snapshot();
    if (snapshot.binding_capacity != MAX_SCRIPT_NATIVE_BINDING_COUNT) {
        return Fail("snapshot did not report binding capacity");
    }

    if (snapshot.binding_count != 1U) {
        return Fail("snapshot did not report binding count");
    }

    if (snapshot.successful_call_count != 1U) {
        return Fail("snapshot did not report successful call count");
    }

    if (snapshot.failed_call_count != 1U) {
        return Fail("snapshot did not report failed call count");
    }

    if (snapshot.last_required_binding_count != 0U) {
        return Fail("snapshot retained required binding count after non-capacity status");
    }

    if (snapshot.last_status != ScriptStatus::InvalidCallId) {
        return Fail("snapshot did not report last status");
    }

    return 0;
}

int ScriptRuntimePhaseDispatchTraceMapsToCallIds() {
    ScriptNativeRegistry registry;
    const ScriptNativeRegistrationResult begin_registration =
        RegisterRuntimeBinding(registry, CALL_RUNTIME_BEGIN, AddOneRuntimeNative);
    if (!begin_registration.Succeeded()) {
        return Fail("runtime begin binding registration failed");
    }

    const ScriptNativeRegistrationResult end_registration =
        RegisterRuntimeBinding(registry, CALL_RUNTIME_END, AddTenRuntimeNative);
    if (!end_registration.Succeeded()) {
        return Fail("runtime end binding registration failed");
    }

    ScriptRuntimePhaseDispatchAdapter adapter;
    const ScriptRuntimePhaseDispatchResult begin_bind =
        adapter.Bind(ScriptRuntimePhase::BeginFrame, CALL_RUNTIME_BEGIN);
    if (!begin_bind.Succeeded()) {
        return Fail("runtime begin phase bind failed");
    }

    const ScriptRuntimePhaseDispatchResult end_bind =
        adapter.Bind(ScriptRuntimePhase::EndFrame, CALL_RUNTIME_END);
    if (!end_bind.Succeeded()) {
        return Fail("runtime end phase bind failed");
    }

    const std::array<ScriptRuntimePhaseTrace, 3U> traces{
        RuntimeTrace(ScriptRuntimePhase::BeginFrame, 7U),
        RuntimeTrace(ScriptRuntimePhase::FixedStep, 7U),
        RuntimeTrace(ScriptRuntimePhase::EndFrame, 7U)};
    std::array<ScriptValue, 1U> results = MakeRuntimeResult(0U);
    const ScriptRuntimePhaseDispatchStatus status = adapter.DispatchTrace(
        registry,
        traces.data(),
        static_cast<std::uint32_t>(traces.size()),
        nullptr,
        0U,
        results.data(),
        static_cast<std::uint32_t>(results.size()));
    if (status != ScriptRuntimePhaseDispatchStatus::Success) {
        return Fail("runtime phase dispatch failed");
    }

    if (results[0].AsUInt32() != 11U) {
        return Fail("runtime phase dispatch did not map call ids in trace order");
    }

    const ScriptRuntimePhaseDispatchSnapshot snapshot = adapter.Snapshot();
    if (snapshot.dispatched_call_count != 2ULL) {
        return Fail("runtime phase dispatch count mismatch");
    }

    if (snapshot.skipped_phase_count != 1ULL) {
        return Fail("runtime phase skipped count mismatch");
    }

    if (snapshot.binding_count != 2U) {
        return Fail("runtime phase binding count mismatch");
    }

    return 0;
}

int ScriptRuntimePhaseDispatchMissingCallReturnsExplicitStatus() {
    ScriptNativeRegistry registry;
    ScriptRuntimePhaseDispatchAdapter adapter;
    const ScriptRuntimePhaseDispatchResult bind_result =
        adapter.Bind(ScriptRuntimePhase::BeginFrame, CALL_UNKNOWN);
    if (!bind_result.Succeeded()) {
        return Fail("missing call fixture bind failed");
    }

    const std::array<ScriptRuntimePhaseTrace, 1U> traces{
        RuntimeTrace(ScriptRuntimePhase::BeginFrame, 8U)};
    std::array<ScriptValue, 1U> results = MakeRuntimeResult(0U);
    const ScriptRuntimePhaseDispatchStatus status = adapter.DispatchTrace(
        registry,
        traces.data(),
        static_cast<std::uint32_t>(traces.size()),
        nullptr,
        0U,
        results.data(),
        static_cast<std::uint32_t>(results.size()));
    if (status != ScriptRuntimePhaseDispatchStatus::MissingCall) {
        return Fail("missing script call did not return explicit dispatch status");
    }

    const ScriptRuntimePhaseDispatchSnapshot snapshot = adapter.Snapshot();
    if (snapshot.last_script_status != ScriptStatus::InvalidCallId) {
        return Fail("missing script call did not preserve script status");
    }

    if (snapshot.failed_dispatch_count != 1ULL) {
        return Fail("missing script call did not record failed dispatch");
    }

    return 0;
}

int ScriptRuntimePhaseDispatchInvalidSlotsReturnExplicitStatus() {
    ScriptNativeRegistry registry;
    const ScriptNativeRegistrationResult registration =
        RegisterRuntimeBinding(registry, CALL_RUNTIME_BEGIN, AddOneRuntimeNative);
    if (!registration.Succeeded()) {
        return Fail("invalid slot fixture registration failed");
    }

    ScriptRuntimePhaseDispatchAdapter adapter;
    const ScriptRuntimePhaseDispatchResult bind_result =
        adapter.Bind(ScriptRuntimePhase::BeginFrame, CALL_RUNTIME_BEGIN);
    if (!bind_result.Succeeded()) {
        return Fail("invalid slot fixture bind failed");
    }

    const std::array<ScriptRuntimePhaseTrace, 1U> traces{
        RuntimeTrace(ScriptRuntimePhase::BeginFrame, 9U)};
    std::array<ScriptValue, 1U> results = MakeSingleIntResult(0);
    const ScriptRuntimePhaseDispatchStatus status = adapter.DispatchTrace(
        registry,
        traces.data(),
        static_cast<std::uint32_t>(traces.size()),
        nullptr,
        0U,
        results.data(),
        static_cast<std::uint32_t>(results.size()));
    if (status != ScriptRuntimePhaseDispatchStatus::InvalidScriptSlot) {
        return Fail("invalid script slot did not return explicit dispatch status");
    }

    const ScriptRuntimePhaseDispatchSnapshot snapshot = adapter.Snapshot();
    if (snapshot.last_script_status != ScriptStatus::ResultTypeMismatch) {
        return Fail("invalid script slot did not preserve script status");
    }

    if (snapshot.failed_dispatch_count != 1ULL) {
        return Fail("invalid script slot did not record failed dispatch");
    }

    return 0;
}

int ScriptRuntimePhaseDispatchInvalidPhaseDoesNotMutate() {
    ScriptRuntimePhaseDispatchAdapter adapter;
    const std::array<ScriptRuntimePhaseTrace, 1U> traces{
        RuntimeTrace(static_cast<ScriptRuntimePhase>(99), 10U)};
    std::array<ScriptValue, 1U> results = MakeRuntimeResult(0U);
    ScriptNativeRegistry registry;

    const ScriptRuntimePhaseDispatchStatus status = adapter.DispatchTrace(
        registry,
        traces.data(),
        static_cast<std::uint32_t>(traces.size()),
        nullptr,
        0U,
        results.data(),
        static_cast<std::uint32_t>(results.size()));
    if (status != ScriptRuntimePhaseDispatchStatus::InvalidPhase) {
        return Fail("invalid runtime phase did not return explicit status");
    }

    const ScriptRuntimePhaseDispatchSnapshot snapshot = adapter.Snapshot();
    if (snapshot.binding_count != 0U) {
        return Fail("invalid runtime phase mutated binding count");
    }

    if (snapshot.dispatched_call_count != 0ULL) {
        return Fail("invalid runtime phase dispatched calls");
    }

    if (snapshot.skipped_phase_count != 0ULL) {
        return Fail("invalid runtime phase skipped phases");
    }

    if (snapshot.failed_dispatch_count != 1ULL) {
        return Fail("invalid runtime phase did not record failed dispatch");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_REGISTER_STABLE_ID, ScriptRegisterNativeCallReturnsStableId},
        {TEST_DUPLICATE, ScriptRegisterDuplicateCallReturnsExplicitStatus},
        {TEST_NULL_FUNCTION, ScriptRegisterRejectsNullFunction},
        {TEST_CAPACITY, ScriptRegistryCapacityOverflowDoesNotMutate},
        {TEST_INVOKE_RESULT, ScriptInvokeNativeCallWritesResultDeterministically},
        {TEST_UNKNOWN_CALL, ScriptInvokeRejectsUnknownCallId},
        {TEST_ARGUMENT_COUNT, ScriptInvokeRejectsArgumentCountMismatch},
        {TEST_ARGUMENT_TYPE, ScriptInvokeRejectsArgumentTypeMismatch},
        {TEST_RESULT_COUNT, ScriptInvokeRejectsResultCountMismatch},
        {TEST_NATIVE_FAILURE, ScriptNativeFailureReturnsExplicitStatus},
        {TEST_CALL_PATH_CAPACITY, ScriptCallPathDoesNotGrowRegistryStorage},
        {TEST_NO_FORBIDDEN_DEPENDENCY, ScriptNoWorldResourcePackageOrGameAdapterDependency},
        {TEST_NO_HIDDEN_ALLOCATION, ScriptNoHiddenAllocationUsesYuMemorySignal},
        {TEST_SNAPSHOT, ScriptSnapshotReportsCountsAndLastStatus},
        {TEST_RUNTIME_PHASE_TRACE, ScriptRuntimePhaseDispatchTraceMapsToCallIds},
        {TEST_RUNTIME_PHASE_MISSING_CALL, ScriptRuntimePhaseDispatchMissingCallReturnsExplicitStatus},
        {TEST_RUNTIME_PHASE_INVALID_SLOTS, ScriptRuntimePhaseDispatchInvalidSlotsReturnExplicitStatus},
        {TEST_RUNTIME_PHASE_INVALID_PHASE, ScriptRuntimePhaseDispatchInvalidPhaseDoesNotMutate}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
