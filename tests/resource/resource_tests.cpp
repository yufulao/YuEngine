#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>

#include "yuengine/memory/memory_accounting_status.h"
#include "yuengine/resource/resource_constants.h"
#include "yuengine/resource/resource_registry.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::resource::resource_descriptor_t;
using yuengine::resource::resource_handle_t;
using ResourceLogicalKey = yuengine::resource::ResourceLogicalKey;
using ResourceRegistry = yuengine::resource::ResourceRegistry;
using yuengine::resource::resource_registry_desc_t;
using yuengine::resource::resource_registration_result_t;
using yuengine::resource::resource_snapshot_t;
using yuengine::resource::ResourceStatus;
using yuengine::resource::resource_type_id_t;
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
constexpr const char* TEST_RETIRE_REFERENCED = "Resource_RetireRejectsOutstandingAcquire";
constexpr const char* TEST_RETIRE_DEPENDED_ON = "Resource_RetireRejectsLiveDependentEdge";
constexpr const char* TEST_MISSING_DEPENDENCY = "Resource_DependencyValidationRejectsMissingDependency";
constexpr const char* TEST_DEPENDENCY_CYCLE = "Resource_DependencyValidationRejectsCycle";
constexpr const char* TEST_NO_FILE_PACKAGE = "Resource_NoFileOrPackageDependency_ForHandleRegistry";
constexpr const char* TEST_DISABLED_DIAGNOSTICS = "Resource_DisabledDiagnosticsDoesNotChangeResults";
constexpr const char* TEST_NO_HIDDEN_ALLOCATION = "Resource_NoHiddenAllocation_UsesYuMemorySignal";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr resource_type_id_t TYPE_TEXTURE{1U};
constexpr resource_type_id_t TYPE_MATERIAL{2U};
constexpr resource_type_id_t TYPE_AUDIO{3U};
constexpr resource_type_id_t TYPE_EFFECT{4U};
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

resource_descriptor_t Descriptor(resource_type_id_t type, const char* key) {
    return resource_descriptor_t{type, ResourceLogicalKey(key), 0U};
}

resource_descriptor_t DescriptorWithReferenceCount(resource_type_id_t type, const char* key, std::uint32_t referenceCount) {
    return resource_descriptor_t{type, ResourceLogicalKey(key), referenceCount};
}

resource_registration_result_t Register(ResourceRegistry& registry, resource_type_id_t type, const char* key) {
    return registry.RegisterSyntheticDescriptor(Descriptor(type, key));
}

bool SnapshotsMatch(const resource_snapshot_t& left, const resource_snapshot_t& right) {
    if (left.RegisteredResourceCount != right.RegisteredResourceCount) {
        return false;
    }

    if (left.AcquiredHandleCount != right.AcquiredHandleCount) {
        return false;
    }

    if (left.ReleasedHandleCount != right.ReleasedHandleCount) {
        return false;
    }

    if (left.DependencyEdgeCount != right.DependencyEdgeCount) {
        return false;
    }

    if (left.FailedOperationCount != right.FailedOperationCount) {
        return false;
    }

    return left.AllocationAccountingStatus == right.AllocationAccountingStatus;
}

int ResourceRegisterSyntheticDescriptorReturnsGenerationHandle() {
    ResourceRegistry registry;
    const resource_registration_result_t result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("synthetic descriptor did not register");
    }

    if (result.Handle.Slot != 0U) {
        return Fail("first resource used unexpected slot");
    }

    if (result.Handle.Generation == INVALID_GENERATION) {
        return Fail("resource handle generation was invalid");
    }

    const resource_snapshot_t snapshot = registry.Snapshot();
    if (snapshot.RegisteredResourceCount != 1U) {
        return Fail("registered resource count was not recorded");
    }

    if (snapshot.TypeCount != 1U) {
        return Fail("resource type count was not recorded");
    }

    if (snapshot.LastStatus != ResourceStatus::Success) {
        return Fail("successful registration did not record success status");
    }

    return 0;
}

int ResourceRegisterDuplicateReturnsExplicitStatus() {
    ResourceRegistry registry;
    const resource_registration_result_t firstResult = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!firstResult.Succeeded()) {
        return Fail("first registration failed");
    }

    const resource_snapshot_t beforeSnapshot = registry.Snapshot();
    const resource_registration_result_t duplicateResult = Register(registry, TYPE_TEXTURE, "texture_a");
    if (duplicateResult.Status != ResourceStatus::DuplicateResource) {
        return Fail("duplicate resource did not return explicit status");
    }

    const resource_snapshot_t afterSnapshot = registry.Snapshot();
    if (afterSnapshot.RegisteredResourceCount != beforeSnapshot.RegisteredResourceCount) {
        return Fail("duplicate resource changed registered count");
    }

    if (afterSnapshot.TypeCount != beforeSnapshot.TypeCount) {
        return Fail("duplicate resource changed type count");
    }

    return 0;
}

int ResourceRegistryRejectsCapacityOverflowWithoutMutation() {
    ResourceRegistry registry(resource_registry_desc_t{1U, 2U, 2U});
    const resource_registration_result_t firstResult = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!firstResult.Succeeded()) {
        return Fail("first registration failed before capacity");
    }

    const resource_snapshot_t beforeSnapshot = registry.Snapshot();
    const resource_registration_result_t overflowResult = Register(registry, TYPE_MATERIAL, "material_a");
    if (overflowResult.Status != ResourceStatus::CapacityExceeded) {
        return Fail("resource capacity overflow did not return explicit status");
    }

    const resource_snapshot_t afterSnapshot = registry.Snapshot();
    if (afterSnapshot.RegisteredResourceCount != beforeSnapshot.RegisteredResourceCount) {
        return Fail("resource capacity overflow changed registered count");
    }

    if (afterSnapshot.TypeCount != beforeSnapshot.TypeCount) {
        return Fail("resource capacity overflow changed type count");
    }

    return 0;
}

int ResourceTypeCapacityOverflowDoesNotMutate() {
    ResourceRegistry registry(resource_registry_desc_t{4U, 1U, 2U});
    const resource_registration_result_t firstResult = Register(registry, TYPE_TEXTURE, TYPE_CAPACITY_TEXTURE_KEY);
    if (!firstResult.Succeeded()) {
        return Fail(TYPE_CAPACITY_FIRST_REGISTRATION_FAILED);
    }

    const resource_snapshot_t beforeSnapshot = registry.Snapshot();
    const resource_registration_result_t overflowResult = Register(registry, TYPE_MATERIAL, TYPE_CAPACITY_MATERIAL_KEY);
    if (overflowResult.Status != ResourceStatus::CapacityExceeded) {
        return Fail(TYPE_CAPACITY_STATUS_FAILED);
    }

    if (overflowResult.Handle.IsValid()) {
        return Fail(TYPE_CAPACITY_VALID_HANDLE_FAILED);
    }

    const resource_snapshot_t afterSnapshot = registry.Snapshot();
    if (afterSnapshot.TypeCount != beforeSnapshot.TypeCount) {
        return Fail(TYPE_CAPACITY_TYPE_COUNT_FAILED);
    }

    if (afterSnapshot.RegisteredResourceCount != beforeSnapshot.RegisteredResourceCount) {
        return Fail(TYPE_CAPACITY_REGISTERED_COUNT_FAILED);
    }

    if (afterSnapshot.AcquiredHandleCount != beforeSnapshot.AcquiredHandleCount) {
        return Fail(TYPE_CAPACITY_ACQUIRED_COUNT_FAILED);
    }

    const resource_registration_result_t retryResult = Register(registry, TYPE_TEXTURE, TYPE_CAPACITY_RETRY_TEXTURE_KEY);
    if (!retryResult.Succeeded()) {
        return Fail(TYPE_CAPACITY_RETRY_REGISTRATION_FAILED);
    }

    if (retryResult.Handle.Slot != 1U) {
        return Fail(TYPE_CAPACITY_RETRY_SLOT_FAILED);
    }

    if (retryResult.Handle.Generation != 1U) {
        return Fail(TYPE_CAPACITY_RETRY_GENERATION_FAILED);
    }

    if (registry.Snapshot().TypeCount != beforeSnapshot.TypeCount) {
        return Fail(TYPE_CAPACITY_RETRY_TYPE_COUNT_FAILED);
    }

    return 0;
}

int ResourceHandleRejectsWrongGeneration() {
    ResourceRegistry registry;
    const resource_registration_result_t result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    const ResourceStatus retireStatus = registry.Retire(result.Handle);
    if (retireStatus != ResourceStatus::Success) {
        return Fail("retire failed before stale handle check");
    }

    const ResourceStatus acquireStatus = registry.Acquire(result.Handle, TYPE_TEXTURE);
    if (acquireStatus != ResourceStatus::GenerationMismatch) {
        return Fail("stale generation handle did not return explicit status");
    }

    if (registry.Snapshot().AcquiredHandleCount != 0U) {
        return Fail("stale handle acquire changed reference count");
    }

    return 0;
}

int ResourceHandleRejectsTypeMismatch() {
    ResourceRegistry registry;
    const resource_registration_result_t result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    const resource_snapshot_t beforeSnapshot = registry.Snapshot();
    const ResourceStatus status = registry.Acquire(result.Handle, TYPE_AUDIO);
    if (status != ResourceStatus::TypeMismatch) {
        return Fail("type mismatch did not return explicit status");
    }

    if (registry.Snapshot().AcquiredHandleCount != beforeSnapshot.AcquiredHandleCount) {
        return Fail("type mismatch changed reference count");
    }

    return 0;
}

int ResourceAcquireReleaseTracksReferenceCount() {
    ResourceRegistry registry;
    const resource_registration_result_t result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    if (registry.Acquire(result.Handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("acquire failed");
    }

    if (registry.Snapshot().AcquiredHandleCount != 1U) {
        return Fail("acquire did not increment reference count");
    }

    if (registry.Release(result.Handle) != ResourceStatus::Success) {
        return Fail("release failed");
    }

    const resource_snapshot_t afterReleaseSnapshot = registry.Snapshot();
    if (afterReleaseSnapshot.AcquiredHandleCount != 0U) {
        return Fail("release did not decrement reference count");
    }

    if (afterReleaseSnapshot.ReleasedHandleCount != 1U) {
        return Fail("release count was not recorded");
    }

    if (registry.Release(result.Handle) != ResourceStatus::NotAcquired) {
        return Fail("release at zero did not return not-acquired status");
    }

    if (registry.Snapshot().AcquiredHandleCount != 0U) {
        return Fail("release at zero mutated reference count");
    }

    return 0;
}

int ResourceRepeatedAcquireIncrementsReferenceCount() {
    ResourceRegistry registry;
    const resource_registration_result_t result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    if (registry.Acquire(result.Handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("first acquire failed");
    }

    if (registry.Acquire(result.Handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("second acquire failed");
    }

    if (registry.Snapshot().AcquiredHandleCount != 2U) {
        return Fail("repeated acquire did not increment reference count");
    }

    return 0;
}

int ResourceAcquireRejectsReferenceCountOverflow() {
    ResourceRegistry registry;
    const resource_registration_result_t result = registry.RegisterSyntheticDescriptor(
        DescriptorWithReferenceCount(TYPE_TEXTURE, "texture_a", std::numeric_limits<std::uint32_t>::max()));
    if (!result.Succeeded()) {
        return Fail("overflow fixture registration failed");
    }

    const resource_snapshot_t beforeSnapshot = registry.Snapshot();
    const ResourceStatus status = registry.Acquire(result.Handle, TYPE_TEXTURE);
    if (status != ResourceStatus::ReferenceCountOverflow) {
        return Fail("reference count overflow did not return explicit status");
    }

    if (registry.Snapshot().AcquiredHandleCount != beforeSnapshot.AcquiredHandleCount) {
        return Fail("reference count overflow changed acquired count");
    }

    return 0;
}

int ResourceRetireRejectsOutstandingAcquire() {
    ResourceRegistry registry;
    const resource_registration_result_t result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    if (registry.Acquire(result.Handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("acquire failed before retire");
    }

    const ResourceStatus retireStatus = registry.Retire(result.Handle);
    if (retireStatus != ResourceStatus::StillReferenced) {
        return Fail("retire with outstanding acquire did not return explicit status");
    }

    if (registry.Snapshot().RegisteredResourceCount != 1U) {
        return Fail("rejected retire changed registered count");
    }

    return 0;
}

int ResourceRetireRejectsLiveDependentEdge() {
    ResourceRegistry registry;
    const resource_registration_result_t dependency = Register(registry, TYPE_TEXTURE, "texture_a");
    const resource_registration_result_t dependent = Register(registry, TYPE_MATERIAL, "material_a");
    if (!dependency.Succeeded()) {
        return Fail("dependency registration failed");
    }

    if (!dependent.Succeeded()) {
        return Fail("dependent registration failed");
    }

    if (registry.AddDependency(dependent.Handle, dependency.Handle) != ResourceStatus::Success) {
        return Fail("dependency edge registration failed");
    }

    if (registry.Retire(dependency.Handle) != ResourceStatus::StillDependedOn) {
        return Fail("retire with live dependent edge did not return explicit status");
    }

    if (registry.Snapshot().DependencyEdgeCount != 1U) {
        return Fail("rejected dependency retire changed edge count");
    }

    if (registry.Retire(dependent.Handle) != ResourceStatus::Success) {
        return Fail("dependent retire failed");
    }

    if (registry.Snapshot().DependencyEdgeCount != 0U) {
        return Fail("successful retire did not clear outbound dependency edge");
    }

    if (registry.Retire(dependency.Handle) != ResourceStatus::Success) {
        return Fail("dependency retire failed after outbound edge cleared");
    }

    return 0;
}

int ResourceDependencyValidationRejectsMissingDependency() {
    ResourceRegistry registry;
    const resource_registration_result_t dependent = Register(registry, TYPE_MATERIAL, "material_a");
    if (!dependent.Succeeded()) {
        return Fail("dependent registration failed");
    }

    const ResourceStatus status = registry.AddDependency(dependent.Handle, resource_handle_t{31U, 1U});
    if (status != ResourceStatus::DependencyMissing) {
        return Fail("missing dependency did not return explicit status");
    }

    const resource_snapshot_t snapshot = registry.Snapshot();
    if (snapshot.DependencyEdgeCount != 0U) {
        return Fail("missing dependency changed edge count");
    }

    if (snapshot.DependencyValidationCount != 1U) {
        return Fail("missing dependency validation was not counted");
    }

    return 0;
}

int ResourceDependencyValidationRejectsCycle() {
    ResourceRegistry registry;
    const resource_registration_result_t first = Register(registry, TYPE_TEXTURE, "texture_a");
    const resource_registration_result_t second = Register(registry, TYPE_MATERIAL, "material_a");
    const resource_registration_result_t third = Register(registry, TYPE_EFFECT, "effect_a");
    if (!first.Succeeded()) {
        return Fail("first registration failed");
    }

    if (!second.Succeeded()) {
        return Fail("second registration failed");
    }

    if (!third.Succeeded()) {
        return Fail("third registration failed");
    }

    if (registry.AddDependency(first.Handle, first.Handle) != ResourceStatus::DependencyCycle) {
        return Fail("self dependency did not return cycle status");
    }

    if (registry.AddDependency(first.Handle, second.Handle) != ResourceStatus::Success) {
        return Fail("first dependency edge failed");
    }

    if (registry.AddDependency(second.Handle, third.Handle) != ResourceStatus::Success) {
        return Fail("second dependency edge failed");
    }

    const ResourceStatus cycleStatus = registry.AddDependency(third.Handle, first.Handle);
    if (cycleStatus != ResourceStatus::DependencyCycle) {
        return Fail("dependency cycle did not return explicit status");
    }

    if (registry.Snapshot().DependencyEdgeCount != 2U) {
        return Fail("dependency cycle changed edge count");
    }

    return 0;
}

int ResourceNoFileOrPackageDependencyForHandleRegistry() {
    ResourceRegistry registry;
    const resource_registration_result_t result = Register(registry, TYPE_TEXTURE, "synthetic_texture");
    if (!result.Succeeded()) {
        return Fail("synthetic resource registration failed");
    }

    if (registry.Acquire(result.Handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("synthetic handle acquire failed");
    }

    if (registry.Release(result.Handle) != ResourceStatus::Success) {
        return Fail("synthetic handle release failed");
    }

    if (registry.Snapshot().RegisteredResourceCount != 1U) {
        return Fail("synthetic handle registry left resource scope");
    }

    return 0;
}

int ResourceDisabledDiagnosticsDoesNotChangeResults() {
    ResourceRegistry recordingRegistry;
    ResourceRegistry diagnosticsDisabledRegistry;
    const resource_registration_result_t recordingResult = Register(recordingRegistry, TYPE_TEXTURE, "texture_a");
    const resource_registration_result_t disabledResult = Register(diagnosticsDisabledRegistry, TYPE_TEXTURE, "texture_a");
    if (!recordingResult.Succeeded()) {
        return Fail("recording registry registration failed");
    }

    if (!disabledResult.Succeeded()) {
        return Fail("disabled registry registration failed");
    }

    const ResourceStatus recordingAcquire = recordingRegistry.Acquire(recordingResult.Handle, TYPE_TEXTURE);
    const ResourceStatus disabledAcquire = diagnosticsDisabledRegistry.Acquire(disabledResult.Handle, TYPE_TEXTURE);
    if (recordingAcquire != disabledAcquire) {
        return Fail("disabled diagnostics changed acquire status");
    }

    const ResourceStatus recordingMismatch = recordingRegistry.Acquire(recordingResult.Handle, TYPE_AUDIO);
    const ResourceStatus disabledMismatch = diagnosticsDisabledRegistry.Acquire(disabledResult.Handle, TYPE_AUDIO);
    if (recordingMismatch != disabledMismatch) {
        return Fail("disabled diagnostics changed failure status");
    }

    if (!SnapshotsMatch(recordingRegistry.Snapshot(), diagnosticsDisabledRegistry.Snapshot())) {
        return Fail("disabled diagnostics changed resource snapshot");
    }

    return 0;
}

int ResourceNoHiddenAllocationUsesYuMemorySignal() {
    ResourceRegistry registry;
    const resource_snapshot_t initialSnapshot = registry.Snapshot();
    if (initialSnapshot.AllocationAccountingStatus != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("resource registry did not expose YuMemory accounting vocabulary");
    }

    const resource_registration_result_t result = Register(registry, TYPE_TEXTURE, "texture_a");
    if (!result.Succeeded()) {
        return Fail("registration failed");
    }

    if (registry.Acquire(result.Handle, TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("acquire failed");
    }

    if (registry.Release(result.Handle) != ResourceStatus::Success) {
        return Fail("release failed");
    }

    const resource_snapshot_t afterSnapshot = registry.Snapshot();
    if (afterSnapshot.ResourceCapacity != initialSnapshot.ResourceCapacity) {
        return Fail("resource capacity changed during handle fixture");
    }

    if (afterSnapshot.DependencyEdgeCapacity != initialSnapshot.DependencyEdgeCapacity) {
        return Fail("dependency edge capacity changed during handle fixture");
    }

    if (afterSnapshot.AllocationAccountingStatus != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("resource registry changed allocation accounting vocabulary");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const TestRegistry testRegistry{
        {TEST_REGISTER, ResourceRegisterSyntheticDescriptorReturnsGenerationHandle},
        {TEST_DUPLICATE, ResourceRegisterDuplicateReturnsExplicitStatus},
        {TEST_CAPACITY, ResourceRegistryRejectsCapacityOverflowWithoutMutation},
        {TEST_TYPE_CAPACITY, ResourceTypeCapacityOverflowDoesNotMutate},
        {TEST_WRONG_GENERATION, ResourceHandleRejectsWrongGeneration},
        {TEST_TYPE_MISMATCH, ResourceHandleRejectsTypeMismatch},
        {TEST_ACQUIRE_RELEASE, ResourceAcquireReleaseTracksReferenceCount},
        {TEST_REPEATED_ACQUIRE, ResourceRepeatedAcquireIncrementsReferenceCount},
        {TEST_REFERENCE_OVERFLOW, ResourceAcquireRejectsReferenceCountOverflow},
        {TEST_RETIRE_REFERENCED, ResourceRetireRejectsOutstandingAcquire},
        {TEST_RETIRE_DEPENDED_ON, ResourceRetireRejectsLiveDependentEdge},
        {TEST_MISSING_DEPENDENCY, ResourceDependencyValidationRejectsMissingDependency},
        {TEST_DEPENDENCY_CYCLE, ResourceDependencyValidationRejectsCycle},
        {TEST_NO_FILE_PACKAGE, ResourceNoFileOrPackageDependencyForHandleRegistry},
        {TEST_DISABLED_DIAGNOSTICS, ResourceDisabledDiagnosticsDoesNotChangeResults},
        {TEST_NO_HIDDEN_ALLOCATION, ResourceNoHiddenAllocationUsesYuMemorySignal}};

    const std::string_view testName(argv[1]);
    const auto testEntry = testRegistry.find(testName);
    if (testEntry == testRegistry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return testEntry->second();
}
