// Module: YuEngine File
// File: Src/YuEngine/File/Include/YuEngine/File/MountTable.h

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
    /**
     * @comment Constructs a MountTable instance.
     */
    MountTable();

    /**
     * @comment Registers loose mount.
     * @param mount_id Input mount id.
     * @param root_path Input root path.
     * @return Explicit operation status.
     */
    FileStatus RegisterLooseMount(MountId mount_id, std::filesystem::path root_path);
    /**
     * @comment Normalizes the operation.
     * @param path Input path.
     * @return Explicit operation result.
     */
    PathNormalizationResult Normalize(VirtualPath path);
    /**
     * @comment Reads the operation.
     * @param request Input request.
     * @return Explicit operation result.
     */
    FileReadResult Read(FileReadRequest request);
    /**
     * @comment Returns a snapshot of the current state.
     * @return Snapshot value.
     */
    FileSnapshot Snapshot() const;
    /**
     * @comment Returns mount identifiers in lookup order.
     * @return Mount order value.
     */
    std::vector<MountId> MountOrder() const;

private:
    std::optional<std::size_t> FindMountIndex(MountId mount_id) const;
    void RecordRejectedPath();
    void RecordLastReadStatus(FileStatus status);

    std::array<MountPoint, MAX_MOUNT_COUNT> mounts_;
    std::size_t mount_count_;
    FileSnapshot snapshot_;
};
}
