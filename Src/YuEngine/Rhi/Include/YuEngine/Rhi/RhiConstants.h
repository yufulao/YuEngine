#pragma once

#include <cstddef>
#include <cstdint>

namespace yuengine::rhi {
constexpr std::size_t MAX_COLOR_TARGETS = 8U;
constexpr std::uint16_t MAX_COLOR_TARGET_EXTENT = 16U;
constexpr std::uint16_t MAX_CAPTURE_FIXTURE_EXTENT = 4U;
constexpr std::size_t MAX_COMMANDS = 32U;
constexpr std::size_t RGBA8_BYTES_PER_PIXEL = 4U;
}
