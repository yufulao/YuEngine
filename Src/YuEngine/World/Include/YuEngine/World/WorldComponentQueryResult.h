// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentQueryResult.h

#pragma once

#include <cstdint>

#include "YuEngine/World/WorldComponentQueryStatus.h"

namespace yuengine::world {
struct WorldComponentQueryResult final {
    WorldComponentQueryStatus status = WorldComponentQueryStatus::Success;
    std::uint32_t matched_record_count = 0U;
    std::uint32_t written_record_count = 0U;

    /**
     * @comment 创建成功 query result。
     * @param matched_record_count 输入 matched record count。
     * @param written_record_count 输入 written record count。
     * @return 显式操作结果。
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
     * @comment 创建失败 query result。
     * @param status 输入 query status。
     * @param matched_record_count 输入 matched record count。
     * @param written_record_count 输入 written record count。
     * @return 显式操作结果。
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
     * @comment 检查 query 是否成功。
     * @return query 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == WorldComponentQueryStatus::Success;
    }
};
}
