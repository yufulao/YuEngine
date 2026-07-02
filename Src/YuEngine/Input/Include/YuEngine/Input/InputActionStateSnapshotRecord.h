// Module: YuEngine Input
// File: Src/YuEngine/Input/Include/YuEngine/Input/InputActionStateSnapshotRecord.h

#pragma once

#include "YuEngine/Input/InputActionId.h"
#include "YuEngine/Input/InputActionState.h"
#include "YuEngine/Input/InputContextId.h"

namespace yuengine::input {
struct InputActionStateSnapshotRecord final {
    InputContextId context{};
    InputActionId action{};
    InputActionState state{};
};
}
