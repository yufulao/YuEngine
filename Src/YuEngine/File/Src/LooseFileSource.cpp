// Module: YuEngine File
// File: Src/YuEngine/File/Src/LooseFileSource.cpp

#include "YuEngine/File/LooseFileSource.h"

#include <fstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include "YuEngine/File/FileConstants.h"

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

    std::size_t segment_start = 0U;
    for (std::size_t index = 0U; index < value.size(); ++index) {
        const char character = value[index];
        if (character == '\\') {
            return FileStatus::InvalidPath;
        }

        if (character != '/') {
            continue;
        }

        const FileStatus status = ValidateSegment(value, segment_start, index - segment_start);
        if (status != FileStatus::Success) {
            return status;
        }

        segment_start = index + 1U;
    }

    return ValidateSegment(value, segment_start, value.size() - segment_start);
}

bool IsPathInsideRoot(const std::filesystem::path& root_path, const std::filesystem::path& candidate_path) {
    auto root_iterator = root_path.begin();
    auto candidate_iterator = candidate_path.begin();
    while (root_iterator != root_path.end()) {
        if (candidate_iterator == candidate_path.end()) {
            return false;
        }

        if (*root_iterator != *candidate_iterator) {
            return false;
        }

        ++root_iterator;
        ++candidate_iterator;
    }

    return true;
}
}

LooseFileSource::LooseFileSource()
    : root_path_() {
}

LooseFileSource::LooseFileSource(std::filesystem::path root_path)
    : root_path_(std::move(root_path)) {
}

FileReadResult LooseFileSource::Read(NormalizedPath path) const {
    const FileStatus path_status = ValidatePublicNormalizedPath(path.Value());
    if (path_status != FileStatus::Success) {
        return FileReadResult::Failure(path_status);
    }

    std::error_code error_code;
    const std::filesystem::path canonical_root = std::filesystem::weakly_canonical(root_path_, error_code);
    if (error_code) {
        return FileReadResult::Failure(FileStatus::ReadFailure);
    }

    error_code.clear();
    const std::filesystem::path resolved_path = root_path_ / std::filesystem::path(std::string(path.Value()));
    const std::filesystem::path canonical_resolved_path = std::filesystem::weakly_canonical(resolved_path, error_code);
    if (error_code) {
        return FileReadResult::Failure(FileStatus::ReadFailure);
    }

    if (!IsPathInsideRoot(canonical_root, canonical_resolved_path)) {
        return FileReadResult::Failure(FileStatus::PathEscape);
    }

    error_code.clear();
    if (!std::filesystem::exists(canonical_resolved_path, error_code)) {
        if (error_code) {
            return FileReadResult::Failure(FileStatus::ReadFailure);
        }

        return FileReadResult::Failure(FileStatus::FileNotFound);
    }

    error_code.clear();
    if (!std::filesystem::is_regular_file(canonical_resolved_path, error_code)) {
        if (error_code) {
            return FileReadResult::Failure(FileStatus::ReadFailure);
        }

        return FileReadResult::Failure(FileStatus::ReadFailure);
    }

    error_code.clear();
    const auto file_size = std::filesystem::file_size(canonical_resolved_path, error_code);
    if (error_code) {
        return FileReadResult::Failure(FileStatus::ReadFailure);
    }

    if (file_size > MAX_FIXTURE_READ_SIZE) {
        return FileReadResult::Failure(FileStatus::ReadTooLarge);
    }

    std::ifstream file(canonical_resolved_path, std::ios::binary);
    if (!file.is_open()) {
        return FileReadResult::Failure(FileStatus::ReadFailure);
    }

    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(file_size));
    if (!bytes.empty()) {
        file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    if (!file.good()) {
        return FileReadResult::Failure(FileStatus::ReadFailure);
    }

    return FileReadResult::Success(std::move(bytes));
}

const std::filesystem::path& LooseFileSource::RootPath() const {
    return root_path_;
}
}
