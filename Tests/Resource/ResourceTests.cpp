// Module: Tests Resource
// File: Tests/Resource/ResourceTests.cpp

#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceRegistry.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::resource::ResourceDescriptor;
using yuengine::resource::ResourceHandle;
using ResourceLogicalKey = yuengine::resource::ResourceLogicalKey;
using ResourceRegistry = yuengine::resource::ResourceRegistry;
using yuengine::resource::ResourceRegistryDesc;
using yuengine::resource::ResourceRegistrationResult;
using yuengine::resource::ResourceSnapshot;
using yuengine::resource::ResourceStatus;
using yuengine::resource::ResourceTypeId;
using yuengine::resource::INVALID_RESOURCE_GENERATION;

namespace {
constexpr const char* TEST_REGISTER = "Resource_RegisterSyntheticDescriptor_ReturnsGenerationHandle";
constexpr const char* TEST_DUPLICATE = "Resource_RegisterDuplicate_ReturnsExplicitStatus";
constexpr const char* TEST_CAPACITY = "Resource_RegistryRejectsCapacityOverflowWithoutMutation";
constexpr const char* TEST_TYPE_CAPACITY = "Resource_TypeCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_WRONG_GENERATION = "Resource_HandleRejectsWrongGeneration";
constexpr const char* TEST_TYPE_MISMATCH = "Resource_HandleRejectsTypeMismatch";
constexpr const char* TEST_ACQUIRE_RELEASE = "Resource_AcquireRelease_TracksReferenceCount";
constexpr const char* TEST_REPEATED_ACQUIRE = "Resource_RepeatedAcquire_IncrementsReferenceCount";
constexpr const char* TEST_REFERENCE_OVERFLOW = "Resource_AcquireRejectsReferenceCountOverflow";
constexpr const char *TEST_VALIDATE_ACQUIRE =
    "Resource_ValidateAcquireChecksHandleTypeAndOverflowWithoutMutation";
constexpr const char *TEST_VALIDATE_ACQUIRE_WORLD_FREE =
    "Resource_ValidateAcquireDoesNotRequireWorld";
constexpr const char* TEST_RETIRE_REFERENCED = "Resource_RetireRejectsOutstandingAcquire";
constexpr const char* TEST_RETIRE_DEPENDED_ON = "Resource_RetireRejectsLiveDependentEdge";
constexpr const char* TEST_MISSING_DEPENDENCY = "Resource_DependencyValidationRejectsMissingDependency";
constexpr const char* TEST_DEPENDENCY_CYCLE = "Resource_DependencyValidationRejectsCycle";
constexpr const char* TEST_NO_FILE_PACKAGE = "Resource_NoFileOrPackageDependency_ForHandleRegistry";
constexpr const char* TEST_DISABLED_DIAGNOSTICS = "Resource_DisabledDiagnosticsDoesNotChangeResults";
constexpr const char* TEST_NO_HIDDEN_ALLOCATION = "Resource_NoHiddenAllocation_UsesYuMemorySignal";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr ResourceTypeId TYPE_TEXTURE{1U};
constexpr ResourceTypeId TYPE_MATERIAL{2U};
constexpr ResourceTypeId TYPE_AUDIO{3U};
constexpr ResourceTypeId TYPE_EFFECT{4U};
constexpr std::uint32_t INVALID_GENERATION = INVALID_RESOURCE_GENERATION;
constexpr const char* TYPE_CAPACITY_TEXTURE_KEY = "texture_a";
constexpr const char* TYPE_CAPACITY_MATERIAL_KEY = "material_a";
constexpr const char* TYPE_CAPACITY_RETRY_TEXTURE_KEY = "texture_b";
constexpr const char* TYPE_CAPACITY_FIRST_REGISTRATION_FAILED = "first registration failed before type capacity";
constexpr const char* TYPE_CAPACITY_STATUS_FAILED = "type capacity overflow did not return explicit status";
constexpr const char* TYPE_CAPACITY_VALID_HANDLE_FAILED = "type capacity overflow returned a valid handle";
constexpr const char* TYPE_CAPACITY_TYPE_COUNT_FAILED = "type capacity overflow changed type count";
constexpr const char* TYPE_CAPACITY_REGISTERED_COUNT_FAILED = "type capacity overflow changed registered count";
constexpr const char* TYPE_CAPACITY_ACQUIRED_COUNT_FAILED = "type capacity overflow changed acquired count";
constexpr const char* TYPE_CAPACITY_RETRY_REGISTRATION_FAILED = "retry after type capacity failure did not register existing type";
constexpr const char* TYPE_CAPACITY_RETRY_SLOT_FAILED = "retry after type capacity failure used wrong slot";
constexpr const char* TYPE_CAPACITY_RETRY_GENERATION_FAILED = "type capacity failure advanced retry slot generation";
constexpr const char* TYPE_CAPACITY_RETRY_TYPE_COUNT_FAILED = "retry with existing type changed type count";
using TestFunction = int (*)();
using TestRegistry = std::unordered_map<std::string_view, TestFunction>;

int Fail(const std::string& message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

ResourceDescriptor Descriptor(ResourceTypeId type, const char* key) {
    return ResourceDescriptor{type, ResourceLogicalKey(key), 0U};
}

ResourceDescriptor DescriptorWithReferenceCount(ResourceTypeId type, const char* key, std::uint32_t reference_count) {
    return ResourceDescriptor{type, ResourceLogicalKey(key), reference_count};
}

ResourceRegistrationResult Register(ResourceRegistry& registry, ResourceTypeId type, const char* key) {
    return registry.RegisterSyntheticDescriptor(Descriptor(type, key));
}

bool SnapshotsMatch(const ResourceSnapshot& left, const ResourceSnapshot& right) {
    if (left.registered_resource_count != right.registered_resource_count) {
        return false;
    }

    if (left.acquired_handle_count != right.acquired_handle_count) {
        return false;
    }

    if (left.released_handle_count != right.released_handle_count) {
        return false;
    }

    if (left.dependency_edge_count != right.dependency_edge_count) {
        return false;
    }

    if (left.failed_operation_count != right.failed_operation_count) {
        return false;
    }

    return left.allocation_accounting_status == right.allocation_accounting_status;
}

int ResourceRegisterSyntheticDescriptorReturnsGenerationHandle() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("synthetic descriptor did not register");
    }

    if (result.handle.slot != 0U) {
        return Fail("first resource used unexpected slot");
    }

    if (result.handle.generation == INVALID_GENERATION) {
        return Fail("resource handle generation was invalid");
    }

    const ResourceSnapshot snapshot = registry.Snapshot();
    if (snapshot.registered_resource_count != 1U) {
        return Fail("registered resource count was not recorded");
    }

    if (snapshot.type_count != 1U) {
        return Fail("resource type count was not recorded");
    }

    if (snapshot.last_status != ResourceStatus::Success) {
        return Fail("successful registration did not record success status");
    }

    return 0;
}

int ResourceRegisterDuplicateReturnsExplicitStatus() {
    ResourceRegistry registry;
    const ResourceRegistrationResult first_result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!first_result.Succeeded()) {
        return Fail("first registration failed");
    }

    const ResourceSnapshot before_snapshot = registry.Snapshot();
    const ResourceRegistrationResult duplicate_result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (duplicate_result.status != ResourceStatus::DuplicateResource) {
        return Fail("duplicate resource did not return explicit status");
    }

    const ResourceSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.registered_resource_count != before_snapshot.registered_resource_count) {
        return Fail("duplicate resource changed registered count");
    }

    if (after_snapshot.type_count != before_snapshot.type_count) {
        return Fail("duplicate resource changed type count");
    }

    return 0;
}

int ResourceRegistryRejectsCapacityOverflowWithoutMutation() {
    ResourceRegistry registry(ResourceRegistryDesc{1U, 2U, 2U});
    const ResourceRegistrationResult first_result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!first_result.Succeeded()) {
        return Fail("first registration failed before capacity");
    }

    const ResourceSnapshot before_snapshot = registry.Snapshot();
    const ResourceRegistrationResult overflow_result = Register(registry, TYPE_MATERIAL, "material_a");
    if (overflow_result.status != ResourceStatus::CapacityExceeded) {
        return Fail("resource capacity overflow did not return explicit status");
    }

    const ResourceSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.registered_resource_count != before_snapshot.registered_resource_count) {
        return Fail("resource capacity overflow changed registered count");
    }

    if (after_snapshot.type_count != before_snapshot.type_count) {
        return Fail("resource capacity overflow changed type count");
    }

    return 0;
}

int ResourceTypeCapacityOverflowDoesNotMutate() {
    ResourceRegistry registry(ResourceRegistryDesc{4U, 1U, 2U});
    const ResourceRegistrationResult first_result = Register(registry, TYPE_TEXTURE, TYPE_CAPACITY_TEXTURE_KEY);
    if (!first_result.Succeeded()) {
        return Fail(TYPE_CAPACITY_FIRST_REGISTRATION_FAILED);
    }

    const ResourceSnapshot before_snapshot = registry.Snapshot();
    const ResourceRegistrationResult overflow_result = Register(registry, TYPE_MATERIAL, TYPE_CAPACITY_MATERIAL_KEY);
    if (overflow_result.status != ResourceStatus::CapacityExceeded) {
        return Fail(TYPE_CAPACITY_STATUS_FAILED);
    }

    if (overflow_result.handle.IsValid()) {
        return Fail(TYPE_CAPACITY_VALID_HANDLE_FAILED);
    }

    const ResourceSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.type_count != before_snapshot.type_count) {
        return Fail(TYPE_CAPACITY_TYPE_COUNT_FAILED);
    }

    if (after_snapshot.registered_resource_count != before_snapshot.registered_resource_count) {
        return Fail(TYPE_CAPACITY_REGISTERED_COUNT_FAILED);
    }

    if (after_snapshot.acquired_handle_count != before_snapshot.acquired_handle_count) {
        return Fail(TYPE_CAPACITY_ACQUIRED_COUNT_FAILED);
    }

    const ResourceRegistrationResult retry_result = Register(registry, TYPE_TEXTURE, TYPE_CAPACITY_RETRY_TEXTURE_KEY);
    if (!retry_result.Succeeded()) {
        return Fail(TYPE_CAPACITY_RETRY_REGISTRATION_FAILED);
    }

    if (retry_result.handle.slot != 1U) {
        return Fail(TYPE_CAPACITY_RETRY_SLOT_FAILED);
    }

    if (retry_result.handle.generation != 1U) {
        return Fail(TYPE_CAPACITY_RETRY_GENERATION_FAILED);
    }

    if (registry.Snapshot().type_count != before_snapshot.type_count) {
        return Fail(TYPE_CAPACITY_RETRY_TYPE_COUNT_FAILED);
    }

    return 0;
}

int ResourceHandleRejectsWrongGeneration() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    const ResourceStatus retire_status = registry.Retire(result.handle);
    if (retire_status != ResourceStatus::Success) {
        return Fail("retire failed before stale handle check");
    }

    const ResourceStatus acquire_status = registry.Acquire(result.handle, TYPE_TEXTURE);
    if (acquire_status != ResourceStatus::GenerationMismatch) {
        return Fail("stale generation handle did not return explicit status");
    }

    if (registry.Snapshot().acquired_handle_count != 0U) {
        return Fail("stale handle acquire changed reference count");
    }

    return 0;
}

int ResourceHandleRejectsTypeMismatch() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    const ResourceSnapshot before_snapshot = registry.Snapshot();
    const ResourceStatus status = registry.Acquire(result.handle, TYPE_AUDIO);
    if (status != ResourceStatus::TypeMismatch) {
        return Fail("type mismatch did not return explicit status");
    }

    if (registry.Snapshot().acquired_handle_count != before_snapshot.acquired_handle_count) {
        return Fail("type mismatch changed reference count");
    }

    return 0;
}

int ResourceAcquireReleaseTracksReferenceCount() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    if (registry.Acquire(result.handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("acquire failed");
    }

    if (registry.Snapshot().acquired_handle_count != 1U) {
        return Fail("acquire did not increment reference count");
    }

    if (registry.Release(result.handle) != ResourceStatus::Success) {
        return Fail("release failed");
    }

    const ResourceSnapshot after_release_snapshot = registry.Snapshot();
    if (after_release_snapshot.acquired_handle_count != 0U) {
        return Fail("release did not decrement reference count");
    }

    if (after_release_snapshot.released_handle_count != 1U) {
        return Fail("release count was not recorded");
    }

    if (registry.Release(result.handle) != ResourceStatus::NotAcquired) {
        return Fail("release at zero did not return not-acquired status");
    }

    if (registry.Snapshot().acquired_handle_count != 0U) {
        return Fail("release at zero mutated reference count");
    }

    return 0;
}

int ResourceRepeatedAcquireIncrementsReferenceCount() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    if (registry.Acquire(result.handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("first acquire failed");
    }

    if (registry.Acquire(result.handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("second acquire failed");
    }

    if (registry.Snapshot().acquired_handle_count != 2U) {
        return Fail("repeated acquire did not increment reference count");
    }

    return 0;
}

int ResourceAcquireRejectsReferenceCountOverflow() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = registry.RegisterSyntheticDescriptor(
        DescriptorWithReferenceCount(TYPE_TEXTURE, "texture_a", std::numeric_limits<std::uint32_t>::max()));
    if (!result.Succeeded()) {
        return Fail("overflow fixture registration failed");
    }

    const ResourceSnapshot before_snapshot = registry.Snapshot();
    const ResourceStatus status = registry.Acquire(result.handle, TYPE_TEXTURE);
    if (status != ResourceStatus::ReferenceCountOverflow) {
        return Fail("reference count overflow did not return explicit status");
    }

    if (registry.Snapshot().acquired_handle_count != before_snapshot.acquired_handle_count) {
        return Fail("reference count overflow changed acquired count");
    }

    return 0;
}

int ResourceValidateAcquireChecksHandleTypeAndOverflowWithoutMutation() {
    ResourceRegistry registry;
    const std::uint32_t max_reference_count = std::numeric_limits<std::uint32_t>::max();
    const std::uint32_t initial_reference_count = max_reference_count - 1U;
    const ResourceRegistrationResult result = registry.RegisterSyntheticDescriptor(
        DescriptorWithReferenceCount(TYPE_TEXTURE, "texture_a", initial_reference_count));
    if (!result.Succeeded()) {
        return Fail("validate acquire fixture registration failed");
    }

    const ResourceSnapshot before_success_snapshot = registry.Snapshot();
    const ResourceStatus success_status = registry.ValidateAcquire(result.handle, TYPE_TEXTURE, 1U);
    if (success_status != ResourceStatus::Success) {
        return Fail("validate acquire rejected valid projected acquire");
    }

    if (!SnapshotsMatch(before_success_snapshot, registry.Snapshot())) {
        return Fail("validate acquire success mutated registry");
    }

    const ResourceSnapshot before_type_snapshot = registry.Snapshot();
    const ResourceStatus type_status = registry.ValidateAcquire(result.handle, TYPE_AUDIO, 1U);
    if (type_status != ResourceStatus::TypeMismatch) {
        return Fail("validate acquire type mismatch returned wrong status");
    }

    if (!SnapshotsMatch(before_type_snapshot, registry.Snapshot())) {
        return Fail("validate acquire type mismatch mutated registry");
    }

    const ResourceSnapshot before_overflow_snapshot = registry.Snapshot();
    const ResourceStatus overflow_status = registry.ValidateAcquire(result.handle, TYPE_TEXTURE, 2U);
    if (overflow_status != ResourceStatus::ReferenceCountOverflow) {
        return Fail("validate acquire overflow returned wrong status");
    }

    if (!SnapshotsMatch(before_overflow_snapshot, registry.Snapshot())) {
        return Fail("validate acquire overflow mutated registry");
    }

    return 0;
}

int ResourceValidateAcquireDoesNotRequireWorld() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_world_free");
    if (!result.Succeeded()) {
        return Fail("validate acquire world-free registration failed");
    }

    const ResourceSnapshot before_validate_snapshot = registry.Snapshot();
    const ResourceStatus validate_status = registry.ValidateAcquire(result.handle, TYPE_TEXTURE, 1U);
    if (validate_status != ResourceStatus::Success) {
        return Fail("validate acquire world-free validation failed");
    }

    if (!SnapshotsMatch(before_validate_snapshot, registry.Snapshot())) {
        return Fail("validate acquire world-free mutated registry");
    }

    if (registry.Acquire(result.handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("validate acquire world-free acquire failed");
    }

    if (registry.Release(result.handle) != ResourceStatus::Success) {
        return Fail("validate acquire world-free release failed");
    }

    return 0;
}

int ResourceRetireRejectsOutstandingAcquire() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    if (registry.Acquire(result.handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("acquire failed before retire");
    }

    const ResourceStatus retire_status = registry.Retire(result.handle);
    if (retire_status != ResourceStatus::StillReferenced) {
        return Fail("retire with outstanding acquire did not return explicit status");
    }

    if (registry.Snapshot().registered_resource_count != 1U) {
        return Fail("rejected retire changed registered count");
    }

    return 0;
}

int ResourceRetireRejectsLiveDependentEdge() {
    ResourceRegistry registry;
    const ResourceRegistrationResult dependency = Register(registry, TYPE_TEXTURE, "texture_a");
    const ResourceRegistrationResult dependent = Register(registry, TYPE_MATERIAL, "material_a");
    if (!dependency.Succeeded()) {
        return Fail("dependency registration failed");
    }

    if (!dependent.Succeeded()) {
        return Fail("dependent registration failed");
    }

    if (registry.AddDependency(dependent.handle, dependency.handle) != ResourceStatus::Success) {
        return Fail("dependency edge registration failed");
    }

    if (registry.Retire(dependency.handle) != ResourceStatus::StillDependedOn) {
        return Fail("retire with live dependent edge did not return explicit status");
    }

    if (registry.Snapshot().dependency_edge_count != 1U) {
        return Fail("rejected dependency retire changed edge count");
    }

    if (registry.Retire(dependent.handle) != ResourceStatus::Success) {
        return Fail("dependent retire failed");
    }

    if (registry.Snapshot().dependency_edge_count != 0U) {
        return Fail("successful retire did not clear outbound dependency edge");
    }

    if (registry.Retire(dependency.handle) != ResourceStatus::Success) {
        return Fail("dependency retire failed after outbound edge cleared");
    }

    return 0;
}

int ResourceDependencyValidationRejectsMissingDependency() {
    ResourceRegistry registry;
    const ResourceRegistrationResult dependent = Register(registry, TYPE_MATERIAL, "material_a");
    if (!dependent.Succeeded()) {
        return Fail("dependent registration failed");
    }

    const ResourceStatus status = registry.AddDependency(dependent.handle, ResourceHandle{31U, 1U});
    if (status != ResourceStatus::DependencyMissing) {
        return Fail("missing dependency did not return explicit status");
    }

    const ResourceSnapshot snapshot = registry.Snapshot();
    if (snapshot.dependency_edge_count != 0U) {
        return Fail("missing dependency changed edge count");
    }

    if (snapshot.dependency_validation_count != 1U) {
        return Fail("missing dependency validation was not counted");
    }

    return 0;
}

int ResourceDependencyValidationRejectsCycle() {
    ResourceRegistry registry;
    const ResourceRegistrationResult first = Register(registry, TYPE_TEXTURE, "texture_a");
    const ResourceRegistrationResult second = Register(registry, TYPE_MATERIAL, "material_a");
    const ResourceRegistrationResult third = Register(registry, TYPE_EFFECT, "effect_a");
    if (!first.Succeeded()) {
        return Fail("first registration failed");
    }

    if (!second.Succeeded()) {
        return Fail("second registration failed");
    }

    if (!third.Succeeded()) {
        return Fail("third registration failed");
    }

    if (registry.AddDependency(first.handle, first.handle) != ResourceStatus::DependencyCycle) {
        return Fail("self dependency did not return cycle status");
    }

    if (registry.AddDependency(first.handle, second.handle) != ResourceStatus::Success) {
        return Fail("first dependency edge failed");
    }

    if (registry.AddDependency(second.handle, third.handle) != ResourceStatus::Success) {
        return Fail("second dependency edge failed");
    }

    const ResourceStatus cycle_status = registry.AddDependency(third.handle, first.handle);
    if (cycle_status != ResourceStatus::DependencyCycle) {
        return Fail("dependency cycle did not return explicit status");
    }

    if (registry.Snapshot().dependency_edge_count != 2U) {
        return Fail("dependency cycle changed edge count");
    }

    return 0;
}

int ResourceNoFileOrPackageDependencyForHandleRegistry() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "synthetic_texture");
    if (!result.Succeeded()) {
        return Fail("synthetic resource registration failed");
    }

    if (registry.Acquire(result.handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("synthetic handle acquire failed");
    }

    if (registry.Release(result.handle) != ResourceStatus::Success) {
        return Fail("synthetic handle release failed");
    }

    if (registry.Snapshot().registered_resource_count != 1U) {
        return Fail("synthetic handle registry left resource scope");
    }

    return 0;
}

int ResourceDisabledDiagnosticsDoesNotChangeResults() {
    ResourceRegistry recording_registry;
    ResourceRegistry diagnostics_disabled_registry;
    const ResourceRegistrationResult recording_result = Register(recording_registry, TYPE_TEXTURE, "texture_a");
    const ResourceRegistrationResult disabled_result = Register(diagnostics_disabled_registry, TYPE_TEXTURE, "texture_a");
    if (!recording_result.Succeeded()) {
        return Fail("recording registry registration failed");
    }

    if (!disabled_result.Succeeded()) {
        return Fail("disabled registry registration failed");
    }

    const ResourceStatus recording_acquire = recording_registry.Acquire(recording_result.handle, TYPE_TEXTURE);
    const ResourceStatus disabled_acquire = diagnostics_disabled_registry.Acquire(disabled_result.handle, TYPE_TEXTURE);
    if (recording_acquire != disabled_acquire) {
        return Fail("disabled diagnostics changed acquire status");
    }

    const ResourceStatus recording_mismatch = recording_registry.Acquire(recording_result.handle, TYPE_AUDIO);
    const ResourceStatus disabled_mismatch = diagnostics_disabled_registry.Acquire(disabled_result.handle, TYPE_AUDIO);
    if (recording_mismatch != disabled_mismatch) {
        return Fail("disabled diagnostics changed failure status");
    }

    if (!SnapshotsMatch(recording_registry.Snapshot(), diagnostics_disabled_registry.Snapshot())) {
        return Fail("disabled diagnostics changed resource snapshot");
    }

    return 0;
}

int ResourceNoHiddenAllocationUsesYuMemorySignal() {
    ResourceRegistry registry;
    const ResourceSnapshot initial_snapshot = registry.Snapshot();
    if (initial_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("resource registry did not expose YuMemory accounting vocabulary");
    }

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    if (registry.Acquire(result.handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("acquire failed");
    }

    if (registry.Release(result.handle) != ResourceStatus::Success) {
        return Fail("release failed");
    }

    const ResourceSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.resource_capacity != initial_snapshot.resource_capacity) {
        return Fail("resource capacity changed during handle fixture");
    }

    if (after_snapshot.dependency_edge_capacity != initial_snapshot.dependency_edge_capacity) {
        return Fail("dependency edge capacity changed during handle fixture");
    }

    if (after_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("resource registry changed allocation accounting vocabulary");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const TestRegistry test_registry{
        {TEST_REGISTER, ResourceRegisterSyntheticDescriptorReturnsGenerationHandle},
        {TEST_DUPLICATE, ResourceRegisterDuplicateReturnsExplicitStatus},
        {TEST_CAPACITY, ResourceRegistryRejectsCapacityOverflowWithoutMutation},
        {TEST_TYPE_CAPACITY, ResourceTypeCapacityOverflowDoesNotMutate},
        {TEST_WRONG_GENERATION, ResourceHandleRejectsWrongGeneration},
        {TEST_TYPE_MISMATCH, ResourceHandleRejectsTypeMismatch},
        {TEST_ACQUIRE_RELEASE, ResourceAcquireReleaseTracksReferenceCount},
        {TEST_REPEATED_ACQUIRE, ResourceRepeatedAcquireIncrementsReferenceCount},
        {TEST_REFERENCE_OVERFLOW, ResourceAcquireRejectsReferenceCountOverflow},
        {TEST_VALIDATE_ACQUIRE, ResourceValidateAcquireChecksHandleTypeAndOverflowWithoutMutation},
        {TEST_VALIDATE_ACQUIRE_WORLD_FREE, ResourceValidateAcquireDoesNotRequireWorld},
        {TEST_RETIRE_REFERENCED, ResourceRetireRejectsOutstandingAcquire},
        {TEST_RETIRE_DEPENDED_ON, ResourceRetireRejectsLiveDependentEdge},
        {TEST_MISSING_DEPENDENCY, ResourceDependencyValidationRejectsMissingDependency},
        {TEST_DEPENDENCY_CYCLE, ResourceDependencyValidationRejectsCycle},
        {TEST_NO_FILE_PACKAGE, ResourceNoFileOrPackageDependencyForHandleRegistry},
        {TEST_DISABLED_DIAGNOSTICS, ResourceDisabledDiagnosticsDoesNotChangeResults},
        {TEST_NO_HIDDEN_ALLOCATION, ResourceNoHiddenAllocationUsesYuMemorySignal}};

    const std::string_view test_name(argv[1]);
    const auto test_entry = test_registry.find(test_name);
    if (test_entry == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_entry->second();
}
