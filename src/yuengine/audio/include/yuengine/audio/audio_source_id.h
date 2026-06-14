#pragma once

#include <cstdint>

namespace yuengine::audio {
struct AudioSourceId final {
    std::uint32_t Slot = 0U;
    std::uint32_t Generation = 0U;
};
}
