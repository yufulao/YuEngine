#include "yuengine/file/mount_table.h"

#include <string>
#include <utility>

namespace yuengine::file {
namespace {
FILE_STATUS PushPathSegment(std::vector<std::string>& segments, const std::string& segment) {
    if (segment.empty()) {
        return FILE_STATUS::Success;
    }

    if (segment == ".") {
        return FILE_STATUS::Success;
    }

    if (segment == "..") {
        if (segments.empty()) {
            return FILE_STATUS::PathEscape;
        }

        segments.pop_back();
        return FILE_STATUS::Success;
    }

    segments.push_back(segment);
    return FILE_STATUS::Success;
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

path_normalization_result_t NormalizePathValue(std::string_view value) {
    if (value.empty()) {
        return path_normalization_result_t::Failure(FILE_STATUS::InvalidPath);
    }

    if (value.size() > MAX_VIRTUAL_PATH_LENGTH) {
        return path_normalization_result_t::Failure(FILE_STATUS::PathTooLong);
    }

    if (IsAbsolutePath(value)) {
        return path_normalization_result_t::Failure(FILE_STATUS::InvalidPath);
    }

    std::vector<std::string> segments;
    std::string segment;
    for (const char character : value) {
        if (character == '\\') {
            return path_normalization_result_t::Failure(FILE_STATUS::InvalidPath);
        }

        if (character == '/') {
            const FILE_STATUS status = PushPathSegment(segments, segment);
            if (status != FILE_STATUS::Success) {
                return path_normalization_result_t::Failure(status);
            }

            segment.clear();
            continue;
        }

        segment.push_back(character);
    }

    const FILE_STATUS finalStatus = PushPathSegment(segments, segment);
    if (finalStatus != FILE_STATUS::Success) {
        return path_normalization_result_t::Failure(finalStatus);
    }

    std::string normalizedValue;
    for (const std::string& normalizedSegment : segments) {
        if (!normalizedValue.empty()) {
            normalizedValue.push_back('/');
        }

        normalizedValue.append(normalizedSegment);
        if (normalizedValue.size() > MAX_NORMALIZED_PATH_LENGTH) {
            return path_normalization_result_t::Failure(FILE_STATUS::PathTooLong);
        }
    }

    if (normalizedValue.empty()) {
        return path_normalization_result_t::Failure(FILE_STATUS::InvalidPath);
    }

    return path_normalization_result_t::Success(NormalizedPath(std::move(normalizedValue)));
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
          yuengine::memory::MEMORY_ACCOUNTING_STATUS::ExplicitlyTrackedOnly,
          FILE_STATUS::Success} {
}

FILE_STATUS MountTable::RegisterLooseMount(MountId mountId, std::filesystem::path rootPath) {
    if (!mountId.IsValid()) {
        return FILE_STATUS::InvalidMount;
    }

    if (FindMountIndex(mountId).has_value()) {
        return FILE_STATUS::DuplicateMount;
    }

    if (_mountCount >= MAX_MOUNT_COUNT) {
        return FILE_STATUS::MountTableFull;
    }

    _mounts[_mountCount] = MountPoint(std::move(mountId), LooseFileSource(std::move(rootPath)));
    ++_mountCount;
    _snapshot.MountCount = _mountCount;
    return FILE_STATUS::Success;
}

path_normalization_result_t MountTable::Normalize(VirtualPath path) {
    ++_snapshot.PathNormalizationCount;
    path_normalization_result_t result = NormalizePathValue(path.Value());
    if (!result.Succeeded()) {
        RecordRejectedPath();
    }

    return result;
}

file_read_result_t MountTable::Read(file_read_request_t request) {
    path_normalization_result_t normalizedPath = Normalize(std::move(request.Path));
    if (!normalizedPath.Succeeded()) {
        RecordLastReadStatus(normalizedPath.Status);
        return file_read_result_t::Failure(normalizedPath.Status);
    }

    ++_snapshot.LookupCount;
    if (normalizedPath.Path.ByteLength() > _snapshot.MaxFixturePathLength) {
        _snapshot.MaxFixturePathLength = normalizedPath.Path.ByteLength();
    }

    const std::optional<std::size_t> mountIndex = FindMountIndex(request.Mount);
    if (!mountIndex.has_value()) {
        RecordLastReadStatus(FILE_STATUS::MountNotFound);
        return file_read_result_t::Failure(FILE_STATUS::MountNotFound);
    }

    file_read_result_t result = _mounts[*mountIndex].Source().Read(normalizedPath.Path);
    if (result.Succeeded()) {
        _snapshot.ReadByteCount += result.Bytes.size();
    }

    RecordLastReadStatus(result.Status);
    return result;
}

file_snapshot_t MountTable::Snapshot() const {
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
    ++_snapshot.RejectedPathCount;
}

void MountTable::RecordLastReadStatus(FILE_STATUS status) {
    _snapshot.LastReadStatus = status;
}
}
