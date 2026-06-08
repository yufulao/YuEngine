#include "yuengine/runtime/RuntimeContext.h"

#include <stdexcept>
#include <utility>

namespace yu::runtime {

void ServiceContainer::setManifest(project::ProjectManifest manifest)
{
    manifest_ = std::move(manifest);
    hasManifest_ = true;
}

const project::ProjectManifest& ServiceContainer::manifest() const
{
    if (!hasManifest_) {
        throw std::runtime_error("runtime project manifest is not loaded");
    }
    return manifest_;
}

project::ProjectManifest& ServiceContainer::manifest()
{
    if (!hasManifest_) {
        throw std::runtime_error("runtime project manifest is not loaded");
    }
    return manifest_;
}

bool ServiceContainer::hasManifest() const
{
    return hasManifest_;
}

resource::VirtualFileSystem& ServiceContainer::vfs()
{
    return vfs_;
}

const resource::VirtualFileSystem& ServiceContainer::vfs() const
{
    return vfs_;
}

native::NativeRegistry& ServiceContainer::nativeRegistry()
{
    return nativeRegistry_;
}

const native::NativeRegistry& ServiceContainer::nativeRegistry() const
{
    return nativeRegistry_;
}

native::NativeServiceCatalog& ServiceContainer::nativeServices()
{
    return nativeServices_;
}

const native::NativeServiceCatalog& ServiceContainer::nativeServices() const
{
    return nativeServices_;
}

std::vector<std::filesystem::path> ServiceContainer::scriptRoots() const
{
    std::vector<std::filesystem::path> roots;
    for (const auto& root : manifest().scriptRoots) {
        roots.push_back(root.path);
    }
    return roots;
}

ServiceContainer& RuntimeContext::services()
{
    return services_;
}

const ServiceContainer& RuntimeContext::services() const
{
    return services_;
}

void RuntimeContext::recordPhase(std::string name, std::string status, std::string detail)
{
    phases_.push_back({std::move(name), std::move(status), std::move(detail)});
}

const std::vector<BootPhaseReport>& RuntimeContext::phases() const
{
    return phases_;
}

} // namespace yu::runtime
