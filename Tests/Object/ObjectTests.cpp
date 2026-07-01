// 模块：Tests Object
// 文件：Tests/Object/ObjectTests.cpp

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
constexpr const char* TEST_TYPE_CAPACITY_ENTRY = "Object_TypeCapacityOverflow_RecordsRejectedTypeEntry";
constexpr const char* TEST_CAPACITY_ENTRY = "Object_CapacityOverflowReportsRejectedDescriptor";
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
constexpr const char* TEST_HOT_PATH_VALIDATE = "Object_HotPathValidateSmoke_DoesNotGrowRuntimeCounts";
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

bool DescriptorMatches(
    const ObjectDescriptor &descriptor,
    ObjectTypeId type,
    std::uint32_t initial_reference_count) {
    if (descriptor.type.value != type.value) {
        return false;
    }

    return descriptor.initial_reference_count == initial_reference_count;
}

bool DescriptorCleared(const ObjectDescriptor &descriptor) {
    if (descriptor.type.IsValid()) {
        return false;
    }

    return descriptor.initial_reference_count == 0U;
}

bool CapacityEntryResultMatches(
    const ObjectRegistrationResult &result,
    std::uint32_t object_capacity,
    std::uint32_t current_object_count,
    std::uint32_t required_object_count,
    std::uint32_t type_capacity,
    std::uint32_t current_type_count,
    std::uint32_t required_type_count) {
    if (result.capacity_entry_object_capacity != object_capacity) {
        return false;
    }

    if (result.capacity_entry_object_count != current_object_count) {
        return false;
    }

    if (result.required_object_count != required_object_count) {
        return false;
    }

    if (result.capacity_entry_type_capacity != type_capacity) {
        return false;
    }

    if (result.capacity_entry_type_count != current_type_count) {
        return false;
    }

    return result.required_type_count == required_type_count;
}

bool CapacityEntrySnapshotMatches(
    const ObjectSnapshot &snapshot,
    std::uint32_t object_capacity,
    std::uint32_t current_object_count,
    std::uint32_t required_object_count,
    std::uint32_t type_capacity,
    std::uint32_t current_type_count,
    std::uint32_t required_type_count) {
    if (snapshot.last_capacity_entry_object_capacity != object_capacity) {
        return false;
    }

    if (snapshot.last_capacity_entry_object_count != current_object_count) {
        return false;
    }

    if (snapshot.last_capacity_entry_required_object_count != required_object_count) {
        return false;
    }

    if (snapshot.last_capacity_entry_type_capacity != type_capacity) {
        return false;
    }

    if (snapshot.last_capacity_entry_type_count != current_type_count) {
        return false;
    }

    return snapshot.last_capacity_entry_required_type_count == required_type_count;
}

bool CapacityEntrySnapshotCleared(const ObjectSnapshot &snapshot) {
    if (snapshot.last_capacity_entry_object_capacity != 0U) {
        return false;
    }

    if (snapshot.last_capacity_entry_object_count != 0U) {
        return false;
    }

    if (snapshot.last_capacity_entry_required_object_count != 0U) {
        return false;
    }

    if (snapshot.last_capacity_entry_type_capacity != 0U) {
        return false;
    }

    if (snapshot.last_capacity_entry_type_count != 0U) {
        return false;
    }

    if (snapshot.last_capacity_entry_required_type_count != 0U) {
        return false;
    }

    return DescriptorCleared(snapshot.last_capacity_entry_descriptor);
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

bool TypeCapacitySnapshotFieldsAreClear(const ObjectSnapshot &snapshot) {
    return snapshot.last_failed_type_capacity_type.value == 0U &&
           snapshot.last_failed_type_capacity == 0U &&
           snapshot.last_failed_type_count == 0U &&
           snapshot.last_failed_required_type_count == 0U;
}

bool TypeCapacityResultFieldsAreClear(const ObjectRegistrationResult &result) {
    return result.failed_type_capacity_type.value == 0U &&
           result.failed_type_capacity == 0U &&
           result.current_type_count == 0U;
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

    if (second_result.required_object_count != 2U ||
        second_result.required_type_count != 2U) {
        return Fail("capacity overflow did not report required counts");
    }

    const ObjectSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.alive_object_count != before_snapshot.alive_object_count) {
        return Fail("capacity overflow changed alive object count");
    }

    if (after_snapshot.last_required_object_count != 2U ||
        after_snapshot.last_required_type_count != 2U) {
        return Fail("capacity overflow snapshot required counts mismatch");
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

    if (second_result.required_object_count != 2U ||
        second_result.required_type_count != 2U) {
        return Fail("type capacity overflow did not report required counts");
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

    if (after_snapshot.last_required_object_count != 2U ||
        after_snapshot.last_required_type_count != 2U) {
        return Fail("type capacity overflow snapshot required counts mismatch");
    }

    const ObjectRegistrationResult retry_result = Create(registry, TYPE_ACTOR);
    if (!retry_result.Succeeded()) {
        return Fail(TYPE_CAPACITY_RETRY_CREATE_MESSAGE);
    }

    if (retry_result.required_object_count != 2U ||
        retry_result.required_type_count != 1U) {
        return Fail("type capacity retry required counts mismatch");
    }

    const ObjectSnapshot retry_snapshot = registry.Snapshot();
    if (retry_snapshot.last_required_object_count != 0U ||
        retry_snapshot.last_required_type_count != 0U) {
        return Fail("type capacity retry did not clear snapshot required counts");
    }

    if (retry_result.handle.slot != 1U) {
        return Fail(TYPE_CAPACITY_RETRY_SLOT_MESSAGE);
    }

    if (retry_result.handle.generation != 1U) {
        return Fail(TYPE_CAPACITY_RETRY_GENERATION_MESSAGE);
    }

    return 0;
}

int ObjectTypeCapacityOverflowRecordsRejectedTypeEntry() {
    ObjectRegistry registry(ObjectRegistryDesc{4U, 1U});
    const ObjectRegistrationResult first_result = Create(registry, TYPE_ACTOR);
    if (!first_result.Succeeded()) {
        return Fail("type capacity entry fixture creation failed");
    }

    const ObjectRegistrationResult capacity_result = Create(registry, TYPE_CAMERA);
    if (capacity_result.status != ObjectStatus::CapacityExceeded) {
        return Fail("type capacity entry status changed");
    }

    if (capacity_result.failed_type_capacity_type.value != TYPE_CAMERA.value) {
        return Fail("type capacity result did not record rejected type");
    }

    if (capacity_result.failed_type_capacity != 1U) {
        return Fail("type capacity result did not record type capacity");
    }

    if (capacity_result.current_type_count != 1U) {
        return Fail("type capacity result did not record current type count");
    }

    if (capacity_result.required_type_count != 2U) {
        return Fail("type capacity result did not record required type count");
    }

    const ObjectSnapshot capacity_snapshot = registry.Snapshot();
    if (capacity_snapshot.last_failed_type_capacity_type.value != TYPE_CAMERA.value) {
        return Fail("type capacity snapshot did not record rejected type");
    }

    if (capacity_snapshot.last_failed_type_capacity != 1U) {
        return Fail("type capacity snapshot did not record type capacity");
    }

    if (capacity_snapshot.last_failed_type_count != 1U) {
        return Fail("type capacity snapshot did not record current type count");
    }

    if (capacity_snapshot.last_failed_required_type_count != 2U) {
        return Fail("type capacity snapshot did not record required type count");
    }

    const ObjectRegistrationResult invalid_result = registry.CreateSyntheticObject(Descriptor(ObjectTypeId{}));
    if (invalid_result.status != ObjectStatus::InvalidType) {
        return Fail("invalid type clear status changed");
    }

    if (!TypeCapacityResultFieldsAreClear(invalid_result)) {
        return Fail("invalid type result kept type capacity entry");
    }

    if (!TypeCapacitySnapshotFieldsAreClear(registry.Snapshot())) {
        return Fail("invalid type did not clear type capacity entry");
    }

    const ObjectRegistrationResult second_capacity_result = Create(registry, TYPE_CAMERA);
    if (second_capacity_result.status != ObjectStatus::CapacityExceeded) {
        return Fail("second type capacity status changed");
    }

    const ObjectRegistrationResult retry_result = Create(registry, TYPE_ACTOR);
    if (!retry_result.Succeeded()) {
        return Fail(TYPE_CAPACITY_RETRY_CREATE_MESSAGE);
    }

    if (!TypeCapacityResultFieldsAreClear(retry_result)) {
        return Fail("type capacity retry result kept rejected type");
    }

    if (!TypeCapacitySnapshotFieldsAreClear(registry.Snapshot())) {
        return Fail("type capacity retry did not clear rejected type");
    }

    ObjectRegistry object_capacity_registry(ObjectRegistryDesc{1U, 2U});
    const ObjectRegistrationResult object_capacity_first = Create(object_capacity_registry, TYPE_ACTOR);
    if (!object_capacity_first.Succeeded()) {
        return Fail("object capacity entry fixture creation failed");
    }

    const ObjectRegistrationResult object_capacity_result = Create(object_capacity_registry, TYPE_CAMERA);
    if (object_capacity_result.status != ObjectStatus::CapacityExceeded) {
        return Fail("object capacity entry status changed");
    }

    if (!TypeCapacityResultFieldsAreClear(object_capacity_result)) {
        return Fail("object capacity result recorded type capacity entry");
    }

    if (!TypeCapacitySnapshotFieldsAreClear(object_capacity_registry.Snapshot())) {
        return Fail("object capacity snapshot recorded type capacity entry");
    }

    return 0;
}

int ObjectCapacityOverflowReportsRejectedDescriptor() {
    ObjectRegistry object_registry(ObjectRegistryDesc{1U, 4U});
    const ObjectRegistrationResult first_object_result = Create(object_registry, TYPE_ACTOR);
    if (!first_object_result.Succeeded()) {
        return Fail("object capacity fixture creation failed");
    }

    const ObjectDescriptor object_failure_descriptor = DescriptorWithReferenceCount(TYPE_CAMERA, 7U);
    const ObjectRegistrationResult object_failure_result =
        object_registry.CreateSyntheticObject(object_failure_descriptor);
    if (object_failure_result.status != ObjectStatus::CapacityExceeded) {
        return Fail("object capacity entry did not return explicit status");
    }

    if (!DescriptorMatches(object_failure_result.capacity_entry_descriptor, TYPE_CAMERA, 7U)) {
        return Fail("object capacity result did not report failed descriptor");
    }

    if (!CapacityEntryResultMatches(object_failure_result, 1U, 1U, 2U, 4U, 1U, 2U)) {
        return Fail("object capacity result did not report capacity entry counts");
    }

    const ObjectSnapshot object_failure_snapshot = object_registry.Snapshot();
    if (!DescriptorMatches(object_failure_snapshot.last_capacity_entry_descriptor, TYPE_CAMERA, 7U)) {
        return Fail("object capacity snapshot did not report failed descriptor");
    }

    if (!CapacityEntrySnapshotMatches(object_failure_snapshot, 1U, 1U, 2U, 4U, 1U, 2U)) {
        return Fail("object capacity snapshot did not report capacity entry counts");
    }

    const ObjectDescriptor invalid_descriptor = Descriptor(ObjectTypeId{});
    const ObjectRegistrationResult invalid_result =
        object_registry.CreateSyntheticObject(invalid_descriptor);
    if (invalid_result.status != ObjectStatus::InvalidType) {
        return Fail("invalid type after capacity did not return explicit status");
    }

    const ObjectSnapshot invalid_snapshot = object_registry.Snapshot();
    if (!CapacityEntrySnapshotCleared(invalid_snapshot)) {
        return Fail("invalid failure did not clear failed descriptor");
    }

    ObjectRegistry type_registry(ObjectRegistryDesc{4U, 1U});
    const ObjectRegistrationResult first_type_result = Create(type_registry, TYPE_ACTOR);
    if (!first_type_result.Succeeded()) {
        return Fail("type capacity fixture creation failed");
    }

    const ObjectDescriptor type_failure_descriptor = DescriptorWithReferenceCount(TYPE_CAMERA, 9U);
    const ObjectRegistrationResult type_failure_result =
        type_registry.CreateSyntheticObject(type_failure_descriptor);
    if (type_failure_result.status != ObjectStatus::CapacityExceeded) {
        return Fail("type capacity entry did not return explicit status");
    }

    if (!DescriptorMatches(type_failure_result.capacity_entry_descriptor, TYPE_CAMERA, 9U)) {
        return Fail("type capacity result did not report failed descriptor");
    }

    if (!CapacityEntryResultMatches(type_failure_result, 4U, 1U, 2U, 1U, 1U, 2U)) {
        return Fail("type capacity result did not report capacity entry counts");
    }

    const ObjectSnapshot type_failure_snapshot = type_registry.Snapshot();
    if (!DescriptorMatches(type_failure_snapshot.last_capacity_entry_descriptor, TYPE_CAMERA, 9U)) {
        return Fail("type capacity snapshot did not report failed descriptor");
    }

    if (!CapacityEntrySnapshotMatches(type_failure_snapshot, 4U, 1U, 2U, 1U, 1U, 2U)) {
        return Fail("type capacity snapshot did not report capacity entry counts");
    }

    const ObjectRegistrationResult retry_result = Create(type_registry, TYPE_ACTOR);
    if (!retry_result.Succeeded()) {
        return Fail("success after type capacity failed");
    }

    const ObjectSnapshot retry_snapshot = type_registry.Snapshot();
    if (!DescriptorCleared(retry_result.capacity_entry_descriptor) ||
        !CapacityEntrySnapshotCleared(retry_snapshot)) {
        return Fail("success did not clear failed descriptor");
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

int ObjectHotPathValidateSmokeDoesNotGrowRuntimeCounts() {
    ObjectRegistry registry(ObjectRegistryDesc{4U, 4U});
    const ObjectRegistrationResult first_result = Create(registry, TYPE_ACTOR);
    const ObjectRegistrationResult second_result = Create(registry, TYPE_CAMERA);
    if (!first_result.Succeeded() || !second_result.Succeeded()) {
        return Fail("hot path fixture creation failed");
    }

    const ObjectSnapshot before_snapshot = registry.Snapshot();
    std::uint32_t iteration = 0U;
    while (iteration < 32U) {
        const ObjectStatus first_status = registry.ValidateAcquire(first_result.handle, 0U);
        if (first_status != ObjectStatus::Success) {
            return Fail("hot path first validate acquire failed");
        }

        const ObjectStatus second_status = registry.ValidateAcquire(second_result.handle, 1U);
        if (second_status != ObjectStatus::Success) {
            return Fail("hot path second validate acquire failed");
        }

        ++iteration;
    }

    const ObjectSnapshot after_snapshot = registry.Snapshot();
    if (after_snapshot.object_capacity != before_snapshot.object_capacity) {
        return Fail("hot path changed object capacity");
    }

    if (after_snapshot.type_capacity != before_snapshot.type_capacity) {
        return Fail("hot path changed type capacity");
    }

    if (!RuntimeCountsMatch(before_snapshot, after_snapshot)) {
        return Fail("hot path changed runtime object counts");
    }

    if (after_snapshot.accepted_operation_count != before_snapshot.accepted_operation_count) {
        return Fail("hot path changed accepted operation count");
    }

    if (after_snapshot.failed_operation_count != before_snapshot.failed_operation_count) {
        return Fail("hot path changed failed operation count");
    }

    if (after_snapshot.last_status != before_snapshot.last_status) {
        return Fail("hot path changed last status");
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
        {TEST_TYPE_CAPACITY_ENTRY, ObjectTypeCapacityOverflowRecordsRejectedTypeEntry},
        {TEST_CAPACITY_ENTRY, ObjectCapacityOverflowReportsRejectedDescriptor},
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
        {TEST_NO_HIDDEN_ALLOCATION, ObjectNoHiddenAllocationUsesYuMemorySignal},
        {TEST_HOT_PATH_VALIDATE, ObjectHotPathValidateSmokeDoesNotGrowRuntimeCounts}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
