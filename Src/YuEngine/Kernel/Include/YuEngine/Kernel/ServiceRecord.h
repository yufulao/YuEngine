// 模块: YuEngine Kernel
// 文件: Src/YuEngine/Kernel/Include/YuEngine/Kernel/ServiceRecord.h

#pragma once

#include <string>
#include <typeindex>

namespace yuengine::kernel {
struct ServiceRecord {
    void* instance;
    std::type_index type;
    std::string owner_module;
};
}
