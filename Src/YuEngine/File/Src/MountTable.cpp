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
    : _mounts(),
      _mountCount(0U),
      _snapshot{
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

    if (_mountCount >= MAX_MOUNT_COUNT) {
        return FileStatus::MountTableFull;
    }

    _mounts[_mountCount] = MountPoint(std::move(mountId), LooseFileSource(std::move(rootPath)));
    ++_mountCount;
    _snapshot.mount_count = _mountCount;
    return FileStatus::Success;
}

PathNormalizationResult MountTable::Normalize(VirtualPath path) {
    ++_snapshot.path_normalization_count;
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

    ++_snapshot.lookup_count;
    if (normalizedPath.path.ByteLength() > _snapshot.max_fixture_path_length) {
        _snapshot.max_fixture_path_length = normalizedPath.path.ByteLength();
    }

    const std::optional<std::size_t> mountIndex = FindMountIndex(request.mount);
    if (!mountIndex.has_value()) {
        RecordLastReadStatus(FileStatus::MountNotFound);
        return FileReadResult::Failure(FileStatus::MountNotFound);
    }

    FileReadResult result = _mounts[*mountIndex].Source().Read(normalizedPath.path);
    if (result.Succeeded()) {
        _snapshot.read_byte_count += result.bytes.size();
    }

    RecordLastReadStatus(result.status);
    return result;
}

FileSnapshot MountTable::Snapshot() const {
    return _snapshot;
}

std::vector<MountId> MountTable::MountOrder() const {
    std::vector<MountId> order;
    order.reserve(_mountCount);
    std::size_t inspectedCount = 0U;
    for (const MountPoint& mount : _mounts) {
        if (inspectedCount >= _mountCount) {
            return order;
        }

        order.push_back(mount.Id());
        ++inspectedCount;
    }

    return order;
}

std::optional<std::size_t> MountTable::FindMountIndex(MountId mountId) const {
    std::size_t index = 0U;
    for (const MountPoint& mount : _mounts) {
        if (index >= _mountCount) {
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
    ++_snapshot.rejected_path_count;
}

void MountTable::RecordLastReadStatus(FileStatus status) {
    _snapshot.last_read_status = status;
}
}
