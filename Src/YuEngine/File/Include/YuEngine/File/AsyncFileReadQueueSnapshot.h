// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/AsyncFileReadQueueSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/File/AsyncFileReadStatus.h"
#include "YuEngine/File/FileReadRequest.h"

namespace yuengine::file {
struct AsyncFileReadQueueSnapshot {
    std::size_t work_capacity = 0U;
    std::size_t completion_capacity = 0U;
    std::uint64_t submitted_count = 0U;
    std::uint64_t completed_count = 0U;
    std::uint64_t failed_count = 0U;
    std::uint64_t canceled_count = 0U;
    std::uint64_t rejected_count = 0U;
    std::uint64_t drained_completion_count = 0U;
    std::uint64_t read_byte_count = 0U;
    std::size_t pending_count = 0U;
    std::size_t completion_pending_count = 0U;
    std::size_t max_queue_depth = 0U;
    std::size_t max_completion_depth = 0U;
    std::size_t required_completion_output_count = 0U;
    std::size_t last_required_queue_capacity = 0U;
    std::uint64_t last_failed_request_index = 0U;
    FileReadRequest last_failed_read_request;
    std::size_t last_failed_output_capacity = 0U;
    std::size_t last_failed_work_capacity = 0U;
    std::size_t last_failed_pending_count = 0U;
    AsyncFileReadStatus last_status = AsyncFileReadStatus::NotInitialized;
    bool is_initialized = false;
    bool is_started = false;
    bool is_shutdown = false;
};
}
