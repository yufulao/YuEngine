// 模块: YuEngine Platform
// 文件: Src/YuEngine/Platform/Include/YuEngine/Platform/PlatformWindowDesc.h

#pragma once

#include <cstddef>
#include <cstdint>

namespace yuengine::platform {
struct PlatformWindowDesc {
    static constexpr std::uint32_t MIN_CLIENT_SIZE = 1U;
    static constexpr std::uint32_t MAX_CLIENT_SIZE = 16384U;
    static constexpr std::uint32_t DEFAULT_CLIENT_WIDTH = 1280U;
    static constexpr std::uint32_t DEFAULT_CLIENT_HEIGHT = 720U;
    static constexpr std::size_t MAX_TITLE_LENGTH = 127U;
    static constexpr std::size_t DEFAULT_EVENT_QUEUE_CAPACITY = 32U;
    static constexpr std::size_t MAX_EVENT_QUEUE_CAPACITY = 64U;

    const char* title = "YuEngine";
    std::uint32_t client_width = DEFAULT_CLIENT_WIDTH;
    std::uint32_t client_height = DEFAULT_CLIENT_HEIGHT;
    bool visible = false;
    std::size_t event_queue_capacity = DEFAULT_EVENT_QUEUE_CAPACITY;
};
}
