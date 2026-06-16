// Module: YuEngine Platform
// File: Src/YuEngine/Platform/Include/YuEngine/Platform/WindowsPlatformWindow.h

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
     * @comment Constructs an empty Windows platform window wrapper.
     */
    WindowsPlatformWindow();

    /**
     * @comment Destroys the wrapped native window when one exists.
     */
    ~WindowsPlatformWindow();

    WindowsPlatformWindow(const WindowsPlatformWindow&) = delete;
    WindowsPlatformWindow& operator=(const WindowsPlatformWindow&) = delete;

    /**
     * @comment Creates a native window from a validated Platform descriptor.
     * @param desc Input window descriptor.
     * @return Explicit operation status.
     */
    PlatformWindowStatus Create(const PlatformWindowDesc& desc);

    /**
     * @comment Destroys the native window and invalidates the native surface.
     * @return Explicit operation status.
     */
    PlatformWindowStatus Destroy();

    /**
     * @comment Shows the native window.
     * @return Explicit operation status.
     */
    PlatformWindowStatus Show();

    /**
     * @comment Hides the native window.
     * @return Explicit operation status.
     */
    PlatformWindowStatus Hide();

    /**
     * @comment Requests a close event without silently destroying the window.
     * @return Explicit operation status.
     */
    PlatformWindowStatus RequestClose();

    /**
     * @comment Polls pending platform events into caller-owned storage.
     * @param events Caller-owned event output buffer.
     * @param event_capacity Number of records available in events.
     * @return Poll status and number of written events.
     */
    PlatformWindowPollResult PollEvents(PlatformWindowEvent* events, std::size_t event_capacity);

    /**
     * @comment Returns the current immutable Platform window snapshot.
     * @return Current snapshot value.
     */
    PlatformWindowSnapshot GetSnapshot() const;

    /**
     * @comment Returns the current opaque native surface value.
     * @return Current native surface value.
     */
    PlatformNativeSurface GetNativeSurface() const;

    /**
     * @comment Returns whether a native window is currently created.
     * @return True when the native window is live.
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
