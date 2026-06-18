// 模块: YuEngine Kernel
// 文件: Src/YuEngine/Kernel/Src/ServiceRegistry.cpp

#include "YuEngine/Kernel/ServiceRegistry.h"

namespace yuengine::kernel {
bool ServiceRegistry::Contains(std::string_view service_id) const {
    return services_.contains(std::string(service_id));
}

bool ServiceRegistry::OwnerHasServices(std::string_view owner_module) const {
    for (const auto& service_entry : services_) {
        if (service_entry.second.owner_module == owner_module) {
            return true;
        }
    }

    return false;
}

void ServiceRegistry::UnregisterOwner(std::string_view owner_module) {
    for (auto iterator = services_.begin(); iterator != services_.end();) {
        if (iterator->second.owner_module == owner_module) {
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

bool ServiceRegistry::RegisterRaw(std::string_view owner_module, std::string_view service_id, void* service, std::type_index service_type) {
    if (!accepting_registrations_) {
        return false;
    }

    if (owner_module.empty()) {
        return false;
    }

    if (service_id.empty()) {
        return false;
    }

    if (service == nullptr) {
        return false;
    }

    const std::string service_key(service_id);
    if (services_.contains(service_key)) {
        return false;
    }

    services_.emplace(service_key, ServiceRecord{service, service_type, std::string(owner_module)});
    return true;
}

void* ServiceRegistry::ResolveRaw(std::string_view service_id, std::type_index service_type) const {
    const auto iterator = services_.find(std::string(service_id));
    if (iterator == services_.end()) {
        return nullptr;
    }

    if (iterator->second.type != service_type) {
        return nullptr;
    }

    return iterator->second.instance;
}
}
