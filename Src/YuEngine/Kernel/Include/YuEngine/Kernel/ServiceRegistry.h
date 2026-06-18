// 模块: YuEngine Kernel
// 文件: Src/YuEngine/Kernel/Include/YuEngine/Kernel/ServiceRegistry.h

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
     * @comment 注册 服务实例.
     * @param owner_module 输入 所有者模块。
     * @param service_id 输入 服务 id。
     * @param service 函数写入的 Service。
     * @return 条件满足时返回 true，否则返回 false。
     */
    template <typename T>
    bool Register(std::string_view owner_module, std::string_view service_id, T& service) {
        return RegisterRaw(owner_module, service_id, &service, std::type_index(typeid(T)));
    }

    /**
     * @comment 解析 服务实例。
     * @param service_id 输入 服务 id。
     * @return 请求对象指针；不可用时返回 nullptr。
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
     * @comment 检查 registry contains 请求的 item.
     * @param service_id 输入 服务 id。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool Contains(std::string_view service_id) const;
    /**
     * @comment 检查 所有者 是否已注册服务。
     * @param owner_module 输入 所有者模块。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool OwnerHasServices(std::string_view owner_module) const;
    /**
     * @comment 注销 所有者的全部服务。
     * @param owner_module 输入 所有者模块。
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
