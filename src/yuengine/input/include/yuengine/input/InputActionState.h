#pragma once

#include <cstdint>

namespace yuengine::input
{
struct InputActionState final
{
    bool IsPressed = false;
    bool ChangedThisFrame = false;
    std::int32_t AxisValue = 0;
};
}
