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
#include "YuEngine/Resource/ResourceLoadCommitRequest.h"
#include "YuEngine/Resource/ResourceLoadCommitStatus.h"
#include "YuEngine/Resource/ResourceLoadState.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/Resource/ResourceResidencyBudgetDesc.h"
#include "YuEngine/Resource/ResourceResidencyRequest.h"
#include "YuEngine/Resource/ResourceResidencySnapshot.h"
#include "YuEngine/Resource/ResourceResidencyState.h"
#include "YuEngine/Resource/ResourceResidencyStatus.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::resource::ResourceDescriptor;
using yuengine::resource::ResourceHandle;
using yuengine::resource::ResourceLoadCommitRequest;
using yuengine::resource::ResourceLoadCommitStatus;
using yuengine::resource::ResourceLoadState;
using yuengine::resource::ResourceResidencyBudgetDesc;
using yuengine::resource::ResourceResidencyRequest;
using yuengine::resource::ResourceResidencySnapshot;
using yuengine::resource::ResourceResidencyState;
using yuengine::resource::ResourceResidencyStatus;
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
constexpr const char *TEST_LOAD_COMMIT_SUCCESS = "Resource_LoadCommit_UploadSuccessSetsTerminalState";
constexpr const char *TEST_LOAD_COMMIT_FAILED_UPLOAD = "Resource_LoadCommit_FailedUploadSetsFailedState";
constexpr const char *TEST_LOAD_COMMIT_INVALID_HANDLE =
    "Resource_LoadCommit_RejectsInvalidHandleWithoutSlotMutation";
constexpr const char *TEST_LOAD_COMMIT_TYPE_MISMATCH =
    "Resource_LoadCommit_RejectsTypeMismatchWithoutMutation";
constexpr const char *TEST_LOAD_COMMIT_DUPLICATE = "Resource_LoadCommit_RejectsDuplicateCommitId";
constexpr const char *TEST_LOAD_COMMIT_INVALID_TRANSITION =
    "Resource_LoadCommit_RejectsInvalidTransition";
constexpr const char *TEST_LOAD_COMMIT_SNAPSHOT = "Resource_LoadCommit_SnapshotTracksCounters";
constexpr const char *TEST_RESIDENCY_ADMIT = "Resource_Residency_AdmitsUploadedSlotWithinBudget";
constexpr const char *TEST_RESIDENCY_UNLOADED =
    "Resource_Residency_RejectsUnloadedWithoutMutation";
constexpr const char *TEST_RESIDENCY_FAILED_LOAD =
    "Resource_Residency_RejectsFailedLoadWithoutMutation";
constexpr const char *TEST_RESIDENCY_TYPE_MISMATCH =
    "Resource_Residency_RejectsTypeMismatchWithoutMutation";
constexpr const char *TEST_RESIDENCY_DUPLICATE = "Resource_Residency_RejectsDuplicateAdmission";
constexpr const char *TEST_RESIDENCY_BUDGET = "Resource_Residency_RejectsBudgetOverflow";
constexpr const char *TEST_RESIDENCY_PIN_UNPIN = "Resource_Residency_PinUnpinTracksCounters";
constexpr const char *TEST_RESIDENCY_CANDIDATE =
    "Resource_Residency_SelectsEvictionCandidateInSlotOrder";
constexpr const char *TEST_RESIDENCY_NO_CANDIDATE = "Resource_Residency_ReportsNoEvictionCandidate";
constexpr const char *TEST_RESIDENCY_FAILED_VALIDATION =
    "Resource_Residency_FailedValidationDoesNotMutateResourceState";
constexpr const char *TEST_RESIDENCY_EVICT = "Resource_Residency_EvictsResourceOwnedStateOnly";
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
constexpr std::uint64_t COMMIT_ONE = 2001U;
constexpr std::uint64_t COMMIT_TWO = 2002U;
constexpr std::uint64_t COMMIT_THREE = 2003U;
constexpr std::uint64_t UPLOAD_ONE = 3001U;
constexpr std::uint64_t UPLOAD_TWO = 3002U;
constexpr std::uint64_t UPLOAD_THREE = 3003U;
constexpr std::uint64_t STAGING_ONE = 4001U;
constexpr std::uint32_t UPLOAD_BYTE_COUNT = 64U;
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

ResourceLoadCommitRequest LoadCommitRequest(
    ResourceHandle resource,
    ResourceTypeId expected_type,
    ResourceLoadState load_state,
    std::uint64_t commit_id,
    std::uint64_t upload_id) {
    ResourceLoadCommitRequest request;
    request.resource = resource;
    request.expected_type = expected_type;
    request.load_state = load_state;
    request.commit_id = commit_id;
    request.upload_id = upload_id;
    request.staging_request_id = STAGING_ONE;
    request.upload_byte_count = UPLOAD_BYTE_COUNT;
    return request;
}

ResourceResidencyRequest ResidencyRequest(ResourceHandle resource, ResourceTypeId expected_type) {
    ResourceResidencyRequest request;
    request.resource = resource;
    request.expected_type = expected_type;
    return request;
}

bool LoadStateMatches(
    const ResourceRegistry &registry,
    ResourceHandle resource,
    ResourceTypeId expected_type,
    ResourceLoadState expected_state) {
    ResourceLoadState load_state = ResourceLoadState::Unloaded;
    const ResourceLoadCommitStatus status = registry.GetLoadState(resource, expected_type, &load_state);
    if (status != ResourceLoadCommitStatus::Success) {
        return false;
    }

    return load_state == expected_state;
}

bool ResidencyStateMatches(
    const ResourceRegistry &registry,
    ResourceHandle resource,
    ResourceTypeId expected_type,
    ResourceResidencyState expected_state) {
    ResourceResidencyState residency_state = ResourceResidencyState::Unloaded;
    const ResourceResidencyStatus status = registry.GetResidencyState(
        resource,
        expected_type,
        &residency_state);
    if (status != ResourceResidencyStatus::Success) {
        return false;
    }

    return residency_state == expected_state;
}

bool CommitLoad(
    ResourceRegistry &registry,
    ResourceHandle resource,
    ResourceTypeId expected_type,
    ResourceLoadState load_state,
    std::uint64_t commit_id,
    std::uint64_t upload_id) {
    const ResourceLoadCommitRequest request = LoadCommitRequest(
        resource,
        expected_type,
        load_state,
        commit_id,
        upload_id);
    return registry.CommitUploadCompletion(request) == ResourceLoadCommitStatus::Success;
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

int ResourceLoadCommitUploadSuccessSetsTerminalState() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_load_success");
    if (!result.Succeeded()) {
        return Fail("load commit fixture registration failed");
    }

    const ResourceLoadCommitRequest request = LoadCommitRequest(
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (registry.CommitUploadCompletion(request) != ResourceLoadCommitStatus::Success) {
        return Fail("load commit did not accept successful upload");
    }

    if (!LoadStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Uploaded)) {
        return Fail("successful load commit did not set uploaded state");
    }

    const ResourceSnapshot snapshot = registry.Snapshot();
    if (snapshot.load_commit_count != 1U) {
        return Fail("load commit count was not tracked");
    }

    if (snapshot.loaded_resource_count != 1U) {
        return Fail("loaded resource count was not tracked");
    }

    if (snapshot.last_load_state != ResourceLoadState::Uploaded) {
        return Fail("last load state did not report uploaded");
    }

    return 0;
}

int ResourceLoadCommitFailedUploadSetsFailedState() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_load_failed");
    if (!result.Succeeded()) {
        return Fail("failed load fixture registration failed");
    }

    const ResourceLoadCommitRequest request = LoadCommitRequest(
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Failed,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (registry.CommitUploadCompletion(request) != ResourceLoadCommitStatus::Success) {
        return Fail("load commit did not accept failed upload");
    }

    if (!LoadStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Failed)) {
        return Fail("failed load commit did not set failed state");
    }

    const ResourceSnapshot snapshot = registry.Snapshot();
    if (snapshot.failed_resource_count != 1U) {
        return Fail("failed resource count was not tracked");
    }

    return 0;
}

int ResourceLoadCommitRejectsInvalidHandleWithoutSlotMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_invalid_handle_guard");
    if (!result.Succeeded()) {
        return Fail("invalid handle fixture registration failed");
    }

    const ResourceSnapshot before_snapshot = registry.Snapshot();
    const ResourceLoadCommitRequest request = LoadCommitRequest(
        ResourceHandle{},
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (registry.CommitUploadCompletion(request) != ResourceLoadCommitStatus::InvalidHandle) {
        return Fail("invalid handle load commit returned wrong status");
    }

    if (!LoadStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Unloaded)) {
        return Fail("invalid handle load commit changed live slot state");
    }

    const ResourceSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.load_commit_count != before_snapshot.load_commit_count) {
        return Fail("invalid handle load commit changed committed count");
    }

    if (after_snapshot.load_commit_record_count != before_snapshot.load_commit_record_count) {
        return Fail("invalid handle load commit stored a record");
    }

    return 0;
}

int ResourceLoadCommitRejectsTypeMismatchWithoutMutation() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_type_guard");
    if (!result.Succeeded()) {
        return Fail("type mismatch fixture registration failed");
    }

    const ResourceLoadCommitRequest request = LoadCommitRequest(
        result.handle,
        TYPE_AUDIO,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (registry.CommitUploadCompletion(request) != ResourceLoadCommitStatus::TypeMismatch) {
        return Fail("type mismatch load commit returned wrong status");
    }

    if (!LoadStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Unloaded)) {
        return Fail("type mismatch load commit changed load state");
    }

    if (registry.Snapshot().load_commit_count != 0U) {
        return Fail("type mismatch load commit changed committed count");
    }

    return 0;
}

int ResourceLoadCommitRejectsDuplicateCommitId() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_duplicate_commit");
    if (!result.Succeeded()) {
        return Fail("duplicate commit fixture registration failed");
    }

    const ResourceLoadCommitRequest first_request = LoadCommitRequest(
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (registry.CommitUploadCompletion(first_request) != ResourceLoadCommitStatus::Success) {
        return Fail("first duplicate fixture commit failed");
    }

    const ResourceLoadCommitRequest duplicate_request = LoadCommitRequest(
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Failed,
        COMMIT_ONE,
        UPLOAD_TWO);
    if (registry.CommitUploadCompletion(duplicate_request) != ResourceLoadCommitStatus::DuplicateCommitId) {
        return Fail("duplicate commit id returned wrong status");
    }

    if (!LoadStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Uploaded)) {
        return Fail("duplicate commit id changed terminal state");
    }

    if (registry.Snapshot().duplicate_load_commit_count != 1U) {
        return Fail("duplicate load commit count was not tracked");
    }

    return 0;
}

int ResourceLoadCommitRejectsInvalidTransition() {
    ResourceRegistry registry;
    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_transition_guard");
    if (!result.Succeeded()) {
        return Fail("invalid transition fixture registration failed");
    }

    const ResourceLoadCommitRequest first_request = LoadCommitRequest(
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (registry.CommitUploadCompletion(first_request) != ResourceLoadCommitStatus::Success) {
        return Fail("first transition fixture commit failed");
    }

    const ResourceLoadCommitRequest second_request = LoadCommitRequest(
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Failed,
        COMMIT_TWO,
        UPLOAD_TWO);
    if (registry.CommitUploadCompletion(second_request) != ResourceLoadCommitStatus::InvalidTransition) {
        return Fail("invalid transition returned wrong status");
    }

    if (!LoadStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Uploaded)) {
        return Fail("invalid transition changed terminal state");
    }

    if (registry.Snapshot().invalid_load_transition_count != 1U) {
        return Fail("invalid transition count was not tracked");
    }

    return 0;
}

int ResourceLoadCommitSnapshotTracksCounters() {
    ResourceRegistry registry;
    const ResourceRegistrationResult uploaded_result = Register(registry, TYPE_TEXTURE, "texture_snapshot_uploaded");
    const ResourceRegistrationResult failed_result = Register(registry, TYPE_MATERIAL, "material_snapshot_failed");
    if (!uploaded_result.Succeeded()) {
        return Fail("snapshot uploaded registration failed");
    }

    if (!failed_result.Succeeded()) {
        return Fail("snapshot failed registration failed");
    }

    const ResourceLoadCommitRequest uploaded_request = LoadCommitRequest(
        uploaded_result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    const ResourceLoadCommitRequest failed_request = LoadCommitRequest(
        failed_result.handle,
        TYPE_MATERIAL,
        ResourceLoadState::Failed,
        COMMIT_TWO,
        UPLOAD_TWO);
    registry.CommitUploadCompletion(uploaded_request);
    registry.CommitUploadCompletion(failed_request);

    const ResourceSnapshot snapshot = registry.Snapshot();
    if (snapshot.load_commit_record_count != 2U) {
        return Fail("snapshot load commit record count changed");
    }

    if (snapshot.load_commit_count != 2U) {
        return Fail("snapshot load commit count changed");
    }

    if (snapshot.loaded_resource_count != 1U) {
        return Fail("snapshot loaded resource count changed");
    }

    if (snapshot.failed_resource_count != 1U) {
        return Fail("snapshot failed resource count changed");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("load commit snapshot changed allocation vocabulary");
    }

    return 0;
}

int ResourceResidencyAdmitsUploadedSlotWithinBudget() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    if (registry.SetResidencyBudget(budget) != ResourceResidencyStatus::Success) {
        return Fail("residency budget setup failed");
    }

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_resident");
    if (!result.Succeeded()) {
        return Fail("residency admission registration failed");
    }

    const bool committed = CommitLoad(
        registry,
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("residency admission upload commit failed");
    }

    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_TEXTURE);
    const ResourceResidencyStatus status = registry.AdmitResident(request);
    if (status != ResourceResidencyStatus::Success) {
        return Fail("uploaded resource was not admitted as resident");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Evictable)) {
        return Fail("admitted resource did not become evictable resident");
    }

    const ResourceResidencySnapshot snapshot = registry.ResidencySnapshot();
    if (snapshot.resident_resource_count != 1U) {
        return Fail("resident resource count was not tracked");
    }

    if (snapshot.resident_byte_count != UPLOAD_BYTE_COUNT) {
        return Fail("resident byte count was not tracked");
    }

    if (snapshot.evictable_byte_count != UPLOAD_BYTE_COUNT) {
        return Fail("evictable byte count was not tracked");
    }

    return 0;
}

int ResourceResidencyRejectsUnloadedWithoutMutation() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_unloaded_residency");
    if (!result.Succeeded()) {
        return Fail("unloaded residency fixture registration failed");
    }

    const ResourceSnapshot before_resource_snapshot = registry.Snapshot();
    const ResourceResidencySnapshot before_residency_snapshot = registry.ResidencySnapshot();
    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_TEXTURE);
    const ResourceResidencyStatus status = registry.AdmitResident(request);
    if (status != ResourceResidencyStatus::NotUploaded) {
        return Fail("unloaded residency request returned wrong status");
    }

    if (!LoadStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Unloaded)) {
        return Fail("unloaded residency rejection changed load state");
    }

    const ResourceSnapshot after_resource_snapshot = registry.Snapshot();
    if (after_resource_snapshot.load_commit_count != before_resource_snapshot.load_commit_count) {
        return Fail("unloaded residency rejection changed load commits");
    }

    const ResourceResidencySnapshot after_residency_snapshot = registry.ResidencySnapshot();
    if (after_residency_snapshot.resident_resource_count != before_residency_snapshot.resident_resource_count) {
        return Fail("unloaded residency rejection changed resident count");
    }

    return 0;
}

int ResourceResidencyRejectsFailedLoadWithoutMutation() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_failed_residency");
    if (!result.Succeeded()) {
        return Fail("failed residency fixture registration failed");
    }

    const bool committed = CommitLoad(
        registry,
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Failed,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("failed residency fixture load commit failed");
    }

    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_TEXTURE);
    const ResourceResidencyStatus status = registry.AdmitResident(request);
    if (status != ResourceResidencyStatus::FailedLoad) {
        return Fail("failed load residency request returned wrong status");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Failed)) {
        return Fail("failed load residency rejection changed state");
    }

    if (registry.ResidencySnapshot().resident_resource_count != 0U) {
        return Fail("failed load residency rejection admitted resource");
    }

    return 0;
}

int ResourceResidencyRejectsTypeMismatchWithoutMutation() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_residency_type");
    if (!result.Succeeded()) {
        return Fail("type mismatch residency fixture registration failed");
    }

    const bool committed = CommitLoad(
        registry,
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("type mismatch residency fixture load commit failed");
    }

    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_AUDIO);
    const ResourceResidencyStatus status = registry.AdmitResident(request);
    if (status != ResourceResidencyStatus::TypeMismatch) {
        return Fail("residency type mismatch returned wrong status");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Uploaded)) {
        return Fail("residency type mismatch changed state");
    }

    if (registry.ResidencySnapshot().resident_resource_count != 0U) {
        return Fail("residency type mismatch admitted resource");
    }

    return 0;
}

int ResourceResidencyRejectsDuplicateAdmission() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_residency_duplicate");
    if (!result.Succeeded()) {
        return Fail("duplicate residency fixture registration failed");
    }

    const bool committed = CommitLoad(
        registry,
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("duplicate residency fixture load commit failed");
    }

    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_TEXTURE);
    if (registry.AdmitResident(request) != ResourceResidencyStatus::Success) {
        return Fail("first residency admission failed");
    }

    const ResourceResidencySnapshot before_snapshot = registry.ResidencySnapshot();
    const ResourceResidencyStatus status = registry.AdmitResident(request);
    if (status != ResourceResidencyStatus::AlreadyResident) {
        return Fail("duplicate residency admission returned wrong status");
    }

    const ResourceResidencySnapshot after_snapshot = registry.ResidencySnapshot();
    if (after_snapshot.resident_resource_count != before_snapshot.resident_resource_count) {
        return Fail("duplicate residency admission changed resident count");
    }

    if (after_snapshot.resident_byte_count != before_snapshot.resident_byte_count) {
        return Fail("duplicate residency admission changed resident bytes");
    }

    return 0;
}

int ResourceResidencyRejectsBudgetOverflow() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 32U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_residency_budget");
    if (!result.Succeeded()) {
        return Fail("budget residency fixture registration failed");
    }

    const bool committed = CommitLoad(
        registry,
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("budget residency fixture load commit failed");
    }

    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_TEXTURE);
    const ResourceResidencyStatus status = registry.AdmitResident(request);
    if (status != ResourceResidencyStatus::BudgetExceeded) {
        return Fail("residency budget overflow returned wrong status");
    }

    const ResourceResidencySnapshot snapshot = registry.ResidencySnapshot();
    if (snapshot.budget_rejected_residency_count != 1U) {
        return Fail("residency budget rejection was not counted");
    }

    if (snapshot.resident_byte_count != 0U) {
        return Fail("residency budget overflow changed resident bytes");
    }

    return 0;
}

int ResourceResidencyPinUnpinTracksCounters() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_residency_pin");
    if (!result.Succeeded()) {
        return Fail("pin residency fixture registration failed");
    }

    const bool committed = CommitLoad(
        registry,
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("pin residency fixture load commit failed");
    }

    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_TEXTURE);
    if (registry.AdmitResident(request) != ResourceResidencyStatus::Success) {
        return Fail("pin residency admission failed");
    }

    if (registry.PinResident(request) != ResourceResidencyStatus::Success) {
        return Fail("pin resident failed");
    }

    ResourceResidencySnapshot pinned_snapshot = registry.ResidencySnapshot();
    if (pinned_snapshot.pinned_byte_count != UPLOAD_BYTE_COUNT) {
        return Fail("pin resident did not track pinned bytes");
    }

    if (pinned_snapshot.evictable_byte_count != 0U) {
        return Fail("pin resident left resource evictable");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Pinned)) {
        return Fail("pin resident did not set pinned state");
    }

    if (registry.UnpinResident(request) != ResourceResidencyStatus::Success) {
        return Fail("unpin resident failed");
    }

    const ResourceResidencySnapshot unpinned_snapshot = registry.ResidencySnapshot();
    if (unpinned_snapshot.pinned_byte_count != 0U) {
        return Fail("unpin resident left pinned bytes");
    }

    if (unpinned_snapshot.evictable_byte_count != UPLOAD_BYTE_COUNT) {
        return Fail("unpin resident did not restore evictable bytes");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Evictable)) {
        return Fail("unpin resident did not restore evictable state");
    }

    return 0;
}

int ResourceResidencySelectsEvictionCandidateInSlotOrder() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 256U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult first = Register(registry, TYPE_TEXTURE, "texture_candidate_first");
    const ResourceRegistrationResult second = Register(registry, TYPE_TEXTURE, "texture_candidate_second");
    const ResourceRegistrationResult third = Register(registry, TYPE_TEXTURE, "texture_candidate_third");
    if (!first.Succeeded()) {
        return Fail("first candidate registration failed");
    }

    if (!second.Succeeded()) {
        return Fail("second candidate registration failed");
    }

    if (!third.Succeeded()) {
        return Fail("third candidate registration failed");
    }

    const bool first_commit = CommitLoad(
        registry,
        first.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    const bool second_commit = CommitLoad(
        registry,
        second.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_TWO,
        UPLOAD_TWO);
    const bool third_commit = CommitLoad(
        registry,
        third.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_THREE,
        UPLOAD_THREE);
    if (!first_commit) {
        return Fail("first candidate load commit failed");
    }

    if (!second_commit) {
        return Fail("second candidate load commit failed");
    }

    if (!third_commit) {
        return Fail("third candidate load commit failed");
    }

    const ResourceResidencyRequest first_request = ResidencyRequest(first.handle, TYPE_TEXTURE);
    const ResourceResidencyRequest second_request = ResidencyRequest(second.handle, TYPE_TEXTURE);
    const ResourceResidencyRequest third_request = ResidencyRequest(third.handle, TYPE_TEXTURE);
    registry.AdmitResident(first_request);
    registry.AdmitResident(second_request);
    registry.AdmitResident(third_request);
    registry.PinResident(first_request);
    registry.Acquire(second.handle, TYPE_TEXTURE);

    ResourceHandle candidate;
    const ResourceResidencyStatus status = registry.SelectEvictionCandidate(&candidate);
    if (status != ResourceResidencyStatus::Success) {
        return Fail("eviction candidate selection failed");
    }

    if (candidate.slot != third.handle.slot) {
        return Fail("eviction candidate did not use deterministic slot order");
    }

    if (candidate.generation != third.handle.generation) {
        return Fail("eviction candidate returned wrong generation");
    }

    if (registry.ResidencySnapshot().eviction_candidate_count != 1U) {
        return Fail("eviction candidate selection was not counted");
    }

    return 0;
}

int ResourceResidencyReportsNoEvictionCandidate() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_no_candidate");
    if (!result.Succeeded()) {
        return Fail("no candidate fixture registration failed");
    }

    const bool committed = CommitLoad(
        registry,
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("no candidate fixture load commit failed");
    }

    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_TEXTURE);
    registry.AdmitResident(request);
    registry.PinResident(request);

    ResourceHandle candidate;
    const ResourceResidencyStatus status = registry.SelectEvictionCandidate(&candidate);
    if (status != ResourceResidencyStatus::NoCandidate) {
        return Fail("no candidate selection returned wrong status");
    }

    if (candidate.IsValid()) {
        return Fail("no candidate selection returned a valid handle");
    }

    if (registry.ResidencySnapshot().eviction_candidate_miss_count != 1U) {
        return Fail("no candidate selection was not counted");
    }

    return 0;
}

int ResourceResidencyFailedValidationDoesNotMutateResourceState() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult dependency = Register(registry, TYPE_TEXTURE, "texture_validation_dependency");
    const ResourceRegistrationResult dependent = Register(registry, TYPE_MATERIAL, "material_validation_dependent");
    if (!dependency.Succeeded()) {
        return Fail("validation dependency registration failed");
    }

    if (!dependent.Succeeded()) {
        return Fail("validation dependent registration failed");
    }

    if (registry.AddDependency(dependent.handle, dependency.handle) != ResourceStatus::Success) {
        return Fail("validation dependency edge failed");
    }

    const bool committed = CommitLoad(
        registry,
        dependency.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("validation fixture load commit failed");
    }

    const ResourceResidencyRequest admit_request = ResidencyRequest(dependency.handle, TYPE_TEXTURE);
    registry.AdmitResident(admit_request);
    const ResourceSnapshot before_resource_snapshot = registry.Snapshot();
    const ResourceResidencySnapshot before_residency_snapshot = registry.ResidencySnapshot();

    const ResourceResidencyRequest invalid_request = ResidencyRequest(ResourceHandle{}, TYPE_TEXTURE);
    const ResourceResidencyStatus status = registry.AdmitResident(invalid_request);
    if (status != ResourceResidencyStatus::InvalidHandle) {
        return Fail("invalid residency request returned wrong status");
    }

    const ResourceSnapshot after_resource_snapshot = registry.Snapshot();
    if (after_resource_snapshot.registered_resource_count != before_resource_snapshot.registered_resource_count) {
        return Fail("invalid residency request changed registered count");
    }

    if (after_resource_snapshot.dependency_edge_count != before_resource_snapshot.dependency_edge_count) {
        return Fail("invalid residency request changed dependency edges");
    }

    if (after_resource_snapshot.load_commit_count != before_resource_snapshot.load_commit_count) {
        return Fail("invalid residency request changed load commits");
    }

    const ResourceResidencySnapshot after_residency_snapshot = registry.ResidencySnapshot();
    if (after_residency_snapshot.resident_resource_count != before_residency_snapshot.resident_resource_count) {
        return Fail("invalid residency request changed resident count");
    }

    if (after_residency_snapshot.resident_byte_count != before_residency_snapshot.resident_byte_count) {
        return Fail("invalid residency request changed resident bytes");
    }

    return 0;
}

int ResourceResidencyEvictsResourceOwnedStateOnly() {
    ResourceRegistry registry;
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = 128U;
    registry.SetResidencyBudget(budget);

    const ResourceRegistrationResult result = Register(registry, TYPE_TEXTURE, "texture_residency_evict");
    if (!result.Succeeded()) {
        return Fail("evict residency fixture registration failed");
    }

    const bool committed = CommitLoad(
        registry,
        result.handle,
        TYPE_TEXTURE,
        ResourceLoadState::Uploaded,
        COMMIT_ONE,
        UPLOAD_ONE);
    if (!committed) {
        return Fail("evict residency fixture load commit failed");
    }

    const ResourceResidencyRequest request = ResidencyRequest(result.handle, TYPE_TEXTURE);
    registry.AdmitResident(request);
    const ResourceResidencyStatus status = registry.EvictResident(request);
    if (status != ResourceResidencyStatus::Success) {
        return Fail("resource-owned eviction state failed");
    }

    if (!ResidencyStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceResidencyState::Evicted)) {
        return Fail("resource-owned eviction did not set evicted state");
    }

    if (!LoadStateMatches(registry, result.handle, TYPE_TEXTURE, ResourceLoadState::Uploaded)) {
        return Fail("resource-owned eviction changed load state");
    }

    const ResourceResidencySnapshot snapshot = registry.ResidencySnapshot();
    if (snapshot.resident_byte_count != 0U) {
        return Fail("resource-owned eviction left resident bytes");
    }

    if (snapshot.evicted_resource_count != 1U) {
        return Fail("resource-owned eviction was not tracked");
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
        {TEST_NO_HIDDEN_ALLOCATION, ResourceNoHiddenAllocationUsesYuMemorySignal},
        {TEST_LOAD_COMMIT_SUCCESS, ResourceLoadCommitUploadSuccessSetsTerminalState},
        {TEST_LOAD_COMMIT_FAILED_UPLOAD, ResourceLoadCommitFailedUploadSetsFailedState},
        {TEST_LOAD_COMMIT_INVALID_HANDLE, ResourceLoadCommitRejectsInvalidHandleWithoutSlotMutation},
        {TEST_LOAD_COMMIT_TYPE_MISMATCH, ResourceLoadCommitRejectsTypeMismatchWithoutMutation},
        {TEST_LOAD_COMMIT_DUPLICATE, ResourceLoadCommitRejectsDuplicateCommitId},
        {TEST_LOAD_COMMIT_INVALID_TRANSITION, ResourceLoadCommitRejectsInvalidTransition},
        {TEST_LOAD_COMMIT_SNAPSHOT, ResourceLoadCommitSnapshotTracksCounters},
        {TEST_RESIDENCY_ADMIT, ResourceResidencyAdmitsUploadedSlotWithinBudget},
        {TEST_RESIDENCY_UNLOADED, ResourceResidencyRejectsUnloadedWithoutMutation},
        {TEST_RESIDENCY_FAILED_LOAD, ResourceResidencyRejectsFailedLoadWithoutMutation},
        {TEST_RESIDENCY_TYPE_MISMATCH, ResourceResidencyRejectsTypeMismatchWithoutMutation},
        {TEST_RESIDENCY_DUPLICATE, ResourceResidencyRejectsDuplicateAdmission},
        {TEST_RESIDENCY_BUDGET, ResourceResidencyRejectsBudgetOverflow},
        {TEST_RESIDENCY_PIN_UNPIN, ResourceResidencyPinUnpinTracksCounters},
        {TEST_RESIDENCY_CANDIDATE, ResourceResidencySelectsEvictionCandidateInSlotOrder},
        {TEST_RESIDENCY_NO_CANDIDATE, ResourceResidencyReportsNoEvictionCandidate},
        {TEST_RESIDENCY_FAILED_VALIDATION, ResourceResidencyFailedValidationDoesNotMutateResourceState},
        {TEST_RESIDENCY_EVICT, ResourceResidencyEvictsResourceOwnedStateOnly}};

    const std::string_view test_name(argv[1]);
    const auto test_entry = test_registry.find(test_name);
    if (test_entry == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_entry->second();
}
