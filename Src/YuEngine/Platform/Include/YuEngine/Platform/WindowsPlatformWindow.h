// 模块: YuEngine Platform
// 文件: Src/YuEngine/Platform/Include/YuEngine/Platform/WindowsPlatformWindow.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/Platform/PlatformNativeSurface.h"
#include "YuEngine/Platform/PlatformWindowDesc.h"
#include "YuEngine/Platform/PlatformWindowEvent.h"
#include "YuEngine/Platform/PlatformWindowPollResult.h"
#include "YuEngine/Platform/PlatformWindowSnapshot.h"
#include "YuEngine/Platform/PlatformWindowStatus.h"

namespace yuengine::platform {
struct WindowsPlatformWindowAccess;

class WindowsPlatformWindow final {
public:
    /**
     * @comment 构造空 Windows platform 窗口 wrapper。
     */
    WindowsPlatformWindow();

    /**
     * @comment 存在 wrapped native 窗口 时销毁它。
     */
    ~WindowsPlatformWindow();

    WindowsPlatformWindow(const WindowsPlatformWindow&) = delete;
    WindowsPlatformWindow& operator=(const WindowsPlatformWindow&) = delete;

    /**
     * @comment 创建 一个 native 窗口 从 一个 已验证 Platform 描述。
     * @param desc 输入 窗口 描述。
     * @return 显式操作状态。
     */
    PlatformWindowStatus Create(const PlatformWindowDesc& desc);

    /**
     * @comment 销毁 native 窗口 并使 native surface 失效。
     * @return 显式操作状态。
     */
    PlatformWindowStatus Destroy();

    /**
     * @comment 显示 native 窗口。
     * @return 显式操作状态。
     */
    PlatformWindowStatus Show();

    /**
     * @comment 隐藏 native 窗口。
     * @return 显式操作状态。
     */
    PlatformWindowStatus Hide();

    /**
     * @comment 请求 一个 close event 且不 silently destroying 窗口。
     * @return 显式操作状态。
     */
    PlatformWindowStatus RequestClose();

    /**
     * @comment 轮询 待处理 platform events 写入 调用方持有 存储.
     * @param events 调用方持有的 event output buffer。
     * @param event_capacity 记录 可用 在 events 的数量。
     * @return Poll 状态 和 数量 的 写入的 events.
     */
    PlatformWindowPollResult PollEvents(PlatformWindowEvent* events, std::size_t event_capacity);

    /**
     * @comment 返回当前 immutable Platform 窗口 快照。
     * @return 当前快照值。
     */
    PlatformWindowSnapshot GetSnapshot() const;

    /**
     * @comment 返回当前 opaque native surface 值。
     * @return 当前 native surface 值。
     */
    PlatformNativeSurface GetNativeSurface() const;

    /**
     * @comment 返回 native 窗口当前是否已创建。
     * @return native 窗口 存活时返回 true。
     */
    bool IsCreated() const;

private:
    friend struct WindowsPlatformWindowAccess;

    bool IsValidDescriptor(const PlatformWindowDesc& desc) const;
    PlatformWindowStatus SetLastStatus(PlatformWindowStatus status);
    PlatformWindowStatus PushPlatformEvent(const PlatformWindowEvent& event);
    void ApplyClientExtent(std::uint32_t client_width, std::uint32_t client_height);
    void ApplyFocusState(bool focused);
    void ApplyMinimizedState(bool minimized);
    void ApplyCloseRequest();
    void ResetEventQueue();
    void InvalidateNativeSurface();

    std::array<PlatformWindowEvent, PlatformWindowDesc::MAX_EVENT_QUEUE_CAPACITY> event_queue_{};
    std::size_t event_queue_capacity_ = PlatformWindowDesc::DEFAULT_EVENT_QUEUE_CAPACITY;
    std::size_t event_read_index_ = 0U;
    std::size_t event_write_index_ = 0U;
    std::size_t event_count_ = 0U;
    std::uint32_t dropped_event_count_ = 0U;
    std::uintptr_t window_value_ = 0U;
    std::uintptr_t instance_value_ = 0U;
    std::uint32_t client_width_ = 0U;
    std::uint32_t client_height_ = 0U;
    bool created_ = false;
    bool destroyed_ = false;
    bool visible_ = false;
    bool focused_ = false;
    bool close_requested_ = false;
    bool minimized_ = false;
    bool restored_ = false;
    PlatformWindowStatus last_status_ = PlatformWindowStatus::NotCreated;
};
}
