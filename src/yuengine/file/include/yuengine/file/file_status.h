#pragma once

namespace yuengine::file {
enum class FILE_STATUS {
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
