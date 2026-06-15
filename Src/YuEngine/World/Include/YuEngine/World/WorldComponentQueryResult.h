// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentQueryResult.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldComponentQueryStatus.h"

namespace yuengine::world {
struct WorldComponentQueryResult final {
    WorldComponentQueryStatus status = WorldComponentQueryStatus::Success;
    std::uint32_t matched_record_count = 0U;
    std::uint32_t written_record_count = 0U;

    /**
     * @comment Creates a successful query result.
     * @param matched_record_count Input matched record count.
     * @param written_record_count Input written record count.
     * @return Explicit operation result.
     */
    static WorldComponentQueryResult Success(
        std::uint32_t matched_record_count,
        std::uint32_t written_record_count) {
        return WorldComponentQueryResult{
            WorldComponentQueryStatus::Success,
            matched_record_count,
            written_record_count};
    }

    /**
     * @comment Creates a failed query result.
     * @param status Input query status.
     * @param matched_record_count Input matched record count.
     * @param written_record_count Input written record count.
     * @return Explicit operation result.
     */
    static WorldComponentQueryResult Failure(
        WorldComponentQueryStatus status,
        std::uint32_t matched_record_count=0U,
        std::uint32_t written_record_count=0U) {
        return WorldComponentQueryResult{
            status,
            matched_record_count,
            written_record_count};
    }

    /**
     * @comment Checks whether the query succeeded.
     * @return True when the query succeeded; false otherwise.
     */
    bool Succeeded() const {
        return status == WorldComponentQueryStatus::Success;
    }
};
}
