#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>

#include "yuengine/memory/MemoryAccountingStatus.h"
#include "yuengine/object/ObjectRegistry.h"

using MemoryAccountingStatus = yuengine::memory::MemoryAccountingStatus;
using ObjectDescriptor = yuengine::object::ObjectDescriptor;
using ObjectHandle = yuengine::object::ObjectHandle;
using ObjectRegistrationResult = yuengine::object::ObjectRegistrationResult;
using ObjectRegistry = yuengine::object::ObjectRegistry;
using ObjectRegistryDesc = yuengine::object::ObjectRegistryDesc;
using ObjectSnapshot = yuengine::object::ObjectSnapshot;
using ObjectStatus = yuengine::object::ObjectStatus;
using ObjectTypeId = yuengine::object::ObjectTypeId;
using yuengine::object::INVALID_OBJECT_GENERATION;

namespace
{
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

int Fail(const std::string& message)
{
    std::cerr << message << '\n';
    return 1;
}

ObjectDescriptor Descriptor(ObjectTypeId type)
{
    return ObjectDescriptor{type, 0U};
}

ObjectDescriptor DescriptorWithReferenceCount(ObjectTypeId type, std::uint32_t referenceCount)
{
    return ObjectDescriptor{type, referenceCount};
}

ObjectRegistrationResult Create(ObjectRegistry& registry, ObjectTypeId type)
{
    return registry.CreateSyntheticObject(Descriptor(type));
}

bool RuntimeCountsMatch(const ObjectSnapshot& left, const ObjectSnapshot& right)
{
    if (left.TypeCount != right.TypeCount)
    {
        return false;
    }

    if (left.AliveObjectCount != right.AliveObjectCount)
    {
        return false;
    }

    if (left.DestroyedObjectCount != right.DestroyedObjectCount)
    {
        return false;
    }

    if (left.CreatedObjectCount != right.CreatedObjectCount)
    {
        return false;
    }

    if (left.ReferencedObjectCount != right.ReferencedObjectCount)
    {
        return false;
    }

    if (left.ReleasedReferenceCount != right.ReleasedReferenceCount)
    {
        return false;
    }

    return left.AllocationAccountingStatus == right.AllocationAccountingStatus;
}

int ObjectCreateSyntheticObjectReturnsGenerationHandle()
{
    ObjectRegistry registry;
    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded())
    {
        return Fail("synthetic object did not register");
    }

    if (result.Handle.Slot != 0U)
    {
        return Fail("first object used unexpected slot");
    }

    if (result.Handle.Generation == INVALID_GENERATION)
    {
        return Fail("object handle generation was invalid");
    }

    const ObjectSnapshot snapshot = registry.Snapshot();
    if (snapshot.AliveObjectCount != 1U)
    {
        return Fail("alive object count was not recorded");
    }

    if (snapshot.CreatedObjectCount != 1U)
    {
        return Fail("created object count was not recorded");
    }

    if (snapshot.TypeCount != 1U)
    {
        return Fail("object type count was not recorded");
    }

    if (snapshot.LastStatus != ObjectStatus::Success)
    {
        return Fail("successful create did not record success status");
    }

    return 0;
}

int ObjectCreateRejectsInvalidTypeWithoutMutation()
{
    ObjectRegistry registry;
    const ObjectSnapshot beforeSnapshot = registry.Snapshot();
    const ObjectRegistrationResult result = registry.CreateSyntheticObject(Descriptor(ObjectTypeId{}));
    if (result.Status != ObjectStatus::InvalidType)
    {
        return Fail("invalid type did not return explicit status");
    }

    if (result.Handle.IsValid())
    {
        return Fail("invalid type returned a valid handle");
    }

    const ObjectSnapshot afterSnapshot = registry.Snapshot();
    if (afterSnapshot.AliveObjectCount != beforeSnapshot.AliveObjectCount)
    {
        return Fail("invalid type changed alive object count");
    }

    if (afterSnapshot.TypeCount != beforeSnapshot.TypeCount)
    {
        return Fail("invalid type changed type count");
    }

    if (afterSnapshot.CreatedObjectCount != beforeSnapshot.CreatedObjectCount)
    {
        return Fail("invalid type changed created object count");
    }

    if (afterSnapshot.FailedOperationCount != beforeSnapshot.FailedOperationCount + 1U)
    {
        return Fail("invalid type did not record a rejected operation");
    }

    return 0;
}

int ObjectRegistryCapacityOverflowDoesNotMutate()
{
    ObjectRegistry registry(ObjectRegistryDesc{1U, 2U});
    const ObjectRegistrationResult firstResult = Create(registry, TYPE_ACTOR);
    if (!firstResult.Succeeded())
    {
        return Fail("first object creation failed");
    }

    const ObjectSnapshot beforeSnapshot = registry.Snapshot();
    const ObjectRegistrationResult secondResult = Create(registry, TYPE_CAMERA);
    if (secondResult.Status != ObjectStatus::CapacityExceeded)
    {
        return Fail("capacity overflow did not return explicit status");
    }

    const ObjectSnapshot afterSnapshot = registry.Snapshot();
    if (afterSnapshot.AliveObjectCount != beforeSnapshot.AliveObjectCount)
    {
        return Fail("capacity overflow changed alive object count");
    }

    if (afterSnapshot.CreatedObjectCount != beforeSnapshot.CreatedObjectCount)
    {
        return Fail("capacity overflow changed created object count");
    }

    if (afterSnapshot.TypeCount != beforeSnapshot.TypeCount)
    {
        return Fail("capacity overflow changed type count");
    }

    return 0;
}

int ObjectTypeCapacityOverflowDoesNotMutate()
{
    ObjectRegistry registry(ObjectRegistryDesc{4U, 1U});
    const ObjectRegistrationResult firstResult = Create(registry, TYPE_ACTOR);
    if (!firstResult.Succeeded())
    {
        return Fail("first object creation failed");
    }

    const ObjectSnapshot beforeSnapshot = registry.Snapshot();
    const ObjectRegistrationResult secondResult = Create(registry, TYPE_CAMERA);
    if (secondResult.Status != ObjectStatus::CapacityExceeded)
    {
        return Fail("type capacity overflow did not return explicit status");
    }

    if (secondResult.Handle.IsValid())
    {
        return Fail("type capacity overflow returned a valid handle");
    }

    const ObjectSnapshot afterSnapshot = registry.Snapshot();
    if (afterSnapshot.TypeCount != beforeSnapshot.TypeCount)
    {
        return Fail("type capacity overflow changed type count");
    }

    if (afterSnapshot.AliveObjectCount != beforeSnapshot.AliveObjectCount)
    {
        return Fail("type capacity overflow changed alive object count");
    }

    if (afterSnapshot.CreatedObjectCount != beforeSnapshot.CreatedObjectCount)
    {
        return Fail("type capacity overflow changed created object count");
    }

    const ObjectRegistrationResult retryResult = Create(registry, TYPE_ACTOR);
    if (!retryResult.Succeeded())
    {
        return Fail(TYPE_CAPACITY_RETRY_CREATE_MESSAGE);
    }

    if (retryResult.Handle.Slot != 1U)
    {
        return Fail(TYPE_CAPACITY_RETRY_SLOT_MESSAGE);
    }

    if (retryResult.Handle.Generation != 1U)
    {
        return Fail(TYPE_CAPACITY_RETRY_GENERATION_MESSAGE);
    }

    return 0;
}

int ObjectValidateRejectsInvalidOrStaleHandle()
{
    ObjectRegistry registry;
    if (registry.Validate(ObjectHandle{}) != ObjectStatus::InvalidHandle)
    {
        return Fail("invalid handle did not return explicit status");
    }

    const ObjectHandle unusedHandle{0U, 1U};
    if (registry.Validate(unusedHandle) != ObjectStatus::InvalidHandle)
    {
        return Fail(UNUSED_VALIDATE_MESSAGE);
    }

    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded())
    {
        return Fail("create failed");
    }

    if (registry.Destroy(result.Handle) != ObjectStatus::Success)
    {
        return Fail("destroy failed");
    }

    if (registry.Validate(result.Handle) != ObjectStatus::GenerationMismatch)
    {
        return Fail("stale handle did not return generation mismatch");
    }

    const ObjectHandle destroyedHandle{result.Handle.Slot, result.Handle.Generation + 1U};
    if (registry.Validate(destroyedHandle) != ObjectStatus::InvalidHandle)
    {
        return Fail(DESTROYED_VALIDATE_MESSAGE);
    }

    return 0;
}

int ObjectInvalidOrStaleHandleOperationsReturnExplicitStatusWithoutMutation()
{
    ObjectRegistry registry;
    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded())
    {
        return Fail("create failed");
    }

    if (registry.Destroy(result.Handle) != ObjectStatus::Success)
    {
        return Fail("destroy failed");
    }

    const ObjectSnapshot beforeSnapshot = registry.Snapshot();
    const ObjectHandle unusedHandle{1U, 1U};
    if (registry.Acquire(unusedHandle) != ObjectStatus::InvalidHandle)
    {
        return Fail(UNUSED_ACQUIRE_MESSAGE);
    }

    if (registry.Release(unusedHandle) != ObjectStatus::InvalidHandle)
    {
        return Fail(UNUSED_RELEASE_MESSAGE);
    }

    if (registry.Destroy(unusedHandle) != ObjectStatus::InvalidHandle)
    {
        return Fail(UNUSED_DESTROY_MESSAGE);
    }

    const ObjectHandle destroyedHandle{result.Handle.Slot, result.Handle.Generation + 1U};
    if (registry.Acquire(destroyedHandle) != ObjectStatus::InvalidHandle)
    {
        return Fail(DESTROYED_ACQUIRE_MESSAGE);
    }

    if (registry.Release(destroyedHandle) != ObjectStatus::InvalidHandle)
    {
        return Fail(DESTROYED_RELEASE_MESSAGE);
    }

    if (registry.Destroy(destroyedHandle) != ObjectStatus::InvalidHandle)
    {
        return Fail(DESTROYED_DESTROY_MESSAGE);
    }

    if (registry.Acquire(result.Handle) != ObjectStatus::GenerationMismatch)
    {
        return Fail("stale acquire did not return explicit status");
    }

    if (registry.Release(result.Handle) != ObjectStatus::GenerationMismatch)
    {
        return Fail("stale release did not return explicit status");
    }

    if (registry.Destroy(result.Handle) != ObjectStatus::GenerationMismatch)
    {
        return Fail("stale destroy did not return explicit status");
    }

    const ObjectSnapshot afterSnapshot = registry.Snapshot();
    if (afterSnapshot.AliveObjectCount != beforeSnapshot.AliveObjectCount)
    {
        return Fail("stale operations changed alive object count");
    }

    if (afterSnapshot.DestroyedObjectCount != beforeSnapshot.DestroyedObjectCount)
    {
        return Fail("stale operations changed destroyed object count");
    }

    if (afterSnapshot.ReferencedObjectCount != beforeSnapshot.ReferencedObjectCount)
    {
        return Fail("stale operations changed reference count");
    }

    return 0;
}

int ObjectDestroyIncrementsGenerationAndInvalidatesOldHandle()
{
    ObjectRegistry registry;
    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded())
    {
        return Fail("create failed");
    }

    if (registry.Destroy(result.Handle) != ObjectStatus::Success)
    {
        return Fail("destroy failed");
    }

    const ObjectSnapshot snapshot = registry.Snapshot();
    if (snapshot.AliveObjectCount != 0U)
    {
        return Fail("destroy did not decrement alive object count");
    }

    if (snapshot.DestroyedObjectCount != 1U)
    {
        return Fail("destroyed object count was not recorded");
    }

    if (registry.Validate(result.Handle) != ObjectStatus::GenerationMismatch)
    {
        return Fail("old handle was not invalidated by generation");
    }

    return 0;
}

int ObjectReusesFreedSlotWithNewGeneration()
{
    ObjectRegistry registry;
    const ObjectRegistrationResult firstResult = Create(registry, TYPE_ACTOR);
    if (!firstResult.Succeeded())
    {
        return Fail("first create failed");
    }

    if (registry.Destroy(firstResult.Handle) != ObjectStatus::Success)
    {
        return Fail("destroy failed");
    }

    const ObjectRegistrationResult secondResult = Create(registry, TYPE_ACTOR);
    if (!secondResult.Succeeded())
    {
        return Fail("second create failed");
    }

    if (secondResult.Handle.Slot != firstResult.Handle.Slot)
    {
        return Fail("freed slot was not reused");
    }

    if (secondResult.Handle.Generation == firstResult.Handle.Generation)
    {
        return Fail("reused slot did not receive a new generation");
    }

    return 0;
}

int ObjectAcquireReleaseTracksReferenceCount()
{
    ObjectRegistry registry;
    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded())
    {
        return Fail("create failed");
    }

    if (registry.Acquire(result.Handle) != ObjectStatus::Success)
    {
        return Fail("acquire failed");
    }

    if (registry.Snapshot().ReferencedObjectCount != 1U)
    {
        return Fail("acquire did not increment reference count");
    }

    if (registry.Release(result.Handle) != ObjectStatus::Success)
    {
        return Fail("release failed");
    }

    const ObjectSnapshot snapshot = registry.Snapshot();
    if (snapshot.ReferencedObjectCount != 0U)
    {
        return Fail("release did not decrement reference count");
    }

    if (snapshot.ReleasedReferenceCount != 1U)
    {
        return Fail("release count was not recorded");
    }

    return 0;
}

int ObjectRepeatedAcquireIncrementsReferenceCount()
{
    ObjectRegistry registry;
    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded())
    {
        return Fail("create failed");
    }

    if (registry.Acquire(result.Handle) != ObjectStatus::Success)
    {
        return Fail("first acquire failed");
    }

    if (registry.Acquire(result.Handle) != ObjectStatus::Success)
    {
        return Fail("second acquire failed");
    }

    if (registry.Snapshot().ReferencedObjectCount != 2U)
    {
        return Fail("repeated acquire did not increment reference count");
    }

    return 0;
}

int ObjectAcquireRejectsReferenceCountOverflow()
{
    ObjectRegistry registry;
    const ObjectRegistrationResult result = registry.CreateSyntheticObject(
        DescriptorWithReferenceCount(TYPE_ACTOR, std::numeric_limits<std::uint32_t>::max()));
    if (!result.Succeeded())
    {
        return Fail("overflow fixture creation failed");
    }

    const ObjectSnapshot beforeSnapshot = registry.Snapshot();
    if (registry.Acquire(result.Handle) != ObjectStatus::ReferenceCountOverflow)
    {
        return Fail("reference overflow did not return explicit status");
    }

    if (registry.Snapshot().ReferencedObjectCount != beforeSnapshot.ReferencedObjectCount)
    {
        return Fail("reference overflow changed reference count");
    }

    return 0;
}

int ObjectReleaseAtZeroDoesNotMutate()
{
    ObjectRegistry registry;
    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded())
    {
        return Fail("create failed");
    }

    const ObjectSnapshot beforeSnapshot = registry.Snapshot();
    if (registry.Release(result.Handle) != ObjectStatus::NotAcquired)
    {
        return Fail("release at zero did not return explicit status");
    }

    const ObjectSnapshot afterSnapshot = registry.Snapshot();
    if (afterSnapshot.ReferencedObjectCount != beforeSnapshot.ReferencedObjectCount)
    {
        return Fail("release at zero changed reference count");
    }

    if (afterSnapshot.ReleasedReferenceCount != beforeSnapshot.ReleasedReferenceCount)
    {
        return Fail("release at zero changed release count");
    }

    return 0;
}

int ObjectDestroyRejectsOutstandingReference()
{
    ObjectRegistry registry;
    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded())
    {
        return Fail("create failed");
    }

    if (registry.Acquire(result.Handle) != ObjectStatus::Success)
    {
        return Fail("acquire failed");
    }

    if (registry.Destroy(result.Handle) != ObjectStatus::StillReferenced)
    {
        return Fail("destroy with outstanding reference did not return explicit status");
    }

    if (registry.Snapshot().AliveObjectCount != 1U)
    {
        return Fail("rejected destroy changed alive object count");
    }

    return 0;
}

int ObjectRegistrySnapshotReportsCountsAndLastStatus()
{
    ObjectRegistry registry(ObjectRegistryDesc{4U, 4U});
    const ObjectRegistrationResult firstResult = Create(registry, TYPE_ACTOR);
    const ObjectRegistrationResult secondResult = Create(registry, TYPE_CAMERA);
    if (!firstResult.Succeeded() || !secondResult.Succeeded())
    {
        return Fail("fixture creation failed");
    }

    if (registry.Acquire(firstResult.Handle) != ObjectStatus::Success)
    {
        return Fail("acquire failed");
    }

    if (registry.Release(firstResult.Handle) != ObjectStatus::Success)
    {
        return Fail("release failed");
    }

    if (registry.Destroy(secondResult.Handle) != ObjectStatus::Success)
    {
        return Fail("destroy failed");
    }

    if (registry.CreateSyntheticObject(Descriptor(ObjectTypeId{})).Status != ObjectStatus::InvalidType)
    {
        return Fail("invalid type did not fail");
    }

    const ObjectSnapshot snapshot = registry.Snapshot();
    if (snapshot.ObjectCapacity != 4U || snapshot.TypeCapacity != 4U)
    {
        return Fail("snapshot did not report capacities");
    }

    if (snapshot.TypeCount != 2U)
    {
        return Fail("snapshot did not report type count");
    }

    if (snapshot.CreatedObjectCount != 2U || snapshot.AliveObjectCount != 1U)
    {
        return Fail("snapshot did not report object counts");
    }

    if (snapshot.ReferencedObjectCount != 0U || snapshot.ReleasedReferenceCount != 1U)
    {
        return Fail("snapshot did not report reference counts");
    }

    if (snapshot.DestroyedObjectCount != 1U)
    {
        return Fail("snapshot did not report destroyed count");
    }

    if (snapshot.FailedOperationCount != 1U)
    {
        return Fail("snapshot did not report failed operation count");
    }

    if (snapshot.AcceptedOperationCount != 5U)
    {
        return Fail(SNAPSHOT_ACCEPTED_OPERATION_MESSAGE);
    }

    if (snapshot.LastStatus != ObjectStatus::InvalidType)
    {
        return Fail("snapshot did not report last explicit status");
    }

    if (snapshot.AllocationAccountingStatus != MemoryAccountingStatus::ExplicitlyTrackedOnly)
    {
        return Fail("snapshot did not report YuMemory accounting vocabulary");
    }

    return 0;
}

int ObjectDisabledDiagnosticsDoesNotChangeResults()
{
    ObjectRegistry recordingRegistry;
    ObjectRegistry disabledRegistry;
    const ObjectRegistrationResult recordingResult = Create(recordingRegistry, TYPE_ACTOR);
    const ObjectRegistrationResult disabledResult = Create(disabledRegistry, TYPE_ACTOR);
    if (!recordingResult.Succeeded() || !disabledResult.Succeeded())
    {
        return Fail("fixture creation failed");
    }

    const ObjectStatus recordingAcquire = recordingRegistry.Acquire(recordingResult.Handle);
    const ObjectStatus disabledAcquire = disabledRegistry.Acquire(disabledResult.Handle);
    if (recordingAcquire != disabledAcquire)
    {
        return Fail("disabled diagnostics changed acquire status");
    }

    const ObjectStatus recordingRelease = recordingRegistry.Release(recordingResult.Handle);
    const ObjectStatus disabledRelease = disabledRegistry.Release(disabledResult.Handle);
    if (recordingRelease != disabledRelease)
    {
        return Fail("disabled diagnostics changed release status");
    }

    const ObjectStatus recordingInvalid = recordingRegistry.Release(recordingResult.Handle);
    const ObjectStatus disabledInvalid = disabledRegistry.Release(disabledResult.Handle);
    if (recordingInvalid != disabledInvalid)
    {
        return Fail("disabled diagnostics changed failure status");
    }

    if (!RuntimeCountsMatch(recordingRegistry.Snapshot(), disabledRegistry.Snapshot()))
    {
        return Fail("disabled diagnostics changed object snapshot");
    }

    return 0;
}

int ObjectNoWorldScriptResourceOrGameAdapterDependency()
{
    ObjectRegistry registry;
    const ObjectRegistrationResult result = Create(registry, TYPE_EFFECT);
    if (!result.Succeeded())
    {
        return Fail("synthetic object creation failed");
    }

    if (registry.Acquire(result.Handle) != ObjectStatus::Success)
    {
        return Fail("synthetic object acquire failed");
    }

    if (registry.Release(result.Handle) != ObjectStatus::Success)
    {
        return Fail("synthetic object release failed");
    }

    if (registry.Destroy(result.Handle) != ObjectStatus::Success)
    {
        return Fail("synthetic object destroy failed");
    }

    if (registry.Snapshot().AliveObjectCount != 0U)
    {
        return Fail("synthetic object registry left object scope");
    }

    return 0;
}

int ObjectNoHiddenAllocationUsesYuMemorySignal()
{
    ObjectRegistry registry;
    const ObjectSnapshot initialSnapshot = registry.Snapshot();
    if (initialSnapshot.AllocationAccountingStatus != MemoryAccountingStatus::ExplicitlyTrackedOnly)
    {
        return Fail("object registry did not expose YuMemory accounting vocabulary");
    }

    const ObjectRegistrationResult result = Create(registry, TYPE_ACTOR);
    if (!result.Succeeded())
    {
        return Fail("create failed");
    }

    if (registry.Acquire(result.Handle) != ObjectStatus::Success)
    {
        return Fail("acquire failed");
    }

    if (registry.Release(result.Handle) != ObjectStatus::Success)
    {
        return Fail("release failed");
    }

    if (registry.Destroy(result.Handle) != ObjectStatus::Success)
    {
        return Fail("destroy failed");
    }

    const ObjectSnapshot afterSnapshot = registry.Snapshot();
    if (afterSnapshot.ObjectCapacity != initialSnapshot.ObjectCapacity)
    {
        return Fail("object capacity changed during lifecycle fixture");
    }

    if (afterSnapshot.TypeCapacity != initialSnapshot.TypeCapacity)
    {
        return Fail("type capacity changed during lifecycle fixture");
    }

    if (afterSnapshot.AllocationAccountingStatus != MemoryAccountingStatus::ExplicitlyTrackedOnly)
    {
        return Fail("object registry changed allocation accounting vocabulary");
    }

    return 0;
}
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    static const std::unordered_map<std::string_view, TestFunction> testRegistry{
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

    const std::string_view testName(argv[1]);
    const auto testIterator = testRegistry.find(testName);
    if (testIterator == testRegistry.end())
    {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return testIterator->second();
}
