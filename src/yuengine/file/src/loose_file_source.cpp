#include "yuengine/file/loose_file_source.h"

#include <fstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include "yuengine/file/file_constants.h"

namespace yuengine::file {
namespace {
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

bool IsCurrentDirectorySegment(std::string_view value, std::size_t start, std::size_t length) {
    if (length != 1U) {
        return false;
    }

    return value[start] == '.';
}

bool IsParentDirectorySegment(std::string_view value, std::size_t start, std::size_t length) {
    if (length != 2U) {
        return false;
    }

    if (value[start] != '.') {
        return false;
    }

    return value[start + 1U] == '.';
}

FileStatus ValidateSegment(std::string_view value, std::size_t start, std::size_t length) {
    if (length == 0U) {
        return FileStatus::InvalidPath;
    }

    if (IsCurrentDirectorySegment(value, start, length)) {
        return FileStatus::InvalidPath;
    }

    if (IsParentDirectorySegment(value, start, length)) {
        return FileStatus::PathEscape;
    }

    return FileStatus::Success;
}

FileStatus ValidatePublicNormalizedPath(std::string_view value) {
    if (value.empty()) {
        return FileStatus::InvalidPath;
    }

    if (value.size() > MAX_NORMALIZED_PATH_LENGTH) {
        return FileStatus::PathTooLong;
    }

    if (IsAbsolutePath(value)) {
        return FileStatus::InvalidPath;
    }

    std::size_t segmentStart = 0U;
    for (std::size_t index = 0U; index < value.size(); ++index) {
        const char character = value[index];
        if (character == '\\') {
            return FileStatus::InvalidPath;
        }

        if (character != '/') {
            continue;
        }

        const FileStatus status = ValidateSegment(value, segmentStart, index - segmentStart);
        if (status != FileStatus::Success) {
            return status;
        }

        segmentStart = index + 1U;
    }

    return ValidateSegment(value, segmentStart, value.size() - segmentStart);
}

bool IsPathInsideRoot(const std::filesystem::path& rootPath, const std::filesystem::path& candidatePath) {
    auto rootIterator = rootPath.begin();
    auto candidateIterator = candidatePath.begin();
    while (rootIterator != rootPath.end()) {
        if (candidateIterator == candidatePath.end()) {
            return false;
        }

        if (*rootIterator != *candidateIterator) {
            return false;
        }

        ++rootIterator;
        ++candidateIterator;
    }

    return true;
}
}

LooseFileSource::LooseFileSource()
    : _rootPath() {
}

LooseFileSource::LooseFileSource(std::filesystem::path rootPath)
    : _rootPath(std::move(rootPath)) {
}

FileReadResult LooseFileSource::Read(NormalizedPath path) const {
    const FileStatus pathStatus = ValidatePublicNormalizedPath(path.Value());
    if (pathStatus != FileStatus::Success) {
        return FileReadResult::Failure(pathStatus);
    }

    std::error_code errorCode;
    const std::filesystem::path canonicalRoot = std::filesystem::weakly_canonical(_rootPath, errorCode);
    if (errorCode) {
        return FileReadResult::Failure(FileStatus::ReadFailure);
    }

    errorCode.clear();
    const std::filesystem::path resolvedPath = _rootPath / std::filesystem::path(std::string(path.Value()));
    const std::filesystem::path canonicalResolvedPath = std::filesystem::weakly_canonical(resolvedPath, errorCode);
    if (errorCode) {
        return FileReadResult::Failure(FileStatus::ReadFailure);
    }

    if (!IsPathInsideRoot(canonicalRoot, canonicalResolvedPath)) {
        return FileReadResult::Failure(FileStatus::PathEscape);
    }

    errorCode.clear();
    if (!std::filesystem::exists(canonicalResolvedPath, errorCode)) {
        if (errorCode) {
            return FileReadResult::Failure(FileStatus::ReadFailure);
        }

        return FileReadResult::Failure(FileStatus::FileNotFound);
    }

    errorCode.clear();
    if (!std::filesystem::is_regular_file(canonicalResolvedPath, errorCode)) {
        if (errorCode) {
            return FileReadResult::Failure(FileStatus::ReadFailure);
        }

        return FileReadResult::Failure(FileStatus::ReadFailure);
    }

    errorCode.clear();
    const auto fileSize = std::filesystem::file_size(canonicalResolvedPath, errorCode);
    if (errorCode) {
        return FileReadResult::Failure(FileStatus::ReadFailure);
    }

    if (fileSize > MAX_FIXTURE_READ_SIZE) {
        return FileReadResult::Failure(FileStatus::ReadTooLarge);
    }

    std::ifstream file(canonicalResolvedPath, std::ios::binary);
    if (!file.is_open()) {
        return FileReadResult::Failure(FileStatus::ReadFailure);
    }

    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(fileSize));
    if (!bytes.empty()) {
        file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    if (!file.good()) {
        return FileReadResult::Failure(FileStatus::ReadFailure);
    }

    return FileReadResult::Success(std::move(bytes));
}

const std::filesystem::path& LooseFileSource::RootPath() const {
    return _rootPath;
}
}
