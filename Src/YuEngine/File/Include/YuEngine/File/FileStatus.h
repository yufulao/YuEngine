// Module: YuEngine File
// File: Src/YuEngine/File/Include/YuEngine/File/FileStatus.h

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
    InvalidBuffer,
    ReadFailure,
    ReadTooLarge,
    WriteFailure,
    WriteTooLarge
};
}
