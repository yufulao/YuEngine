// 模块: YuEngine Script
// 文件: Src/YuEngine/Script/Include/YuEngine/Script/ScriptNativeRegistryDesc.h

#pragma once

#include <cstdint>

#include "YuEngine/Script/ScriptConstants.h"

namespace yuengine::script {
struct ScriptNativeRegistryDesc final {
    std::uint32_t binding_capacity = MAX_SCRIPT_NATIVE_BINDING_COUNT;
};
}
