#pragma once

#include <string>
#include <typeindex>

namespace yuengine::kernel
{
struct ServiceRecord
{
    void* Instance;
    std::type_index Type;
    std::string OwnerModule;
};
}
