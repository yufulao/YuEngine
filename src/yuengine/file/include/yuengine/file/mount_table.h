#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <vector>

#include "yuengine/file/file_constants.h"
#include "yuengine/file/file_read_request.h"
#include "yuengine/file/file_read_result.h"
#include "yuengine/file/file_snapshot.h"
#include "yuengine/file/mount_point.h"
#include "yuengine/file/path_normalization_result.h"

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

    std::array<MountPoint, MAX_MOUNT_COUNT> _mounts;
    std::size_t _mountCount;
    FileSnapshot _snapshot;
};
}
