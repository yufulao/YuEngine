#pragma once

namespace yuengine::file {
enum class FileStatus {
    Success,
    InvalidMount,
    InvalidPath,
    PathEscape,
    PathTooLong,
    DuplicateMount,
    MountTableFull,
    MountNotFound,
    FileNotFound,
    ReadFailure,
    ReadTooLarge
};
}
