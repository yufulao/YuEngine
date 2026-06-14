#include "YuEngine/Kernel/ServiceRegistry.h"

namespace yuengine::kernel {
bool ServiceRegistry::Contains(std::string_view serviceId) const {
    return services_.contains(std::string(serviceId));
}

bool ServiceRegistry::OwnerHasServices(std::string_view ownerModule) const {
    for (const auto& serviceEntry : services_) {
        if (serviceEntry.second.owner_module == ownerModule) {
            return true;
        }
    }

    return false;
}

void ServiceRegistry::UnregisterOwner(std::string_view ownerModule) {
    for (auto iterator = services_.begin(); iterator != services_.end();) {
        if (iterator->second.owner_module == ownerModule) {
            iterator = services_.erase(iterator);
            continue;
        }

        ++iterator;
    }
}

void ServiceRegistry::OpenRegistrationWindow() {
    accepting_registrations_ = true;
}

void ServiceRegistry::CloseRegistrationWindow() {
    accepting_registrations_ = false;
}

bool ServiceRegistry::RegisterRaw(std::string_view ownerModule, std::string_view serviceId, void* service, std::type_index serviceType) {
    if (!accepting_registrations_) {
        return false;
    }

    if (ownerModule.empty()) {
        return false;
    }

    if (serviceId.empty()) {
        return false;
    }

    if (service == nullptr) {
        return false;
    }

    const std::string serviceKey(serviceId);
    if (services_.contains(serviceKey)) {
        return false;
    }

    services_.emplace(serviceKey, ServiceRecord{service, serviceType, std::string(ownerModule)});
    return true;
}

void* ServiceRegistry::ResolveRaw(std::string_view serviceId, std::type_index serviceType) const {
    const auto iterator = services_.find(std::string(serviceId));
    if (iterator == services_.end()) {
        return nullptr;
    }

    if (iterator->second.type != serviceType) {
        return nullptr;
    }

    return iterator->second.instance;
}
}
