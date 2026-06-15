// Module: YuEngine Kernel
// File: Src/YuEngine/Kernel/Include/YuEngine/Kernel/IModule.h

#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "YuEngine/Kernel/KernelResult.h"
#include "YuEngine/Kernel/ServiceRegistry.h"

namespace yuengine::kernel {
class IModule {
public:
    virtual ~IModule() = default;

    /**
     * @comment Returns the module name.
     * @return Name value.
     */
    virtual std::string_view Name() const = 0;
    /**
     * @comment Returns declared module dependencies.
     * @return Dependencies value.
     */
    virtual std::vector<std::string_view> Dependencies() const = 0;
    /**
     * @comment Returns required service identifiers.
     * @return Required services value.
     */
    virtual std::vector<std::string_view> RequiredServices() const = 0;
    /**
     * @comment Returns published service identifiers.
     * @return Published services value.
     */
    virtual std::vector<std::string_view> PublishedServices() const = 0;
    /**
     * @comment Starts the component.
     * @param service_registry Service registry updated by the function.
     * @param lifecycle_trace Lifecycle trace updated by the function.
     * @return Explicit operation result.
     */
    virtual KernelResult Start(ServiceRegistry& service_registry, std::vector<std::string>& lifecycle_trace) = 0;
    /**
     * @comment Updates the component for one frame.
     * @param frame_index Input frame index.
     * @param tick_time_nanoseconds Input tick time nanoseconds.
     * @param lifecycle_trace Lifecycle trace updated by the function.
     * @return Explicit operation result.
     */
    virtual KernelResult Update(std::uint32_t frame_index, std::uint64_t tick_time_nanoseconds, std::vector<std::string>& lifecycle_trace) = 0;
    /**
     * @comment Shuts down the component.
     * @param lifecycle_trace Lifecycle trace updated by the function.
     * @return Explicit operation result.
     */
    virtual KernelResult Shutdown(std::vector<std::string>& lifecycle_trace) = 0;
};
}
