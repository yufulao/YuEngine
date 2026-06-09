#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace yuengine::kernel
{
class ServiceRegistry final
{
public:
    static constexpr const char* LookupPolicy = "SETUP_PATH_ONLY_CACHE_POINTERS_FOR_HOT_PATHS";

    template <typename T>
    bool Register(std::string_view ownerModule, std::string_view serviceId, T& service)
    {
        return RegisterRaw(ownerModule, serviceId, &service);
    }

    template <typename T>
    T* Resolve(std::string_view serviceId) const
    {
        void* service = ResolveRaw(serviceId);
        if (service == nullptr)
        {
            return nullptr;
        }

        return static_cast<T*>(service);
    }

    bool Contains(std::string_view serviceId) const;
    bool OwnerHasServices(std::string_view ownerModule) const;
    void UnregisterOwner(std::string_view ownerModule);

private:
    struct ServiceRecord
    {
        void* Instance;
        std::string OwnerModule;
    };

    bool RegisterRaw(std::string_view ownerModule, std::string_view serviceId, void* service);
    void* ResolveRaw(std::string_view serviceId) const;

    std::unordered_map<std::string, ServiceRecord> _services;
};
}
