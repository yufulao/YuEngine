// Module: YuEngine Input
// File: Src/YuEngine/Input/Include/YuEngine/Input/InputBridge.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/Input/InputBridgeDesc.h"
#include "YuEngine/Input/InputBridgeEvent.h"
#include "YuEngine/Input/InputBridgeSnapshot.h"
#include "YuEngine/Input/InputGamepadState.h"
#include "YuEngine/Input/InputStatus.h"

namespace yuengine::input {
class InputBridge final {
public:
    /**
     * @comment Constructs an empty input bridge owner.
     */
    InputBridge();

    /**
     * @comment Shuts down the bridge owner when initialized.
     */
    ~InputBridge();

    InputBridge(const InputBridge &) = delete;
    InputBridge &operator=(const InputBridge &) = delete;

    /**
     * @comment Initializes the bridge with fixed capacity and backend policy.
     * @param desc Input bridge descriptor.
     * @return Explicit operation status.
     */
    InputStatus Initialize(const InputBridgeDesc &desc);

    /**
     * @comment Shuts down the bridge and clears queued events.
     * @return Explicit operation status.
     */
    InputStatus Shutdown();

    /**
     * @comment Updates source focus state and records focus counters.
     * @param focused Whether input source focus is active.
     * @return Explicit operation status.
     */
    InputStatus SetFocus(bool focused);

    /**
     * @comment Submits a backend-neutral event into bounded bridge storage.
     * @param event Event value.
     * @return Explicit operation status.
     */
    InputStatus SubmitEvent(const InputBridgeEvent &event);

    /**
     * @comment Translates a native source message into bounded bridge events.
     * @param message_code Source message code.
     * @param word_value Source word-sized value.
     * @param long_value Source long-sized value.
     * @param source_focused Whether source focus is active for this message.
     * @return Explicit operation status.
     */
    InputStatus SubmitSourceMessage(std::uint32_t message_code, std::uintptr_t word_value, std::intptr_t long_value, bool source_focused);

    /**
     * @comment Submits a backend-neutral gamepad state into bridge events.
     * @param state Gamepad state value.
     * @return Explicit operation status.
     */
    InputStatus SubmitGamepadState(const InputGamepadState &state);

    /**
     * @comment Polls a native gamepad source and submits the translated state.
     * @param user_index Native gamepad slot index.
     * @return Explicit operation status.
     */
    InputStatus PollGamepad(std::uint32_t user_index);

    /**
     * @comment Drains queued bridge events into caller-owned storage.
     * @param events Caller-owned output buffer.
     * @param event_capacity Number of records available in events.
     * @param out_event_count Written event count.
     * @return Explicit operation status.
     */
    InputStatus DrainEvents(InputBridgeEvent *events, std::size_t event_capacity, std::size_t &out_event_count);

    /**
     * @comment Returns current bridge counters and status.
     * @return Snapshot value.
     */
    InputBridgeSnapshot Snapshot() const;

private:
    InputStatus ValidateDesc(const InputBridgeDesc &desc) const;
    InputStatus ValidateEvent(const InputBridgeEvent &event) const;
    InputStatus RecordStatus(InputStatus status);
    InputStatus RejectEvent(InputStatus status);
    InputStatus AcceptEvent(const InputBridgeEvent &event);
    InputStatus AcceptGamepadState(const InputGamepadState &state);
    void ClearQueuedEvents();
    std::size_t CountGamepadStateEvents(const InputGamepadState &state) const;
    void SubmitGamepadStateEvents(const InputGamepadState &state);
    bool IsDeviceValid(InputDeviceId device) const;
    bool IsEventKnown(InputBridgeEventType type) const;
    bool IsAxisValueValid(std::int32_t value) const;

    std::array<InputBridgeEvent, InputBridgeDesc::MAX_EVENT_CAPACITY> events_{};
    InputBridgeDesc desc_{};
    InputBridgeSnapshot snapshot_{};
    InputGamepadState gamepad_state_{};
    std::size_t read_index_ = 0U;
    std::size_t write_index_ = 0U;
    std::size_t event_count_ = 0U;
    bool initialized_ = false;
    bool focused_ = false;
};
}
