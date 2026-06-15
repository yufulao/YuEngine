// Module: Tests World
// File: Tests/World/WorldTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
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
using yuengine::world::MAX_WORLD_OBJECT_COUNT;
using yuengine::world::MAX_WORLD_PHASE_TRACE_COUNT;
using yuengine::world::WORLD_UPDATE_PHASE_COUNT;
using yuengine::world::WORLD_INSTANCE_SERVICE_ID;
using yuengine::world::WORLD_KERNEL_MODULE_NAME;
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
using TestFunction = int (*)();

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
        {TEST_TRANSFORM_CORE_FREE, WorldTransformBridgeWorldInstanceCoreRemainsTransformStorageFree}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
