// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/PackageResourceStagingQueue.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/File/AsyncFileReadResult.h"
#include "YuEngine/Streaming/PackageResourceStagingCompletion.h"
#include "YuEngine/Streaming/PackageResourceStagingConstants.h"
#include "YuEngine/Streaming/PackageResourceStagingQueueDesc.h"
#include "YuEngine/Streaming/PackageResourceStagingPendingRequestSnapshot.h"
#include "YuEngine/Streaming/PackageResourceStagingRequest.h"
#include "YuEngine/Streaming/PackageResourceStagingSnapshot.h"
#include "YuEngine/Streaming/PackageResourceStagingStatus.h"
#include "YuEngine/Streaming/PackageResourceStagingSubmitResult.h"

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
     * @comment 验证并提交 package-resource staging 请求。
     * @param request 输入请求。
     * @return 显式操作状态。
     */
    PackageResourceStagingStatus Submit(const PackageResourceStagingRequest &request);
    /**
     * @comment 原子式预检并提交 package-resource staging batch 请求。
     * @param requests 输入请求数组。
     * @param request_count 输入请求数量。
     * @param output_results 输出 per-request submit 结果。
     * @param output_capacity 输出结果容量。
     * @return batch submit 结果。
     */
    PackageResourceStagingBatchSubmitResult SubmitBatch(
        const PackageResourceStagingRequest *requests,
        std::uint32_t request_count,
        PackageResourceStagingSubmitResult *output_results,
        std::uint32_t output_capacity);
    /**
     * @comment 接受调用方已排空的异步文件读取完成记录。
     * @param file_result 输入 file completion 值。
     * @return 显式操作状态。
     */
    PackageResourceStagingStatus CompleteFileRead(const file::AsyncFileReadResult &file_result);
    /**
     * @comment 提取 staging completion 记录并写入调用方持有的存储。
     * @param output_completions 输出 completion 存储。
     * @param output_capacity 输出存储容量。
     * @param written_count 输出写入数量。
     * @return 显式操作状态。
     */
    PackageResourceStagingStatus DrainCompletions(
        PackageResourceStagingCompletion *output_completions,
        std::uint32_t output_capacity,
        std::uint32_t *written_count);
    /**
     * @comment 返回当前 pending staging 请求数量。
     * @param pending_count 输出 pending 请求数量。
     * @return 显式操作状态。
     */
    PackageResourceStagingStatus GetPendingCountSnapshot(std::uint32_t *pending_count) const;
    /**
     * @comment 返回指定资源类型的当前 pending staging 请求数量。
     * @param expected_type 输入资源类型。
     * @param pending_count 输出 pending 请求数量。
     * @return 显式操作状态。
     */
    PackageResourceStagingStatus GetPendingRequestTypeCountSnapshot(
        resource::ResourceTypeId expected_type,
        std::uint32_t *pending_count) const;
    /**
     * @comment 枚举当前 pending staging 请求快照。
     * @param output_requests 输出 pending 请求快照存储。
     * @param output_capacity 输出存储容量。
     * @param written_count 输出写入数量。
     * @return pending 请求枚举结果。
     */
    PackageResourceStagingPendingRequestEnumerationResult EnumeratePendingRequests(
        PackageResourceStagingPendingRequestSnapshot *output_requests,
        std::uint32_t output_capacity,
        std::uint32_t *written_count) const;
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
