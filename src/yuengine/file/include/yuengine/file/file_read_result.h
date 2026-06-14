#pragma once

#include <cstdint>
#include <vector>

#include "yuengine/file/file_status.h"

namespace yuengine::file {
struct FileReadResult {
    FILE_STATUS Status;
    std::vector<std::uint8_t> Bytes;

    static FileReadResult Success(std::vector<std::uint8_t> bytes);
    static FileReadResult Failure(FILE_STATUS status);
    bool Succeeded() const;
};
}
