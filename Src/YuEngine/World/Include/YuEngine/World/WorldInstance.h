// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldInstance.h

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
     * @comment 构造 WorldInstance 实例。
     */
    WorldInstance();
    /**
     * @comment 构造 WorldInstance 实例。
     * @param desc 输入 descriptor。
     */
    explicit WorldInstance(WorldDesc desc);

    /**
     * @comment 注册一个 fixture world object。
     * @param desc 输入 world object descriptor。
     * @return 显式操作结果。
     */
    WorldRegistrationResult RegisterObject(const WorldObjectDesc &desc);
    /**
     * @comment 移除一个 fixture world object。
     * @param id 输入 world object id。
     * @return 显式操作状态。
     */
    WorldStatus RemoveObject(WorldObjectId id);
    /**
     * @comment 启动 world lifecycle。
     * @return 显式操作状态。
     */
    WorldStatus Start();
    /**
     * @comment 运行 one deterministic world update。
     * @param frame_index 输入 frame index。
     * @param fixed_step_duration 输入 fixed step duration。
     * @param frame_delta_duration 输入 frame delta duration。
     * @return 显式操作状态。
     */
    WorldStatus Update(std::uint64_t frame_index,
        std::uint64_t fixed_step_duration,
        std::uint64_t frame_delta_duration);
    /**
     * @comment 停止 world lifecycle。
     * @return 显式操作状态。
     */
    WorldStatus Stop();
    /**
     * @comment 返回当前 world 状态快照。
     * @return 快照值。
     */
    WorldSnapshot Snapshot() const;
    /**
     * @comment 返回 deterministic phase trace records。
     * @return immutable phase trace records 指针。
     */
    const WorldPhaseTrace *GetPhaseTrace() const;
    /**
     * @comment 返回 deterministic phase trace record 数量。
     * @return Phase trace record 数量。
     */
    std::uint32_t GetPhaseTraceCount() const;
    /**
     * @comment 检查 fixture world object 是否存在。
     * @param id 输入 world object id。
     * @return world object 存在时返回 true，否则返回 false。
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
