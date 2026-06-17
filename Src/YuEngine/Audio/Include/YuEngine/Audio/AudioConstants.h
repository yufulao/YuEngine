// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioConstants.h

#pragma once

#include <cstddef>
#include <cstdint>

namespace yuengine::audio {
constexpr std::uint32_t SAMPLE_RATE = 48000U;
constexpr std::uint16_t CHANNEL_COUNT = 2U;
constexpr std::size_t MAX_SOURCES = 8U;
constexpr std::size_t MAX_PCM_SAMPLE_PACKETS = 8U;
constexpr std::size_t MAX_VOICES = 16U;
constexpr std::size_t MAX_SOURCE_FRAMES = 256U;
constexpr std::size_t MAX_OUTPUT_FRAMES = 64U;
constexpr std::uint32_t MAX_Q15_GAIN = 32767U;
constexpr std::int16_t S16_MIN = -32768;
constexpr std::int16_t S16_MAX = 32767;
}
