// Module: YuEngine Kernel
// File: Src/YuEngine/Kernel/Include/YuEngine/Kernel/ServiceRegistry.h

#pragma once

#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>

#include "YuEngine/Kernel/ServiceRecord.h"

namespace yuengine::kernel {
class ServiceRegistry final {
public:
    static constexpr const char* LOOKUP_POLICY = "SETUP_PATH_ONLY_CACHE_POINTERS_FOR_HOT_PATHS";

    /**
     * @comment Registers the service instance.
     * @param owner_module Input owner module.
     * @param service_id Input service id.
     * @param service Service updated by the function.
     * @return True when the condition is satisfied; false otherwise.
     */
    template <typename T>
    bool Register(std::string_view owner_module, std::string_view service_id, T& service) {
        return RegisterRaw(owner_module, service_id, &service, std::type_index(typeid(T)));
    }

    /**
     * @comment Resolves the service instance.
     * @param service_id Input service id.
     * @return Pointer to the requested object, or nullptr when unavailable.
     */
    template <typename T>
    T* Resolve(std::string_view service_id) const {
        void* service = ResolveRaw(service_id, std::type_index(typeid(T)));
        if (service == nullptr) {
            return nullptr;
        }

        return static_cast<T*>(service);
    }

    /**
     * @comment Checks whether the registry contains the requested item.
     * @param service_id Input service id.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool Contains(std::string_view service_id) const;
    /**
     * @comment Checks whether the owner has registered services.
     * @param owner_module Input owner module.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool OwnerHasServices(std::string_view owner_module) const;
    /**
     * @comment Unregisters all services for the owner.
     * @param owner_module Input owner module.
     */
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
