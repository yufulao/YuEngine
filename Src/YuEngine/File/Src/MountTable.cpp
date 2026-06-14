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

    const FileStatus final_status = PushPathSegment(segments, segment);
    if (final_status != FileStatus::Success) {
        return PathNormalizationResult::Failure(final_status);
    }

    std::string normalized_value;
    for (const std::string& normalized_segment : segments) {
        if (!normalized_value.empty()) {
            normalized_value.push_back('/');
        }

        normalized_value.append(normalized_segment);
        if (normalized_value.size() > MAX_NORMALIZED_PATH_LENGTH) {
            return PathNormalizationResult::Failure(FileStatus::PathTooLong);
        }
    }

    if (normalized_value.empty()) {
        return PathNormalizationResult::Failure(FileStatus::InvalidPath);
    }

    return PathNormalizationResult::Success(NormalizedPath(std::move(normalized_value)));
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

FileStatus MountTable::RegisterLooseMount(MountId mount_id, std::filesystem::path root_path) {
    if (!mount_id.IsValid()) {
        return FileStatus::InvalidMount;
    }

    if (FindMountIndex(mount_id).has_value()) {
        return FileStatus::DuplicateMount;
    }

    if (mount_count_ >= MAX_MOUNT_COUNT) {
        return FileStatus::MountTableFull;
    }

    mounts_[mount_count_] = MountPoint(std::move(mount_id), LooseFileSource(std::move(root_path)));
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
    PathNormalizationResult normalized_path = Normalize(std::move(request.path));
    if (!normalized_path.Succeeded()) {
        RecordLastReadStatus(normalized_path.status);
        return FileReadResult::Failure(normalized_path.status);
    }

    ++snapshot_.lookup_count;
    if (normalized_path.path.ByteLength() > snapshot_.max_fixture_path_length) {
        snapshot_.max_fixture_path_length = normalized_path.path.ByteLength();
    }

    const std::optional<std::size_t> mount_index = FindMountIndex(request.mount);
    if (!mount_index.has_value()) {
        RecordLastReadStatus(FileStatus::MountNotFound);
        return FileReadResult::Failure(FileStatus::MountNotFound);
    }

    FileReadResult result = mounts_[*mount_index].Source().Read(normalized_path.path);
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
    std::size_t inspected_count = 0U;
    for (const MountPoint& mount : mounts_) {
        if (inspected_count >= mount_count_) {
            return order;
        }

        order.push_back(mount.Id());
        ++inspected_count;
    }

    return order;
}

std::optional<std::size_t> MountTable::FindMountIndex(MountId mount_id) const {
    std::size_t index = 0U;
    for (const MountPoint& mount : mounts_) {
        if (index >= mount_count_) {
            return std::nullopt;
        }

        if (mount.Id().Value() == mount_id.Value()) {
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
