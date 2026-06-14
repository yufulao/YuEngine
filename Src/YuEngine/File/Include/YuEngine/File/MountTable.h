#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <vector>

#include "YuEngine/File/FileConstants.h"
#include "YuEngine/File/FileReadRequest.h"
#include "YuEngine/File/FileReadResult.h"
#include "YuEngine/File/FileSnapshot.h"
#include "YuEngine/File/MountPoint.h"
#include "YuEngine/File/PathNormalizationResult.h"

namespace yuengine::file {
class MountTable final {
public:
    MountTable();

    FileStatus RegisterLooseMount(MountId mountId, std::filesystem::path rootPath);
    PathNormalizationResult Normalize(VirtualPath path);
    FileReadResult Read(FileReadRequest request);
    FileSnapshot Snapshot() const;
    std::vector<MountId> MountOrder() const;

private:
    std::optional<std::size_t> FindMountIndex(MountId mountId) const;
    void RecordRejectedPath();
    void RecordLastReadStatus(FileStatus status);

    std::array<MountPoint, MAX_MOUNT_COUNT> mounts_;
    std::size_t mount_count_;
    FileSnapshot snapshot_;
};
}
