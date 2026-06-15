// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldInstance.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/World/WorldConstants.h"
#include "YuEngine/World/WorldDesc.h"
#include "YuEngine/World/WorldObjectDesc.h"
#include "YuEngine/World/WorldObjectSlot.h"
#include "YuEngine/World/WorldPhaseTrace.h"
#include "YuEngine/World/WorldRegistrationResult.h"
#include "YuEngine/World/WorldSnapshot.h"
#include "YuEngine/World/WorldStatus.h"

namespace yuengine::world {
class WorldInstance final {
public:
    /**
     * @comment Constructs a WorldInstance instance.
     */
    WorldInstance();
    /**
     * @comment Constructs a WorldInstance instance.
     * @param desc Input descriptor.
     */
    explicit WorldInstance(WorldDesc desc);

    /**
     * @comment Registers a fixture world object.
     * @param desc Input world object descriptor.
     * @return Explicit operation result.
     */
    WorldRegistrationResult RegisterObject(const WorldObjectDesc &desc);
    /**
     * @comment Removes a fixture world object.
     * @param id Input world object id.
     * @return Explicit operation status.
     */
    WorldStatus RemoveObject(WorldObjectId id);
    /**
     * @comment Starts the world lifecycle.
     * @return Explicit operation status.
     */
    WorldStatus Start();
    /**
     * @comment Runs one deterministic world update.
     * @param frame_index Input frame index.
     * @param fixed_step_duration Input fixed step duration.
     * @param frame_delta_duration Input frame delta duration.
     * @return Explicit operation status.
     */
    WorldStatus Update(std::uint64_t frame_index,
        std::uint64_t fixed_step_duration,
        std::uint64_t frame_delta_duration);
    /**
     * @comment Stops the world lifecycle.
     * @return Explicit operation status.
     */
    WorldStatus Stop();
    /**
     * @comment Returns a snapshot of the current world state.
     * @return Snapshot value.
     */
    WorldSnapshot Snapshot() const;
    /**
     * @comment Returns deterministic phase trace records.
     * @return Pointer to immutable phase trace records.
     */
    const WorldPhaseTrace *GetPhaseTrace() const;
    /**
     * @comment Returns deterministic phase trace record count.
     * @return Phase trace record count.
     */
    std::uint32_t GetPhaseTraceCount() const;
    /**
     * @comment Checks whether a fixture world object exists.
     * @param id Input world object id.
     * @return True when the world object exists; false otherwise.
     */
    bool ContainsObject(WorldObjectId id) const;

private:
    WorldStatus RecordFailure(WorldStatus status);
    void RecordSuccess();
    WorldStatus ValidateSetupState() const;
    WorldObjectSlot *FindSlot(WorldObjectId id);
    const WorldObjectSlot *FindSlot(WorldObjectId id) const;
    WorldObjectSlot *FindFreeSlot();
    void ClearRegisteredObjects();
    void RecountActiveObjects();
    void ResetPhaseTrace();
    void AppendPhaseTrace(WorldUpdatePhase phase,
        std::uint64_t frame_index,
        std::uint32_t active_object_count,
        std::uint32_t skipped_object_count);

    std::array<WorldObjectSlot, MAX_WORLD_OBJECT_COUNT> slots_;
    std::array<WorldPhaseTrace, MAX_WORLD_PHASE_TRACE_COUNT> phase_trace_;
    WorldSnapshot snapshot_;
};
}
