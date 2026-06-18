// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/PackageResourceStagingQueue.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/File/AsyncFileReadResult.h"
#include "YuEngine/Streaming/PackageResourceStagingCompletion.h"
#include "YuEngine/Streaming/PackageResourceStagingConstants.h"
#include "YuEngine/Streaming/PackageResourceStagingQueueDesc.h"
#include "YuEngine/Streaming/PackageResourceStagingRequest.h"
#include "YuEngine/Streaming/PackageResourceStagingSnapshot.h"
#include "YuEngine/Streaming/PackageResourceStagingStatus.h"

namespace yuengine::streaming {
class PackageResourceStagingQueue final {
public:
    /**
     * @comment 使用默认固定存储构造队列。
     */
    PackageResourceStagingQueue();
    /**
     * @comment 使用固定存储上限构造队列。
     * @param desc 输入描述。
     */
    explicit PackageResourceStagingQueue(PackageResourceStagingQueueDesc desc);

    /**
     * @comment 验证并 submits 一个 package-resource staging 请求。
     * @param request 输入 请求。
     * @return 显式操作状态。
     */
    PackageResourceStagingStatus Submit(const PackageResourceStagingRequest &request);
    /**
     * @comment 接受调用方已排空的异步文件读取完成记录。
     * @param file_result 输入 file completion 值。
     * @return 显式操作状态。
     */
    PackageResourceStagingStatus CompleteFileRead(const file::AsyncFileReadResult &file_result);
    /**
     * @comment 提取 staging completion 记录 写入 调用方持有 存储.
     * @param output_completions 输出 completion 存储。
     * @param output_capacity 输出 存储容量。
     * @param written_count 输出 写入数量。
     * @return 显式操作状态。
     */
    PackageResourceStagingStatus DrainCompletions(
        PackageResourceStagingCompletion *output_completions,
        std::uint32_t output_capacity,
        std::uint32_t *written_count);
    /**
     * @comment 返回队列快照值。
     * @return 快照值。
     */
    PackageResourceStagingSnapshot Snapshot() const;

private:
    struct PendingRecord {
        bool is_active = false;
        package::PackageLoadPlanRecord package_record;
        resource::ResourceHandle resource;
        resource::ResourceTypeId expected_type;
        std::uint64_t request_id = 0U;
    };

    PackageResourceStagingStatus RecordRejected(PackageResourceStagingStatus status);
    PackageResourceStagingStatus RecordRejected(
        PackageResourceStagingStatus status,
        resource::ResourceStatus resource_status);
    PackageResourceStagingStatus RecordFileSubmitFailure(file::AsyncFileReadStatus async_file_status);
    PackageResourceStagingStatus RecordMissingCompletion();
    PackageResourceStagingStatus RecordCompletionOverflow();
    PackageResourceStagingStatus ValidateRequest(const PackageResourceStagingRequest &request) const;
    bool HasRequestId(std::uint64_t request_id) const;
    PendingRecord *FindFreePendingRecord();
    PendingRecord *FindPendingRecord(std::uint64_t request_id);
    void StorePendingRecord(const PackageResourceStagingRequest &request);
    bool AppendCompletion(const PackageResourceStagingCompletion &completion);
    void RemovePendingRecord(PendingRecord &pending_record);
    void UpdateMaxPendingCount();
    void UpdateMaxCompletionCount();

    std::array<PendingRecord, MAX_PACKAGE_RESOURCE_STAGING_REQUEST_COUNT> pending_records_;
    std::array<PackageResourceStagingCompletion, MAX_PACKAGE_RESOURCE_STAGING_COMPLETION_COUNT> completions_;
    PackageResourceStagingSnapshot snapshot_;
};
}
