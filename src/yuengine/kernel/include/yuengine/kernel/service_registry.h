#pragma once

#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>

#include "yuengine/kernel/service_record.h"

namespace yuengine::kernel {
class ServiceRegistry final {
public:
    static constexpr const char* LookupPolicy = "SETUP_PATH_ONLY_CACHE_POINTERS_FOR_HOT_PATHS";

    template <typename T>
    bool Register(std::string_view ownerModule, std::string_view serviceId, T& service) {
        return RegisterRaw(ownerModule, serviceId, &service, std::type_index(typeid(T)));
    }

    template <typename T>
    T* Resolve(std::string_view serviceId) const {
        void* service = ResolveRaw(serviceId, std::type_index(typeid(T)));
        if (service == nullptr) {
            return nullptr;
        }

        return static_cast<T*>(service);
    }

    bool Contains(std::string_view serviceId) const;
    bool OwnerHasServices(std::string_view ownerModule) const;
    void UnregisterOwner(std::string_view ownerModule);

private:
    friend class EngineKernel;

    bool RegisterRaw(std::string_view ownerModule, std::string_view serviceId, void* service, std::type_index serviceType);
    void* ResolveRaw(std::string_view serviceId, std::type_index serviceType) const;
    void OpenRegistrationWindow();
    void CloseRegistrationWindow();

    std::unordered_map<std::string, ServiceRecord> _services;
    bool _acceptingRegistrations = true;
};
}
