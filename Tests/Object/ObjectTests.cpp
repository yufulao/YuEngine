// Module: Tests Object
// File: Tests/Object/ObjectTests.cpp

#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>

#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Object/ObjectRegistry.h"

using yuengine::memory::MemoryAccountingStatus;
using yuengine::object::ObjectDescriptor;
using yuengine::object::ObjectHandle;
using yuengine::object::ObjectRegistrationResult;
using ObjectRegistry = yuengine::object::ObjectRegistry;
using yuengine::object::ObjectRegistryDesc;
using yuengine::object::ObjectSnapshot;
using yuengine::object::ObjectStatus;
using yuengine::object::ObjectTypeId;
using yuengine::object::INVALID_OBJECT_GENERATION;

namespace {
constexpr const char* TEST_CREATE = "Object_CreateSyntheticObject_ReturnsGenerationHandle";
constexpr const char* TEST_INVALID_TYPE = "Object_CreateRejectsInvalidTypeWithoutMutation";
constexpr const char* TEST_CAPACITY = "Object_RegistryCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_TYPE_CAPACITY = "Object_TypeCapacityOverflow_DoesNotMutate";
constexpr const char* TEST_VALIDATE_STALE = "Object_ValidateRejectsInvalidOrStaleHandle";
constexpr const char* TEST_STALE_OPERATIONS = "Object_InvalidOrStaleHandleOperations_ReturnExplicitStatusWithoutMutation";
constexpr const char* TEST_DESTROY_GENERATION = "Object_DestroyIncrementsGenerationAndInvalidatesOldHandle";
constexpr const char* TEST_REUSE_SLOT = "Object_ReusesFreedSlotWithNewGeneration";
constexpr const char* TEST_ACQUIRE_RELEASE = "Object_AcquireRelease_TracksReferenceCount";
constexpr const char* TEST_REPEATED_ACQUIRE = "Object_RepeatedAcquire_IncrementsReferenceCount";
constexpr const char* TEST_REFERENCE_OVERFLOW = "Object_AcquireRejectsReferenceCountOverflow";
constexpr const char* TEST_RELEASE_ZERO = "Object_ReleaseAtZero_DoesNotMutate";
constexpr const char* TEST_DESTROY_REFERENCED = "Object_DestroyRejectsOutstandingReference";
constexpr const char* TEST_SNAPSHOT = "Object_RegistrySnapshot_ReportsCountsAndLastStatus";
constexpr const char* TEST_DISABLED_DIAGNOSTICS = "Object_DisabledDiagnosticsDoesNotChangeResults";
constexpr const char* TEST_NO_FORBIDDEN_DEPENDENCY = "Object_NoWorldScriptResourceOrGameAdapterDependency";
constexpr const char* TEST_NO_HIDDEN_ALLOCATION = "Object_NoHiddenAllocation_UsesYuMemorySignal";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr const char* TYPE_CAPACITY_RETRY_CREATE_MESSAGE = "type capacity failure consumed free slot";
constexpr const char* TYPE_CAPACITY_RETRY_SLOT_MESSAGE = "retry after type capacity failure used wrong slot";
constexpr const char* TYPE_CAPACITY_RETRY_GENERATION_MESSAGE = "type capacity failure advanced free slot generation";
constexpr const char* UNUSED_VALIDATE_MESSAGE = "unused slot handle did not return invalid handle";
constexpr const char* DESTROYED_VALIDATE_MESSAGE = "already-destroyed handle did not return invalid handle";
constexpr const char* UNUSED_ACQUIRE_MESSAGE = "unused slot acquire did not return explicit status";
constexpr const char* UNUSED_RELEASE_MESSAGE = "unused slot release did not return explicit status";
constexpr const char* UNUSED_DESTROY_MESSAGE = "unused slot destroy did not return explicit status";
constexpr const char* DESTROYED_ACQUIRE_MESSAGE = "already-destroyed acquire did not return explicit status";
constexpr const char* DESTROYED_RELEASE_MESSAGE = "already-destroyed release did not return explicit status";
constexpr const char* DESTROYED_DESTROY_MESSAGE = "already-destroyed destroy did not return explicit status";
constexpr const char* SNAPSHOT_ACCEPTED_OPERATION_MESSAGE = "snapshot did not report accepted operation count";
constexpr ObjectTypeId TYPE_ACTOR{1U};
constexpr ObjectTypeId TYPE_CAMERA{2U};
constexpr ObjectTypeId TYPE_EFFECT{3U};
constexpr std::uint32_t INVALID_GENERATION = INVALID_OBJECT_GENERATION;
using TestFunction = int (*)();

int Fail(const std::string& message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

ObjectDescriptor Descriptor(ObjectTypeId type) {
    return ObjectDescriptor{type, 0U};
}

ObjectDescriptor DescriptorWithReferenceCount(ObjectTypeId type, std::uint32_t reference_count) {
    return ObjectDescriptor{type, reference_count};
}

ObjectRegistrationResult Create(ObjectRegistry& registry, ObjectTypeId type) {
    return registry.CreateSyntheticObject(Descriptor(type));
}

bool RuntimeCountsMatch(const ObjectSnapshot& left, const ObjectSnapshot& right) {
    if (left.type_count != right.type_count) {
        return false;
    }

    if (left.alive_object_count != right.alive_object_count) {
        return false;
    }

    if (left.destroyed_object_count != right.destroyed_object_count) {
        return false;
    }

    if (left.created_object_count != right.created_object_count) {
        return false;
    }

    if (left.referenced_object_count != right.referenced_object_count) {
        return false;
    }

    if (left.released_reference_count != right.released_reference_count) {
        return false;
    }

    return left.allocation_accounting_status == right.allocation_accounting_status;
}

int ObjectCreateSyntheticObjectReturnsGenerationHandle() {
    ObjectRegistry registry;
    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded()) {
        return Fail("synthetic object did not register");
    }

    if (result.handle.slot != 0U) {
        return Fail("first object used unexpected slot");
    }

    if (result.handle.generation == INVALID_GENERATION) {
        return Fail("object handle generation was invalid");
    }

    const ObjectSnapshot snapshot = registry.Snapshot();
    if (snapshot.alive_object_count != 1U) {
        return Fail("alive object count was not recorded");
    }

    if (snapshot.created_object_count != 1U) {
        return Fail("created object count was not recorded");
    }

    if (snapshot.type_count != 1U) {
        return Fail("object type count was not recorded");
    }

    if (snapshot.last_status != ObjectStatus::Success) {
        return Fail("successful create did not record success status");
    }

    return 0;
}

int ObjectCreateRejectsInvalidTypeWithoutMutation() {
    ObjectRegistry registry;
    const ObjectSnapshot before_snapshot = registry.Snapshot();
    const ObjectRegistrationResult result = registry.CreateSyntheticObject(Descriptor(ObjectTypeId{}));
    if (result.status != ObjectStatus::InvalidType) {
        return Fail("invalid type did not return explicit status");
    }

    if (result.handle.IsValid()) {
        return Fail("invalid type returned a valid handle");
    }

    const ObjectSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.alive_object_count != before_snapshot.alive_object_count) {
        return Fail("invalid type changed alive object count");
    }

    if (after_snapshot.type_count != before_snapshot.type_count) {
        return Fail("invalid type changed type count");
    }

    if (after_snapshot.created_object_count != before_snapshot.created_object_count) {
        return Fail("invalid type changed created object count");
    }

    if (after_snapshot.failed_operation_count != before_snapshot.failed_operation_count + 1U) {
        return Fail("invalid type did not record a rejected operation");
    }

    return 0;
}

int ObjectRegistryCapacityOverflowDoesNotMutate() {
    ObjectRegistry registry(ObjectRegistryDesc{1U, 2U});
    const ObjectRegistrationResult first_result = Create(registry, TYPE_ACTOR);
    if (!first_result.Succeeded()) {
        return Fail("first object creation failed");
    }

    const ObjectSnapshot before_snapshot = registry.Snapshot();
    const ObjectRegistrationResult second_result = Create(registry, TYPE_CAMERA);
    if (second_result.status != ObjectStatus::CapacityExceeded) {
        return Fail("capacity overflow did not return explicit status");
    }

    const ObjectSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.alive_object_count != before_snapshot.alive_object_count) {
        return Fail("capacity overflow changed alive object count");
    }

    if (after_snapshot.created_object_count != before_snapshot.created_object_count) {
        return Fail("capacity overflow changed created object count");
    }

    if (after_snapshot.type_count != before_snapshot.type_count) {
        return Fail("capacity overflow changed type count");
    }

    return 0;
}

int ObjectTypeCapacityOverflowDoesNotMutate() {
    ObjectRegistry registry(ObjectRegistryDesc{4U, 1U});
    const ObjectRegistrationResult first_result = Create(registry, TYPE_ACTOR);
    if (!first_result.Succeeded()) {
        return Fail("first object creation failed");
    }

    const ObjectSnapshot before_snapshot = registry.Snapshot();
    const ObjectRegistrationResult second_result = Create(registry, TYPE_CAMERA);
    if (second_result.status != ObjectStatus::CapacityExceeded) {
        return Fail("type capacity overflow did not return explicit status");
    }

    if (second_result.handle.IsValid()) {
        return Fail("type capacity overflow returned a valid handle");
    }

    const ObjectSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.type_count != before_snapshot.type_count) {
        return Fail("type capacity overflow changed type count");
    }

    if (after_snapshot.alive_object_count != before_snapshot.alive_object_count) {
        return Fail("type capacity overflow changed alive object count");
    }

    if (after_snapshot.created_object_count != before_snapshot.created_object_count) {
        return Fail("type capacity overflow changed created object count");
    }

    const ObjectRegistrationResult retry_result = Create(registry, TYPE_ACTOR);
    if (!retry_result.Succeeded()) {
        return Fail(TYPE_CAPACITY_RETRY_CREATE_MESSAGE);
    }

    if (retry_result.handle.slot != 1U) {
        return Fail(TYPE_CAPACITY_RETRY_SLOT_MESSAGE);
    }

    if (retry_result.handle.generation != 1U) {
        return Fail(TYPE_CAPACITY_RETRY_GENERATION_MESSAGE);
    }

    return 0;
}

int ObjectValidateRejectsInvalidOrStaleHandle() {
    ObjectRegistry registry;
    if (registry.Validate(ObjectHandle{}) != ObjectStatus::InvalidHandle) {
        return Fail("invalid handle did not return explicit status");
    }

    const ObjectHandle unused_handle{0U, 1U};
    if (registry.Validate(unused_handle) != ObjectStatus::InvalidHandle) {
        return Fail(UNUSED_VALIDATE_MESSAGE);
    }

    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded()) {
        return Fail("create failed");
    }

    if (registry.Destroy(result.handle) != ObjectStatus::Success) {
        return Fail("destroy failed");
    }

    if (registry.Validate(result.handle) != ObjectStatus::GenerationMismatch) {
        return Fail("stale handle did not return generation mismatch");
    }

    const ObjectHandle destroyed_handle{result.handle.slot, result.handle.generation + 1U};
    if (registry.Validate(destroyed_handle) != ObjectStatus::InvalidHandle) {
        return Fail(DESTROYED_VALIDATE_MESSAGE);
    }

    return 0;
}

int ObjectInvalidOrStaleHandleOperationsReturnExplicitStatusWithoutMutation() {
    ObjectRegistry registry;
    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded()) {
        return Fail("create failed");
    }

    if (registry.Destroy(result.handle) != ObjectStatus::Success) {
        return Fail("destroy failed");
    }

    const ObjectSnapshot before_snapshot = registry.Snapshot();
    const ObjectHandle unused_handle{1U, 1U};
    if (registry.Acquire(unused_handle) != ObjectStatus::InvalidHandle) {
        return Fail(UNUSED_ACQUIRE_MESSAGE);
    }

    if (registry.Release(unused_handle) != ObjectStatus::InvalidHandle) {
        return Fail(UNUSED_RELEASE_MESSAGE);
    }

    if (registry.Destroy(unused_handle) != ObjectStatus::InvalidHandle) {
        return Fail(UNUSED_DESTROY_MESSAGE);
    }

    const ObjectHandle destroyed_handle{result.handle.slot, result.handle.generation + 1U};
    if (registry.Acquire(destroyed_handle) != ObjectStatus::InvalidHandle) {
        return Fail(DESTROYED_ACQUIRE_MESSAGE);
    }

    if (registry.Release(destroyed_handle) != ObjectStatus::InvalidHandle) {
        return Fail(DESTROYED_RELEASE_MESSAGE);
    }

    if (registry.Destroy(destroyed_handle) != ObjectStatus::InvalidHandle) {
        return Fail(DESTROYED_DESTROY_MESSAGE);
    }

    if (registry.Acquire(result.handle) != ObjectStatus::GenerationMismatch) {
        return Fail("stale acquire did not return explicit status");
    }

    if (registry.Release(result.handle) != ObjectStatus::GenerationMismatch) {
        return Fail("stale release did not return explicit status");
    }

    if (registry.Destroy(result.handle) != ObjectStatus::GenerationMismatch) {
        return Fail("stale destroy did not return explicit status");
    }

    const ObjectSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.alive_object_count != before_snapshot.alive_object_count) {
        return Fail("stale operations changed alive object count");
    }

    if (after_snapshot.destroyed_object_count != before_snapshot.destroyed_object_count) {
        return Fail("stale operations changed destroyed object count");
    }

    if (after_snapshot.referenced_object_count != before_snapshot.referenced_object_count) {
        return Fail("stale operations changed reference count");
    }

    return 0;
}

int ObjectDestroyIncrementsGenerationAndInvalidatesOldHandle() {
    ObjectRegistry registry;
    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded()) {
        return Fail("create failed");
    }

    if (registry.Destroy(result.handle) != ObjectStatus::Success) {
        return Fail("destroy failed");
    }

    const ObjectSnapshot snapshot = registry.Snapshot();
    if (snapshot.alive_object_count != 0U) {
        return Fail("destroy did not decrement alive object count");
    }

    if (snapshot.destroyed_object_count != 1U) {
        return Fail("destroyed object count was not recorded");
    }

    if (registry.Validate(result.handle) != ObjectStatus::GenerationMismatch) {
        return Fail("old handle was not invalidated by generation");
    }

    return 0;
}

int ObjectReusesFreedSlotWithNewGeneration() {
    ObjectRegistry registry;
    const ObjectRegistrationResult first_result = Create(registry, TYPE_ACTOR);
    if (!first_result.Succeeded()) {
        return Fail("first create failed");
    }

    if (registry.Destroy(first_result.handle) != ObjectStatus::Success) {
        return Fail("destroy failed");
    }

    const ObjectRegistrationResult second_result = Create(registry, TYPE_ACTOR);
    if (!second_result.Succeeded()) {
        return Fail("second create failed");
    }

    if (second_result.handle.slot != first_result.handle.slot) {
        return Fail("freed slot was not reused");
    }

    if (second_result.handle.generation == first_result.handle.generation) {
        return Fail("reused slot did not receive a new generation");
    }

    return 0;
}

int ObjectAcquireReleaseTracksReferenceCount() {
    ObjectRegistry registry;
    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded()) {
        return Fail("create failed");
    }

    if (registry.Acquire(result.handle) != ObjectStatus::Success) {
        return Fail("acquire failed");
    }

    if (registry.Snapshot().referenced_object_count != 1U) {
        return Fail("acquire did not increment reference count");
    }

    if (registry.Release(result.handle) != ObjectStatus::Success) {
        return Fail("release failed");
    }

    const ObjectSnapshot snapshot = registry.Snapshot();
    if (snapshot.referenced_object_count != 0U) {
        return Fail("release did not decrement reference count");
    }

    if (snapshot.released_reference_count != 1U) {
        return Fail("release count was not recorded");
    }

    return 0;
}

int ObjectRepeatedAcquireIncrementsReferenceCount() {
    ObjectRegistry registry;
    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded()) {
        return Fail("create failed");
    }

    if (registry.Acquire(result.handle) != ObjectStatus::Success) {
        return Fail("first acquire failed");
    }

    if (registry.Acquire(result.handle) != ObjectStatus::Success) {
        return Fail("second acquire failed");
    }

    if (registry.Snapshot().referenced_object_count != 2U) {
        return Fail("repeated acquire did not increment reference count");
    }

    return 0;
}

int ObjectAcquireRejectsReferenceCountOverflow() {
    ObjectRegistry registry;
    const ObjectRegistrationResult result = registry.CreateSyntheticObject(
        DescriptorWithReferenceCount(TYPE_ACTOR, std::numeric_limits<std::uint32_t>::max()));
    if (!result.Succeeded()) {
        return Fail("overflow fixture creation failed");
    }

    const ObjectSnapshot before_snapshot = registry.Snapshot();
    if (registry.Acquire(result.handle) != ObjectStatus::ReferenceCountOverflow) {
        return Fail("reference overflow did not return explicit status");
    }

    if (registry.Snapshot().referenced_object_count != before_snapshot.referenced_object_count) {
        return Fail("reference overflow changed reference count");
    }

    return 0;
}

int ObjectReleaseAtZeroDoesNotMutate() {
    ObjectRegistry registry;
    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded()) {
        return Fail("create failed");
    }

    const ObjectSnapshot before_snapshot = registry.Snapshot();
    if (registry.Release(result.handle) != ObjectStatus::NotAcquired) {
        return Fail("release at zero did not return explicit status");
    }

    const ObjectSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.referenced_object_count != before_snapshot.referenced_object_count) {
        return Fail("release at zero changed reference count");
    }

    if (after_snapshot.released_reference_count != before_snapshot.released_reference_count) {
        return Fail("release at zero changed release count");
    }

    return 0;
}

int ObjectDestroyRejectsOutstandingReference() {
    ObjectRegistry registry;
    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded()) {
        return Fail("create failed");
    }

    if (registry.Acquire(result.handle) != ObjectStatus::Success) {
        return Fail("acquire failed");
    }

    if (registry.Destroy(result.handle) != ObjectStatus::StillReferenced) {
        return Fail("destroy with outstanding reference did not return explicit status");
    }

    if (registry.Snapshot().alive_object_count != 1U) {
        return Fail("rejected destroy changed alive object count");
    }

    return 0;
}

int ObjectRegistrySnapshotReportsCountsAndLastStatus() {
    ObjectRegistry registry(ObjectRegistryDesc{4U, 4U});
    const ObjectRegistrationResult first_result = Create(registry, TYPE_ACTOR);
    const ObjectRegistrationResult second_result = Create(registry, TYPE_CAMERA);
    if (!first_result.Succeeded() || !second_result.Succeeded()) {
        return Fail("fixture creation failed");
    }

    if (registry.Acquire(first_result.handle) != ObjectStatus::Success) {
        return Fail("acquire failed");
    }

    if (registry.Release(first_result.handle) != ObjectStatus::Success) {
        return Fail("release failed");
    }

    if (registry.Destroy(second_result.handle) != ObjectStatus::Success) {
        return Fail("destroy failed");
    }

    if (registry.CreateSyntheticObject(Descriptor(ObjectTypeId{})).status != ObjectStatus::InvalidType) {
        return Fail("invalid type did not fail");
    }

    const ObjectSnapshot snapshot = registry.Snapshot();
    if (snapshot.object_capacity != 4U || snapshot.type_capacity != 4U) {
        return Fail("snapshot did not report capacities");
    }

    if (snapshot.type_count != 2U) {
        return Fail("snapshot did not report type count");
    }

    if (snapshot.created_object_count != 2U || snapshot.alive_object_count != 1U) {
        return Fail("snapshot did not report object counts");
    }

    if (snapshot.referenced_object_count != 0U || snapshot.released_reference_count != 1U) {
        return Fail("snapshot did not report reference counts");
    }

    if (snapshot.destroyed_object_count != 1U) {
        return Fail("snapshot did not report destroyed count");
    }

    if (snapshot.failed_operation_count != 1U) {
        return Fail("snapshot did not report failed operation count");
    }

    if (snapshot.accepted_operation_count != 5U) {
        return Fail(SNAPSHOT_ACCEPTED_OPERATION_MESSAGE);
    }

    if (snapshot.last_status != ObjectStatus::InvalidType) {
        return Fail("snapshot did not report last explicit status");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("snapshot did not report YuMemory accounting vocabulary");
    }

    return 0;
}

int ObjectDisabledDiagnosticsDoesNotChangeResults() {
    ObjectRegistry recording_registry;
    ObjectRegistry disabled_registry;
    const ObjectRegistrationResult recording_result = Create(recording_registry, TYPE_ACTOR);
    const ObjectRegistrationResult disabled_result = Create(disabled_registry, TYPE_ACTOR);
    if (!recording_result.Succeeded() || !disabled_result.Succeeded()) {
        return Fail("fixture creation failed");
    }

    const ObjectStatus recording_acquire = recording_registry.Acquire(recording_result.handle);
    const ObjectStatus disabled_acquire = disabled_registry.Acquire(disabled_result.handle);
    if (recording_acquire != disabled_acquire) {
        return Fail("disabled diagnostics changed acquire status");
    }

    const ObjectStatus recording_release = recording_registry.Release(recording_result.handle);
    const ObjectStatus disabled_release = disabled_registry.Release(disabled_result.handle);
    if (recording_release != disabled_release) {
        return Fail("disabled diagnostics changed release status");
    }

    const ObjectStatus recording_invalid = recording_registry.Release(recording_result.handle);
    const ObjectStatus disabled_invalid = disabled_registry.Release(disabled_result.handle);
    if (recording_invalid != disabled_invalid) {
        return Fail("disabled diagnostics changed failure status");
    }

    if (!RuntimeCountsMatch(recording_registry.Snapshot(), disabled_registry.Snapshot())) {
        return Fail("disabled diagnostics changed object snapshot");
    }

    return 0;
}

int ObjectNoWorldScriptResourceOrGameAdapterDependency() {
    ObjectRegistry registry;
    const ObjectRegistrationResult result = Create(registry, TYPE_EFFECT);
    if (!result.Succeeded()) {
        return Fail("synthetic object creation failed");
    }

    if (registry.Acquire(result.handle) != ObjectStatus::Success) {
        return Fail("synthetic object acquire failed");
    }

    if (registry.Release(result.handle) != ObjectStatus::Success) {
        return Fail("synthetic object release failed");
    }

    if (registry.Destroy(result.handle) != ObjectStatus::Success) {
        return Fail("synthetic object destroy failed");
    }

    if (registry.Snapshot().alive_object_count != 0U) {
        return Fail("synthetic object registry left object scope");
    }

    return 0;
}

int ObjectNoHiddenAllocationUsesYuMemorySignal() {
    ObjectRegistry registry;
    const ObjectSnapshot initial_snapshot = registry.Snapshot();
    if (initial_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("object registry did not expose YuMemory accounting vocabulary");
    }

    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded()) {
        return Fail("create failed");
    }

    if (registry.Acquire(result.handle) != ObjectStatus::Success) {
        return Fail("acquire failed");
    }

    if (registry.Release(result.handle) != ObjectStatus::Success) {
        return Fail("release failed");
    }

    if (registry.Destroy(result.handle) != ObjectStatus::Success) {
        return Fail("destroy failed");
    }

    const ObjectSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.object_capacity != initial_snapshot.object_capacity) {
        return Fail("object capacity changed during lifecycle fixture");
    }

    if (after_snapshot.type_capacity != initial_snapshot.type_capacity) {
        return Fail("type capacity changed during lifecycle fixture");
    }

    if (after_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("object registry changed allocation accounting vocabulary");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_CREATE, ObjectCreateSyntheticObjectReturnsGenerationHandle},
        {TEST_INVALID_TYPE, ObjectCreateRejectsInvalidTypeWithoutMutation},
        {TEST_CAPACITY, ObjectRegistryCapacityOverflowDoesNotMutate},
        {TEST_TYPE_CAPACITY, ObjectTypeCapacityOverflowDoesNotMutate},
        {TEST_VALIDATE_STALE, ObjectValidateRejectsInvalidOrStaleHandle},
        {TEST_STALE_OPERATIONS, ObjectInvalidOrStaleHandleOperationsReturnExplicitStatusWithoutMutation},
        {TEST_DESTROY_GENERATION, ObjectDestroyIncrementsGenerationAndInvalidatesOldHandle},
        {TEST_REUSE_SLOT, ObjectReusesFreedSlotWithNewGeneration},
        {TEST_ACQUIRE_RELEASE, ObjectAcquireReleaseTracksReferenceCount},
        {TEST_REPEATED_ACQUIRE, ObjectRepeatedAcquireIncrementsReferenceCount},
        {TEST_REFERENCE_OVERFLOW, ObjectAcquireRejectsReferenceCountOverflow},
        {TEST_RELEASE_ZERO, ObjectReleaseAtZeroDoesNotMutate},
        {TEST_DESTROY_REFERENCED, ObjectDestroyRejectsOutstandingReference},
        {TEST_SNAPSHOT, ObjectRegistrySnapshotReportsCountsAndLastStatus},
        {TEST_DISABLED_DIAGNOSTICS, ObjectDisabledDiagnosticsDoesNotChangeResults},
        {TEST_NO_FORBIDDEN_DEPENDENCY, ObjectNoWorldScriptResourceOrGameAdapterDependency},
        {TEST_NO_HIDDEN_ALLOCATION, ObjectNoHiddenAllocationUsesYuMemorySignal}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
