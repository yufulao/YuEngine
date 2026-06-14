#include "yuengine/kernel/ServiceRegistry.h"

namespace yuengine::kernel {
bool ServiceRegistry::Contains(std::string_view serviceId) const {
    return _services.contains(std::string(serviceId));
}

bool ServiceRegistry::OwnerHasServices(std::string_view ownerModule) const {
    for (const auto& serviceEntry : _services) {
        if (serviceEntry.second.OwnerModule == ownerModule) {
            return true;
        }
    }

    return false;
}

void ServiceRegistry::UnregisterOwner(std::string_view ownerModule) {
    for (auto iterator = _services.begin(); iterator != _services.end();) {
        if (iterator->second.OwnerModule == ownerModule) {
            iterator = _services.erase(iterator);
            continue;
        }

        ++iterator;
    }
}

void ServiceRegistry::OpenRegistrationWindow() {
    _acceptingRegistrations = true;
}

void ServiceRegistry::CloseRegistrationWindow() {
    _acceptingRegistrations = false;
}

bool ServiceRegistry::RegisterRaw(std::string_view ownerModule, std::string_view serviceId, void* service, std::type_index serviceType) {
    if (!_acceptingRegistrations) {
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
    if (_services.contains(serviceKey)) {
        return false;
    }

    _services.emplace(serviceKey, ServiceRecord{service, serviceType, std::string(ownerModule)});
    return true;
}

void* ServiceRegistry::ResolveRaw(std::string_view serviceId, std::type_index serviceType) const {
    const auto iterator = _services.find(std::string(serviceId));
    if (iterator == _services.end()) {
        return nullptr;
    }

    if (iterator->second.Type != serviceType) {
        return nullptr;
    }

    return iterator->second.Instance;
}
}
