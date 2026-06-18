// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Include/YuEngine/Input/InputReplay.h

#pragma once

#include <array>
#include <cstddef>

#include "YuEngine/Input/InputActionBinding.h"
#include "YuEngine/Input/InputActionQueryResult.h"
#include "YuEngine/Input/InputActionState.h"
#include "YuEngine/Input/InputApplyResult.h"
#include "YuEngine/Input/InputBindingResult.h"
#include "YuEngine/Input/InputConstants.h"
#include "YuEngine/Input/InputEvent.h"
#include "YuEngine/Input/InputReplaySnapshot.h"
#include "YuEngine/Input/InputStatus.h"
#include "YuEngine/Input/InputReplayFrame.h"

namespace yuengine::input {
class InputReplay final {
public:
    /**
     * @comment 构造 InputReplay 实例。
     */
    InputReplay();

    /**
     * @comment 注册 action binding。
     * @param device 输入 device。
     * @param control 输入 control。
     * @param action 输入 action。
     * @return 显式操作结果。
     */
    InputBindingResult RegisterActionBinding(InputDeviceId device, InputControlId control, InputActionId action);
    /**
     * @comment 记录 replay event。
     * @param frame_index 输入 frame index。
     * @param event 输入 event。
     * @return 显式操作状态。
     */
    InputStatus RecordReplayEvent(std::size_t frame_index, InputEvent event);
    /**
     * @comment 应用 next frame。
     * @return 显式操作结果。
     */
    InputApplyResult ApplyNextFrame();
    /**
     * @comment 重置 frame state。
     * @return 显式操作状态。
     */
    InputStatus ResetFrameState();
    /**
     * @comment 查询 action。
     * @param action 输入 action。
     * @return 显式操作结果。
     */
    InputActionQueryResult QueryAction(InputActionId action) const;
    /**
     * @comment 返回当前状态快照。
     * @return 快照值。
     */
    InputReplaySnapshot Snapshot() const;
    /**
     * @comment 返回一个 frame 的 event count。
     * @param frame_index 输入 frame index。
     * @return Event count for frame 值。
     */
    std::size_t EventCountForFrame(std::size_t frame_index) const;

private:
    InputStatus RecordFailure(InputStatus status);
    InputStatus RejectReplayEvent(InputStatus status);
    bool IsDeviceValid(InputDeviceId device) const;
    bool IsActionInRange(InputActionId action) const;
    bool IsEventTypeKnown(InputEventType type) const;
    bool IsAxisValueValid(std::int32_t value) const;
    bool HasBindingForControl(InputDeviceId device, InputControlId control) const;
    const InputActionBinding* FindBinding(InputDeviceId device, InputControlId control) const;
    void MarkActionChanged(InputActionId action);
    void RecalculateChangedActionCount();
    std::size_t ReplayStorageCapacity() const;

    std::array<InputActionBinding, MAX_INPUT_BINDINGS> bindings_;
    std::array<InputReplayFrame, MAX_REPLAY_FRAMES> frames_;
    std::array<InputActionState, MAX_INPUT_ACTIONS> actions_;
    std::array<bool, MAX_INPUT_ACTIONS> registered_actions_;
    InputReplaySnapshot snapshot_;
    std::size_t binding_count_;
    std::size_t recorded_frame_count_;
    std::size_t next_frame_index_;
};
}
