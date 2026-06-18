// 模块: YuEngine World
// 文件: Src/YuEngine/World/Src/WorldKernelModule.cpp

#include "YuEngine/World/WorldKernelModule.h"

#include "YuEngine/World/WorldServiceIds.h"

namespace yuengine::world {
namespace {
constexpr const char *TRACE_WORLD_START = "world.module.start";
constexpr const char *TRACE_WORLD_UPDATE = "world.module.update";
constexpr const char *TRACE_WORLD_SHUTDOWN = "world.module.shutdown";
constexpr const char *WORLD_START_FAILED_MESSAGE = "world start failed";
constexpr const char *WORLD_SERVICE_PUBLICATION_FAILED_MESSAGE = "world service publication failed";
constexpr const char *WORLD_UPDATE_FAILED_MESSAGE = "world update failed";
constexpr const char *WORLD_SHUTDOWN_FAILED_MESSAGE = "world shutdown failed";
}

WorldKernelModule::WorldKernelModule(WorldInstance &world, WorldKernelModuleDesc desc)
    : world_(world),
      module_name_(NormalizeText(desc.module_name, WORLD_KERNEL_MODULE_NAME)),
      world_service_id_(NormalizeText(desc.world_service_id, WORLD_INSTANCE_SERVICE_ID)),
      fixed_step_duration_(desc.fixed_step_duration),
      publish_world_service_(desc.publish_world_service) {
}

std::string_view WorldKernelModule::Name() const {
    return module_name_;
}

std::vector<std::string_view> WorldKernelModule::Dependencies() const {
    return std::vector<std::string_view>();
}

std::vector<std::string_view> WorldKernelModule::RequiredServices() const {
    return std::vector<std::string_view>();
}

std::vector<std::string_view> WorldKernelModule::PublishedServices() const {
    if (!publish_world_service_) {
        return std::vector<std::string_view>();
    }

    return std::vector<std::string_view>{world_service_id_};
}

kernel::KernelResult WorldKernelModule::Start(kernel::ServiceRegistry &service_registry, std::vector<std::string> &lifecycle_trace) {
    lifecycle_trace.push_back(TRACE_WORLD_START);

    if (publish_world_service_ && !service_registry.Register(Name(), world_service_id_, world_)) {
        return FailServicePublication();
    }

    const WorldStatus start_status = world_.Start();
    if (start_status != WorldStatus::Success) {
        return FailStart(WORLD_START_FAILED_MESSAGE);
    }

    world_started_ = true;
    return kernel::KernelResult::Success();
}

kernel::KernelResult WorldKernelModule::Update(std::uint32_t frame_index,
    std::uint64_t tick_time_nanoseconds,
    std::vector<std::string> &lifecycle_trace) {
    lifecycle_trace.push_back(TRACE_WORLD_UPDATE);

    const WorldStatus update_status = world_.Update(frame_index, fixed_step_duration_, tick_time_nanoseconds);
    if (update_status != WorldStatus::Success) {
        return FailUpdate();
    }

    return kernel::KernelResult::Success();
}

kernel::KernelResult WorldKernelModule::Shutdown(std::vector<std::string> &lifecycle_trace) {
    lifecycle_trace.push_back(TRACE_WORLD_SHUTDOWN);

    if (!world_started_) {
        return kernel::KernelResult::Success();
    }

    const WorldStatus shutdown_status = world_.Stop();
    if (shutdown_status != WorldStatus::Success) {
        return FailShutdown();
    }

    world_started_ = false;
    return kernel::KernelResult::Success();
}

const char *WorldKernelModule::NormalizeText(const char *text, const char *fallback) const {
    if (text == nullptr) {
        return fallback;
    }

    return text;
}

kernel::KernelResult WorldKernelModule::FailStart(const char *message) const {
    return kernel::KernelResult::Failure(kernel::KernelStatus::StartupFailure, message);
}

kernel::KernelResult WorldKernelModule::FailServicePublication() const {
    return kernel::KernelResult::Failure(kernel::KernelStatus::StartupFailure, WORLD_SERVICE_PUBLICATION_FAILED_MESSAGE);
}

kernel::KernelResult WorldKernelModule::FailUpdate() const {
    return kernel::KernelResult::Failure(kernel::KernelStatus::UpdateFailure, WORLD_UPDATE_FAILED_MESSAGE);
}

kernel::KernelResult WorldKernelModule::FailShutdown() const {
    return kernel::KernelResult::Failure(kernel::KernelStatus::ShutdownFailure, WORLD_SHUTDOWN_FAILED_MESSAGE);
}
}
