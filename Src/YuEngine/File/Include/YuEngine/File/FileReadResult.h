// Module: YuEngine File
// File: Src/YuEngine/File/Include/YuEngine/File/FileReadResult.h

#pragma once

#include <cstdint>
#include <vector>

#include "YuEngine/File/FileStatus.h"

namespace yuengine::file {
struct FileReadResult {
    FileStatus status;
    std::vector<std::uint8_t> bytes;

    /**
     * @comment Creates a successful result.
     * @param bytes Input byte count or byte payload.
     * @return Explicit operation result.
     */
    static FileReadResult Success(std::vector<std::uint8_t> bytes);
    /**
     * @comment Creates a failed result.
     * @param status Input status.
     * @return Explicit operation result.
     */
    static FileReadResult Failure(FileStatus status);
    /**
     * @comment Checks whether the result succeeded.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool Succeeded() const;
};
}
