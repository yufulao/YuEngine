// Module: YuEngine Input
// File: Src/YuEngine/Input/Include/YuEngine/Input/InputActionQueryResult.h

#pragma once

#include "YuEngine/Input/InputActionState.h"
#include "YuEngine/Input/InputStatus.h"

namespace yuengine::input {
struct InputActionQueryResult final {
    InputStatus status = InputStatus::Success;
    InputActionState state;
};
}
