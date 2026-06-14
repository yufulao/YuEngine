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
