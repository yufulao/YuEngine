// 模块: YuEngine Platform
// 文件: Src/YuEngine/Platform/Src/WindowsPlatformWindow.cpp

#include "YuEngine/Platform/WindowsPlatformWindow.h"

#include <algorithm>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

namespace yuengine::platform {
namespace {
bool IsValidTitle(const char* title) {
    if (title == nullptr) {
        return false;
    }

    for (std::size_t index = 0U; index < PlatformWindowDesc::MAX_TITLE_LENGTH; ++index) {
        const char title_character = title[index];
        if (title_character == '\0') {
            return index > 0U;
        }

        const unsigned char title_value = static_cast<unsigned char>(title_character);
        if (title_value > 127U) {
            return false;
        }
    }

    return title[PlatformWindowDesc::MAX_TITLE_LENGTH] == '\0';
}

#if defined(_WIN32)
constexpr const wchar_t* WINDOWS_PLATFORM_WINDOW_CLASS_NAME = L"YuEnginePlatformWindow";
constexpr std::size_t TITLE_BUFFER_CAPACITY = PlatformWindowDesc::MAX_TITLE_LENGTH + 1U;
constexpr std::uint32_t RAW_MOUSE_LEFT_BUTTON = 1U;
constexpr std::uint32_t RAW_MOUSE_RIGHT_BUTTON = 2U;
constexpr std::uint32_t RAW_MOUSE_MIDDLE_BUTTON = 3U;

HWND ToWindowHandle(std::uintptr_t window_value) {
    return reinterpret_cast<HWND>(window_value);
}

HINSTANCE ToInstanceHandle(std::uintptr_t instance_value) {
    return reinterpret_cast<HINSTANCE>(instance_value);
}

bool CopyTitleToWideBuffer(const char* title, wchar_t* output, std::size_t output_capacity) {
    if (title == nullptr) {
        return false;
    }

    if (output == nullptr) {
        return false;
    }

    if (output_capacity < TITLE_BUFFER_CAPACITY) {
        return false;
    }

    for (std::size_t index = 0U; index < PlatformWindowDesc::MAX_TITLE_LENGTH; ++index) {
        const char title_character = title[index];
        if (title_character == '\0') {
            output[index] = L'\0';
            return index > 0U;
        }

        const unsigned char title_value = static_cast<unsigned char>(title_character);
        if (title_value > 127U) {
            return false;
        }

        output[index] = static_cast<wchar_t>(title_value);
    }

    if (title[PlatformWindowDesc::MAX_TITLE_LENGTH] != '\0') {
        return false;
    }

    output[PlatformWindowDesc::MAX_TITLE_LENGTH] = L'\0';
    return true;
}

bool RegisterWindowClass(HINSTANCE instance_handle);
LRESULT CALLBACK WindowsPlatformWindowProc(HWND window_handle, UINT message, WPARAM word_param, LPARAM long_param);

std::int32_t ReadPointerX(LPARAM long_param) {
    const std::uintptr_t packed_value = static_cast<std::uintptr_t>(long_param);
    const std::uint16_t low_word = static_cast<std::uint16_t>(packed_value & 0xffffU);
    return static_cast<std::int32_t>(static_cast<std::int16_t>(low_word));
}

std::int32_t ReadPointerY(LPARAM long_param) {
    const std::uintptr_t packed_value = static_cast<std::uintptr_t>(long_param);
    const std::uint16_t high_word = static_cast<std::uint16_t>((packed_value >> 16U) & 0xffffU);
    return static_cast<std::int32_t>(static_cast<std::int16_t>(high_word));
}
#endif
}

struct WindowsPlatformWindowAccess {
    static void ApplyClientExtent(WindowsPlatformWindow& window, std::uint32_t client_width, std::uint32_t client_height) {
        window.ApplyClientExtent(client_width, client_height);
    }

    static void ApplyFocusState(WindowsPlatformWindow& window, bool focused) {
        window.ApplyFocusState(focused);
    }

    static void ApplyMinimizedState(WindowsPlatformWindow& window, bool minimized) {
        window.ApplyMinimizedState(minimized);
    }

    static void ApplyCloseRequest(WindowsPlatformWindow& window) {
        window.ApplyCloseRequest();
    }

    static PlatformWindowStatus PushPlatformEvent(WindowsPlatformWindow& window, const PlatformWindowEvent& event) {
        return window.PushPlatformEvent(event);
    }
};

WindowsPlatformWindow::WindowsPlatformWindow() = default;

WindowsPlatformWindow::~WindowsPlatformWindow() {
    static_cast<void>(Destroy());
}

PlatformWindowStatus WindowsPlatformWindow::Create(const PlatformWindowDesc& desc) {
    if (!IsValidDescriptor(desc)) {
        return SetLastStatus(PlatformWindowStatus::InvalidDescriptor);
    }

    if (created_) {
        return SetLastStatus(PlatformWindowStatus::AlreadyCreated);
    }

    if (destroyed_) {
        return SetLastStatus(PlatformWindowStatus::Closed);
    }

#if !defined(_WIN32)
    static_cast<void>(desc);
    return SetLastStatus(PlatformWindowStatus::Unsupported);
#endif

#if defined(_WIN32)
    HINSTANCE instance_handle = GetModuleHandleW(nullptr);
    if (instance_handle == nullptr) {
        return SetLastStatus(PlatformWindowStatus::NativeCallFailed);
    }

    if (!RegisterWindowClass(instance_handle)) {
        return SetLastStatus(PlatformWindowStatus::NativeCallFailed);
    }

    std::array<wchar_t, TITLE_BUFFER_CAPACITY> title_buffer{};
    if (!CopyTitleToWideBuffer(desc.title, title_buffer.data(), title_buffer.size())) {
        return SetLastStatus(PlatformWindowStatus::InvalidDescriptor);
    }

    RECT window_rect{};
    window_rect.right = static_cast<LONG>(desc.client_width);
    window_rect.bottom = static_cast<LONG>(desc.client_height);

    const DWORD window_style = WS_OVERLAPPEDWINDOW;
    const BOOL adjusted = AdjustWindowRect(&window_rect, window_style, FALSE);
    if (adjusted == FALSE) {
        return SetLastStatus(PlatformWindowStatus::NativeCallFailed);
    }

    const LONG adjusted_width = window_rect.right - window_rect.left;
    const LONG adjusted_height = window_rect.bottom - window_rect.top;
    const int native_width = static_cast<int>(adjusted_width);
    const int native_height = static_cast<int>(adjusted_height);

    HWND window_handle = CreateWindowExW(
        0U,
        WINDOWS_PLATFORM_WINDOW_CLASS_NAME,
        title_buffer.data(),
        window_style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        native_width,
        native_height,
        nullptr,
        nullptr,
        instance_handle,
        this);
    if (window_handle == nullptr) {
        return SetLastStatus(PlatformWindowStatus::NativeCallFailed);
    }

    window_value_ = reinterpret_cast<std::uintptr_t>(window_handle);
    instance_value_ = reinterpret_cast<std::uintptr_t>(instance_handle);
    client_width_ = desc.client_width;
    client_height_ = desc.client_height;
    event_queue_capacity_ = desc.event_queue_capacity;
    created_ = true;
    destroyed_ = false;
    visible_ = desc.visible;
    focused_ = GetFocus() == window_handle;
    close_requested_ = false;
    minimized_ = IsIconic(window_handle) != FALSE;
    restored_ = !minimized_;
    ResetEventQueue();

    RECT client_rect{};
    const BOOL client_rect_result = GetClientRect(window_handle, &client_rect);
    if (client_rect_result != FALSE) {
        const LONG native_client_width = client_rect.right - client_rect.left;
        const LONG native_client_height = client_rect.bottom - client_rect.top;
        client_width_ = static_cast<std::uint32_t>(native_client_width);
        client_height_ = static_cast<std::uint32_t>(native_client_height);
    }

    if (desc.visible) {
        ShowWindow(window_handle, SW_SHOW);
        UpdateWindow(window_handle);
    }

    return SetLastStatus(PlatformWindowStatus::Success);
#endif
}

PlatformWindowStatus WindowsPlatformWindow::Destroy() {
    if (created_) {
#if defined(_WIN32)
        HWND window_handle = ToWindowHandle(window_value_);
        if (window_handle != nullptr) {
            const BOOL destroy_result = DestroyWindow(window_handle);
            if (destroy_result == FALSE) {
                return SetLastStatus(PlatformWindowStatus::NativeCallFailed);
            }
        }
#endif
    }

    created_ = false;
    destroyed_ = true;
    visible_ = false;
    focused_ = false;
    close_requested_ = false;
    minimized_ = false;
    restored_ = false;
    client_width_ = 0U;
    client_height_ = 0U;
    ResetEventQueue();
    InvalidateNativeSurface();
    return SetLastStatus(PlatformWindowStatus::Success);
}

PlatformWindowStatus WindowsPlatformWindow::Show() {
    if (!created_) {
        if (destroyed_) {
            return SetLastStatus(PlatformWindowStatus::Closed);
        }

        return SetLastStatus(PlatformWindowStatus::NotCreated);
    }

#if defined(_WIN32)
    HWND window_handle = ToWindowHandle(window_value_);
    if (window_handle == nullptr) {
        return SetLastStatus(PlatformWindowStatus::InvalidLifecycle);
    }

    ShowWindow(window_handle, SW_SHOW);
#endif

    visible_ = true;
    return SetLastStatus(PlatformWindowStatus::Success);
}

PlatformWindowStatus WindowsPlatformWindow::Hide() {
    if (!created_) {
        if (destroyed_) {
            return SetLastStatus(PlatformWindowStatus::Closed);
        }

        return SetLastStatus(PlatformWindowStatus::NotCreated);
    }

#if defined(_WIN32)
    HWND window_handle = ToWindowHandle(window_value_);
    if (window_handle == nullptr) {
        return SetLastStatus(PlatformWindowStatus::InvalidLifecycle);
    }

    ShowWindow(window_handle, SW_HIDE);
#endif

    visible_ = false;
    return SetLastStatus(PlatformWindowStatus::Success);
}

PlatformWindowStatus WindowsPlatformWindow::RequestClose() {
    if (!created_) {
        if (destroyed_) {
            return SetLastStatus(PlatformWindowStatus::Closed);
        }

        return SetLastStatus(PlatformWindowStatus::NotCreated);
    }

    close_requested_ = true;

#if defined(_WIN32)
    HWND window_handle = ToWindowHandle(window_value_);
    if (window_handle == nullptr) {
        return SetLastStatus(PlatformWindowStatus::InvalidLifecycle);
    }

    const BOOL post_result = PostMessageW(window_handle, WM_CLOSE, 0U, 0);
    if (post_result == FALSE) {
        return SetLastStatus(PlatformWindowStatus::NativeCallFailed);
    }
#endif

    return SetLastStatus(PlatformWindowStatus::Success);
}

PlatformWindowPollResult WindowsPlatformWindow::PollEvents(PlatformWindowEvent* events, std::size_t event_capacity) {
    PlatformWindowPollResult result{};
    result.dropped_event_count = dropped_event_count_;

    if (events == nullptr) {
        ClearPollOutputCapacityFailure();
        result.status = SetLastStatus(PlatformWindowStatus::NullPointer);
        return result;
    }

    if (!created_) {
        ClearPollOutputCapacityFailure();
        if (destroyed_) {
            result.status = SetLastStatus(PlatformWindowStatus::Closed);
            return result;
        }

        result.status = SetLastStatus(PlatformWindowStatus::NotCreated);
        return result;
    }

#if defined(_WIN32)
    HWND window_handle = ToWindowHandle(window_value_);
    if (window_handle == nullptr) {
        ClearPollOutputCapacityFailure();
        result.status = SetLastStatus(PlatformWindowStatus::InvalidLifecycle);
        return result;
    }

    MSG message{};
    std::size_t processed_message_count = 0U;
    while (processed_message_count < PlatformWindowDesc::MAX_EVENT_QUEUE_CAPACITY) {
        const BOOL has_message = PeekMessageW(&message, window_handle, 0U, 0U, PM_REMOVE);
        if (has_message == FALSE) {
            break;
        }

        TranslateMessage(&message);
        DispatchMessageW(&message);
        ++processed_message_count;
    }
#endif

    const std::size_t queued_event_count = event_count_;
    const std::size_t writable_count = std::min(event_capacity, event_count_);
    for (std::size_t index = 0U; index < writable_count; ++index) {
        events[index] = event_queue_[event_read_index_];
        event_read_index_ = (event_read_index_ + 1U) % event_queue_capacity_;
        --event_count_;
    }

    result.event_count = writable_count;
    result.events_remaining = event_count_ > 0U;
    result.dropped_event_count = dropped_event_count_;
    if (result.dropped_event_count > 0U) {
        ClearPollOutputCapacityFailure();
        result.status = SetLastStatus(PlatformWindowStatus::EventQueueOverflow);
        return result;
    }

    if (result.events_remaining) {
        RecordPollOutputCapacityFailure(event_capacity, writable_count, queued_event_count);
        result.output_capacity = last_poll_output_capacity_;
        result.output_event_count = last_poll_output_event_count_;
        result.queued_event_count = last_poll_queued_event_count_;
        result.required_output_event_count = last_required_poll_output_event_count_;
        result.first_undrained_event_index = last_first_undrained_poll_event_index_;
        result.first_undrained_event = last_first_undrained_poll_event_;
        result.status = SetLastStatus(PlatformWindowStatus::OutputBufferFull);
        return result;
    }

    ClearPollOutputCapacityFailure();
    result.status = SetLastStatus(PlatformWindowStatus::Success);
    return result;
}

PlatformWindowSnapshot WindowsPlatformWindow::GetSnapshot() const {
    PlatformWindowSnapshot snapshot{};
    snapshot.created = created_;
    snapshot.visible = visible_;
    snapshot.focused = focused_;
    snapshot.close_requested = close_requested_;
    snapshot.minimized = minimized_;
    snapshot.restored = restored_;
    snapshot.client_width = client_width_;
    snapshot.client_height = client_height_;
    snapshot.queued_event_count = event_count_;
    snapshot.event_queue_capacity = event_queue_capacity_;
    if (dropped_event_count_ != 0U) {
        snapshot.required_queued_event_count = event_queue_capacity_ + dropped_event_count_;
    }

    snapshot.last_failed_event = last_failed_event_;
    snapshot.last_failed_event_type = last_failed_event_type_;
    snapshot.last_failed_event_index = last_failed_event_index_;
    snapshot.last_failed_event_queue_capacity = last_failed_event_queue_capacity_;
    snapshot.last_failed_queued_event_count = last_failed_queued_event_count_;
    snapshot.last_required_queued_event_count = last_required_queued_event_count_;
    snapshot.dropped_event_count = dropped_event_count_;
    snapshot.last_failed_event_raw_code = last_failed_event_raw_code_;
    snapshot.last_failed_event_pointer_x = last_failed_event_pointer_x_;
    snapshot.last_failed_event_pointer_y = last_failed_event_pointer_y_;
    snapshot.last_failed_event_wheel_delta = last_failed_event_wheel_delta_;
    snapshot.last_poll_output_capacity = last_poll_output_capacity_;
    snapshot.last_poll_output_event_count = last_poll_output_event_count_;
    snapshot.last_poll_queued_event_count = last_poll_queued_event_count_;
    snapshot.last_required_poll_output_event_count = last_required_poll_output_event_count_;
    snapshot.last_first_undrained_poll_event_index = last_first_undrained_poll_event_index_;
    snapshot.last_first_undrained_poll_event = last_first_undrained_poll_event_;
    snapshot.last_status = last_status_;
    snapshot.native_surface = GetNativeSurface();
    return snapshot;
}

PlatformNativeSurface WindowsPlatformWindow::GetNativeSurface() const {
    PlatformNativeSurface surface{};
    surface.window_value = window_value_;
    surface.instance_value = instance_value_;
    surface.valid = created_ && window_value_ != 0U && instance_value_ != 0U;
    return surface;
}

bool WindowsPlatformWindow::IsCreated() const {
    return created_;
}

bool WindowsPlatformWindow::IsValidDescriptor(const PlatformWindowDesc& desc) const {
    if (!IsValidTitle(desc.title)) {
        return false;
    }

    if (desc.client_width < PlatformWindowDesc::MIN_CLIENT_SIZE) {
        return false;
    }

    if (desc.client_width > PlatformWindowDesc::MAX_CLIENT_SIZE) {
        return false;
    }

    if (desc.client_height < PlatformWindowDesc::MIN_CLIENT_SIZE) {
        return false;
    }

    if (desc.client_height > PlatformWindowDesc::MAX_CLIENT_SIZE) {
        return false;
    }

    if (desc.event_queue_capacity == 0U) {
        return false;
    }

    if (desc.event_queue_capacity > PlatformWindowDesc::MAX_EVENT_QUEUE_CAPACITY) {
        return false;
    }

    return true;
}

PlatformWindowStatus WindowsPlatformWindow::SetLastStatus(PlatformWindowStatus status) {
    if (status != PlatformWindowStatus::EventQueueOverflow) {
        ClearEventQueueCapacityFailure();
    }

    last_status_ = status;
    return status;
}

PlatformWindowStatus WindowsPlatformWindow::PushPlatformEvent(const PlatformWindowEvent& event) {
    if (event_queue_capacity_ == 0U) {
        RecordEventQueueCapacityFailure(event);
        ++dropped_event_count_;
        ClearPollOutputCapacityFailure();
        return SetLastStatus(PlatformWindowStatus::EventQueueOverflow);
    }

    if (event_count_ >= event_queue_capacity_) {
        RecordEventQueueCapacityFailure(event);
        ++dropped_event_count_;
        ClearPollOutputCapacityFailure();
        return SetLastStatus(PlatformWindowStatus::EventQueueOverflow);
    }

    event_queue_[event_write_index_] = event;
    event_write_index_ = (event_write_index_ + 1U) % event_queue_capacity_;
    ++event_count_;
    return SetLastStatus(PlatformWindowStatus::Success);
}

void WindowsPlatformWindow::ApplyClientExtent(std::uint32_t client_width, std::uint32_t client_height) {
    client_width_ = client_width;
    client_height_ = client_height;
}

void WindowsPlatformWindow::ApplyFocusState(bool focused) {
    focused_ = focused;
}

void WindowsPlatformWindow::ApplyMinimizedState(bool minimized) {
    minimized_ = minimized;
    restored_ = !minimized;
}

void WindowsPlatformWindow::ApplyCloseRequest() {
    close_requested_ = true;
}

void WindowsPlatformWindow::ResetEventQueue() {
    event_read_index_ = 0U;
    event_write_index_ = 0U;
    event_count_ = 0U;
    dropped_event_count_ = 0U;
    ClearEventQueueCapacityFailure();
    ClearPollOutputCapacityFailure();
}

void WindowsPlatformWindow::RecordEventQueueCapacityFailure(const PlatformWindowEvent& event) {
    const std::size_t dropped_event_count = static_cast<std::size_t>(dropped_event_count_);
    const std::size_t failed_event_index = event_count_ + dropped_event_count;
    const std::size_t required_queued_event_count = failed_event_index + 1U;
    last_failed_event_ = event;
    last_failed_event_type_ = event.type;
    last_failed_event_raw_code_ = event.raw_code;
    last_failed_event_pointer_x_ = event.pointer_x;
    last_failed_event_pointer_y_ = event.pointer_y;
    last_failed_event_wheel_delta_ = event.wheel_delta;
    last_failed_event_index_ = failed_event_index;
    last_failed_event_queue_capacity_ = event_queue_capacity_;
    last_failed_queued_event_count_ = event_count_;
    last_required_queued_event_count_ = required_queued_event_count;
}

void WindowsPlatformWindow::ClearEventQueueCapacityFailure() {
    last_failed_event_ = PlatformWindowEvent{};
    last_failed_event_type_ = PlatformWindowEventType::None;
    last_failed_event_raw_code_ = 0U;
    last_failed_event_pointer_x_ = 0;
    last_failed_event_pointer_y_ = 0;
    last_failed_event_wheel_delta_ = 0;
    last_failed_event_index_ = 0U;
    last_failed_event_queue_capacity_ = 0U;
    last_failed_queued_event_count_ = 0U;
    last_required_queued_event_count_ = 0U;
}

void WindowsPlatformWindow::InvalidateNativeSurface() {
    window_value_ = 0U;
    instance_value_ = 0U;
}

void WindowsPlatformWindow::ClearPollOutputCapacityFailure() {
    last_poll_output_capacity_ = 0U;
    last_poll_output_event_count_ = 0U;
    last_poll_queued_event_count_ = 0U;
    last_required_poll_output_event_count_ = 0U;
    last_first_undrained_poll_event_index_ = 0U;
    last_first_undrained_poll_event_ = PlatformWindowEvent{};
}

void WindowsPlatformWindow::RecordPollOutputCapacityFailure(
    std::size_t output_capacity,
    std::size_t output_event_count,
    std::size_t queued_event_count) {
    last_poll_output_capacity_ = output_capacity;
    last_poll_output_event_count_ = output_event_count;
    last_poll_queued_event_count_ = queued_event_count;
    last_required_poll_output_event_count_ = queued_event_count;
    last_first_undrained_poll_event_index_ = output_event_count;
    last_first_undrained_poll_event_ = PlatformWindowEvent{};
    if (event_count_ == 0U) {
        return;
    }

    last_first_undrained_poll_event_ = event_queue_[event_read_index_];
}

#if defined(_WIN32)
namespace {
bool RegisterWindowClass(HINSTANCE instance_handle) {
    WNDCLASSW window_class{};
    window_class.lpfnWndProc = WindowsPlatformWindowProc;
    window_class.hInstance = instance_handle;
    window_class.lpszClassName = WINDOWS_PLATFORM_WINDOW_CLASS_NAME;

    const ATOM atom = RegisterClassW(&window_class);
    if (atom != 0U) {
        return true;
    }

    const DWORD error_code = GetLastError();
    if (error_code == ERROR_CLASS_ALREADY_EXISTS) {
        return true;
    }

    return false;
}

LRESULT CALLBACK WindowsPlatformWindowProc(HWND window_handle, UINT message, WPARAM word_param, LPARAM long_param) {
    if (message == WM_NCCREATE) {
        const CREATESTRUCTW* create_struct = reinterpret_cast<const CREATESTRUCTW*>(long_param);
        if (create_struct != nullptr) {
            WindowsPlatformWindow* platform_window = static_cast<WindowsPlatformWindow*>(create_struct->lpCreateParams);
            SetWindowLongPtrW(window_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(platform_window));
        }
    }

    const LONG_PTR user_data = GetWindowLongPtrW(window_handle, GWLP_USERDATA);
    WindowsPlatformWindow* platform_window = reinterpret_cast<WindowsPlatformWindow*>(user_data);
    if (platform_window == nullptr) {
        return DefWindowProcW(window_handle, message, word_param, long_param);
    }

    switch (message) {
        case WM_CLOSE:
        {
            PlatformWindowEvent close_event{};
            close_event.type = PlatformWindowEventType::CloseRequested;
            WindowsPlatformWindowAccess::ApplyCloseRequest(*platform_window);
            static_cast<void>(WindowsPlatformWindowAccess::PushPlatformEvent(*platform_window, close_event));
            return 0;
        }
        case WM_SIZE:
        {
            const std::uint32_t client_width = static_cast<std::uint32_t>(LOWORD(long_param));
            const std::uint32_t client_height = static_cast<std::uint32_t>(HIWORD(long_param));
            PlatformWindowEvent resize_event{};
            resize_event.type = PlatformWindowEventType::Resized;
            resize_event.client_width = client_width;
            resize_event.client_height = client_height;
            WindowsPlatformWindowAccess::ApplyClientExtent(*platform_window, client_width, client_height);
            static_cast<void>(WindowsPlatformWindowAccess::PushPlatformEvent(*platform_window, resize_event));

            if (word_param == SIZE_MINIMIZED) {
                PlatformWindowEvent minimized_event{};
                minimized_event.type = PlatformWindowEventType::Minimized;
                WindowsPlatformWindowAccess::ApplyMinimizedState(*platform_window, true);
                static_cast<void>(WindowsPlatformWindowAccess::PushPlatformEvent(*platform_window, minimized_event));
                return 0;
            }

            if (word_param == SIZE_RESTORED) {
                PlatformWindowEvent restored_event{};
                restored_event.type = PlatformWindowEventType::Restored;
                WindowsPlatformWindowAccess::ApplyMinimizedState(*platform_window, false);
                static_cast<void>(WindowsPlatformWindowAccess::PushPlatformEvent(*platform_window, restored_event));
                return 0;
            }

            break;
        }
        case WM_SETFOCUS:
        {
            PlatformWindowEvent focus_event{};
            focus_event.type = PlatformWindowEventType::FocusGained;
            WindowsPlatformWindowAccess::ApplyFocusState(*platform_window, true);
            static_cast<void>(WindowsPlatformWindowAccess::PushPlatformEvent(*platform_window, focus_event));
            return 0;
        }
        case WM_KILLFOCUS:
        {
            PlatformWindowEvent focus_event{};
            focus_event.type = PlatformWindowEventType::FocusLost;
            WindowsPlatformWindowAccess::ApplyFocusState(*platform_window, false);
            static_cast<void>(WindowsPlatformWindowAccess::PushPlatformEvent(*platform_window, focus_event));
            return 0;
        }
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            PlatformWindowEvent key_event{};
            key_event.type = PlatformWindowEventType::RawKeyDown;
            key_event.raw_code = static_cast<std::uint32_t>(word_param);
            static_cast<void>(WindowsPlatformWindowAccess::PushPlatformEvent(*platform_window, key_event));
            return 0;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            PlatformWindowEvent key_event{};
            key_event.type = PlatformWindowEventType::RawKeyUp;
            key_event.raw_code = static_cast<std::uint32_t>(word_param);
            static_cast<void>(WindowsPlatformWindowAccess::PushPlatformEvent(*platform_window, key_event));
            return 0;
        }
        case WM_MOUSEMOVE:
        {
            PlatformWindowEvent mouse_event{};
            mouse_event.type = PlatformWindowEventType::RawMouseMove;
            mouse_event.pointer_x = ReadPointerX(long_param);
            mouse_event.pointer_y = ReadPointerY(long_param);
            static_cast<void>(WindowsPlatformWindowAccess::PushPlatformEvent(*platform_window, mouse_event));
            return 0;
        }
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        {
            PlatformWindowEvent mouse_event{};
            mouse_event.type = message == WM_LBUTTONDOWN ? PlatformWindowEventType::RawMouseButtonDown : PlatformWindowEventType::RawMouseButtonUp;
            mouse_event.raw_code = RAW_MOUSE_LEFT_BUTTON;
            mouse_event.pointer_x = ReadPointerX(long_param);
            mouse_event.pointer_y = ReadPointerY(long_param);
            static_cast<void>(WindowsPlatformWindowAccess::PushPlatformEvent(*platform_window, mouse_event));
            return 0;
        }
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        {
            PlatformWindowEvent mouse_event{};
            mouse_event.type = message == WM_RBUTTONDOWN ? PlatformWindowEventType::RawMouseButtonDown : PlatformWindowEventType::RawMouseButtonUp;
            mouse_event.raw_code = RAW_MOUSE_RIGHT_BUTTON;
            mouse_event.pointer_x = ReadPointerX(long_param);
            mouse_event.pointer_y = ReadPointerY(long_param);
            static_cast<void>(WindowsPlatformWindowAccess::PushPlatformEvent(*platform_window, mouse_event));
            return 0;
        }
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        {
            PlatformWindowEvent mouse_event{};
            mouse_event.type = message == WM_MBUTTONDOWN ? PlatformWindowEventType::RawMouseButtonDown : PlatformWindowEventType::RawMouseButtonUp;
            mouse_event.raw_code = RAW_MOUSE_MIDDLE_BUTTON;
            mouse_event.pointer_x = ReadPointerX(long_param);
            mouse_event.pointer_y = ReadPointerY(long_param);
            static_cast<void>(WindowsPlatformWindowAccess::PushPlatformEvent(*platform_window, mouse_event));
            return 0;
        }
        case WM_MOUSEWHEEL:
        {
            PlatformWindowEvent mouse_event{};
            mouse_event.type = PlatformWindowEventType::RawMouseMove;
            mouse_event.wheel_delta = static_cast<std::int32_t>(GET_WHEEL_DELTA_WPARAM(word_param));
            static_cast<void>(WindowsPlatformWindowAccess::PushPlatformEvent(*platform_window, mouse_event));
            return 0;
        }
        default:
        {
            break;
        }
    }

    return DefWindowProcW(window_handle, message, word_param, long_param);
}
}
#endif
}
