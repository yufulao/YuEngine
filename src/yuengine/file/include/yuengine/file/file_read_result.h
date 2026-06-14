#pragma once

#include <cstdint>
#include <vector>

#include "yuengine/file/file_status.h"

namespace yuengine::file {
struct file_read_result_t {
    FileStatus Status;
    std::vector<std::uint8_t> Bytes;

    static file_read_result_t Success(std::vector<std::uint8_t> bytes);
    static file_read_result_t Failure(FileStatus status);
    bool Succeeded() const;
};
}
