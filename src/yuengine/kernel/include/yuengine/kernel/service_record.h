#pragma once

#include <string>
#include <typeindex>

namespace yuengine::kernel {
struct service_record_t {
    void* Instance;
    std::type_index Type;
    std::string OwnerModule;
};
}
