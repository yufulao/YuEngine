// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Include/YuEngine/Input/InputCommandMapper.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/Input/InputActionState.h"
#include "YuEngine/Input/InputCommandBinding.h"
#include "YuEngine/Input/InputCommandMapperSnapshot.h"
#include "YuEngine/Input/InputCommandSnapshot.h"
#include "YuEngine/Input/InputConstants.h"
#include "YuEngine/Input/InputContextFocusMode.h"
#include "YuEngine/Input/InputContextId.h"
#include "YuEngine/Input/InputEvent.h"
#include "YuEngine/Input/InputStatus.h"

namespace yuengine::input {
class InputCommandMapper final {
public:
    /**
     * @comment 构造 InputCommandMapper 实例。
     */
    InputCommandMapper();

    /**
     * @comment 注册 input context。
     * @param context 输入 context。
     * @return 显式操作状态。
     */
    InputStatus RegisterContext(InputContextId context);

    /**
     * @comment 设置 active input context 和 focus mode。
     * @param context 输入 context。
     * @param focus_mode 输入 focus mode。
     * @return 显式操作状态。
     */
    InputStatus SetActiveContext(InputContextId context, InputContextFocusMode focus_mode);

    /**
     * @comment 注册 input command binding。
     * @param binding 输入 binding。
     * @return 显式操作状态。
     */
    InputStatus RegisterBinding(InputCommandBinding binding);

    /**
     * @comment 根据 L0 input events 构建 frame command snapshot。
     * @param frame_index 输入 frame index。
     * @param events 输入 L0 event span。
     * @param snapshot 输出 snapshot。
     * @return 显式操作状态。
     */
    InputStatus BuildSnapshot(
        std::uint64_t frame_index,
        std::span<const InputEvent> events,
        InputCommandSnapshot *snapshot);

    /**
     * @comment 返回 mapper 当前状态快照。
     * @return 快照值。
     */
    InputCommandMapperSnapshot Snapshot() const;

private:
    InputStatus RecordStatus(InputStatus status);
    InputStatus RecordFailure(InputStatus status);
    InputStatus RecordBindingCapacityFailure(InputCommandBinding binding);
    InputStatus RejectOperation(InputStatus status);
    bool IsContextInRange(InputContextId context) const;
    bool IsActionInRange(InputActionId action) const;
    bool IsDeviceValid(InputDeviceId device) const;
    bool IsAxisValueValid(std::int32_t value) const;
    bool IsEventTypeKnown(InputEventType type) const;
    bool IsValueKindKnown(InputCommandValueKind value_kind) const;
    bool HasContext(InputContextId context) const;
    bool HasBindingForControl(InputContextId context, InputDeviceId device, InputControlId control) const;
    bool HasActionRecord(InputContextId context, InputActionId action) const;
    const InputCommandBinding *FindBinding(InputContextId context, InputDeviceId device, InputControlId control) const;
    InputStatus ValidateBinding(InputCommandBinding binding) const;
    InputStatus ValidateEvents(std::span<const InputEvent> events) const;
    InputStatus EmitActiveCommands(InputCommandSnapshot *snapshot) const;
    void ResetFrameFlags();
    void ApplyEvent(const InputEvent &event);

    std::array<InputContextId, MAX_INPUT_CONTEXTS> contexts_;
    std::array<InputCommandBinding, MAX_INPUT_BINDINGS> bindings_;
    std::array<InputActionState, MAX_INPUT_ACTIONS> actions_;
    std::array<bool, MAX_INPUT_ACTIONS> active_actions_;
    InputCommandMapperSnapshot snapshot_;
    std::size_t context_count_ = 0U;
    std::size_t binding_count_ = 0U;
    bool active_context_set_ = false;
};
}
