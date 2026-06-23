// 模块: YuEngine Package
// 文件: Src/YuEngine/Package/Include/YuEngine/Package/PackageArtifact.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/File/FileStatus.h"
#include "YuEngine/File/MountId.h"
#include "YuEngine/File/MountTable.h"
#include "YuEngine/File/VirtualPath.h"
#include "YuEngine/Package/PackageEntryDescriptor.h"
#include "YuEngine/Package/PackageEntryId.h"
#include "YuEngine/Package/PackageId.h"
#include "YuEngine/Package/PackageRegistry.h"
#include "YuEngine/Package/PackageRegistryDesc.h"
#include "YuEngine/Package/PackageStatus.h"

namespace yuengine::package {
struct PackageArtifactDependency final {
    PackageEntryId dependent;
    PackageEntryId dependency;
};

struct PackageArtifactWriteRequest final {
    yuengine::file::MountTable *mount_table = nullptr;
    yuengine::file::MountId mount;
    yuengine::file::VirtualPath artifact_path;
    PackageId package;
    const PackageEntryDescriptor *entries = nullptr;
    std::uint32_t entry_count = 0U;
    const PackageArtifactDependency *dependencies = nullptr;
    std::uint32_t dependency_count = 0U;
};

struct PackageArtifactReadRequest final {
    yuengine::file::MountTable *mount_table = nullptr;
    yuengine::file::MountId mount;
    yuengine::file::VirtualPath artifact_path;
    PackageRegistry *registry = nullptr;
    PackageRegistryDesc registry_desc{};
};

struct PackageArtifactResult final {
    PackageStatus status = PackageStatus::InvalidArtifact;
    yuengine::file::FileStatus file_status = yuengine::file::FileStatus::Success;
    std::size_t artifact_byte_count = 0U;
    std::uint32_t manifest_count = 0U;
    std::uint32_t entry_count = 0U;
    std::uint32_t dependency_count = 0U;
    bool wrote_artifact = false;
    bool read_artifact = false;
    bool rebuilt_registry = false;
};

PackageArtifactResult WritePackageArtifact(const PackageArtifactWriteRequest &request);
PackageArtifactResult ReadPackageArtifact(const PackageArtifactReadRequest &request);
}
