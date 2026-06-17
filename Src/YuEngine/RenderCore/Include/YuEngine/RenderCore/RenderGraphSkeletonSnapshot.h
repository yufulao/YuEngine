// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeletonSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonOperation.h"
#include "YuEngine/RenderCore/RenderGraphSkeletonStatus.h"

namespace yuengine::rendercore {
/**
 * @comment Contains bounded RenderCore render graph skeleton counters and last status values.
 */
struct RenderGraphSkeletonSnapshot final {
    std::size_t graph_record_capacity = 0U;
    std::size_t pass_record_capacity = 0U;
    std::size_t dependency_record_capacity = 0U;
    std::size_t graph_record_count = 0U;
    std::uint64_t accepted_graph_count = 0U;
    std::uint64_t prepared_graph_count = 0U;
    std::uint64_t released_graph_count = 0U;
    std::uint64_t failed_validation_count = 0U;
    std::uint64_t duplicate_graph_id_count = 0U;
    std::uint64_t duplicate_pass_id_count = 0U;
    std::uint64_t missing_dependency_count = 0U;
    std::uint64_t self_dependency_count = 0U;
    std::uint64_t dependency_cycle_count = 0U;
    std::uint64_t pass_capacity_rejected_count = 0U;
    std::uint64_t dependency_capacity_rejected_count = 0U;
    std::uint64_t invalid_pass_request_count = 0U;
    std::uint32_t last_graph_id = 0U;
    std::size_t last_pass_count = 0U;
    std::size_t last_dependency_count = 0U;
    std::size_t last_failed_pass_index = 0U;
    std::size_t last_failed_dependency_index = 0U;
    std::uint32_t last_pass_id = 0U;
    std::uint32_t last_dependency_before_pass_id = 0U;
    std::uint32_t last_dependency_after_pass_id = 0U;
    RenderGraphSkeletonStatus last_status = RenderGraphSkeletonStatus::InvalidArgument;
    RenderGraphSkeletonOperation last_operation = RenderGraphSkeletonOperation::None;
    RenderFixturePassStatus last_pass_status = RenderFixturePassStatus::InvalidArgument;
};
}
