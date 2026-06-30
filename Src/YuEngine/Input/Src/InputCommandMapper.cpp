// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Src/InputCommandMapper.cpp

#include "YuEngine/Input/InputCommandMapper.h"

namespace yuengine::input {
InputCommandMapper::InputCommandMapper()
    : contexts_(),
      bindings_(),
      actions_(),
      active_actions_(),
      snapshot_(),
      context_count_(0U),
      binding_count_(0U),
      active_context_set_(false) {
    snapshot_.context_capacity = MAX_INPUT_CONTEXTS;
    snapshot_.binding_capacity = MAX_INPUT_BINDINGS;
    snapshot_.command_capacity = MAX_INPUT_COMMAND_RECORDS;
}

InputStatus InputCommandMapper::RegisterContext(InputContextId context) {
    if (!IsContextInRange(context)) {
        return RecordFailure(InputStatus::UnknownContext);
    }

    if (HasContext(context)) {
        return RecordFailure(InputStatus::DuplicateBinding);
    }

    if (context_count_ >= contexts_.size()) {
        return RecordFailure(InputStatus::CapacityExceeded);
    }

    contexts_[context_count_] = context;
    ++context_count_;
    snapshot_.context_count = context_count_;
    return RecordStatus(InputStatus::Success);
}

InputStatus InputCommandMapper::SetActiveContext(InputContextId context, InputContextFocusMode focus_mode) {
    if (!HasContext(context)) {
        return RecordFailure(InputStatus::UnknownContext);
    }

    snapshot_.active_context = context;
    snapshot_.focus_mode = focus_mode;
    active_context_set_ = true;
    return RecordStatus(InputStatus::Success);
}

InputStatus InputCommandMapper::RegisterBinding(InputCommandBinding binding) {
    const InputStatus binding_status = ValidateBinding(binding);
    if (binding_status != InputStatus::Success) {
        return RecordFailure(binding_status);
    }

    if (binding_count_ >= bindings_.size()) {
        return RecordFailure(InputStatus::CapacityExceeded);
    }

    bindings_[binding_count_] = binding;
    ++binding_count_;
    snapshot_.binding_count = binding_count_;
    active_actions_[binding.action.value] = true;
    return RecordStatus(InputStatus::Success);
}

InputStatus InputCommandMapper::BuildSnapshot(
    std::uint64_t frame_index,
    std::span<const InputEvent> events,
    InputCommandSnapshot *snapshot) {
    if (snapshot == nullptr) {
        return RejectOperation(InputStatus::NullPointer);
    }

    *snapshot = InputCommandSnapshot{};
    snapshot->frame_index = frame_index;
    if (!active_context_set_) {
        snapshot->status = InputStatus::UnknownContext;
        return RejectOperation(InputStatus::UnknownContext);
    }

    if (!HasContext(snapshot_.active_context)) {
        snapshot->status = InputStatus::UnknownContext;
        return RejectOperation(InputStatus::UnknownContext);
    }

    if (snapshot_.focus_mode == InputContextFocusMode::RejectInput) {
        snapshot->status = InputStatus::FocusLost;
        return RejectOperation(InputStatus::FocusLost);
    }

    const InputStatus event_status = ValidateEvents(events);
    if (event_status != InputStatus::Success) {
        snapshot->status = event_status;
        return RejectOperation(event_status);
    }

    ResetFrameFlags();
    for (const InputEvent &event : events) {
        ApplyEvent(event);
    }

    const InputStatus emit_status = EmitActiveCommands(snapshot);
    snapshot->status = emit_status;
    if (emit_status != InputStatus::Success) {
        return RejectOperation(emit_status);
    }

    snapshot_.accepted_event_count += events.size();
    ++snapshot_.build_count;
    return RecordStatus(InputStatus::Success);
}

InputCommandMapperSnapshot InputCommandMapper::Snapshot() const {
    return snapshot_;
}

InputStatus InputCommandMapper::RecordStatus(InputStatus status) {
    snapshot_.last_status = status;
    return status;
}

InputStatus InputCommandMapper::RecordFailure(InputStatus status) {
    ++snapshot_.failed_operation_count;
    return RecordStatus(status);
}

InputStatus InputCommandMapper::RejectOperation(InputStatus status) {
    ++snapshot_.rejected_event_count;
    return RecordFailure(status);
}

bool InputCommandMapper::IsContextInRange(InputContextId context) const {
    return context.value < MAX_INPUT_CONTEXTS;
}

bool InputCommandMapper::IsActionInRange(InputActionId action) const {
    return action.value < MAX_INPUT_ACTIONS;
}

bool InputCommandMapper::IsDeviceValid(InputDeviceId device) const {
    return device.value < MAX_INPUT_DEVICES;
}

bool InputCommandMapper::IsAxisValueValid(std::int32_t value) const {
    if (value < AXIS_MIN_VALUE) {
        return false;
    }

    return value <= AXIS_MAX_VALUE;
}

bool InputCommandMapper::IsEventTypeKnown(InputEventType type) const {
    if (type == InputEventType::ButtonPressed) {
        return true;
    }

    if (type == InputEventType::ButtonReleased) {
        return true;
    }

    return type == InputEventType::Axis;
}

bool InputCommandMapper::IsValueKindKnown(InputCommandValueKind value_kind) const {
    if (value_kind == InputCommandValueKind::Button) {
        return true;
    }

    return value_kind == InputCommandValueKind::Axis;
}

bool InputCommandMapper::HasContext(InputContextId context) const {
    for (std::size_t index = 0U; index < context_count_; ++index) {
        if (contexts_[index].value == context.value) {
            return true;
        }
    }

    return false;
}

bool InputCommandMapper::HasBindingForControl(InputContextId context, InputDeviceId device, InputControlId control) const {
    return FindBinding(context, device, control) != nullptr;
}

bool InputCommandMapper::HasActionRecord(InputContextId context, InputActionId action) const {
    for (std::size_t index = 0U; index < binding_count_; ++index) {
        const InputCommandBinding &binding = bindings_[index];
        if (binding.context.value != context.value) {
            continue;
        }

        if (binding.action.value == action.value) {
            return true;
        }
    }

    return false;
}

const InputCommandBinding *InputCommandMapper::FindBinding(
    InputContextId context,
    InputDeviceId device,
    InputControlId control) const {
    for (std::size_t index = 0U; index < binding_count_; ++index) {
        const InputCommandBinding &binding = bindings_[index];
        if (binding.context.value != context.value) {
            continue;
        }

        if (binding.device.value != device.value) {
            continue;
        }

        if (binding.control.value != control.value) {
            continue;
        }

        return &binding;
    }

    return nullptr;
}

InputStatus InputCommandMapper::ValidateBinding(InputCommandBinding binding) const {
    if (!HasContext(binding.context)) {
        return InputStatus::UnknownContext;
    }

    if (!IsDeviceValid(binding.device)) {
        return InputStatus::UnknownDeviceControl;
    }

    if (!IsActionInRange(binding.action)) {
        return InputStatus::UnknownAction;
    }

    if (!IsValueKindKnown(binding.value_kind)) {
        return InputStatus::InvalidDescriptor;
    }

    if (HasBindingForControl(binding.context, binding.device, binding.control)) {
        return InputStatus::DuplicateBinding;
    }

    return InputStatus::Success;
}

InputStatus InputCommandMapper::ValidateEvents(std::span<const InputEvent> events) const {
    for (const InputEvent &event : events) {
        if (!IsEventTypeKnown(event.type)) {
            return InputStatus::InvalidEvent;
        }

        if (event.type == InputEventType::Axis) {
            if (!IsAxisValueValid(event.axis_value)) {
                return InputStatus::InvalidAxisValue;
            }
        }

        const InputCommandBinding *binding = FindBinding(snapshot_.active_context, event.device, event.control);
        if (binding == nullptr) {
            return InputStatus::UnknownDeviceControl;
        }

        if (binding->value_kind == InputCommandValueKind::Axis) {
            if (event.type != InputEventType::Axis) {
                return InputStatus::InvalidEvent;
            }
        }

        if (binding->value_kind == InputCommandValueKind::Button) {
            if (event.type == InputEventType::Axis) {
                return InputStatus::InvalidEvent;
            }
        }
    }

    return InputStatus::Success;
}

InputStatus InputCommandMapper::EmitActiveCommands(InputCommandSnapshot *snapshot) const {
    if (snapshot == nullptr) {
        return InputStatus::NullPointer;
    }

    for (std::size_t index = 0U; index < MAX_INPUT_ACTIONS; ++index) {
        if (!active_actions_[index]) {
            continue;
        }

        const InputActionId action{static_cast<std::uint32_t>(index)};
        if (!HasActionRecord(snapshot_.active_context, action)) {
            continue;
        }

        InputCommandValueKind value_kind = InputCommandValueKind::Button;
        for (std::size_t binding_index = 0U; binding_index < binding_count_; ++binding_index) {
            const InputCommandBinding &binding = bindings_[binding_index];
            if (binding.context.value != snapshot_.active_context.value) {
                continue;
            }

            if (binding.action.value != action.value) {
                continue;
            }

            value_kind = binding.value_kind;
            break;
        }

        if (snapshot->command_count >= snapshot->commands.size()) {
            return InputStatus::CapacityExceeded;
        }

        InputCommandRecord &record = snapshot->commands[snapshot->command_count];
        record.action = action;
        record.value_kind = value_kind;
        record.held = actions_[index].is_pressed;
        record.axis_value = actions_[index].axis_value;
        if (value_kind == InputCommandValueKind::Button) {
            record.pressed_this_frame = actions_[index].changed_this_frame && actions_[index].is_pressed;
            record.released_this_frame = actions_[index].changed_this_frame && !actions_[index].is_pressed;
        }

        ++snapshot->command_count;
    }

    return InputStatus::Success;
}

void InputCommandMapper::ResetFrameFlags() {
    for (InputActionState &action : actions_) {
        action.changed_this_frame = false;
    }
}

void InputCommandMapper::ApplyEvent(const InputEvent &event) {
    const InputCommandBinding *binding = FindBinding(snapshot_.active_context, event.device, event.control);
    if (binding == nullptr) {
        return;
    }

    InputActionState &action = actions_[binding->action.value];
    if (event.type == InputEventType::ButtonPressed) {
        action.is_pressed = true;
        action.changed_this_frame = true;
        return;
    }

    if (event.type == InputEventType::ButtonReleased) {
        action.is_pressed = false;
        action.changed_this_frame = true;
        return;
    }

    if (event.type == InputEventType::Axis) {
        action.axis_value = event.axis_value;
        action.changed_this_frame = true;
        return;
    }
}
}
