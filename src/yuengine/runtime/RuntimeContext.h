#pragma once

#include "yuengine/native/NativeRegistry.h"
#include "yuengine/native/NativeServices.h"
#include "yuengine/project/ProjectManifest.h"
#include "yuengine/resource/VirtualFileSystem.h"

#include <filesystem>
#include <string>
#include <vector>

namespace yu::runtime {

struct BootPhaseReport {
    std::string name;
    std::string status;
    std::string detail;
};

class ServiceContainer {
public:
    void setManifest(project::ProjectManifest manifest);
    const project::ProjectManifest& manifest() const;
    project::ProjectManifest& manifest();
    bool hasManifest() const;

    resource::VirtualFileSystem& vfs();
    const resource::VirtualFileSystem& vfs() const;

    native::NativeRegistry& nativeRegistry();
    const native::NativeRegistry& nativeRegistry() const;

    native::NativeServiceCatalog& nativeServices();
    const native::NativeServiceCatalog& nativeServices() const;

    std::vector<std::filesystem::path> scriptRoots() const;

private:
    bool hasManifest_ = false;
    project::ProjectManifest manifest_;
    resource::VirtualFileSystem vfs_;
    native::NativeRegistry nativeRegistry_;
    native::NativeServiceCatalog nativeServices_;
};

class RuntimeContext {
public:
    ServiceContainer& services();
    const ServiceContainer& services() const;

    void recordPhase(std::string name, std::string status, std::string detail);
    const std::vector<BootPhaseReport>& phases() const;

private:
    ServiceContainer services_;
    std::vector<BootPhaseReport> phases_;
};

} // namespace yu::runtime
