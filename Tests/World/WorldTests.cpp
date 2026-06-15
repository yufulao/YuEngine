// Module: Tests World
// File: Tests/World/WorldTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "YuEngine/Kernel/EngineKernel.h"
#include "YuEngine/Kernel/KernelHostRuntime.h"
#include "YuEngine/Kernel/KernelStatus.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Object/ObjectDescriptor.h"
#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/Object/ObjectRegistrationResult.h"
#include "YuEngine/Object/ObjectRegistry.h"
#include "YuEngine/Object/ObjectRegistryDesc.h"
#include "YuEngine/Object/ObjectSnapshot.h"
#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/Object/ObjectTypeId.h"
#include "YuEngine/Platform/FixedFrameClock.h"
#include "YuEngine/Platform/HeadlessHost.h"
#include "YuEngine/Platform/HeadlessHostConfig.h"
#include "YuEngine/Platform/HostStatus.h"
#include "YuEngine/Script/ScriptCallId.h"
#include "YuEngine/Script/ScriptNativeBinding.h"
#include "YuEngine/Script/ScriptNativeRegistry.h"
#include "YuEngine/Script/ScriptStatus.h"
#include "YuEngine/Script/ScriptValue.h"
#include "YuEngine/Script/ScriptValueType.h"
#include "YuEngine/Serialize/SerializeConstants.h"
#include "YuEngine/Serialize/SerializeReader.h"
#include "YuEngine/Serialize/SerializeSnapshot.h"
#include "YuEngine/Serialize/SerializeStatus.h"
#include "YuEngine/Serialize/SerializeWriter.h"
#include "YuEngine/Resource/ResourceDescriptor.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceLogicalKey.h"
#include "YuEngine/Resource/ResourceRegistrationResult.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/Resource/ResourceRegistryDesc.h"
#include "YuEngine/Resource/ResourceSnapshot.h"
#include "YuEngine/Resource/ResourceStatus.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/World/WorldComponentAttachment.h"
#include "YuEngine/World/WorldComponentAttachmentBridge.h"
#include "YuEngine/World/WorldComponentAttachmentBridgeDesc.h"
#include "YuEngine/World/WorldComponentAttachmentResult.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshot.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotBridge.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotBridgeDesc.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotBridgeSnapshot.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotConstants.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotRecord.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotResult.h"
#include "YuEngine/World/WorldComponentAttachmentSnapshotStatus.h"
#include "YuEngine/World/WorldComponentAttachmentStatus.h"
#include "YuEngine/World/WorldComponentQueryBridge.h"
#include "YuEngine/World/WorldComponentQueryDesc.h"
#include "YuEngine/World/WorldComponentQueryResult.h"
#include "YuEngine/World/WorldComponentQuerySnapshot.h"
#include "YuEngine/World/WorldComponentQueryStatus.h"
#include "YuEngine/World/WorldComponentResourceBinding.h"
#include "YuEngine/World/WorldComponentResourceBindingBridge.h"
#include "YuEngine/World/WorldComponentResourceBindingBridgeDesc.h"
#include "YuEngine/World/WorldComponentResourceBindingResult.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreBridge.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreResult.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreSnapshot.h"
#include "YuEngine/World/WorldComponentResourceBindingRestoreStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshot.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotBridge.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotBridgeDesc.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotBridgeSnapshot.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotConstants.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotRecord.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotResult.h"
#include "YuEngine/World/WorldComponentResourceBindingSnapshotStatus.h"
#include "YuEngine/World/WorldComponentResourceBindingStatus.h"
#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldDesc.h"
#include "YuEngine/World/WorldInstance.h"
#include "YuEngine/World/WorldKernelModule.h"
#include "YuEngine/World/WorldKernelModuleDesc.h"
#include "YuEngine/World/WorldLifecycleState.h"
#include "YuEngine/World/WorldObjectDesc.h"
#include "YuEngine/World/WorldPhaseTrace.h"
#include "YuEngine/World/WorldObjectIdentityBridge.h"
#include "YuEngine/World/WorldObjectIdentityResult.h"
#include "YuEngine/World/WorldObjectIdentitySnapshot.h"
#include "YuEngine/World/WorldObjectIdentityStatus.h"
#include "YuEngine/World/WorldRegistrationResult.h"
#include "YuEngine/World/WorldResourceBindingBridge.h"
#include "YuEngine/World/WorldResourceBindingBridgeDesc.h"
#include "YuEngine/World/WorldResourceBindingResult.h"
#include "YuEngine/World/WorldResourceBindingSnapshot.h"
#include "YuEngine/World/WorldResourceBindingStatus.h"
#include "YuEngine/World/WorldScriptDispatchBridge.h"
#include "YuEngine/World/WorldScriptDispatchBridgeDesc.h"
#include "YuEngine/World/WorldScriptDispatchConstants.h"
#include "YuEngine/World/WorldScriptDispatchResult.h"
#include "YuEngine/World/WorldScriptDispatchSnapshot.h"
#include "YuEngine/World/WorldScriptDispatchStatus.h"
#include "YuEngine/World/WorldSerializeSnapshotBridge.h"
#include "YuEngine/World/WorldSerializeSnapshotBridgeDesc.h"
#include "YuEngine/World/WorldSerializeSnapshotBridgeSnapshot.h"
#include "YuEngine/World/WorldSerializeSnapshotConstants.h"
#include "YuEngine/World/WorldSerializeSnapshotResult.h"
#include "YuEngine/World/WorldSerializeSnapshotState.h"
#include "YuEngine/World/WorldSerializeSnapshotStatus.h"
#include "YuEngine/World/WorldSceneAssemblyBridge.h"
#include "YuEngine/World/WorldSceneAssemblyBridgeDesc.h"
#include "YuEngine/World/WorldSceneAssemblyResult.h"
#include "YuEngine/World/WorldSceneAssemblySnapshot.h"
#include "YuEngine/World/WorldSceneAssemblyStatus.h"
#include "YuEngine/World/WorldSnapshot.h"
#include "YuEngine/World/WorldStatus.h"
#include "YuEngine/World/WorldTransformBridge.h"
#include "YuEngine/World/WorldTransformBridgeDesc.h"
#include "YuEngine/World/WorldTransformResult.h"
#include "YuEngine/World/WorldTransformSnapshot.h"
#include "YuEngine/World/WorldTransformState.h"
#include "YuEngine/World/WorldTransformStatus.h"
#include "YuEngine/World/WorldUpdatePhase.h"
#include "YuEngine/World/WorldServiceIds.h"

using yuengine::kernel::EngineKernel;
using yuengine::kernel::KernelHostRuntime;
using yuengine::kernel::KernelStatus;
using yuengine::memory::MemoryAccountingStatus;
using yuengine::object::ObjectDescriptor;
using yuengine::object::ObjectHandle;
using yuengine::object::ObjectRegistrationResult;
using yuengine::object::ObjectRegistry;
using yuengine::object::ObjectRegistryDesc;
using yuengine::object::ObjectSnapshot;
using yuengine::object::ObjectStatus;
using yuengine::object::ObjectTypeId;
using yuengine::platform::FixedFrameClock;
using yuengine::platform::HeadlessHost;
using yuengine::platform::HeadlessHostConfig;
using yuengine::platform::HostStatus;
using yuengine::script::ScriptCallId;
using yuengine::script::ScriptNativeBinding;
using yuengine::script::ScriptNativeRegistry;
using yuengine::script::ScriptStatus;
using yuengine::script::ScriptValue;
using yuengine::script::ScriptValueType;
using yuengine::serialize::MAX_STREAM_BYTE_COUNT;
using yuengine::serialize::SerializeReader;
using yuengine::serialize::SerializeSnapshot;
using yuengine::serialize::SerializeStatus;
using yuengine::serialize::SerializeWriter;
using yuengine::serialize::STREAM_HEADER_BYTE_COUNT;
using yuengine::resource::ResourceDescriptor;
using yuengine::resource::ResourceHandle;
using yuengine::resource::ResourceLogicalKey;
using yuengine::resource::ResourceRegistrationResult;
using yuengine::resource::ResourceRegistry;
using yuengine::resource::ResourceRegistryDesc;
using yuengine::resource::ResourceSnapshot;
using yuengine::resource::ResourceStatus;
using yuengine::resource::ResourceTypeId;
using yuengine::world::MAX_WORLD_OBJECT_COUNT;
using yuengine::world::MAX_WORLD_PHASE_TRACE_COUNT;
using yuengine::world::MAX_WORLD_SERIALIZE_PHASE_TRACE_COUNT;
using yuengine::world::MAX_WORLD_SCRIPT_DISPATCH_BINDING_COUNT;
using yuengine::world::WORLD_UPDATE_PHASE_COUNT;
using yuengine::world::WORLD_INSTANCE_SERVICE_ID;
using yuengine::world::WORLD_KERNEL_MODULE_NAME;
using yuengine::world::WORLD_SERIALIZE_FIELD_ACTIVE_OBJECT_COUNT;
using yuengine::world::WORLD_SERIALIZE_FIELD_ALLOCATION_STATUS;
using yuengine::world::WORLD_SERIALIZE_FIELD_FRAME_COUNT;
using yuengine::world::WORLD_SERIALIZE_FIELD_LAST_FIXED_STEP_DURATION;
using yuengine::world::WORLD_SERIALIZE_FIELD_LAST_FRAME_DELTA_DURATION;
using yuengine::world::WORLD_SERIALIZE_FIELD_LAST_FRAME_INDEX;
using yuengine::world::WORLD_SERIALIZE_FIELD_LAST_STATUS;
using yuengine::world::WORLD_SERIALIZE_FIELD_LIFECYCLE_STATE;
using yuengine::world::WORLD_SERIALIZE_FIELD_OBJECT_CAPACITY;
using yuengine::world::WORLD_SERIALIZE_FIELD_PHASE_EXECUTION_COUNT;
using yuengine::world::WORLD_SERIALIZE_FIELD_PHASE_TRACE_CAPACITY;
using yuengine::world::WORLD_SERIALIZE_FIELD_PHASE_TRACE_COUNT;
using yuengine::world::WORLD_SERIALIZE_FIELD_REGISTERED_OBJECT_COUNT;
using yuengine::world::WORLD_SERIALIZE_FIELD_SKIPPED_OBJECT_COUNT;
using yuengine::world::WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID;
using yuengine::world::WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_FIELD_RECORD_BYTES;
using yuengine::world::WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_PAYLOAD_BYTE_COUNT;
using yuengine::world::WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_RECORD_CAPACITY;
using yuengine::world::WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_RECORD_ID_BASE;
using yuengine::world::WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_FIELD_CHUNK_COUNT;
using yuengine::world::WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_FIELD_RECORD_COUNT;
using yuengine::world::WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_FIELD_SCHEMA_VERSION;
using yuengine::world::WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_METADATA_RECORD_ID;
using yuengine::world::WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_RECORD_BYTE_COUNT;
using yuengine::world::WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_SCHEMA_VERSION;
using yuengine::world::WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_FIELD_RECORD_BYTES;
using yuengine::world::WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_PAYLOAD_BYTE_COUNT;
using yuengine::world::WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_RECORD_CAPACITY;
using yuengine::world::WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_RECORD_ID_BASE;
using yuengine::world::WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_FIELD_CHUNK_COUNT;
using yuengine::world::WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_FIELD_RECORD_COUNT;
using yuengine::world::WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_FIELD_SCHEMA_VERSION;
using yuengine::world::WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_METADATA_RECORD_ID;
using yuengine::world::WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_RECORD_BYTE_COUNT;
using yuengine::world::WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_SCHEMA_VERSION;
using yuengine::world::WorldComponentAttachmentBridge;
using yuengine::world::WorldComponentAttachmentBridgeDesc;
using yuengine::world::WorldComponentAttachment;
using yuengine::world::WorldComponentAttachmentResult;
using yuengine::world::WorldComponentAttachmentSnapshot;
using yuengine::world::WorldComponentAttachmentSnapshotBridge;
using yuengine::world::WorldComponentAttachmentSnapshotBridgeDesc;
using yuengine::world::WorldComponentAttachmentSnapshotBridgeSnapshot;
using yuengine::world::WorldComponentAttachmentSnapshotRecord;
using yuengine::world::WorldComponentAttachmentSnapshotResult;
using yuengine::world::WorldComponentAttachmentSnapshotStatus;
using yuengine::world::WorldComponentAttachmentStatus;
using yuengine::world::WorldComponentSlotId;
using yuengine::world::WorldComponentTypeId;
using yuengine::world::WorldComponentQueryBridge;
using yuengine::world::WorldComponentQueryObjectDesc;
using yuengine::world::WorldComponentQueryResult;
using yuengine::world::WorldComponentQuerySnapshot;
using yuengine::world::WorldComponentQueryStatus;
using yuengine::world::WorldComponentQueryTypeDesc;
using yuengine::world::WorldComponentResourceBinding;
using yuengine::world::WorldComponentResourceBindingBridge;
using yuengine::world::WorldComponentResourceBindingBridgeDesc;
using yuengine::world::WorldComponentResourceBindingResult;
using yuengine::world::WorldComponentResourceBindingRestoreBridge;
using yuengine::world::WorldComponentResourceBindingRestoreResult;
using yuengine::world::WorldComponentResourceBindingRestoreSnapshot;
using yuengine::world::WorldComponentResourceBindingRestoreStatus;
using yuengine::world::WorldComponentResourceBindingSnapshot;
using yuengine::world::WorldComponentResourceBindingSnapshotBridge;
using yuengine::world::WorldComponentResourceBindingSnapshotBridgeDesc;
using yuengine::world::WorldComponentResourceBindingSnapshotBridgeSnapshot;
using yuengine::world::WorldComponentResourceBindingSnapshotRecord;
using yuengine::world::WorldComponentResourceBindingSnapshotResult;
using yuengine::world::WorldComponentResourceBindingSnapshotStatus;
using yuengine::world::WorldComponentResourceBindingStatus;
using yuengine::world::WorldDesc;
using yuengine::world::WorldInstance;
using yuengine::world::WorldKernelModule;
using yuengine::world::WorldKernelModuleDesc;
using yuengine::world::WorldLifecycleState;
using yuengine::world::WorldObjectDesc;
using yuengine::world::WorldObjectId;
using yuengine::world::WorldObjectIdentityBridge;
using yuengine::world::WorldObjectIdentityResult;
using yuengine::world::WorldObjectIdentitySnapshot;
using yuengine::world::WorldObjectIdentityStatus;
using yuengine::world::WorldPhaseTrace;
using yuengine::world::WorldRegistrationResult;
using yuengine::world::WorldResourceBindingBridge;
using yuengine::world::WorldResourceBindingBridgeDesc;
using yuengine::world::WorldResourceBindingResult;
using yuengine::world::WorldResourceBindingSnapshot;
using yuengine::world::WorldResourceBindingStatus;
using yuengine::world::WorldScriptDispatchBridge;
using yuengine::world::WorldScriptDispatchBridgeDesc;
using yuengine::world::WorldScriptDispatchResult;
using yuengine::world::WorldScriptDispatchSnapshot;
using yuengine::world::WorldScriptDispatchStatus;
using yuengine::world::WorldSerializeSnapshotBridge;
using yuengine::world::WorldSerializeSnapshotBridgeDesc;
using yuengine::world::WorldSerializeSnapshotBridgeSnapshot;
using yuengine::world::WorldSerializeSnapshotResult;
using yuengine::world::WorldSerializeSnapshotStatus;
using yuengine::world::WorldSceneAssemblyBridge;
using yuengine::world::WorldSceneAssemblyBridgeDesc;
using yuengine::world::WorldSceneAssemblyResult;
using yuengine::world::WorldSceneAssemblySnapshot;
using yuengine::world::WorldSceneAssemblyStatus;
using yuengine::world::WorldSnapshot;
using yuengine::world::WorldStatus;
using yuengine::world::WorldTransformBridge;
using yuengine::world::WorldTransformBridgeDesc;
using yuengine::world::WorldTransformResult;
using yuengine::world::WorldTransformSnapshot;
using yuengine::world::WorldTransformState;
using yuengine::world::WorldTransformStatus;
using yuengine::world::WorldUpdatePhase;

namespace {
constexpr const char *TEST_CREATE = "World_CreateWithFixedCapacity_ReportsSnapshot";
constexpr const char *TEST_START_STOP = "World_StartStop_RunsDeterministicLifecycle";
constexpr const char *TEST_PHASE_ORDER = "World_UpdateRunsPhasesInFixedOrder";
constexpr const char *TEST_UPDATE_BEFORE_START = "World_UpdateBeforeStart_ReturnsExplicitStatus";
constexpr const char *TEST_UPDATE_AFTER_STOP = "World_UpdateAfterStop_ReturnsExplicitStatus";
constexpr const char *TEST_DUPLICATE = "World_RegisterDuplicateObject_DoesNotMutate";
constexpr const char *TEST_OVERFLOW = "World_RegisterOverflow_DoesNotMutate";
constexpr const char *TEST_DISABLED_SKIP = "World_DisabledObject_IsSkipped";
constexpr const char *TEST_UPDATE_PATH = "World_UpdatePath_DoesNotGrowStorage";
constexpr const char *TEST_STOP_CLEARS = "World_StopClearsActiveEntries";
constexpr const char *TEST_NO_SCRIPT_RESOURCE = "World_NoScriptResourcePackageFileOrGameAdapterDependency";
constexpr const char *TEST_NO_ACTOR_COMPONENT = "World_NoActorComponentOrTransformHierarchy";
constexpr const char *TEST_SNAPSHOT = "World_SnapshotReportsCountsAndLastStatus";
constexpr const char *TEST_MODULE_START_SERVICE = "WorldKernelModule_StartPublishesWorldService";
constexpr const char *TEST_MODULE_UPDATE_ORDER = "WorldKernelModule_UpdateTicksWorldInKernelOrder";
constexpr const char *TEST_MODULE_SHUTDOWN = "WorldKernelModule_ShutdownStopsWorld";
constexpr const char *TEST_MODULE_START_FAILURE = "WorldKernelModule_StartFailurePropagatesExplicitStatus";
constexpr const char *TEST_MODULE_UPDATE_FAILURE = "WorldKernelModule_UpdateFailureTriggersKernelTeardown";
constexpr const char *TEST_MODULE_HEADLESS_HOST = "WorldKernelModule_HeadlessHostDrivesWorldDeterministically";
constexpr const char *TEST_MODULE_UPDATE_PATH = "WorldKernelModule_UpdatePathDoesNotGrowWorldStorage";
constexpr const char *TEST_MODULE_NO_SCRIPT_RESOURCE = "WorldKernelModule_NoScriptResourcePackageFileOrGameAdapterDependency";
constexpr const char *TEST_MODULE_NO_ACTOR_COMPONENT = "WorldKernelModule_NoActorComponentOrTransformHierarchy";
constexpr const char *TEST_MODULE_CORE_KERNEL_FREE = "WorldKernelModule_CoreWorldInstanceRemainsKernelFree";
constexpr const char *TEST_IDENTITY_BIND_VALID = "WorldObjectIdentityBridge_BindValidObject_AcquiresHandle";
constexpr const char *TEST_IDENTITY_INVALID_WORLD_ID = "WorldObjectIdentityBridge_BindRejectsInvalidWorldIdWithoutMutation";
constexpr const char *TEST_IDENTITY_MISSING_WORLD_OBJECT = "WorldObjectIdentityBridge_BindRejectsMissingWorldObjectWithoutMutation";
constexpr const char *TEST_IDENTITY_INVALID_OBJECT_HANDLE = "WorldObjectIdentityBridge_BindRejectsInvalidObjectHandleWithoutMutation";
constexpr const char *TEST_IDENTITY_DUPLICATE_WORLD_ID = "WorldObjectIdentityBridge_BindRejectsDuplicateWorldObjectId";
constexpr const char *TEST_IDENTITY_DUPLICATE_OBJECT_HANDLE = "WorldObjectIdentityBridge_BindRejectsDuplicateObjectHandle";
constexpr const char *TEST_IDENTITY_REMOVE_RELEASES = "WorldObjectIdentityBridge_RemoveReleasesHandle";
constexpr const char *TEST_IDENTITY_CLEAR_RELEASES = "WorldObjectIdentityBridge_ClearReleasesAllHandles";
constexpr const char *TEST_IDENTITY_STALE_GENERATION = "WorldObjectIdentityBridge_StaleGenerationInvalidatesBinding";
constexpr const char *TEST_IDENTITY_UPDATE_PATH = "WorldObjectIdentityBridge_UpdatePathDoesNotGrowWorldStorage";
constexpr const char *TEST_IDENTITY_NO_SCRIPT_RESOURCE = "WorldObjectIdentityBridge_NoScriptResourcePackageFileOrGameAdapterDependency";
constexpr const char *TEST_IDENTITY_NO_ACTOR_COMPONENT = "WorldObjectIdentityBridge_NoActorComponentOrTransformHierarchy";
constexpr const char *TEST_IDENTITY_CORE_OBJECT_FREE = "WorldObjectIdentityBridge_WorldInstanceCoreRemainsObjectFree";
constexpr const char *TEST_TRANSFORM_REGISTER_VALID = "WorldTransformBridge_RegisterValidObject_StoresTransform";
constexpr const char *TEST_TRANSFORM_INVALID_WORLD_ID = "WorldTransformBridge_RegisterRejectsInvalidWorldIdWithoutMutation";
constexpr const char *TEST_TRANSFORM_MISSING_WORLD_OBJECT = "WorldTransformBridge_RegisterRejectsMissingWorldObjectWithoutMutation";
constexpr const char *TEST_TRANSFORM_DUPLICATE_WORLD_ID = "WorldTransformBridge_RegisterRejectsDuplicateWorldObjectId";
constexpr const char *TEST_TRANSFORM_CAPACITY_OVERFLOW = "WorldTransformBridge_RegisterRejectsCapacityOverflowWithoutMutation";
constexpr const char *TEST_TRANSFORM_SET_EXISTING = "WorldTransformBridge_SetUpdatesExistingRecord";
constexpr const char *TEST_TRANSFORM_SET_MISSING = "WorldTransformBridge_SetRejectsMissingRecordWithoutMutation";
constexpr const char *TEST_TRANSFORM_QUERY = "WorldTransformBridge_QueryReturnsStoredTransform";
constexpr const char *TEST_TRANSFORM_REMOVE = "WorldTransformBridge_RemoveClearsRecord";
constexpr const char *TEST_TRANSFORM_CLEAR = "WorldTransformBridge_ClearRemovesAllRecords";
constexpr const char *TEST_TRANSFORM_UPDATE_PATH = "WorldTransformBridge_UpdatePathDoesNotGrowWorldStorage";
constexpr const char *TEST_TRANSFORM_NO_SCRIPT_RESOURCE = "WorldTransformBridge_NoScriptResourcePackageFileObjectOrGameAdapterDependency";
constexpr const char *TEST_TRANSFORM_NO_ACTOR_COMPONENT = "WorldTransformBridge_NoActorComponentSceneGraphOrHierarchy";
constexpr const char *TEST_TRANSFORM_CORE_FREE = "WorldTransformBridge_WorldInstanceCoreRemainsTransformStorageFree";
constexpr const char *TEST_SCRIPT_DISPATCH_BIND_VALID = "WorldScriptDispatchBridge_BindPhaseCall_ReturnsStableBinding";
constexpr const char *TEST_SCRIPT_DISPATCH_INVALID_CALL = "WorldScriptDispatchBridge_BindRejectsInvalidCallIdWithoutMutation";
constexpr const char *TEST_SCRIPT_DISPATCH_DUPLICATE_PHASE = "WorldScriptDispatchBridge_BindRejectsDuplicatePhaseWithoutMutation";
constexpr const char *TEST_SCRIPT_DISPATCH_CAPACITY = "WorldScriptDispatchBridge_BindRejectsCapacityOverflowWithoutMutation";
constexpr const char *TEST_SCRIPT_DISPATCH_ORDER = "WorldScriptDispatchBridge_DispatchTraceInvokesPhasesInTraceOrder";
constexpr const char *TEST_SCRIPT_DISPATCH_SKIP = "WorldScriptDispatchBridge_DispatchSkipsUnboundPhase";
constexpr const char *TEST_SCRIPT_DISPATCH_TRACE_BUFFER = "WorldScriptDispatchBridge_DispatchRejectsInvalidTraceBuffer";
constexpr const char *TEST_SCRIPT_DISPATCH_SLOT_BUFFERS = "WorldScriptDispatchBridge_DispatchRejectsInvalidSlotBuffers";
constexpr const char *TEST_SCRIPT_DISPATCH_SCRIPT_FAILURE = "WorldScriptDispatchBridge_DispatchPropagatesScriptFailure";
constexpr const char *TEST_SCRIPT_DISPATCH_PATH = "WorldScriptDispatchBridge_DispatchPathDoesNotGrowStorage";
constexpr const char *TEST_SCRIPT_DISPATCH_SNAPSHOT = "WorldScriptDispatchBridge_SnapshotReportsCountsAndLastStatus";
constexpr const char *TEST_SCRIPT_DISPATCH_NO_ACTOR_COMPONENT = "WorldScriptDispatchBridge_NoActorComponentSceneGraphOrGameAdapterDependency";
constexpr const char *TEST_SCRIPT_DISPATCH_NO_RESOURCE_OBJECT = "WorldScriptDispatchBridge_NoResourcePackageFileSerializeOrObjectOwnershipDependency";
constexpr const char *TEST_SCRIPT_DISPATCH_WORLD_CORE_FREE = "WorldScriptDispatchBridge_WorldInstanceCoreRemainsScriptFree";
constexpr const char *TEST_SCRIPT_DISPATCH_SCRIPT_CORE_FREE = "WorldScriptDispatchBridge_ScriptRegistryCoreRemainsWorldFree";
constexpr const char *TEST_SERIALIZE_ROUND_TRIP = "WorldSerializeSnapshotBridge_WriteWorldSnapshot_RoundTripsDeterministically";
constexpr const char *TEST_SERIALIZE_TRACE_ORDER = "WorldSerializeSnapshotBridge_WritePhaseTraceRecordsInOrder";
constexpr const char *TEST_SERIALIZE_TRANSFORM = "WorldSerializeSnapshotBridge_WriteOptionalTransformSnapshotCounters";
constexpr const char *TEST_SERIALIZE_SMALL_TRACE_OUTPUT = "WorldSerializeSnapshotBridge_ReadRejectsSmallTraceOutputWithoutOverrun";
constexpr const char *TEST_SERIALIZE_INVALID_TRACE_BUFFER = "WorldSerializeSnapshotBridge_WriteRejectsInvalidTraceBufferWithoutMutation";
constexpr const char *TEST_SERIALIZE_TRACE_OVERFLOW = "WorldSerializeSnapshotBridge_WriteRejectsTraceOverflowWithoutMutation";
constexpr const char *TEST_SERIALIZE_WRITE_FAILURE = "WorldSerializeSnapshotBridge_SerializeFailureMapsExplicitStatus";
constexpr const char *TEST_SERIALIZE_READ_FAILURE = "WorldSerializeSnapshotBridge_ReadFailureMapsExplicitStatus";
constexpr const char *TEST_SERIALIZE_INVALID_ENUM = "WorldSerializeSnapshotBridge_ReadRejectsInvalidEnumValuesWithoutMutation";
constexpr const char *TEST_SERIALIZE_NO_WORLD_MUTATION = "WorldSerializeSnapshotBridge_NoWorldMutationDuringReadWrite";
constexpr const char *TEST_SERIALIZE_PATH = "WorldSerializeSnapshotBridge_ReadWritePathDoesNotGrowStorage";
constexpr const char *TEST_SERIALIZE_SNAPSHOT = "WorldSerializeSnapshotBridge_SnapshotReportsCountsAndLastStatus";
constexpr const char *TEST_SERIALIZE_NO_FILE_PACKAGE = "WorldSerializeSnapshotBridge_NoFilePackageResourceSaveGameOrGameAdapterDependency";
constexpr const char *TEST_SERIALIZE_NO_ACTOR_COMPONENT = "WorldSerializeSnapshotBridge_NoActorComponentSceneGraphOrGameplayDependency";
constexpr const char *TEST_SERIALIZE_WORLD_CORE_FREE = "WorldSerializeSnapshotBridge_WorldInstanceCoreRemainsSerializeFree";
constexpr const char *TEST_SERIALIZE_CORE_FREE = "WorldSerializeSnapshotBridge_SerializeCoreRemainsWorldFree";
constexpr const char *TEST_RESOURCE_BIND_VALID = "WorldResourceBindingBridge_BindValidResource_AcquiresHandle";
constexpr const char *TEST_RESOURCE_BIND_NULL_REGISTRY = "WorldResourceBindingBridge_BindRejectsNullRegistryWithoutMutation";
constexpr const char *TEST_RESOURCE_BIND_INVALID_WORLD = "WorldResourceBindingBridge_BindRejectsInvalidWorldIdWithoutMutation";
constexpr const char *TEST_RESOURCE_BIND_INVALID_HANDLE = "WorldResourceBindingBridge_BindRejectsInvalidResourceHandleWithoutMutation";
constexpr const char *TEST_RESOURCE_BIND_STALE_HANDLE = "WorldResourceBindingBridge_BindRejectsStaleResourceHandleWithoutMutation";
constexpr const char *TEST_RESOURCE_BIND_TYPE_MISMATCH = "WorldResourceBindingBridge_BindRejectsTypeMismatchWithoutMutation";
constexpr const char *TEST_RESOURCE_BIND_DUPLICATE_WORLD = "WorldResourceBindingBridge_BindRejectsDuplicateWorldObjectId";
constexpr const char *TEST_RESOURCE_BIND_CAPACITY = "WorldResourceBindingBridge_BindRejectsCapacityOverflowWithoutMutation";
constexpr const char *TEST_RESOURCE_REMOVE_RELEASES = "WorldResourceBindingBridge_RemoveReleasesHandle";
constexpr const char *TEST_RESOURCE_REMOVE_NULL_REGISTRY = "WorldResourceBindingBridge_RemoveRejectsNullRegistryWithoutMutation";
constexpr const char *TEST_RESOURCE_REMOVE_MISSING = "WorldResourceBindingBridge_RemoveRejectsMissingWorldObjectWithoutMutation";
constexpr const char *TEST_RESOURCE_REMOVE_RELEASE_FAILURE = "WorldResourceBindingBridge_RemoveReleaseFailureKeepsBinding";
constexpr const char *TEST_RESOURCE_CLEAR_RELEASES = "WorldResourceBindingBridge_ClearReleasesAllHandles";
constexpr const char *TEST_RESOURCE_CLEAR_NULL_REGISTRY = "WorldResourceBindingBridge_ClearRejectsNullRegistryWithoutMutation";
constexpr const char *TEST_RESOURCE_CLEAR_RELEASE_FAILURE = "WorldResourceBindingBridge_ClearReleaseFailurePreservesUnreleasedBindings";
constexpr const char *TEST_RESOURCE_RETIRE_HELD = "WorldResourceBindingBridge_BoundResourceCannotRetireUntilReleased";
constexpr const char *TEST_RESOURCE_QUERY = "WorldResourceBindingBridge_QueryReturnsStoredBinding";
constexpr const char *TEST_RESOURCE_QUERY_READ_ONLY = "WorldResourceBindingBridge_QueryIsReadOnlyAndBounded";
constexpr const char *TEST_RESOURCE_UPDATE_PATH = "WorldResourceBindingBridge_UpdatePathDoesNotGrowWorldStorage";
constexpr const char *TEST_RESOURCE_NO_WORLD_QUERY = "WorldResourceBindingBridge_DoesNotQueryOrMutateWorldInstance";
constexpr const char *TEST_RESOURCE_SNAPSHOT = "WorldResourceBindingBridge_SnapshotReportsCountsAndLastStatus";
constexpr const char *TEST_RESOURCE_NO_FILE_PACKAGE = "WorldResourceBindingBridge_NoFilePackageLoadDecodeUploadOrGameAdapterDependency";
constexpr const char *TEST_RESOURCE_WORLD_CORE_FREE = "WorldResourceBindingBridge_WorldInstanceCoreRemainsResourceFree";
constexpr const char *TEST_RESOURCE_CORE_FREE = "WorldResourceBindingBridge_ResourceCoreRemainsWorldFree";
constexpr const char *TEST_COMPONENT_RESOURCE_BIND_VALID =
    "WorldComponentResourceBindingBridge_BindValidAttachmentResource_AcquiresHandle";
constexpr const char *TEST_COMPONENT_RESOURCE_BIND_NULL_ATTACHMENT =
    "WorldComponentResourceBindingBridge_BindRejectsNullAttachmentSourceWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_BIND_NULL_REGISTRY =
    "WorldComponentResourceBindingBridge_BindRejectsNullRegistryWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_BIND_INVALID_WORLD =
    "WorldComponentResourceBindingBridge_BindRejectsInvalidWorldIdWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_BIND_INVALID_TYPE =
    "WorldComponentResourceBindingBridge_BindRejectsInvalidComponentTypeWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_BIND_INVALID_SLOT =
    "WorldComponentResourceBindingBridge_BindRejectsInvalidComponentSlotWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_BIND_MISSING_ATTACHMENT =
    "WorldComponentResourceBindingBridge_BindRejectsMissingAttachmentWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_BIND_INVALID_HANDLE =
    "WorldComponentResourceBindingBridge_BindRejectsInvalidResourceHandleWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_BIND_STALE_HANDLE =
    "WorldComponentResourceBindingBridge_BindRejectsStaleResourceHandleWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_BIND_TYPE_MISMATCH =
    "WorldComponentResourceBindingBridge_BindRejectsTypeMismatchWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_BIND_DUPLICATE =
    "WorldComponentResourceBindingBridge_BindRejectsDuplicateComponentBinding";
constexpr const char *TEST_COMPONENT_RESOURCE_BIND_CAPACITY =
    "WorldComponentResourceBindingBridge_BindRejectsCapacityOverflowWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_REMOVE_RELEASES =
    "WorldComponentResourceBindingBridge_RemoveReleasesHandle";
constexpr const char *TEST_COMPONENT_RESOURCE_REMOVE_NULL_REGISTRY =
    "WorldComponentResourceBindingBridge_RemoveRejectsNullRegistryWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_REMOVE_MISSING =
    "WorldComponentResourceBindingBridge_RemoveRejectsMissingBindingWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_REMOVE_RELEASE_FAILURE =
    "WorldComponentResourceBindingBridge_RemoveReleaseFailureKeepsBinding";
constexpr const char *TEST_COMPONENT_RESOURCE_CLEAR_RELEASES =
    "WorldComponentResourceBindingBridge_ClearReleasesAllHandles";
constexpr const char *TEST_COMPONENT_RESOURCE_CLEAR_NULL_REGISTRY =
    "WorldComponentResourceBindingBridge_ClearRejectsNullRegistryWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_CLEAR_RELEASE_FAILURE =
    "WorldComponentResourceBindingBridge_ClearReleaseFailurePreservesUnreleasedBindings";
constexpr const char *TEST_COMPONENT_RESOURCE_RETIRE_HELD =
    "WorldComponentResourceBindingBridge_BoundResourceCannotRetireUntilReleased";
constexpr const char *TEST_COMPONENT_RESOURCE_QUERY =
    "WorldComponentResourceBindingBridge_QueryReturnsStoredBinding";
constexpr const char *TEST_COMPONENT_RESOURCE_QUERY_READ_ONLY =
    "WorldComponentResourceBindingBridge_QueryIsReadOnlyAndBounded";
constexpr const char *TEST_COMPONENT_RESOURCE_UPDATE_PATH =
    "WorldComponentResourceBindingBridge_UpdatePathDoesNotGrowStorage";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT =
    "WorldComponentResourceBindingBridge_SnapshotReportsCountsAndLastStatus";
constexpr const char *TEST_COMPONENT_RESOURCE_NO_WORLD_QUERY =
    "WorldComponentResourceBindingBridge_DoesNotQueryOrMutateWorldInstance";
constexpr const char *TEST_COMPONENT_RESOURCE_NO_PAYLOAD =
    "WorldComponentResourceBindingBridge_NoActorComponentPayloadOrLifecycle";
constexpr const char *TEST_COMPONENT_RESOURCE_NO_FILE_PACKAGE =
    "WorldComponentResourceBindingBridge_NoFilePackageLoadDecodeUploadOrGameAdapterDependency";
constexpr const char *TEST_COMPONENT_RESOURCE_NO_RENDER_PHYSICS =
    "WorldComponentResourceBindingBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency";
constexpr const char *TEST_COMPONENT_RESOURCE_WORLD_CORE_FREE =
    "WorldComponentResourceBindingBridge_WorldInstanceCoreRemainsComponentResourceFree";
constexpr const char *TEST_COMPONENT_RESOURCE_RESOURCE_CORE_FREE =
    "WorldComponentResourceBindingBridge_ResourceCoreRemainsWorldFree";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_ROUND_TRIP =
    "WorldComponentResourceBindingSnapshotBridge_WriteReadRoundTripsBindingsInSlotOrder";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_EMPTY_WRITE =
    "WorldComponentResourceBindingSnapshotBridge_WriteEmptyBridgeProducesZeroRecords";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_NULL_SOURCE =
    "WorldComponentResourceBindingSnapshotBridge_WriteRejectsNullSourceWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_NULL_WRITER =
    "WorldComponentResourceBindingSnapshotBridge_WriteRejectsNullWriterWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_WRITER_OVERFLOW =
    "WorldComponentResourceBindingSnapshotBridge_WriteRejectsWriterOverflowWithoutOverrun";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_READ_OUTPUT =
    "WorldComponentResourceBindingSnapshotBridge_ReadWritesCallerOwnedRecords";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_NULL_READER =
    "WorldComponentResourceBindingSnapshotBridge_ReadRejectsNullReaderWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_NULL_OUTPUT =
    "WorldComponentResourceBindingSnapshotBridge_ReadRejectsNullOutputWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_SMALL_OUTPUT =
    "WorldComponentResourceBindingSnapshotBridge_ReadRejectsOutputCapacityTooSmallWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_UNKNOWN_VERSION =
    "WorldComponentResourceBindingSnapshotBridge_ReadRejectsUnknownVersionWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_MALFORMED_COUNT =
    "WorldComponentResourceBindingSnapshotBridge_ReadRejectsMalformedRecordCountWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_INVALID_WORLD =
    "WorldComponentResourceBindingSnapshotBridge_ReadRejectsInvalidWorldIdWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_INVALID_TYPE =
    "WorldComponentResourceBindingSnapshotBridge_ReadRejectsInvalidComponentTypeWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_INVALID_SLOT =
    "WorldComponentResourceBindingSnapshotBridge_ReadRejectsInvalidComponentSlotWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_INVALID_HANDLE =
    "WorldComponentResourceBindingSnapshotBridge_ReadRejectsInvalidResourceHandleWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_INVALID_RESOURCE_TYPE =
    "WorldComponentResourceBindingSnapshotBridge_ReadRejectsInvalidResourceTypeWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_DUPLICATE =
    "WorldComponentResourceBindingSnapshotBridge_ReadRejectsDuplicateBindingWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_NO_ACQUIRE =
    "WorldComponentResourceBindingSnapshotBridge_ReadDoesNotAcquireOrReleaseResources";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_PATH =
    "WorldComponentResourceBindingSnapshotBridge_WriteReadPathDoesNotGrowStorage";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_COUNTERS =
    "WorldComponentResourceBindingSnapshotBridge_SnapshotReportsCountsAndLastStatus";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_NO_PAYLOAD =
    "WorldComponentResourceBindingSnapshotBridge_NoActorComponentPayloadOrLifecycle";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_NO_FILE_PACKAGE =
    "WorldComponentResourceBindingSnapshotBridge_NoFilePackageLoadDecodeUploadOrGameAdapterDependency";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_NO_RENDER_PHYSICS =
    "WorldComponentResourceBindingSnapshotBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_WORLD_CORE_FREE =
    "WorldComponentResourceBindingSnapshotBridge_WorldInstanceCoreRemainsSnapshotFree";
constexpr const char *TEST_COMPONENT_RESOURCE_SNAPSHOT_RESOURCE_CORE_FREE =
    "WorldComponentResourceBindingSnapshotBridge_ResourceCoreRemainsWorldFree";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_ORDER =
    "WorldComponentResourceBindingRestoreBridge_RestoresRecordsInInputOrder";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_EMPTY =
    "WorldComponentResourceBindingRestoreBridge_RestoresEmptyInputWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_NULL_DESTINATION =
    "WorldComponentResourceBindingRestoreBridge_RejectsNullDestinationWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_NULL_ATTACHMENT =
    "WorldComponentResourceBindingRestoreBridge_RejectsNullAttachmentSourceWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_NULL_REGISTRY =
    "WorldComponentResourceBindingRestoreBridge_RejectsNullRegistryWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_NULL_INPUT =
    "WorldComponentResourceBindingRestoreBridge_RejectsNullInputWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_INVALID_WORLD =
    "WorldComponentResourceBindingRestoreBridge_RejectsInvalidWorldIdWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_INVALID_TYPE =
    "WorldComponentResourceBindingRestoreBridge_RejectsInvalidComponentTypeWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_INVALID_SLOT =
    "WorldComponentResourceBindingRestoreBridge_RejectsInvalidComponentSlotWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_MISSING_ATTACHMENT =
    "WorldComponentResourceBindingRestoreBridge_RejectsMissingAttachmentWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_INVALID_HANDLE =
    "WorldComponentResourceBindingRestoreBridge_RejectsInvalidResourceHandleWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_STALE_HANDLE =
    "WorldComponentResourceBindingRestoreBridge_RejectsStaleResourceHandleWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_TYPE_MISMATCH =
    "WorldComponentResourceBindingRestoreBridge_RejectsResourceTypeMismatchWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_DUPLICATE =
    "WorldComponentResourceBindingRestoreBridge_RejectsDuplicateInputWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_DESTINATION_CAPACITY =
    "WorldComponentResourceBindingRestoreBridge_RejectsDestinationCapacityOverflowWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_NON_EMPTY_DESTINATION =
    "WorldComponentResourceBindingRestoreBridge_RejectsNonEmptyDestinationWithoutMutation";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_PREFLIGHT_ACQUIRE =
    "WorldComponentResourceBindingRestoreBridge_AcquiresOnlyAfterPreflight";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_ACQUIRE_FAILURE =
    "WorldComponentResourceBindingRestoreBridge_ResourceAcquireFailureDoesNotPartiallyRestore";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_COUNTERS =
    "WorldComponentResourceBindingRestoreBridge_SnapshotReportsCountsAndLastStatus";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_NO_PAYLOAD =
    "WorldComponentResourceBindingRestoreBridge_NoActorComponentPayloadOrLifecycle";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_NO_FILE_PACKAGE =
    "WorldComponentResourceBindingRestoreBridge_NoFilePackageLoadDecodeUploadOrGameAdapterDependency";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_NO_RENDER_PHYSICS =
    "WorldComponentResourceBindingRestoreBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_WORLD_CORE_FREE =
    "WorldComponentResourceBindingRestoreBridge_WorldInstanceCoreRemainsRestoreFree";
constexpr const char *TEST_COMPONENT_RESOURCE_RESTORE_RESOURCE_CORE_FREE =
    "WorldComponentResourceBindingRestoreBridge_ResourceCoreRemainsWorldFree";
constexpr const char *TEST_SCENE_ASSEMBLY_ORDER =
    "WorldSceneAssemblyBridge_RestoresAttachmentAndBindingRecordsInInputOrder";
constexpr const char *TEST_SCENE_ASSEMBLY_EMPTY =
    "WorldSceneAssemblyBridge_RestoresEmptyAssemblyWithoutMutation";
constexpr const char *TEST_SCENE_ASSEMBLY_NULL_ATTACHMENT_DESTINATION =
    "WorldSceneAssemblyBridge_RejectsNullAttachmentDestinationWithoutMutation";
constexpr const char *TEST_SCENE_ASSEMBLY_NULL_BINDING_DESTINATION =
    "WorldSceneAssemblyBridge_RejectsNullBindingDestinationWithoutMutation";
constexpr const char *TEST_SCENE_ASSEMBLY_NULL_REGISTRY =
    "WorldSceneAssemblyBridge_RejectsNullRegistryWithoutMutation";
constexpr const char *TEST_SCENE_ASSEMBLY_NULL_ATTACHMENT_INPUT =
    "WorldSceneAssemblyBridge_RejectsNullAttachmentInputWithoutMutation";
constexpr const char *TEST_SCENE_ASSEMBLY_NULL_BINDING_INPUT =
    "WorldSceneAssemblyBridge_RejectsNullBindingInputWithoutMutation";
constexpr const char *TEST_SCENE_ASSEMBLY_INVALID_ATTACHMENT =
    "WorldSceneAssemblyBridge_RejectsInvalidAttachmentRecordWithoutMutation";
constexpr const char *TEST_SCENE_ASSEMBLY_INVALID_BINDING =
    "WorldSceneAssemblyBridge_RejectsInvalidBindingRecordWithoutMutation";
constexpr const char *TEST_SCENE_ASSEMBLY_MISSING_ATTACHMENT =
    "WorldSceneAssemblyBridge_RejectsMissingAttachmentForBindingWithoutMutation";
constexpr const char *TEST_SCENE_ASSEMBLY_DUPLICATE_ATTACHMENT =
    "WorldSceneAssemblyBridge_RejectsDuplicateAttachmentInputWithoutMutation";
constexpr const char *TEST_SCENE_ASSEMBLY_DUPLICATE_BINDING =
    "WorldSceneAssemblyBridge_RejectsDuplicateBindingInputWithoutMutation";
constexpr const char *TEST_SCENE_ASSEMBLY_ATTACHMENT_CAPACITY =
    "WorldSceneAssemblyBridge_RejectsAttachmentCapacityOverflowWithoutMutation";
constexpr const char *TEST_SCENE_ASSEMBLY_BINDING_CAPACITY =
    "WorldSceneAssemblyBridge_RejectsBindingCapacityOverflowWithoutMutation";
constexpr const char *TEST_SCENE_ASSEMBLY_NON_EMPTY_DESTINATIONS =
    "WorldSceneAssemblyBridge_RejectsNonEmptyDestinationsWithoutMutation";
constexpr const char *TEST_SCENE_ASSEMBLY_RESOURCE_PREFLIGHT =
    "WorldSceneAssemblyBridge_ValidatesResourceHandlesBeforeMutation";
constexpr const char *TEST_SCENE_ASSEMBLY_BINDING_PREFLIGHT =
    "WorldSceneAssemblyBridge_BindingPreflightFailureDoesNotRestoreAttachments";
constexpr const char *TEST_SCENE_ASSEMBLY_RESOURCE_ACQUIRE_FAILURE =
    "WorldSceneAssemblyBridge_ResourceAcquireFailureDoesNotPartiallyAssemble";
constexpr const char *TEST_SCENE_ASSEMBLY_RESTORE_PATH =
    "WorldSceneAssemblyBridge_RestorePathDoesNotGrowStorage";
constexpr const char *TEST_SCENE_ASSEMBLY_NO_HIDDEN_ALLOCATION =
    "WorldSceneAssemblyBridge_NoHiddenAllocation_UsesYuMemorySignal";
constexpr const char *TEST_SCENE_ASSEMBLY_COUNTERS =
    "WorldSceneAssemblyBridge_SnapshotReportsCountsAndLastStatus";
constexpr const char *TEST_SCENE_ASSEMBLY_NO_PAYLOAD =
    "WorldSceneAssemblyBridge_NoActorComponentPayloadOrLifecycle";
constexpr const char *TEST_SCENE_ASSEMBLY_NO_OBJECT_SCRIPT =
    "WorldSceneAssemblyBridge_NoObjectScriptSerializeThreadPlatformDiagnosticsDependency";
constexpr const char *TEST_SCENE_ASSEMBLY_NO_FILE_PACKAGE =
    "WorldSceneAssemblyBridge_NoFilePackageLoadDecodeUploadOrGameAdapterDependency";
constexpr const char *TEST_SCENE_ASSEMBLY_NO_RENDER_PHYSICS =
    "WorldSceneAssemblyBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency";
constexpr const char *TEST_SCENE_ASSEMBLY_WORLD_CORE_FREE =
    "WorldSceneAssemblyBridge_WorldInstanceCoreRemainsAssemblyFree";
constexpr const char *TEST_SCENE_ASSEMBLY_RESOURCE_CORE_FREE =
    "WorldSceneAssemblyBridge_ResourceCoreRemainsWorldFree";
constexpr const char *TEST_COMPONENT_ADD_VALID = "WorldComponentAttachmentBridge_AddValidAttachment_StoresRecord";
constexpr const char *TEST_COMPONENT_ADD_INVALID_WORLD = "WorldComponentAttachmentBridge_AddRejectsInvalidWorldIdWithoutMutation";
constexpr const char *TEST_COMPONENT_ADD_INVALID_TYPE = "WorldComponentAttachmentBridge_AddRejectsInvalidComponentTypeWithoutMutation";
constexpr const char *TEST_COMPONENT_ADD_INVALID_SLOT = "WorldComponentAttachmentBridge_AddRejectsInvalidComponentSlotWithoutMutation";
constexpr const char *TEST_COMPONENT_ADD_DUPLICATE = "WorldComponentAttachmentBridge_AddRejectsDuplicateTypeForWorldObject";
constexpr const char *TEST_COMPONENT_ADD_CAPACITY = "WorldComponentAttachmentBridge_AddRejectsCapacityOverflowWithoutMutation";
constexpr const char *TEST_COMPONENT_QUERY_STORED = "WorldComponentAttachmentBridge_QueryReturnsStoredAttachment";
constexpr const char *TEST_COMPONENT_QUERY_MISSING = "WorldComponentAttachmentBridge_QueryRejectsMissingAttachmentWithoutMutation";
constexpr const char *TEST_COMPONENT_QUERY_READ_ONLY = "WorldComponentAttachmentBridge_QueryIsReadOnlyAndBounded";
constexpr const char *TEST_COMPONENT_REMOVE_CLEARS = "WorldComponentAttachmentBridge_RemoveClearsAttachment";
constexpr const char *TEST_COMPONENT_REMOVE_MISSING = "WorldComponentAttachmentBridge_RemoveRejectsMissingAttachmentWithoutMutation";
constexpr const char *TEST_COMPONENT_CLEAR_ALL = "WorldComponentAttachmentBridge_ClearRemovesAllAttachmentsInSlotOrder";
constexpr const char *TEST_COMPONENT_UPDATE_PATH = "WorldComponentAttachmentBridge_UpdatePathDoesNotGrowStorage";
constexpr const char *TEST_COMPONENT_SNAPSHOT = "WorldComponentAttachmentBridge_SnapshotReportsCountsAndLastStatus";
constexpr const char *TEST_COMPONENT_NO_WORLD_QUERY = "WorldComponentAttachmentBridge_DoesNotQueryOrMutateWorldInstance";
constexpr const char *TEST_COMPONENT_NO_BEHAVIOR = "WorldComponentAttachmentBridge_NoActorComponentBehaviorOrLifecycle";
constexpr const char *TEST_COMPONENT_NO_OBJECT_RESOURCE = "WorldComponentAttachmentBridge_NoObjectResourceScriptSerializeOrGameAdapterDependency";
constexpr const char *TEST_COMPONENT_NO_FILE_PACKAGE = "WorldComponentAttachmentBridge_NoFilePackageThreadPlatformDiagnosticsDependency";
constexpr const char *TEST_COMPONENT_NO_RENDER_PHYSICS = "WorldComponentAttachmentBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency";
constexpr const char *TEST_COMPONENT_WORLD_CORE_FREE = "WorldComponentAttachmentBridge_WorldInstanceCoreRemainsAttachmentFree";
constexpr const char *TEST_QUERY_TYPE_MATCHES = "WorldComponentQueryBridge_QueryTypeReturnsMatchingWorldObjectsInSlotOrder";
constexpr const char *TEST_QUERY_TYPE_MISSING = "WorldComponentQueryBridge_QueryTypeReturnsZeroForMissingType";
constexpr const char *TEST_QUERY_OBJECT_MATCHES = "WorldComponentQueryBridge_QueryObjectReturnsMatchingAttachmentsInSlotOrder";
constexpr const char *TEST_QUERY_OBJECT_MISSING = "WorldComponentQueryBridge_QueryObjectReturnsZeroForMissingObject";
constexpr const char *TEST_QUERY_NULL_SOURCE = "WorldComponentQueryBridge_QueryRejectsNullSourceWithoutMutation";
constexpr const char *TEST_QUERY_INVALID_TYPE = "WorldComponentQueryBridge_QueryRejectsInvalidComponentTypeWithoutMutation";
constexpr const char *TEST_QUERY_INVALID_WORLD = "WorldComponentQueryBridge_QueryRejectsInvalidWorldIdWithoutMutation";
constexpr const char *TEST_QUERY_NULL_OUTPUT = "WorldComponentQueryBridge_QueryRejectsNullOutputWhenCapacityNonZero";
constexpr const char *TEST_QUERY_OUTPUT_OVERFLOW = "WorldComponentQueryBridge_QueryRejectsOutputOverflowWithoutOverrun";
constexpr const char *TEST_QUERY_READ_ONLY = "WorldComponentQueryBridge_QueryIsReadOnlyForAttachmentStorage";
constexpr const char *TEST_QUERY_UPDATE_PATH = "WorldComponentQueryBridge_QueryPathDoesNotGrowStorage";
constexpr const char *TEST_QUERY_SNAPSHOT = "WorldComponentQueryBridge_SnapshotReportsCountsAndLastStatus";
constexpr const char *TEST_QUERY_NO_BEHAVIOR = "WorldComponentQueryBridge_NoActorComponentBehaviorOrLifecycle";
constexpr const char *TEST_QUERY_NO_OBJECT_RESOURCE = "WorldComponentQueryBridge_NoObjectResourceScriptSerializeOrGameAdapterDependency";
constexpr const char *TEST_QUERY_NO_FILE_PACKAGE = "WorldComponentQueryBridge_NoFilePackageThreadPlatformDiagnosticsDependency";
constexpr const char *TEST_QUERY_NO_RENDER_PHYSICS = "WorldComponentQueryBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency";
constexpr const char *TEST_QUERY_WORLD_CORE_FREE = "WorldComponentQueryBridge_WorldInstanceCoreRemainsQueryFree";
constexpr const char *TEST_COMPONENT_SNAPSHOT_ROUND_TRIP =
    "WorldComponentAttachmentSnapshotBridge_WriteReadRoundTripsAttachmentsInSlotOrder";
constexpr const char *TEST_COMPONENT_SNAPSHOT_EMPTY_WRITE =
    "WorldComponentAttachmentSnapshotBridge_WriteEmptyBridgeProducesZeroRecords";
constexpr const char *TEST_COMPONENT_SNAPSHOT_NULL_SOURCE =
    "WorldComponentAttachmentSnapshotBridge_WriteRejectsNullSourceWithoutMutation";
constexpr const char *TEST_COMPONENT_SNAPSHOT_NULL_WRITER =
    "WorldComponentAttachmentSnapshotBridge_WriteRejectsNullWriterWithoutMutation";
constexpr const char *TEST_COMPONENT_SNAPSHOT_WRITER_OVERFLOW =
    "WorldComponentAttachmentSnapshotBridge_WriteRejectsWriterOverflowWithoutOverrun";
constexpr const char *TEST_COMPONENT_SNAPSHOT_READ_RESTORES =
    "WorldComponentAttachmentSnapshotBridge_ReadRestoresAttachmentRecords";
constexpr const char *TEST_COMPONENT_SNAPSHOT_READ_ZERO_CLEARS =
    "WorldComponentAttachmentSnapshotBridge_ReadZeroRecordStreamClearsDestination";
constexpr const char *TEST_COMPONENT_SNAPSHOT_NULL_READER =
    "WorldComponentAttachmentSnapshotBridge_ReadRejectsNullReaderWithoutMutation";
constexpr const char *TEST_COMPONENT_SNAPSHOT_NULL_DESTINATION =
    "WorldComponentAttachmentSnapshotBridge_ReadRejectsNullDestinationWithoutMutation";
constexpr const char *TEST_COMPONENT_SNAPSHOT_UNKNOWN_VERSION =
    "WorldComponentAttachmentSnapshotBridge_ReadRejectsUnknownVersionWithoutMutation";
constexpr const char *TEST_COMPONENT_SNAPSHOT_MALFORMED_COUNT =
    "WorldComponentAttachmentSnapshotBridge_ReadRejectsMalformedRecordCountWithoutMutation";
constexpr const char *TEST_COMPONENT_SNAPSHOT_INVALID_WORLD =
    "WorldComponentAttachmentSnapshotBridge_ReadRejectsInvalidWorldIdWithoutMutation";
constexpr const char *TEST_COMPONENT_SNAPSHOT_INVALID_TYPE =
    "WorldComponentAttachmentSnapshotBridge_ReadRejectsInvalidComponentTypeWithoutMutation";
constexpr const char *TEST_COMPONENT_SNAPSHOT_INVALID_SLOT =
    "WorldComponentAttachmentSnapshotBridge_ReadRejectsInvalidComponentSlotWithoutMutation";
constexpr const char *TEST_COMPONENT_SNAPSHOT_DUPLICATE =
    "WorldComponentAttachmentSnapshotBridge_ReadRejectsDuplicateAttachmentWithoutMutation";
constexpr const char *TEST_COMPONENT_SNAPSHOT_PATH =
    "WorldComponentAttachmentSnapshotBridge_WriteReadPathDoesNotGrowStorage";
constexpr const char *TEST_COMPONENT_SNAPSHOT_COUNTERS =
    "WorldComponentAttachmentSnapshotBridge_SnapshotReportsCountsAndLastStatus";
constexpr const char *TEST_COMPONENT_SNAPSHOT_NO_PAYLOAD =
    "WorldComponentAttachmentSnapshotBridge_NoActorComponentPayloadOrLifecycle";
constexpr const char *TEST_COMPONENT_SNAPSHOT_NO_OBJECT_RESOURCE =
    "WorldComponentAttachmentSnapshotBridge_NoObjectResourceScriptFilePackageOrGameAdapterDependency";
constexpr const char *TEST_COMPONENT_SNAPSHOT_NO_RENDER_PHYSICS =
    "WorldComponentAttachmentSnapshotBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency";
constexpr const char *TEST_COMPONENT_SNAPSHOT_WORLD_CORE_FREE =
    "WorldComponentAttachmentSnapshotBridge_WorldInstanceCoreRemainsSnapshotFree";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr const char *TRACE_KERNEL_START = "kernel.start";
constexpr const char *TRACE_KERNEL_UPDATE = "kernel.update";
constexpr const char *TRACE_KERNEL_SHUTDOWN = "kernel.shutdown";
constexpr const char *TRACE_WORLD_MODULE_START = "world.module.start";
constexpr const char *TRACE_WORLD_MODULE_UPDATE = "world.module.update";
constexpr const char *TRACE_WORLD_MODULE_SHUTDOWN = "world.module.shutdown";
constexpr WorldObjectId OBJECT_PLAYER{1U};
constexpr WorldObjectId OBJECT_CAMERA{2U};
constexpr WorldObjectId OBJECT_EFFECT{3U};
constexpr ObjectTypeId OBJECT_TYPE_PLAYER{1U};
constexpr ObjectTypeId OBJECT_TYPE_CAMERA{2U};
constexpr ObjectTypeId OBJECT_TYPE_EFFECT{3U};
constexpr ResourceTypeId RESOURCE_TYPE_TEXTURE{1U};
constexpr ResourceTypeId RESOURCE_TYPE_MATERIAL{2U};
constexpr ResourceTypeId RESOURCE_TYPE_AUDIO{3U};
constexpr WorldComponentTypeId COMPONENT_TYPE_PRIMARY{1U};
constexpr WorldComponentTypeId COMPONENT_TYPE_SECONDARY{2U};
constexpr WorldComponentTypeId COMPONENT_TYPE_TERTIARY{3U};
constexpr WorldComponentSlotId COMPONENT_SLOT_PRIMARY{11U};
constexpr WorldComponentSlotId COMPONENT_SLOT_SECONDARY{12U};
constexpr WorldComponentSlotId COMPONENT_SLOT_TERTIARY{13U};
constexpr ScriptCallId SCRIPT_CALL_BEGIN{11U};
constexpr ScriptCallId SCRIPT_CALL_FIXED{12U};
constexpr ScriptCallId SCRIPT_CALL_FRAME{13U};
constexpr ScriptCallId SCRIPT_CALL_END{14U};
constexpr ScriptCallId SCRIPT_CALL_FAILING{15U};
constexpr ScriptCallId SCRIPT_CALL_UNKNOWN{99U};
using TestFunction = int (*)();
using SerializeBuffer = std::array<std::uint8_t, MAX_STREAM_BYTE_COUNT>;

class TestLogSink final : public yuengine::diagnostics::ILogSink {
public:
    void Write(std::string_view module_name, yuengine::diagnostics::LogLevel level, std::string_view message) override {
        static_cast<void>(module_name);
        static_cast<void>(level);
        static_cast<void>(message);
    }

    void SetEnabled(bool enabled) override {
        static_cast<void>(enabled);
    }

    bool IsEnabled() const override {
        return false;
    }

    bool SetModuleEnabled(std::string_view module_name, bool enabled) override {
        static_cast<void>(module_name);
        static_cast<void>(enabled);
        return false;
    }

    bool IsModuleEnabled(std::string_view module_name) const override {
        static_cast<void>(module_name);
        return false;
    }
};

int Fail(const std::string &message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

WorldObjectDesc Object(WorldObjectId id, bool is_enabled=true) {
    return WorldObjectDesc{id, is_enabled};
}

WorldInstance MakeWorld(std::uint32_t object_capacity=MAX_WORLD_OBJECT_COUNT,
    std::uint32_t phase_trace_capacity=MAX_WORLD_PHASE_TRACE_COUNT) {
    WorldDesc desc{};
    desc.object_capacity = object_capacity;
    desc.phase_trace_capacity = phase_trace_capacity;
    return WorldInstance(desc);
}

WorldRegistrationResult Register(WorldInstance &world, WorldObjectId id, bool is_enabled=true) {
    const WorldObjectDesc desc = Object(id, is_enabled);
    return world.RegisterObject(desc);
}

ObjectRegistry MakeRegistry(std::uint32_t object_capacity=8U, std::uint32_t type_capacity=8U) {
    ObjectRegistryDesc desc{};
    desc.object_capacity = object_capacity;
    desc.type_capacity = type_capacity;
    return ObjectRegistry(desc);
}

ObjectRegistrationResult CreateObject(ObjectRegistry &registry,
    ObjectTypeId type=OBJECT_TYPE_PLAYER,
    std::uint32_t initial_reference_count=0U) {
    ObjectDescriptor descriptor{};
    descriptor.type = type;
    descriptor.initial_reference_count = initial_reference_count;
    return registry.CreateSyntheticObject(descriptor);
}

ResourceDescriptor Resource(ResourceTypeId type, const char *key, std::uint32_t initial_reference_count=0U) {
    ResourceDescriptor descriptor{};
    descriptor.type = type;
    descriptor.logical_key = ResourceLogicalKey(key);
    descriptor.initial_reference_count = initial_reference_count;
    return descriptor;
}

ResourceRegistry MakeResourceRegistry(std::uint32_t resource_capacity=8U, std::uint32_t type_capacity=8U) {
    ResourceRegistryDesc desc{};
    desc.resource_capacity = resource_capacity;
    desc.type_capacity = type_capacity;
    return ResourceRegistry(desc);
}

ResourceRegistrationResult RegisterResource(ResourceRegistry &registry,
    ResourceTypeId type=RESOURCE_TYPE_TEXTURE,
    const char *key="texture_a",
    std::uint32_t initial_reference_count=0U) {
    const ResourceDescriptor descriptor = Resource(type, key, initial_reference_count);
    return registry.RegisterSyntheticDescriptor(descriptor);
}

bool ResourceSnapshotsMatch(const ResourceSnapshot &left, const ResourceSnapshot &right) {
    if (left.registered_resource_count != right.registered_resource_count) {
        return false;
    }

    if (left.type_count != right.type_count) {
        return false;
    }

    if (left.acquired_handle_count != right.acquired_handle_count) {
        return false;
    }

    if (left.released_handle_count != right.released_handle_count) {
        return false;
    }

    if (left.retired_resource_count != right.retired_resource_count) {
        return false;
    }

    if (left.dependency_edge_count != right.dependency_edge_count) {
        return false;
    }

    if (left.failed_operation_count != right.failed_operation_count) {
        return false;
    }

    if (left.allocation_accounting_status != right.allocation_accounting_status) {
        return false;
    }

    return left.last_status == right.last_status;
}

WorldTransformState Transform(float base_value) {
    WorldTransformState transform_state{};
    transform_state.translation_x = base_value;
    transform_state.translation_y = base_value + 1.0F;
    transform_state.translation_z = base_value + 2.0F;
    transform_state.rotation_x = base_value + 3.0F;
    transform_state.rotation_y = base_value + 4.0F;
    transform_state.rotation_z = base_value + 5.0F;
    transform_state.rotation_w = base_value + 6.0F;
    transform_state.scale_x = base_value + 7.0F;
    transform_state.scale_y = base_value + 8.0F;
    transform_state.scale_z = base_value + 9.0F;
    return transform_state;
}

bool TransformMatches(const WorldTransformState &left, const WorldTransformState &right) {
    if (left.translation_x != right.translation_x) {
        return false;
    }

    if (left.translation_y != right.translation_y) {
        return false;
    }

    if (left.translation_z != right.translation_z) {
        return false;
    }

    if (left.rotation_x != right.rotation_x) {
        return false;
    }

    if (left.rotation_y != right.rotation_y) {
        return false;
    }

    if (left.rotation_z != right.rotation_z) {
        return false;
    }

    if (left.rotation_w != right.rotation_w) {
        return false;
    }

    if (left.scale_x != right.scale_x) {
        return false;
    }

    if (left.scale_y != right.scale_y) {
        return false;
    }

    return left.scale_z == right.scale_z;
}

ScriptStatus AppendDispatchCode(const ScriptValue *arguments,
    std::uint32_t argument_count,
    ScriptValue *results,
    std::uint32_t result_count,
    std::uint64_t code) {
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

    const std::uint64_t current_value = results[0].AsUInt64();
    const std::uint64_t next_value = (current_value * 10U) + code;
    results[0] = ScriptValue::UInt64(next_value);
    return ScriptStatus::Success;
}

ScriptStatus BeginDispatchNative(const ScriptValue *arguments,
    std::uint32_t argument_count,
    ScriptValue *results,
    std::uint32_t result_count) {
    return AppendDispatchCode(arguments, argument_count, results, result_count, 1U);
}

ScriptStatus FixedDispatchNative(const ScriptValue *arguments,
    std::uint32_t argument_count,
    ScriptValue *results,
    std::uint32_t result_count) {
    return AppendDispatchCode(arguments, argument_count, results, result_count, 2U);
}

ScriptStatus FrameDispatchNative(const ScriptValue *arguments,
    std::uint32_t argument_count,
    ScriptValue *results,
    std::uint32_t result_count) {
    return AppendDispatchCode(arguments, argument_count, results, result_count, 3U);
}

ScriptStatus EndDispatchNative(const ScriptValue *arguments,
    std::uint32_t argument_count,
    ScriptValue *results,
    std::uint32_t result_count) {
    return AppendDispatchCode(arguments, argument_count, results, result_count, 4U);
}

ScriptStatus FailingDispatchNative(const ScriptValue *arguments,
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

    results[0] = ScriptValue::UInt64(42U);
    return ScriptStatus::NativeCallFailed;
}

ScriptNativeBinding MakeDispatchBinding(ScriptCallId call_id,
    yuengine::script::ScriptNativeFunction function) {
    ScriptNativeBinding binding{};
    binding.call_id = call_id;
    binding.function = function;
    binding.argument_count = 0U;
    binding.result_count = 1U;
    binding.result_types[0] = ScriptValueType::UInt64;
    return binding;
}

int RegisterDispatchBinding(ScriptNativeRegistry &registry,
    ScriptCallId call_id,
    yuengine::script::ScriptNativeFunction function) {
    const ScriptNativeBinding binding = MakeDispatchBinding(call_id, function);
    const auto registration = registry.RegisterNativeCall(binding);
    if (!registration.Succeeded()) {
        return Fail("dispatch native registration failed");
    }

    return 0;
}

std::array<ScriptValue, 1> MakeDispatchResults(std::uint64_t value=0U) {
    std::array<ScriptValue, 1> results{};
    results[0] = ScriptValue::UInt64(value);
    return results;
}

WorldPhaseTrace Trace(WorldUpdatePhase phase) {
    WorldPhaseTrace trace{};
    trace.phase = phase;
    trace.frame_index = 1U;
    trace.active_object_count = 1U;
    trace.skipped_object_count = 0U;
    return trace;
}

WorldKernelModuleDesc MakeModuleDesc(std::uint64_t fixed_step_duration=16U) {
    WorldKernelModuleDesc desc{};
    desc.fixed_step_duration = fixed_step_duration;
    return desc;
}

bool TraceContains(const std::vector<std::string> &lifecycle_trace, std::string_view trace_entry) {
    for (const std::string &entry : lifecycle_trace) {
        if (entry == trace_entry) {
            return true;
        }
    }

    return false;
}

int RequireKernelStart(EngineKernel &kernel, std::vector<std::string> &lifecycle_trace) {
    const auto start_result = kernel.Start(lifecycle_trace);
    if (!start_result.succeeded) {
        return Fail("kernel did not start");
    }

    return 0;
}

int RequireKernelUpdate(EngineKernel &kernel,
    std::uint32_t frame_index,
    std::uint64_t tick_time_nanoseconds,
    std::vector<std::string> &lifecycle_trace) {
    const auto update_result = kernel.Update(frame_index, tick_time_nanoseconds, lifecycle_trace);
    if (!update_result.succeeded) {
        return Fail("kernel did not update");
    }

    return 0;
}

int RequireKernelShutdown(EngineKernel &kernel, std::vector<std::string> &lifecycle_trace) {
    const auto shutdown_result = kernel.Shutdown(lifecycle_trace);
    if (!shutdown_result.succeeded) {
        return Fail("kernel did not shut down");
    }

    return 0;
}

int RequireSuccessfulStart(WorldInstance &world) {
    const WorldStatus status = world.Start();
    if (status != WorldStatus::Success) {
        return Fail("world did not start");
    }

    return 0;
}

int RequireSuccessfulUpdate(WorldInstance &world) {
    const WorldStatus status = world.Update(1U, 16U, 17U);
    if (status != WorldStatus::Success) {
        return Fail("world update failed");
    }

    return 0;
}

bool SnapshotRuntimeCountsMatch(const WorldSnapshot &left, const WorldSnapshot &right) {
    if (left.registered_object_count != right.registered_object_count) {
        return false;
    }

    if (left.active_object_count != right.active_object_count) {
        return false;
    }

    if (left.frame_count != right.frame_count) {
        return false;
    }

    if (left.phase_execution_count != right.phase_execution_count) {
        return false;
    }

    if (left.skipped_object_count != right.skipped_object_count) {
        return false;
    }

    return left.allocation_accounting_status == right.allocation_accounting_status;
}

WorldSnapshot SerializableWorldSnapshot(std::uint32_t phase_trace_count=0U) {
    WorldSnapshot snapshot{};
    snapshot.object_capacity = 8U;
    snapshot.phase_trace_capacity = MAX_WORLD_PHASE_TRACE_COUNT;
    snapshot.registered_object_count = 3U;
    snapshot.active_object_count = 2U;
    snapshot.frame_count = 5U;
    snapshot.phase_execution_count = 20U;
    snapshot.skipped_object_count = 1U;
    snapshot.last_frame_index = 4U;
    snapshot.last_fixed_step_duration = 16U;
    snapshot.last_frame_delta_duration = 17U;
    snapshot.phase_trace_count = phase_trace_count;
    snapshot.allocation_accounting_status = MemoryAccountingStatus::ExplicitlyTrackedOnly;
    snapshot.lifecycle_state = WorldLifecycleState::Running;
    snapshot.last_status = WorldStatus::Success;
    return snapshot;
}

WorldTransformSnapshot SerializableTransformSnapshot() {
    WorldTransformSnapshot snapshot{};
    snapshot.bridge_capacity = 8U;
    snapshot.record_count = 2U;
    snapshot.updated_record_count = 3U;
    snapshot.removed_record_count = 1U;
    snapshot.failed_operation_count = 0U;
    snapshot.allocation_accounting_status = MemoryAccountingStatus::ExplicitlyTrackedOnly;
    snapshot.last_status = WorldTransformStatus::Success;
    return snapshot;
}

WorldPhaseTrace Trace(WorldUpdatePhase phase,
    std::uint64_t frame_index,
    std::uint32_t active_object_count,
    std::uint32_t skipped_object_count) {
    WorldPhaseTrace trace{};
    trace.phase = phase;
    trace.frame_index = frame_index;
    trace.active_object_count = active_object_count;
    trace.skipped_object_count = skipped_object_count;
    return trace;
}

bool WorldSnapshotsMatch(const WorldSnapshot &left, const WorldSnapshot &right) {
    if (left.object_capacity != right.object_capacity) {
        return false;
    }

    if (left.phase_trace_capacity != right.phase_trace_capacity) {
        return false;
    }

    if (left.registered_object_count != right.registered_object_count) {
        return false;
    }

    if (left.active_object_count != right.active_object_count) {
        return false;
    }

    if (left.frame_count != right.frame_count) {
        return false;
    }

    if (left.phase_execution_count != right.phase_execution_count) {
        return false;
    }

    if (left.skipped_object_count != right.skipped_object_count) {
        return false;
    }

    if (left.last_frame_index != right.last_frame_index) {
        return false;
    }

    if (left.last_fixed_step_duration != right.last_fixed_step_duration) {
        return false;
    }

    if (left.last_frame_delta_duration != right.last_frame_delta_duration) {
        return false;
    }

    if (left.phase_trace_count != right.phase_trace_count) {
        return false;
    }

    if (left.allocation_accounting_status != right.allocation_accounting_status) {
        return false;
    }

    if (left.lifecycle_state != right.lifecycle_state) {
        return false;
    }

    return left.last_status == right.last_status;
}

bool ComponentAttachmentSnapshotsMatch(
    const WorldComponentAttachmentSnapshot &left,
    const WorldComponentAttachmentSnapshot &right) {
    if (left.attachment_capacity != right.attachment_capacity) {
        return false;
    }

    if (left.active_attachment_count != right.active_attachment_count) {
        return false;
    }

    if (left.added_attachment_count != right.added_attachment_count) {
        return false;
    }

    if (left.removed_attachment_count != right.removed_attachment_count) {
        return false;
    }

    if (left.cleared_attachment_count != right.cleared_attachment_count) {
        return false;
    }

    if (left.query_count != right.query_count) {
        return false;
    }

    if (left.duplicate_rejection_count != right.duplicate_rejection_count) {
        return false;
    }

    if (left.failed_operation_count != right.failed_operation_count) {
        return false;
    }

    if (left.allocation_accounting_status != right.allocation_accounting_status) {
        return false;
    }

    return left.last_status == right.last_status;
}

bool ComponentAttachmentSnapshotBridgeSnapshotsMatch(
    const WorldComponentAttachmentSnapshotBridgeSnapshot &left,
    const WorldComponentAttachmentSnapshotBridgeSnapshot &right) {
    if (left.attachment_capacity != right.attachment_capacity) {
        return false;
    }

    if (left.write_count != right.write_count) {
        return false;
    }

    if (left.read_count != right.read_count) {
        return false;
    }

    if (left.written_record_count != right.written_record_count) {
        return false;
    }

    if (left.read_record_count != right.read_record_count) {
        return false;
    }

    if (left.rejected_record_count != right.rejected_record_count) {
        return false;
    }

    if (left.failed_operation_count != right.failed_operation_count) {
        return false;
    }

    if (left.allocation_accounting_status != right.allocation_accounting_status) {
        return false;
    }

    if (left.last_serialize_status != right.last_serialize_status) {
        return false;
    }

    return left.last_status == right.last_status;
}

bool SerializeSnapshotsMatch(const SerializeSnapshot &left, const SerializeSnapshot &right) {
    if (left.major_version != right.major_version) {
        return false;
    }

    if (left.minor_version != right.minor_version) {
        return false;
    }

    if (left.committed_byte_count != right.committed_byte_count) {
        return false;
    }

    if (left.record_count != right.record_count) {
        return false;
    }

    if (left.field_count != right.field_count) {
        return false;
    }

    if (left.accepted_operation_count != right.accepted_operation_count) {
        return false;
    }

    if (left.failed_operation_count != right.failed_operation_count) {
        return false;
    }

    if (left.allocation_accounting_status != right.allocation_accounting_status) {
        return false;
    }

    return left.last_status == right.last_status;
}

bool PhaseTracesMatch(const WorldPhaseTrace &left, const WorldPhaseTrace &right) {
    if (left.phase != right.phase) {
        return false;
    }

    if (left.frame_index != right.frame_index) {
        return false;
    }

    if (left.active_object_count != right.active_object_count) {
        return false;
    }

    return left.skipped_object_count == right.skipped_object_count;
}

bool TransformSnapshotsMatch(const WorldTransformSnapshot &left, const WorldTransformSnapshot &right) {
    if (left.bridge_capacity != right.bridge_capacity) {
        return false;
    }

    if (left.record_count != right.record_count) {
        return false;
    }

    if (left.updated_record_count != right.updated_record_count) {
        return false;
    }

    if (left.removed_record_count != right.removed_record_count) {
        return false;
    }

    if (left.failed_operation_count != right.failed_operation_count) {
        return false;
    }

    if (left.allocation_accounting_status != right.allocation_accounting_status) {
        return false;
    }

    return left.last_status == right.last_status;
}

bool SerializeBytesMatch(const SerializeBuffer &left, const SerializeBuffer &right, std::uint32_t byte_count) {
    std::uint32_t index = 0U;
    while (index < byte_count) {
        if (left[index] != right[index]) {
            return false;
        }

        ++index;
    }

    return true;
}

int BeginSerializeStream(SerializeWriter &writer) {
    const SerializeStatus status = writer.BeginStream();
    if (status != SerializeStatus::Success) {
        return Fail("serialize begin stream failed");
    }

    return 0;
}

int OpenSerializeStream(SerializeReader &reader) {
    const SerializeStatus status = reader.OpenStream();
    if (status != SerializeStatus::Success) {
        return Fail("serialize open stream failed");
    }

    return 0;
}

void EncodeComponentAttachmentSnapshotUInt32(std::uint8_t *bytes, std::uint32_t value) {
    bytes[0U] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

void EncodeComponentAttachmentSnapshotRecord(
    std::uint8_t *bytes,
    const WorldComponentAttachmentSnapshotRecord &record) {
    EncodeComponentAttachmentSnapshotUInt32(bytes, record.world_object_id.value);
    EncodeComponentAttachmentSnapshotUInt32(bytes + 4U, record.component_type_id.value);
    EncodeComponentAttachmentSnapshotUInt32(bytes + 8U, record.component_slot_id.value);
}

std::uint32_t CalculateComponentAttachmentSnapshotChunkCount(std::uint32_t record_count) {
    return (record_count + WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_RECORD_CAPACITY - 1U) /
        WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_RECORD_CAPACITY;
}

std::uint32_t GetComponentAttachmentSnapshotChunkRecordCount(
    std::uint32_t record_count,
    std::uint32_t chunk_index) {
    const std::uint32_t first_record_index =
        chunk_index * WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_RECORD_CAPACITY;
    if (record_count <= first_record_index) {
        return 0U;
    }

    const std::uint32_t remaining_record_count = record_count - first_record_index;
    if (remaining_record_count > WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_RECORD_CAPACITY) {
        return WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_RECORD_CAPACITY;
    }

    return remaining_record_count;
}

int AddComponentAttachment(
    WorldComponentAttachmentBridge &bridge,
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    WorldComponentSlotId component_slot_id,
    const char *error_message) {
    const WorldComponentAttachmentResult result = bridge.Add(
        world_object_id,
        component_type_id,
        component_slot_id);
    if (!result.Succeeded()) {
        return Fail(error_message);
    }

    return 0;
}

WorldComponentResourceBindingResult BindComponentResource(
    WorldComponentResourceBindingBridge &bridge,
    const WorldComponentAttachmentBridge *attachment_bridge,
    ResourceRegistry *registry,
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    WorldComponentSlotId component_slot_id,
    ResourceHandle resource_handle,
    ResourceTypeId expected_resource_type) {
    return bridge.Bind(
        attachment_bridge,
        registry,
        world_object_id,
        component_type_id,
        component_slot_id,
        resource_handle,
        expected_resource_type);
}

bool ComponentResourceBindingSnapshotsMatch(
    const WorldComponentResourceBindingSnapshot &left,
    const WorldComponentResourceBindingSnapshot &right) {
    if (left.binding_capacity != right.binding_capacity) {
        return false;
    }

    if (left.active_binding_count != right.active_binding_count) {
        return false;
    }

    if (left.acquired_binding_count != right.acquired_binding_count) {
        return false;
    }

    if (left.released_binding_count != right.released_binding_count) {
        return false;
    }

    if (left.cleared_binding_count != right.cleared_binding_count) {
        return false;
    }

    if (left.query_count != right.query_count) {
        return false;
    }

    if (left.failed_operation_count != right.failed_operation_count) {
        return false;
    }

    if (left.allocation_accounting_status != right.allocation_accounting_status) {
        return false;
    }

    if (left.last_resource_status != right.last_resource_status) {
        return false;
    }

    return left.last_status == right.last_status;
}

bool ComponentResourceBindingsMatch(
    const WorldComponentResourceBinding &left,
    const WorldComponentResourceBinding &right) {
    if (left.world_object_id.value != right.world_object_id.value) {
        return false;
    }

    if (left.component_type_id.value != right.component_type_id.value) {
        return false;
    }

    if (left.component_slot_id.value != right.component_slot_id.value) {
        return false;
    }

    if (left.resource_handle.slot != right.resource_handle.slot) {
        return false;
    }

    if (left.resource_handle.generation != right.resource_handle.generation) {
        return false;
    }

    if (left.expected_resource_type.value != right.expected_resource_type.value) {
        return false;
    }

    if (left.is_bound != right.is_bound) {
        return false;
    }

    return left.is_acquired == right.is_acquired;
}

bool ComponentResourceBindingMatchesSnapshotRecord(
    const WorldComponentResourceBinding &binding,
    const WorldComponentResourceBindingSnapshotRecord &record) {
    if (binding.world_object_id.value != record.world_object_id.value) {
        return false;
    }

    if (binding.component_type_id.value != record.component_type_id.value) {
        return false;
    }

    if (binding.component_slot_id.value != record.component_slot_id.value) {
        return false;
    }

    if (binding.resource_handle.slot != record.resource_handle.slot) {
        return false;
    }

    if (binding.resource_handle.generation != record.resource_handle.generation) {
        return false;
    }

    if (binding.expected_resource_type.value != record.expected_resource_type.value) {
        return false;
    }

    if (!binding.is_bound) {
        return false;
    }

    return !binding.is_acquired;
}

WorldComponentResourceBinding SentinelComponentResourceBinding() {
    WorldComponentResourceBinding binding{};
    binding.world_object_id = OBJECT_EFFECT;
    binding.component_type_id = COMPONENT_TYPE_TERTIARY;
    binding.component_slot_id = COMPONENT_SLOT_TERTIARY;
    binding.resource_handle.slot = 777U;
    binding.resource_handle.generation = 888U;
    binding.expected_resource_type = RESOURCE_TYPE_AUDIO;
    binding.is_bound = true;
    binding.is_acquired = true;
    return binding;
}

WorldComponentResourceBindingSnapshotRecord MakeComponentResourceBindingSnapshotRecord(
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    WorldComponentSlotId component_slot_id,
    ResourceHandle resource_handle,
    ResourceTypeId expected_resource_type) {
    WorldComponentResourceBindingSnapshotRecord record{};
    record.world_object_id = world_object_id;
    record.component_type_id = component_type_id;
    record.component_slot_id = component_slot_id;
    record.resource_handle = resource_handle;
    record.expected_resource_type = expected_resource_type;
    return record;
}

void EncodeComponentResourceBindingSnapshotUInt32(std::uint8_t *bytes, std::uint32_t value) {
    bytes[0U] = static_cast<std::uint8_t>(value & 0xFFU);
    bytes[1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);
    bytes[2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    bytes[3U] = static_cast<std::uint8_t>((value >> 24U) & 0xFFU);
}

void EncodeComponentResourceBindingSnapshotRecord(
    std::uint8_t *bytes,
    const WorldComponentResourceBindingSnapshotRecord &record) {
    EncodeComponentResourceBindingSnapshotUInt32(bytes, record.world_object_id.value);
    EncodeComponentResourceBindingSnapshotUInt32(bytes + 4U, record.component_type_id.value);
    EncodeComponentResourceBindingSnapshotUInt32(bytes + 8U, record.component_slot_id.value);
    EncodeComponentResourceBindingSnapshotUInt32(bytes + 12U, record.resource_handle.slot);
    EncodeComponentResourceBindingSnapshotUInt32(bytes + 16U, record.resource_handle.generation);
    EncodeComponentResourceBindingSnapshotUInt32(bytes + 20U, record.expected_resource_type.value);
}

std::uint32_t CalculateComponentResourceBindingSnapshotChunkCount(std::uint32_t record_count) {
    return (record_count + WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_RECORD_CAPACITY - 1U) /
        WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_RECORD_CAPACITY;
}

std::uint32_t GetComponentResourceBindingSnapshotChunkRecordCount(
    std::uint32_t record_count,
    std::uint32_t chunk_index) {
    const std::uint32_t first_record_index =
        chunk_index * WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_RECORD_CAPACITY;
    if (record_count <= first_record_index) {
        return 0U;
    }

    const std::uint32_t remaining_record_count = record_count - first_record_index;
    if (remaining_record_count > WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_RECORD_CAPACITY) {
        return WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_RECORD_CAPACITY;
    }

    return remaining_record_count;
}

int AddComponentResourceBindingFixture(
    WorldComponentAttachmentBridge &attachment_bridge,
    ResourceRegistry &registry,
    WorldComponentResourceBindingBridge &binding_bridge,
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    WorldComponentSlotId component_slot_id,
    ResourceTypeId resource_type,
    const char *resource_key,
    ResourceHandle *out_resource_handle) {
    if (AddComponentAttachment(
            attachment_bridge,
            world_object_id,
            component_type_id,
            component_slot_id,
            "component resource binding snapshot attachment add failed") != 0) {
        return 1;
    }

    const ResourceRegistrationResult resource = RegisterResource(registry, resource_type, resource_key);
    if (!resource.Succeeded()) {
        return Fail("component resource binding snapshot registration failed");
    }

    const WorldComponentResourceBindingResult result = BindComponentResource(
        binding_bridge,
        &attachment_bridge,
        &registry,
        world_object_id,
        component_type_id,
        component_slot_id,
        resource.handle,
        resource_type);
    if (!result.Succeeded()) {
        return Fail("component resource binding snapshot bind failed");
    }

    if (out_resource_handle != nullptr) {
        *out_resource_handle = resource.handle;
    }

    return 0;
}

int WriteComponentResourceBindingSnapshotMetadata(
    SerializeWriter &writer,
    std::uint32_t schema_version,
    std::uint32_t record_count,
    std::uint32_t chunk_count) {
    if (writer.BeginRecord(WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_METADATA_RECORD_ID) !=
        SerializeStatus::Success) {
        return Fail("component resource binding snapshot metadata begin failed");
    }

    if (writer.WriteUInt32(
            WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_FIELD_SCHEMA_VERSION,
            schema_version) != SerializeStatus::Success) {
        return Fail("component resource binding snapshot schema write failed");
    }

    if (writer.WriteUInt32(
            WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_FIELD_RECORD_COUNT,
            record_count) != SerializeStatus::Success) {
        return Fail("component resource binding snapshot record count write failed");
    }

    if (writer.WriteUInt32(
            WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_FIELD_CHUNK_COUNT,
            chunk_count) != SerializeStatus::Success) {
        return Fail("component resource binding snapshot chunk count write failed");
    }

    return 0;
}

int WriteComponentResourceBindingSnapshotRecordChunks(
    SerializeWriter &writer,
    const WorldComponentResourceBindingSnapshotRecord *records,
    std::uint32_t record_count) {
    const std::uint32_t chunk_count = CalculateComponentResourceBindingSnapshotChunkCount(record_count);
    std::uint32_t chunk_index = 0U;
    while (chunk_index < chunk_count) {
        const std::uint32_t first_record_index =
            chunk_index * WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_RECORD_CAPACITY;
        const std::uint32_t chunk_record_count = GetComponentResourceBindingSnapshotChunkRecordCount(
            record_count,
            chunk_index);
        std::array<std::uint8_t, WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_PAYLOAD_BYTE_COUNT> payload{};
        std::uint32_t record_index = 0U;
        while (record_index < chunk_record_count) {
            const std::uint32_t payload_offset =
                record_index * WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_RECORD_BYTE_COUNT;
            EncodeComponentResourceBindingSnapshotRecord(
                payload.data() + payload_offset,
                records[first_record_index + record_index]);
            ++record_index;
        }

        const std::uint32_t record_id_value =
            WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_RECORD_ID_BASE + chunk_index;
        const yuengine::serialize::SerializeRecordId record_id{record_id_value};
        if (writer.BeginRecord(record_id) != SerializeStatus::Success) {
            return Fail("component resource binding snapshot chunk begin failed");
        }

        const std::uint32_t payload_byte_count =
            chunk_record_count * WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_RECORD_BYTE_COUNT;
        if (writer.WriteFixedBytes(
                WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_CHUNK_FIELD_RECORD_BYTES,
                payload.data(),
                payload_byte_count) != SerializeStatus::Success) {
            return Fail("component resource binding snapshot chunk write failed");
        }

        ++chunk_index;
    }

    return 0;
}

int WriteComponentResourceBindingSnapshotFixtureStream(
    SerializeWriter &writer,
    const WorldComponentResourceBindingSnapshotRecord *records,
    std::uint32_t record_count) {
    const std::uint32_t chunk_count = CalculateComponentResourceBindingSnapshotChunkCount(record_count);
    if (WriteComponentResourceBindingSnapshotMetadata(
            writer,
            WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_SCHEMA_VERSION,
            record_count,
            chunk_count) != 0) {
        return 1;
    }

    return WriteComponentResourceBindingSnapshotRecordChunks(writer, records, record_count);
}

int WriteComponentResourceBindingSnapshotToBuffer(
    WorldComponentResourceBindingSnapshotBridge &snapshot_bridge,
    WorldComponentResourceBindingBridge &source_bridge,
    SerializeBuffer &buffer,
    std::uint32_t &committed_byte_count) {
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    const WorldComponentResourceBindingSnapshotResult write_result = snapshot_bridge.WriteSnapshot(
        &writer,
        &source_bridge);
    if (!write_result.Succeeded()) {
        return Fail("component resource binding snapshot fixture write failed");
    }

    committed_byte_count = write_result.state.committed_byte_count;
    return 0;
}

int ReadRejectedComponentResourceBindingSnapshotRecords(
    const WorldComponentResourceBindingSnapshotRecord *records,
    std::uint32_t record_count,
    WorldComponentResourceBindingSnapshotStatus expected_status,
    const char *error_message) {
    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    if (WriteComponentResourceBindingSnapshotFixtureStream(writer, records, record_count) != 0) {
        return 1;
    }

    const WorldComponentResourceBinding sentinel_binding = SentinelComponentResourceBinding();
    std::array<WorldComponentResourceBinding, 2U> output_bindings{sentinel_binding, sentinel_binding};
    std::uint32_t binding_count = 77U;
    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    WorldComponentResourceBindingSnapshotBridge bridge;
    const WorldComponentResourceBindingSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        output_bindings.data(),
        static_cast<std::uint32_t>(output_bindings.size()),
        &binding_count);
    if (result.status != expected_status) {
        return Fail(error_message);
    }

    if (binding_count != 77U) {
        return Fail("component resource binding snapshot rejected read mutated count");
    }

    if (!ComponentResourceBindingsMatch(output_bindings[0], sentinel_binding)) {
        return Fail("component resource binding snapshot rejected read mutated first output");
    }

    if (!ComponentResourceBindingsMatch(output_bindings[1], sentinel_binding)) {
        return Fail("component resource binding snapshot rejected read mutated second output");
    }

    return 0;
}

WorldComponentResourceBinding MakeComponentResourceBinding(
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    WorldComponentSlotId component_slot_id,
    ResourceHandle resource_handle,
    ResourceTypeId expected_resource_type) {
    WorldComponentResourceBinding binding{};
    binding.world_object_id = world_object_id;
    binding.component_type_id = component_type_id;
    binding.component_slot_id = component_slot_id;
    binding.resource_handle = resource_handle;
    binding.expected_resource_type = expected_resource_type;
    binding.is_bound = true;
    binding.is_acquired = false;
    return binding;
}

bool ComponentResourceRestoredBindingMatchesInput(
    const WorldComponentResourceBinding &restored_binding,
    const WorldComponentResourceBinding &input_binding) {
    if (restored_binding.world_object_id.value != input_binding.world_object_id.value) {
        return false;
    }

    if (restored_binding.component_type_id.value != input_binding.component_type_id.value) {
        return false;
    }

    if (restored_binding.component_slot_id.value != input_binding.component_slot_id.value) {
        return false;
    }

    if (restored_binding.resource_handle.slot != input_binding.resource_handle.slot) {
        return false;
    }

    if (restored_binding.resource_handle.generation != input_binding.resource_handle.generation) {
        return false;
    }

    if (restored_binding.expected_resource_type.value != input_binding.expected_resource_type.value) {
        return false;
    }

    if (!restored_binding.is_bound) {
        return false;
    }

    return restored_binding.is_acquired;
}

int AddComponentResourceRestoreInput(
    WorldComponentAttachmentBridge &attachment_bridge,
    ResourceRegistry &registry,
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    WorldComponentSlotId component_slot_id,
    ResourceTypeId resource_type,
    const char *resource_key,
    WorldComponentResourceBinding *out_binding) {
    if (out_binding == nullptr) {
        return Fail("component resource restore output binding was null");
    }

    if (AddComponentAttachment(
            attachment_bridge,
            world_object_id,
            component_type_id,
            component_slot_id,
            "component resource restore attachment add failed") != 0) {
        return 1;
    }

    const ResourceRegistrationResult resource = RegisterResource(registry, resource_type, resource_key);
    if (!resource.Succeeded()) {
        return Fail("component resource restore registration failed");
    }

    *out_binding = MakeComponentResourceBinding(
        world_object_id,
        component_type_id,
        component_slot_id,
        resource.handle,
        resource_type);
    return 0;
}

int ExpectComponentResourceRestoreFailureWithoutMutation(
    WorldComponentResourceBindingRestoreBridge &restore_bridge,
    WorldComponentResourceBindingBridge *destination_bridge,
    const WorldComponentAttachmentBridge *attachment_bridge,
    ResourceRegistry *resource_registry,
    const WorldComponentResourceBinding *input_bindings,
    std::uint32_t input_binding_count,
    WorldComponentResourceBindingRestoreStatus expected_status,
    const char *error_message) {
    const bool has_destination = destination_bridge != nullptr;
    WorldComponentResourceBindingSnapshot before_destination{};
    if (has_destination) {
        before_destination = destination_bridge->Snapshot();
    }

    const bool has_registry = resource_registry != nullptr;
    ResourceSnapshot before_registry{};
    if (has_registry) {
        before_registry = resource_registry->Snapshot();
    }

    const WorldComponentResourceBindingRestoreResult result = restore_bridge.Restore(
        destination_bridge,
        attachment_bridge,
        resource_registry,
        input_bindings,
        input_binding_count);
    if (result.status != expected_status) {
        return Fail(error_message);
    }

    if (has_destination) {
        const WorldComponentResourceBindingSnapshot after_destination = destination_bridge->Snapshot();
        if (!ComponentResourceBindingSnapshotsMatch(before_destination, after_destination)) {
            return Fail("component resource restore failure mutated destination");
        }
    }

    if (has_registry) {
        const ResourceSnapshot after_registry = resource_registry->Snapshot();
        if (!ResourceSnapshotsMatch(before_registry, after_registry)) {
            return Fail("component resource restore failure mutated registry");
        }
    }

    return 0;
}

WorldComponentAttachmentSnapshotRecord MakeSceneAttachmentRecord(
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    WorldComponentSlotId component_slot_id) {
    WorldComponentAttachmentSnapshotRecord record{};
    record.world_object_id = world_object_id;
    record.component_type_id = component_type_id;
    record.component_slot_id = component_slot_id;
    return record;
}

WorldComponentResourceBindingSnapshotRecord MakeSceneBindingRecord(
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    WorldComponentSlotId component_slot_id,
    ResourceHandle resource_handle,
    ResourceTypeId resource_type) {
    WorldComponentResourceBindingSnapshotRecord record{};
    record.world_object_id = world_object_id;
    record.component_type_id = component_type_id;
    record.component_slot_id = component_slot_id;
    record.resource_handle = resource_handle;
    record.expected_resource_type = resource_type;
    return record;
}

int RegisterSceneBindingInput(
    ResourceRegistry &registry,
    WorldObjectId world_object_id,
    WorldComponentTypeId component_type_id,
    WorldComponentSlotId component_slot_id,
    ResourceTypeId resource_type,
    const char *resource_key,
    WorldComponentResourceBindingSnapshotRecord *out_binding) {
    if (out_binding == nullptr) {
        return Fail("scene assembly output binding was null");
    }

    const ResourceRegistrationResult resource = RegisterResource(registry, resource_type, resource_key);
    if (!resource.Succeeded()) {
        return Fail("scene assembly resource registration failed");
    }

    *out_binding = MakeSceneBindingRecord(
        world_object_id,
        component_type_id,
        component_slot_id,
        resource.handle,
        resource_type);
    return 0;
}

bool SceneAttachmentMatches(
    const WorldComponentAttachment &attachment,
    const WorldComponentAttachmentSnapshotRecord &record) {
    if (!attachment.is_attached) {
        return false;
    }

    if (attachment.world_object_id.value != record.world_object_id.value) {
        return false;
    }

    if (attachment.component_type_id.value != record.component_type_id.value) {
        return false;
    }

    return attachment.component_slot_id.value == record.component_slot_id.value;
}

bool SceneBindingMatches(
    const WorldComponentResourceBinding &binding,
    const WorldComponentResourceBindingSnapshotRecord &record) {
    if (!binding.is_bound) {
        return false;
    }

    if (!binding.is_acquired) {
        return false;
    }

    if (binding.world_object_id.value != record.world_object_id.value) {
        return false;
    }

    if (binding.component_type_id.value != record.component_type_id.value) {
        return false;
    }

    if (binding.component_slot_id.value != record.component_slot_id.value) {
        return false;
    }

    if (binding.resource_handle.slot != record.resource_handle.slot) {
        return false;
    }

    if (binding.resource_handle.generation != record.resource_handle.generation) {
        return false;
    }

    return binding.expected_resource_type.value == record.expected_resource_type.value;
}

int ExpectSceneAssemblyFailureWithoutMutation(
    WorldSceneAssemblyBridge &assembly_bridge,
    WorldComponentAttachmentBridge *attachment_destination,
    WorldComponentResourceBindingBridge *binding_destination,
    ResourceRegistry *resource_registry,
    const WorldComponentAttachmentSnapshotRecord *input_attachments,
    std::uint32_t input_attachment_count,
    const WorldComponentResourceBindingSnapshotRecord *input_bindings,
    std::uint32_t input_binding_count,
    WorldSceneAssemblyStatus expected_status,
    const char *error_message) {
    const bool has_attachment_destination = attachment_destination != nullptr;
    WorldComponentAttachmentSnapshot before_attachment_destination{};
    if (has_attachment_destination) {
        before_attachment_destination = attachment_destination->Snapshot();
    }

    const bool has_binding_destination = binding_destination != nullptr;
    WorldComponentResourceBindingSnapshot before_binding_destination{};
    if (has_binding_destination) {
        before_binding_destination = binding_destination->Snapshot();
    }

    const bool has_registry = resource_registry != nullptr;
    ResourceSnapshot before_registry{};
    if (has_registry) {
        before_registry = resource_registry->Snapshot();
    }

    const WorldSceneAssemblyResult result = assembly_bridge.Restore(
        attachment_destination,
        binding_destination,
        resource_registry,
        input_attachments,
        input_attachment_count,
        input_bindings,
        input_binding_count);
    if (result.status != expected_status) {
        return Fail(error_message);
    }

    if (has_attachment_destination) {
        const WorldComponentAttachmentSnapshot after_attachment_destination =
            attachment_destination->Snapshot();
        if (!ComponentAttachmentSnapshotsMatch(before_attachment_destination, after_attachment_destination)) {
            return Fail("scene assembly failure mutated attachment destination");
        }
    }

    if (has_binding_destination) {
        const WorldComponentResourceBindingSnapshot after_binding_destination =
            binding_destination->Snapshot();
        if (!ComponentResourceBindingSnapshotsMatch(before_binding_destination, after_binding_destination)) {
            return Fail("scene assembly failure mutated binding destination");
        }
    }

    if (has_registry) {
        const ResourceSnapshot after_registry = resource_registry->Snapshot();
        if (!ResourceSnapshotsMatch(before_registry, after_registry)) {
            return Fail("scene assembly failure mutated registry");
        }
    }

    return 0;
}

int WriteComponentAttachmentSnapshotMetadata(
    SerializeWriter &writer,
    std::uint32_t schema_version,
    std::uint32_t record_count,
    std::uint32_t chunk_count) {
    if (writer.BeginRecord(WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_METADATA_RECORD_ID) != SerializeStatus::Success) {
        return Fail("component attachment snapshot metadata begin failed");
    }

    if (writer.WriteUInt32(
            WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_FIELD_SCHEMA_VERSION,
            schema_version) != SerializeStatus::Success) {
        return Fail("component attachment snapshot schema write failed");
    }

    if (writer.WriteUInt32(
            WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_FIELD_RECORD_COUNT,
            record_count) != SerializeStatus::Success) {
        return Fail("component attachment snapshot record count write failed");
    }

    if (writer.WriteUInt32(
            WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_FIELD_CHUNK_COUNT,
            chunk_count) != SerializeStatus::Success) {
        return Fail("component attachment snapshot chunk count write failed");
    }

    return 0;
}

int WriteComponentAttachmentSnapshotRecordChunks(
    SerializeWriter &writer,
    const WorldComponentAttachmentSnapshotRecord *records,
    std::uint32_t record_count) {
    const std::uint32_t chunk_count = CalculateComponentAttachmentSnapshotChunkCount(record_count);
    std::uint32_t chunk_index = 0U;
    while (chunk_index < chunk_count) {
        const std::uint32_t first_record_index =
            chunk_index * WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_RECORD_CAPACITY;
        const std::uint32_t chunk_record_count = GetComponentAttachmentSnapshotChunkRecordCount(
            record_count,
            chunk_index);
        std::array<std::uint8_t, WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_PAYLOAD_BYTE_COUNT> payload{};
        std::uint32_t record_index = 0U;
        while (record_index < chunk_record_count) {
            const std::uint32_t payload_offset =
                record_index * WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_RECORD_BYTE_COUNT;
            EncodeComponentAttachmentSnapshotRecord(
                payload.data() + payload_offset,
                records[first_record_index + record_index]);
            ++record_index;
        }

        const std::uint32_t record_id_value =
            WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_RECORD_ID_BASE + chunk_index;
        const yuengine::serialize::SerializeRecordId record_id{record_id_value};
        if (writer.BeginRecord(record_id) != SerializeStatus::Success) {
            return Fail("component attachment snapshot chunk begin failed");
        }

        const std::uint32_t payload_byte_count =
            chunk_record_count * WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_RECORD_BYTE_COUNT;
        if (writer.WriteFixedBytes(
                WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_CHUNK_FIELD_RECORD_BYTES,
                payload.data(),
                payload_byte_count) != SerializeStatus::Success) {
            return Fail("component attachment snapshot chunk write failed");
        }

        ++chunk_index;
    }

    return 0;
}

int WriteComponentAttachmentSnapshotFixtureStream(
    SerializeWriter &writer,
    const WorldComponentAttachmentSnapshotRecord *records,
    std::uint32_t record_count) {
    const std::uint32_t chunk_count = CalculateComponentAttachmentSnapshotChunkCount(record_count);
    if (WriteComponentAttachmentSnapshotMetadata(
            writer,
            WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_SCHEMA_VERSION,
            record_count,
            chunk_count) != 0) {
        return 1;
    }

    return WriteComponentAttachmentSnapshotRecordChunks(writer, records, record_count);
}

int WriteComponentAttachmentSnapshotToBuffer(
    WorldComponentAttachmentSnapshotBridge &bridge,
    WorldComponentAttachmentBridge &source_bridge,
    SerializeBuffer &buffer,
    std::uint32_t &committed_byte_count) {
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    const WorldComponentAttachmentSnapshotResult write_result = bridge.WriteSnapshot(
        &writer,
        &source_bridge);
    if (!write_result.Succeeded()) {
        return Fail("component attachment snapshot fixture write failed");
    }

    committed_byte_count = write_result.state.committed_byte_count;
    return 0;
}

int WriteInvalidEnumWorldStream(SerializeWriter &writer) {
    if (writer.BeginRecord(WORLD_SERIALIZE_WORLD_SNAPSHOT_RECORD_ID) != SerializeStatus::Success) {
        return Fail("invalid enum begin record failed");
    }

    const WorldSnapshot snapshot = SerializableWorldSnapshot(0U);
    if (writer.WriteUInt32(WORLD_SERIALIZE_FIELD_OBJECT_CAPACITY, snapshot.object_capacity) != SerializeStatus::Success) {
        return Fail("invalid enum object capacity write failed");
    }

    if (writer.WriteUInt32(WORLD_SERIALIZE_FIELD_PHASE_TRACE_CAPACITY, snapshot.phase_trace_capacity) != SerializeStatus::Success) {
        return Fail("invalid enum phase trace capacity write failed");
    }

    if (writer.WriteUInt32(WORLD_SERIALIZE_FIELD_REGISTERED_OBJECT_COUNT, snapshot.registered_object_count) != SerializeStatus::Success) {
        return Fail("invalid enum registered count write failed");
    }

    if (writer.WriteUInt32(WORLD_SERIALIZE_FIELD_ACTIVE_OBJECT_COUNT, snapshot.active_object_count) != SerializeStatus::Success) {
        return Fail("invalid enum active count write failed");
    }

    if (writer.WriteUInt64(WORLD_SERIALIZE_FIELD_FRAME_COUNT, snapshot.frame_count) != SerializeStatus::Success) {
        return Fail("invalid enum frame count write failed");
    }

    if (writer.WriteUInt64(WORLD_SERIALIZE_FIELD_PHASE_EXECUTION_COUNT, snapshot.phase_execution_count) != SerializeStatus::Success) {
        return Fail("invalid enum phase execution count write failed");
    }

    if (writer.WriteUInt64(WORLD_SERIALIZE_FIELD_SKIPPED_OBJECT_COUNT, snapshot.skipped_object_count) != SerializeStatus::Success) {
        return Fail("invalid enum skipped count write failed");
    }

    if (writer.WriteUInt64(WORLD_SERIALIZE_FIELD_LAST_FRAME_INDEX, snapshot.last_frame_index) != SerializeStatus::Success) {
        return Fail("invalid enum frame index write failed");
    }

    if (writer.WriteUInt64(WORLD_SERIALIZE_FIELD_LAST_FIXED_STEP_DURATION, snapshot.last_fixed_step_duration) != SerializeStatus::Success) {
        return Fail("invalid enum fixed step write failed");
    }

    if (writer.WriteUInt64(WORLD_SERIALIZE_FIELD_LAST_FRAME_DELTA_DURATION, snapshot.last_frame_delta_duration) != SerializeStatus::Success) {
        return Fail("invalid enum frame delta write failed");
    }

    if (writer.WriteUInt32(WORLD_SERIALIZE_FIELD_PHASE_TRACE_COUNT, snapshot.phase_trace_count) != SerializeStatus::Success) {
        return Fail("invalid enum trace count write failed");
    }

    if (writer.WriteUInt32(WORLD_SERIALIZE_FIELD_ALLOCATION_STATUS,
            static_cast<std::uint32_t>(snapshot.allocation_accounting_status)) != SerializeStatus::Success) {
        return Fail("invalid enum allocation status write failed");
    }

    if (writer.WriteUInt32(WORLD_SERIALIZE_FIELD_LIFECYCLE_STATE, 999U) != SerializeStatus::Success) {
        return Fail("invalid enum lifecycle write failed");
    }

    if (writer.WriteUInt32(WORLD_SERIALIZE_FIELD_LAST_STATUS,
            static_cast<std::uint32_t>(snapshot.last_status)) != SerializeStatus::Success) {
        return Fail("invalid enum world status write failed");
    }

    return 0;
}

int FillWriterNearStreamCapacity(SerializeWriter &writer) {
    const yuengine::serialize::SerializeRecordId record{3000U};
    if (writer.BeginRecord(record) != SerializeStatus::Success) {
        return Fail("partial write fixture begin record failed");
    }

    std::array<std::uint8_t, yuengine::serialize::MAX_FIELD_PAYLOAD_BYTE_COUNT> payload{};
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload.size());
    std::uint32_t field_index = 0U;
    while (field_index < 15U) {
        const std::uint32_t field_id = 3000U + field_index;
        const yuengine::serialize::SerializeFieldId field{field_id};
        const SerializeStatus status = writer.WriteFixedBytes(field, payload.data(), payload_byte_count);
        if (status != SerializeStatus::Success) {
            return Fail("partial write fixture filler write failed");
        }

        ++field_index;
    }

    return 0;
}

int WorldCreateWithFixedCapacityReportsSnapshot() {
    WorldInstance world = MakeWorld(8U, 8U);
    const WorldSnapshot snapshot = world.Snapshot();
    if (snapshot.object_capacity != 8U) {
        return Fail("world object capacity was not recorded");
    }

    if (snapshot.phase_trace_capacity != 8U) {
        return Fail("world phase trace capacity was not recorded");
    }

    if (snapshot.lifecycle_state != WorldLifecycleState::Created) {
        return Fail("world did not start in created state");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("world did not expose YuMemory accounting vocabulary");
    }

    return 0;
}

int WorldStartStopRunsDeterministicLifecycle() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("player object registration failed");
    }

    if (RequireSuccessfulStart(world) != 0) {
        return 1;
    }

    if (world.Snapshot().lifecycle_state != WorldLifecycleState::Running) {
        return Fail("world did not enter running state");
    }

    const WorldStatus stop_status = world.Stop();
    if (stop_status != WorldStatus::Success) {
        return Fail("world did not stop");
    }

    const WorldSnapshot snapshot = world.Snapshot();
    if (snapshot.lifecycle_state != WorldLifecycleState::Stopped) {
        return Fail("world did not enter stopped state");
    }

    if (snapshot.registered_object_count != 0U) {
        return Fail("stop did not clear registered entries");
    }

    return 0;
}

int WorldUpdateRunsPhasesInFixedOrder() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("phase fixture registration failed");
    }

    if (RequireSuccessfulStart(world) != 0) {
        return 1;
    }

    const WorldStatus update_status = world.Update(9U, 16U, 17U);
    if (update_status != WorldStatus::Success) {
        return Fail("phase fixture update failed");
    }

    if (world.GetPhaseTraceCount() != WORLD_UPDATE_PHASE_COUNT) {
        return Fail("phase trace count was not deterministic");
    }

    const std::array<WorldUpdatePhase, WORLD_UPDATE_PHASE_COUNT> expected_phases{
        WorldUpdatePhase::BeginFrame,
        WorldUpdatePhase::FixedStep,
        WorldUpdatePhase::FrameStep,
        WorldUpdatePhase::EndFrame};
    const WorldPhaseTrace *trace = world.GetPhaseTrace();
    for (std::uint32_t index = 0U; index < WORLD_UPDATE_PHASE_COUNT; ++index) {
        if (trace[index].phase != expected_phases[index]) {
            return Fail("phase trace order was not deterministic");
        }

        if (trace[index].frame_index != 9U) {
            return Fail("phase trace frame index was not recorded");
        }
    }

    return 0;
}

int WorldUpdateBeforeStartReturnsExplicitStatus() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("before start registration failed");
    }

    const WorldSnapshot before_snapshot = world.Snapshot();
    const WorldStatus status = world.Update(1U, 16U, 17U);
    if (status != WorldStatus::InvalidLifecycleState) {
        return Fail("update before start did not return explicit status");
    }

    const WorldSnapshot after_snapshot = world.Snapshot();
    if (after_snapshot.frame_count != before_snapshot.frame_count) {
        return Fail("update before start mutated frame count");
    }

    if (after_snapshot.phase_execution_count != before_snapshot.phase_execution_count) {
        return Fail("update before start mutated phase count");
    }

    return 0;
}

int WorldUpdateAfterStopReturnsExplicitStatus() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("after stop registration failed");
    }

    if (RequireSuccessfulStart(world) != 0) {
        return 1;
    }

    if (world.Stop() != WorldStatus::Success) {
        return Fail("after stop fixture stop failed");
    }

    const WorldSnapshot before_snapshot = world.Snapshot();
    const WorldStatus status = world.Update(2U, 16U, 17U);
    if (status != WorldStatus::InvalidLifecycleState) {
        return Fail("update after stop did not return explicit status");
    }

    const WorldSnapshot after_snapshot = world.Snapshot();
    if (after_snapshot.frame_count != before_snapshot.frame_count) {
        return Fail("update after stop mutated frame count");
    }

    return 0;
}

int WorldRegisterDuplicateObjectDoesNotMutate() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("first duplicate fixture registration failed");
    }

    const WorldSnapshot before_snapshot = world.Snapshot();
    const WorldRegistrationResult duplicate_result = Register(world, OBJECT_PLAYER);
    if (duplicate_result.status != WorldStatus::DuplicateObjectId) {
        return Fail("duplicate object did not return explicit status");
    }

    const WorldSnapshot after_snapshot = world.Snapshot();
    if (after_snapshot.registered_object_count != before_snapshot.registered_object_count) {
        return Fail("duplicate object mutated registered count");
    }

    if (after_snapshot.active_object_count != before_snapshot.active_object_count) {
        return Fail("duplicate object mutated active count");
    }

    return 0;
}

int WorldRegisterOverflowDoesNotMutate() {
    WorldInstance world = MakeWorld(1U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("overflow first registration failed");
    }

    const WorldSnapshot before_snapshot = world.Snapshot();
    const WorldRegistrationResult overflow_result = Register(world, OBJECT_CAMERA);
    if (overflow_result.status != WorldStatus::CapacityExceeded) {
        return Fail("overflow registration did not return explicit status");
    }

    const WorldSnapshot after_snapshot = world.Snapshot();
    if (after_snapshot.registered_object_count != before_snapshot.registered_object_count) {
        return Fail("overflow registration mutated registered count");
    }

    if (after_snapshot.active_object_count != before_snapshot.active_object_count) {
        return Fail("overflow registration mutated active count");
    }

    return 0;
}

int WorldDisabledObjectIsSkipped() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER, true).Succeeded()) {
        return Fail("enabled object registration failed");
    }

    if (!Register(world, OBJECT_EFFECT, false).Succeeded()) {
        return Fail("disabled object registration failed");
    }

    if (RequireSuccessfulStart(world) != 0) {
        return 1;
    }

    if (RequireSuccessfulUpdate(world) != 0) {
        return 1;
    }

    const WorldSnapshot snapshot = world.Snapshot();
    if (snapshot.active_object_count != 1U) {
        return Fail("disabled object changed active object count");
    }

    if (snapshot.skipped_object_count != 1U) {
        return Fail("disabled object was not skipped");
    }

    const WorldPhaseTrace *trace = world.GetPhaseTrace();
    if (trace[0].skipped_object_count != 1U) {
        return Fail("phase trace did not record skipped object count");
    }

    return 0;
}

int WorldUpdatePathDoesNotGrowStorage() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("update path registration failed");
    }

    if (RequireSuccessfulStart(world) != 0) {
        return 1;
    }

    const WorldSnapshot before_snapshot = world.Snapshot();
    for (std::uint64_t frame_index = 1U; frame_index <= 3U; ++frame_index) {
        const WorldStatus update_status = world.Update(frame_index, 16U, 17U);
        if (update_status != WorldStatus::Success) {
            return Fail("update path fixture update failed");
        }
    }

    const WorldSnapshot after_snapshot = world.Snapshot();
    if (after_snapshot.object_capacity != before_snapshot.object_capacity) {
        return Fail("update path mutated object capacity");
    }

    if (after_snapshot.phase_trace_capacity != before_snapshot.phase_trace_capacity) {
        return Fail("update path mutated phase trace capacity");
    }

    if (after_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("update path accounting signal changed");
    }

    return 0;
}

int WorldStopClearsActiveEntries() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("stop clear player registration failed");
    }

    if (!Register(world, OBJECT_CAMERA).Succeeded()) {
        return Fail("stop clear camera registration failed");
    }

    if (RequireSuccessfulStart(world) != 0) {
        return 1;
    }

    if (world.Stop() != WorldStatus::Success) {
        return Fail("stop clear fixture stop failed");
    }

    const WorldSnapshot snapshot = world.Snapshot();
    if (snapshot.registered_object_count != 0U) {
        return Fail("stop did not clear registered object count");
    }

    if (snapshot.active_object_count != 0U) {
        return Fail("stop did not clear active object count");
    }

    return 0;
}

int WorldNoScriptResourcePackageFileOrGameAdapterDependency() {
    WorldInstance world = MakeWorld(2U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("dependency fixture registration failed");
    }

    if (RequireSuccessfulStart(world) != 0) {
        return 1;
    }

    if (RequireSuccessfulUpdate(world) != 0) {
        return 1;
    }

    const WorldSnapshot snapshot = world.Snapshot();
    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("world did not keep dependency surface bounded to YuMemory vocabulary");
    }

    return 0;
}

int WorldNoActorComponentOrTransformHierarchy() {
    WorldInstance world = MakeWorld(2U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("hierarchy fixture registration failed");
    }

    if (!Register(world, OBJECT_CAMERA).Succeeded()) {
        return Fail("hierarchy fixture second registration failed");
    }

    if (world.RemoveObject(OBJECT_PLAYER) != WorldStatus::Success) {
        return Fail("world object removal failed");
    }

    const WorldSnapshot snapshot = world.Snapshot();
    if (snapshot.registered_object_count != 1U) {
        return Fail("world object removal did not stay a flat registry operation");
    }

    return 0;
}

int WorldSnapshotReportsCountsAndLastStatus() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("snapshot fixture registration failed");
    }

    if (RequireSuccessfulStart(world) != 0) {
        return 1;
    }

    if (RequireSuccessfulUpdate(world) != 0) {
        return 1;
    }

    const WorldStatus invalid_status = world.Update(2U, 0U, 17U);
    if (invalid_status != WorldStatus::InvalidTimeStep) {
        return Fail("invalid time step did not return explicit status");
    }

    const WorldSnapshot snapshot = world.Snapshot();
    if (snapshot.frame_count != 1U) {
        return Fail("snapshot did not preserve previous valid frame count");
    }

    if (snapshot.phase_execution_count != WORLD_UPDATE_PHASE_COUNT) {
        return Fail("snapshot did not report phase execution count");
    }

    if (snapshot.last_status != WorldStatus::InvalidTimeStep) {
        return Fail("snapshot did not report last status");
    }

    return 0;
}

int WorldKernelModuleStartPublishesWorldService() {
    WorldInstance world = MakeWorld(4U, 8U);
    WorldKernelModule module(world);
    EngineKernel kernel;
    std::vector<std::string> lifecycle_trace;

    if (!kernel.RegisterModule(module)) {
        return Fail("world module registration failed");
    }

    if (RequireKernelStart(kernel, lifecycle_trace) != 0) {
        return 1;
    }

    WorldInstance *service = kernel.Services().Resolve<WorldInstance>(WORLD_INSTANCE_SERVICE_ID);
    if (service != &world) {
        return Fail("world service did not resolve to the fixture instance");
    }

    if (world.Snapshot().lifecycle_state != WorldLifecycleState::Running) {
        return Fail("world service start did not run world lifecycle");
    }

    return RequireKernelShutdown(kernel, lifecycle_trace);
}

int WorldKernelModuleUpdateTicksWorldInKernelOrder() {
    WorldInstance world = MakeWorld(4U, 8U);
    WorldKernelModule module(world);
    EngineKernel kernel;
    std::vector<std::string> lifecycle_trace;

    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("kernel order fixture registration failed");
    }

    if (!kernel.RegisterModule(module)) {
        return Fail("kernel order module registration failed");
    }

    if (RequireKernelStart(kernel, lifecycle_trace) != 0) {
        return 1;
    }

    if (RequireKernelUpdate(kernel, 7U, 33U, lifecycle_trace) != 0) {
        return 1;
    }

    const std::vector<std::string> expected_trace{
        TRACE_KERNEL_START,
        TRACE_WORLD_MODULE_START,
        TRACE_KERNEL_UPDATE,
        TRACE_WORLD_MODULE_UPDATE};
    if (lifecycle_trace != expected_trace) {
        return Fail("world module lifecycle order did not match kernel order");
    }

    const WorldSnapshot snapshot = world.Snapshot();
    if (snapshot.frame_count != 1U) {
        return Fail("world module update did not advance frame count");
    }

    if (snapshot.last_frame_index != 7U) {
        return Fail("world module update did not use kernel frame index");
    }

    if (snapshot.last_fixed_step_duration != 16U) {
        return Fail("world module update did not use adapter fixed step");
    }

    if (snapshot.last_frame_delta_duration != 33U) {
        return Fail("world module update did not use kernel tick duration");
    }

    return RequireKernelShutdown(kernel, lifecycle_trace);
}

int WorldKernelModuleShutdownStopsWorld() {
    WorldInstance world = MakeWorld(4U, 8U);
    WorldKernelModule module(world);
    EngineKernel kernel;
    std::vector<std::string> lifecycle_trace;

    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("shutdown fixture registration failed");
    }

    if (!kernel.RegisterModule(module)) {
        return Fail("shutdown module registration failed");
    }

    if (RequireKernelStart(kernel, lifecycle_trace) != 0) {
        return 1;
    }

    if (RequireKernelShutdown(kernel, lifecycle_trace) != 0) {
        return 1;
    }

    if (world.Snapshot().lifecycle_state != WorldLifecycleState::Stopped) {
        return Fail("world module shutdown did not stop the world");
    }

    if (kernel.Services().Resolve<WorldInstance>(WORLD_INSTANCE_SERVICE_ID) != nullptr) {
        return Fail("world service was not removed after shutdown");
    }

    return 0;
}

int WorldKernelModuleStartFailurePropagatesExplicitStatus() {
    WorldInstance world = MakeWorld(0U, 8U);
    WorldKernelModule module(world);
    EngineKernel kernel;
    std::vector<std::string> lifecycle_trace;

    if (!kernel.RegisterModule(module)) {
        return Fail("start failure module registration failed");
    }

    const auto start_result = kernel.Start(lifecycle_trace);
    if (start_result.succeeded) {
        return Fail("invalid world startup was not rejected");
    }

    if (start_result.status != KernelStatus::StartupFailure) {
        return Fail("invalid world startup had wrong kernel status");
    }

    if (kernel.Services().Resolve<WorldInstance>(WORLD_INSTANCE_SERVICE_ID) != nullptr) {
        return Fail("failed startup left world service registered");
    }

    if (!TraceContains(lifecycle_trace, TRACE_WORLD_MODULE_SHUTDOWN)) {
        return Fail("failed startup did not run module cleanup");
    }

    return 0;
}

int WorldKernelModuleUpdateFailureTriggersKernelTeardown() {
    WorldInstance world = MakeWorld(4U, 8U);
    WorldKernelModuleDesc desc = MakeModuleDesc(0U);
    WorldKernelModule module(world, desc);
    EngineKernel kernel;
    std::vector<std::string> lifecycle_trace;

    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("update failure fixture registration failed");
    }

    if (!kernel.RegisterModule(module)) {
        return Fail("update failure module registration failed");
    }

    if (RequireKernelStart(kernel, lifecycle_trace) != 0) {
        return 1;
    }

    const auto update_result = kernel.Update(1U, 17U, lifecycle_trace);
    if (update_result.succeeded) {
        return Fail("invalid world update was not rejected");
    }

    if (update_result.status != KernelStatus::UpdateFailure) {
        return Fail("invalid world update had wrong kernel status");
    }

    if (world.Snapshot().lifecycle_state != WorldLifecycleState::Stopped) {
        return Fail("kernel update failure did not stop the world");
    }

    if (kernel.Services().Resolve<WorldInstance>(WORLD_INSTANCE_SERVICE_ID) != nullptr) {
        return Fail("kernel update failure did not remove world service");
    }

    return RequireKernelShutdown(kernel, lifecycle_trace);
}

int WorldKernelModuleHeadlessHostDrivesWorldDeterministically() {
    WorldInstance world = MakeWorld(4U, 8U);
    WorldKernelModule module(world);
    EngineKernel kernel;
    KernelHostRuntime runtime(kernel);
    FixedFrameClock frame_clock(17U, 0U);
    TestLogSink log_sink;
    HeadlessHost host(frame_clock, log_sink);
    HeadlessHostConfig config{2U};

    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("headless fixture registration failed");
    }

    if (!kernel.RegisterModule(module)) {
        return Fail("headless module registration failed");
    }

    const auto run_result = host.Run(runtime, config);
    if (run_result.status != HostStatus::Success) {
        return Fail("headless host did not drive world successfully");
    }

    if (run_result.tick_count != 2U) {
        return Fail("headless host tick count was not deterministic");
    }

    if (!TraceContains(run_result.lifecycle_trace, TRACE_WORLD_MODULE_UPDATE)) {
        return Fail("headless host trace did not include world update");
    }

    const WorldSnapshot snapshot = world.Snapshot();
    if (snapshot.lifecycle_state != WorldLifecycleState::Stopped) {
        return Fail("headless host did not stop world");
    }

    if (snapshot.frame_count != 2U) {
        return Fail("headless host did not tick world twice");
    }

    if (snapshot.last_frame_index != 1U) {
        return Fail("headless host did not pass deterministic frame index");
    }

    if (snapshot.last_frame_delta_duration != 17U) {
        return Fail("headless host did not pass deterministic tick duration");
    }

    return 0;
}

int WorldKernelModuleUpdatePathDoesNotGrowWorldStorage() {
    WorldInstance world = MakeWorld(4U, 8U);
    WorldKernelModule module(world);
    EngineKernel kernel;
    std::vector<std::string> lifecycle_trace;

    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("module update path registration failed");
    }

    if (!kernel.RegisterModule(module)) {
        return Fail("module update path module registration failed");
    }

    if (RequireKernelStart(kernel, lifecycle_trace) != 0) {
        return 1;
    }

    const WorldSnapshot before_snapshot = world.Snapshot();
    for (std::uint32_t frame_index = 0U; frame_index < 3U; ++frame_index) {
        if (RequireKernelUpdate(kernel, frame_index, 17U, lifecycle_trace) != 0) {
            return 1;
        }
    }

    const WorldSnapshot after_snapshot = world.Snapshot();
    if (after_snapshot.object_capacity != before_snapshot.object_capacity) {
        return Fail("kernel update path mutated object capacity");
    }

    if (after_snapshot.phase_trace_capacity != before_snapshot.phase_trace_capacity) {
        return Fail("kernel update path mutated phase trace capacity");
    }

    if (after_snapshot.allocation_accounting_status != before_snapshot.allocation_accounting_status) {
        return Fail("kernel update path mutated allocation accounting signal");
    }

    return RequireKernelShutdown(kernel, lifecycle_trace);
}

int WorldKernelModuleNoScriptResourcePackageFileOrGameAdapterDependency() {
    WorldInstance world = MakeWorld(2U, 8U);
    WorldKernelModule module(world);

    if (!module.Dependencies().empty()) {
        return Fail("world module declared unexpected module dependency");
    }

    if (!module.RequiredServices().empty()) {
        return Fail("world module declared unexpected required service");
    }

    const std::vector<std::string_view> published_services = module.PublishedServices();
    if (published_services.size() != 1U) {
        return Fail("world module did not declare exactly one world service");
    }

    if (published_services[0] != WORLD_INSTANCE_SERVICE_ID) {
        return Fail("world module published unexpected service id");
    }

    return 0;
}

int WorldKernelModuleNoActorComponentOrTransformHierarchy() {
    WorldInstance world = MakeWorld(4U, 8U);
    WorldKernelModule module(world);
    EngineKernel kernel;
    std::vector<std::string> lifecycle_trace;

    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("module hierarchy player registration failed");
    }

    if (!Register(world, OBJECT_CAMERA).Succeeded()) {
        return Fail("module hierarchy camera registration failed");
    }

    if (!kernel.RegisterModule(module)) {
        return Fail("module hierarchy module registration failed");
    }

    if (RequireKernelStart(kernel, lifecycle_trace) != 0) {
        return 1;
    }

    if (RequireKernelUpdate(kernel, 0U, 17U, lifecycle_trace) != 0) {
        return 1;
    }

    const WorldSnapshot snapshot = world.Snapshot();
    if (snapshot.registered_object_count != 2U) {
        return Fail("world module changed flat object registration count");
    }

    if (snapshot.active_object_count != 2U) {
        return Fail("world module changed flat active object count");
    }

    return RequireKernelShutdown(kernel, lifecycle_trace);
}

int WorldKernelModuleCoreWorldInstanceRemainsKernelFree() {
    WorldInstance world = MakeWorld(2U, 8U);
    WorldKernelModule module(world);

    if (module.Name() != WORLD_KERNEL_MODULE_NAME) {
        return Fail("world module name was not stable");
    }

    if (world.Snapshot().lifecycle_state != WorldLifecycleState::Created) {
        return Fail("world core construction did not stay standalone");
    }

    const WorldStatus start_status = world.Start();
    if (start_status != WorldStatus::Success) {
        return Fail("world core standalone start failed");
    }

    const WorldStatus stop_status = world.Stop();
    if (stop_status != WorldStatus::Success) {
        return Fail("world core standalone stop failed");
    }

    return 0;
}

int WorldObjectIdentityBridgeBindValidObjectAcquiresHandle() {
    WorldInstance world = MakeWorld(4U, 8U);
    ObjectRegistry registry = MakeRegistry();
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("identity valid world object registration failed");
    }

    const ObjectRegistrationResult object_result = CreateObject(registry);
    if (!object_result.Succeeded()) {
        return Fail("identity valid object creation failed");
    }

    WorldObjectIdentityBridge bridge(world, registry);
    const WorldObjectIdentityResult bind_result = bridge.Bind(OBJECT_PLAYER, object_result.handle);
    if (!bind_result.Succeeded()) {
        return Fail("identity valid bind failed");
    }

    const WorldObjectIdentitySnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.binding_count != 1U) {
        return Fail("identity valid bind did not record binding count");
    }

    if (bridge_snapshot.acquired_handle_count != 1U) {
        return Fail("identity valid bind did not record acquired handle count");
    }

    const ObjectSnapshot object_snapshot = registry.Snapshot();
    if (object_snapshot.referenced_object_count != 1U) {
        return Fail("identity valid bind did not acquire object handle");
    }

    return 0;
}

int WorldObjectIdentityBridgeBindRejectsInvalidWorldIdWithoutMutation() {
    WorldInstance world = MakeWorld(4U, 8U);
    ObjectRegistry registry = MakeRegistry();
    const ObjectRegistrationResult object_result = CreateObject(registry);
    if (!object_result.Succeeded()) {
        return Fail("identity invalid world id object creation failed");
    }

    WorldObjectIdentityBridge bridge(world, registry);
    const WorldObjectIdentitySnapshot before_bridge = bridge.Snapshot();
    const ObjectSnapshot before_object = registry.Snapshot();
    const WorldObjectIdentityResult bind_result = bridge.Bind(WorldObjectId{}, object_result.handle);
    if (bind_result.status != WorldObjectIdentityStatus::InvalidWorldObjectId) {
        return Fail("identity invalid world id returned wrong status");
    }

    const WorldObjectIdentitySnapshot after_bridge = bridge.Snapshot();
    if (after_bridge.binding_count != before_bridge.binding_count) {
        return Fail("identity invalid world id mutated binding count");
    }

    const ObjectSnapshot after_object = registry.Snapshot();
    if (after_object.referenced_object_count != before_object.referenced_object_count) {
        return Fail("identity invalid world id acquired object handle");
    }

    return 0;
}

int WorldObjectIdentityBridgeBindRejectsMissingWorldObjectWithoutMutation() {
    WorldInstance world = MakeWorld(4U, 8U);
    ObjectRegistry registry = MakeRegistry();
    const ObjectRegistrationResult object_result = CreateObject(registry);
    if (!object_result.Succeeded()) {
        return Fail("identity missing world object creation failed");
    }

    WorldObjectIdentityBridge bridge(world, registry);
    const ObjectSnapshot before_object = registry.Snapshot();
    const WorldObjectIdentityResult bind_result = bridge.Bind(OBJECT_PLAYER, object_result.handle);
    if (bind_result.status != WorldObjectIdentityStatus::MissingWorldObject) {
        return Fail("identity missing world object returned wrong status");
    }

    if (bridge.Snapshot().binding_count != 0U) {
        return Fail("identity missing world object mutated bridge binding count");
    }

    if (registry.Snapshot().referenced_object_count != before_object.referenced_object_count) {
        return Fail("identity missing world object acquired handle");
    }

    return 0;
}

int WorldObjectIdentityBridgeBindRejectsInvalidObjectHandleWithoutMutation() {
    WorldInstance world = MakeWorld(4U, 8U);
    ObjectRegistry registry = MakeRegistry();
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("identity invalid handle world registration failed");
    }

    WorldObjectIdentityBridge bridge(world, registry);
    const WorldObjectIdentityResult bind_result = bridge.Bind(OBJECT_PLAYER, ObjectHandle{});
    if (bind_result.status != WorldObjectIdentityStatus::InvalidObjectHandle) {
        return Fail("identity invalid handle returned wrong status");
    }

    if (bridge.Snapshot().binding_count != 0U) {
        return Fail("identity invalid handle mutated binding count");
    }

    if (registry.Snapshot().referenced_object_count != 0U) {
        return Fail("identity invalid handle acquired reference");
    }

    return 0;
}

int WorldObjectIdentityBridgeBindRejectsDuplicateWorldObjectId() {
    WorldInstance world = MakeWorld(4U, 8U);
    ObjectRegistry registry = MakeRegistry();
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("identity duplicate world object registration failed");
    }

    const ObjectRegistrationResult first_object = CreateObject(registry, OBJECT_TYPE_PLAYER);
    const ObjectRegistrationResult second_object = CreateObject(registry, OBJECT_TYPE_CAMERA);
    if (!first_object.Succeeded() || !second_object.Succeeded()) {
        return Fail("identity duplicate world object creation failed");
    }

    WorldObjectIdentityBridge bridge(world, registry);
    if (!bridge.Bind(OBJECT_PLAYER, first_object.handle).Succeeded()) {
        return Fail("identity duplicate world first bind failed");
    }

    const ObjectSnapshot before_object = registry.Snapshot();
    const WorldObjectIdentityResult bind_result = bridge.Bind(OBJECT_PLAYER, second_object.handle);
    if (bind_result.status != WorldObjectIdentityStatus::DuplicateWorldObjectId) {
        return Fail("identity duplicate world id returned wrong status");
    }

    if (bridge.Snapshot().binding_count != 1U) {
        return Fail("identity duplicate world id mutated binding count");
    }

    if (registry.Snapshot().referenced_object_count != before_object.referenced_object_count) {
        return Fail("identity duplicate world id acquired second handle");
    }

    return 0;
}

int WorldObjectIdentityBridgeBindRejectsDuplicateObjectHandle() {
    WorldInstance world = MakeWorld(4U, 8U);
    ObjectRegistry registry = MakeRegistry();
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("identity duplicate handle player registration failed");
    }

    if (!Register(world, OBJECT_CAMERA).Succeeded()) {
        return Fail("identity duplicate handle camera registration failed");
    }

    const ObjectRegistrationResult object_result = CreateObject(registry);
    if (!object_result.Succeeded()) {
        return Fail("identity duplicate handle object creation failed");
    }

    WorldObjectIdentityBridge bridge(world, registry);
    if (!bridge.Bind(OBJECT_PLAYER, object_result.handle).Succeeded()) {
        return Fail("identity duplicate handle first bind failed");
    }

    const ObjectSnapshot before_object = registry.Snapshot();
    const WorldObjectIdentityResult bind_result = bridge.Bind(OBJECT_CAMERA, object_result.handle);
    if (bind_result.status != WorldObjectIdentityStatus::DuplicateObjectHandle) {
        return Fail("identity duplicate handle returned wrong status");
    }

    if (bridge.Snapshot().binding_count != 1U) {
        return Fail("identity duplicate handle mutated binding count");
    }

    if (registry.Snapshot().referenced_object_count != before_object.referenced_object_count) {
        return Fail("identity duplicate handle acquired reference twice");
    }

    return 0;
}

int WorldObjectIdentityBridgeRemoveReleasesHandle() {
    WorldInstance world = MakeWorld(4U, 8U);
    ObjectRegistry registry = MakeRegistry();
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("identity remove world registration failed");
    }

    const ObjectRegistrationResult object_result = CreateObject(registry);
    if (!object_result.Succeeded()) {
        return Fail("identity remove object creation failed");
    }

    WorldObjectIdentityBridge bridge(world, registry);
    if (!bridge.Bind(OBJECT_PLAYER, object_result.handle).Succeeded()) {
        return Fail("identity remove bind failed");
    }

    const WorldObjectIdentityStatus remove_status = bridge.Remove(OBJECT_PLAYER);
    if (remove_status != WorldObjectIdentityStatus::Success) {
        return Fail("identity remove failed");
    }

    const WorldObjectIdentitySnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.binding_count != 0U) {
        return Fail("identity remove did not clear binding count");
    }

    if (bridge_snapshot.released_handle_count != 1U) {
        return Fail("identity remove did not record release count");
    }

    if (registry.Snapshot().referenced_object_count != 0U) {
        return Fail("identity remove did not release object handle");
    }

    return 0;
}

int WorldObjectIdentityBridgeClearReleasesAllHandles() {
    WorldInstance world = MakeWorld(4U, 8U);
    ObjectRegistry registry = MakeRegistry();
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("identity clear player registration failed");
    }

    if (!Register(world, OBJECT_CAMERA).Succeeded()) {
        return Fail("identity clear camera registration failed");
    }

    const ObjectRegistrationResult first_object = CreateObject(registry, OBJECT_TYPE_PLAYER);
    const ObjectRegistrationResult second_object = CreateObject(registry, OBJECT_TYPE_CAMERA);
    if (!first_object.Succeeded() || !second_object.Succeeded()) {
        return Fail("identity clear object creation failed");
    }

    WorldObjectIdentityBridge bridge(world, registry);
    if (!bridge.Bind(OBJECT_PLAYER, first_object.handle).Succeeded()) {
        return Fail("identity clear first bind failed");
    }

    if (!bridge.Bind(OBJECT_CAMERA, second_object.handle).Succeeded()) {
        return Fail("identity clear second bind failed");
    }

    const WorldObjectIdentityStatus clear_status = bridge.Clear();
    if (clear_status != WorldObjectIdentityStatus::Success) {
        return Fail("identity clear failed");
    }

    const WorldObjectIdentitySnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.binding_count != 0U) {
        return Fail("identity clear did not clear binding count");
    }

    if (bridge_snapshot.released_handle_count != 2U) {
        return Fail("identity clear did not release all handles");
    }

    if (registry.Snapshot().referenced_object_count != 0U) {
        return Fail("identity clear did not release object references");
    }

    return 0;
}

int WorldObjectIdentityBridgeStaleGenerationInvalidatesBinding() {
    WorldInstance world = MakeWorld(4U, 8U);
    ObjectRegistry registry = MakeRegistry();
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("identity stale world registration failed");
    }

    const ObjectRegistrationResult object_result = CreateObject(registry);
    if (!object_result.Succeeded()) {
        return Fail("identity stale object creation failed");
    }

    WorldObjectIdentityBridge bridge(world, registry);
    if (!bridge.Bind(OBJECT_PLAYER, object_result.handle).Succeeded()) {
        return Fail("identity stale bind failed");
    }

    if (registry.Release(object_result.handle) != ObjectStatus::Success) {
        return Fail("identity stale external release failed");
    }

    if (registry.Destroy(object_result.handle) != ObjectStatus::Success) {
        return Fail("identity stale external destroy failed");
    }

    const WorldObjectIdentityStatus validate_status = bridge.Validate(OBJECT_PLAYER);
    if (validate_status != WorldObjectIdentityStatus::StaleObjectHandle) {
        return Fail("identity stale generation did not invalidate binding");
    }

    if (bridge.Snapshot().binding_count != 1U) {
        return Fail("identity stale validation mutated binding count");
    }

    return 0;
}

int WorldObjectIdentityBridgeUpdatePathDoesNotGrowWorldStorage() {
    WorldInstance world = MakeWorld(4U, 8U);
    ObjectRegistry registry = MakeRegistry();
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("identity update path world registration failed");
    }

    const ObjectRegistrationResult object_result = CreateObject(registry);
    if (!object_result.Succeeded()) {
        return Fail("identity update path object creation failed");
    }

    WorldObjectIdentityBridge bridge(world, registry);
    if (!bridge.Bind(OBJECT_PLAYER, object_result.handle).Succeeded()) {
        return Fail("identity update path bind failed");
    }

    if (RequireSuccessfulStart(world) != 0) {
        return 1;
    }

    const WorldSnapshot before_snapshot = world.Snapshot();
    for (std::uint64_t frame_index = 1U; frame_index <= 3U; ++frame_index) {
        const WorldStatus update_status = world.Update(frame_index, 16U, 17U);
        if (update_status != WorldStatus::Success) {
            return Fail("identity update path world update failed");
        }
    }

    const WorldSnapshot after_snapshot = world.Snapshot();
    if (after_snapshot.object_capacity != before_snapshot.object_capacity) {
        return Fail("identity update path mutated world object capacity");
    }

    if (after_snapshot.phase_trace_capacity != before_snapshot.phase_trace_capacity) {
        return Fail("identity update path mutated world phase trace capacity");
    }

    if (after_snapshot.allocation_accounting_status != before_snapshot.allocation_accounting_status) {
        return Fail("identity update path mutated world allocation accounting");
    }

    return 0;
}

int WorldObjectIdentityBridgeNoScriptResourcePackageFileOrGameAdapterDependency() {
    WorldInstance world = MakeWorld(2U, 8U);
    ObjectRegistry registry = MakeRegistry();
    WorldObjectIdentityBridge bridge(world, registry);

    const WorldObjectIdentitySnapshot snapshot = bridge.Snapshot();
    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("identity bridge did not keep YuMemory accounting vocabulary");
    }

    if (snapshot.last_status != WorldObjectIdentityStatus::Success) {
        return Fail("identity bridge initial status was not explicit success");
    }

    return 0;
}

int WorldObjectIdentityBridgeNoActorComponentOrTransformHierarchy() {
    WorldInstance world = MakeWorld(4U, 8U);
    ObjectRegistry registry = MakeRegistry();
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("identity hierarchy player registration failed");
    }

    if (!Register(world, OBJECT_CAMERA).Succeeded()) {
        return Fail("identity hierarchy camera registration failed");
    }

    const ObjectRegistrationResult first_object = CreateObject(registry, OBJECT_TYPE_PLAYER);
    const ObjectRegistrationResult second_object = CreateObject(registry, OBJECT_TYPE_CAMERA);
    if (!first_object.Succeeded() || !second_object.Succeeded()) {
        return Fail("identity hierarchy object creation failed");
    }

    WorldObjectIdentityBridge bridge(world, registry);
    if (!bridge.Bind(OBJECT_PLAYER, first_object.handle).Succeeded()) {
        return Fail("identity hierarchy first bind failed");
    }

    if (!bridge.Bind(OBJECT_CAMERA, second_object.handle).Succeeded()) {
        return Fail("identity hierarchy second bind failed");
    }

    if (bridge.Remove(OBJECT_PLAYER) != WorldObjectIdentityStatus::Success) {
        return Fail("identity hierarchy remove failed");
    }

    if (bridge.Snapshot().binding_count != 1U) {
        return Fail("identity hierarchy bridge did not remain a flat binding table");
    }

    return 0;
}

int WorldObjectIdentityBridgeWorldInstanceCoreRemainsObjectFree() {
    WorldInstance world = MakeWorld(2U, 8U);
    ObjectRegistry registry = MakeRegistry();
    WorldObjectIdentityBridge bridge(world, registry);

    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("identity core object-free world registration failed");
    }

    if (!world.ContainsObject(OBJECT_PLAYER)) {
        return Fail("identity core object-free world query failed");
    }

    if (bridge.Snapshot().binding_count != 0U) {
        return Fail("identity bridge construction mutated world core state");
    }

    if (RequireSuccessfulStart(world) != 0) {
        return 1;
    }

    if (world.Stop() != WorldStatus::Success) {
        return Fail("identity core object-free standalone world stop failed");
    }

    return 0;
}

int WorldTransformBridgeRegisterValidObjectStoresTransform() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("transform valid world object registration failed");
    }

    WorldTransformBridge bridge(world);
    const WorldTransformState transform_state = Transform(10.0F);
    const WorldTransformResult register_result = bridge.Register(OBJECT_PLAYER, transform_state);
    if (!register_result.Succeeded()) {
        return Fail("transform valid registration failed");
    }

    if (!TransformMatches(register_result.transform_state, transform_state)) {
        return Fail("transform valid registration returned wrong state");
    }

    const WorldTransformSnapshot snapshot = bridge.Snapshot();
    if (snapshot.record_count != 1U) {
        return Fail("transform valid registration did not record count");
    }

    if (snapshot.last_status != WorldTransformStatus::Success) {
        return Fail("transform valid registration did not record success");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("transform valid registration changed allocation accounting");
    }

    return 0;
}

int WorldTransformBridgeRegisterRejectsInvalidWorldIdWithoutMutation() {
    WorldInstance world = MakeWorld(4U, 8U);
    WorldTransformBridge bridge(world);
    const WorldTransformSnapshot before_snapshot = bridge.Snapshot();
    const WorldTransformState transform_state = Transform(20.0F);
    const WorldTransformResult register_result = bridge.Register(WorldObjectId{}, transform_state);
    if (register_result.status != WorldTransformStatus::InvalidWorldObjectId) {
        return Fail("transform invalid world id returned wrong status");
    }

    const WorldTransformSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.record_count != before_snapshot.record_count) {
        return Fail("transform invalid world id mutated record count");
    }

    if (after_snapshot.updated_record_count != before_snapshot.updated_record_count) {
        return Fail("transform invalid world id mutated update count");
    }

    return 0;
}

int WorldTransformBridgeRegisterRejectsMissingWorldObjectWithoutMutation() {
    WorldInstance world = MakeWorld(4U, 8U);
    WorldTransformBridge bridge(world);
    const WorldTransformState transform_state = Transform(30.0F);
    const WorldTransformResult register_result = bridge.Register(OBJECT_PLAYER, transform_state);
    if (register_result.status != WorldTransformStatus::MissingWorldObject) {
        return Fail("transform missing world object returned wrong status");
    }

    if (bridge.Snapshot().record_count != 0U) {
        return Fail("transform missing world object mutated record count");
    }

    return 0;
}

int WorldTransformBridgeRegisterRejectsDuplicateWorldObjectId() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("transform duplicate world object registration failed");
    }

    WorldTransformBridge bridge(world);
    const WorldTransformState first_state = Transform(40.0F);
    if (!bridge.Register(OBJECT_PLAYER, first_state).Succeeded()) {
        return Fail("transform duplicate first registration failed");
    }

    const WorldTransformState second_state = Transform(50.0F);
    const WorldTransformResult duplicate_result = bridge.Register(OBJECT_PLAYER, second_state);
    if (duplicate_result.status != WorldTransformStatus::DuplicateWorldObjectId) {
        return Fail("transform duplicate world id returned wrong status");
    }

    if (bridge.Snapshot().record_count != 1U) {
        return Fail("transform duplicate world id mutated record count");
    }

    const WorldTransformResult query_result = bridge.Query(OBJECT_PLAYER);
    if (!TransformMatches(query_result.transform_state, first_state)) {
        return Fail("transform duplicate world id replaced existing state");
    }

    return 0;
}

int WorldTransformBridgeRegisterRejectsCapacityOverflowWithoutMutation() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("transform overflow player registration failed");
    }

    if (!Register(world, OBJECT_CAMERA).Succeeded()) {
        return Fail("transform overflow camera registration failed");
    }

    WorldTransformBridgeDesc desc{};
    desc.bridge_capacity = 1U;
    WorldTransformBridge bridge(world, desc);
    const WorldTransformState first_state = Transform(60.0F);
    if (!bridge.Register(OBJECT_PLAYER, first_state).Succeeded()) {
        return Fail("transform overflow first registration failed");
    }

    const WorldTransformState second_state = Transform(70.0F);
    const WorldTransformResult overflow_result = bridge.Register(OBJECT_CAMERA, second_state);
    if (overflow_result.status != WorldTransformStatus::CapacityExceeded) {
        return Fail("transform overflow returned wrong status");
    }

    if (bridge.Snapshot().record_count != 1U) {
        return Fail("transform overflow mutated record count");
    }

    return 0;
}

int WorldTransformBridgeSetUpdatesExistingRecord() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("transform set world registration failed");
    }

    WorldTransformBridge bridge(world);
    const WorldTransformState initial_state = Transform(80.0F);
    if (!bridge.Register(OBJECT_PLAYER, initial_state).Succeeded()) {
        return Fail("transform set initial registration failed");
    }

    const WorldTransformState updated_state = Transform(90.0F);
    const WorldTransformStatus set_status = bridge.Set(OBJECT_PLAYER, updated_state);
    if (set_status != WorldTransformStatus::Success) {
        return Fail("transform set existing record failed");
    }

    const WorldTransformResult query_result = bridge.Query(OBJECT_PLAYER);
    if (!TransformMatches(query_result.transform_state, updated_state)) {
        return Fail("transform set did not update state");
    }

    if (bridge.Snapshot().updated_record_count != 1U) {
        return Fail("transform set did not record update count");
    }

    return 0;
}

int WorldTransformBridgeSetRejectsMissingRecordWithoutMutation() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("transform set missing world registration failed");
    }

    WorldTransformBridge bridge(world);
    const WorldTransformSnapshot before_snapshot = bridge.Snapshot();
    const WorldTransformState transform_state = Transform(100.0F);
    const WorldTransformStatus set_status = bridge.Set(OBJECT_PLAYER, transform_state);
    if (set_status != WorldTransformStatus::TransformNotFound) {
        return Fail("transform set missing returned wrong status");
    }

    const WorldTransformSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.record_count != before_snapshot.record_count) {
        return Fail("transform set missing mutated record count");
    }

    if (after_snapshot.updated_record_count != before_snapshot.updated_record_count) {
        return Fail("transform set missing mutated update count");
    }

    return 0;
}

int WorldTransformBridgeQueryReturnsStoredTransform() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("transform query world registration failed");
    }

    WorldTransformBridge bridge(world);
    const WorldTransformState transform_state = Transform(110.0F);
    if (!bridge.Register(OBJECT_PLAYER, transform_state).Succeeded()) {
        return Fail("transform query registration failed");
    }

    const WorldTransformResult query_result = bridge.Query(OBJECT_PLAYER);
    if (!query_result.Succeeded()) {
        return Fail("transform query failed");
    }

    if (!TransformMatches(query_result.transform_state, transform_state)) {
        return Fail("transform query returned wrong state");
    }

    return 0;
}

int WorldTransformBridgeRemoveClearsRecord() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("transform remove world registration failed");
    }

    WorldTransformBridge bridge(world);
    const WorldTransformState transform_state = Transform(120.0F);
    if (!bridge.Register(OBJECT_PLAYER, transform_state).Succeeded()) {
        return Fail("transform remove registration failed");
    }

    const WorldTransformStatus remove_status = bridge.Remove(OBJECT_PLAYER);
    if (remove_status != WorldTransformStatus::Success) {
        return Fail("transform remove failed");
    }

    const WorldTransformSnapshot snapshot = bridge.Snapshot();
    if (snapshot.record_count != 0U) {
        return Fail("transform remove did not clear record count");
    }

    if (snapshot.removed_record_count != 1U) {
        return Fail("transform remove did not record removal count");
    }

    const WorldTransformResult query_result = bridge.Query(OBJECT_PLAYER);
    if (query_result.status != WorldTransformStatus::TransformNotFound) {
        return Fail("transform remove did not clear query record");
    }

    return 0;
}

int WorldTransformBridgeClearRemovesAllRecords() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("transform clear player registration failed");
    }

    if (!Register(world, OBJECT_CAMERA).Succeeded()) {
        return Fail("transform clear camera registration failed");
    }

    WorldTransformBridge bridge(world);
    const WorldTransformState first_state = Transform(130.0F);
    const WorldTransformState second_state = Transform(140.0F);
    if (!bridge.Register(OBJECT_PLAYER, first_state).Succeeded()) {
        return Fail("transform clear first registration failed");
    }

    if (!bridge.Register(OBJECT_CAMERA, second_state).Succeeded()) {
        return Fail("transform clear second registration failed");
    }

    const WorldTransformStatus clear_status = bridge.Clear();
    if (clear_status != WorldTransformStatus::Success) {
        return Fail("transform clear failed");
    }

    const WorldTransformSnapshot snapshot = bridge.Snapshot();
    if (snapshot.record_count != 0U) {
        return Fail("transform clear did not clear record count");
    }

    if (snapshot.removed_record_count != 2U) {
        return Fail("transform clear did not record all removals");
    }

    return 0;
}

int WorldTransformBridgeUpdatePathDoesNotGrowWorldStorage() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("transform update path world registration failed");
    }

    WorldTransformBridge bridge(world);
    const WorldTransformState transform_state = Transform(150.0F);
    if (!bridge.Register(OBJECT_PLAYER, transform_state).Succeeded()) {
        return Fail("transform update path registration failed");
    }

    if (RequireSuccessfulStart(world) != 0) {
        return 1;
    }

    const WorldSnapshot before_snapshot = world.Snapshot();
    for (std::uint64_t frame_index = 1U; frame_index <= 3U; ++frame_index) {
        const WorldStatus update_status = world.Update(frame_index, 16U, 17U);
        if (update_status != WorldStatus::Success) {
            return Fail("transform update path world update failed");
        }
    }

    const WorldSnapshot after_snapshot = world.Snapshot();
    if (after_snapshot.object_capacity != before_snapshot.object_capacity) {
        return Fail("transform update path mutated world object capacity");
    }

    if (after_snapshot.phase_trace_capacity != before_snapshot.phase_trace_capacity) {
        return Fail("transform update path mutated world phase trace capacity");
    }

    if (after_snapshot.allocation_accounting_status != before_snapshot.allocation_accounting_status) {
        return Fail("transform update path mutated world allocation accounting");
    }

    if (bridge.Snapshot().record_count != 1U) {
        return Fail("transform update path mutated transform record count");
    }

    return 0;
}

int WorldTransformBridgeNoScriptResourcePackageFileObjectOrGameAdapterDependency() {
    WorldInstance world = MakeWorld(2U, 8U);
    WorldTransformBridge bridge(world);

    const WorldTransformSnapshot snapshot = bridge.Snapshot();
    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("transform bridge did not keep YuMemory accounting vocabulary");
    }

    if (snapshot.last_status != WorldTransformStatus::Success) {
        return Fail("transform bridge initial status was not explicit success");
    }

    return 0;
}

int WorldTransformBridgeNoActorComponentSceneGraphOrHierarchy() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("transform hierarchy player registration failed");
    }

    if (!Register(world, OBJECT_CAMERA).Succeeded()) {
        return Fail("transform hierarchy camera registration failed");
    }

    WorldTransformBridge bridge(world);
    const WorldTransformState first_state = Transform(160.0F);
    const WorldTransformState second_state = Transform(170.0F);
    if (!bridge.Register(OBJECT_PLAYER, first_state).Succeeded()) {
        return Fail("transform hierarchy first registration failed");
    }

    if (!bridge.Register(OBJECT_CAMERA, second_state).Succeeded()) {
        return Fail("transform hierarchy second registration failed");
    }

    if (bridge.Remove(OBJECT_PLAYER) != WorldTransformStatus::Success) {
        return Fail("transform hierarchy remove failed");
    }

    if (bridge.Snapshot().record_count != 1U) {
        return Fail("transform hierarchy bridge did not remain a flat record table");
    }

    return 0;
}

int WorldTransformBridgeWorldInstanceCoreRemainsTransformStorageFree() {
    WorldInstance world = MakeWorld(2U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("transform core-free world registration failed");
    }

    const WorldSnapshot before_snapshot = world.Snapshot();
    WorldTransformBridge bridge(world);
    const WorldTransformState transform_state = Transform(180.0F);
    if (!bridge.Register(OBJECT_PLAYER, transform_state).Succeeded()) {
        return Fail("transform core-free registration failed");
    }

    const WorldSnapshot after_snapshot = world.Snapshot();
    if (!SnapshotRuntimeCountsMatch(before_snapshot, after_snapshot)) {
        return Fail("transform core-free bridge mutated world runtime counts");
    }

    if (RequireSuccessfulStart(world) != 0) {
        return 1;
    }

    if (world.Stop() != WorldStatus::Success) {
        return Fail("transform core-free standalone world stop failed");
    }

    return 0;
}

int WorldScriptDispatchBridgeBindPhaseCallReturnsStableBinding() {
    WorldScriptDispatchBridge bridge;
    const WorldScriptDispatchResult result = bridge.Bind(WorldUpdatePhase::BeginFrame, SCRIPT_CALL_BEGIN);
    if (!result.Succeeded()) {
        return Fail("script dispatch bind failed");
    }

    if (result.phase != WorldUpdatePhase::BeginFrame) {
        return Fail("script dispatch bind returned wrong phase");
    }

    if (result.call_id.value != SCRIPT_CALL_BEGIN.value) {
        return Fail("script dispatch bind returned wrong call id");
    }

    const WorldScriptDispatchSnapshot snapshot = bridge.Snapshot();
    if (snapshot.binding_capacity != WORLD_UPDATE_PHASE_COUNT) {
        return Fail("script dispatch default capacity was not phase count");
    }

    if (snapshot.binding_count != 1U) {
        return Fail("script dispatch bind did not record binding count");
    }

    if (snapshot.last_status != WorldScriptDispatchStatus::Success) {
        return Fail("script dispatch bind did not record success");
    }

    return 0;
}

int WorldScriptDispatchBridgeBindRejectsInvalidCallIdWithoutMutation() {
    WorldScriptDispatchBridge bridge;
    const WorldScriptDispatchSnapshot before_snapshot = bridge.Snapshot();
    const WorldScriptDispatchResult result = bridge.Bind(WorldUpdatePhase::BeginFrame, ScriptCallId{});
    if (result.status != WorldScriptDispatchStatus::InvalidCallId) {
        return Fail("script dispatch invalid call id returned wrong status");
    }

    const WorldScriptDispatchSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.binding_count != before_snapshot.binding_count) {
        return Fail("script dispatch invalid call id mutated binding count");
    }

    if (after_snapshot.failed_dispatch_count != before_snapshot.failed_dispatch_count) {
        return Fail("script dispatch invalid call id mutated dispatch failure count");
    }

    return 0;
}

int WorldScriptDispatchBridgeBindRejectsDuplicatePhaseWithoutMutation() {
    WorldScriptDispatchBridge bridge;
    if (!bridge.Bind(WorldUpdatePhase::BeginFrame, SCRIPT_CALL_BEGIN).Succeeded()) {
        return Fail("script dispatch duplicate first bind failed");
    }

    const WorldScriptDispatchSnapshot before_snapshot = bridge.Snapshot();
    const WorldScriptDispatchResult duplicate_result = bridge.Bind(WorldUpdatePhase::BeginFrame, SCRIPT_CALL_FIXED);
    if (duplicate_result.status != WorldScriptDispatchStatus::DuplicatePhase) {
        return Fail("script dispatch duplicate phase returned wrong status");
    }

    const WorldScriptDispatchSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.binding_count != before_snapshot.binding_count) {
        return Fail("script dispatch duplicate phase mutated binding count");
    }

    if (after_snapshot.binding_capacity != before_snapshot.binding_capacity) {
        return Fail("script dispatch duplicate phase mutated binding capacity");
    }

    return 0;
}

int WorldScriptDispatchBridgeBindRejectsCapacityOverflowWithoutMutation() {
    WorldScriptDispatchBridgeDesc desc{};
    desc.binding_capacity = 1U;
    WorldScriptDispatchBridge bridge(desc);
    if (!bridge.Bind(WorldUpdatePhase::BeginFrame, SCRIPT_CALL_BEGIN).Succeeded()) {
        return Fail("script dispatch capacity first bind failed");
    }

    const WorldScriptDispatchSnapshot before_snapshot = bridge.Snapshot();
    const WorldScriptDispatchResult overflow_result = bridge.Bind(WorldUpdatePhase::FixedStep, SCRIPT_CALL_FIXED);
    if (overflow_result.status != WorldScriptDispatchStatus::CapacityExceeded) {
        return Fail("script dispatch capacity overflow returned wrong status");
    }

    const WorldScriptDispatchSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.binding_count != before_snapshot.binding_count) {
        return Fail("script dispatch capacity overflow mutated binding count");
    }

    if (after_snapshot.binding_capacity != 1U) {
        return Fail("script dispatch capacity overflow mutated capacity");
    }

    return 0;
}

int WorldScriptDispatchBridgeDispatchTraceInvokesPhasesInTraceOrder() {
    ScriptNativeRegistry registry;
    if (RegisterDispatchBinding(registry, SCRIPT_CALL_BEGIN, BeginDispatchNative) != 0) {
        return 1;
    }

    if (RegisterDispatchBinding(registry, SCRIPT_CALL_FIXED, FixedDispatchNative) != 0) {
        return 1;
    }

    if (RegisterDispatchBinding(registry, SCRIPT_CALL_FRAME, FrameDispatchNative) != 0) {
        return 1;
    }

    WorldScriptDispatchBridge bridge;
    if (!bridge.Bind(WorldUpdatePhase::BeginFrame, SCRIPT_CALL_BEGIN).Succeeded()) {
        return Fail("script dispatch order begin bind failed");
    }

    if (!bridge.Bind(WorldUpdatePhase::FixedStep, SCRIPT_CALL_FIXED).Succeeded()) {
        return Fail("script dispatch order fixed bind failed");
    }

    if (!bridge.Bind(WorldUpdatePhase::FrameStep, SCRIPT_CALL_FRAME).Succeeded()) {
        return Fail("script dispatch order frame bind failed");
    }

    std::array<WorldPhaseTrace, 3U> traces{
        Trace(WorldUpdatePhase::FrameStep),
        Trace(WorldUpdatePhase::BeginFrame),
        Trace(WorldUpdatePhase::FixedStep)};
    std::array<ScriptValue, 1U> results = MakeDispatchResults();
    const WorldScriptDispatchStatus status = bridge.DispatchTrace(
        registry,
        traces.data(),
        static_cast<std::uint32_t>(traces.size()),
        nullptr,
        0U,
        results.data(),
        static_cast<std::uint32_t>(results.size()));
    if (status != WorldScriptDispatchStatus::Success) {
        return Fail("script dispatch order dispatch failed");
    }

    if (results[0].AsUInt64() != 312U) {
        return Fail("script dispatch order did not follow trace order");
    }

    if (bridge.Snapshot().dispatched_call_count != 3U) {
        return Fail("script dispatch order did not record dispatch count");
    }

    return 0;
}

int WorldScriptDispatchBridgeDispatchSkipsUnboundPhase() {
    ScriptNativeRegistry registry;
    if (RegisterDispatchBinding(registry, SCRIPT_CALL_BEGIN, BeginDispatchNative) != 0) {
        return 1;
    }

    WorldScriptDispatchBridge bridge;
    if (!bridge.Bind(WorldUpdatePhase::BeginFrame, SCRIPT_CALL_BEGIN).Succeeded()) {
        return Fail("script dispatch skip bind failed");
    }

    std::array<WorldPhaseTrace, 2U> traces{
        Trace(WorldUpdatePhase::BeginFrame),
        Trace(WorldUpdatePhase::EndFrame)};
    std::array<ScriptValue, 1U> results = MakeDispatchResults();
    const WorldScriptDispatchStatus status = bridge.DispatchTrace(
        registry,
        traces.data(),
        static_cast<std::uint32_t>(traces.size()),
        nullptr,
        0U,
        results.data(),
        static_cast<std::uint32_t>(results.size()));
    if (status != WorldScriptDispatchStatus::Success) {
        return Fail("script dispatch skip dispatch failed");
    }

    const WorldScriptDispatchSnapshot snapshot = bridge.Snapshot();
    if (snapshot.dispatched_call_count != 1U) {
        return Fail("script dispatch skip recorded wrong dispatch count");
    }

    if (snapshot.skipped_phase_count != 1U) {
        return Fail("script dispatch skip did not count unbound phase");
    }

    if (results[0].AsUInt64() != 1U) {
        return Fail("script dispatch skip invoked wrong phase");
    }

    return 0;
}

int WorldScriptDispatchBridgeDispatchRejectsInvalidTraceBuffer() {
    ScriptNativeRegistry registry;
    WorldScriptDispatchBridge bridge;
    const WorldScriptDispatchStatus null_status = bridge.DispatchTrace(
        registry,
        nullptr,
        1U,
        nullptr,
        0U,
        nullptr,
        0U);
    if (null_status != WorldScriptDispatchStatus::InvalidTraceBuffer) {
        return Fail("script dispatch null trace returned wrong status");
    }

    std::array<WorldPhaseTrace, 1U> traces{Trace(static_cast<WorldUpdatePhase>(99))};
    const WorldScriptDispatchStatus phase_status = bridge.DispatchTrace(
        registry,
        traces.data(),
        static_cast<std::uint32_t>(traces.size()),
        nullptr,
        0U,
        nullptr,
        0U);
    if (phase_status != WorldScriptDispatchStatus::InvalidPhase) {
        return Fail("script dispatch invalid phase returned wrong status");
    }

    if (bridge.Snapshot().failed_dispatch_count != 2U) {
        return Fail("script dispatch invalid trace did not record failures");
    }

    return 0;
}

int WorldScriptDispatchBridgeDispatchRejectsInvalidSlotBuffers() {
    ScriptNativeRegistry registry;
    WorldScriptDispatchBridge bridge;
    if (!bridge.Bind(WorldUpdatePhase::BeginFrame, SCRIPT_CALL_BEGIN).Succeeded()) {
        return Fail("script dispatch slot bind failed");
    }

    std::array<WorldPhaseTrace, 1U> traces{Trace(WorldUpdatePhase::BeginFrame)};
    std::array<ScriptValue, 1U> results = MakeDispatchResults();
    const WorldScriptDispatchStatus argument_status = bridge.DispatchTrace(
        registry,
        traces.data(),
        static_cast<std::uint32_t>(traces.size()),
        nullptr,
        1U,
        results.data(),
        static_cast<std::uint32_t>(results.size()));
    if (argument_status != WorldScriptDispatchStatus::InvalidArgumentBuffer) {
        return Fail("script dispatch invalid argument buffer returned wrong status");
    }

    const WorldScriptDispatchStatus result_status = bridge.DispatchTrace(
        registry,
        traces.data(),
        static_cast<std::uint32_t>(traces.size()),
        nullptr,
        0U,
        nullptr,
        1U);
    if (result_status != WorldScriptDispatchStatus::InvalidResultBuffer) {
        return Fail("script dispatch invalid result buffer returned wrong status");
    }

    if (bridge.Snapshot().dispatched_call_count != 0U) {
        return Fail("script dispatch invalid slot buffers dispatched calls");
    }

    return 0;
}

int WorldScriptDispatchBridgeDispatchPropagatesScriptFailure() {
    ScriptNativeRegistry empty_registry;
    WorldScriptDispatchBridge unknown_bridge;
    if (!unknown_bridge.Bind(WorldUpdatePhase::BeginFrame, SCRIPT_CALL_UNKNOWN).Succeeded()) {
        return Fail("script dispatch unknown bind failed");
    }

    std::array<WorldPhaseTrace, 1U> traces{Trace(WorldUpdatePhase::BeginFrame)};
    std::array<ScriptValue, 1U> results = MakeDispatchResults();
    const WorldScriptDispatchStatus unknown_status = unknown_bridge.DispatchTrace(
        empty_registry,
        traces.data(),
        static_cast<std::uint32_t>(traces.size()),
        nullptr,
        0U,
        results.data(),
        static_cast<std::uint32_t>(results.size()));
    if (unknown_status != WorldScriptDispatchStatus::ScriptCallFailed) {
        return Fail("script dispatch unknown call did not map to bridge failure");
    }

    if (unknown_bridge.Snapshot().last_script_status != ScriptStatus::InvalidCallId) {
        return Fail("script dispatch unknown call did not preserve script status");
    }

    ScriptNativeRegistry failing_registry;
    if (RegisterDispatchBinding(failing_registry, SCRIPT_CALL_FAILING, FailingDispatchNative) != 0) {
        return 1;
    }

    WorldScriptDispatchBridge failing_bridge;
    if (!failing_bridge.Bind(WorldUpdatePhase::BeginFrame, SCRIPT_CALL_FAILING).Succeeded()) {
        return Fail("script dispatch failing bind failed");
    }

    results = MakeDispatchResults();
    const WorldScriptDispatchStatus failing_status = failing_bridge.DispatchTrace(
        failing_registry,
        traces.data(),
        static_cast<std::uint32_t>(traces.size()),
        nullptr,
        0U,
        results.data(),
        static_cast<std::uint32_t>(results.size()));
    if (failing_status != WorldScriptDispatchStatus::ScriptCallFailed) {
        return Fail("script dispatch native failure did not map to bridge failure");
    }

    if (failing_bridge.Snapshot().last_script_status != ScriptStatus::NativeCallFailed) {
        return Fail("script dispatch native failure did not preserve script status");
    }

    if (results[0].AsUInt64() != 42U) {
        return Fail("script dispatch native failure did not preserve caller result slot");
    }

    return 0;
}

int WorldScriptDispatchBridgeDispatchPathDoesNotGrowStorage() {
    ScriptNativeRegistry registry;
    if (RegisterDispatchBinding(registry, SCRIPT_CALL_BEGIN, BeginDispatchNative) != 0) {
        return 1;
    }

    WorldScriptDispatchBridge bridge;
    if (!bridge.Bind(WorldUpdatePhase::BeginFrame, SCRIPT_CALL_BEGIN).Succeeded()) {
        return Fail("script dispatch path bind failed");
    }

    const WorldScriptDispatchSnapshot before_snapshot = bridge.Snapshot();
    std::array<WorldPhaseTrace, 1U> traces{Trace(WorldUpdatePhase::BeginFrame)};
    std::array<ScriptValue, 1U> results = MakeDispatchResults();
    std::uint32_t dispatch_index = 0U;
    while (dispatch_index < 3U) {
        const WorldScriptDispatchStatus status = bridge.DispatchTrace(
            registry,
            traces.data(),
            static_cast<std::uint32_t>(traces.size()),
            nullptr,
            0U,
            results.data(),
            static_cast<std::uint32_t>(results.size()));
        if (status != WorldScriptDispatchStatus::Success) {
            return Fail("script dispatch path dispatch failed");
        }

        ++dispatch_index;
    }

    const WorldScriptDispatchSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.binding_capacity != before_snapshot.binding_capacity) {
        return Fail("script dispatch path mutated binding capacity");
    }

    if (after_snapshot.binding_count != before_snapshot.binding_count) {
        return Fail("script dispatch path mutated binding count");
    }

    if (after_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("script dispatch path changed allocation accounting");
    }

    return 0;
}

int WorldScriptDispatchBridgeSnapshotReportsCountsAndLastStatus() {
    ScriptNativeRegistry registry;
    if (RegisterDispatchBinding(registry, SCRIPT_CALL_BEGIN, BeginDispatchNative) != 0) {
        return 1;
    }

    if (RegisterDispatchBinding(registry, SCRIPT_CALL_FIXED, FixedDispatchNative) != 0) {
        return 1;
    }

    WorldScriptDispatchBridge bridge;
    if (!bridge.Bind(WorldUpdatePhase::BeginFrame, SCRIPT_CALL_BEGIN).Succeeded()) {
        return Fail("script dispatch snapshot begin bind failed");
    }

    if (!bridge.Bind(WorldUpdatePhase::FixedStep, SCRIPT_CALL_FIXED).Succeeded()) {
        return Fail("script dispatch snapshot fixed bind failed");
    }

    std::array<WorldPhaseTrace, 3U> traces{
        Trace(WorldUpdatePhase::BeginFrame),
        Trace(WorldUpdatePhase::EndFrame),
        Trace(WorldUpdatePhase::FixedStep)};
    std::array<ScriptValue, 1U> results = MakeDispatchResults();
    if (bridge.DispatchTrace(
            registry,
            traces.data(),
            static_cast<std::uint32_t>(traces.size()),
            nullptr,
            0U,
            results.data(),
            static_cast<std::uint32_t>(results.size())) != WorldScriptDispatchStatus::Success) {
        return Fail("script dispatch snapshot success dispatch failed");
    }

    const WorldScriptDispatchStatus failure_status = bridge.DispatchTrace(
        registry,
        nullptr,
        1U,
        nullptr,
        0U,
        nullptr,
        0U);
    if (failure_status != WorldScriptDispatchStatus::InvalidTraceBuffer) {
        return Fail("script dispatch snapshot failure dispatch returned wrong status");
    }

    const WorldScriptDispatchSnapshot snapshot = bridge.Snapshot();
    if (snapshot.binding_count != 2U) {
        return Fail("script dispatch snapshot did not report binding count");
    }

    if (snapshot.dispatched_call_count != 2U) {
        return Fail("script dispatch snapshot did not report dispatch count");
    }

    if (snapshot.skipped_phase_count != 1U) {
        return Fail("script dispatch snapshot did not report skipped count");
    }

    if (snapshot.failed_dispatch_count != 1U) {
        return Fail("script dispatch snapshot did not report failure count");
    }

    if (snapshot.last_status != WorldScriptDispatchStatus::InvalidTraceBuffer) {
        return Fail("script dispatch snapshot did not report last status");
    }

    return 0;
}

int WorldScriptDispatchBridgeNoActorComponentSceneGraphOrGameAdapterDependency() {
    WorldScriptDispatchBridge bridge;
    if (!bridge.Bind(WorldUpdatePhase::BeginFrame, SCRIPT_CALL_BEGIN).Succeeded()) {
        return Fail("script dispatch no actor bind failed");
    }

    if (!bridge.Bind(WorldUpdatePhase::EndFrame, SCRIPT_CALL_END).Succeeded()) {
        return Fail("script dispatch no actor second bind failed");
    }

    const WorldScriptDispatchSnapshot snapshot = bridge.Snapshot();
    if (snapshot.binding_count != 2U) {
        return Fail("script dispatch no actor bridge did not remain phase table");
    }

    if (snapshot.binding_capacity != WORLD_UPDATE_PHASE_COUNT) {
        return Fail("script dispatch no actor default capacity changed");
    }

    return 0;
}

int WorldScriptDispatchBridgeNoResourcePackageFileSerializeOrObjectOwnershipDependency() {
    ScriptNativeRegistry registry;
    if (RegisterDispatchBinding(registry, SCRIPT_CALL_END, EndDispatchNative) != 0) {
        return 1;
    }

    WorldScriptDispatchBridge bridge;
    if (!bridge.Bind(WorldUpdatePhase::EndFrame, SCRIPT_CALL_END).Succeeded()) {
        return Fail("script dispatch no forbidden dependency bind failed");
    }

    std::array<WorldPhaseTrace, 1U> traces{Trace(WorldUpdatePhase::EndFrame)};
    std::array<ScriptValue, 1U> results = MakeDispatchResults();
    const WorldScriptDispatchStatus status = bridge.DispatchTrace(
        registry,
        traces.data(),
        static_cast<std::uint32_t>(traces.size()),
        nullptr,
        0U,
        results.data(),
        static_cast<std::uint32_t>(results.size()));
    if (status != WorldScriptDispatchStatus::Success) {
        return Fail("script dispatch no forbidden dependency dispatch failed");
    }

    if (results[0].AsUInt64() != 4U) {
        return Fail("script dispatch no forbidden dependency result changed");
    }

    return 0;
}

int WorldScriptDispatchBridgeWorldInstanceCoreRemainsScriptFree() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("script dispatch world core-free registration failed");
    }

    const WorldSnapshot before_snapshot = world.Snapshot();
    WorldScriptDispatchBridge bridge;
    if (!bridge.Bind(WorldUpdatePhase::BeginFrame, SCRIPT_CALL_BEGIN).Succeeded()) {
        return Fail("script dispatch world core-free bind failed");
    }

    const WorldSnapshot after_snapshot = world.Snapshot();
    if (!SnapshotRuntimeCountsMatch(before_snapshot, after_snapshot)) {
        return Fail("script dispatch bridge mutated world runtime counts");
    }

    if (RequireSuccessfulStart(world) != 0) {
        return 1;
    }

    if (RequireSuccessfulUpdate(world) != 0) {
        return 1;
    }

    if (world.Stop() != WorldStatus::Success) {
        return Fail("script dispatch world core-free stop failed");
    }

    return 0;
}

int WorldScriptDispatchBridgeScriptRegistryCoreRemainsWorldFree() {
    ScriptNativeRegistry registry;
    if (RegisterDispatchBinding(registry, SCRIPT_CALL_BEGIN, BeginDispatchNative) != 0) {
        return 1;
    }

    const auto before_snapshot = registry.Snapshot();
    WorldScriptDispatchBridge bridge;
    if (!bridge.Bind(WorldUpdatePhase::BeginFrame, SCRIPT_CALL_BEGIN).Succeeded()) {
        return Fail("script dispatch script core-free bind failed");
    }

    std::array<WorldPhaseTrace, 1U> traces{Trace(WorldUpdatePhase::BeginFrame)};
    std::array<ScriptValue, 1U> results = MakeDispatchResults();
    const WorldScriptDispatchStatus status = bridge.DispatchTrace(
        registry,
        traces.data(),
        static_cast<std::uint32_t>(traces.size()),
        nullptr,
        0U,
        results.data(),
        static_cast<std::uint32_t>(results.size()));
    if (status != WorldScriptDispatchStatus::Success) {
        return Fail("script dispatch script core-free dispatch failed");
    }

    const auto after_snapshot = registry.Snapshot();
    if (after_snapshot.binding_capacity != before_snapshot.binding_capacity) {
        return Fail("script dispatch script core-free mutated registry capacity");
    }

    if (after_snapshot.binding_count != before_snapshot.binding_count) {
        return Fail("script dispatch script core-free mutated registry binding count");
    }

    if (after_snapshot.successful_call_count != 1U) {
        return Fail("script dispatch script core-free did not invoke registry");
    }

    return 0;
}

int WorldSerializeSnapshotBridgeWriteWorldSnapshotRoundTripsDeterministically() {
    const WorldSnapshot input_snapshot = SerializableWorldSnapshot(2U);
    const std::array<WorldPhaseTrace, 2U> input_traces{
        Trace(WorldUpdatePhase::BeginFrame, 7U, 2U, 0U),
        Trace(WorldUpdatePhase::EndFrame, 7U, 2U, 1U)};
    SerializeBuffer first_buffer{};
    SerializeBuffer second_buffer{};
    WorldSerializeSnapshotBridge bridge;
    SerializeWriter first_writer(first_buffer.data(), static_cast<std::uint32_t>(first_buffer.size()));
    if (BeginSerializeStream(first_writer) != 0) {
        return 1;
    }

    const WorldSerializeSnapshotResult write_result = bridge.WriteSnapshot(
        &first_writer,
        input_snapshot,
        input_traces.data(),
        static_cast<std::uint32_t>(input_traces.size()));
    if (!write_result.Succeeded()) {
        return Fail("serialize bridge round trip write failed");
    }

    WorldSerializeSnapshotBridge second_bridge;
    SerializeWriter second_writer(second_buffer.data(), static_cast<std::uint32_t>(second_buffer.size()));
    if (BeginSerializeStream(second_writer) != 0) {
        return 1;
    }

    const WorldSerializeSnapshotResult second_write = second_bridge.WriteSnapshot(
        &second_writer,
        input_snapshot,
        input_traces.data(),
        static_cast<std::uint32_t>(input_traces.size()));
    if (!second_write.Succeeded()) {
        return Fail("serialize bridge deterministic second write failed");
    }

    if (write_result.state.committed_byte_count != second_write.state.committed_byte_count) {
        return Fail("serialize bridge deterministic byte count changed");
    }

    if (!SerializeBytesMatch(first_buffer, second_buffer, write_result.state.committed_byte_count)) {
        return Fail("serialize bridge deterministic bytes changed");
    }

    SerializeReader reader(first_buffer.data(), write_result.state.committed_byte_count);
    if (OpenSerializeStream(reader) != 0) {
        return 1;
    }

    WorldSnapshot output_snapshot{};
    std::array<WorldPhaseTrace, 2U> output_traces{};
    std::uint32_t output_trace_count = 0U;
    const WorldSerializeSnapshotResult read_result = bridge.ReadSnapshot(
        &reader,
        &output_snapshot,
        output_traces.data(),
        static_cast<std::uint32_t>(output_traces.size()),
        &output_trace_count);
    if (!read_result.Succeeded()) {
        return Fail("serialize bridge round trip read failed");
    }

    if (!WorldSnapshotsMatch(input_snapshot, output_snapshot)) {
        return Fail("serialize bridge round trip world snapshot changed");
    }

    if (output_trace_count != input_traces.size()) {
        return Fail("serialize bridge round trip trace count changed");
    }

    if (!PhaseTracesMatch(input_traces[0], output_traces[0])) {
        return Fail("serialize bridge round trip first trace changed");
    }

    if (!PhaseTracesMatch(input_traces[1], output_traces[1])) {
        return Fail("serialize bridge round trip second trace changed");
    }

    return 0;
}

int WorldSerializeSnapshotBridgeWritePhaseTraceRecordsInOrder() {
    const WorldSnapshot input_snapshot = SerializableWorldSnapshot(3U);
    const std::array<WorldPhaseTrace, 3U> input_traces{
        Trace(WorldUpdatePhase::FrameStep, 9U, 3U, 0U),
        Trace(WorldUpdatePhase::BeginFrame, 9U, 3U, 1U),
        Trace(WorldUpdatePhase::FixedStep, 9U, 2U, 2U)};
    SerializeBuffer buffer{};
    WorldSerializeSnapshotBridge bridge;
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    const WorldSerializeSnapshotResult write_result = bridge.WriteSnapshot(
        &writer,
        input_snapshot,
        input_traces.data(),
        static_cast<std::uint32_t>(input_traces.size()));
    if (!write_result.Succeeded()) {
        return Fail("serialize bridge trace order write failed");
    }

    SerializeReader reader(buffer.data(), write_result.state.committed_byte_count);
    if (OpenSerializeStream(reader) != 0) {
        return 1;
    }

    WorldSnapshot output_snapshot{};
    std::array<WorldPhaseTrace, 3U> output_traces{};
    std::uint32_t output_trace_count = 0U;
    const WorldSerializeSnapshotResult read_result = bridge.ReadSnapshot(
        &reader,
        &output_snapshot,
        output_traces.data(),
        static_cast<std::uint32_t>(output_traces.size()),
        &output_trace_count);
    if (!read_result.Succeeded()) {
        return Fail("serialize bridge trace order read failed");
    }

    if (output_trace_count != input_traces.size()) {
        return Fail("serialize bridge trace order count changed");
    }

    std::uint32_t trace_index = 0U;
    while (trace_index < output_trace_count) {
        if (!PhaseTracesMatch(input_traces[trace_index], output_traces[trace_index])) {
            return Fail("serialize bridge trace order changed");
        }

        ++trace_index;
    }

    return 0;
}

int WorldSerializeSnapshotBridgeWriteOptionalTransformSnapshotCounters() {
    const WorldSnapshot input_snapshot = SerializableWorldSnapshot(0U);
    const WorldTransformSnapshot input_transform = SerializableTransformSnapshot();
    SerializeBuffer buffer{};
    WorldSerializeSnapshotBridge bridge;
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    const WorldSerializeSnapshotResult write_result = bridge.WriteSnapshot(
        &writer,
        input_snapshot,
        nullptr,
        0U,
        &input_transform);
    if (!write_result.Succeeded()) {
        return Fail("serialize bridge transform write failed");
    }

    if (write_result.state.transform_snapshot_count != 1U) {
        return Fail("serialize bridge transform write did not report optional record");
    }

    SerializeReader reader(buffer.data(), write_result.state.committed_byte_count);
    if (OpenSerializeStream(reader) != 0) {
        return 1;
    }

    WorldSnapshot output_snapshot{};
    std::uint32_t output_trace_count = 0U;
    WorldTransformSnapshot output_transform{};
    const WorldSerializeSnapshotResult read_result = bridge.ReadSnapshot(
        &reader,
        &output_snapshot,
        nullptr,
        0U,
        &output_trace_count,
        &output_transform);
    if (!read_result.Succeeded()) {
        return Fail("serialize bridge transform read failed");
    }

    if (!TransformSnapshotsMatch(input_transform, output_transform)) {
        return Fail("serialize bridge transform counters changed");
    }

    if (bridge.Snapshot().read_transform_snapshot_count != 1U) {
        return Fail("serialize bridge transform read count was not recorded");
    }

    return 0;
}

int WorldSerializeSnapshotBridgeReadRejectsSmallTraceOutputWithoutOverrun() {
    const WorldSnapshot input_snapshot = SerializableWorldSnapshot(2U);
    const std::array<WorldPhaseTrace, 2U> input_traces{
        Trace(WorldUpdatePhase::BeginFrame, 11U, 2U, 0U),
        Trace(WorldUpdatePhase::EndFrame, 11U, 2U, 0U)};
    SerializeBuffer buffer{};
    WorldSerializeSnapshotBridge bridge;
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    const WorldSerializeSnapshotResult write_result = bridge.WriteSnapshot(
        &writer,
        input_snapshot,
        input_traces.data(),
        static_cast<std::uint32_t>(input_traces.size()));
    if (!write_result.Succeeded()) {
        return Fail("serialize bridge small output fixture write failed");
    }

    SerializeReader reader(buffer.data(), write_result.state.committed_byte_count);
    if (OpenSerializeStream(reader) != 0) {
        return 1;
    }

    WorldSnapshot output_snapshot{};
    std::array<WorldPhaseTrace, 2U> output_traces{
        Trace(WorldUpdatePhase::FixedStep, 99U, 99U, 99U),
        Trace(WorldUpdatePhase::FrameStep, 98U, 98U, 98U)};
    const std::array<WorldPhaseTrace, 2U> before_traces = output_traces;
    std::uint32_t output_trace_count = 55U;
    const WorldSerializeSnapshotResult read_result = bridge.ReadSnapshot(
        &reader,
        &output_snapshot,
        output_traces.data(),
        1U,
        &output_trace_count);
    if (read_result.status != WorldSerializeSnapshotStatus::TraceCapacityExceeded) {
        return Fail("serialize bridge small output returned wrong status");
    }

    if (output_trace_count != 55U) {
        return Fail("serialize bridge small output changed trace count");
    }

    if (!PhaseTracesMatch(before_traces[0], output_traces[0])) {
        return Fail("serialize bridge small output changed first trace");
    }

    if (!PhaseTracesMatch(before_traces[1], output_traces[1])) {
        return Fail("serialize bridge small output changed second trace");
    }

    return 0;
}

int WorldSerializeSnapshotBridgeWriteRejectsInvalidTraceBufferWithoutMutation() {
    const WorldSnapshot input_snapshot = SerializableWorldSnapshot(1U);
    SerializeBuffer buffer{};
    WorldSerializeSnapshotBridge bridge;
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    const auto before_writer = writer.Snapshot();
    const WorldSerializeSnapshotBridgeSnapshot before_bridge = bridge.Snapshot();
    const WorldSerializeSnapshotResult result = bridge.WriteSnapshot(&writer, input_snapshot, nullptr, 1U);
    if (result.status != WorldSerializeSnapshotStatus::InvalidTraceBuffer) {
        return Fail("serialize bridge invalid trace returned wrong status");
    }

    const auto after_writer = writer.Snapshot();
    if (after_writer.committed_byte_count != before_writer.committed_byte_count) {
        return Fail("serialize bridge invalid trace wrote bytes");
    }

    if (after_writer.record_count != before_writer.record_count) {
        return Fail("serialize bridge invalid trace wrote record");
    }

    const WorldSerializeSnapshotBridgeSnapshot after_bridge = bridge.Snapshot();
    if (after_bridge.written_snapshot_count != before_bridge.written_snapshot_count) {
        return Fail("serialize bridge invalid trace mutated write count");
    }

    if (after_bridge.written_trace_count != before_bridge.written_trace_count) {
        return Fail("serialize bridge invalid trace mutated trace count");
    }

    return 0;
}

int WorldSerializeSnapshotBridgeWriteRejectsTraceOverflowWithoutMutation() {
    WorldSerializeSnapshotBridgeDesc desc{};
    desc.phase_trace_capacity = 1U;
    WorldSerializeSnapshotBridge bridge(desc);
    const WorldSnapshot input_snapshot = SerializableWorldSnapshot(2U);
    const std::array<WorldPhaseTrace, 2U> input_traces{
        Trace(WorldUpdatePhase::BeginFrame, 12U, 2U, 0U),
        Trace(WorldUpdatePhase::EndFrame, 12U, 2U, 0U)};
    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    const auto before_writer = writer.Snapshot();
    const WorldSerializeSnapshotResult result = bridge.WriteSnapshot(
        &writer,
        input_snapshot,
        input_traces.data(),
        static_cast<std::uint32_t>(input_traces.size()));
    if (result.status != WorldSerializeSnapshotStatus::TraceCapacityExceeded) {
        return Fail("serialize bridge trace overflow returned wrong status");
    }

    const auto after_writer = writer.Snapshot();
    if (after_writer.committed_byte_count != before_writer.committed_byte_count) {
        return Fail("serialize bridge trace overflow wrote bytes");
    }

    if (bridge.Snapshot().written_trace_count != 0U) {
        return Fail("serialize bridge trace overflow mutated trace count");
    }

    return 0;
}

int WorldSerializeSnapshotBridgeSerializeFailureMapsExplicitStatus() {
    const WorldSnapshot input_snapshot = SerializableWorldSnapshot(0U);
    std::array<std::uint8_t, STREAM_HEADER_BYTE_COUNT> buffer{};
    WorldSerializeSnapshotBridge bridge;
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    const WorldSerializeSnapshotResult result = bridge.WriteSnapshot(&writer, input_snapshot, nullptr, 0U);
    if (result.status != WorldSerializeSnapshotStatus::SerializeFailure) {
        return Fail("serialize bridge write failure returned wrong bridge status");
    }

    if (result.serialize_status == SerializeStatus::Success) {
        return Fail("serialize bridge write failure did not report serialize status");
    }

    const WorldSerializeSnapshotBridgeSnapshot snapshot = bridge.Snapshot();
    if (snapshot.last_status != WorldSerializeSnapshotStatus::SerializeFailure) {
        return Fail("serialize bridge write failure did not record last status");
    }

    if (snapshot.failed_operation_count != 1U) {
        return Fail("serialize bridge write failure did not count failure");
    }

    SerializeBuffer partial_buffer{};
    WorldSerializeSnapshotBridge partial_bridge;
    SerializeWriter partial_writer(partial_buffer.data(), static_cast<std::uint32_t>(partial_buffer.size()));
    if (BeginSerializeStream(partial_writer) != 0) {
        return 1;
    }

    if (FillWriterNearStreamCapacity(partial_writer) != 0) {
        return 1;
    }

    const auto before_partial_writer = partial_writer.Snapshot();
    const WorldSerializeSnapshotResult partial_result =
        partial_bridge.WriteSnapshot(&partial_writer, input_snapshot, nullptr, 0U);
    if (partial_result.status != WorldSerializeSnapshotStatus::SerializeFailure) {
        return Fail("serialize bridge partial write failure returned wrong bridge status");
    }

    if (partial_result.serialize_status != SerializeStatus::BufferTooSmall) {
        return Fail("serialize bridge partial write failure returned wrong serialize status");
    }

    const auto after_partial_writer = partial_writer.Snapshot();
    if (after_partial_writer.committed_byte_count != before_partial_writer.committed_byte_count) {
        return Fail("serialize bridge partial write failure mutated byte count");
    }

    if (after_partial_writer.record_count != before_partial_writer.record_count) {
        return Fail("serialize bridge partial write failure mutated record count");
    }

    if (after_partial_writer.field_count != before_partial_writer.field_count) {
        return Fail("serialize bridge partial write failure mutated field count");
    }

    if (after_partial_writer.accepted_operation_count != before_partial_writer.accepted_operation_count) {
        return Fail("serialize bridge partial write failure mutated accepted count");
    }

    if (after_partial_writer.failed_operation_count != before_partial_writer.failed_operation_count) {
        return Fail("serialize bridge partial write failure mutated failed count");
    }

    if (after_partial_writer.last_status != before_partial_writer.last_status) {
        return Fail("serialize bridge partial write failure mutated last status");
    }

    const WorldSerializeSnapshotBridgeSnapshot partial_snapshot = partial_bridge.Snapshot();
    if (partial_snapshot.failed_operation_count != 1U) {
        return Fail("serialize bridge partial write failure did not count failure");
    }

    return 0;
}

int WorldSerializeSnapshotBridgeReadFailureMapsExplicitStatus() {
    SerializeBuffer buffer{};
    WorldSerializeSnapshotBridge bridge;
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    if (OpenSerializeStream(reader) != 0) {
        return 1;
    }

    WorldSnapshot output_snapshot{};
    std::uint32_t output_trace_count = 0U;
    const WorldSerializeSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        &output_snapshot,
        nullptr,
        0U,
        &output_trace_count);
    if (result.status != WorldSerializeSnapshotStatus::SerializeFailure) {
        return Fail("serialize bridge read failure returned wrong bridge status");
    }

    if (result.serialize_status == SerializeStatus::Success) {
        return Fail("serialize bridge read failure did not report serialize status");
    }

    if (bridge.Snapshot().last_serialize_status == SerializeStatus::Success) {
        return Fail("serialize bridge read failure did not record serialize status");
    }

    return 0;
}

int WorldSerializeSnapshotBridgeReadRejectsInvalidEnumValuesWithoutMutation() {
    SerializeBuffer buffer{};
    WorldSerializeSnapshotBridge bridge;
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    if (WriteInvalidEnumWorldStream(writer) != 0) {
        return 1;
    }

    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    if (OpenSerializeStream(reader) != 0) {
        return 1;
    }

    WorldSnapshot output_snapshot = SerializableWorldSnapshot(0U);
    const WorldSnapshot before_snapshot = output_snapshot;
    std::array<WorldPhaseTrace, 1U> output_traces{};
    std::uint32_t output_trace_count = 77U;
    const WorldSerializeSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        &output_snapshot,
        output_traces.data(),
        static_cast<std::uint32_t>(output_traces.size()),
        &output_trace_count);
    if (result.status != WorldSerializeSnapshotStatus::InvalidEnumValue) {
        return Fail("serialize bridge invalid enum returned wrong status");
    }

    if (!WorldSnapshotsMatch(before_snapshot, output_snapshot)) {
        return Fail("serialize bridge invalid enum mutated output snapshot");
    }

    if (output_trace_count != 77U) {
        return Fail("serialize bridge invalid enum mutated output trace count");
    }

    return 0;
}

int WorldSerializeSnapshotBridgeNoWorldMutationDuringReadWrite() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("serialize bridge world mutation registration failed");
    }

    const WorldSnapshot before_world = world.Snapshot();
    SerializeBuffer buffer{};
    WorldSerializeSnapshotBridge bridge;
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    const WorldSerializeSnapshotResult write_result = bridge.WriteSnapshot(&writer, before_world, nullptr, 0U);
    if (!write_result.Succeeded()) {
        return Fail("serialize bridge world mutation write failed");
    }

    SerializeReader reader(buffer.data(), write_result.state.committed_byte_count);
    if (OpenSerializeStream(reader) != 0) {
        return 1;
    }

    WorldSnapshot output_snapshot{};
    std::uint32_t output_trace_count = 0U;
    const WorldSerializeSnapshotResult read_result = bridge.ReadSnapshot(
        &reader,
        &output_snapshot,
        nullptr,
        0U,
        &output_trace_count);
    if (!read_result.Succeeded()) {
        return Fail("serialize bridge world mutation read failed");
    }

    const WorldSnapshot after_world = world.Snapshot();
    if (!WorldSnapshotsMatch(before_world, after_world)) {
        return Fail("serialize bridge mutated world instance");
    }

    return 0;
}

int WorldSerializeSnapshotBridgeReadWritePathDoesNotGrowStorage() {
    const WorldSnapshot input_snapshot = SerializableWorldSnapshot(1U);
    const std::array<WorldPhaseTrace, 1U> input_traces{Trace(WorldUpdatePhase::BeginFrame, 13U, 1U, 0U)};
    WorldSerializeSnapshotBridge bridge;
    const WorldSerializeSnapshotBridgeSnapshot before_snapshot = bridge.Snapshot();
    std::uint32_t iteration = 0U;
    while (iteration < 3U) {
        SerializeBuffer buffer{};
        SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
        if (BeginSerializeStream(writer) != 0) {
            return 1;
        }

        const WorldSerializeSnapshotResult write_result = bridge.WriteSnapshot(
            &writer,
            input_snapshot,
            input_traces.data(),
            static_cast<std::uint32_t>(input_traces.size()));
        if (!write_result.Succeeded()) {
            return Fail("serialize bridge path write failed");
        }

        SerializeReader reader(buffer.data(), write_result.state.committed_byte_count);
        if (OpenSerializeStream(reader) != 0) {
            return 1;
        }

        WorldSnapshot output_snapshot{};
        std::array<WorldPhaseTrace, 1U> output_traces{};
        std::uint32_t output_trace_count = 0U;
        const WorldSerializeSnapshotResult read_result = bridge.ReadSnapshot(
            &reader,
            &output_snapshot,
            output_traces.data(),
            static_cast<std::uint32_t>(output_traces.size()),
            &output_trace_count);
        if (!read_result.Succeeded()) {
            return Fail("serialize bridge path read failed");
        }

        ++iteration;
    }

    const WorldSerializeSnapshotBridgeSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.phase_trace_capacity != before_snapshot.phase_trace_capacity) {
        return Fail("serialize bridge path mutated capacity");
    }

    if (after_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("serialize bridge path changed allocation accounting");
    }

    return 0;
}

int WorldSerializeSnapshotBridgeSnapshotReportsCountsAndLastStatus() {
    const WorldSnapshot input_snapshot = SerializableWorldSnapshot(1U);
    const std::array<WorldPhaseTrace, 1U> input_traces{Trace(WorldUpdatePhase::BeginFrame, 14U, 1U, 0U)};
    SerializeBuffer buffer{};
    WorldSerializeSnapshotBridge bridge;
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    const WorldSerializeSnapshotResult write_result = bridge.WriteSnapshot(
        &writer,
        input_snapshot,
        input_traces.data(),
        static_cast<std::uint32_t>(input_traces.size()));
    if (!write_result.Succeeded()) {
        return Fail("serialize bridge snapshot write failed");
    }

    SerializeReader reader(buffer.data(), write_result.state.committed_byte_count);
    if (OpenSerializeStream(reader) != 0) {
        return 1;
    }

    WorldSnapshot output_snapshot{};
    std::array<WorldPhaseTrace, 1U> output_traces{};
    std::uint32_t output_trace_count = 0U;
    const WorldSerializeSnapshotResult read_result = bridge.ReadSnapshot(
        &reader,
        &output_snapshot,
        output_traces.data(),
        static_cast<std::uint32_t>(output_traces.size()),
        &output_trace_count);
    if (!read_result.Succeeded()) {
        return Fail("serialize bridge snapshot read failed");
    }

    const WorldSerializeSnapshotResult failure_result = bridge.ReadSnapshot(
        nullptr,
        &output_snapshot,
        output_traces.data(),
        static_cast<std::uint32_t>(output_traces.size()),
        &output_trace_count);
    if (failure_result.status != WorldSerializeSnapshotStatus::InvalidReader) {
        return Fail("serialize bridge snapshot failure returned wrong status");
    }

    const WorldSerializeSnapshotBridgeSnapshot snapshot = bridge.Snapshot();
    if (snapshot.written_snapshot_count != 1U) {
        return Fail("serialize bridge snapshot did not report write count");
    }

    if (snapshot.written_trace_count != 1U) {
        return Fail("serialize bridge snapshot did not report written trace count");
    }

    if (snapshot.read_snapshot_count != 1U) {
        return Fail("serialize bridge snapshot did not report read count");
    }

    if (snapshot.read_trace_count != 1U) {
        return Fail("serialize bridge snapshot did not report read trace count");
    }

    if (snapshot.skipped_optional_record_count != 2U) {
        return Fail("serialize bridge snapshot did not report skipped optional records");
    }

    if (snapshot.failed_operation_count != 1U) {
        return Fail("serialize bridge snapshot did not report failure count");
    }

    if (snapshot.last_status != WorldSerializeSnapshotStatus::InvalidReader) {
        return Fail("serialize bridge snapshot did not report last status");
    }

    return 0;
}

int WorldSerializeSnapshotBridgeNoFilePackageResourceSaveGameOrGameAdapterDependency() {
    WorldSerializeSnapshotBridge bridge;
    const WorldSerializeSnapshotBridgeSnapshot snapshot = bridge.Snapshot();
    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("serialize bridge dependency test changed allocation vocabulary");
    }

    if (snapshot.phase_trace_capacity != MAX_WORLD_SERIALIZE_PHASE_TRACE_COUNT) {
        return Fail("serialize bridge dependency test changed default capacity");
    }

    return 0;
}

int WorldSerializeSnapshotBridgeNoActorComponentSceneGraphOrGameplayDependency() {
    const WorldSnapshot input_snapshot = SerializableWorldSnapshot(2U);
    const std::array<WorldPhaseTrace, 2U> input_traces{
        Trace(WorldUpdatePhase::FixedStep, 15U, 2U, 0U),
        Trace(WorldUpdatePhase::FrameStep, 15U, 1U, 1U)};
    SerializeBuffer buffer{};
    WorldSerializeSnapshotBridge bridge;
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    const WorldSerializeSnapshotResult result = bridge.WriteSnapshot(
        &writer,
        input_snapshot,
        input_traces.data(),
        static_cast<std::uint32_t>(input_traces.size()));
    if (!result.Succeeded()) {
        return Fail("serialize bridge actor boundary write failed");
    }

    if (bridge.Snapshot().written_trace_count != 2U) {
        return Fail("serialize bridge actor boundary did not remain trace based");
    }

    return 0;
}

int WorldSerializeSnapshotBridgeWorldInstanceCoreRemainsSerializeFree() {
    WorldInstance world = MakeWorld(4U, 8U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("serialize bridge world core-free registration failed");
    }

    const WorldSnapshot before_snapshot = world.Snapshot();
    WorldSerializeSnapshotBridge bridge;
    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    const WorldSerializeSnapshotResult result = bridge.WriteSnapshot(&writer, before_snapshot, nullptr, 0U);
    if (!result.Succeeded()) {
        return Fail("serialize bridge world core-free write failed");
    }

    const WorldSnapshot after_snapshot = world.Snapshot();
    if (!WorldSnapshotsMatch(before_snapshot, after_snapshot)) {
        return Fail("serialize bridge world core-free mutated world");
    }

    return 0;
}

int WorldSerializeSnapshotBridgeSerializeCoreRemainsWorldFree() {
    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    const yuengine::serialize::SerializeRecordId record{21U};
    const yuengine::serialize::SerializeFieldId field{31U};
    if (writer.BeginRecord(record) != SerializeStatus::Success) {
        return Fail("serialize core-free begin record failed");
    }

    if (writer.WriteUInt32(field, 77U) != SerializeStatus::Success) {
        return Fail("serialize core-free write failed");
    }

    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    if (OpenSerializeStream(reader) != 0) {
        return 1;
    }

    std::uint32_t value = 0U;
    if (reader.ReadUInt32(record, field, value) != SerializeStatus::Success) {
        return Fail("serialize core-free read failed");
    }

    if (value != 77U) {
        return Fail("serialize core-free value changed");
    }

    return 0;
}

int WorldResourceBindingBridgeBindValidResourceAcquiresHandle() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("resource binding valid fixture registration failed");
    }

    WorldResourceBindingBridge bridge;
    const WorldResourceBindingResult result = bridge.Bind(
        &registry,
        OBJECT_PLAYER,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!result.Succeeded()) {
        return Fail("resource binding valid bind failed");
    }

    if (result.world_object_id.value != OBJECT_PLAYER.value) {
        return Fail("resource binding valid returned wrong world object id");
    }

    if (result.resource_handle.slot != resource.handle.slot) {
        return Fail("resource binding valid returned wrong resource slot");
    }

    const WorldResourceBindingSnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.active_binding_count != 1U) {
        return Fail("resource binding valid did not record active binding");
    }

    if (bridge_snapshot.acquired_binding_count != 1U) {
        return Fail("resource binding valid did not record acquired binding");
    }

    if (registry.Snapshot().acquired_handle_count != 1U) {
        return Fail("resource binding valid did not acquire resource handle");
    }

    return 0;
}

int WorldResourceBindingBridgeBindRejectsNullRegistryWithoutMutation() {
    WorldResourceBindingBridge bridge;
    const WorldResourceBindingSnapshot before_snapshot = bridge.Snapshot();
    const WorldResourceBindingResult result = bridge.Bind(
        nullptr,
        OBJECT_PLAYER,
        ResourceHandle{},
        RESOURCE_TYPE_TEXTURE);
    if (result.status != WorldResourceBindingStatus::InvalidResourceRegistry) {
        return Fail("resource binding null registry returned wrong status");
    }

    const WorldResourceBindingSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.active_binding_count != before_snapshot.active_binding_count) {
        return Fail("resource binding null registry mutated active count");
    }

    if (after_snapshot.acquired_binding_count != before_snapshot.acquired_binding_count) {
        return Fail("resource binding null registry mutated acquired count");
    }

    return 0;
}

int WorldResourceBindingBridgeBindRejectsInvalidWorldIdWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("resource binding invalid world fixture registration failed");
    }

    WorldResourceBindingBridge bridge;
    const ResourceSnapshot before_registry = registry.Snapshot();
    const WorldResourceBindingResult result = bridge.Bind(
        &registry,
        WorldObjectId{},
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (result.status != WorldResourceBindingStatus::InvalidWorldObjectId) {
        return Fail("resource binding invalid world returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 0U) {
        return Fail("resource binding invalid world mutated binding count");
    }

    if (!ResourceSnapshotsMatch(before_registry, registry.Snapshot())) {
        return Fail("resource binding invalid world mutated registry");
    }

    return 0;
}

int WorldResourceBindingBridgeBindRejectsInvalidResourceHandleWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    WorldResourceBindingBridge bridge;
    const ResourceSnapshot before_registry = registry.Snapshot();
    const WorldResourceBindingResult result = bridge.Bind(
        &registry,
        OBJECT_PLAYER,
        ResourceHandle{},
        RESOURCE_TYPE_TEXTURE);
    if (result.status != WorldResourceBindingStatus::InvalidResourceHandle) {
        return Fail("resource binding invalid handle returned wrong status");
    }

    if (result.resource_status != ResourceStatus::InvalidHandle) {
        return Fail("resource binding invalid handle returned wrong resource status");
    }

    if (bridge.Snapshot().active_binding_count != 0U) {
        return Fail("resource binding invalid handle mutated binding count");
    }

    if (registry.Snapshot().acquired_handle_count != before_registry.acquired_handle_count) {
        return Fail("resource binding invalid handle acquired resource");
    }

    return 0;
}

int WorldResourceBindingBridgeBindRejectsStaleResourceHandleWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("resource binding stale fixture registration failed");
    }

    if (registry.Retire(resource.handle) != ResourceStatus::Success) {
        return Fail("resource binding stale fixture retire failed");
    }

    WorldResourceBindingBridge bridge;
    const WorldResourceBindingResult result = bridge.Bind(
        &registry,
        OBJECT_PLAYER,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (result.status != WorldResourceBindingStatus::StaleResourceHandle) {
        return Fail("resource binding stale handle returned wrong status");
    }

    if (result.resource_status != ResourceStatus::GenerationMismatch) {
        return Fail("resource binding stale handle returned wrong resource status");
    }

    if (bridge.Snapshot().active_binding_count != 0U) {
        return Fail("resource binding stale handle mutated binding count");
    }

    return 0;
}

int WorldResourceBindingBridgeBindRejectsTypeMismatchWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_a");
    if (!resource.Succeeded()) {
        return Fail("resource binding type mismatch fixture registration failed");
    }

    WorldResourceBindingBridge bridge;
    const WorldResourceBindingResult result = bridge.Bind(
        &registry,
        OBJECT_PLAYER,
        resource.handle,
        RESOURCE_TYPE_MATERIAL);
    if (result.status != WorldResourceBindingStatus::ResourceTypeMismatch) {
        return Fail("resource binding type mismatch returned wrong status");
    }

    if (result.resource_status != ResourceStatus::TypeMismatch) {
        return Fail("resource binding type mismatch returned wrong resource status");
    }

    if (registry.Snapshot().acquired_handle_count != 0U) {
        return Fail("resource binding type mismatch acquired resource");
    }

    return 0;
}

int WorldResourceBindingBridgeBindRejectsDuplicateWorldObjectId() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult first_resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_a");
    const ResourceRegistrationResult second_resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_b");
    if (!first_resource.Succeeded() || !second_resource.Succeeded()) {
        return Fail("resource binding duplicate fixture registration failed");
    }

    WorldResourceBindingBridge bridge;
    if (!bridge.Bind(&registry, OBJECT_PLAYER, first_resource.handle, RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("resource binding duplicate first bind failed");
    }

    const ResourceSnapshot before_registry = registry.Snapshot();
    const WorldResourceBindingResult duplicate_result = bridge.Bind(
        &registry,
        OBJECT_PLAYER,
        second_resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (duplicate_result.status != WorldResourceBindingStatus::DuplicateWorldObjectId) {
        return Fail("resource binding duplicate world returned wrong status");
    }

    if (registry.Snapshot().acquired_handle_count != before_registry.acquired_handle_count) {
        return Fail("resource binding duplicate world acquired resource");
    }

    return 0;
}

int WorldResourceBindingBridgeBindRejectsCapacityOverflowWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult first_resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_a");
    const ResourceRegistrationResult second_resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_b");
    if (!first_resource.Succeeded() || !second_resource.Succeeded()) {
        return Fail("resource binding capacity fixture registration failed");
    }

    WorldResourceBindingBridgeDesc desc{};
    desc.bridge_capacity = 1U;
    WorldResourceBindingBridge bridge(desc);
    if (!bridge.Bind(&registry, OBJECT_PLAYER, first_resource.handle, RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("resource binding capacity first bind failed");
    }

    const ResourceSnapshot before_registry = registry.Snapshot();
    const WorldResourceBindingResult overflow_result = bridge.Bind(
        &registry,
        OBJECT_CAMERA,
        second_resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (overflow_result.status != WorldResourceBindingStatus::CapacityExceeded) {
        return Fail("resource binding capacity overflow returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 1U) {
        return Fail("resource binding capacity overflow mutated binding count");
    }

    if (registry.Snapshot().acquired_handle_count != before_registry.acquired_handle_count) {
        return Fail("resource binding capacity overflow acquired resource");
    }

    return 0;
}

int WorldResourceBindingBridgeRemoveReleasesHandle() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("resource binding remove fixture registration failed");
    }

    WorldResourceBindingBridge bridge;
    if (!bridge.Bind(&registry, OBJECT_PLAYER, resource.handle, RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("resource binding remove bind failed");
    }

    const WorldResourceBindingStatus remove_status = bridge.Remove(&registry, OBJECT_PLAYER);
    if (remove_status != WorldResourceBindingStatus::Success) {
        return Fail("resource binding remove returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 0U) {
        return Fail("resource binding remove did not clear binding");
    }

    if (registry.Snapshot().acquired_handle_count != 0U) {
        return Fail("resource binding remove did not release resource");
    }

    return 0;
}

int WorldResourceBindingBridgeRemoveRejectsNullRegistryWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("resource binding remove null fixture registration failed");
    }

    WorldResourceBindingBridge bridge;
    if (!bridge.Bind(&registry, OBJECT_PLAYER, resource.handle, RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("resource binding remove null bind failed");
    }

    const WorldResourceBindingStatus remove_status = bridge.Remove(nullptr, OBJECT_PLAYER);
    if (remove_status != WorldResourceBindingStatus::InvalidResourceRegistry) {
        return Fail("resource binding remove null returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 1U) {
        return Fail("resource binding remove null cleared binding");
    }

    if (registry.Snapshot().acquired_handle_count != 1U) {
        return Fail("resource binding remove null released resource");
    }

    return 0;
}

int WorldResourceBindingBridgeRemoveRejectsMissingWorldObjectWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    WorldResourceBindingBridge bridge;
    const ResourceSnapshot before_registry = registry.Snapshot();
    const WorldResourceBindingStatus remove_status = bridge.Remove(&registry, OBJECT_PLAYER);
    if (remove_status != WorldResourceBindingStatus::BindingNotFound) {
        return Fail("resource binding remove missing returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 0U) {
        return Fail("resource binding remove missing mutated binding count");
    }

    if (!ResourceSnapshotsMatch(before_registry, registry.Snapshot())) {
        return Fail("resource binding remove missing mutated registry");
    }

    return 0;
}

int WorldResourceBindingBridgeRemoveReleaseFailureKeepsBinding() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("resource binding remove failure fixture registration failed");
    }

    WorldResourceBindingBridge bridge;
    if (!bridge.Bind(&registry, OBJECT_PLAYER, resource.handle, RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("resource binding remove failure bind failed");
    }

    if (registry.Release(resource.handle) != ResourceStatus::Success) {
        return Fail("resource binding remove failure external release failed");
    }

    const WorldResourceBindingStatus remove_status = bridge.Remove(&registry, OBJECT_PLAYER);
    if (remove_status != WorldResourceBindingStatus::ResourceReleaseFailed) {
        return Fail("resource binding remove failure returned wrong status");
    }

    const WorldResourceBindingSnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.active_binding_count != 1U) {
        return Fail("resource binding remove failure cleared binding");
    }

    if (bridge_snapshot.last_resource_status != ResourceStatus::NotAcquired) {
        return Fail("resource binding remove failure recorded wrong resource status");
    }

    if (!bridge.Query(OBJECT_PLAYER).Succeeded()) {
        return Fail("resource binding remove failure lost query binding");
    }

    return 0;
}

int WorldResourceBindingBridgeClearReleasesAllHandles() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult first_resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_a");
    const ResourceRegistrationResult second_resource = RegisterResource(registry, RESOURCE_TYPE_AUDIO, "audio_a");
    if (!first_resource.Succeeded() || !second_resource.Succeeded()) {
        return Fail("resource binding clear fixture registration failed");
    }

    WorldResourceBindingBridge bridge;
    if (!bridge.Bind(&registry, OBJECT_PLAYER, first_resource.handle, RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("resource binding clear first bind failed");
    }

    if (!bridge.Bind(&registry, OBJECT_CAMERA, second_resource.handle, RESOURCE_TYPE_AUDIO).Succeeded()) {
        return Fail("resource binding clear second bind failed");
    }

    const WorldResourceBindingStatus clear_status = bridge.Clear(&registry);
    if (clear_status != WorldResourceBindingStatus::Success) {
        return Fail("resource binding clear returned wrong status");
    }

    const WorldResourceBindingSnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.active_binding_count != 0U) {
        return Fail("resource binding clear did not clear bindings");
    }

    if (bridge_snapshot.cleared_binding_count != 2U) {
        return Fail("resource binding clear did not count cleared bindings");
    }

    if (registry.Snapshot().acquired_handle_count != 0U) {
        return Fail("resource binding clear did not release all resources");
    }

    return 0;
}

int WorldResourceBindingBridgeClearRejectsNullRegistryWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("resource binding clear null fixture registration failed");
    }

    WorldResourceBindingBridge bridge;
    if (!bridge.Bind(&registry, OBJECT_PLAYER, resource.handle, RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("resource binding clear null bind failed");
    }

    const WorldResourceBindingStatus clear_status = bridge.Clear(nullptr);
    if (clear_status != WorldResourceBindingStatus::InvalidResourceRegistry) {
        return Fail("resource binding clear null returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 1U) {
        return Fail("resource binding clear null cleared binding");
    }

    if (registry.Snapshot().acquired_handle_count != 1U) {
        return Fail("resource binding clear null released resource");
    }

    return 0;
}

int WorldResourceBindingBridgeClearReleaseFailurePreservesUnreleasedBindings() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult first_resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_a");
    const ResourceRegistrationResult second_resource = RegisterResource(registry, RESOURCE_TYPE_AUDIO, "audio_a");
    if (!first_resource.Succeeded() || !second_resource.Succeeded()) {
        return Fail("resource binding clear failure fixture registration failed");
    }

    WorldResourceBindingBridge bridge;
    if (!bridge.Bind(&registry, OBJECT_PLAYER, first_resource.handle, RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("resource binding clear failure first bind failed");
    }

    if (!bridge.Bind(&registry, OBJECT_CAMERA, second_resource.handle, RESOURCE_TYPE_AUDIO).Succeeded()) {
        return Fail("resource binding clear failure second bind failed");
    }

    if (registry.Release(second_resource.handle) != ResourceStatus::Success) {
        return Fail("resource binding clear failure external release failed");
    }

    const WorldResourceBindingStatus clear_status = bridge.Clear(&registry);
    if (clear_status != WorldResourceBindingStatus::ResourceReleaseFailed) {
        return Fail("resource binding clear failure returned wrong status");
    }

    const WorldResourceBindingSnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.active_binding_count != 1U) {
        return Fail("resource binding clear failure did not preserve remaining binding");
    }

    if (bridge_snapshot.cleared_binding_count != 1U) {
        return Fail("resource binding clear failure counted wrong cleared bindings");
    }

    const WorldResourceBindingResult query_result = bridge.Query(OBJECT_CAMERA);
    if (!query_result.Succeeded()) {
        return Fail("resource binding clear failure lost unreleased binding");
    }

    return 0;
}

int WorldResourceBindingBridgeBoundResourceCannotRetireUntilReleased() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("resource binding retire fixture registration failed");
    }

    WorldResourceBindingBridge bridge;
    if (!bridge.Bind(&registry, OBJECT_PLAYER, resource.handle, RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("resource binding retire bind failed");
    }

    if (registry.Retire(resource.handle) != ResourceStatus::StillReferenced) {
        return Fail("resource binding retire while bound returned wrong status");
    }

    if (bridge.Remove(&registry, OBJECT_PLAYER) != WorldResourceBindingStatus::Success) {
        return Fail("resource binding retire remove failed");
    }

    if (registry.Retire(resource.handle) != ResourceStatus::Success) {
        return Fail("resource binding retire after remove failed");
    }

    return 0;
}

int WorldResourceBindingBridgeQueryReturnsStoredBinding() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry, RESOURCE_TYPE_MATERIAL, "material_a");
    if (!resource.Succeeded()) {
        return Fail("resource binding query fixture registration failed");
    }

    WorldResourceBindingBridge bridge;
    if (!bridge.Bind(&registry, OBJECT_PLAYER, resource.handle, RESOURCE_TYPE_MATERIAL).Succeeded()) {
        return Fail("resource binding query bind failed");
    }

    const WorldResourceBindingResult query_result = bridge.Query(OBJECT_PLAYER);
    if (!query_result.Succeeded()) {
        return Fail("resource binding query failed");
    }

    if (query_result.resource_handle.slot != resource.handle.slot) {
        return Fail("resource binding query returned wrong resource slot");
    }

    if (query_result.expected_resource_type.value != RESOURCE_TYPE_MATERIAL.value) {
        return Fail("resource binding query returned wrong resource type");
    }

    return 0;
}

int WorldResourceBindingBridgeQueryIsReadOnlyAndBounded() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("resource binding query readonly fixture registration failed");
    }

    WorldResourceBindingBridge bridge;
    if (!bridge.Bind(&registry, OBJECT_PLAYER, resource.handle, RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("resource binding query readonly bind failed");
    }

    const WorldResourceBindingSnapshot before_bridge = bridge.Snapshot();
    const ResourceSnapshot before_registry = registry.Snapshot();
    std::uint32_t query_index = 0U;
    while (query_index < 3U) {
        if (!bridge.Query(OBJECT_PLAYER).Succeeded()) {
            return Fail("resource binding query readonly query failed");
        }

        ++query_index;
    }

    const WorldResourceBindingSnapshot after_bridge = bridge.Snapshot();
    if (after_bridge.active_binding_count != before_bridge.active_binding_count) {
        return Fail("resource binding query readonly mutated active count");
    }

    if (after_bridge.acquired_binding_count != before_bridge.acquired_binding_count) {
        return Fail("resource binding query readonly mutated acquired count");
    }

    if (after_bridge.released_binding_count != before_bridge.released_binding_count) {
        return Fail("resource binding query readonly mutated release count");
    }

    if (after_bridge.failed_operation_count != before_bridge.failed_operation_count) {
        return Fail("resource binding query readonly mutated failure count");
    }

    if (!ResourceSnapshotsMatch(before_registry, registry.Snapshot())) {
        return Fail("resource binding query readonly mutated registry");
    }

    return 0;
}

int WorldResourceBindingBridgeUpdatePathDoesNotGrowWorldStorage() {
    ResourceRegistry registry = MakeResourceRegistry();
    WorldResourceBindingBridgeDesc desc{};
    desc.bridge_capacity = 2U;
    WorldResourceBindingBridge bridge(desc);
    const WorldResourceBindingSnapshot before_bridge = bridge.Snapshot();
    std::uint32_t iteration = 0U;
    while (iteration < 2U) {
        const char *key = iteration == 0U ? "texture_a" : "texture_b";
        const ResourceRegistrationResult resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, key);
        if (!resource.Succeeded()) {
            return Fail("resource binding update path registration failed");
        }

        if (!bridge.Bind(&registry, OBJECT_PLAYER, resource.handle, RESOURCE_TYPE_TEXTURE).Succeeded()) {
            return Fail("resource binding update path bind failed");
        }

        if (!bridge.Query(OBJECT_PLAYER).Succeeded()) {
            return Fail("resource binding update path query failed");
        }

        if (bridge.Remove(&registry, OBJECT_PLAYER) != WorldResourceBindingStatus::Success) {
            return Fail("resource binding update path remove failed");
        }

        ++iteration;
    }

    const WorldResourceBindingSnapshot after_bridge = bridge.Snapshot();
    if (after_bridge.bridge_capacity != before_bridge.bridge_capacity) {
        return Fail("resource binding update path changed capacity");
    }

    if (after_bridge.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("resource binding update path changed allocation accounting");
    }

    return 0;
}

int WorldResourceBindingBridgeDoesNotQueryOrMutateWorldInstance() {
    WorldInstance world = MakeWorld(4U, 4U);
    const WorldSnapshot before_world = world.Snapshot();
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("resource binding no-world-query registration failed");
    }

    WorldResourceBindingBridge bridge;
    const WorldResourceBindingResult result = bridge.Bind(
        &registry,
        OBJECT_PLAYER,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!result.Succeeded()) {
        return Fail("resource binding no-world-query bind failed");
    }

    const WorldSnapshot after_world = world.Snapshot();
    if (!WorldSnapshotsMatch(before_world, after_world)) {
        return Fail("resource binding no-world-query mutated world");
    }

    return 0;
}

int WorldResourceBindingBridgeSnapshotReportsCountsAndLastStatus() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("resource binding snapshot fixture registration failed");
    }

    WorldResourceBindingBridge bridge;
    if (!bridge.Bind(&registry, OBJECT_PLAYER, resource.handle, RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("resource binding snapshot bind failed");
    }

    const WorldResourceBindingStatus failure_status = bridge.Remove(nullptr, OBJECT_PLAYER);
    if (failure_status != WorldResourceBindingStatus::InvalidResourceRegistry) {
        return Fail("resource binding snapshot failure status failed");
    }

    const WorldResourceBindingSnapshot snapshot = bridge.Snapshot();
    if (snapshot.active_binding_count != 1U) {
        return Fail("resource binding snapshot active count wrong");
    }

    if (snapshot.acquired_binding_count != 1U) {
        return Fail("resource binding snapshot acquired count wrong");
    }

    if (snapshot.failed_operation_count != 1U) {
        return Fail("resource binding snapshot failure count wrong");
    }

    if (snapshot.last_status != WorldResourceBindingStatus::InvalidResourceRegistry) {
        return Fail("resource binding snapshot last status wrong");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("resource binding snapshot allocation accounting wrong");
    }

    return 0;
}

int WorldResourceBindingBridgeNoFilePackageLoadDecodeUploadOrGameAdapterDependency() {
    WorldResourceBindingBridge bridge;
    const WorldResourceBindingSnapshot snapshot = bridge.Snapshot();
    if (snapshot.bridge_capacity != MAX_WORLD_OBJECT_COUNT) {
        return Fail("resource binding dependency test changed default capacity");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("resource binding dependency test changed allocation accounting");
    }

    return 0;
}

int WorldResourceBindingBridgeWorldInstanceCoreRemainsResourceFree() {
    WorldInstance world = MakeWorld(4U, 4U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("resource binding world core-free registration failed");
    }

    const WorldSnapshot before_world = world.Snapshot();
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("resource binding world core-free resource registration failed");
    }

    WorldResourceBindingBridge bridge;
    const WorldResourceBindingResult result = bridge.Bind(
        &registry,
        OBJECT_EFFECT,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!result.Succeeded()) {
        return Fail("resource binding world core-free bind failed");
    }

    const WorldSnapshot after_world = world.Snapshot();
    if (!WorldSnapshotsMatch(before_world, after_world)) {
        return Fail("resource binding world core-free mutated world");
    }

    return 0;
}

int WorldResourceBindingBridgeResourceCoreRemainsWorldFree() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("resource binding resource core-free registration failed");
    }

    if (registry.Acquire(resource.handle, RESOURCE_TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("resource binding resource core-free acquire failed");
    }

    if (registry.Release(resource.handle) != ResourceStatus::Success) {
        return Fail("resource binding resource core-free release failed");
    }

    if (registry.Retire(resource.handle) != ResourceStatus::Success) {
        return Fail("resource binding resource core-free retire failed");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeBindValidAttachmentResourceAcquiresHandle() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding valid attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding valid resource registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    const WorldComponentResourceBindingResult result = BindComponentResource(
        bridge,
        &attachment_bridge,
        &registry,
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!result.Succeeded()) {
        return Fail("component resource binding valid bind failed");
    }

    if (result.world_object_id.value != OBJECT_PLAYER.value) {
        return Fail("component resource binding valid returned wrong world id");
    }

    if (result.component_type_id.value != COMPONENT_TYPE_PRIMARY.value) {
        return Fail("component resource binding valid returned wrong component type");
    }

    if (result.component_slot_id.value != COMPONENT_SLOT_PRIMARY.value) {
        return Fail("component resource binding valid returned wrong component slot");
    }

    if (result.resource_handle.slot != resource.handle.slot) {
        return Fail("component resource binding valid returned wrong resource slot");
    }

    const WorldComponentResourceBindingSnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.active_binding_count != 1U) {
        return Fail("component resource binding valid did not record active binding");
    }

    if (bridge_snapshot.acquired_binding_count != 1U) {
        return Fail("component resource binding valid did not record acquired binding");
    }

    if (registry.Snapshot().acquired_handle_count != 1U) {
        return Fail("component resource binding valid did not acquire resource handle");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeBindRejectsNullAttachmentSourceWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding null attachment fixture registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    const ResourceSnapshot before_registry = registry.Snapshot();
    const WorldComponentResourceBindingResult result = BindComponentResource(
        bridge,
        nullptr,
        &registry,
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (result.status != WorldComponentResourceBindingStatus::InvalidAttachmentSource) {
        return Fail("component resource binding null attachment returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 0U) {
        return Fail("component resource binding null attachment mutated binding count");
    }

    if (!ResourceSnapshotsMatch(before_registry, registry.Snapshot())) {
        return Fail("component resource binding null attachment mutated registry");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeBindRejectsNullRegistryWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding null registry attachment add failed") != 0) {
        return 1;
    }

    WorldComponentResourceBindingBridge bridge;
    const WorldComponentResourceBindingResult result = BindComponentResource(
        bridge,
        &attachment_bridge,
        nullptr,
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        ResourceHandle{},
        RESOURCE_TYPE_TEXTURE);
    if (result.status != WorldComponentResourceBindingStatus::InvalidResourceRegistry) {
        return Fail("component resource binding null registry returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 0U) {
        return Fail("component resource binding null registry mutated binding count");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeBindRejectsInvalidWorldIdWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding invalid world fixture registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    const ResourceSnapshot before_registry = registry.Snapshot();
    const WorldComponentResourceBindingResult result = BindComponentResource(
        bridge,
        &attachment_bridge,
        &registry,
        WorldObjectId{},
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (result.status != WorldComponentResourceBindingStatus::InvalidWorldObjectId) {
        return Fail("component resource binding invalid world returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 0U) {
        return Fail("component resource binding invalid world mutated binding count");
    }

    if (!ResourceSnapshotsMatch(before_registry, registry.Snapshot())) {
        return Fail("component resource binding invalid world mutated registry");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeBindRejectsInvalidComponentTypeWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding invalid type fixture registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    const ResourceSnapshot before_registry = registry.Snapshot();
    const WorldComponentResourceBindingResult result = BindComponentResource(
        bridge,
        &attachment_bridge,
        &registry,
        OBJECT_PLAYER,
        WorldComponentTypeId{},
        COMPONENT_SLOT_PRIMARY,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (result.status != WorldComponentResourceBindingStatus::InvalidComponentTypeId) {
        return Fail("component resource binding invalid type returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 0U) {
        return Fail("component resource binding invalid type mutated binding count");
    }

    if (!ResourceSnapshotsMatch(before_registry, registry.Snapshot())) {
        return Fail("component resource binding invalid type mutated registry");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeBindRejectsInvalidComponentSlotWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding invalid slot fixture registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    const ResourceSnapshot before_registry = registry.Snapshot();
    const WorldComponentResourceBindingResult result = BindComponentResource(
        bridge,
        &attachment_bridge,
        &registry,
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        WorldComponentSlotId{},
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (result.status != WorldComponentResourceBindingStatus::InvalidComponentSlotId) {
        return Fail("component resource binding invalid slot returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 0U) {
        return Fail("component resource binding invalid slot mutated binding count");
    }

    if (!ResourceSnapshotsMatch(before_registry, registry.Snapshot())) {
        return Fail("component resource binding invalid slot mutated registry");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeBindRejectsMissingAttachmentWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding missing attachment registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    const ResourceSnapshot before_registry = registry.Snapshot();
    const WorldComponentResourceBindingResult result = BindComponentResource(
        bridge,
        &attachment_bridge,
        &registry,
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (result.status != WorldComponentResourceBindingStatus::AttachmentNotFound) {
        return Fail("component resource binding missing attachment returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 0U) {
        return Fail("component resource binding missing attachment mutated binding count");
    }

    if (!ResourceSnapshotsMatch(before_registry, registry.Snapshot())) {
        return Fail("component resource binding missing attachment mutated registry");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeBindRejectsInvalidResourceHandleWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding invalid handle attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBindingBridge bridge;
    const ResourceSnapshot before_registry = registry.Snapshot();
    const WorldComponentResourceBindingResult result = BindComponentResource(
        bridge,
        &attachment_bridge,
        &registry,
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        ResourceHandle{},
        RESOURCE_TYPE_TEXTURE);
    if (result.status != WorldComponentResourceBindingStatus::InvalidResourceHandle) {
        return Fail("component resource binding invalid handle returned wrong status");
    }

    if (result.resource_status != ResourceStatus::InvalidHandle) {
        return Fail("component resource binding invalid handle returned wrong resource status");
    }

    if (bridge.Snapshot().active_binding_count != 0U) {
        return Fail("component resource binding invalid handle mutated binding count");
    }

    if (registry.Snapshot().acquired_handle_count != before_registry.acquired_handle_count) {
        return Fail("component resource binding invalid handle acquired resource");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeBindRejectsStaleResourceHandleWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding stale attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding stale registration failed");
    }

    if (registry.Retire(resource.handle) != ResourceStatus::Success) {
        return Fail("component resource binding stale retire failed");
    }

    WorldComponentResourceBindingBridge bridge;
    const WorldComponentResourceBindingResult result = BindComponentResource(
        bridge,
        &attachment_bridge,
        &registry,
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (result.status != WorldComponentResourceBindingStatus::StaleResourceHandle) {
        return Fail("component resource binding stale returned wrong status");
    }

    if (result.resource_status != ResourceStatus::GenerationMismatch) {
        return Fail("component resource binding stale returned wrong resource status");
    }

    if (bridge.Snapshot().active_binding_count != 0U) {
        return Fail("component resource binding stale mutated binding count");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeBindRejectsTypeMismatchWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding type mismatch attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_a");
    if (!resource.Succeeded()) {
        return Fail("component resource binding type mismatch registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    const WorldComponentResourceBindingResult result = BindComponentResource(
        bridge,
        &attachment_bridge,
        &registry,
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        resource.handle,
        RESOURCE_TYPE_MATERIAL);
    if (result.status != WorldComponentResourceBindingStatus::ResourceTypeMismatch) {
        return Fail("component resource binding type mismatch returned wrong status");
    }

    if (result.resource_status != ResourceStatus::TypeMismatch) {
        return Fail("component resource binding type mismatch returned wrong resource status");
    }

    if (registry.Snapshot().acquired_handle_count != 0U) {
        return Fail("component resource binding type mismatch acquired resource");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeBindRejectsDuplicateComponentBinding() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding duplicate attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult first_resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_a");
    const ResourceRegistrationResult second_resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_b");
    if (!first_resource.Succeeded() || !second_resource.Succeeded()) {
        return Fail("component resource binding duplicate registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    if (!BindComponentResource(
            bridge,
            &attachment_bridge,
            &registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            first_resource.handle,
            RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("component resource binding duplicate first bind failed");
    }

    const ResourceSnapshot before_registry = registry.Snapshot();
    const WorldComponentResourceBindingResult duplicate_result = BindComponentResource(
        bridge,
        &attachment_bridge,
        &registry,
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        second_resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (duplicate_result.status != WorldComponentResourceBindingStatus::DuplicateComponentBinding) {
        return Fail("component resource binding duplicate returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 1U) {
        return Fail("component resource binding duplicate mutated binding count");
    }

    if (registry.Snapshot().acquired_handle_count != before_registry.acquired_handle_count) {
        return Fail("component resource binding duplicate acquired resource");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeBindRejectsCapacityOverflowWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding capacity first attachment add failed") != 0) {
        return 1;
    }

    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_CAMERA,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_SECONDARY,
            "component resource binding capacity second attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult first_resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_a");
    const ResourceRegistrationResult second_resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_b");
    if (!first_resource.Succeeded() || !second_resource.Succeeded()) {
        return Fail("component resource binding capacity registration failed");
    }

    WorldComponentResourceBindingBridgeDesc desc{};
    desc.binding_capacity = 1U;
    WorldComponentResourceBindingBridge bridge(desc);
    if (!BindComponentResource(
            bridge,
            &attachment_bridge,
            &registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            first_resource.handle,
            RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("component resource binding capacity first bind failed");
    }

    const ResourceSnapshot before_registry = registry.Snapshot();
    const WorldComponentResourceBindingResult overflow_result = BindComponentResource(
        bridge,
        &attachment_bridge,
        &registry,
        OBJECT_CAMERA,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_SECONDARY,
        second_resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (overflow_result.status != WorldComponentResourceBindingStatus::CapacityExceeded) {
        return Fail("component resource binding capacity returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 1U) {
        return Fail("component resource binding capacity mutated binding count");
    }

    if (registry.Snapshot().acquired_handle_count != before_registry.acquired_handle_count) {
        return Fail("component resource binding capacity acquired resource");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeRemoveReleasesHandle() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding remove attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding remove registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    if (!BindComponentResource(
            bridge,
            &attachment_bridge,
            &registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            resource.handle,
            RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("component resource binding remove bind failed");
    }

    const WorldComponentResourceBindingStatus remove_status = bridge.Remove(
        &registry,
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY);
    if (remove_status != WorldComponentResourceBindingStatus::Success) {
        return Fail("component resource binding remove returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 0U) {
        return Fail("component resource binding remove did not clear binding");
    }

    if (registry.Snapshot().acquired_handle_count != 0U) {
        return Fail("component resource binding remove did not release resource");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeRemoveRejectsNullRegistryWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding remove null attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding remove null registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    if (!BindComponentResource(
            bridge,
            &attachment_bridge,
            &registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            resource.handle,
            RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("component resource binding remove null bind failed");
    }

    const WorldComponentResourceBindingStatus remove_status = bridge.Remove(
        nullptr,
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY);
    if (remove_status != WorldComponentResourceBindingStatus::InvalidResourceRegistry) {
        return Fail("component resource binding remove null returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 1U) {
        return Fail("component resource binding remove null cleared binding");
    }

    if (registry.Snapshot().acquired_handle_count != 1U) {
        return Fail("component resource binding remove null released resource");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeRemoveRejectsMissingBindingWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBindingBridge bridge;
    const ResourceSnapshot before_registry = registry.Snapshot();
    const WorldComponentResourceBindingStatus remove_status = bridge.Remove(
        &registry,
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY);
    if (remove_status != WorldComponentResourceBindingStatus::BindingNotFound) {
        return Fail("component resource binding remove missing returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 0U) {
        return Fail("component resource binding remove missing mutated binding count");
    }

    if (!ResourceSnapshotsMatch(before_registry, registry.Snapshot())) {
        return Fail("component resource binding remove missing mutated registry");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeRemoveReleaseFailureKeepsBinding() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding remove failure attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding remove failure registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    if (!BindComponentResource(
            bridge,
            &attachment_bridge,
            &registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            resource.handle,
            RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("component resource binding remove failure bind failed");
    }

    if (registry.Release(resource.handle) != ResourceStatus::Success) {
        return Fail("component resource binding remove failure external release failed");
    }

    const WorldComponentResourceBindingStatus remove_status = bridge.Remove(
        &registry,
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY);
    if (remove_status != WorldComponentResourceBindingStatus::ResourceReleaseFailed) {
        return Fail("component resource binding remove failure returned wrong status");
    }

    const WorldComponentResourceBindingSnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.active_binding_count != 1U) {
        return Fail("component resource binding remove failure cleared binding");
    }

    if (bridge_snapshot.last_resource_status != ResourceStatus::NotAcquired) {
        return Fail("component resource binding remove failure recorded wrong resource status");
    }

    if (!bridge.Query(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component resource binding remove failure lost query binding");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeClearReleasesAllHandles() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding clear first attachment add failed") != 0) {
        return 1;
    }

    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            "component resource binding clear second attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult first_resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_a");
    const ResourceRegistrationResult second_resource = RegisterResource(registry, RESOURCE_TYPE_AUDIO, "audio_a");
    if (!first_resource.Succeeded() || !second_resource.Succeeded()) {
        return Fail("component resource binding clear registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    if (!BindComponentResource(
            bridge,
            &attachment_bridge,
            &registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            first_resource.handle,
            RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("component resource binding clear first bind failed");
    }

    if (!BindComponentResource(
            bridge,
            &attachment_bridge,
            &registry,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            second_resource.handle,
            RESOURCE_TYPE_AUDIO).Succeeded()) {
        return Fail("component resource binding clear second bind failed");
    }

    const WorldComponentResourceBindingStatus clear_status = bridge.Clear(&registry);
    if (clear_status != WorldComponentResourceBindingStatus::Success) {
        return Fail("component resource binding clear returned wrong status");
    }

    const WorldComponentResourceBindingSnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.active_binding_count != 0U) {
        return Fail("component resource binding clear did not clear bindings");
    }

    if (bridge_snapshot.cleared_binding_count != 2U) {
        return Fail("component resource binding clear counted wrong cleared bindings");
    }

    if (registry.Snapshot().acquired_handle_count != 0U) {
        return Fail("component resource binding clear did not release resources");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeClearRejectsNullRegistryWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding clear null attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding clear null registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    if (!BindComponentResource(
            bridge,
            &attachment_bridge,
            &registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            resource.handle,
            RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("component resource binding clear null bind failed");
    }

    const WorldComponentResourceBindingStatus clear_status = bridge.Clear(nullptr);
    if (clear_status != WorldComponentResourceBindingStatus::InvalidResourceRegistry) {
        return Fail("component resource binding clear null returned wrong status");
    }

    if (bridge.Snapshot().active_binding_count != 1U) {
        return Fail("component resource binding clear null cleared binding");
    }

    if (registry.Snapshot().acquired_handle_count != 1U) {
        return Fail("component resource binding clear null released resource");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeClearReleaseFailurePreservesUnreleasedBindings() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding clear failure first attachment add failed") != 0) {
        return 1;
    }

    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            "component resource binding clear failure second attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult first_resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_a");
    const ResourceRegistrationResult second_resource = RegisterResource(registry, RESOURCE_TYPE_AUDIO, "audio_a");
    if (!first_resource.Succeeded() || !second_resource.Succeeded()) {
        return Fail("component resource binding clear failure registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    if (!BindComponentResource(
            bridge,
            &attachment_bridge,
            &registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            first_resource.handle,
            RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("component resource binding clear failure first bind failed");
    }

    if (!BindComponentResource(
            bridge,
            &attachment_bridge,
            &registry,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            second_resource.handle,
            RESOURCE_TYPE_AUDIO).Succeeded()) {
        return Fail("component resource binding clear failure second bind failed");
    }

    if (registry.Release(second_resource.handle) != ResourceStatus::Success) {
        return Fail("component resource binding clear failure external release failed");
    }

    const WorldComponentResourceBindingStatus clear_status = bridge.Clear(&registry);
    if (clear_status != WorldComponentResourceBindingStatus::ResourceReleaseFailed) {
        return Fail("component resource binding clear failure returned wrong status");
    }

    const WorldComponentResourceBindingSnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.active_binding_count != 1U) {
        return Fail("component resource binding clear failure did not preserve remaining binding");
    }

    if (bridge_snapshot.cleared_binding_count != 1U) {
        return Fail("component resource binding clear failure counted wrong cleared bindings");
    }

    if (!bridge.Query(OBJECT_CAMERA, COMPONENT_TYPE_SECONDARY, COMPONENT_SLOT_SECONDARY).Succeeded()) {
        return Fail("component resource binding clear failure lost unreleased binding");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeBoundResourceCannotRetireUntilReleased() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding retire attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding retire registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    if (!BindComponentResource(
            bridge,
            &attachment_bridge,
            &registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            resource.handle,
            RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("component resource binding retire bind failed");
    }

    if (registry.Retire(resource.handle) != ResourceStatus::StillReferenced) {
        return Fail("component resource binding retire while bound returned wrong status");
    }

    if (bridge.Remove(
            &registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY) != WorldComponentResourceBindingStatus::Success) {
        return Fail("component resource binding retire remove failed");
    }

    if (registry.Retire(resource.handle) != ResourceStatus::Success) {
        return Fail("component resource binding retire after remove failed");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeQueryReturnsStoredBinding() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding query attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry, RESOURCE_TYPE_MATERIAL, "material_a");
    if (!resource.Succeeded()) {
        return Fail("component resource binding query registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    if (!BindComponentResource(
            bridge,
            &attachment_bridge,
            &registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            resource.handle,
            RESOURCE_TYPE_MATERIAL).Succeeded()) {
        return Fail("component resource binding query bind failed");
    }

    const WorldComponentResourceBindingResult query_result = bridge.Query(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY);
    if (!query_result.Succeeded()) {
        return Fail("component resource binding query failed");
    }

    if (query_result.resource_handle.slot != resource.handle.slot) {
        return Fail("component resource binding query returned wrong resource slot");
    }

    if (query_result.expected_resource_type.value != RESOURCE_TYPE_MATERIAL.value) {
        return Fail("component resource binding query returned wrong resource type");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeQueryIsReadOnlyAndBounded() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding query readonly attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding query readonly registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    if (!BindComponentResource(
            bridge,
            &attachment_bridge,
            &registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            resource.handle,
            RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("component resource binding query readonly bind failed");
    }

    const WorldComponentResourceBindingSnapshot before_bridge = bridge.Snapshot();
    const ResourceSnapshot before_registry = registry.Snapshot();
    std::uint32_t query_index = 0U;
    while (query_index < 3U) {
        if (!bridge.Query(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
            return Fail("component resource binding query readonly query failed");
        }

        ++query_index;
    }

    const WorldComponentResourceBindingSnapshot after_bridge = bridge.Snapshot();
    if (after_bridge.active_binding_count != before_bridge.active_binding_count) {
        return Fail("component resource binding query readonly mutated active count");
    }

    if (after_bridge.acquired_binding_count != before_bridge.acquired_binding_count) {
        return Fail("component resource binding query readonly mutated acquired count");
    }

    if (after_bridge.released_binding_count != before_bridge.released_binding_count) {
        return Fail("component resource binding query readonly mutated release count");
    }

    if (after_bridge.failed_operation_count != before_bridge.failed_operation_count) {
        return Fail("component resource binding query readonly mutated failure count");
    }

    if (!ResourceSnapshotsMatch(before_registry, registry.Snapshot())) {
        return Fail("component resource binding query readonly mutated registry");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeUpdatePathDoesNotGrowStorage() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding update path attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBindingBridgeDesc desc{};
    desc.binding_capacity = 2U;
    WorldComponentResourceBindingBridge bridge(desc);
    const WorldComponentResourceBindingSnapshot before_bridge = bridge.Snapshot();
    std::uint32_t iteration = 0U;
    while (iteration < 2U) {
        const char *key = iteration == 0U ? "texture_a" : "texture_b";
        const ResourceRegistrationResult resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, key);
        if (!resource.Succeeded()) {
            return Fail("component resource binding update path registration failed");
        }

        if (!BindComponentResource(
                bridge,
                &attachment_bridge,
                &registry,
                OBJECT_PLAYER,
                COMPONENT_TYPE_PRIMARY,
                COMPONENT_SLOT_PRIMARY,
                resource.handle,
                RESOURCE_TYPE_TEXTURE).Succeeded()) {
            return Fail("component resource binding update path bind failed");
        }

        if (!bridge.Query(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
            return Fail("component resource binding update path query failed");
        }

        if (bridge.Remove(
                &registry,
                OBJECT_PLAYER,
                COMPONENT_TYPE_PRIMARY,
                COMPONENT_SLOT_PRIMARY) != WorldComponentResourceBindingStatus::Success) {
            return Fail("component resource binding update path remove failed");
        }

        ++iteration;
    }

    const WorldComponentResourceBindingSnapshot after_bridge = bridge.Snapshot();
    if (after_bridge.binding_capacity != before_bridge.binding_capacity) {
        return Fail("component resource binding update path changed capacity");
    }

    if (after_bridge.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("component resource binding update path changed allocation accounting");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeSnapshotReportsCountsAndLastStatus() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding snapshot attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding snapshot registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    if (!BindComponentResource(
            bridge,
            &attachment_bridge,
            &registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            resource.handle,
            RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("component resource binding snapshot bind failed");
    }

    const WorldComponentResourceBindingStatus failure_status = bridge.Remove(
        nullptr,
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY);
    if (failure_status != WorldComponentResourceBindingStatus::InvalidResourceRegistry) {
        return Fail("component resource binding snapshot failure status failed");
    }

    const WorldComponentResourceBindingSnapshot snapshot = bridge.Snapshot();
    if (snapshot.active_binding_count != 1U) {
        return Fail("component resource binding snapshot active count wrong");
    }

    if (snapshot.acquired_binding_count != 1U) {
        return Fail("component resource binding snapshot acquired count wrong");
    }

    if (snapshot.failed_operation_count != 1U) {
        return Fail("component resource binding snapshot failure count wrong");
    }

    if (snapshot.last_status != WorldComponentResourceBindingStatus::InvalidResourceRegistry) {
        return Fail("component resource binding snapshot last status wrong");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("component resource binding snapshot allocation accounting wrong");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeDoesNotQueryOrMutateWorldInstance() {
    WorldInstance world = MakeWorld(4U, 4U);
    const WorldSnapshot before_world = world.Snapshot();

    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding no-world-query attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding no-world-query registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    const WorldComponentResourceBindingResult result = BindComponentResource(
        bridge,
        &attachment_bridge,
        &registry,
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!result.Succeeded()) {
        return Fail("component resource binding no-world-query bind failed");
    }

    const WorldSnapshot after_world = world.Snapshot();
    if (!WorldSnapshotsMatch(before_world, after_world)) {
        return Fail("component resource binding no-world-query mutated world");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeNoActorComponentPayloadOrLifecycle() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding no-payload attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding no-payload registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    if (!BindComponentResource(
            bridge,
            &attachment_bridge,
            &registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            resource.handle,
            RESOURCE_TYPE_TEXTURE).Succeeded()) {
        return Fail("component resource binding no-payload bind failed");
    }

    const WorldComponentResourceBindingResult query_result = bridge.Query(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY);
    if (!query_result.Succeeded()) {
        return Fail("component resource binding no-payload query failed");
    }

    if (query_result.expected_resource_type.value != RESOURCE_TYPE_TEXTURE.value) {
        return Fail("component resource binding no-payload changed resource type");
    }

    if (bridge.Snapshot().active_binding_count != 1U) {
        return Fail("component resource binding no-payload changed active count");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeNoFilePackageLoadDecodeUploadOrGameAdapterDependency() {
    WorldComponentResourceBindingBridge bridge;
    const WorldComponentResourceBindingSnapshot snapshot = bridge.Snapshot();
    if (snapshot.binding_capacity != MAX_WORLD_OBJECT_COUNT) {
        return Fail("component resource binding dependency test changed default capacity");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("component resource binding dependency test changed allocation accounting");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeNoRenderPhysicsAudioInputUiToolOrReportDependency() {
    WorldComponentResourceBindingBridge bridge;
    const WorldComponentResourceBindingSnapshot snapshot = bridge.Snapshot();
    if (snapshot.binding_capacity != MAX_WORLD_OBJECT_COUNT) {
        return Fail("component resource binding render dependency test changed default capacity");
    }

    if (snapshot.last_status != WorldComponentResourceBindingStatus::Success) {
        return Fail("component resource binding render dependency test changed last status");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeWorldInstanceCoreRemainsComponentResourceFree() {
    WorldInstance world = MakeWorld(4U, 4U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("component resource binding world core-free registration failed");
    }

    const WorldSnapshot before_world = world.Snapshot();
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_EFFECT,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource binding world core-free attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding world core-free resource registration failed");
    }

    WorldComponentResourceBindingBridge bridge;
    const WorldComponentResourceBindingResult result = BindComponentResource(
        bridge,
        &attachment_bridge,
        &registry,
        OBJECT_EFFECT,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    if (!result.Succeeded()) {
        return Fail("component resource binding world core-free bind failed");
    }

    const WorldSnapshot after_world = world.Snapshot();
    if (!WorldSnapshotsMatch(before_world, after_world)) {
        return Fail("component resource binding world core-free mutated world");
    }

    return 0;
}

int WorldComponentResourceBindingBridgeResourceCoreRemainsWorldFree() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding resource core-free registration failed");
    }

    if (registry.Acquire(resource.handle, RESOURCE_TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("component resource binding resource core-free acquire failed");
    }

    if (registry.Release(resource.handle) != ResourceStatus::Success) {
        return Fail("component resource binding resource core-free release failed");
    }

    if (registry.Retire(resource.handle) != ResourceStatus::Success) {
        return Fail("component resource binding resource core-free retire failed");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeWriteReadRoundTripsBindingsInSlotOrder() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBindingBridge source_bridge;
    ResourceHandle first_handle{};
    ResourceHandle second_handle{};
    ResourceHandle third_handle{};
    if (AddComponentResourceBindingFixture(
            attachment_bridge,
            registry,
            source_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_snapshot_a",
            &first_handle) != 0) {
        return 1;
    }

    if (AddComponentResourceBindingFixture(
            attachment_bridge,
            registry,
            source_bridge,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            RESOURCE_TYPE_MATERIAL,
            "material_snapshot_a",
            &second_handle) != 0) {
        return 1;
    }

    if (AddComponentResourceBindingFixture(
            attachment_bridge,
            registry,
            source_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_TERTIARY,
            COMPONENT_SLOT_TERTIARY,
            RESOURCE_TYPE_AUDIO,
            "audio_snapshot_a",
            &third_handle) != 0) {
        return 1;
    }

    WorldComponentResourceBindingSnapshotBridge snapshot_bridge;
    SerializeBuffer buffer{};
    std::uint32_t committed_byte_count = 0U;
    if (WriteComponentResourceBindingSnapshotToBuffer(
            snapshot_bridge,
            source_bridge,
            buffer,
            committed_byte_count) != 0) {
        return 1;
    }

    std::array<WorldComponentResourceBinding, 3U> output_bindings{};
    std::uint32_t binding_count = 0U;
    SerializeReader reader(buffer.data(), committed_byte_count);
    const WorldComponentResourceBindingSnapshotResult read_result = snapshot_bridge.ReadSnapshot(
        &reader,
        output_bindings.data(),
        static_cast<std::uint32_t>(output_bindings.size()),
        &binding_count);
    if (!read_result.Succeeded()) {
        return Fail("component resource binding snapshot round trip read failed");
    }

    if (binding_count != 3U) {
        return Fail("component resource binding snapshot round trip count wrong");
    }

    if (output_bindings[0].world_object_id.value != OBJECT_PLAYER.value) {
        return Fail("component resource binding snapshot round trip first object wrong");
    }

    if (output_bindings[0].resource_handle.slot != first_handle.slot) {
        return Fail("component resource binding snapshot round trip first resource wrong");
    }

    if (output_bindings[1].world_object_id.value != OBJECT_CAMERA.value) {
        return Fail("component resource binding snapshot round trip second object wrong");
    }

    if (output_bindings[1].resource_handle.slot != second_handle.slot) {
        return Fail("component resource binding snapshot round trip second resource wrong");
    }

    if (output_bindings[2].component_type_id.value != COMPONENT_TYPE_TERTIARY.value) {
        return Fail("component resource binding snapshot round trip third type wrong");
    }

    if (output_bindings[2].resource_handle.slot != third_handle.slot) {
        return Fail("component resource binding snapshot round trip third resource wrong");
    }

    if (output_bindings[0].is_acquired || output_bindings[1].is_acquired || output_bindings[2].is_acquired) {
        return Fail("component resource binding snapshot round trip acquired output");
    }

    if (read_result.state.binding_record_count != 3U) {
        return Fail("component resource binding snapshot round trip state count wrong");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeWriteEmptyBridgeProducesZeroRecords() {
    WorldComponentResourceBindingBridge source_bridge;
    WorldComponentResourceBindingSnapshotBridge bridge;
    SerializeBuffer buffer{};
    std::uint32_t committed_byte_count = 0U;
    if (WriteComponentResourceBindingSnapshotToBuffer(
            bridge,
            source_bridge,
            buffer,
            committed_byte_count) != 0) {
        return 1;
    }

    const WorldComponentResourceBindingSnapshotBridgeSnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.written_record_count != 0U) {
        return Fail("component resource binding snapshot empty write record count wrong");
    }

    SerializeReader reader(buffer.data(), committed_byte_count);
    if (OpenSerializeStream(reader) != 0) {
        return 1;
    }

    std::uint32_t record_count = 999U;
    const SerializeStatus status = reader.ReadUInt32(
        WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_METADATA_RECORD_ID,
        WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_FIELD_RECORD_COUNT,
        record_count);
    if (status != SerializeStatus::Success) {
        return Fail("component resource binding snapshot empty metadata read failed");
    }

    if (record_count != 0U) {
        return Fail("component resource binding snapshot empty write metadata count wrong");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeWriteRejectsNullSourceWithoutMutation() {
    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    const SerializeSnapshot before_writer = writer.Snapshot();
    WorldComponentResourceBindingSnapshotBridge bridge;
    const WorldComponentResourceBindingSnapshotResult result = bridge.WriteSnapshot(&writer, nullptr);
    if (result.status != WorldComponentResourceBindingSnapshotStatus::InvalidSourceBridge) {
        return Fail("component resource binding snapshot null source status wrong");
    }

    const SerializeSnapshot after_writer = writer.Snapshot();
    if (!SerializeSnapshotsMatch(before_writer, after_writer)) {
        return Fail("component resource binding snapshot null source mutated writer");
    }

    if (bridge.Snapshot().failed_operation_count != 1U) {
        return Fail("component resource binding snapshot null source failure count wrong");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeWriteRejectsNullWriterWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBindingBridge source_bridge;
    if (AddComponentResourceBindingFixture(
            attachment_bridge,
            registry,
            source_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_snapshot_null_writer",
            nullptr) != 0) {
        return 1;
    }

    const WorldComponentResourceBindingSnapshot before_source = source_bridge.Snapshot();
    WorldComponentResourceBindingSnapshotBridge bridge;
    const WorldComponentResourceBindingSnapshotResult result = bridge.WriteSnapshot(nullptr, &source_bridge);
    if (result.status != WorldComponentResourceBindingSnapshotStatus::InvalidWriter) {
        return Fail("component resource binding snapshot null writer status wrong");
    }

    const WorldComponentResourceBindingSnapshot after_source = source_bridge.Snapshot();
    if (!ComponentResourceBindingSnapshotsMatch(before_source, after_source)) {
        return Fail("component resource binding snapshot null writer mutated source");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeWriteRejectsWriterOverflowWithoutOverrun() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBindingBridge source_bridge;
    if (AddComponentResourceBindingFixture(
            attachment_bridge,
            registry,
            source_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_snapshot_overflow",
            nullptr) != 0) {
        return 1;
    }

    std::array<std::uint8_t, STREAM_HEADER_BYTE_COUNT + 1U> buffer{};
    buffer[STREAM_HEADER_BYTE_COUNT] = 0xABU;
    SerializeWriter writer(buffer.data(), STREAM_HEADER_BYTE_COUNT);
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    WorldComponentResourceBindingSnapshotBridge bridge;
    const WorldComponentResourceBindingSnapshotResult result = bridge.WriteSnapshot(&writer, &source_bridge);
    if (result.status != WorldComponentResourceBindingSnapshotStatus::SerializeFailure) {
        return Fail("component resource binding snapshot writer overflow status wrong");
    }

    if (result.serialize_status != SerializeStatus::BufferTooSmall) {
        return Fail("component resource binding snapshot writer overflow serialize status wrong");
    }

    if (buffer[STREAM_HEADER_BYTE_COUNT] != 0xABU) {
        return Fail("component resource binding snapshot writer overflow overran buffer");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeReadWritesCallerOwnedRecords() {
    ResourceHandle handle{};
    handle.slot = 1U;
    handle.generation = 1U;
    const WorldComponentResourceBindingSnapshotRecord record = MakeComponentResourceBindingSnapshotRecord(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        handle,
        RESOURCE_TYPE_TEXTURE);
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> records{record};

    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    if (WriteComponentResourceBindingSnapshotFixtureStream(writer, records.data(), 1U) != 0) {
        return 1;
    }

    const WorldComponentResourceBinding sentinel_binding = SentinelComponentResourceBinding();
    std::array<WorldComponentResourceBinding, 2U> output_bindings{sentinel_binding, sentinel_binding};
    std::uint32_t binding_count = 77U;
    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    WorldComponentResourceBindingSnapshotBridge bridge;
    const WorldComponentResourceBindingSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        output_bindings.data(),
        static_cast<std::uint32_t>(output_bindings.size()),
        &binding_count);
    if (!result.Succeeded()) {
        return Fail("component resource binding snapshot read output failed");
    }

    if (binding_count != 1U) {
        return Fail("component resource binding snapshot read output count wrong");
    }

    if (!ComponentResourceBindingMatchesSnapshotRecord(output_bindings[0], record)) {
        return Fail("component resource binding snapshot read output record wrong");
    }

    if (!ComponentResourceBindingsMatch(output_bindings[1], sentinel_binding)) {
        return Fail("component resource binding snapshot read output overran output");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeReadRejectsNullReaderWithoutMutation() {
    const WorldComponentResourceBinding sentinel_binding = SentinelComponentResourceBinding();
    std::array<WorldComponentResourceBinding, 1U> output_bindings{sentinel_binding};
    std::uint32_t binding_count = 77U;
    WorldComponentResourceBindingSnapshotBridge bridge;
    const WorldComponentResourceBindingSnapshotResult result = bridge.ReadSnapshot(
        nullptr,
        output_bindings.data(),
        static_cast<std::uint32_t>(output_bindings.size()),
        &binding_count);
    if (result.status != WorldComponentResourceBindingSnapshotStatus::InvalidReader) {
        return Fail("component resource binding snapshot null reader status wrong");
    }

    if (binding_count != 77U) {
        return Fail("component resource binding snapshot null reader mutated count");
    }

    if (!ComponentResourceBindingsMatch(output_bindings[0], sentinel_binding)) {
        return Fail("component resource binding snapshot null reader mutated output");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeReadRejectsNullOutputWithoutMutation() {
    ResourceHandle handle{};
    handle.slot = 1U;
    handle.generation = 1U;
    const WorldComponentResourceBindingSnapshotRecord record = MakeComponentResourceBindingSnapshotRecord(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        handle,
        RESOURCE_TYPE_TEXTURE);
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> records{record};

    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    if (WriteComponentResourceBindingSnapshotFixtureStream(writer, records.data(), 1U) != 0) {
        return 1;
    }

    std::uint32_t binding_count = 77U;
    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    WorldComponentResourceBindingSnapshotBridge bridge;
    const WorldComponentResourceBindingSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        nullptr,
        1U,
        &binding_count);
    if (result.status != WorldComponentResourceBindingSnapshotStatus::InvalidOutput) {
        return Fail("component resource binding snapshot null output status wrong");
    }

    if (binding_count != 77U) {
        return Fail("component resource binding snapshot null output mutated count");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeReadRejectsOutputCapacityTooSmallWithoutMutation() {
    ResourceHandle first_handle{};
    first_handle.slot = 1U;
    first_handle.generation = 1U;
    ResourceHandle second_handle{};
    second_handle.slot = 2U;
    second_handle.generation = 1U;
    const WorldComponentResourceBindingSnapshotRecord first_record = MakeComponentResourceBindingSnapshotRecord(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        first_handle,
        RESOURCE_TYPE_TEXTURE);
    const WorldComponentResourceBindingSnapshotRecord second_record = MakeComponentResourceBindingSnapshotRecord(
        OBJECT_CAMERA,
        COMPONENT_TYPE_SECONDARY,
        COMPONENT_SLOT_SECONDARY,
        second_handle,
        RESOURCE_TYPE_MATERIAL);
    std::array<WorldComponentResourceBindingSnapshotRecord, 2U> records{first_record, second_record};

    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    if (WriteComponentResourceBindingSnapshotFixtureStream(writer, records.data(), 2U) != 0) {
        return 1;
    }

    const WorldComponentResourceBinding sentinel_binding = SentinelComponentResourceBinding();
    std::array<WorldComponentResourceBinding, 1U> output_bindings{sentinel_binding};
    std::uint32_t binding_count = 77U;
    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    WorldComponentResourceBindingSnapshotBridge bridge;
    const WorldComponentResourceBindingSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        output_bindings.data(),
        static_cast<std::uint32_t>(output_bindings.size()),
        &binding_count);
    if (result.status != WorldComponentResourceBindingSnapshotStatus::OutputCapacityExceeded) {
        return Fail("component resource binding snapshot small output status wrong");
    }

    if (binding_count != 77U) {
        return Fail("component resource binding snapshot small output mutated count");
    }

    if (!ComponentResourceBindingsMatch(output_bindings[0], sentinel_binding)) {
        return Fail("component resource binding snapshot small output mutated output");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeReadRejectsUnknownVersionWithoutMutation() {
    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    if (WriteComponentResourceBindingSnapshotMetadata(writer, 999U, 0U, 0U) != 0) {
        return 1;
    }

    const WorldComponentResourceBinding sentinel_binding = SentinelComponentResourceBinding();
    std::array<WorldComponentResourceBinding, 1U> output_bindings{sentinel_binding};
    std::uint32_t binding_count = 77U;
    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    WorldComponentResourceBindingSnapshotBridge bridge;
    const WorldComponentResourceBindingSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        output_bindings.data(),
        static_cast<std::uint32_t>(output_bindings.size()),
        &binding_count);
    if (result.status != WorldComponentResourceBindingSnapshotStatus::UnsupportedVersion) {
        return Fail("component resource binding snapshot unknown version status wrong");
    }

    if (binding_count != 77U) {
        return Fail("component resource binding snapshot unknown version mutated count");
    }

    if (!ComponentResourceBindingsMatch(output_bindings[0], sentinel_binding)) {
        return Fail("component resource binding snapshot unknown version mutated output");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeReadRejectsMalformedRecordCountWithoutMutation() {
    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    if (WriteComponentResourceBindingSnapshotMetadata(
            writer,
            WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_SCHEMA_VERSION,
            1U,
            0U) != 0) {
        return 1;
    }

    const WorldComponentResourceBinding sentinel_binding = SentinelComponentResourceBinding();
    std::array<WorldComponentResourceBinding, 1U> output_bindings{sentinel_binding};
    std::uint32_t binding_count = 77U;
    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    WorldComponentResourceBindingSnapshotBridge bridge;
    const WorldComponentResourceBindingSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        output_bindings.data(),
        static_cast<std::uint32_t>(output_bindings.size()),
        &binding_count);
    if (result.status != WorldComponentResourceBindingSnapshotStatus::MalformedRecordCount) {
        return Fail("component resource binding snapshot malformed count status wrong");
    }

    if (binding_count != 77U) {
        return Fail("component resource binding snapshot malformed count mutated count");
    }

    if (!ComponentResourceBindingsMatch(output_bindings[0], sentinel_binding)) {
        return Fail("component resource binding snapshot malformed count mutated output");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeReadRejectsInvalidWorldIdWithoutMutation() {
    ResourceHandle handle{};
    handle.slot = 1U;
    handle.generation = 1U;
    const WorldComponentResourceBindingSnapshotRecord record = MakeComponentResourceBindingSnapshotRecord(
        WorldObjectId{},
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        handle,
        RESOURCE_TYPE_TEXTURE);
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> records{record};
    return ReadRejectedComponentResourceBindingSnapshotRecords(
        records.data(),
        1U,
        WorldComponentResourceBindingSnapshotStatus::InvalidWorldObjectId,
        "component resource binding snapshot invalid world status wrong");
}

int WorldComponentResourceBindingSnapshotBridgeReadRejectsInvalidComponentTypeWithoutMutation() {
    ResourceHandle handle{};
    handle.slot = 1U;
    handle.generation = 1U;
    const WorldComponentResourceBindingSnapshotRecord record = MakeComponentResourceBindingSnapshotRecord(
        OBJECT_PLAYER,
        WorldComponentTypeId{},
        COMPONENT_SLOT_PRIMARY,
        handle,
        RESOURCE_TYPE_TEXTURE);
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> records{record};
    return ReadRejectedComponentResourceBindingSnapshotRecords(
        records.data(),
        1U,
        WorldComponentResourceBindingSnapshotStatus::InvalidComponentTypeId,
        "component resource binding snapshot invalid type status wrong");
}

int WorldComponentResourceBindingSnapshotBridgeReadRejectsInvalidComponentSlotWithoutMutation() {
    ResourceHandle handle{};
    handle.slot = 1U;
    handle.generation = 1U;
    const WorldComponentResourceBindingSnapshotRecord record = MakeComponentResourceBindingSnapshotRecord(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        WorldComponentSlotId{},
        handle,
        RESOURCE_TYPE_TEXTURE);
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> records{record};
    return ReadRejectedComponentResourceBindingSnapshotRecords(
        records.data(),
        1U,
        WorldComponentResourceBindingSnapshotStatus::InvalidComponentSlotId,
        "component resource binding snapshot invalid slot status wrong");
}

int WorldComponentResourceBindingSnapshotBridgeReadRejectsInvalidResourceHandleWithoutMutation() {
    const WorldComponentResourceBindingSnapshotRecord record = MakeComponentResourceBindingSnapshotRecord(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        ResourceHandle{},
        RESOURCE_TYPE_TEXTURE);
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> records{record};
    return ReadRejectedComponentResourceBindingSnapshotRecords(
        records.data(),
        1U,
        WorldComponentResourceBindingSnapshotStatus::InvalidResourceHandle,
        "component resource binding snapshot invalid handle status wrong");
}

int WorldComponentResourceBindingSnapshotBridgeReadRejectsInvalidResourceTypeWithoutMutation() {
    ResourceHandle handle{};
    handle.slot = 1U;
    handle.generation = 1U;
    const WorldComponentResourceBindingSnapshotRecord record = MakeComponentResourceBindingSnapshotRecord(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        handle,
        ResourceTypeId{});
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> records{record};
    return ReadRejectedComponentResourceBindingSnapshotRecords(
        records.data(),
        1U,
        WorldComponentResourceBindingSnapshotStatus::InvalidResourceTypeId,
        "component resource binding snapshot invalid resource type status wrong");
}

int WorldComponentResourceBindingSnapshotBridgeReadRejectsDuplicateBindingWithoutMutation() {
    ResourceHandle first_handle{};
    first_handle.slot = 1U;
    first_handle.generation = 1U;
    ResourceHandle second_handle{};
    second_handle.slot = 2U;
    second_handle.generation = 1U;
    const WorldComponentResourceBindingSnapshotRecord first_record = MakeComponentResourceBindingSnapshotRecord(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        first_handle,
        RESOURCE_TYPE_TEXTURE);
    const WorldComponentResourceBindingSnapshotRecord second_record = MakeComponentResourceBindingSnapshotRecord(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        second_handle,
        RESOURCE_TYPE_MATERIAL);
    std::array<WorldComponentResourceBindingSnapshotRecord, 2U> records{first_record, second_record};
    return ReadRejectedComponentResourceBindingSnapshotRecords(
        records.data(),
        2U,
        WorldComponentResourceBindingSnapshotStatus::DuplicateBinding,
        "component resource binding snapshot duplicate status wrong");
}

int WorldComponentResourceBindingSnapshotBridgeReadDoesNotAcquireOrReleaseResources() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding snapshot no acquire registration failed");
    }

    const WorldComponentResourceBindingSnapshotRecord record = MakeComponentResourceBindingSnapshotRecord(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> records{record};
    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    if (WriteComponentResourceBindingSnapshotFixtureStream(writer, records.data(), 1U) != 0) {
        return 1;
    }

    const ResourceSnapshot before_registry = registry.Snapshot();
    std::array<WorldComponentResourceBinding, 1U> output_bindings{};
    std::uint32_t binding_count = 0U;
    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    WorldComponentResourceBindingSnapshotBridge bridge;
    const WorldComponentResourceBindingSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        output_bindings.data(),
        static_cast<std::uint32_t>(output_bindings.size()),
        &binding_count);
    if (!result.Succeeded()) {
        return Fail("component resource binding snapshot no acquire read failed");
    }

    if (!ResourceSnapshotsMatch(before_registry, registry.Snapshot())) {
        return Fail("component resource binding snapshot read mutated registry");
    }

    if (output_bindings[0].is_acquired) {
        return Fail("component resource binding snapshot read marked output acquired");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeWriteReadPathDoesNotGrowStorage() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBindingBridge source_bridge;
    if (AddComponentResourceBindingFixture(
            attachment_bridge,
            registry,
            source_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_snapshot_path_a",
            nullptr) != 0) {
        return 1;
    }

    if (AddComponentResourceBindingFixture(
            attachment_bridge,
            registry,
            source_bridge,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            RESOURCE_TYPE_MATERIAL,
            "material_snapshot_path_a",
            nullptr) != 0) {
        return 1;
    }

    WorldComponentResourceBindingSnapshotBridgeDesc desc{};
    desc.binding_capacity = 2U;
    WorldComponentResourceBindingSnapshotBridge bridge(desc);
    const WorldComponentResourceBindingSnapshotBridgeSnapshot before_snapshot = bridge.Snapshot();
    std::uint32_t iteration = 0U;
    while (iteration < 3U) {
        SerializeBuffer buffer{};
        std::uint32_t committed_byte_count = 0U;
        if (WriteComponentResourceBindingSnapshotToBuffer(
                bridge,
                source_bridge,
                buffer,
                committed_byte_count) != 0) {
            return 1;
        }

        std::array<WorldComponentResourceBinding, 2U> output_bindings{};
        std::uint32_t binding_count = 0U;
        SerializeReader reader(buffer.data(), committed_byte_count);
        const WorldComponentResourceBindingSnapshotResult read_result = bridge.ReadSnapshot(
            &reader,
            output_bindings.data(),
            static_cast<std::uint32_t>(output_bindings.size()),
            &binding_count);
        if (!read_result.Succeeded()) {
            return Fail("component resource binding snapshot path read failed");
        }

        ++iteration;
    }

    const WorldComponentResourceBindingSnapshotBridgeSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.binding_capacity != before_snapshot.binding_capacity) {
        return Fail("component resource binding snapshot path changed capacity");
    }

    if (after_snapshot.allocation_accounting_status != before_snapshot.allocation_accounting_status) {
        return Fail("component resource binding snapshot path changed allocation accounting");
    }

    if (after_snapshot.write_count != 3U) {
        return Fail("component resource binding snapshot path write count wrong");
    }

    if (after_snapshot.read_count != 3U) {
        return Fail("component resource binding snapshot path read count wrong");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeSnapshotReportsCountsAndLastStatus() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBindingBridge source_bridge;
    if (AddComponentResourceBindingFixture(
            attachment_bridge,
            registry,
            source_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_snapshot_count_a",
            nullptr) != 0) {
        return 1;
    }

    if (AddComponentResourceBindingFixture(
            attachment_bridge,
            registry,
            source_bridge,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            RESOURCE_TYPE_MATERIAL,
            "material_snapshot_count_a",
            nullptr) != 0) {
        return 1;
    }

    WorldComponentResourceBindingSnapshotBridge bridge;
    SerializeBuffer buffer{};
    std::uint32_t committed_byte_count = 0U;
    if (WriteComponentResourceBindingSnapshotToBuffer(
            bridge,
            source_bridge,
            buffer,
            committed_byte_count) != 0) {
        return 1;
    }

    std::array<WorldComponentResourceBinding, 2U> output_bindings{};
    std::uint32_t binding_count = 0U;
    SerializeReader reader(buffer.data(), committed_byte_count);
    const WorldComponentResourceBindingSnapshotResult read_result = bridge.ReadSnapshot(
        &reader,
        output_bindings.data(),
        static_cast<std::uint32_t>(output_bindings.size()),
        &binding_count);
    if (!read_result.Succeeded()) {
        return Fail("component resource binding snapshot counters read failed");
    }

    const WorldComponentResourceBindingSnapshotResult failure_result = bridge.ReadSnapshot(
        nullptr,
        output_bindings.data(),
        static_cast<std::uint32_t>(output_bindings.size()),
        &binding_count);
    if (failure_result.status != WorldComponentResourceBindingSnapshotStatus::InvalidReader) {
        return Fail("component resource binding snapshot counters failure status wrong");
    }

    const WorldComponentResourceBindingSnapshotBridgeSnapshot snapshot = bridge.Snapshot();
    if (snapshot.write_count != 1U) {
        return Fail("component resource binding snapshot counters write count wrong");
    }

    if (snapshot.read_count != 1U) {
        return Fail("component resource binding snapshot counters read count wrong");
    }

    if (snapshot.written_record_count != 2U) {
        return Fail("component resource binding snapshot counters written count wrong");
    }

    if (snapshot.read_record_count != 2U) {
        return Fail("component resource binding snapshot counters read record count wrong");
    }

    if (snapshot.failed_operation_count != 1U) {
        return Fail("component resource binding snapshot counters failure count wrong");
    }

    if (snapshot.last_status != WorldComponentResourceBindingSnapshotStatus::InvalidReader) {
        return Fail("component resource binding snapshot counters last status wrong");
    }

    if (snapshot.last_serialize_status != SerializeStatus::Success) {
        return Fail("component resource binding snapshot counters serialize status wrong");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeNoActorComponentPayloadOrLifecycle() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBindingBridge source_bridge;
    ResourceHandle handle{};
    if (AddComponentResourceBindingFixture(
            attachment_bridge,
            registry,
            source_bridge,
            OBJECT_EFFECT,
            COMPONENT_TYPE_TERTIARY,
            COMPONENT_SLOT_TERTIARY,
            RESOURCE_TYPE_AUDIO,
            "audio_snapshot_payload_a",
            &handle) != 0) {
        return 1;
    }

    WorldComponentResourceBindingSnapshotBridge bridge;
    SerializeBuffer buffer{};
    std::uint32_t committed_byte_count = 0U;
    if (WriteComponentResourceBindingSnapshotToBuffer(
            bridge,
            source_bridge,
            buffer,
            committed_byte_count) != 0) {
        return 1;
    }

    std::array<WorldComponentResourceBinding, 1U> output_bindings{};
    std::uint32_t binding_count = 0U;
    SerializeReader reader(buffer.data(), committed_byte_count);
    const WorldComponentResourceBindingSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        output_bindings.data(),
        static_cast<std::uint32_t>(output_bindings.size()),
        &binding_count);
    if (!result.Succeeded()) {
        return Fail("component resource binding snapshot payload boundary read failed");
    }

    if (output_bindings[0].component_slot_id.value != COMPONENT_SLOT_TERTIARY.value) {
        return Fail("component resource binding snapshot payload boundary slot wrong");
    }

    if (output_bindings[0].resource_handle.slot != handle.slot) {
        return Fail("component resource binding snapshot payload boundary resource wrong");
    }

    if (output_bindings[0].is_acquired) {
        return Fail("component resource binding snapshot payload boundary acquired output");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeNoFilePackageLoadDecodeUploadOrGameAdapterDependency() {
    WorldComponentResourceBindingSnapshotBridge bridge;
    const WorldComponentResourceBindingSnapshotBridgeSnapshot snapshot = bridge.Snapshot();
    if (snapshot.binding_capacity != MAX_WORLD_OBJECT_COUNT) {
        return Fail("component resource binding snapshot dependency test changed default capacity");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("component resource binding snapshot dependency test changed allocation accounting");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeNoRenderPhysicsAudioInputUiToolOrReportDependency() {
    WorldComponentResourceBindingSnapshotBridge bridge;
    const WorldComponentResourceBindingSnapshotBridgeSnapshot snapshot = bridge.Snapshot();
    if (snapshot.binding_capacity != MAX_WORLD_OBJECT_COUNT) {
        return Fail("component resource binding snapshot render dependency test changed default capacity");
    }

    if (snapshot.last_status != WorldComponentResourceBindingSnapshotStatus::Success) {
        return Fail("component resource binding snapshot render dependency test changed last status");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeWorldInstanceCoreRemainsSnapshotFree() {
    WorldInstance world = MakeWorld(4U, 4U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("component resource binding snapshot world core-free registration failed");
    }

    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBindingBridge source_bridge;
    if (AddComponentResourceBindingFixture(
            attachment_bridge,
            registry,
            source_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_snapshot_world_core_a",
            nullptr) != 0) {
        return 1;
    }

    const WorldSnapshot before_world = world.Snapshot();
    WorldComponentResourceBindingSnapshotBridge bridge;
    SerializeBuffer buffer{};
    std::uint32_t committed_byte_count = 0U;
    if (WriteComponentResourceBindingSnapshotToBuffer(
            bridge,
            source_bridge,
            buffer,
            committed_byte_count) != 0) {
        return 1;
    }

    std::array<WorldComponentResourceBinding, 1U> output_bindings{};
    std::uint32_t binding_count = 0U;
    SerializeReader reader(buffer.data(), committed_byte_count);
    if (!bridge.ReadSnapshot(
            &reader,
            output_bindings.data(),
            static_cast<std::uint32_t>(output_bindings.size()),
            &binding_count).Succeeded()) {
        return Fail("component resource binding snapshot world core-free read failed");
    }

    const WorldSnapshot after_world = world.Snapshot();
    if (!WorldSnapshotsMatch(before_world, after_world)) {
        return Fail("component resource binding snapshot bridge mutated world");
    }

    return 0;
}

int WorldComponentResourceBindingSnapshotBridgeResourceCoreRemainsWorldFree() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry);
    if (!resource.Succeeded()) {
        return Fail("component resource binding snapshot resource core-free registration failed");
    }

    const WorldComponentResourceBindingSnapshotRecord record = MakeComponentResourceBindingSnapshotRecord(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> records{record};
    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    if (WriteComponentResourceBindingSnapshotFixtureStream(writer, records.data(), 1U) != 0) {
        return 1;
    }

    const ResourceSnapshot before_registry = registry.Snapshot();
    std::array<WorldComponentResourceBinding, 1U> output_bindings{};
    std::uint32_t binding_count = 0U;
    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    WorldComponentResourceBindingSnapshotBridge bridge;
    const WorldComponentResourceBindingSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        output_bindings.data(),
        static_cast<std::uint32_t>(output_bindings.size()),
        &binding_count);
    if (!result.Succeeded()) {
        return Fail("component resource binding snapshot resource core-free read failed");
    }

    if (!ResourceSnapshotsMatch(before_registry, registry.Snapshot())) {
        return Fail("component resource binding snapshot resource core-free mutated registry");
    }

    if (registry.Acquire(resource.handle, RESOURCE_TYPE_TEXTURE) != ResourceStatus::Success) {
        return Fail("component resource binding snapshot resource core-free acquire failed");
    }

    if (registry.Release(resource.handle) != ResourceStatus::Success) {
        return Fail("component resource binding snapshot resource core-free release failed");
    }

    if (registry.Retire(resource.handle) != ResourceStatus::Success) {
        return Fail("component resource binding snapshot resource core-free retire failed");
    }

    return 0;
}

int WorldComponentResourceBindingRestoreBridgeRestoresRecordsInInputOrder() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentResourceBinding, 2U> input_bindings{};
    if (AddComponentResourceRestoreInput(
            attachment_bridge,
            registry,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            RESOURCE_TYPE_MATERIAL,
            "material_restore_order_a",
            &input_bindings[0]) != 0) {
        return 1;
    }

    if (AddComponentResourceRestoreInput(
            attachment_bridge,
            registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_restore_order_a",
            &input_bindings[1]) != 0) {
        return 1;
    }

    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    const std::uint32_t input_count = static_cast<std::uint32_t>(input_bindings.size());
    const WorldComponentResourceBindingRestoreResult result = restore_bridge.Restore(
        &destination_bridge,
        &attachment_bridge,
        &registry,
        input_bindings.data(),
        input_count);
    if (!result.Succeeded()) {
        return Fail("component resource restore order restore failed");
    }

    if (result.state.restored_binding_count != input_count) {
        return Fail("component resource restore order restored wrong count");
    }

    std::array<WorldComponentResourceBinding, 2U> restored_bindings{};
    const std::uint32_t restored_count = destination_bridge.ExportBindings(
        restored_bindings.data(),
        input_count);
    if (restored_count != input_count) {
        return Fail("component resource restore order exported wrong count");
    }

    if (!ComponentResourceRestoredBindingMatchesInput(restored_bindings[0], input_bindings[0])) {
        return Fail("component resource restore order changed first record");
    }

    if (!ComponentResourceRestoredBindingMatchesInput(restored_bindings[1], input_bindings[1])) {
        return Fail("component resource restore order changed second record");
    }

    if (registry.Snapshot().acquired_handle_count != input_count) {
        return Fail("component resource restore order acquired wrong resource count");
    }

    return 0;
}

int WorldComponentResourceBindingRestoreBridgeRestoresEmptyInputWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    std::array<WorldComponentResourceBinding, 1U> input_bindings{};
    const WorldComponentResourceBindingSnapshot before_destination = destination_bridge.Snapshot();
    const ResourceSnapshot before_registry = registry.Snapshot();
    const WorldComponentResourceBindingRestoreResult result = restore_bridge.Restore(
        &destination_bridge,
        &attachment_bridge,
        &registry,
        input_bindings.data(),
        0U);
    if (!result.Succeeded()) {
        return Fail("component resource restore empty returned failure");
    }

    if (result.state.input_binding_count != 0U) {
        return Fail("component resource restore empty recorded input count");
    }

    if (result.state.restored_binding_count != 0U) {
        return Fail("component resource restore empty restored binding");
    }

    if (!ComponentResourceBindingSnapshotsMatch(before_destination, destination_bridge.Snapshot())) {
        return Fail("component resource restore empty mutated destination");
    }

    if (!ResourceSnapshotsMatch(before_registry, registry.Snapshot())) {
        return Fail("component resource restore empty mutated registry");
    }

    return 0;
}

int WorldComponentResourceBindingRestoreBridgeRejectsNullDestinationWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBinding input_binding{};
    if (AddComponentResourceRestoreInput(
            attachment_bridge,
            registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_restore_null_destination",
            &input_binding) != 0) {
        return 1;
    }

    WorldComponentResourceBindingRestoreBridge restore_bridge;
    return ExpectComponentResourceRestoreFailureWithoutMutation(
        restore_bridge,
        nullptr,
        &attachment_bridge,
        &registry,
        &input_binding,
        1U,
        WorldComponentResourceBindingRestoreStatus::InvalidDestinationBridge,
        "component resource restore null destination returned wrong status");
}

int WorldComponentResourceBindingRestoreBridgeRejectsNullAttachmentSourceWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_restore_null_attachment");
    if (!resource.Succeeded()) {
        return Fail("component resource restore null attachment registration failed");
    }

    WorldComponentResourceBinding input_binding = MakeComponentResourceBinding(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    return ExpectComponentResourceRestoreFailureWithoutMutation(
        restore_bridge,
        &destination_bridge,
        nullptr,
        &registry,
        &input_binding,
        1U,
        WorldComponentResourceBindingRestoreStatus::InvalidAttachmentSource,
        "component resource restore null attachment returned wrong status");
}

int WorldComponentResourceBindingRestoreBridgeRejectsNullRegistryWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource restore null registry attachment add failed") != 0) {
        return 1;
    }

    const ResourceHandle resource_handle{1U, 1U};
    WorldComponentResourceBinding input_binding = MakeComponentResourceBinding(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        resource_handle,
        RESOURCE_TYPE_TEXTURE);
    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    return ExpectComponentResourceRestoreFailureWithoutMutation(
        restore_bridge,
        &destination_bridge,
        &attachment_bridge,
        nullptr,
        &input_binding,
        1U,
        WorldComponentResourceBindingRestoreStatus::InvalidResourceRegistry,
        "component resource restore null registry returned wrong status");
}

int WorldComponentResourceBindingRestoreBridgeRejectsNullInputWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    return ExpectComponentResourceRestoreFailureWithoutMutation(
        restore_bridge,
        &destination_bridge,
        &attachment_bridge,
        &registry,
        nullptr,
        1U,
        WorldComponentResourceBindingRestoreStatus::InvalidInput,
        "component resource restore null input returned wrong status");
}

int WorldComponentResourceBindingRestoreBridgeRejectsInvalidWorldIdWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_restore_invalid_world");
    if (!resource.Succeeded()) {
        return Fail("component resource restore invalid world registration failed");
    }

    WorldComponentResourceBinding input_binding = MakeComponentResourceBinding(
        WorldObjectId{},
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    return ExpectComponentResourceRestoreFailureWithoutMutation(
        restore_bridge,
        &destination_bridge,
        &attachment_bridge,
        &registry,
        &input_binding,
        1U,
        WorldComponentResourceBindingRestoreStatus::InvalidWorldObjectId,
        "component resource restore invalid world returned wrong status");
}

int WorldComponentResourceBindingRestoreBridgeRejectsInvalidComponentTypeWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_restore_invalid_type");
    if (!resource.Succeeded()) {
        return Fail("component resource restore invalid type registration failed");
    }

    WorldComponentResourceBinding input_binding = MakeComponentResourceBinding(
        OBJECT_PLAYER,
        WorldComponentTypeId{},
        COMPONENT_SLOT_PRIMARY,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    return ExpectComponentResourceRestoreFailureWithoutMutation(
        restore_bridge,
        &destination_bridge,
        &attachment_bridge,
        &registry,
        &input_binding,
        1U,
        WorldComponentResourceBindingRestoreStatus::InvalidComponentTypeId,
        "component resource restore invalid type returned wrong status");
}

int WorldComponentResourceBindingRestoreBridgeRejectsInvalidComponentSlotWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_restore_invalid_slot");
    if (!resource.Succeeded()) {
        return Fail("component resource restore invalid slot registration failed");
    }

    WorldComponentResourceBinding input_binding = MakeComponentResourceBinding(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        WorldComponentSlotId{},
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    return ExpectComponentResourceRestoreFailureWithoutMutation(
        restore_bridge,
        &destination_bridge,
        &attachment_bridge,
        &registry,
        &input_binding,
        1U,
        WorldComponentResourceBindingRestoreStatus::InvalidComponentSlotId,
        "component resource restore invalid slot returned wrong status");
}

int WorldComponentResourceBindingRestoreBridgeRejectsMissingAttachmentWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_restore_missing_attachment");
    if (!resource.Succeeded()) {
        return Fail("component resource restore missing attachment registration failed");
    }

    WorldComponentResourceBinding input_binding = MakeComponentResourceBinding(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    return ExpectComponentResourceRestoreFailureWithoutMutation(
        restore_bridge,
        &destination_bridge,
        &attachment_bridge,
        &registry,
        &input_binding,
        1U,
        WorldComponentResourceBindingRestoreStatus::MissingAttachment,
        "component resource restore missing attachment returned wrong status");
}

int WorldComponentResourceBindingRestoreBridgeRejectsInvalidResourceHandleWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource restore invalid handle attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBinding input_binding = MakeComponentResourceBinding(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        ResourceHandle{},
        RESOURCE_TYPE_TEXTURE);
    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    return ExpectComponentResourceRestoreFailureWithoutMutation(
        restore_bridge,
        &destination_bridge,
        &attachment_bridge,
        &registry,
        &input_binding,
        1U,
        WorldComponentResourceBindingRestoreStatus::InvalidResourceHandle,
        "component resource restore invalid handle returned wrong status");
}

int WorldComponentResourceBindingRestoreBridgeRejectsStaleResourceHandleWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource restore stale attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_restore_stale");
    if (!resource.Succeeded()) {
        return Fail("component resource restore stale registration failed");
    }

    if (registry.Retire(resource.handle) != ResourceStatus::Success) {
        return Fail("component resource restore stale retire failed");
    }

    WorldComponentResourceBinding input_binding = MakeComponentResourceBinding(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        resource.handle,
        RESOURCE_TYPE_TEXTURE);
    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    return ExpectComponentResourceRestoreFailureWithoutMutation(
        restore_bridge,
        &destination_bridge,
        &attachment_bridge,
        &registry,
        &input_binding,
        1U,
        WorldComponentResourceBindingRestoreStatus::StaleResourceHandle,
        "component resource restore stale handle returned wrong status");
}

int WorldComponentResourceBindingRestoreBridgeRejectsResourceTypeMismatchWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource restore mismatch attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(registry, RESOURCE_TYPE_TEXTURE, "texture_restore_mismatch");
    if (!resource.Succeeded()) {
        return Fail("component resource restore mismatch registration failed");
    }

    WorldComponentResourceBinding input_binding = MakeComponentResourceBinding(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        resource.handle,
        RESOURCE_TYPE_MATERIAL);
    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    return ExpectComponentResourceRestoreFailureWithoutMutation(
        restore_bridge,
        &destination_bridge,
        &attachment_bridge,
        &registry,
        &input_binding,
        1U,
        WorldComponentResourceBindingRestoreStatus::ResourceTypeMismatch,
        "component resource restore type mismatch returned wrong status");
}

int WorldComponentResourceBindingRestoreBridgeRejectsDuplicateInputWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentResourceBinding, 2U> input_bindings{};
    if (AddComponentResourceRestoreInput(
            attachment_bridge,
            registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_restore_duplicate_a",
            &input_bindings[0]) != 0) {
        return 1;
    }

    const ResourceRegistrationResult second_resource = RegisterResource(
        registry,
        RESOURCE_TYPE_TEXTURE,
        "texture_restore_duplicate_b");
    if (!second_resource.Succeeded()) {
        return Fail("component resource restore duplicate second registration failed");
    }

    input_bindings[1] = MakeComponentResourceBinding(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY,
        second_resource.handle,
        RESOURCE_TYPE_TEXTURE);
    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    const std::uint32_t input_count = static_cast<std::uint32_t>(input_bindings.size());
    return ExpectComponentResourceRestoreFailureWithoutMutation(
        restore_bridge,
        &destination_bridge,
        &attachment_bridge,
        &registry,
        input_bindings.data(),
        input_count,
        WorldComponentResourceBindingRestoreStatus::DuplicateInputBinding,
        "component resource restore duplicate returned wrong status");
}

int WorldComponentResourceBindingRestoreBridgeRejectsDestinationCapacityOverflowWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentResourceBinding, 2U> input_bindings{};
    if (AddComponentResourceRestoreInput(
            attachment_bridge,
            registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_restore_capacity_a",
            &input_bindings[0]) != 0) {
        return 1;
    }

    if (AddComponentResourceRestoreInput(
            attachment_bridge,
            registry,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            RESOURCE_TYPE_AUDIO,
            "audio_restore_capacity_a",
            &input_bindings[1]) != 0) {
        return 1;
    }

    WorldComponentResourceBindingBridgeDesc destination_desc{};
    destination_desc.binding_capacity = 1U;
    WorldComponentResourceBindingBridge destination_bridge(destination_desc);
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    const std::uint32_t input_count = static_cast<std::uint32_t>(input_bindings.size());
    return ExpectComponentResourceRestoreFailureWithoutMutation(
        restore_bridge,
        &destination_bridge,
        &attachment_bridge,
        &registry,
        input_bindings.data(),
        input_count,
        WorldComponentResourceBindingRestoreStatus::DestinationCapacityExceeded,
        "component resource restore destination capacity returned wrong status");
}

int WorldComponentResourceBindingRestoreBridgeRejectsNonEmptyDestinationWithoutMutation() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBinding existing_input{};
    if (AddComponentResourceRestoreInput(
            attachment_bridge,
            registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_restore_non_empty_existing",
            &existing_input) != 0) {
        return 1;
    }

    if (!BindComponentResource(
            destination_bridge,
            &attachment_bridge,
            &registry,
            existing_input.world_object_id,
            existing_input.component_type_id,
            existing_input.component_slot_id,
            existing_input.resource_handle,
            existing_input.expected_resource_type).Succeeded()) {
        return Fail("component resource restore non-empty fixture bind failed");
    }

    WorldComponentResourceBinding input_binding{};
    if (AddComponentResourceRestoreInput(
            attachment_bridge,
            registry,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            RESOURCE_TYPE_AUDIO,
            "audio_restore_non_empty_new",
            &input_binding) != 0) {
        return 1;
    }

    WorldComponentResourceBindingRestoreBridge restore_bridge;
    return ExpectComponentResourceRestoreFailureWithoutMutation(
        restore_bridge,
        &destination_bridge,
        &attachment_bridge,
        &registry,
        &input_binding,
        1U,
        WorldComponentResourceBindingRestoreStatus::DestinationNotEmpty,
        "component resource restore non-empty destination returned wrong status");
}

int WorldComponentResourceBindingRestoreBridgeAcquiresOnlyAfterPreflight() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentResourceBinding, 2U> input_bindings{};
    if (AddComponentResourceRestoreInput(
            attachment_bridge,
            registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_restore_preflight_a",
            &input_bindings[0]) != 0) {
        return 1;
    }

    const ResourceRegistrationResult second_resource = RegisterResource(
        registry,
        RESOURCE_TYPE_MATERIAL,
        "material_restore_preflight_a");
    if (!second_resource.Succeeded()) {
        return Fail("component resource restore preflight second registration failed");
    }

    input_bindings[1] = MakeComponentResourceBinding(
        OBJECT_CAMERA,
        COMPONENT_TYPE_SECONDARY,
        COMPONENT_SLOT_SECONDARY,
        second_resource.handle,
        RESOURCE_TYPE_MATERIAL);
    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    const std::uint32_t input_count = static_cast<std::uint32_t>(input_bindings.size());
    return ExpectComponentResourceRestoreFailureWithoutMutation(
        restore_bridge,
        &destination_bridge,
        &attachment_bridge,
        &registry,
        input_bindings.data(),
        input_count,
        WorldComponentResourceBindingRestoreStatus::MissingAttachment,
        "component resource restore preflight returned wrong status");
}

int WorldComponentResourceBindingRestoreBridgeResourceAcquireFailureDoesNotPartiallyRestore() {
    WorldComponentAttachmentBridge attachment_bridge;
    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component resource restore acquire first attachment add failed") != 0) {
        return 1;
    }

    if (AddComponentAttachment(
            attachment_bridge,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            "component resource restore acquire second attachment add failed") != 0) {
        return 1;
    }

    ResourceRegistry registry = MakeResourceRegistry();
    const std::uint32_t max_reference_count = std::numeric_limits<std::uint32_t>::max();
    const std::uint32_t initial_reference_count = max_reference_count - 1U;
    const ResourceRegistrationResult resource = RegisterResource(
        registry,
        RESOURCE_TYPE_TEXTURE,
        "texture_restore_acquire_overflow",
        initial_reference_count);
    if (!resource.Succeeded()) {
        return Fail("component resource restore acquire overflow registration failed");
    }

    std::array<WorldComponentResourceBinding, 2U> input_bindings{
        MakeComponentResourceBinding(
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            resource.handle,
            RESOURCE_TYPE_TEXTURE),
        MakeComponentResourceBinding(
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            resource.handle,
            RESOURCE_TYPE_TEXTURE)};
    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    const std::uint32_t input_count = static_cast<std::uint32_t>(input_bindings.size());
    return ExpectComponentResourceRestoreFailureWithoutMutation(
        restore_bridge,
        &destination_bridge,
        &attachment_bridge,
        &registry,
        input_bindings.data(),
        input_count,
        WorldComponentResourceBindingRestoreStatus::ResourceAcquireWouldOverflow,
        "component resource restore acquire failure returned wrong status");
}

int WorldComponentResourceBindingRestoreBridgeSnapshotReportsCountsAndLastStatus() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentResourceBinding, 2U> input_bindings{};
    if (AddComponentResourceRestoreInput(
            attachment_bridge,
            registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_restore_counter_a",
            &input_bindings[0]) != 0) {
        return 1;
    }

    if (AddComponentResourceRestoreInput(
            attachment_bridge,
            registry,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            RESOURCE_TYPE_AUDIO,
            "audio_restore_counter_a",
            &input_bindings[1]) != 0) {
        return 1;
    }

    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    const std::uint32_t input_count = static_cast<std::uint32_t>(input_bindings.size());
    if (!restore_bridge.Restore(
            &destination_bridge,
            &attachment_bridge,
            &registry,
            input_bindings.data(),
            input_count).Succeeded()) {
        return Fail("component resource restore counters success failed");
    }

    const WorldComponentResourceBindingRestoreResult failure_result = restore_bridge.Restore(
        &destination_bridge,
        &attachment_bridge,
        &registry,
        nullptr,
        1U);
    if (failure_result.status != WorldComponentResourceBindingRestoreStatus::InvalidInput) {
        return Fail("component resource restore counters failure status wrong");
    }

    const WorldComponentResourceBindingRestoreSnapshot snapshot = restore_bridge.Snapshot();
    if (snapshot.restore_attempt_count != 2U) {
        return Fail("component resource restore counters attempt count wrong");
    }

    if (snapshot.restored_binding_count != input_count) {
        return Fail("component resource restore counters restored count wrong");
    }

    if (snapshot.failed_operation_count != 1U) {
        return Fail("component resource restore counters failure count wrong");
    }

    if (snapshot.last_status != WorldComponentResourceBindingRestoreStatus::InvalidInput) {
        return Fail("component resource restore counters last status wrong");
    }

    if (snapshot.last_binding_status != WorldComponentResourceBindingStatus::Success) {
        return Fail("component resource restore counters binding status wrong");
    }

    if (snapshot.last_resource_status != ResourceStatus::Success) {
        return Fail("component resource restore counters resource status wrong");
    }

    return 0;
}

int WorldComponentResourceBindingRestoreBridgeNoActorComponentPayloadOrLifecycle() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBinding input_binding{};
    if (AddComponentResourceRestoreInput(
            attachment_bridge,
            registry,
            OBJECT_EFFECT,
            COMPONENT_TYPE_TERTIARY,
            COMPONENT_SLOT_TERTIARY,
            RESOURCE_TYPE_AUDIO,
            "audio_restore_payload_a",
            &input_binding) != 0) {
        return 1;
    }

    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    const WorldComponentResourceBindingRestoreResult result = restore_bridge.Restore(
        &destination_bridge,
        &attachment_bridge,
        &registry,
        &input_binding,
        1U);
    if (!result.Succeeded()) {
        return Fail("component resource restore payload boundary restore failed");
    }

    const WorldComponentResourceBindingResult query_result = destination_bridge.Query(
        OBJECT_EFFECT,
        COMPONENT_TYPE_TERTIARY,
        COMPONENT_SLOT_TERTIARY);
    if (!query_result.Succeeded()) {
        return Fail("component resource restore payload boundary query failed");
    }

    if (query_result.resource_handle.slot != input_binding.resource_handle.slot) {
        return Fail("component resource restore payload boundary resource wrong");
    }

    return 0;
}

int WorldComponentResourceBindingRestoreBridgeNoFilePackageLoadDecodeUploadOrGameAdapterDependency() {
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    const WorldComponentResourceBindingRestoreSnapshot snapshot = restore_bridge.Snapshot();
    if (snapshot.binding_capacity != MAX_WORLD_OBJECT_COUNT) {
        return Fail("component resource restore dependency test changed default capacity");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("component resource restore dependency test changed allocation accounting");
    }

    return 0;
}

int WorldComponentResourceBindingRestoreBridgeNoRenderPhysicsAudioInputUiToolOrReportDependency() {
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    const WorldComponentResourceBindingRestoreSnapshot snapshot = restore_bridge.Snapshot();
    if (snapshot.binding_capacity != MAX_WORLD_OBJECT_COUNT) {
        return Fail("component resource restore render dependency test changed default capacity");
    }

    if (snapshot.last_status != WorldComponentResourceBindingRestoreStatus::Success) {
        return Fail("component resource restore render dependency test changed last status");
    }

    return 0;
}

int WorldComponentResourceBindingRestoreBridgeWorldInstanceCoreRemainsRestoreFree() {
    WorldInstance world = MakeWorld(4U, 4U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("component resource restore world core-free registration failed");
    }

    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBinding input_binding{};
    if (AddComponentResourceRestoreInput(
            attachment_bridge,
            registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_restore_world_core_a",
            &input_binding) != 0) {
        return 1;
    }

    const WorldSnapshot before_world = world.Snapshot();
    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    const WorldComponentResourceBindingRestoreResult result = restore_bridge.Restore(
        &destination_bridge,
        &attachment_bridge,
        &registry,
        &input_binding,
        1U);
    if (!result.Succeeded()) {
        return Fail("component resource restore world core-free restore failed");
    }

    const WorldSnapshot after_world = world.Snapshot();
    if (!WorldSnapshotsMatch(before_world, after_world)) {
        return Fail("component resource restore bridge mutated world");
    }

    return 0;
}

int WorldComponentResourceBindingRestoreBridgeResourceCoreRemainsWorldFree() {
    WorldComponentAttachmentBridge attachment_bridge;
    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentResourceBinding input_binding{};
    if (AddComponentResourceRestoreInput(
            attachment_bridge,
            registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_restore_resource_core_a",
            &input_binding) != 0) {
        return 1;
    }

    WorldComponentResourceBindingBridge destination_bridge;
    WorldComponentResourceBindingRestoreBridge restore_bridge;
    const WorldComponentResourceBindingRestoreResult result = restore_bridge.Restore(
        &destination_bridge,
        &attachment_bridge,
        &registry,
        &input_binding,
        1U);
    if (!result.Succeeded()) {
        return Fail("component resource restore resource core-free restore failed");
    }

    if (destination_bridge.Clear(&registry) != WorldComponentResourceBindingStatus::Success) {
        return Fail("component resource restore resource core-free clear failed");
    }

    if (registry.Retire(input_binding.resource_handle) != ResourceStatus::Success) {
        return Fail("component resource restore resource core-free retire failed");
    }

    return 0;
}

int WorldSceneAssemblyBridgeRestoresAttachmentAndBindingRecordsInInputOrder() {
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentAttachmentSnapshotRecord, 2U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY),
        MakeSceneAttachmentRecord(OBJECT_CAMERA, COMPONENT_TYPE_SECONDARY, COMPONENT_SLOT_SECONDARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 2U> input_bindings{};
    if (RegisterSceneBindingInput(
            registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_scene_order_a",
            &input_bindings[0]) != 0) {
        return 1;
    }

    if (RegisterSceneBindingInput(
            registry,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            RESOURCE_TYPE_AUDIO,
            "audio_scene_order_a",
            &input_bindings[1]) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    const std::uint32_t attachment_count = static_cast<std::uint32_t>(input_attachments.size());
    const std::uint32_t binding_count = static_cast<std::uint32_t>(input_bindings.size());
    const WorldSceneAssemblyResult result = assembly_bridge.Restore(
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        attachment_count,
        input_bindings.data(),
        binding_count);
    if (!result.Succeeded()) {
        return Fail("scene assembly order restore failed");
    }

    std::array<WorldComponentAttachment, 2U> restored_attachments{};
    const std::uint32_t restored_attachment_count = attachment_destination.ExportAttachments(
        restored_attachments.data(),
        static_cast<std::uint32_t>(restored_attachments.size()));
    if (restored_attachment_count != attachment_count) {
        return Fail("scene assembly order attachment count wrong");
    }

    if (!SceneAttachmentMatches(restored_attachments[0], input_attachments[0])) {
        return Fail("scene assembly order first attachment wrong");
    }

    if (!SceneAttachmentMatches(restored_attachments[1], input_attachments[1])) {
        return Fail("scene assembly order second attachment wrong");
    }

    std::array<WorldComponentResourceBinding, 2U> restored_bindings{};
    const std::uint32_t restored_binding_count = binding_destination.ExportBindings(
        restored_bindings.data(),
        static_cast<std::uint32_t>(restored_bindings.size()));
    if (restored_binding_count != binding_count) {
        return Fail("scene assembly order binding count wrong");
    }

    if (!SceneBindingMatches(restored_bindings[0], input_bindings[0])) {
        return Fail("scene assembly order first binding wrong");
    }

    if (!SceneBindingMatches(restored_bindings[1], input_bindings[1])) {
        return Fail("scene assembly order second binding wrong");
    }

    return 0;
}

int WorldSceneAssemblyBridgeRestoresEmptyAssemblyWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> input_attachments{};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{};
    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    const WorldComponentAttachmentSnapshot before_attachment = attachment_destination.Snapshot();
    const WorldComponentResourceBindingSnapshot before_binding = binding_destination.Snapshot();
    const ResourceSnapshot before_registry = registry.Snapshot();
    WorldSceneAssemblyBridge assembly_bridge;
    const WorldSceneAssemblyResult result = assembly_bridge.Restore(
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        0U,
        input_bindings.data(),
        0U);
    if (!result.Succeeded()) {
        return Fail("scene assembly empty restore failed");
    }

    if (!ComponentAttachmentSnapshotsMatch(before_attachment, attachment_destination.Snapshot())) {
        return Fail("scene assembly empty mutated attachment destination");
    }

    if (!ComponentResourceBindingSnapshotsMatch(before_binding, binding_destination.Snapshot())) {
        return Fail("scene assembly empty mutated binding destination");
    }

    if (!ResourceSnapshotsMatch(before_registry, registry.Snapshot())) {
        return Fail("scene assembly empty mutated registry");
    }

    return 0;
}

int WorldSceneAssemblyBridgeRejectsNullAttachmentDestinationWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{};
    WorldComponentResourceBindingBridge binding_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    return ExpectSceneAssemblyFailureWithoutMutation(
        assembly_bridge,
        nullptr,
        &binding_destination,
        &registry,
        input_attachments.data(),
        1U,
        input_bindings.data(),
        0U,
        WorldSceneAssemblyStatus::InvalidAttachmentDestination,
        "scene assembly null attachment destination status wrong");
}

int WorldSceneAssemblyBridgeRejectsNullBindingDestinationWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{};
    WorldComponentAttachmentBridge attachment_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    return ExpectSceneAssemblyFailureWithoutMutation(
        assembly_bridge,
        &attachment_destination,
        nullptr,
        &registry,
        input_attachments.data(),
        1U,
        input_bindings.data(),
        0U,
        WorldSceneAssemblyStatus::InvalidBindingDestination,
        "scene assembly null binding destination status wrong");
}

int WorldSceneAssemblyBridgeRejectsNullRegistryWithoutMutation() {
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{};
    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    return ExpectSceneAssemblyFailureWithoutMutation(
        assembly_bridge,
        &attachment_destination,
        &binding_destination,
        nullptr,
        input_attachments.data(),
        1U,
        input_bindings.data(),
        0U,
        WorldSceneAssemblyStatus::InvalidResourceRegistry,
        "scene assembly null registry status wrong");
}

int WorldSceneAssemblyBridgeRejectsNullAttachmentInputWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{};
    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    return ExpectSceneAssemblyFailureWithoutMutation(
        assembly_bridge,
        &attachment_destination,
        &binding_destination,
        &registry,
        nullptr,
        1U,
        input_bindings.data(),
        0U,
        WorldSceneAssemblyStatus::InvalidAttachmentInput,
        "scene assembly null attachment input status wrong");
}

int WorldSceneAssemblyBridgeRejectsNullBindingInputWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY)};
    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    return ExpectSceneAssemblyFailureWithoutMutation(
        assembly_bridge,
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        1U,
        nullptr,
        1U,
        WorldSceneAssemblyStatus::InvalidBindingInput,
        "scene assembly null binding input status wrong");
}

int WorldSceneAssemblyBridgeRejectsInvalidAttachmentRecordWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> input_attachments{
        MakeSceneAttachmentRecord(WorldObjectId{}, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{};
    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    return ExpectSceneAssemblyFailureWithoutMutation(
        assembly_bridge,
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        1U,
        input_bindings.data(),
        0U,
        WorldSceneAssemblyStatus::InvalidWorldObjectId,
        "scene assembly invalid attachment status wrong");
}

int WorldSceneAssemblyBridgeRejectsInvalidBindingRecordWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(
        registry,
        RESOURCE_TYPE_TEXTURE,
        "texture_scene_invalid_binding");
    if (!resource.Succeeded()) {
        return Fail("scene assembly invalid binding registration failed");
    }

    std::array<WorldComponentAttachmentSnapshotRecord, 1U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{
        MakeSceneBindingRecord(
            OBJECT_PLAYER,
            WorldComponentTypeId{},
            COMPONENT_SLOT_PRIMARY,
            resource.handle,
            RESOURCE_TYPE_TEXTURE)};
    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    return ExpectSceneAssemblyFailureWithoutMutation(
        assembly_bridge,
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        1U,
        input_bindings.data(),
        1U,
        WorldSceneAssemblyStatus::InvalidComponentTypeId,
        "scene assembly invalid binding status wrong");
}

int WorldSceneAssemblyBridgeRejectsMissingAttachmentForBindingWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{};
    if (RegisterSceneBindingInput(
            registry,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            RESOURCE_TYPE_AUDIO,
            "audio_scene_missing_attachment",
            &input_bindings[0]) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    return ExpectSceneAssemblyFailureWithoutMutation(
        assembly_bridge,
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        1U,
        input_bindings.data(),
        1U,
        WorldSceneAssemblyStatus::MissingAttachment,
        "scene assembly missing attachment status wrong");
}

int WorldSceneAssemblyBridgeRejectsDuplicateAttachmentInputWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentAttachmentSnapshotRecord, 2U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY),
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_SECONDARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{};
    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    return ExpectSceneAssemblyFailureWithoutMutation(
        assembly_bridge,
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        static_cast<std::uint32_t>(input_attachments.size()),
        input_bindings.data(),
        0U,
        WorldSceneAssemblyStatus::DuplicateAttachmentInput,
        "scene assembly duplicate attachment status wrong");
}

int WorldSceneAssemblyBridgeRejectsDuplicateBindingInputWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 2U> input_bindings{};
    if (RegisterSceneBindingInput(
            registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_scene_duplicate_binding_a",
            &input_bindings[0]) != 0) {
        return 1;
    }

    if (RegisterSceneBindingInput(
            registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_scene_duplicate_binding_b",
            &input_bindings[1]) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    return ExpectSceneAssemblyFailureWithoutMutation(
        assembly_bridge,
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        1U,
        input_bindings.data(),
        static_cast<std::uint32_t>(input_bindings.size()),
        WorldSceneAssemblyStatus::DuplicateBindingInput,
        "scene assembly duplicate binding status wrong");
}

int WorldSceneAssemblyBridgeRejectsAttachmentCapacityOverflowWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentAttachmentSnapshotRecord, 2U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY),
        MakeSceneAttachmentRecord(OBJECT_CAMERA, COMPONENT_TYPE_SECONDARY, COMPONENT_SLOT_SECONDARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{};
    WorldSceneAssemblyBridgeDesc desc{};
    desc.attachment_capacity = 1U;
    WorldSceneAssemblyBridge assembly_bridge(desc);
    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    return ExpectSceneAssemblyFailureWithoutMutation(
        assembly_bridge,
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        static_cast<std::uint32_t>(input_attachments.size()),
        input_bindings.data(),
        0U,
        WorldSceneAssemblyStatus::AttachmentCapacityExceeded,
        "scene assembly attachment capacity status wrong");
}

int WorldSceneAssemblyBridgeRejectsBindingCapacityOverflowWithoutMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentAttachmentSnapshotRecord, 2U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY),
        MakeSceneAttachmentRecord(OBJECT_CAMERA, COMPONENT_TYPE_SECONDARY, COMPONENT_SLOT_SECONDARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 2U> input_bindings{};
    if (RegisterSceneBindingInput(
            registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_scene_binding_capacity",
            &input_bindings[0]) != 0) {
        return 1;
    }

    if (RegisterSceneBindingInput(
            registry,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            RESOURCE_TYPE_AUDIO,
            "audio_scene_binding_capacity",
            &input_bindings[1]) != 0) {
        return 1;
    }

    WorldSceneAssemblyBridgeDesc desc{};
    desc.binding_capacity = 1U;
    WorldSceneAssemblyBridge assembly_bridge(desc);
    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    return ExpectSceneAssemblyFailureWithoutMutation(
        assembly_bridge,
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        static_cast<std::uint32_t>(input_attachments.size()),
        input_bindings.data(),
        static_cast<std::uint32_t>(input_bindings.size()),
        WorldSceneAssemblyStatus::BindingCapacityExceeded,
        "scene assembly binding capacity status wrong");
}

int WorldSceneAssemblyBridgeRejectsNonEmptyDestinationsWithoutMutation() {
    {
        ResourceRegistry registry = MakeResourceRegistry();
        std::array<WorldComponentAttachmentSnapshotRecord, 1U> input_attachments{};
        std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{};
        WorldComponentAttachmentBridge attachment_destination;
        if (AddComponentAttachment(
                attachment_destination,
                OBJECT_PLAYER,
                COMPONENT_TYPE_PRIMARY,
                COMPONENT_SLOT_PRIMARY,
                "scene assembly non-empty attachment add failed") != 0) {
            return 1;
        }

        WorldComponentResourceBindingBridge binding_destination;
        WorldSceneAssemblyBridge assembly_bridge;
        if (ExpectSceneAssemblyFailureWithoutMutation(
                assembly_bridge,
                &attachment_destination,
                &binding_destination,
                &registry,
                input_attachments.data(),
                0U,
                input_bindings.data(),
                0U,
                WorldSceneAssemblyStatus::DestinationNotEmpty,
                "scene assembly non-empty attachment destination status wrong") != 0) {
            return 1;
        }
    }

    ResourceRegistry registry = MakeResourceRegistry();
    WorldComponentAttachmentBridge attachment_source;
    WorldComponentResourceBindingSnapshotRecord existing_binding{};
    if (RegisterSceneBindingInput(
            registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_scene_non_empty_binding",
            &existing_binding) != 0) {
        return 1;
    }

    if (AddComponentAttachment(
            attachment_source,
            existing_binding.world_object_id,
            existing_binding.component_type_id,
            existing_binding.component_slot_id,
            "scene assembly non-empty binding attachment add failed") != 0) {
        return 1;
    }

    WorldComponentResourceBindingBridge binding_destination;
    if (!BindComponentResource(
            binding_destination,
            &attachment_source,
            &registry,
            existing_binding.world_object_id,
            existing_binding.component_type_id,
            existing_binding.component_slot_id,
            existing_binding.resource_handle,
            existing_binding.expected_resource_type).Succeeded()) {
        return Fail("scene assembly non-empty binding fixture bind failed");
    }

    std::array<WorldComponentAttachmentSnapshotRecord, 1U> input_attachments{};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{};
    WorldComponentAttachmentBridge attachment_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    return ExpectSceneAssemblyFailureWithoutMutation(
        assembly_bridge,
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        0U,
        input_bindings.data(),
        0U,
        WorldSceneAssemblyStatus::DestinationNotEmpty,
        "scene assembly non-empty binding destination status wrong");
}

int WorldSceneAssemblyBridgeValidatesResourceHandlesBeforeMutation() {
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{
        MakeSceneBindingRecord(
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            ResourceHandle{},
            RESOURCE_TYPE_TEXTURE)};
    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    return ExpectSceneAssemblyFailureWithoutMutation(
        assembly_bridge,
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        1U,
        input_bindings.data(),
        1U,
        WorldSceneAssemblyStatus::InvalidResourceHandle,
        "scene assembly invalid resource handle status wrong");
}

int WorldSceneAssemblyBridgeBindingPreflightFailureDoesNotRestoreAttachments() {
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{};
    if (RegisterSceneBindingInput(
            registry,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            RESOURCE_TYPE_AUDIO,
            "audio_scene_binding_preflight",
            &input_bindings[0]) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    return ExpectSceneAssemblyFailureWithoutMutation(
        assembly_bridge,
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        1U,
        input_bindings.data(),
        1U,
        WorldSceneAssemblyStatus::MissingAttachment,
        "scene assembly binding preflight status wrong");
}

int WorldSceneAssemblyBridgeResourceAcquireFailureDoesNotPartiallyAssemble() {
    ResourceRegistry registry = MakeResourceRegistry();
    const std::uint32_t max_reference_count = std::numeric_limits<std::uint32_t>::max();
    const std::uint32_t initial_reference_count = max_reference_count - 1U;
    const ResourceRegistrationResult resource = RegisterResource(
        registry,
        RESOURCE_TYPE_TEXTURE,
        "texture_scene_acquire_overflow",
        initial_reference_count);
    if (!resource.Succeeded()) {
        return Fail("scene assembly acquire overflow registration failed");
    }

    std::array<WorldComponentAttachmentSnapshotRecord, 2U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY),
        MakeSceneAttachmentRecord(OBJECT_CAMERA, COMPONENT_TYPE_SECONDARY, COMPONENT_SLOT_SECONDARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 2U> input_bindings{
        MakeSceneBindingRecord(
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            resource.handle,
            RESOURCE_TYPE_TEXTURE),
        MakeSceneBindingRecord(
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            resource.handle,
            RESOURCE_TYPE_TEXTURE)};
    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    return ExpectSceneAssemblyFailureWithoutMutation(
        assembly_bridge,
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        static_cast<std::uint32_t>(input_attachments.size()),
        input_bindings.data(),
        static_cast<std::uint32_t>(input_bindings.size()),
        WorldSceneAssemblyStatus::ResourceAcquireWouldOverflow,
        "scene assembly acquire overflow status wrong");
}

int WorldSceneAssemblyBridgeRestorePathDoesNotGrowStorage() {
    WorldSceneAssemblyBridgeDesc desc{};
    desc.attachment_capacity = 2U;
    desc.binding_capacity = 2U;
    WorldSceneAssemblyBridge assembly_bridge(desc);
    const WorldSceneAssemblySnapshot before_snapshot = assembly_bridge.Snapshot();
    std::uint32_t iteration = 0U;
    while (iteration < 3U) {
        ResourceRegistry registry = MakeResourceRegistry();
        std::array<WorldComponentAttachmentSnapshotRecord, 1U> input_attachments{
            MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY)};
        std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{};
        if (RegisterSceneBindingInput(
                registry,
                OBJECT_PLAYER,
                COMPONENT_TYPE_PRIMARY,
                COMPONENT_SLOT_PRIMARY,
                RESOURCE_TYPE_TEXTURE,
                "texture_scene_path",
                &input_bindings[0]) != 0) {
            return 1;
        }

        WorldComponentAttachmentBridge attachment_destination;
        WorldComponentResourceBindingBridge binding_destination;
        const WorldSceneAssemblyResult result = assembly_bridge.Restore(
            &attachment_destination,
            &binding_destination,
            &registry,
            input_attachments.data(),
            1U,
            input_bindings.data(),
            1U);
        if (!result.Succeeded()) {
            return Fail("scene assembly path restore failed");
        }

        ++iteration;
    }

    const WorldSceneAssemblySnapshot after_snapshot = assembly_bridge.Snapshot();
    if (after_snapshot.attachment_capacity != before_snapshot.attachment_capacity) {
        return Fail("scene assembly path changed attachment capacity");
    }

    if (after_snapshot.binding_capacity != before_snapshot.binding_capacity) {
        return Fail("scene assembly path changed binding capacity");
    }

    if (after_snapshot.allocation_accounting_status != before_snapshot.allocation_accounting_status) {
        return Fail("scene assembly path changed allocation accounting");
    }

    return 0;
}

int WorldSceneAssemblyBridgeNoHiddenAllocationUsesYuMemorySignal() {
    WorldSceneAssemblyBridge assembly_bridge;
    const WorldSceneAssemblySnapshot snapshot = assembly_bridge.Snapshot();
    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("scene assembly allocation accounting status wrong");
    }

    return 0;
}

int WorldSceneAssemblyBridgeSnapshotReportsCountsAndLastStatus() {
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentAttachmentSnapshotRecord, 2U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY),
        MakeSceneAttachmentRecord(OBJECT_CAMERA, COMPONENT_TYPE_SECONDARY, COMPONENT_SLOT_SECONDARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{};
    if (RegisterSceneBindingInput(
            registry,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            RESOURCE_TYPE_TEXTURE,
            "texture_scene_counter",
            &input_bindings[0]) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    const WorldSceneAssemblyResult success_result = assembly_bridge.Restore(
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        static_cast<std::uint32_t>(input_attachments.size()),
        input_bindings.data(),
        1U);
    if (!success_result.Succeeded()) {
        return Fail("scene assembly counters success failed");
    }

    const WorldSceneAssemblyResult failure_result = assembly_bridge.Restore(
        &attachment_destination,
        &binding_destination,
        &registry,
        nullptr,
        1U,
        input_bindings.data(),
        0U);
    if (failure_result.status != WorldSceneAssemblyStatus::InvalidAttachmentInput) {
        return Fail("scene assembly counters failure status wrong");
    }

    const WorldSceneAssemblySnapshot snapshot = assembly_bridge.Snapshot();
    if (snapshot.assembly_attempt_count != 2U) {
        return Fail("scene assembly counters attempt count wrong");
    }

    if (snapshot.restored_attachment_count != input_attachments.size()) {
        return Fail("scene assembly counters restored attachment count wrong");
    }

    if (snapshot.restored_binding_count != 1U) {
        return Fail("scene assembly counters restored binding count wrong");
    }

    if (snapshot.failed_operation_count != 1U) {
        return Fail("scene assembly counters failed count wrong");
    }

    if (snapshot.last_status != WorldSceneAssemblyStatus::InvalidAttachmentInput) {
        return Fail("scene assembly counters last status wrong");
    }

    return 0;
}

int WorldSceneAssemblyBridgeNoActorComponentPayloadOrLifecycle() {
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_EFFECT, COMPONENT_TYPE_TERTIARY, COMPONENT_SLOT_TERTIARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{};
    if (RegisterSceneBindingInput(
            registry,
            OBJECT_EFFECT,
            COMPONENT_TYPE_TERTIARY,
            COMPONENT_SLOT_TERTIARY,
            RESOURCE_TYPE_AUDIO,
            "audio_scene_payload",
            &input_bindings[0]) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    const WorldSceneAssemblyResult result = assembly_bridge.Restore(
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        1U,
        input_bindings.data(),
        1U);
    if (!result.Succeeded()) {
        return Fail("scene assembly payload restore failed");
    }

    const WorldComponentAttachmentResult attachment_result = attachment_destination.Query(
        OBJECT_EFFECT,
        COMPONENT_TYPE_TERTIARY);
    if (!attachment_result.Succeeded()) {
        return Fail("scene assembly payload attachment query failed");
    }

    if (attachment_result.component_slot_id.value != COMPONENT_SLOT_TERTIARY.value) {
        return Fail("scene assembly payload attachment slot wrong");
    }

    const WorldComponentResourceBindingResult binding_result = binding_destination.Query(
        OBJECT_EFFECT,
        COMPONENT_TYPE_TERTIARY,
        COMPONENT_SLOT_TERTIARY);
    if (!binding_result.Succeeded()) {
        return Fail("scene assembly payload binding query failed");
    }

    if (binding_result.resource_handle.slot != input_bindings[0].resource_handle.slot) {
        return Fail("scene assembly payload binding resource wrong");
    }

    return 0;
}

int WorldSceneAssemblyBridgeNoObjectScriptSerializeThreadPlatformDiagnosticsDependency() {
    WorldSceneAssemblyBridge assembly_bridge;
    const WorldSceneAssemblySnapshot snapshot = assembly_bridge.Snapshot();
    if (snapshot.attachment_capacity != MAX_WORLD_OBJECT_COUNT) {
        return Fail("scene assembly object dependency test changed attachment capacity");
    }

    if (snapshot.last_status != WorldSceneAssemblyStatus::Success) {
        return Fail("scene assembly object dependency test changed last status");
    }

    return 0;
}

int WorldSceneAssemblyBridgeNoFilePackageLoadDecodeUploadOrGameAdapterDependency() {
    WorldSceneAssemblyBridge assembly_bridge;
    const WorldSceneAssemblySnapshot snapshot = assembly_bridge.Snapshot();
    if (snapshot.binding_capacity != MAX_WORLD_OBJECT_COUNT) {
        return Fail("scene assembly file dependency test changed binding capacity");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("scene assembly file dependency test changed allocation accounting");
    }

    return 0;
}

int WorldSceneAssemblyBridgeNoRenderPhysicsAudioInputUiToolOrReportDependency() {
    WorldSceneAssemblyBridge assembly_bridge;
    const WorldSceneAssemblySnapshot snapshot = assembly_bridge.Snapshot();
    if (snapshot.attachment_capacity != MAX_WORLD_OBJECT_COUNT) {
        return Fail("scene assembly render dependency test changed attachment capacity");
    }

    if (snapshot.last_binding_status != WorldComponentResourceBindingStatus::Success) {
        return Fail("scene assembly render dependency test changed binding status");
    }

    return 0;
}

int WorldSceneAssemblyBridgeWorldInstanceCoreRemainsAssemblyFree() {
    WorldInstance world = MakeWorld(4U, 4U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("scene assembly world core-free registration failed");
    }

    const WorldSnapshot before_world = world.Snapshot();
    ResourceRegistry registry = MakeResourceRegistry();
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{};
    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    const WorldSceneAssemblyResult result = assembly_bridge.Restore(
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        1U,
        input_bindings.data(),
        0U);
    if (!result.Succeeded()) {
        return Fail("scene assembly world core-free restore failed");
    }

    const WorldSnapshot after_world = world.Snapshot();
    if (!WorldSnapshotsMatch(before_world, after_world)) {
        return Fail("scene assembly mutated world core");
    }

    return 0;
}

int WorldSceneAssemblyBridgeResourceCoreRemainsWorldFree() {
    ResourceRegistry registry = MakeResourceRegistry();
    const ResourceRegistrationResult resource = RegisterResource(
        registry,
        RESOURCE_TYPE_TEXTURE,
        "texture_scene_resource_core");
    if (!resource.Succeeded()) {
        return Fail("scene assembly resource core-free registration failed");
    }

    const ResourceSnapshot before_registry = registry.Snapshot();
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> input_attachments{
        MakeSceneAttachmentRecord(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY)};
    std::array<WorldComponentResourceBindingSnapshotRecord, 1U> input_bindings{};
    WorldComponentAttachmentBridge attachment_destination;
    WorldComponentResourceBindingBridge binding_destination;
    WorldSceneAssemblyBridge assembly_bridge;
    const WorldSceneAssemblyResult result = assembly_bridge.Restore(
        &attachment_destination,
        &binding_destination,
        &registry,
        input_attachments.data(),
        1U,
        input_bindings.data(),
        0U);
    if (!result.Succeeded()) {
        return Fail("scene assembly resource core-free restore failed");
    }

    if (!ResourceSnapshotsMatch(before_registry, registry.Snapshot())) {
        return Fail("scene assembly mutated resource core without bindings");
    }

    return 0;
}

int WorldComponentAttachmentBridgeAddValidAttachmentStoresRecord() {
    WorldComponentAttachmentBridge bridge;
    const WorldComponentAttachmentResult result = bridge.Add(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY);
    if (!result.Succeeded()) {
        return Fail("component attachment valid add failed");
    }

    if (result.world_object_id.value != OBJECT_PLAYER.value) {
        return Fail("component attachment valid add returned wrong world id");
    }

    if (result.component_type_id.value != COMPONENT_TYPE_PRIMARY.value) {
        return Fail("component attachment valid add returned wrong type");
    }

    if (result.component_slot_id.value != COMPONENT_SLOT_PRIMARY.value) {
        return Fail("component attachment valid add returned wrong slot");
    }

    const WorldComponentAttachmentSnapshot snapshot = bridge.Snapshot();
    if (snapshot.active_attachment_count != 1U) {
        return Fail("component attachment valid add did not record active count");
    }

    if (snapshot.added_attachment_count != 1U) {
        return Fail("component attachment valid add did not record added count");
    }

    if (snapshot.last_status != WorldComponentAttachmentStatus::Success) {
        return Fail("component attachment valid add did not record success");
    }

    return 0;
}

int WorldComponentAttachmentBridgeAddRejectsInvalidWorldIdWithoutMutation() {
    WorldComponentAttachmentBridge bridge;
    const WorldComponentAttachmentSnapshot before_snapshot = bridge.Snapshot();
    const WorldComponentAttachmentResult result = bridge.Add(
        WorldObjectId{},
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY);
    if (result.status != WorldComponentAttachmentStatus::InvalidWorldObjectId) {
        return Fail("component attachment invalid world returned wrong status");
    }

    const WorldComponentAttachmentSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.active_attachment_count != before_snapshot.active_attachment_count) {
        return Fail("component attachment invalid world mutated active count");
    }

    if (after_snapshot.added_attachment_count != before_snapshot.added_attachment_count) {
        return Fail("component attachment invalid world mutated added count");
    }

    return 0;
}

int WorldComponentAttachmentBridgeAddRejectsInvalidComponentTypeWithoutMutation() {
    WorldComponentAttachmentBridge bridge;
    const WorldComponentAttachmentSnapshot before_snapshot = bridge.Snapshot();
    const WorldComponentAttachmentResult result = bridge.Add(
        OBJECT_PLAYER,
        WorldComponentTypeId{},
        COMPONENT_SLOT_PRIMARY);
    if (result.status != WorldComponentAttachmentStatus::InvalidComponentTypeId) {
        return Fail("component attachment invalid type returned wrong status");
    }

    const WorldComponentAttachmentSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.active_attachment_count != before_snapshot.active_attachment_count) {
        return Fail("component attachment invalid type mutated active count");
    }

    if (after_snapshot.added_attachment_count != before_snapshot.added_attachment_count) {
        return Fail("component attachment invalid type mutated added count");
    }

    return 0;
}

int WorldComponentAttachmentBridgeAddRejectsInvalidComponentSlotWithoutMutation() {
    WorldComponentAttachmentBridge bridge;
    const WorldComponentAttachmentSnapshot before_snapshot = bridge.Snapshot();
    const WorldComponentAttachmentResult result = bridge.Add(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        WorldComponentSlotId{});
    if (result.status != WorldComponentAttachmentStatus::InvalidComponentSlotId) {
        return Fail("component attachment invalid slot returned wrong status");
    }

    const WorldComponentAttachmentSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.active_attachment_count != before_snapshot.active_attachment_count) {
        return Fail("component attachment invalid slot mutated active count");
    }

    if (after_snapshot.added_attachment_count != before_snapshot.added_attachment_count) {
        return Fail("component attachment invalid slot mutated added count");
    }

    return 0;
}

int WorldComponentAttachmentBridgeAddRejectsDuplicateTypeForWorldObject() {
    WorldComponentAttachmentBridge bridge;
    if (!bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component attachment duplicate first add failed");
    }

    const WorldComponentAttachmentResult duplicate_result = bridge.Add(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_SECONDARY);
    if (duplicate_result.status != WorldComponentAttachmentStatus::DuplicateAttachment) {
        return Fail("component attachment duplicate returned wrong status");
    }

    const WorldComponentAttachmentSnapshot snapshot = bridge.Snapshot();
    if (snapshot.active_attachment_count != 1U) {
        return Fail("component attachment duplicate mutated active count");
    }

    if (snapshot.duplicate_rejection_count != 1U) {
        return Fail("component attachment duplicate did not count rejection");
    }

    const WorldComponentAttachmentResult query_result = bridge.Query(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY);
    if (query_result.component_slot_id.value != COMPONENT_SLOT_PRIMARY.value) {
        return Fail("component attachment duplicate replaced stored slot");
    }

    return 0;
}

int WorldComponentAttachmentBridgeAddRejectsCapacityOverflowWithoutMutation() {
    WorldComponentAttachmentBridgeDesc desc{};
    desc.attachment_capacity = 1U;
    WorldComponentAttachmentBridge bridge(desc);
    if (!bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component attachment capacity first add failed");
    }

    const WorldComponentAttachmentResult overflow_result = bridge.Add(
        OBJECT_CAMERA,
        COMPONENT_TYPE_SECONDARY,
        COMPONENT_SLOT_SECONDARY);
    if (overflow_result.status != WorldComponentAttachmentStatus::CapacityExceeded) {
        return Fail("component attachment capacity returned wrong status");
    }

    const WorldComponentAttachmentSnapshot snapshot = bridge.Snapshot();
    if (snapshot.active_attachment_count != 1U) {
        return Fail("component attachment capacity mutated active count");
    }

    if (snapshot.added_attachment_count != 1U) {
        return Fail("component attachment capacity mutated added count");
    }

    return 0;
}

int WorldComponentAttachmentBridgeQueryReturnsStoredAttachment() {
    WorldComponentAttachmentBridge bridge;
    if (!bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_SECONDARY, COMPONENT_SLOT_SECONDARY).Succeeded()) {
        return Fail("component attachment query fixture add failed");
    }

    const WorldComponentAttachmentResult query_result = bridge.Query(OBJECT_PLAYER, COMPONENT_TYPE_SECONDARY);
    if (!query_result.Succeeded()) {
        return Fail("component attachment query failed");
    }

    if (query_result.component_slot_id.value != COMPONENT_SLOT_SECONDARY.value) {
        return Fail("component attachment query returned wrong slot");
    }

    if (query_result.component_type_id.value != COMPONENT_TYPE_SECONDARY.value) {
        return Fail("component attachment query returned wrong type");
    }

    return 0;
}

int WorldComponentAttachmentBridgeQueryRejectsMissingAttachmentWithoutMutation() {
    WorldComponentAttachmentBridge bridge;
    const WorldComponentAttachmentSnapshot before_snapshot = bridge.Snapshot();
    const WorldComponentAttachmentResult query_result = bridge.Query(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY);
    if (query_result.status != WorldComponentAttachmentStatus::AttachmentNotFound) {
        return Fail("component attachment missing query returned wrong status");
    }

    const WorldComponentAttachmentSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.active_attachment_count != before_snapshot.active_attachment_count) {
        return Fail("component attachment missing query mutated active count");
    }

    if (after_snapshot.added_attachment_count != before_snapshot.added_attachment_count) {
        return Fail("component attachment missing query mutated added count");
    }

    return 0;
}

int WorldComponentAttachmentBridgeQueryIsReadOnlyAndBounded() {
    WorldComponentAttachmentBridge bridge;
    if (!bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component attachment readonly fixture add failed");
    }

    const WorldComponentAttachmentSnapshot before_snapshot = bridge.Snapshot();
    std::uint32_t query_index = 0U;
    while (query_index < 3U) {
        if (!bridge.Query(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY).Succeeded()) {
            return Fail("component attachment readonly query failed");
        }

        ++query_index;
    }

    const WorldComponentAttachmentSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.attachment_capacity != before_snapshot.attachment_capacity) {
        return Fail("component attachment readonly mutated capacity");
    }

    if (after_snapshot.active_attachment_count != before_snapshot.active_attachment_count) {
        return Fail("component attachment readonly mutated active count");
    }

    if (after_snapshot.added_attachment_count != before_snapshot.added_attachment_count) {
        return Fail("component attachment readonly mutated added count");
    }

    if (after_snapshot.removed_attachment_count != before_snapshot.removed_attachment_count) {
        return Fail("component attachment readonly mutated removed count");
    }

    if (after_snapshot.query_count != before_snapshot.query_count + 3U) {
        return Fail("component attachment readonly query count wrong");
    }

    return 0;
}

int WorldComponentAttachmentBridgeRemoveClearsAttachment() {
    WorldComponentAttachmentBridge bridge;
    if (!bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component attachment remove fixture add failed");
    }

    const WorldComponentAttachmentStatus remove_status = bridge.Remove(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY);
    if (remove_status != WorldComponentAttachmentStatus::Success) {
        return Fail("component attachment remove returned wrong status");
    }

    const WorldComponentAttachmentSnapshot snapshot = bridge.Snapshot();
    if (snapshot.active_attachment_count != 0U) {
        return Fail("component attachment remove did not clear active count");
    }

    if (snapshot.removed_attachment_count != 1U) {
        return Fail("component attachment remove did not count removal");
    }

    const WorldComponentAttachmentResult query_result = bridge.Query(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY);
    if (query_result.status != WorldComponentAttachmentStatus::AttachmentNotFound) {
        return Fail("component attachment remove left queryable record");
    }

    return 0;
}

int WorldComponentAttachmentBridgeRemoveRejectsMissingAttachmentWithoutMutation() {
    WorldComponentAttachmentBridge bridge;
    const WorldComponentAttachmentSnapshot before_snapshot = bridge.Snapshot();
    const WorldComponentAttachmentStatus remove_status = bridge.Remove(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY);
    if (remove_status != WorldComponentAttachmentStatus::AttachmentNotFound) {
        return Fail("component attachment missing remove returned wrong status");
    }

    const WorldComponentAttachmentSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.active_attachment_count != before_snapshot.active_attachment_count) {
        return Fail("component attachment missing remove mutated active count");
    }

    if (after_snapshot.removed_attachment_count != before_snapshot.removed_attachment_count) {
        return Fail("component attachment missing remove mutated removed count");
    }

    return 0;
}

int WorldComponentAttachmentBridgeClearRemovesAllAttachmentsInSlotOrder() {
    WorldComponentAttachmentBridge bridge;
    if (!bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_TERTIARY).Succeeded()) {
        return Fail("component attachment clear first add failed");
    }

    if (!bridge.Add(OBJECT_CAMERA, COMPONENT_TYPE_SECONDARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component attachment clear second add failed");
    }

    if (!bridge.Add(OBJECT_EFFECT, COMPONENT_TYPE_TERTIARY, COMPONENT_SLOT_SECONDARY).Succeeded()) {
        return Fail("component attachment clear third add failed");
    }

    const WorldComponentAttachmentStatus clear_status = bridge.Clear();
    if (clear_status != WorldComponentAttachmentStatus::Success) {
        return Fail("component attachment clear returned wrong status");
    }

    const WorldComponentAttachmentSnapshot snapshot = bridge.Snapshot();
    if (snapshot.active_attachment_count != 0U) {
        return Fail("component attachment clear did not clear active count");
    }

    if (snapshot.cleared_attachment_count != 3U) {
        return Fail("component attachment clear did not count cleared attachments");
    }

    return 0;
}

int WorldComponentAttachmentBridgeUpdatePathDoesNotGrowStorage() {
    WorldComponentAttachmentBridgeDesc desc{};
    desc.attachment_capacity = 2U;
    WorldComponentAttachmentBridge bridge(desc);
    const WorldComponentAttachmentSnapshot before_snapshot = bridge.Snapshot();
    std::uint32_t iteration = 0U;
    while (iteration < 2U) {
        WorldComponentTypeId component_type_id = COMPONENT_TYPE_PRIMARY;
        WorldComponentSlotId component_slot_id = COMPONENT_SLOT_PRIMARY;
        if (iteration == 1U) {
            component_type_id = COMPONENT_TYPE_SECONDARY;
            component_slot_id = COMPONENT_SLOT_SECONDARY;
        }

        if (!bridge.Add(OBJECT_PLAYER, component_type_id, component_slot_id).Succeeded()) {
            return Fail("component attachment update path add failed");
        }

        if (!bridge.Query(OBJECT_PLAYER, component_type_id).Succeeded()) {
            return Fail("component attachment update path query failed");
        }

        if (bridge.Remove(OBJECT_PLAYER, component_type_id) != WorldComponentAttachmentStatus::Success) {
            return Fail("component attachment update path remove failed");
        }

        ++iteration;
    }

    const WorldComponentAttachmentSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.attachment_capacity != before_snapshot.attachment_capacity) {
        return Fail("component attachment update path changed capacity");
    }

    if (after_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("component attachment update path changed allocation accounting");
    }

    if (after_snapshot.active_attachment_count != 0U) {
        return Fail("component attachment update path left active attachment");
    }

    return 0;
}

int WorldComponentAttachmentBridgeSnapshotReportsCountsAndLastStatus() {
    WorldComponentAttachmentBridge bridge;
    if (!bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component attachment snapshot add failed");
    }

    const WorldComponentAttachmentResult duplicate_result = bridge.Add(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_SECONDARY);
    if (duplicate_result.status != WorldComponentAttachmentStatus::DuplicateAttachment) {
        return Fail("component attachment snapshot duplicate status wrong");
    }

    const WorldComponentAttachmentSnapshot snapshot = bridge.Snapshot();
    if (snapshot.active_attachment_count != 1U) {
        return Fail("component attachment snapshot active count wrong");
    }

    if (snapshot.added_attachment_count != 1U) {
        return Fail("component attachment snapshot added count wrong");
    }

    if (snapshot.duplicate_rejection_count != 1U) {
        return Fail("component attachment snapshot duplicate count wrong");
    }

    if (snapshot.failed_operation_count != 1U) {
        return Fail("component attachment snapshot failure count wrong");
    }

    if (snapshot.last_status != WorldComponentAttachmentStatus::DuplicateAttachment) {
        return Fail("component attachment snapshot last status wrong");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("component attachment snapshot allocation accounting wrong");
    }

    return 0;
}

int WorldComponentAttachmentBridgeDoesNotQueryOrMutateWorldInstance() {
    WorldInstance world = MakeWorld(4U, 4U);
    const WorldSnapshot before_world = world.Snapshot();
    WorldComponentAttachmentBridge bridge;
    const WorldComponentAttachmentResult result = bridge.Add(
        OBJECT_EFFECT,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY);
    if (!result.Succeeded()) {
        return Fail("component attachment no-world-query add failed");
    }

    const WorldSnapshot after_world = world.Snapshot();
    if (!WorldSnapshotsMatch(before_world, after_world)) {
        return Fail("component attachment no-world-query mutated world");
    }

    return 0;
}

int WorldComponentAttachmentBridgeNoActorComponentBehaviorOrLifecycle() {
    WorldComponentAttachmentBridge bridge;
    if (!bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component attachment behavior first add failed");
    }

    if (!bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_SECONDARY, COMPONENT_SLOT_SECONDARY).Succeeded()) {
        return Fail("component attachment behavior second add failed");
    }

    const WorldComponentAttachmentSnapshot snapshot = bridge.Snapshot();
    if (snapshot.active_attachment_count != 2U) {
        return Fail("component attachment behavior did not remain a sidecar table");
    }

    if (snapshot.attachment_capacity != MAX_WORLD_OBJECT_COUNT) {
        return Fail("component attachment behavior changed default capacity");
    }

    return 0;
}

int WorldComponentAttachmentBridgeNoObjectResourceScriptSerializeOrGameAdapterDependency() {
    WorldComponentAttachmentBridge bridge;
    const WorldComponentAttachmentResult result = bridge.Add(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY);
    if (!result.Succeeded()) {
        return Fail("component attachment module boundary add failed");
    }

    const WorldComponentAttachmentResult query_result = bridge.Query(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY);
    if (!query_result.Succeeded()) {
        return Fail("component attachment module boundary query failed");
    }

    if (bridge.Remove(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY) != WorldComponentAttachmentStatus::Success) {
        return Fail("component attachment module boundary remove failed");
    }

    return 0;
}

int WorldComponentAttachmentBridgeNoFilePackageThreadPlatformDiagnosticsDependency() {
    WorldComponentAttachmentBridge bridge;
    const WorldComponentAttachmentSnapshot snapshot = bridge.Snapshot();
    if (snapshot.attachment_capacity != MAX_WORLD_OBJECT_COUNT) {
        return Fail("component attachment file boundary changed default capacity");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("component attachment file boundary changed allocation accounting");
    }

    return 0;
}

int WorldComponentAttachmentBridgeNoRenderPhysicsAudioInputUiToolOrReportDependency() {
    WorldComponentAttachmentBridge bridge;
    const WorldComponentAttachmentResult result = bridge.Add(
        OBJECT_PLAYER,
        COMPONENT_TYPE_TERTIARY,
        COMPONENT_SLOT_TERTIARY);
    if (!result.Succeeded()) {
        return Fail("component attachment render boundary add failed");
    }

    const WorldComponentAttachmentSnapshot snapshot = bridge.Snapshot();
    if (snapshot.active_attachment_count != 1U) {
        return Fail("component attachment render boundary changed active count");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("component attachment render boundary changed allocation accounting");
    }

    return 0;
}

int WorldComponentAttachmentBridgeWorldInstanceCoreRemainsAttachmentFree() {
    WorldInstance world = MakeWorld(4U, 4U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("component attachment world core-free registration failed");
    }

    const WorldSnapshot before_world = world.Snapshot();
    WorldComponentAttachmentBridge bridge;
    const WorldComponentAttachmentResult result = bridge.Add(
        OBJECT_PLAYER,
        COMPONENT_TYPE_PRIMARY,
        COMPONENT_SLOT_PRIMARY);
    if (!result.Succeeded()) {
        return Fail("component attachment world core-free add failed");
    }

    const WorldSnapshot after_world = world.Snapshot();
    if (!WorldSnapshotsMatch(before_world, after_world)) {
        return Fail("component attachment world core-free mutated world");
    }

    return 0;
}

int WorldComponentQueryBridgeQueryTypeReturnsMatchingWorldObjectsInSlotOrder() {
    WorldComponentAttachmentBridge source_bridge;
    if (!source_bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component query type first add failed");
    }

    if (!source_bridge.Add(OBJECT_CAMERA, COMPONENT_TYPE_SECONDARY, COMPONENT_SLOT_SECONDARY).Succeeded()) {
        return Fail("component query type second add failed");
    }

    if (!source_bridge.Add(OBJECT_EFFECT, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_TERTIARY).Succeeded()) {
        return Fail("component query type third add failed");
    }

    WorldComponentQueryBridge bridge;
    std::array<WorldObjectId, 2U> output_world_object_ids{};
    WorldComponentQueryTypeDesc desc{};
    desc.source_bridge = &source_bridge;
    desc.component_type_id = COMPONENT_TYPE_PRIMARY;
    desc.output_world_object_ids = output_world_object_ids.data();
    desc.output_capacity = static_cast<std::uint32_t>(output_world_object_ids.size());

    const WorldComponentQueryResult result = bridge.QueryType(desc);
    if (!result.Succeeded()) {
        return Fail("component query type result failed");
    }

    if (result.matched_record_count != 2U) {
        return Fail("component query type matched count wrong");
    }

    if (result.written_record_count != 2U) {
        return Fail("component query type written count wrong");
    }

    if (output_world_object_ids[0].value != OBJECT_PLAYER.value) {
        return Fail("component query type first output wrong");
    }

    if (output_world_object_ids[1].value != OBJECT_EFFECT.value) {
        return Fail("component query type second output wrong");
    }

    return 0;
}

int WorldComponentQueryBridgeQueryTypeReturnsZeroForMissingType() {
    WorldComponentAttachmentBridge source_bridge;
    if (!source_bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component query missing type add failed");
    }

    WorldComponentQueryBridge bridge;
    std::array<WorldObjectId, 1U> output_world_object_ids{OBJECT_CAMERA};
    WorldComponentQueryTypeDesc desc{};
    desc.source_bridge = &source_bridge;
    desc.component_type_id = COMPONENT_TYPE_TERTIARY;
    desc.output_world_object_ids = output_world_object_ids.data();
    desc.output_capacity = static_cast<std::uint32_t>(output_world_object_ids.size());

    const WorldComponentQueryResult result = bridge.QueryType(desc);
    if (!result.Succeeded()) {
        return Fail("component query missing type result failed");
    }

    if (result.matched_record_count != 0U) {
        return Fail("component query missing type matched count wrong");
    }

    if (result.written_record_count != 0U) {
        return Fail("component query missing type written count wrong");
    }

    if (output_world_object_ids[0].value != OBJECT_CAMERA.value) {
        return Fail("component query missing type mutated output");
    }

    return 0;
}

int WorldComponentQueryBridgeQueryObjectReturnsMatchingAttachmentsInSlotOrder() {
    WorldComponentAttachmentBridge source_bridge;
    if (!source_bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component query object first add failed");
    }

    if (!source_bridge.Add(OBJECT_CAMERA, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_SECONDARY).Succeeded()) {
        return Fail("component query object second add failed");
    }

    if (!source_bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_SECONDARY, COMPONENT_SLOT_TERTIARY).Succeeded()) {
        return Fail("component query object third add failed");
    }

    WorldComponentQueryBridge bridge;
    std::array<WorldComponentAttachment, 2U> output_attachments{};
    WorldComponentQueryObjectDesc desc{};
    desc.source_bridge = &source_bridge;
    desc.world_object_id = OBJECT_PLAYER;
    desc.output_attachments = output_attachments.data();
    desc.output_capacity = static_cast<std::uint32_t>(output_attachments.size());

    const WorldComponentQueryResult result = bridge.QueryObject(desc);
    if (!result.Succeeded()) {
        return Fail("component query object result failed");
    }

    if (result.matched_record_count != 2U) {
        return Fail("component query object matched count wrong");
    }

    if (output_attachments[0].component_type_id.value != COMPONENT_TYPE_PRIMARY.value) {
        return Fail("component query object first type wrong");
    }

    if (output_attachments[0].component_slot_id.value != COMPONENT_SLOT_PRIMARY.value) {
        return Fail("component query object first slot wrong");
    }

    if (output_attachments[1].component_type_id.value != COMPONENT_TYPE_SECONDARY.value) {
        return Fail("component query object second type wrong");
    }

    if (output_attachments[1].component_slot_id.value != COMPONENT_SLOT_TERTIARY.value) {
        return Fail("component query object second slot wrong");
    }

    return 0;
}

int WorldComponentQueryBridgeQueryObjectReturnsZeroForMissingObject() {
    WorldComponentAttachmentBridge source_bridge;
    if (!source_bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component query missing object add failed");
    }

    WorldComponentAttachment output_attachment{};
    output_attachment.world_object_id = OBJECT_PLAYER;
    output_attachment.component_type_id = COMPONENT_TYPE_PRIMARY;
    output_attachment.component_slot_id = COMPONENT_SLOT_PRIMARY;
    output_attachment.is_attached = true;

    WorldComponentQueryBridge bridge;
    std::array<WorldComponentAttachment, 1U> output_attachments{output_attachment};
    WorldComponentQueryObjectDesc desc{};
    desc.source_bridge = &source_bridge;
    desc.world_object_id = OBJECT_EFFECT;
    desc.output_attachments = output_attachments.data();
    desc.output_capacity = static_cast<std::uint32_t>(output_attachments.size());

    const WorldComponentQueryResult result = bridge.QueryObject(desc);
    if (!result.Succeeded()) {
        return Fail("component query missing object result failed");
    }

    if (result.matched_record_count != 0U) {
        return Fail("component query missing object matched count wrong");
    }

    if (result.written_record_count != 0U) {
        return Fail("component query missing object written count wrong");
    }

    if (output_attachments[0].component_slot_id.value != COMPONENT_SLOT_PRIMARY.value) {
        return Fail("component query missing object mutated output");
    }

    return 0;
}

int WorldComponentQueryBridgeQueryRejectsNullSourceWithoutMutation() {
    WorldComponentQueryBridge bridge;
    std::array<WorldObjectId, 1U> output_world_object_ids{OBJECT_EFFECT};
    WorldComponentQueryTypeDesc desc{};
    desc.source_bridge = nullptr;
    desc.component_type_id = COMPONENT_TYPE_PRIMARY;
    desc.output_world_object_ids = output_world_object_ids.data();
    desc.output_capacity = static_cast<std::uint32_t>(output_world_object_ids.size());

    const WorldComponentQueryResult result = bridge.QueryType(desc);
    if (result.status != WorldComponentQueryStatus::InvalidSourceBridge) {
        return Fail("component query null source status wrong");
    }

    if (output_world_object_ids[0].value != OBJECT_EFFECT.value) {
        return Fail("component query null source mutated output");
    }

    const WorldComponentQuerySnapshot snapshot = bridge.Snapshot();
    if (snapshot.failed_operation_count != 1U) {
        return Fail("component query null source failure count wrong");
    }

    return 0;
}

int WorldComponentQueryBridgeQueryRejectsInvalidComponentTypeWithoutMutation() {
    WorldComponentAttachmentBridge source_bridge;
    if (!source_bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component query invalid type add failed");
    }

    const WorldComponentAttachmentSnapshot before_snapshot = source_bridge.Snapshot();
    WorldComponentQueryBridge bridge;
    std::array<WorldObjectId, 1U> output_world_object_ids{OBJECT_EFFECT};
    WorldComponentQueryTypeDesc desc{};
    desc.source_bridge = &source_bridge;
    desc.component_type_id = WorldComponentTypeId{};
    desc.output_world_object_ids = output_world_object_ids.data();
    desc.output_capacity = static_cast<std::uint32_t>(output_world_object_ids.size());

    const WorldComponentQueryResult result = bridge.QueryType(desc);
    if (result.status != WorldComponentQueryStatus::InvalidComponentTypeId) {
        return Fail("component query invalid type status wrong");
    }

    if (output_world_object_ids[0].value != OBJECT_EFFECT.value) {
        return Fail("component query invalid type mutated output");
    }

    const WorldComponentAttachmentSnapshot after_snapshot = source_bridge.Snapshot();
    if (!ComponentAttachmentSnapshotsMatch(before_snapshot, after_snapshot)) {
        return Fail("component query invalid type mutated source");
    }

    return 0;
}

int WorldComponentQueryBridgeQueryRejectsInvalidWorldIdWithoutMutation() {
    WorldComponentAttachmentBridge source_bridge;
    if (!source_bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component query invalid world add failed");
    }

    WorldComponentAttachment output_attachment{};
    output_attachment.world_object_id = OBJECT_EFFECT;
    output_attachment.component_type_id = COMPONENT_TYPE_TERTIARY;
    output_attachment.component_slot_id = COMPONENT_SLOT_TERTIARY;
    output_attachment.is_attached = true;

    const WorldComponentAttachmentSnapshot before_snapshot = source_bridge.Snapshot();
    WorldComponentQueryBridge bridge;
    std::array<WorldComponentAttachment, 1U> output_attachments{output_attachment};
    WorldComponentQueryObjectDesc desc{};
    desc.source_bridge = &source_bridge;
    desc.world_object_id = WorldObjectId{};
    desc.output_attachments = output_attachments.data();
    desc.output_capacity = static_cast<std::uint32_t>(output_attachments.size());

    const WorldComponentQueryResult result = bridge.QueryObject(desc);
    if (result.status != WorldComponentQueryStatus::InvalidWorldObjectId) {
        return Fail("component query invalid world status wrong");
    }

    if (output_attachments[0].component_slot_id.value != COMPONENT_SLOT_TERTIARY.value) {
        return Fail("component query invalid world mutated output");
    }

    const WorldComponentAttachmentSnapshot after_snapshot = source_bridge.Snapshot();
    if (!ComponentAttachmentSnapshotsMatch(before_snapshot, after_snapshot)) {
        return Fail("component query invalid world mutated source");
    }

    return 0;
}

int WorldComponentQueryBridgeQueryRejectsNullOutputWhenCapacityNonZero() {
    WorldComponentAttachmentBridge source_bridge;
    if (!source_bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component query null output add failed");
    }

    WorldComponentQueryBridge bridge;
    WorldComponentQueryTypeDesc desc{};
    desc.source_bridge = &source_bridge;
    desc.component_type_id = COMPONENT_TYPE_PRIMARY;
    desc.output_world_object_ids = nullptr;
    desc.output_capacity = 1U;

    const WorldComponentQueryResult result = bridge.QueryType(desc);
    if (result.status != WorldComponentQueryStatus::InvalidOutputBuffer) {
        return Fail("component query null output status wrong");
    }

    const WorldComponentQuerySnapshot snapshot = bridge.Snapshot();
    if (snapshot.failed_operation_count != 1U) {
        return Fail("component query null output failure count wrong");
    }

    return 0;
}

int WorldComponentQueryBridgeQueryRejectsOutputOverflowWithoutOverrun() {
    WorldComponentAttachmentBridge source_bridge;
    if (!source_bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component query overflow first add failed");
    }

    if (!source_bridge.Add(OBJECT_EFFECT, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_SECONDARY).Succeeded()) {
        return Fail("component query overflow second add failed");
    }

    WorldComponentQueryBridge bridge;
    std::array<WorldObjectId, 2U> output_world_object_ids{WorldObjectId{}, OBJECT_CAMERA};
    WorldComponentQueryTypeDesc desc{};
    desc.source_bridge = &source_bridge;
    desc.component_type_id = COMPONENT_TYPE_PRIMARY;
    desc.output_world_object_ids = output_world_object_ids.data();
    desc.output_capacity = 1U;

    const WorldComponentQueryResult result = bridge.QueryType(desc);
    if (result.status != WorldComponentQueryStatus::OutputCapacityExceeded) {
        return Fail("component query overflow status wrong");
    }

    if (result.matched_record_count != 2U) {
        return Fail("component query overflow matched count wrong");
    }

    if (result.written_record_count != 1U) {
        return Fail("component query overflow written count wrong");
    }

    if (output_world_object_ids[0].value != OBJECT_PLAYER.value) {
        return Fail("component query overflow first output wrong");
    }

    if (output_world_object_ids[1].value != OBJECT_CAMERA.value) {
        return Fail("component query overflow overran output");
    }

    return 0;
}

int WorldComponentQueryBridgeQueryIsReadOnlyForAttachmentStorage() {
    WorldComponentAttachmentBridge source_bridge;
    if (!source_bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component query read-only first add failed");
    }

    if (!source_bridge.Add(OBJECT_EFFECT, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_SECONDARY).Succeeded()) {
        return Fail("component query read-only second add failed");
    }

    const WorldComponentAttachmentSnapshot before_snapshot = source_bridge.Snapshot();
    WorldComponentQueryBridge bridge;
    std::array<WorldObjectId, 2U> output_world_object_ids{};
    WorldComponentQueryTypeDesc desc{};
    desc.source_bridge = &source_bridge;
    desc.component_type_id = COMPONENT_TYPE_PRIMARY;
    desc.output_world_object_ids = output_world_object_ids.data();
    desc.output_capacity = static_cast<std::uint32_t>(output_world_object_ids.size());

    const WorldComponentQueryResult result = bridge.QueryType(desc);
    if (!result.Succeeded()) {
        return Fail("component query read-only result failed");
    }

    const WorldComponentAttachmentSnapshot after_snapshot = source_bridge.Snapshot();
    if (!ComponentAttachmentSnapshotsMatch(before_snapshot, after_snapshot)) {
        return Fail("component query read-only mutated source");
    }

    return 0;
}

int WorldComponentQueryBridgeQueryPathDoesNotGrowStorage() {
    WorldComponentAttachmentBridge source_bridge;
    if (!source_bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component query path first add failed");
    }

    if (!source_bridge.Add(OBJECT_EFFECT, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_SECONDARY).Succeeded()) {
        return Fail("component query path second add failed");
    }

    WorldComponentQueryBridge bridge;
    const WorldComponentQuerySnapshot before_snapshot = bridge.Snapshot();
    std::array<WorldObjectId, 2U> output_world_object_ids{};
    WorldComponentQueryTypeDesc desc{};
    desc.source_bridge = &source_bridge;
    desc.component_type_id = COMPONENT_TYPE_PRIMARY;
    desc.output_world_object_ids = output_world_object_ids.data();
    desc.output_capacity = static_cast<std::uint32_t>(output_world_object_ids.size());

    std::uint32_t iteration = 0U;
    while (iteration < 3U) {
        const WorldComponentQueryResult result = bridge.QueryType(desc);
        if (!result.Succeeded()) {
            return Fail("component query path result failed");
        }

        ++iteration;
    }

    const WorldComponentQuerySnapshot after_snapshot = bridge.Snapshot();
    if (before_snapshot.allocation_accounting_status != after_snapshot.allocation_accounting_status) {
        return Fail("component query path changed allocation accounting");
    }

    if (after_snapshot.query_count != 3U) {
        return Fail("component query path query count wrong");
    }

    if (after_snapshot.matched_record_count != 6U) {
        return Fail("component query path matched count wrong");
    }

    return 0;
}

int WorldComponentQueryBridgeSnapshotReportsCountsAndLastStatus() {
    WorldComponentAttachmentBridge source_bridge;
    if (!source_bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component query snapshot first add failed");
    }

    if (!source_bridge.Add(OBJECT_EFFECT, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_SECONDARY).Succeeded()) {
        return Fail("component query snapshot second add failed");
    }

    WorldComponentQueryBridge bridge;
    std::array<WorldObjectId, 2U> full_output_world_object_ids{};
    WorldComponentQueryTypeDesc full_desc{};
    full_desc.source_bridge = &source_bridge;
    full_desc.component_type_id = COMPONENT_TYPE_PRIMARY;
    full_desc.output_world_object_ids = full_output_world_object_ids.data();
    full_desc.output_capacity = static_cast<std::uint32_t>(full_output_world_object_ids.size());

    if (!bridge.QueryType(full_desc).Succeeded()) {
        return Fail("component query snapshot success query failed");
    }

    std::array<WorldObjectId, 1U> small_output_world_object_ids{};
    WorldComponentQueryTypeDesc small_desc{};
    small_desc.source_bridge = &source_bridge;
    small_desc.component_type_id = COMPONENT_TYPE_PRIMARY;
    small_desc.output_world_object_ids = small_output_world_object_ids.data();
    small_desc.output_capacity = static_cast<std::uint32_t>(small_output_world_object_ids.size());

    const WorldComponentQueryResult overflow_result = bridge.QueryType(small_desc);
    if (overflow_result.status != WorldComponentQueryStatus::OutputCapacityExceeded) {
        return Fail("component query snapshot overflow status wrong");
    }

    const WorldComponentQuerySnapshot snapshot = bridge.Snapshot();
    if (snapshot.query_count != 2U) {
        return Fail("component query snapshot query count wrong");
    }

    if (snapshot.matched_record_count != 4U) {
        return Fail("component query snapshot matched count wrong");
    }

    if (snapshot.overflow_rejection_count != 1U) {
        return Fail("component query snapshot overflow count wrong");
    }

    if (snapshot.failed_operation_count != 1U) {
        return Fail("component query snapshot failure count wrong");
    }

    if (snapshot.last_status != WorldComponentQueryStatus::OutputCapacityExceeded) {
        return Fail("component query snapshot last status wrong");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("component query snapshot allocation accounting wrong");
    }

    return 0;
}

int WorldComponentQueryBridgeNoActorComponentBehaviorOrLifecycle() {
    WorldComponentAttachmentBridge source_bridge;
    if (!source_bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component query behavior first add failed");
    }

    if (!source_bridge.Add(OBJECT_CAMERA, COMPONENT_TYPE_SECONDARY, COMPONENT_SLOT_SECONDARY).Succeeded()) {
        return Fail("component query behavior second add failed");
    }

    WorldComponentQueryBridge bridge;
    std::array<WorldComponentAttachment, 2U> output_attachments{};
    WorldComponentQueryObjectDesc desc{};
    desc.source_bridge = &source_bridge;
    desc.world_object_id = OBJECT_PLAYER;
    desc.output_attachments = output_attachments.data();
    desc.output_capacity = static_cast<std::uint32_t>(output_attachments.size());

    const WorldComponentQueryResult result = bridge.QueryObject(desc);
    if (!result.Succeeded()) {
        return Fail("component query behavior result failed");
    }

    if (result.matched_record_count != 1U) {
        return Fail("component query behavior matched count wrong");
    }

    const WorldComponentAttachmentSnapshot source_snapshot = source_bridge.Snapshot();
    if (source_snapshot.active_attachment_count != 2U) {
        return Fail("component query behavior changed attachment storage");
    }

    return 0;
}

int WorldComponentQueryBridgeNoObjectResourceScriptSerializeOrGameAdapterDependency() {
    WorldComponentAttachmentBridge source_bridge;
    if (!source_bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component query module boundary add failed");
    }

    WorldComponentQueryBridge bridge;
    std::array<WorldObjectId, 1U> output_world_object_ids{};
    WorldComponentQueryTypeDesc desc{};
    desc.source_bridge = &source_bridge;
    desc.component_type_id = COMPONENT_TYPE_PRIMARY;
    desc.output_world_object_ids = output_world_object_ids.data();
    desc.output_capacity = static_cast<std::uint32_t>(output_world_object_ids.size());

    const WorldComponentQueryResult result = bridge.QueryType(desc);
    if (!result.Succeeded()) {
        return Fail("component query module boundary query failed");
    }

    if (output_world_object_ids[0].value != OBJECT_PLAYER.value) {
        return Fail("component query module boundary output wrong");
    }

    return 0;
}

int WorldComponentQueryBridgeNoFilePackageThreadPlatformDiagnosticsDependency() {
    WorldComponentQueryBridge bridge;
    const WorldComponentQuerySnapshot snapshot = bridge.Snapshot();
    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("component query file boundary allocation accounting wrong");
    }

    if (snapshot.query_count != 0U) {
        return Fail("component query file boundary query count wrong");
    }

    return 0;
}

int WorldComponentQueryBridgeNoRenderPhysicsAudioInputUiToolOrReportDependency() {
    WorldComponentAttachmentBridge source_bridge;
    if (!source_bridge.Add(OBJECT_EFFECT, COMPONENT_TYPE_TERTIARY, COMPONENT_SLOT_TERTIARY).Succeeded()) {
        return Fail("component query render boundary add failed");
    }

    WorldComponentQueryBridge bridge;
    std::array<WorldObjectId, 1U> output_world_object_ids{};
    WorldComponentQueryTypeDesc desc{};
    desc.source_bridge = &source_bridge;
    desc.component_type_id = COMPONENT_TYPE_TERTIARY;
    desc.output_world_object_ids = output_world_object_ids.data();
    desc.output_capacity = static_cast<std::uint32_t>(output_world_object_ids.size());

    const WorldComponentQueryResult result = bridge.QueryType(desc);
    if (!result.Succeeded()) {
        return Fail("component query render boundary result failed");
    }

    const WorldComponentQuerySnapshot snapshot = bridge.Snapshot();
    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("component query render boundary allocation accounting wrong");
    }

    return 0;
}

int WorldComponentQueryBridgeWorldInstanceCoreRemainsQueryFree() {
    WorldInstance world = MakeWorld(4U, 4U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("component query world core-free registration failed");
    }

    WorldComponentAttachmentBridge source_bridge;
    if (!source_bridge.Add(OBJECT_PLAYER, COMPONENT_TYPE_PRIMARY, COMPONENT_SLOT_PRIMARY).Succeeded()) {
        return Fail("component query world core-free add failed");
    }

    const WorldSnapshot before_world = world.Snapshot();
    WorldComponentQueryBridge bridge;
    std::array<WorldObjectId, 1U> output_world_object_ids{};
    WorldComponentQueryTypeDesc desc{};
    desc.source_bridge = &source_bridge;
    desc.component_type_id = COMPONENT_TYPE_PRIMARY;
    desc.output_world_object_ids = output_world_object_ids.data();
    desc.output_capacity = static_cast<std::uint32_t>(output_world_object_ids.size());

    const WorldComponentQueryResult result = bridge.QueryType(desc);
    if (!result.Succeeded()) {
        return Fail("component query world core-free result failed");
    }

    const WorldSnapshot after_world = world.Snapshot();
    if (!WorldSnapshotsMatch(before_world, after_world)) {
        return Fail("component query world core-free mutated world");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeWriteReadRoundTripsAttachmentsInSlotOrder() {
    WorldComponentAttachmentBridge source_bridge;
    if (AddComponentAttachment(
            source_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component attachment snapshot round trip first add failed") != 0) {
        return 1;
    }

    if (AddComponentAttachment(
            source_bridge,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            "component attachment snapshot round trip second add failed") != 0) {
        return 1;
    }

    if (AddComponentAttachment(
            source_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_TERTIARY,
            COMPONENT_SLOT_TERTIARY,
            "component attachment snapshot round trip third add failed") != 0) {
        return 1;
    }

    WorldComponentAttachmentSnapshotBridge bridge;
    SerializeBuffer buffer{};
    std::uint32_t committed_byte_count = 0U;
    if (WriteComponentAttachmentSnapshotToBuffer(
            bridge,
            source_bridge,
            buffer,
            committed_byte_count) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge destination_bridge;
    SerializeReader reader(buffer.data(), committed_byte_count);
    const WorldComponentAttachmentSnapshotResult read_result = bridge.ReadSnapshot(
        &reader,
        &destination_bridge);
    if (!read_result.Succeeded()) {
        return Fail("component attachment snapshot round trip read failed");
    }

    std::array<WorldComponentAttachment, 3U> attachments{};
    const std::uint32_t attachment_count = destination_bridge.ExportAttachments(
        attachments.data(),
        static_cast<std::uint32_t>(attachments.size()));
    if (attachment_count != 3U) {
        return Fail("component attachment snapshot round trip count wrong");
    }

    if (attachments[0].world_object_id.value != OBJECT_PLAYER.value) {
        return Fail("component attachment snapshot round trip first object wrong");
    }

    if (attachments[0].component_type_id.value != COMPONENT_TYPE_PRIMARY.value) {
        return Fail("component attachment snapshot round trip first type wrong");
    }

    if (attachments[1].world_object_id.value != OBJECT_CAMERA.value) {
        return Fail("component attachment snapshot round trip second object wrong");
    }

    if (attachments[1].component_slot_id.value != COMPONENT_SLOT_SECONDARY.value) {
        return Fail("component attachment snapshot round trip second slot wrong");
    }

    if (attachments[2].component_type_id.value != COMPONENT_TYPE_TERTIARY.value) {
        return Fail("component attachment snapshot round trip third type wrong");
    }

    if (read_result.state.attachment_record_count != 3U) {
        return Fail("component attachment snapshot round trip read state count wrong");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeWriteEmptyBridgeProducesZeroRecords() {
    WorldComponentAttachmentBridge source_bridge;
    WorldComponentAttachmentSnapshotBridge bridge;
    SerializeBuffer buffer{};
    std::uint32_t committed_byte_count = 0U;
    if (WriteComponentAttachmentSnapshotToBuffer(
            bridge,
            source_bridge,
            buffer,
            committed_byte_count) != 0) {
        return 1;
    }

    const WorldComponentAttachmentSnapshotBridgeSnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.written_record_count != 0U) {
        return Fail("component attachment snapshot empty write record count wrong");
    }

    SerializeReader reader(buffer.data(), committed_byte_count);
    if (OpenSerializeStream(reader) != 0) {
        return 1;
    }

    std::uint32_t record_count = 999U;
    const SerializeStatus status = reader.ReadUInt32(
        WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_METADATA_RECORD_ID,
        WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_FIELD_RECORD_COUNT,
        record_count);
    if (status != SerializeStatus::Success) {
        return Fail("component attachment snapshot empty metadata read failed");
    }

    if (record_count != 0U) {
        return Fail("component attachment snapshot empty write metadata count wrong");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeWriteRejectsNullSourceWithoutMutation() {
    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    const SerializeSnapshot before_writer = writer.Snapshot();
    WorldComponentAttachmentSnapshotBridge bridge;
    const WorldComponentAttachmentSnapshotResult result = bridge.WriteSnapshot(&writer, nullptr);
    if (result.status != WorldComponentAttachmentSnapshotStatus::InvalidSourceBridge) {
        return Fail("component attachment snapshot null source status wrong");
    }

    const SerializeSnapshot after_writer = writer.Snapshot();
    if (!SerializeSnapshotsMatch(before_writer, after_writer)) {
        return Fail("component attachment snapshot null source mutated writer");
    }

    if (bridge.Snapshot().failed_operation_count != 1U) {
        return Fail("component attachment snapshot null source failure count wrong");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeWriteRejectsNullWriterWithoutMutation() {
    WorldComponentAttachmentBridge source_bridge;
    if (AddComponentAttachment(
            source_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component attachment snapshot null writer add failed") != 0) {
        return 1;
    }

    const WorldComponentAttachmentSnapshot before_source = source_bridge.Snapshot();
    WorldComponentAttachmentSnapshotBridge bridge;
    const WorldComponentAttachmentSnapshotResult result = bridge.WriteSnapshot(nullptr, &source_bridge);
    if (result.status != WorldComponentAttachmentSnapshotStatus::InvalidWriter) {
        return Fail("component attachment snapshot null writer status wrong");
    }

    const WorldComponentAttachmentSnapshot after_source = source_bridge.Snapshot();
    if (!ComponentAttachmentSnapshotsMatch(before_source, after_source)) {
        return Fail("component attachment snapshot null writer mutated source");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeWriteRejectsWriterOverflowWithoutOverrun() {
    WorldComponentAttachmentBridge source_bridge;
    if (AddComponentAttachment(
            source_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component attachment snapshot writer overflow add failed") != 0) {
        return 1;
    }

    std::array<std::uint8_t, STREAM_HEADER_BYTE_COUNT + 1U> buffer{};
    buffer[STREAM_HEADER_BYTE_COUNT] = 0xABU;
    SerializeWriter writer(buffer.data(), STREAM_HEADER_BYTE_COUNT);
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    WorldComponentAttachmentSnapshotBridge bridge;
    const WorldComponentAttachmentSnapshotResult result = bridge.WriteSnapshot(&writer, &source_bridge);
    if (result.status != WorldComponentAttachmentSnapshotStatus::SerializeFailure) {
        return Fail("component attachment snapshot writer overflow status wrong");
    }

    if (result.serialize_status != SerializeStatus::BufferTooSmall) {
        return Fail("component attachment snapshot writer overflow serialize status wrong");
    }

    if (buffer[STREAM_HEADER_BYTE_COUNT] != 0xABU) {
        return Fail("component attachment snapshot writer overflow overran buffer");
    }

    if (bridge.Snapshot().failed_operation_count != 1U) {
        return Fail("component attachment snapshot writer overflow failure count wrong");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeReadRestoresAttachmentRecords() {
    WorldComponentAttachmentBridge source_bridge;
    if (AddComponentAttachment(
            source_bridge,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            "component attachment snapshot read restore source add failed") != 0) {
        return 1;
    }

    SerializeBuffer buffer{};
    std::uint32_t committed_byte_count = 0U;
    WorldComponentAttachmentSnapshotBridge bridge;
    if (WriteComponentAttachmentSnapshotToBuffer(
            bridge,
            source_bridge,
            buffer,
            committed_byte_count) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge destination_bridge;
    SerializeReader reader(buffer.data(), committed_byte_count);
    const WorldComponentAttachmentSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        &destination_bridge);
    if (!result.Succeeded()) {
        return Fail("component attachment snapshot read restore failed");
    }

    const WorldComponentAttachmentResult query_result = destination_bridge.Query(
        OBJECT_CAMERA,
        COMPONENT_TYPE_SECONDARY);
    if (!query_result.Succeeded()) {
        return Fail("component attachment snapshot read restore query failed");
    }

    if (query_result.component_slot_id.value != COMPONENT_SLOT_SECONDARY.value) {
        return Fail("component attachment snapshot read restore slot wrong");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeReadZeroRecordStreamClearsDestination() {
    WorldComponentAttachmentBridge source_bridge;
    SerializeBuffer buffer{};
    std::uint32_t committed_byte_count = 0U;
    WorldComponentAttachmentSnapshotBridge bridge;
    if (WriteComponentAttachmentSnapshotToBuffer(
            bridge,
            source_bridge,
            buffer,
            committed_byte_count) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge destination_bridge;
    if (AddComponentAttachment(
            destination_bridge,
            OBJECT_EFFECT,
            COMPONENT_TYPE_TERTIARY,
            COMPONENT_SLOT_TERTIARY,
            "component attachment snapshot zero read destination add failed") != 0) {
        return 1;
    }

    SerializeReader reader(buffer.data(), committed_byte_count);
    const WorldComponentAttachmentSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        &destination_bridge);
    if (!result.Succeeded()) {
        return Fail("component attachment snapshot zero read failed");
    }

    const WorldComponentAttachmentSnapshot destination_snapshot = destination_bridge.Snapshot();
    if (destination_snapshot.active_attachment_count != 0U) {
        return Fail("component attachment snapshot zero read did not clear destination");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeReadRejectsNullReaderWithoutMutation() {
    WorldComponentAttachmentBridge destination_bridge;
    if (AddComponentAttachment(
            destination_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component attachment snapshot null reader destination add failed") != 0) {
        return 1;
    }

    const WorldComponentAttachmentSnapshot before_destination = destination_bridge.Snapshot();
    WorldComponentAttachmentSnapshotBridge bridge;
    const WorldComponentAttachmentSnapshotResult result = bridge.ReadSnapshot(nullptr, &destination_bridge);
    if (result.status != WorldComponentAttachmentSnapshotStatus::InvalidReader) {
        return Fail("component attachment snapshot null reader status wrong");
    }

    const WorldComponentAttachmentSnapshot after_destination = destination_bridge.Snapshot();
    if (!ComponentAttachmentSnapshotsMatch(before_destination, after_destination)) {
        return Fail("component attachment snapshot null reader mutated destination");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeReadRejectsNullDestinationWithoutMutation() {
    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    if (WriteComponentAttachmentSnapshotMetadata(
            writer,
            WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_SCHEMA_VERSION,
            0U,
            0U) != 0) {
        return 1;
    }

    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    WorldComponentAttachmentSnapshotBridge bridge;
    const WorldComponentAttachmentSnapshotResult result = bridge.ReadSnapshot(&reader, nullptr);
    if (result.status != WorldComponentAttachmentSnapshotStatus::InvalidDestinationBridge) {
        return Fail("component attachment snapshot null destination status wrong");
    }

    if (bridge.Snapshot().failed_operation_count != 1U) {
        return Fail("component attachment snapshot null destination failure count wrong");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeReadRejectsUnknownVersionWithoutMutation() {
    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    if (WriteComponentAttachmentSnapshotMetadata(writer, 999U, 0U, 0U) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge destination_bridge;
    if (AddComponentAttachment(
            destination_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component attachment snapshot unknown version destination add failed") != 0) {
        return 1;
    }

    const WorldComponentAttachmentSnapshot before_destination = destination_bridge.Snapshot();
    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    WorldComponentAttachmentSnapshotBridge bridge;
    const WorldComponentAttachmentSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        &destination_bridge);
    if (result.status != WorldComponentAttachmentSnapshotStatus::UnsupportedVersion) {
        return Fail("component attachment snapshot unknown version status wrong");
    }

    const WorldComponentAttachmentSnapshot after_destination = destination_bridge.Snapshot();
    if (!ComponentAttachmentSnapshotsMatch(before_destination, after_destination)) {
        return Fail("component attachment snapshot unknown version mutated destination");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeReadRejectsMalformedRecordCountWithoutMutation() {
    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    if (WriteComponentAttachmentSnapshotMetadata(
            writer,
            WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_SCHEMA_VERSION,
            1U,
            0U) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge destination_bridge;
    if (AddComponentAttachment(
            destination_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component attachment snapshot malformed destination add failed") != 0) {
        return 1;
    }

    const WorldComponentAttachmentSnapshot before_destination = destination_bridge.Snapshot();
    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    WorldComponentAttachmentSnapshotBridge bridge;
    const WorldComponentAttachmentSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        &destination_bridge);
    if (result.status != WorldComponentAttachmentSnapshotStatus::MalformedRecordCount) {
        return Fail("component attachment snapshot malformed count status wrong");
    }

    const WorldComponentAttachmentSnapshot after_destination = destination_bridge.Snapshot();
    if (!ComponentAttachmentSnapshotsMatch(before_destination, after_destination)) {
        return Fail("component attachment snapshot malformed count mutated destination");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeReadRejectsInvalidWorldIdWithoutMutation() {
    WorldComponentAttachmentSnapshotRecord record{};
    record.world_object_id = WorldObjectId{};
    record.component_type_id = COMPONENT_TYPE_PRIMARY;
    record.component_slot_id = COMPONENT_SLOT_PRIMARY;
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> records{record};

    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    if (WriteComponentAttachmentSnapshotFixtureStream(writer, records.data(), 1U) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge destination_bridge;
    if (AddComponentAttachment(
            destination_bridge,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            "component attachment snapshot invalid world destination add failed") != 0) {
        return 1;
    }

    const WorldComponentAttachmentSnapshot before_destination = destination_bridge.Snapshot();
    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    WorldComponentAttachmentSnapshotBridge bridge;
    const WorldComponentAttachmentSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        &destination_bridge);
    if (result.status != WorldComponentAttachmentSnapshotStatus::InvalidWorldObjectId) {
        return Fail("component attachment snapshot invalid world status wrong");
    }

    const WorldComponentAttachmentSnapshot after_destination = destination_bridge.Snapshot();
    if (!ComponentAttachmentSnapshotsMatch(before_destination, after_destination)) {
        return Fail("component attachment snapshot invalid world mutated destination");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeReadRejectsInvalidComponentTypeWithoutMutation() {
    WorldComponentAttachmentSnapshotRecord record{};
    record.world_object_id = OBJECT_PLAYER;
    record.component_type_id = WorldComponentTypeId{};
    record.component_slot_id = COMPONENT_SLOT_PRIMARY;
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> records{record};

    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    if (WriteComponentAttachmentSnapshotFixtureStream(writer, records.data(), 1U) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge destination_bridge;
    if (AddComponentAttachment(
            destination_bridge,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            "component attachment snapshot invalid type destination add failed") != 0) {
        return 1;
    }

    const WorldComponentAttachmentSnapshot before_destination = destination_bridge.Snapshot();
    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    WorldComponentAttachmentSnapshotBridge bridge;
    const WorldComponentAttachmentSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        &destination_bridge);
    if (result.status != WorldComponentAttachmentSnapshotStatus::InvalidComponentTypeId) {
        return Fail("component attachment snapshot invalid type status wrong");
    }

    const WorldComponentAttachmentSnapshot after_destination = destination_bridge.Snapshot();
    if (!ComponentAttachmentSnapshotsMatch(before_destination, after_destination)) {
        return Fail("component attachment snapshot invalid type mutated destination");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeReadRejectsInvalidComponentSlotWithoutMutation() {
    WorldComponentAttachmentSnapshotRecord record{};
    record.world_object_id = OBJECT_PLAYER;
    record.component_type_id = COMPONENT_TYPE_PRIMARY;
    record.component_slot_id = WorldComponentSlotId{};
    std::array<WorldComponentAttachmentSnapshotRecord, 1U> records{record};

    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    if (WriteComponentAttachmentSnapshotFixtureStream(writer, records.data(), 1U) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge destination_bridge;
    if (AddComponentAttachment(
            destination_bridge,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            "component attachment snapshot invalid slot destination add failed") != 0) {
        return 1;
    }

    const WorldComponentAttachmentSnapshot before_destination = destination_bridge.Snapshot();
    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    WorldComponentAttachmentSnapshotBridge bridge;
    const WorldComponentAttachmentSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        &destination_bridge);
    if (result.status != WorldComponentAttachmentSnapshotStatus::InvalidComponentSlotId) {
        return Fail("component attachment snapshot invalid slot status wrong");
    }

    const WorldComponentAttachmentSnapshot after_destination = destination_bridge.Snapshot();
    if (!ComponentAttachmentSnapshotsMatch(before_destination, after_destination)) {
        return Fail("component attachment snapshot invalid slot mutated destination");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeReadRejectsDuplicateAttachmentWithoutMutation() {
    WorldComponentAttachmentSnapshotRecord first_record{};
    first_record.world_object_id = OBJECT_PLAYER;
    first_record.component_type_id = COMPONENT_TYPE_PRIMARY;
    first_record.component_slot_id = COMPONENT_SLOT_PRIMARY;

    WorldComponentAttachmentSnapshotRecord second_record{};
    second_record.world_object_id = OBJECT_PLAYER;
    second_record.component_type_id = COMPONENT_TYPE_PRIMARY;
    second_record.component_slot_id = COMPONENT_SLOT_SECONDARY;
    std::array<WorldComponentAttachmentSnapshotRecord, 2U> records{first_record, second_record};

    SerializeBuffer buffer{};
    SerializeWriter writer(buffer.data(), static_cast<std::uint32_t>(buffer.size()));
    if (BeginSerializeStream(writer) != 0) {
        return 1;
    }

    if (WriteComponentAttachmentSnapshotFixtureStream(writer, records.data(), 2U) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge destination_bridge;
    if (AddComponentAttachment(
            destination_bridge,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            "component attachment snapshot duplicate destination add failed") != 0) {
        return 1;
    }

    const WorldComponentAttachmentSnapshot before_destination = destination_bridge.Snapshot();
    SerializeReader reader(buffer.data(), writer.Snapshot().committed_byte_count);
    WorldComponentAttachmentSnapshotBridge bridge;
    const WorldComponentAttachmentSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        &destination_bridge);
    if (result.status != WorldComponentAttachmentSnapshotStatus::DuplicateAttachment) {
        return Fail("component attachment snapshot duplicate status wrong");
    }

    const WorldComponentAttachmentSnapshot after_destination = destination_bridge.Snapshot();
    if (!ComponentAttachmentSnapshotsMatch(before_destination, after_destination)) {
        return Fail("component attachment snapshot duplicate mutated destination");
    }

    const WorldComponentAttachmentSnapshotBridgeSnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.rejected_record_count != 1U) {
        return Fail("component attachment snapshot duplicate rejection count wrong");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeWriteReadPathDoesNotGrowStorage() {
    WorldComponentAttachmentBridge source_bridge;
    if (AddComponentAttachment(
            source_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component attachment snapshot path first add failed") != 0) {
        return 1;
    }

    if (AddComponentAttachment(
            source_bridge,
            OBJECT_EFFECT,
            COMPONENT_TYPE_TERTIARY,
            COMPONENT_SLOT_TERTIARY,
            "component attachment snapshot path second add failed") != 0) {
        return 1;
    }

    WorldComponentAttachmentSnapshotBridgeDesc desc{};
    desc.attachment_capacity = 2U;
    WorldComponentAttachmentSnapshotBridge bridge(desc);
    WorldComponentAttachmentBridge destination_bridge;
    const WorldComponentAttachmentSnapshotBridgeSnapshot before_snapshot = bridge.Snapshot();
    std::uint32_t iteration = 0U;
    while (iteration < 3U) {
        SerializeBuffer buffer{};
        std::uint32_t committed_byte_count = 0U;
        if (WriteComponentAttachmentSnapshotToBuffer(
                bridge,
                source_bridge,
                buffer,
                committed_byte_count) != 0) {
            return 1;
        }

        SerializeReader reader(buffer.data(), committed_byte_count);
        const WorldComponentAttachmentSnapshotResult read_result = bridge.ReadSnapshot(
            &reader,
            &destination_bridge);
        if (!read_result.Succeeded()) {
            return Fail("component attachment snapshot path read failed");
        }

        ++iteration;
    }

    const WorldComponentAttachmentSnapshotBridgeSnapshot after_snapshot = bridge.Snapshot();
    if (after_snapshot.attachment_capacity != before_snapshot.attachment_capacity) {
        return Fail("component attachment snapshot path changed capacity");
    }

    if (after_snapshot.allocation_accounting_status != before_snapshot.allocation_accounting_status) {
        return Fail("component attachment snapshot path changed allocation accounting");
    }

    if (after_snapshot.write_count != 3U) {
        return Fail("component attachment snapshot path write count wrong");
    }

    if (after_snapshot.read_count != 3U) {
        return Fail("component attachment snapshot path read count wrong");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeSnapshotReportsCountsAndLastStatus() {
    WorldComponentAttachmentBridge source_bridge;
    if (AddComponentAttachment(
            source_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component attachment snapshot counters first add failed") != 0) {
        return 1;
    }

    if (AddComponentAttachment(
            source_bridge,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            "component attachment snapshot counters second add failed") != 0) {
        return 1;
    }

    WorldComponentAttachmentSnapshotBridge bridge;
    SerializeBuffer buffer{};
    std::uint32_t committed_byte_count = 0U;
    if (WriteComponentAttachmentSnapshotToBuffer(
            bridge,
            source_bridge,
            buffer,
            committed_byte_count) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge destination_bridge;
    SerializeReader reader(buffer.data(), committed_byte_count);
    const WorldComponentAttachmentSnapshotResult read_result = bridge.ReadSnapshot(
        &reader,
        &destination_bridge);
    if (!read_result.Succeeded()) {
        return Fail("component attachment snapshot counters read failed");
    }

    const WorldComponentAttachmentSnapshotResult failure_result = bridge.ReadSnapshot(
        nullptr,
        &destination_bridge);
    if (failure_result.status != WorldComponentAttachmentSnapshotStatus::InvalidReader) {
        return Fail("component attachment snapshot counters failure status wrong");
    }

    const WorldComponentAttachmentSnapshotBridgeSnapshot snapshot = bridge.Snapshot();
    if (snapshot.write_count != 1U) {
        return Fail("component attachment snapshot counters write count wrong");
    }

    if (snapshot.read_count != 1U) {
        return Fail("component attachment snapshot counters read count wrong");
    }

    if (snapshot.written_record_count != 2U) {
        return Fail("component attachment snapshot counters written count wrong");
    }

    if (snapshot.read_record_count != 2U) {
        return Fail("component attachment snapshot counters read record count wrong");
    }

    if (snapshot.failed_operation_count != 1U) {
        return Fail("component attachment snapshot counters failure count wrong");
    }

    if (snapshot.last_status != WorldComponentAttachmentSnapshotStatus::InvalidReader) {
        return Fail("component attachment snapshot counters last status wrong");
    }

    if (snapshot.last_serialize_status != SerializeStatus::Success) {
        return Fail("component attachment snapshot counters serialize status wrong");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeNoActorComponentPayloadOrLifecycle() {
    WorldComponentAttachmentBridge source_bridge;
    if (AddComponentAttachment(
            source_bridge,
            OBJECT_EFFECT,
            COMPONENT_TYPE_TERTIARY,
            COMPONENT_SLOT_TERTIARY,
            "component attachment snapshot payload boundary add failed") != 0) {
        return 1;
    }

    WorldComponentAttachmentSnapshotBridge bridge;
    SerializeBuffer buffer{};
    std::uint32_t committed_byte_count = 0U;
    if (WriteComponentAttachmentSnapshotToBuffer(
            bridge,
            source_bridge,
            buffer,
            committed_byte_count) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge destination_bridge;
    SerializeReader reader(buffer.data(), committed_byte_count);
    const WorldComponentAttachmentSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        &destination_bridge);
    if (!result.Succeeded()) {
        return Fail("component attachment snapshot payload boundary read failed");
    }

    const WorldComponentAttachmentResult query_result = destination_bridge.Query(
        OBJECT_EFFECT,
        COMPONENT_TYPE_TERTIARY);
    if (!query_result.Succeeded()) {
        return Fail("component attachment snapshot payload boundary query failed");
    }

    if (query_result.component_slot_id.value != COMPONENT_SLOT_TERTIARY.value) {
        return Fail("component attachment snapshot payload boundary slot wrong");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeNoObjectResourceScriptFilePackageOrGameAdapterDependency() {
    WorldComponentAttachmentBridge source_bridge;
    if (AddComponentAttachment(
            source_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component attachment snapshot module boundary add failed") != 0) {
        return 1;
    }

    WorldComponentAttachmentSnapshotBridge bridge;
    SerializeBuffer buffer{};
    std::uint32_t committed_byte_count = 0U;
    if (WriteComponentAttachmentSnapshotToBuffer(
            bridge,
            source_bridge,
            buffer,
            committed_byte_count) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge destination_bridge;
    SerializeReader reader(buffer.data(), committed_byte_count);
    const WorldComponentAttachmentSnapshotResult result = bridge.ReadSnapshot(
        &reader,
        &destination_bridge);
    if (!result.Succeeded()) {
        return Fail("component attachment snapshot module boundary read failed");
    }

    const WorldComponentAttachmentSnapshotBridgeSnapshot snapshot = bridge.Snapshot();
    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("component attachment snapshot module boundary allocation accounting wrong");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeNoRenderPhysicsAudioInputUiToolOrReportDependency() {
    WorldComponentAttachmentBridge source_bridge;
    if (AddComponentAttachment(
            source_bridge,
            OBJECT_CAMERA,
            COMPONENT_TYPE_SECONDARY,
            COMPONENT_SLOT_SECONDARY,
            "component attachment snapshot render boundary add failed") != 0) {
        return 1;
    }

    WorldComponentAttachmentSnapshotBridge bridge;
    SerializeBuffer buffer{};
    std::uint32_t committed_byte_count = 0U;
    if (WriteComponentAttachmentSnapshotToBuffer(
            bridge,
            source_bridge,
            buffer,
            committed_byte_count) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge destination_bridge;
    SerializeReader reader(buffer.data(), committed_byte_count);
    if (!bridge.ReadSnapshot(&reader, &destination_bridge).Succeeded()) {
        return Fail("component attachment snapshot render boundary read failed");
    }

    const WorldComponentAttachmentSnapshotBridgeSnapshot snapshot = bridge.Snapshot();
    if (snapshot.write_count != 1U) {
        return Fail("component attachment snapshot render boundary write count wrong");
    }

    if (snapshot.read_count != 1U) {
        return Fail("component attachment snapshot render boundary read count wrong");
    }

    return 0;
}

int WorldComponentAttachmentSnapshotBridgeWorldInstanceCoreRemainsSnapshotFree() {
    WorldInstance world = MakeWorld(4U, 4U);
    if (!Register(world, OBJECT_PLAYER).Succeeded()) {
        return Fail("component attachment snapshot world core-free registration failed");
    }

    WorldComponentAttachmentBridge source_bridge;
    if (AddComponentAttachment(
            source_bridge,
            OBJECT_PLAYER,
            COMPONENT_TYPE_PRIMARY,
            COMPONENT_SLOT_PRIMARY,
            "component attachment snapshot world core-free add failed") != 0) {
        return 1;
    }

    const WorldSnapshot before_world = world.Snapshot();
    WorldComponentAttachmentSnapshotBridge bridge;
    SerializeBuffer buffer{};
    std::uint32_t committed_byte_count = 0U;
    if (WriteComponentAttachmentSnapshotToBuffer(
            bridge,
            source_bridge,
            buffer,
            committed_byte_count) != 0) {
        return 1;
    }

    WorldComponentAttachmentBridge destination_bridge;
    SerializeReader reader(buffer.data(), committed_byte_count);
    if (!bridge.ReadSnapshot(&reader, &destination_bridge).Succeeded()) {
        return Fail("component attachment snapshot world core-free read failed");
    }

    const WorldSnapshot after_world = world.Snapshot();
    if (!WorldSnapshotsMatch(before_world, after_world)) {
        return Fail("component attachment snapshot bridge mutated world");
    }

    return 0;
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_CREATE, WorldCreateWithFixedCapacityReportsSnapshot},
        {TEST_START_STOP, WorldStartStopRunsDeterministicLifecycle},
        {TEST_PHASE_ORDER, WorldUpdateRunsPhasesInFixedOrder},
        {TEST_UPDATE_BEFORE_START, WorldUpdateBeforeStartReturnsExplicitStatus},
        {TEST_UPDATE_AFTER_STOP, WorldUpdateAfterStopReturnsExplicitStatus},
        {TEST_DUPLICATE, WorldRegisterDuplicateObjectDoesNotMutate},
        {TEST_OVERFLOW, WorldRegisterOverflowDoesNotMutate},
        {TEST_DISABLED_SKIP, WorldDisabledObjectIsSkipped},
        {TEST_UPDATE_PATH, WorldUpdatePathDoesNotGrowStorage},
        {TEST_STOP_CLEARS, WorldStopClearsActiveEntries},
        {TEST_NO_SCRIPT_RESOURCE, WorldNoScriptResourcePackageFileOrGameAdapterDependency},
        {TEST_NO_ACTOR_COMPONENT, WorldNoActorComponentOrTransformHierarchy},
        {TEST_SNAPSHOT, WorldSnapshotReportsCountsAndLastStatus},
        {TEST_MODULE_START_SERVICE, WorldKernelModuleStartPublishesWorldService},
        {TEST_MODULE_UPDATE_ORDER, WorldKernelModuleUpdateTicksWorldInKernelOrder},
        {TEST_MODULE_SHUTDOWN, WorldKernelModuleShutdownStopsWorld},
        {TEST_MODULE_START_FAILURE, WorldKernelModuleStartFailurePropagatesExplicitStatus},
        {TEST_MODULE_UPDATE_FAILURE, WorldKernelModuleUpdateFailureTriggersKernelTeardown},
        {TEST_MODULE_HEADLESS_HOST, WorldKernelModuleHeadlessHostDrivesWorldDeterministically},
        {TEST_MODULE_UPDATE_PATH, WorldKernelModuleUpdatePathDoesNotGrowWorldStorage},
        {TEST_MODULE_NO_SCRIPT_RESOURCE, WorldKernelModuleNoScriptResourcePackageFileOrGameAdapterDependency},
        {TEST_MODULE_NO_ACTOR_COMPONENT, WorldKernelModuleNoActorComponentOrTransformHierarchy},
        {TEST_MODULE_CORE_KERNEL_FREE, WorldKernelModuleCoreWorldInstanceRemainsKernelFree},
        {TEST_IDENTITY_BIND_VALID, WorldObjectIdentityBridgeBindValidObjectAcquiresHandle},
        {TEST_IDENTITY_INVALID_WORLD_ID, WorldObjectIdentityBridgeBindRejectsInvalidWorldIdWithoutMutation},
        {TEST_IDENTITY_MISSING_WORLD_OBJECT, WorldObjectIdentityBridgeBindRejectsMissingWorldObjectWithoutMutation},
        {TEST_IDENTITY_INVALID_OBJECT_HANDLE, WorldObjectIdentityBridgeBindRejectsInvalidObjectHandleWithoutMutation},
        {TEST_IDENTITY_DUPLICATE_WORLD_ID, WorldObjectIdentityBridgeBindRejectsDuplicateWorldObjectId},
        {TEST_IDENTITY_DUPLICATE_OBJECT_HANDLE, WorldObjectIdentityBridgeBindRejectsDuplicateObjectHandle},
        {TEST_IDENTITY_REMOVE_RELEASES, WorldObjectIdentityBridgeRemoveReleasesHandle},
        {TEST_IDENTITY_CLEAR_RELEASES, WorldObjectIdentityBridgeClearReleasesAllHandles},
        {TEST_IDENTITY_STALE_GENERATION, WorldObjectIdentityBridgeStaleGenerationInvalidatesBinding},
        {TEST_IDENTITY_UPDATE_PATH, WorldObjectIdentityBridgeUpdatePathDoesNotGrowWorldStorage},
        {TEST_IDENTITY_NO_SCRIPT_RESOURCE, WorldObjectIdentityBridgeNoScriptResourcePackageFileOrGameAdapterDependency},
        {TEST_IDENTITY_NO_ACTOR_COMPONENT, WorldObjectIdentityBridgeNoActorComponentOrTransformHierarchy},
        {TEST_IDENTITY_CORE_OBJECT_FREE, WorldObjectIdentityBridgeWorldInstanceCoreRemainsObjectFree},
        {TEST_TRANSFORM_REGISTER_VALID, WorldTransformBridgeRegisterValidObjectStoresTransform},
        {TEST_TRANSFORM_INVALID_WORLD_ID, WorldTransformBridgeRegisterRejectsInvalidWorldIdWithoutMutation},
        {TEST_TRANSFORM_MISSING_WORLD_OBJECT, WorldTransformBridgeRegisterRejectsMissingWorldObjectWithoutMutation},
        {TEST_TRANSFORM_DUPLICATE_WORLD_ID, WorldTransformBridgeRegisterRejectsDuplicateWorldObjectId},
        {TEST_TRANSFORM_CAPACITY_OVERFLOW, WorldTransformBridgeRegisterRejectsCapacityOverflowWithoutMutation},
        {TEST_TRANSFORM_SET_EXISTING, WorldTransformBridgeSetUpdatesExistingRecord},
        {TEST_TRANSFORM_SET_MISSING, WorldTransformBridgeSetRejectsMissingRecordWithoutMutation},
        {TEST_TRANSFORM_QUERY, WorldTransformBridgeQueryReturnsStoredTransform},
        {TEST_TRANSFORM_REMOVE, WorldTransformBridgeRemoveClearsRecord},
        {TEST_TRANSFORM_CLEAR, WorldTransformBridgeClearRemovesAllRecords},
        {TEST_TRANSFORM_UPDATE_PATH, WorldTransformBridgeUpdatePathDoesNotGrowWorldStorage},
        {TEST_TRANSFORM_NO_SCRIPT_RESOURCE, WorldTransformBridgeNoScriptResourcePackageFileObjectOrGameAdapterDependency},
        {TEST_TRANSFORM_NO_ACTOR_COMPONENT, WorldTransformBridgeNoActorComponentSceneGraphOrHierarchy},
        {TEST_TRANSFORM_CORE_FREE, WorldTransformBridgeWorldInstanceCoreRemainsTransformStorageFree},
        {TEST_SCRIPT_DISPATCH_BIND_VALID, WorldScriptDispatchBridgeBindPhaseCallReturnsStableBinding},
        {TEST_SCRIPT_DISPATCH_INVALID_CALL, WorldScriptDispatchBridgeBindRejectsInvalidCallIdWithoutMutation},
        {TEST_SCRIPT_DISPATCH_DUPLICATE_PHASE, WorldScriptDispatchBridgeBindRejectsDuplicatePhaseWithoutMutation},
        {TEST_SCRIPT_DISPATCH_CAPACITY, WorldScriptDispatchBridgeBindRejectsCapacityOverflowWithoutMutation},
        {TEST_SCRIPT_DISPATCH_ORDER, WorldScriptDispatchBridgeDispatchTraceInvokesPhasesInTraceOrder},
        {TEST_SCRIPT_DISPATCH_SKIP, WorldScriptDispatchBridgeDispatchSkipsUnboundPhase},
        {TEST_SCRIPT_DISPATCH_TRACE_BUFFER, WorldScriptDispatchBridgeDispatchRejectsInvalidTraceBuffer},
        {TEST_SCRIPT_DISPATCH_SLOT_BUFFERS, WorldScriptDispatchBridgeDispatchRejectsInvalidSlotBuffers},
        {TEST_SCRIPT_DISPATCH_SCRIPT_FAILURE, WorldScriptDispatchBridgeDispatchPropagatesScriptFailure},
        {TEST_SCRIPT_DISPATCH_PATH, WorldScriptDispatchBridgeDispatchPathDoesNotGrowStorage},
        {TEST_SCRIPT_DISPATCH_SNAPSHOT, WorldScriptDispatchBridgeSnapshotReportsCountsAndLastStatus},
        {TEST_SCRIPT_DISPATCH_NO_ACTOR_COMPONENT, WorldScriptDispatchBridgeNoActorComponentSceneGraphOrGameAdapterDependency},
        {TEST_SCRIPT_DISPATCH_NO_RESOURCE_OBJECT, WorldScriptDispatchBridgeNoResourcePackageFileSerializeOrObjectOwnershipDependency},
        {TEST_SCRIPT_DISPATCH_WORLD_CORE_FREE, WorldScriptDispatchBridgeWorldInstanceCoreRemainsScriptFree},
        {TEST_SCRIPT_DISPATCH_SCRIPT_CORE_FREE, WorldScriptDispatchBridgeScriptRegistryCoreRemainsWorldFree},
        {TEST_SERIALIZE_ROUND_TRIP, WorldSerializeSnapshotBridgeWriteWorldSnapshotRoundTripsDeterministically},
        {TEST_SERIALIZE_TRACE_ORDER, WorldSerializeSnapshotBridgeWritePhaseTraceRecordsInOrder},
        {TEST_SERIALIZE_TRANSFORM, WorldSerializeSnapshotBridgeWriteOptionalTransformSnapshotCounters},
        {TEST_SERIALIZE_SMALL_TRACE_OUTPUT, WorldSerializeSnapshotBridgeReadRejectsSmallTraceOutputWithoutOverrun},
        {TEST_SERIALIZE_INVALID_TRACE_BUFFER, WorldSerializeSnapshotBridgeWriteRejectsInvalidTraceBufferWithoutMutation},
        {TEST_SERIALIZE_TRACE_OVERFLOW, WorldSerializeSnapshotBridgeWriteRejectsTraceOverflowWithoutMutation},
        {TEST_SERIALIZE_WRITE_FAILURE, WorldSerializeSnapshotBridgeSerializeFailureMapsExplicitStatus},
        {TEST_SERIALIZE_READ_FAILURE, WorldSerializeSnapshotBridgeReadFailureMapsExplicitStatus},
        {TEST_SERIALIZE_INVALID_ENUM, WorldSerializeSnapshotBridgeReadRejectsInvalidEnumValuesWithoutMutation},
        {TEST_SERIALIZE_NO_WORLD_MUTATION, WorldSerializeSnapshotBridgeNoWorldMutationDuringReadWrite},
        {TEST_SERIALIZE_PATH, WorldSerializeSnapshotBridgeReadWritePathDoesNotGrowStorage},
        {TEST_SERIALIZE_SNAPSHOT, WorldSerializeSnapshotBridgeSnapshotReportsCountsAndLastStatus},
        {TEST_SERIALIZE_NO_FILE_PACKAGE, WorldSerializeSnapshotBridgeNoFilePackageResourceSaveGameOrGameAdapterDependency},
        {TEST_SERIALIZE_NO_ACTOR_COMPONENT, WorldSerializeSnapshotBridgeNoActorComponentSceneGraphOrGameplayDependency},
        {TEST_SERIALIZE_WORLD_CORE_FREE, WorldSerializeSnapshotBridgeWorldInstanceCoreRemainsSerializeFree},
        {TEST_SERIALIZE_CORE_FREE, WorldSerializeSnapshotBridgeSerializeCoreRemainsWorldFree},
        {TEST_RESOURCE_BIND_VALID, WorldResourceBindingBridgeBindValidResourceAcquiresHandle},
        {TEST_RESOURCE_BIND_NULL_REGISTRY, WorldResourceBindingBridgeBindRejectsNullRegistryWithoutMutation},
        {TEST_RESOURCE_BIND_INVALID_WORLD, WorldResourceBindingBridgeBindRejectsInvalidWorldIdWithoutMutation},
        {TEST_RESOURCE_BIND_INVALID_HANDLE, WorldResourceBindingBridgeBindRejectsInvalidResourceHandleWithoutMutation},
        {TEST_RESOURCE_BIND_STALE_HANDLE, WorldResourceBindingBridgeBindRejectsStaleResourceHandleWithoutMutation},
        {TEST_RESOURCE_BIND_TYPE_MISMATCH, WorldResourceBindingBridgeBindRejectsTypeMismatchWithoutMutation},
        {TEST_RESOURCE_BIND_DUPLICATE_WORLD, WorldResourceBindingBridgeBindRejectsDuplicateWorldObjectId},
        {TEST_RESOURCE_BIND_CAPACITY, WorldResourceBindingBridgeBindRejectsCapacityOverflowWithoutMutation},
        {TEST_RESOURCE_REMOVE_RELEASES, WorldResourceBindingBridgeRemoveReleasesHandle},
        {TEST_RESOURCE_REMOVE_NULL_REGISTRY, WorldResourceBindingBridgeRemoveRejectsNullRegistryWithoutMutation},
        {TEST_RESOURCE_REMOVE_MISSING, WorldResourceBindingBridgeRemoveRejectsMissingWorldObjectWithoutMutation},
        {TEST_RESOURCE_REMOVE_RELEASE_FAILURE, WorldResourceBindingBridgeRemoveReleaseFailureKeepsBinding},
        {TEST_RESOURCE_CLEAR_RELEASES, WorldResourceBindingBridgeClearReleasesAllHandles},
        {TEST_RESOURCE_CLEAR_NULL_REGISTRY, WorldResourceBindingBridgeClearRejectsNullRegistryWithoutMutation},
        {TEST_RESOURCE_CLEAR_RELEASE_FAILURE, WorldResourceBindingBridgeClearReleaseFailurePreservesUnreleasedBindings},
        {TEST_RESOURCE_RETIRE_HELD, WorldResourceBindingBridgeBoundResourceCannotRetireUntilReleased},
        {TEST_RESOURCE_QUERY, WorldResourceBindingBridgeQueryReturnsStoredBinding},
        {TEST_RESOURCE_QUERY_READ_ONLY, WorldResourceBindingBridgeQueryIsReadOnlyAndBounded},
        {TEST_RESOURCE_UPDATE_PATH, WorldResourceBindingBridgeUpdatePathDoesNotGrowWorldStorage},
        {TEST_RESOURCE_NO_WORLD_QUERY, WorldResourceBindingBridgeDoesNotQueryOrMutateWorldInstance},
        {TEST_RESOURCE_SNAPSHOT, WorldResourceBindingBridgeSnapshotReportsCountsAndLastStatus},
        {TEST_RESOURCE_NO_FILE_PACKAGE, WorldResourceBindingBridgeNoFilePackageLoadDecodeUploadOrGameAdapterDependency},
        {TEST_RESOURCE_WORLD_CORE_FREE, WorldResourceBindingBridgeWorldInstanceCoreRemainsResourceFree},
        {TEST_RESOURCE_CORE_FREE, WorldResourceBindingBridgeResourceCoreRemainsWorldFree},
        {TEST_COMPONENT_RESOURCE_BIND_VALID,
            WorldComponentResourceBindingBridgeBindValidAttachmentResourceAcquiresHandle},
        {TEST_COMPONENT_RESOURCE_BIND_NULL_ATTACHMENT,
            WorldComponentResourceBindingBridgeBindRejectsNullAttachmentSourceWithoutMutation},
        {TEST_COMPONENT_RESOURCE_BIND_NULL_REGISTRY,
            WorldComponentResourceBindingBridgeBindRejectsNullRegistryWithoutMutation},
        {TEST_COMPONENT_RESOURCE_BIND_INVALID_WORLD,
            WorldComponentResourceBindingBridgeBindRejectsInvalidWorldIdWithoutMutation},
        {TEST_COMPONENT_RESOURCE_BIND_INVALID_TYPE,
            WorldComponentResourceBindingBridgeBindRejectsInvalidComponentTypeWithoutMutation},
        {TEST_COMPONENT_RESOURCE_BIND_INVALID_SLOT,
            WorldComponentResourceBindingBridgeBindRejectsInvalidComponentSlotWithoutMutation},
        {TEST_COMPONENT_RESOURCE_BIND_MISSING_ATTACHMENT,
            WorldComponentResourceBindingBridgeBindRejectsMissingAttachmentWithoutMutation},
        {TEST_COMPONENT_RESOURCE_BIND_INVALID_HANDLE,
            WorldComponentResourceBindingBridgeBindRejectsInvalidResourceHandleWithoutMutation},
        {TEST_COMPONENT_RESOURCE_BIND_STALE_HANDLE,
            WorldComponentResourceBindingBridgeBindRejectsStaleResourceHandleWithoutMutation},
        {TEST_COMPONENT_RESOURCE_BIND_TYPE_MISMATCH,
            WorldComponentResourceBindingBridgeBindRejectsTypeMismatchWithoutMutation},
        {TEST_COMPONENT_RESOURCE_BIND_DUPLICATE,
            WorldComponentResourceBindingBridgeBindRejectsDuplicateComponentBinding},
        {TEST_COMPONENT_RESOURCE_BIND_CAPACITY,
            WorldComponentResourceBindingBridgeBindRejectsCapacityOverflowWithoutMutation},
        {TEST_COMPONENT_RESOURCE_REMOVE_RELEASES,
            WorldComponentResourceBindingBridgeRemoveReleasesHandle},
        {TEST_COMPONENT_RESOURCE_REMOVE_NULL_REGISTRY,
            WorldComponentResourceBindingBridgeRemoveRejectsNullRegistryWithoutMutation},
        {TEST_COMPONENT_RESOURCE_REMOVE_MISSING,
            WorldComponentResourceBindingBridgeRemoveRejectsMissingBindingWithoutMutation},
        {TEST_COMPONENT_RESOURCE_REMOVE_RELEASE_FAILURE,
            WorldComponentResourceBindingBridgeRemoveReleaseFailureKeepsBinding},
        {TEST_COMPONENT_RESOURCE_CLEAR_RELEASES,
            WorldComponentResourceBindingBridgeClearReleasesAllHandles},
        {TEST_COMPONENT_RESOURCE_CLEAR_NULL_REGISTRY,
            WorldComponentResourceBindingBridgeClearRejectsNullRegistryWithoutMutation},
        {TEST_COMPONENT_RESOURCE_CLEAR_RELEASE_FAILURE,
            WorldComponentResourceBindingBridgeClearReleaseFailurePreservesUnreleasedBindings},
        {TEST_COMPONENT_RESOURCE_RETIRE_HELD,
            WorldComponentResourceBindingBridgeBoundResourceCannotRetireUntilReleased},
        {TEST_COMPONENT_RESOURCE_QUERY,
            WorldComponentResourceBindingBridgeQueryReturnsStoredBinding},
        {TEST_COMPONENT_RESOURCE_QUERY_READ_ONLY,
            WorldComponentResourceBindingBridgeQueryIsReadOnlyAndBounded},
        {TEST_COMPONENT_RESOURCE_UPDATE_PATH,
            WorldComponentResourceBindingBridgeUpdatePathDoesNotGrowStorage},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT,
            WorldComponentResourceBindingBridgeSnapshotReportsCountsAndLastStatus},
        {TEST_COMPONENT_RESOURCE_NO_WORLD_QUERY,
            WorldComponentResourceBindingBridgeDoesNotQueryOrMutateWorldInstance},
        {TEST_COMPONENT_RESOURCE_NO_PAYLOAD,
            WorldComponentResourceBindingBridgeNoActorComponentPayloadOrLifecycle},
        {TEST_COMPONENT_RESOURCE_NO_FILE_PACKAGE,
            WorldComponentResourceBindingBridgeNoFilePackageLoadDecodeUploadOrGameAdapterDependency},
        {TEST_COMPONENT_RESOURCE_NO_RENDER_PHYSICS,
            WorldComponentResourceBindingBridgeNoRenderPhysicsAudioInputUiToolOrReportDependency},
        {TEST_COMPONENT_RESOURCE_WORLD_CORE_FREE,
            WorldComponentResourceBindingBridgeWorldInstanceCoreRemainsComponentResourceFree},
        {TEST_COMPONENT_RESOURCE_RESOURCE_CORE_FREE,
            WorldComponentResourceBindingBridgeResourceCoreRemainsWorldFree},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_ROUND_TRIP,
            WorldComponentResourceBindingSnapshotBridgeWriteReadRoundTripsBindingsInSlotOrder},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_EMPTY_WRITE,
            WorldComponentResourceBindingSnapshotBridgeWriteEmptyBridgeProducesZeroRecords},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_NULL_SOURCE,
            WorldComponentResourceBindingSnapshotBridgeWriteRejectsNullSourceWithoutMutation},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_NULL_WRITER,
            WorldComponentResourceBindingSnapshotBridgeWriteRejectsNullWriterWithoutMutation},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_WRITER_OVERFLOW,
            WorldComponentResourceBindingSnapshotBridgeWriteRejectsWriterOverflowWithoutOverrun},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_READ_OUTPUT,
            WorldComponentResourceBindingSnapshotBridgeReadWritesCallerOwnedRecords},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_NULL_READER,
            WorldComponentResourceBindingSnapshotBridgeReadRejectsNullReaderWithoutMutation},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_NULL_OUTPUT,
            WorldComponentResourceBindingSnapshotBridgeReadRejectsNullOutputWithoutMutation},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_SMALL_OUTPUT,
            WorldComponentResourceBindingSnapshotBridgeReadRejectsOutputCapacityTooSmallWithoutMutation},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_UNKNOWN_VERSION,
            WorldComponentResourceBindingSnapshotBridgeReadRejectsUnknownVersionWithoutMutation},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_MALFORMED_COUNT,
            WorldComponentResourceBindingSnapshotBridgeReadRejectsMalformedRecordCountWithoutMutation},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_INVALID_WORLD,
            WorldComponentResourceBindingSnapshotBridgeReadRejectsInvalidWorldIdWithoutMutation},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_INVALID_TYPE,
            WorldComponentResourceBindingSnapshotBridgeReadRejectsInvalidComponentTypeWithoutMutation},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_INVALID_SLOT,
            WorldComponentResourceBindingSnapshotBridgeReadRejectsInvalidComponentSlotWithoutMutation},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_INVALID_HANDLE,
            WorldComponentResourceBindingSnapshotBridgeReadRejectsInvalidResourceHandleWithoutMutation},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_INVALID_RESOURCE_TYPE,
            WorldComponentResourceBindingSnapshotBridgeReadRejectsInvalidResourceTypeWithoutMutation},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_DUPLICATE,
            WorldComponentResourceBindingSnapshotBridgeReadRejectsDuplicateBindingWithoutMutation},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_NO_ACQUIRE,
            WorldComponentResourceBindingSnapshotBridgeReadDoesNotAcquireOrReleaseResources},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_PATH,
            WorldComponentResourceBindingSnapshotBridgeWriteReadPathDoesNotGrowStorage},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_COUNTERS,
            WorldComponentResourceBindingSnapshotBridgeSnapshotReportsCountsAndLastStatus},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_NO_PAYLOAD,
            WorldComponentResourceBindingSnapshotBridgeNoActorComponentPayloadOrLifecycle},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_NO_FILE_PACKAGE,
            WorldComponentResourceBindingSnapshotBridgeNoFilePackageLoadDecodeUploadOrGameAdapterDependency},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_NO_RENDER_PHYSICS,
            WorldComponentResourceBindingSnapshotBridgeNoRenderPhysicsAudioInputUiToolOrReportDependency},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_WORLD_CORE_FREE,
            WorldComponentResourceBindingSnapshotBridgeWorldInstanceCoreRemainsSnapshotFree},
        {TEST_COMPONENT_RESOURCE_SNAPSHOT_RESOURCE_CORE_FREE,
            WorldComponentResourceBindingSnapshotBridgeResourceCoreRemainsWorldFree},
        {TEST_COMPONENT_RESOURCE_RESTORE_ORDER,
            WorldComponentResourceBindingRestoreBridgeRestoresRecordsInInputOrder},
        {TEST_COMPONENT_RESOURCE_RESTORE_EMPTY,
            WorldComponentResourceBindingRestoreBridgeRestoresEmptyInputWithoutMutation},
        {TEST_COMPONENT_RESOURCE_RESTORE_NULL_DESTINATION,
            WorldComponentResourceBindingRestoreBridgeRejectsNullDestinationWithoutMutation},
        {TEST_COMPONENT_RESOURCE_RESTORE_NULL_ATTACHMENT,
            WorldComponentResourceBindingRestoreBridgeRejectsNullAttachmentSourceWithoutMutation},
        {TEST_COMPONENT_RESOURCE_RESTORE_NULL_REGISTRY,
            WorldComponentResourceBindingRestoreBridgeRejectsNullRegistryWithoutMutation},
        {TEST_COMPONENT_RESOURCE_RESTORE_NULL_INPUT,
            WorldComponentResourceBindingRestoreBridgeRejectsNullInputWithoutMutation},
        {TEST_COMPONENT_RESOURCE_RESTORE_INVALID_WORLD,
            WorldComponentResourceBindingRestoreBridgeRejectsInvalidWorldIdWithoutMutation},
        {TEST_COMPONENT_RESOURCE_RESTORE_INVALID_TYPE,
            WorldComponentResourceBindingRestoreBridgeRejectsInvalidComponentTypeWithoutMutation},
        {TEST_COMPONENT_RESOURCE_RESTORE_INVALID_SLOT,
            WorldComponentResourceBindingRestoreBridgeRejectsInvalidComponentSlotWithoutMutation},
        {TEST_COMPONENT_RESOURCE_RESTORE_MISSING_ATTACHMENT,
            WorldComponentResourceBindingRestoreBridgeRejectsMissingAttachmentWithoutMutation},
        {TEST_COMPONENT_RESOURCE_RESTORE_INVALID_HANDLE,
            WorldComponentResourceBindingRestoreBridgeRejectsInvalidResourceHandleWithoutMutation},
        {TEST_COMPONENT_RESOURCE_RESTORE_STALE_HANDLE,
            WorldComponentResourceBindingRestoreBridgeRejectsStaleResourceHandleWithoutMutation},
        {TEST_COMPONENT_RESOURCE_RESTORE_TYPE_MISMATCH,
            WorldComponentResourceBindingRestoreBridgeRejectsResourceTypeMismatchWithoutMutation},
        {TEST_COMPONENT_RESOURCE_RESTORE_DUPLICATE,
            WorldComponentResourceBindingRestoreBridgeRejectsDuplicateInputWithoutMutation},
        {TEST_COMPONENT_RESOURCE_RESTORE_DESTINATION_CAPACITY,
            WorldComponentResourceBindingRestoreBridgeRejectsDestinationCapacityOverflowWithoutMutation},
        {TEST_COMPONENT_RESOURCE_RESTORE_NON_EMPTY_DESTINATION,
            WorldComponentResourceBindingRestoreBridgeRejectsNonEmptyDestinationWithoutMutation},
        {TEST_COMPONENT_RESOURCE_RESTORE_PREFLIGHT_ACQUIRE,
            WorldComponentResourceBindingRestoreBridgeAcquiresOnlyAfterPreflight},
        {TEST_COMPONENT_RESOURCE_RESTORE_ACQUIRE_FAILURE,
            WorldComponentResourceBindingRestoreBridgeResourceAcquireFailureDoesNotPartiallyRestore},
        {TEST_COMPONENT_RESOURCE_RESTORE_COUNTERS,
            WorldComponentResourceBindingRestoreBridgeSnapshotReportsCountsAndLastStatus},
        {TEST_COMPONENT_RESOURCE_RESTORE_NO_PAYLOAD,
            WorldComponentResourceBindingRestoreBridgeNoActorComponentPayloadOrLifecycle},
        {TEST_COMPONENT_RESOURCE_RESTORE_NO_FILE_PACKAGE,
            WorldComponentResourceBindingRestoreBridgeNoFilePackageLoadDecodeUploadOrGameAdapterDependency},
        {TEST_COMPONENT_RESOURCE_RESTORE_NO_RENDER_PHYSICS,
            WorldComponentResourceBindingRestoreBridgeNoRenderPhysicsAudioInputUiToolOrReportDependency},
        {TEST_COMPONENT_RESOURCE_RESTORE_WORLD_CORE_FREE,
            WorldComponentResourceBindingRestoreBridgeWorldInstanceCoreRemainsRestoreFree},
        {TEST_COMPONENT_RESOURCE_RESTORE_RESOURCE_CORE_FREE,
            WorldComponentResourceBindingRestoreBridgeResourceCoreRemainsWorldFree},
        {TEST_SCENE_ASSEMBLY_ORDER,
            WorldSceneAssemblyBridgeRestoresAttachmentAndBindingRecordsInInputOrder},
        {TEST_SCENE_ASSEMBLY_EMPTY,
            WorldSceneAssemblyBridgeRestoresEmptyAssemblyWithoutMutation},
        {TEST_SCENE_ASSEMBLY_NULL_ATTACHMENT_DESTINATION,
            WorldSceneAssemblyBridgeRejectsNullAttachmentDestinationWithoutMutation},
        {TEST_SCENE_ASSEMBLY_NULL_BINDING_DESTINATION,
            WorldSceneAssemblyBridgeRejectsNullBindingDestinationWithoutMutation},
        {TEST_SCENE_ASSEMBLY_NULL_REGISTRY,
            WorldSceneAssemblyBridgeRejectsNullRegistryWithoutMutation},
        {TEST_SCENE_ASSEMBLY_NULL_ATTACHMENT_INPUT,
            WorldSceneAssemblyBridgeRejectsNullAttachmentInputWithoutMutation},
        {TEST_SCENE_ASSEMBLY_NULL_BINDING_INPUT,
            WorldSceneAssemblyBridgeRejectsNullBindingInputWithoutMutation},
        {TEST_SCENE_ASSEMBLY_INVALID_ATTACHMENT,
            WorldSceneAssemblyBridgeRejectsInvalidAttachmentRecordWithoutMutation},
        {TEST_SCENE_ASSEMBLY_INVALID_BINDING,
            WorldSceneAssemblyBridgeRejectsInvalidBindingRecordWithoutMutation},
        {TEST_SCENE_ASSEMBLY_MISSING_ATTACHMENT,
            WorldSceneAssemblyBridgeRejectsMissingAttachmentForBindingWithoutMutation},
        {TEST_SCENE_ASSEMBLY_DUPLICATE_ATTACHMENT,
            WorldSceneAssemblyBridgeRejectsDuplicateAttachmentInputWithoutMutation},
        {TEST_SCENE_ASSEMBLY_DUPLICATE_BINDING,
            WorldSceneAssemblyBridgeRejectsDuplicateBindingInputWithoutMutation},
        {TEST_SCENE_ASSEMBLY_ATTACHMENT_CAPACITY,
            WorldSceneAssemblyBridgeRejectsAttachmentCapacityOverflowWithoutMutation},
        {TEST_SCENE_ASSEMBLY_BINDING_CAPACITY,
            WorldSceneAssemblyBridgeRejectsBindingCapacityOverflowWithoutMutation},
        {TEST_SCENE_ASSEMBLY_NON_EMPTY_DESTINATIONS,
            WorldSceneAssemblyBridgeRejectsNonEmptyDestinationsWithoutMutation},
        {TEST_SCENE_ASSEMBLY_RESOURCE_PREFLIGHT,
            WorldSceneAssemblyBridgeValidatesResourceHandlesBeforeMutation},
        {TEST_SCENE_ASSEMBLY_BINDING_PREFLIGHT,
            WorldSceneAssemblyBridgeBindingPreflightFailureDoesNotRestoreAttachments},
        {TEST_SCENE_ASSEMBLY_RESOURCE_ACQUIRE_FAILURE,
            WorldSceneAssemblyBridgeResourceAcquireFailureDoesNotPartiallyAssemble},
        {TEST_SCENE_ASSEMBLY_RESTORE_PATH,
            WorldSceneAssemblyBridgeRestorePathDoesNotGrowStorage},
        {TEST_SCENE_ASSEMBLY_NO_HIDDEN_ALLOCATION,
            WorldSceneAssemblyBridgeNoHiddenAllocationUsesYuMemorySignal},
        {TEST_SCENE_ASSEMBLY_COUNTERS,
            WorldSceneAssemblyBridgeSnapshotReportsCountsAndLastStatus},
        {TEST_SCENE_ASSEMBLY_NO_PAYLOAD,
            WorldSceneAssemblyBridgeNoActorComponentPayloadOrLifecycle},
        {TEST_SCENE_ASSEMBLY_NO_OBJECT_SCRIPT,
            WorldSceneAssemblyBridgeNoObjectScriptSerializeThreadPlatformDiagnosticsDependency},
        {TEST_SCENE_ASSEMBLY_NO_FILE_PACKAGE,
            WorldSceneAssemblyBridgeNoFilePackageLoadDecodeUploadOrGameAdapterDependency},
        {TEST_SCENE_ASSEMBLY_NO_RENDER_PHYSICS,
            WorldSceneAssemblyBridgeNoRenderPhysicsAudioInputUiToolOrReportDependency},
        {TEST_SCENE_ASSEMBLY_WORLD_CORE_FREE,
            WorldSceneAssemblyBridgeWorldInstanceCoreRemainsAssemblyFree},
        {TEST_SCENE_ASSEMBLY_RESOURCE_CORE_FREE,
            WorldSceneAssemblyBridgeResourceCoreRemainsWorldFree},
        {TEST_COMPONENT_ADD_VALID, WorldComponentAttachmentBridgeAddValidAttachmentStoresRecord},
        {TEST_COMPONENT_ADD_INVALID_WORLD, WorldComponentAttachmentBridgeAddRejectsInvalidWorldIdWithoutMutation},
        {TEST_COMPONENT_ADD_INVALID_TYPE, WorldComponentAttachmentBridgeAddRejectsInvalidComponentTypeWithoutMutation},
        {TEST_COMPONENT_ADD_INVALID_SLOT, WorldComponentAttachmentBridgeAddRejectsInvalidComponentSlotWithoutMutation},
        {TEST_COMPONENT_ADD_DUPLICATE, WorldComponentAttachmentBridgeAddRejectsDuplicateTypeForWorldObject},
        {TEST_COMPONENT_ADD_CAPACITY, WorldComponentAttachmentBridgeAddRejectsCapacityOverflowWithoutMutation},
        {TEST_COMPONENT_QUERY_STORED, WorldComponentAttachmentBridgeQueryReturnsStoredAttachment},
        {TEST_COMPONENT_QUERY_MISSING, WorldComponentAttachmentBridgeQueryRejectsMissingAttachmentWithoutMutation},
        {TEST_COMPONENT_QUERY_READ_ONLY, WorldComponentAttachmentBridgeQueryIsReadOnlyAndBounded},
        {TEST_COMPONENT_REMOVE_CLEARS, WorldComponentAttachmentBridgeRemoveClearsAttachment},
        {TEST_COMPONENT_REMOVE_MISSING, WorldComponentAttachmentBridgeRemoveRejectsMissingAttachmentWithoutMutation},
        {TEST_COMPONENT_CLEAR_ALL, WorldComponentAttachmentBridgeClearRemovesAllAttachmentsInSlotOrder},
        {TEST_COMPONENT_UPDATE_PATH, WorldComponentAttachmentBridgeUpdatePathDoesNotGrowStorage},
        {TEST_COMPONENT_SNAPSHOT, WorldComponentAttachmentBridgeSnapshotReportsCountsAndLastStatus},
        {TEST_COMPONENT_NO_WORLD_QUERY, WorldComponentAttachmentBridgeDoesNotQueryOrMutateWorldInstance},
        {TEST_COMPONENT_NO_BEHAVIOR, WorldComponentAttachmentBridgeNoActorComponentBehaviorOrLifecycle},
        {TEST_COMPONENT_NO_OBJECT_RESOURCE, WorldComponentAttachmentBridgeNoObjectResourceScriptSerializeOrGameAdapterDependency},
        {TEST_COMPONENT_NO_FILE_PACKAGE, WorldComponentAttachmentBridgeNoFilePackageThreadPlatformDiagnosticsDependency},
        {TEST_COMPONENT_NO_RENDER_PHYSICS, WorldComponentAttachmentBridgeNoRenderPhysicsAudioInputUiToolOrReportDependency},
        {TEST_COMPONENT_WORLD_CORE_FREE, WorldComponentAttachmentBridgeWorldInstanceCoreRemainsAttachmentFree},
        {TEST_QUERY_TYPE_MATCHES, WorldComponentQueryBridgeQueryTypeReturnsMatchingWorldObjectsInSlotOrder},
        {TEST_QUERY_TYPE_MISSING, WorldComponentQueryBridgeQueryTypeReturnsZeroForMissingType},
        {TEST_QUERY_OBJECT_MATCHES, WorldComponentQueryBridgeQueryObjectReturnsMatchingAttachmentsInSlotOrder},
        {TEST_QUERY_OBJECT_MISSING, WorldComponentQueryBridgeQueryObjectReturnsZeroForMissingObject},
        {TEST_QUERY_NULL_SOURCE, WorldComponentQueryBridgeQueryRejectsNullSourceWithoutMutation},
        {TEST_QUERY_INVALID_TYPE, WorldComponentQueryBridgeQueryRejectsInvalidComponentTypeWithoutMutation},
        {TEST_QUERY_INVALID_WORLD, WorldComponentQueryBridgeQueryRejectsInvalidWorldIdWithoutMutation},
        {TEST_QUERY_NULL_OUTPUT, WorldComponentQueryBridgeQueryRejectsNullOutputWhenCapacityNonZero},
        {TEST_QUERY_OUTPUT_OVERFLOW, WorldComponentQueryBridgeQueryRejectsOutputOverflowWithoutOverrun},
        {TEST_QUERY_READ_ONLY, WorldComponentQueryBridgeQueryIsReadOnlyForAttachmentStorage},
        {TEST_QUERY_UPDATE_PATH, WorldComponentQueryBridgeQueryPathDoesNotGrowStorage},
        {TEST_QUERY_SNAPSHOT, WorldComponentQueryBridgeSnapshotReportsCountsAndLastStatus},
        {TEST_QUERY_NO_BEHAVIOR, WorldComponentQueryBridgeNoActorComponentBehaviorOrLifecycle},
        {TEST_QUERY_NO_OBJECT_RESOURCE, WorldComponentQueryBridgeNoObjectResourceScriptSerializeOrGameAdapterDependency},
        {TEST_QUERY_NO_FILE_PACKAGE, WorldComponentQueryBridgeNoFilePackageThreadPlatformDiagnosticsDependency},
        {TEST_QUERY_NO_RENDER_PHYSICS, WorldComponentQueryBridgeNoRenderPhysicsAudioInputUiToolOrReportDependency},
        {TEST_QUERY_WORLD_CORE_FREE, WorldComponentQueryBridgeWorldInstanceCoreRemainsQueryFree},
        {TEST_COMPONENT_SNAPSHOT_ROUND_TRIP,
            WorldComponentAttachmentSnapshotBridgeWriteReadRoundTripsAttachmentsInSlotOrder},
        {TEST_COMPONENT_SNAPSHOT_EMPTY_WRITE,
            WorldComponentAttachmentSnapshotBridgeWriteEmptyBridgeProducesZeroRecords},
        {TEST_COMPONENT_SNAPSHOT_NULL_SOURCE,
            WorldComponentAttachmentSnapshotBridgeWriteRejectsNullSourceWithoutMutation},
        {TEST_COMPONENT_SNAPSHOT_NULL_WRITER,
            WorldComponentAttachmentSnapshotBridgeWriteRejectsNullWriterWithoutMutation},
        {TEST_COMPONENT_SNAPSHOT_WRITER_OVERFLOW,
            WorldComponentAttachmentSnapshotBridgeWriteRejectsWriterOverflowWithoutOverrun},
        {TEST_COMPONENT_SNAPSHOT_READ_RESTORES,
            WorldComponentAttachmentSnapshotBridgeReadRestoresAttachmentRecords},
        {TEST_COMPONENT_SNAPSHOT_READ_ZERO_CLEARS,
            WorldComponentAttachmentSnapshotBridgeReadZeroRecordStreamClearsDestination},
        {TEST_COMPONENT_SNAPSHOT_NULL_READER,
            WorldComponentAttachmentSnapshotBridgeReadRejectsNullReaderWithoutMutation},
        {TEST_COMPONENT_SNAPSHOT_NULL_DESTINATION,
            WorldComponentAttachmentSnapshotBridgeReadRejectsNullDestinationWithoutMutation},
        {TEST_COMPONENT_SNAPSHOT_UNKNOWN_VERSION,
            WorldComponentAttachmentSnapshotBridgeReadRejectsUnknownVersionWithoutMutation},
        {TEST_COMPONENT_SNAPSHOT_MALFORMED_COUNT,
            WorldComponentAttachmentSnapshotBridgeReadRejectsMalformedRecordCountWithoutMutation},
        {TEST_COMPONENT_SNAPSHOT_INVALID_WORLD,
            WorldComponentAttachmentSnapshotBridgeReadRejectsInvalidWorldIdWithoutMutation},
        {TEST_COMPONENT_SNAPSHOT_INVALID_TYPE,
            WorldComponentAttachmentSnapshotBridgeReadRejectsInvalidComponentTypeWithoutMutation},
        {TEST_COMPONENT_SNAPSHOT_INVALID_SLOT,
            WorldComponentAttachmentSnapshotBridgeReadRejectsInvalidComponentSlotWithoutMutation},
        {TEST_COMPONENT_SNAPSHOT_DUPLICATE,
            WorldComponentAttachmentSnapshotBridgeReadRejectsDuplicateAttachmentWithoutMutation},
        {TEST_COMPONENT_SNAPSHOT_PATH,
            WorldComponentAttachmentSnapshotBridgeWriteReadPathDoesNotGrowStorage},
        {TEST_COMPONENT_SNAPSHOT_COUNTERS,
            WorldComponentAttachmentSnapshotBridgeSnapshotReportsCountsAndLastStatus},
        {TEST_COMPONENT_SNAPSHOT_NO_PAYLOAD,
            WorldComponentAttachmentSnapshotBridgeNoActorComponentPayloadOrLifecycle},
        {TEST_COMPONENT_SNAPSHOT_NO_OBJECT_RESOURCE,
            WorldComponentAttachmentSnapshotBridgeNoObjectResourceScriptFilePackageOrGameAdapterDependency},
        {TEST_COMPONENT_SNAPSHOT_NO_RENDER_PHYSICS,
            WorldComponentAttachmentSnapshotBridgeNoRenderPhysicsAudioInputUiToolOrReportDependency},
        {TEST_COMPONENT_SNAPSHOT_WORLD_CORE_FREE,
            WorldComponentAttachmentSnapshotBridgeWorldInstanceCoreRemainsSnapshotFree}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
