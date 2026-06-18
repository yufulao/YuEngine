// 模块: YuEngine World
// 文件: Src/YuEngine/World/Src/WorldInstance.cpp

#include "YuEngine/World/WorldInstance.h"

using yuengine::memory::MemoryAccountingStatus;

namespace yuengine::world {
namespace {
std::uint32_t ClampCapacity(std::uint32_t requested_capacity, std::uint32_t maximum_capacity) {
    if (requested_capacity > maximum_capacity) {
        return maximum_capacity;
    }

    return requested_capacity;
}
}

WorldInstance::WorldInstance()
    : WorldInstance(WorldDesc{}) {
}

WorldInstance::WorldInstance(WorldDesc desc)
    : slots_{},
      phase_trace_{},
      snapshot_{
          ClampCapacity(desc.object_capacity, MAX_WORLD_OBJECT_COUNT),
          ClampCapacity(desc.phase_trace_capacity, MAX_WORLD_PHASE_TRACE_COUNT),
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          MemoryAccountingStatus::ExplicitlyTrackedOnly,
          WorldLifecycleState::Created,
          WorldStatus::Success} {
    if (desc.object_capacity == 0U) {
        snapshot_.lifecycle_state = WorldLifecycleState::Failed;
        snapshot_.last_status = WorldStatus::InvalidObjectCapacity;
        return;
    }

    if (desc.phase_trace_capacity < WORLD_UPDATE_PHASE_COUNT) {
        snapshot_.lifecycle_state = WorldLifecycleState::Failed;
        snapshot_.last_status = WorldStatus::InvalidPhaseTraceCapacity;
        return;
    }
}

WorldRegistrationResult WorldInstance::RegisterObject(const WorldObjectDesc &desc) {
    const WorldStatus setup_status = ValidateSetupState();
    if (setup_status != WorldStatus::Success) {
        return WorldRegistrationResult::Failure(RecordFailure(setup_status));
    }

    if (!desc.id.IsValid()) {
        return WorldRegistrationResult::Failure(RecordFailure(WorldStatus::InvalidObjectId));
    }

    if (FindSlot(desc.id) != nullptr) {
        return WorldRegistrationResult::Failure(RecordFailure(WorldStatus::DuplicateObjectId));
    }

    if (snapshot_.registered_object_count >= snapshot_.object_capacity) {
        return WorldRegistrationResult::Failure(RecordFailure(WorldStatus::CapacityExceeded));
    }

    WorldObjectSlot *slot = FindFreeSlot();
    if (slot == nullptr) {
        return WorldRegistrationResult::Failure(RecordFailure(WorldStatus::CapacityExceeded));
    }

    slot->id = desc.id;
    slot->is_registered = true;
    slot->is_enabled = desc.is_enabled;
    ++snapshot_.registered_object_count;
    RecountActiveObjects();
    RecordSuccess();
    return WorldRegistrationResult::Success(desc.id);
}

WorldStatus WorldInstance::RemoveObject(WorldObjectId id) {
    if (!id.IsValid()) {
        return RecordFailure(WorldStatus::InvalidObjectId);
    }

    if (snapshot_.lifecycle_state != WorldLifecycleState::Created &&
        snapshot_.lifecycle_state != WorldLifecycleState::Running) {
        return RecordFailure(WorldStatus::InvalidLifecycleState);
    }

    WorldObjectSlot *slot = FindSlot(id);
    if (slot == nullptr) {
        return RecordFailure(WorldStatus::ObjectNotFound);
    }

    slot->id = WorldObjectId{};
    slot->is_registered = false;
    slot->is_enabled = false;
    --snapshot_.registered_object_count;
    RecountActiveObjects();
    RecordSuccess();
    return WorldStatus::Success;
}

WorldStatus WorldInstance::Start() {
    if (snapshot_.lifecycle_state != WorldLifecycleState::Created) {
        return RecordFailure(WorldStatus::InvalidLifecycleState);
    }

    snapshot_.lifecycle_state = WorldLifecycleState::Starting;
    snapshot_.lifecycle_state = WorldLifecycleState::Running;
    RecordSuccess();
    return WorldStatus::Success;
}

WorldStatus WorldInstance::Update(std::uint64_t frame_index,
    std::uint64_t fixed_step_duration,
    std::uint64_t frame_delta_duration) {
    if (snapshot_.lifecycle_state != WorldLifecycleState::Running) {
        return RecordFailure(WorldStatus::InvalidLifecycleState);
    }

    if (fixed_step_duration == 0U) {
        return RecordFailure(WorldStatus::InvalidTimeStep);
    }

    if (frame_delta_duration == 0U) {
        return RecordFailure(WorldStatus::InvalidTimeStep);
    }

    std::uint32_t skipped_object_count = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.object_capacity; ++index) {
        const WorldObjectSlot &slot = slots_[index];
        if (!slot.is_registered) {
            continue;
        }

        if (slot.is_enabled) {
            continue;
        }

        ++skipped_object_count;
    }

    ResetPhaseTrace();
    AppendPhaseTrace(WorldUpdatePhase::BeginFrame, frame_index, snapshot_.active_object_count, skipped_object_count);
    AppendPhaseTrace(WorldUpdatePhase::FixedStep, frame_index, snapshot_.active_object_count, skipped_object_count);
    AppendPhaseTrace(WorldUpdatePhase::FrameStep, frame_index, snapshot_.active_object_count, skipped_object_count);
    AppendPhaseTrace(WorldUpdatePhase::EndFrame, frame_index, snapshot_.active_object_count, skipped_object_count);

    ++snapshot_.frame_count;
    snapshot_.phase_execution_count += WORLD_UPDATE_PHASE_COUNT;
    snapshot_.skipped_object_count += skipped_object_count;
    snapshot_.last_frame_index = frame_index;
    snapshot_.last_fixed_step_duration = fixed_step_duration;
    snapshot_.last_frame_delta_duration = frame_delta_duration;
    RecordSuccess();
    return WorldStatus::Success;
}

WorldStatus WorldInstance::Stop() {
    if (snapshot_.lifecycle_state != WorldLifecycleState::Running) {
        return RecordFailure(WorldStatus::InvalidLifecycleState);
    }

    snapshot_.lifecycle_state = WorldLifecycleState::Stopping;
    ClearRegisteredObjects();
    snapshot_.lifecycle_state = WorldLifecycleState::Stopped;
    RecordSuccess();
    return WorldStatus::Success;
}

WorldSnapshot WorldInstance::Snapshot() const {
    return snapshot_;
}

const WorldPhaseTrace *WorldInstance::GetPhaseTrace() const {
    return phase_trace_.data();
}

std::uint32_t WorldInstance::GetPhaseTraceCount() const {
    return snapshot_.phase_trace_count;
}

bool WorldInstance::ContainsObject(WorldObjectId id) const {
    if (!id.IsValid()) {
        return false;
    }

    return FindSlot(id) != nullptr;
}

WorldStatus WorldInstance::RecordFailure(WorldStatus status) {
    snapshot_.last_status = status;
    return status;
}

void WorldInstance::RecordSuccess() {
    snapshot_.last_status = WorldStatus::Success;
}

WorldStatus WorldInstance::ValidateSetupState() const {
    if (snapshot_.lifecycle_state != WorldLifecycleState::Created) {
        return WorldStatus::InvalidLifecycleState;
    }

    return WorldStatus::Success;
}

WorldObjectSlot *WorldInstance::FindSlot(WorldObjectId id) {
    for (std::uint32_t index = 0U; index < snapshot_.object_capacity; ++index) {
        WorldObjectSlot &slot = slots_[index];
        if (!slot.is_registered) {
            continue;
        }

        if (slot.id.value == id.value) {
            return &slot;
        }
    }

    return nullptr;
}

const WorldObjectSlot *WorldInstance::FindSlot(WorldObjectId id) const {
    for (std::uint32_t index = 0U; index < snapshot_.object_capacity; ++index) {
        const WorldObjectSlot &slot = slots_[index];
        if (!slot.is_registered) {
            continue;
        }

        if (slot.id.value == id.value) {
            return &slot;
        }
    }

    return nullptr;
}

WorldObjectSlot *WorldInstance::FindFreeSlot() {
    for (std::uint32_t index = 0U; index < snapshot_.object_capacity; ++index) {
        WorldObjectSlot &slot = slots_[index];
        if (slot.is_registered) {
            continue;
        }

        return &slot;
    }

    return nullptr;
}

void WorldInstance::ClearRegisteredObjects() {
    for (std::uint32_t index = 0U; index < snapshot_.object_capacity; ++index) {
        WorldObjectSlot &slot = slots_[index];
        slot.id = WorldObjectId{};
        slot.is_registered = false;
        slot.is_enabled = false;
    }

    snapshot_.registered_object_count = 0U;
    snapshot_.active_object_count = 0U;
}

void WorldInstance::RecountActiveObjects() {
    std::uint32_t active_object_count = 0U;
    for (std::uint32_t index = 0U; index < snapshot_.object_capacity; ++index) {
        const WorldObjectSlot &slot = slots_[index];
        if (!slot.is_registered) {
            continue;
        }

        if (!slot.is_enabled) {
            continue;
        }

        ++active_object_count;
    }

    snapshot_.active_object_count = active_object_count;
}

void WorldInstance::ResetPhaseTrace() {
    snapshot_.phase_trace_count = 0U;
}

void WorldInstance::AppendPhaseTrace(WorldUpdatePhase phase,
    std::uint64_t frame_index,
    std::uint32_t active_object_count,
    std::uint32_t skipped_object_count) {
    if (snapshot_.phase_trace_count >= snapshot_.phase_trace_capacity) {
        snapshot_.last_status = WorldStatus::InvalidTraceBuffer;
        return;
    }

    WorldPhaseTrace &trace = phase_trace_[snapshot_.phase_trace_count];
    trace.phase = phase;
    trace.frame_index = frame_index;
    trace.active_object_count = active_object_count;
    trace.skipped_object_count = skipped_object_count;
    ++snapshot_.phase_trace_count;
}
}
