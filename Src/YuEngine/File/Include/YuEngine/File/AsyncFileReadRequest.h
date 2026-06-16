// Module: YuEngine File
// File: Src/YuEngine/File/Include/YuEngine/File/AsyncFileReadRequest.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/File/FileReadRequest.h"

namespace yuengine::file {
class MountTable;

struct AsyncFileReadRequest {
    MountTable* mount_table = nullptr;
    FileReadRequest read_request;
    std::uint64_t request_index = 0U;
    std::uint8_t* output_bytes = nullptr;
    std::size_t output_capacity = 0U;
};
}
