// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Include/YuEngine/Input/InputBridge.h

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
     * @comment 构造空的 input bridge owner。
     */
    InputBridge();

    /**
     * @comment 已初始化时关闭 bridge owner。
     */
    ~InputBridge();

    InputBridge(const InputBridge &) = delete;
    InputBridge &operator=(const InputBridge &) = delete;

    /**
     * @comment 使用 fixed capacity 和 backend policy 初始化 bridge。
     * @param desc 输入 bridge descriptor。
     * @return 显式操作状态。
     */
    InputStatus Initialize(const InputBridgeDesc &desc);

    /**
     * @comment 关闭 bridge 并清空 queued events。
     * @return 显式操作状态。
     */
    InputStatus Shutdown();

    /**
     * @comment 更新 source focus state 并记录 focus counters。
     * @param focused 表示 input source focus 是否 active。
     * @return 显式操作状态。
     */
    InputStatus SetFocus(bool focused);

    /**
     * @comment 将 backend-neutral event 提交到 bounded bridge storage。
     * @param event Event 值。
     * @return 显式操作状态。
     */
    InputStatus SubmitEvent(const InputBridgeEvent &event);

    /**
     * @comment 将 native source message 转换为 bounded bridge events。
     * @param message_code 输入 source message code。
     * @param word_value Source word-sized 值。
     * @param long_value Source long-sized 值。
     * @param source_focused 表示此 message 的 source focus 是否 active。
     * @return 显式操作状态。
     */
    InputStatus SubmitSourceMessage(std::uint32_t message_code, std::uintptr_t word_value, std::intptr_t long_value, bool source_focused);

    /**
     * @comment 将 backend-neutral gamepad state 提交到 bridge events。
     * @param state Gamepad state 值。
     * @return 显式操作状态。
     */
    InputStatus SubmitGamepadState(const InputGamepadState &state);

    /**
     * @comment 轮询 native gamepad source 并提交 translated state。
     * @param user_index 输入 native gamepad slot index。
     * @return 显式操作状态。
     */
    InputStatus PollGamepad(std::uint32_t user_index);

    /**
     * @comment 将 queued bridge events 排空到调用方持有存储。
     * @param events 调用方持有的 output buffer。
     * @param event_capacity events 中可用 record 数量。
     * @param out_event_count Written event 数量。
     * @return 显式操作状态。
     */
    InputStatus DrainEvents(InputBridgeEvent *events, std::size_t event_capacity, std::size_t &out_event_count);

    /**
     * @comment 返回当前 bridge counters 和 status。
     * @return 快照值。
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
