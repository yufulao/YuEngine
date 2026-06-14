#include "YuEngine/File/MountTable.h"

#include <string>
#include <utility>

namespace yuengine::file {
namespace {
FileStatus PushPathSegment(std::vector<std::string>& segments, const std::string& segment) {
    if (segment.empty()) {
        return FileStatus::Success;
    }

    if (segment == ".") {
        return FileStatus::Success;
    }

    if (segment == "..") {
        if (segments.empty()) {
            return FileStatus::PathEscape;
        }

        segments.pop_back();
        return FileStatus::Success;
    }

    segments.push_back(segment);
    return FileStatus::Success;
}

bool IsAbsolutePath(std::string_view value) {
    if (value.empty()) {
        return false;
    }

    if (value.front() == '/') {
        return true;
    }

    if (value.front() == '\\') {
        return true;
    }

    if (value.size() < 2U) {
        return false;
    }

    return value[1U] == ':';
}

PathNormalizationResult NormalizePathValue(std::string_view value) {
    if (value.empty()) {
        return PathNormalizationResult::Failure(FileStatus::InvalidPath);
    }

    if (value.size() > MAX_VIRTUAL_PATH_LENGTH) {
        return PathNormalizationResult::Failure(FileStatus::PathTooLong);
    }

    if (IsAbsolutePath(value)) {
        return PathNormalizationResult::Failure(FileStatus::InvalidPath);
    }

    std::vector<std::string> segments;
    std::string segment;
    for (const char character : value) {
        if (character == '\\') {
            return PathNormalizationResult::Failure(FileStatus::InvalidPath);
        }

        if (character == '/') {
            const FileStatus status = PushPathSegment(segments, segment);
            if (status != FileStatus::Success) {
                return PathNormalizationResult::Failure(status);
            }

            segment.clear();
            continue;
        }

        segment.push_back(character);
    }

    const FileStatus finalStatus = PushPathSegment(segments, segment);
    if (finalStatus != FileStatus::Success) {
        return PathNormalizationResult::Failure(finalStatus);
    }

    std::string normalizedValue;
    for (const std::string& normalizedSegment : segments) {
        if (!normalizedValue.empty()) {
            normalizedValue.push_back('/');
        }

        normalizedValue.append(normalizedSegment);
        if (normalizedValue.size() > MAX_NORMALIZED_PATH_LENGTH) {
            return PathNormalizationResult::Failure(FileStatus::PathTooLong);
        }
    }

    if (normalizedValue.empty()) {
        return PathNormalizationResult::Failure(FileStatus::InvalidPath);
    }

    return PathNormalizationResult::Success(NormalizedPath(std::move(normalizedValue)));
}
}

MountTable::MountTable()
    : mounts_(),
      mount_count_(0U),
      snapshot_{
          0U,
          0U,
          0U,
          0U,
          0U,
          0U,
          yuengine::memory::MemoryAccountingStatus::ExplicitlyTrackedOnly,
          FileStatus::Success} {
}

FileStatus MountTable::RegisterLooseMount(MountId mountId, std::filesystem::path rootPath) {
    if (!mountId.IsValid()) {
        return FileStatus::InvalidMount;
    }

    if (FindMountIndex(mountId).has_value()) {
        return FileStatus::DuplicateMount;
    }

    if (mount_count_ >= MAX_MOUNT_COUNT) {
        return FileStatus::MountTableFull;
    }

    mounts_[mount_count_] = MountPoint(std::move(mountId), LooseFileSource(std::move(rootPath)));
    ++mount_count_;
    snapshot_.mount_count = mount_count_;
    return FileStatus::Success;
}

PathNormalizationResult MountTable::Normalize(VirtualPath path) {
    ++snapshot_.path_normalization_count;
    PathNormalizationResult result = NormalizePathValue(path.Value());
    if (!result.Succeeded()) {
        RecordRejectedPath();
    }

    return result;
}

FileReadResult MountTable::Read(FileReadRequest request) {
    PathNormalizationResult normalizedPath = Normalize(std::move(request.path));
    if (!normalizedPath.Succeeded()) {
        RecordLastReadStatus(normalizedPath.status);
        return FileReadResult::Failure(normalizedPath.status);
    }

    ++snapshot_.lookup_count;
    if (normalizedPath.path.ByteLength() > snapshot_.max_fixture_path_length) {
        snapshot_.max_fixture_path_length = normalizedPath.path.ByteLength();
    }

    const std::optional<std::size_t> mountIndex = FindMountIndex(request.mount);
    if (!mountIndex.has_value()) {
        RecordLastReadStatus(FileStatus::MountNotFound);
        return FileReadResult::Failure(FileStatus::MountNotFound);
    }

    FileReadResult result = mounts_[*mountIndex].Source().Read(normalizedPath.path);
    if (result.Succeeded()) {
        snapshot_.read_byte_count += result.bytes.size();
    }

    RecordLastReadStatus(result.status);
    return result;
}

FileSnapshot MountTable::Snapshot() const {
    return snapshot_;
}

std::vector<MountId> MountTable::MountOrder() const {
    std::vector<MountId> order;
    order.reserve(mount_count_);
    std::size_t inspectedCount = 0U;
    for (const MountPoint& mount : mounts_) {
        if (inspectedCount >= mount_count_) {
            return order;
        }

        order.push_back(mount.Id());
        ++inspectedCount;
    }

    return order;
}

std::optional<std::size_t> MountTable::FindMountIndex(MountId mountId) const {
    std::size_t index = 0U;
    for (const MountPoint& mount : mounts_) {
        if (index >= mount_count_) {
            return std::nullopt;
        }

        if (mount.Id().Value() == mountId.Value()) {
            return index;
        }

        ++index;
    }

    return std::nullopt;
}

void MountTable::RecordRejectedPath() {
    ++snapshot_.rejected_path_count;
}

void MountTable::RecordLastReadStatus(FileStatus status) {
    snapshot_.last_read_status = status;
}
}
