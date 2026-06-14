#pragma once

#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>

#include "YuEngine/Kernel/ServiceRecord.h"

namespace yuengine::kernel {
class ServiceRegistry final {
public:
    static constexpr const char* LookupPolicy = "SETUP_PATH_ONLY_CACHE_POINTERS_FOR_HOT_PATHS";

    template <typename T>
    bool Register(std::string_view owner_module, std::string_view service_id, T& service) {
        return RegisterRaw(owner_module, service_id, &service, std::type_index(typeid(T)));
    }

    template <typename T>
    T* Resolve(std::string_view service_id) const {
        void* service = ResolveRaw(service_id, std::type_index(typeid(T)));
        if (service == nullptr) {
            return nullptr;
        }

        return static_cast<T*>(service);
    }

    bool Contains(std::string_view service_id) const;
    bool OwnerHasServices(std::string_view owner_module) const;
    void UnregisterOwner(std::string_view owner_module);

private:
    friend class EngineKernel;

    bool RegisterRaw(std::string_view owner_module, std::string_view service_id, void* service, std::type_index service_type);
    void* ResolveRaw(std::string_view service_id, std::type_index service_type) const;
    void OpenRegistrationWindow();
    void CloseRegistrationWindow();

    std::unordered_map<std::string, ServiceRecord> services_;
    bool accepting_registrations_ = true;
};
}
