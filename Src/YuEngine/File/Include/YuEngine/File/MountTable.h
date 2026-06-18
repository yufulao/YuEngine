// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/MountTable.h

#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <vector>

#include "YuEngine/File/FileConstants.h"
#include "YuEngine/File/FileReadRequest.h"
#include "YuEngine/File/FileReadResult.h"
#include "YuEngine/File/FileSnapshot.h"
#include "YuEngine/File/FileWriteRequest.h"
#include "YuEngine/File/FileWriteResult.h"
#include "YuEngine/File/MountPoint.h"
#include "YuEngine/File/PathNormalizationResult.h"

namespace yuengine::file {
class MountTable final {
public:
    /**
     * @comment 构造 MountTable 实例。
     */
    MountTable();

    /**
     * @comment 注册 loose mount。
     * @param mount_id 输入 mount id。
     * @param root_path 输入 根路径。
     * @return 显式操作状态。
     */
    FileStatus RegisterLooseMount(MountId mount_id, std::filesystem::path root_path);
    /**
     * @comment 归一化操作。
     * @param path 输入 path。
     * @return 显式操作结果。
     */
    PathNormalizationResult Normalize(VirtualPath path);
    /**
     * @comment 读取操作。
     * @param request 输入 请求。
     * @return 显式操作结果。
     */
    FileReadResult Read(FileReadRequest request);
    /**
     * @comment 写入 操作。
     * @param request 输入 请求。
     * @return 显式操作结果。
     */
    FileWriteResult Write(FileWriteRequest request);
    /**
     * @comment 返回当前状态快照。
     * @return 快照值。
     */
    FileSnapshot Snapshot() const;
    /**
     * @comment 返回 mount identifiers 在 lookup order。
     * @return Mount order 值。
     */
    std::vector<MountId> MountOrder() const;

private:
    std::optional<std::size_t> FindMountIndex(MountId mount_id) const;
    void RecordRejectedPath();
    void RecordLastReadStatus(FileStatus status);
    void RecordLastWriteStatus(FileStatus status);

    std::array<MountPoint, MAX_MOUNT_COUNT> mounts_;
    std::size_t mount_count_;
    FileSnapshot snapshot_;
};
}
