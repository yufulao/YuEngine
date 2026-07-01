// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceRegistry.h

#pragma once

#include <array>
#include <cstdint>
#include <cstddef>

#include "YuEngine/Resource/ResourceCachePayloadBudgetDesc.h"
#include "YuEngine/Resource/ResourceCachePayloadOperation.h"
#include "YuEngine/Resource/ResourceCachePayloadRecord.h"
#include "YuEngine/Resource/ResourceCachePayloadRequest.h"
#include "YuEngine/Resource/ResourceCachePayloadSnapshot.h"
#include "YuEngine/Resource/ResourceCachePayloadStatus.h"
#include "YuEngine/Resource/ResourceConstants.h"
#include "YuEngine/Resource/ResourceDecodedPayloadBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodedPayloadOperation.h"
#include "YuEngine/Resource/ResourceDecodedPayloadRecord.h"
#include "YuEngine/Resource/ResourceDecodedPayloadRequest.h"
#include "YuEngine/Resource/ResourceDecodedPayloadSnapshot.h"
#include "YuEngine/Resource/ResourceDecodedPayloadStatus.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodePlanBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodePlanOperation.h"
#include "YuEngine/Resource/ResourceDecodePlanRecord.h"
#include "YuEngine/Resource/ResourceDecodePlanRequest.h"
#include "YuEngine/Resource/ResourceDecodePlanSnapshot.h"
#include "YuEngine/Resource/ResourceDecodePlanStatus.h"
#include "YuEngine/Resource/ResourceDecodeResultBudgetDesc.h"
#include "YuEngine/Resource/ResourceDecodeResultOperation.h"
#include "YuEngine/Resource/ResourceDecodeResultRecord.h"
#include "YuEngine/Resource/ResourceDecodeResultRequest.h"
#include "YuEngine/Resource/ResourceDecodeResultSnapshot.h"
#include "YuEngine/Resource/ResourceDecodeResultStatus.h"
#include "YuEngine/Resource/ResourceDependencyEdge.h"
#include "YuEngine/Resource/ResourceDescriptor.h"
#include "YuEngine/Resource/ResourceLoadCommitRecord.h"
#include "YuEngine/Resource/ResourceLoadCommitRequest.h"
#include "YuEngine/Resource/ResourceLoadCommitStatus.h"
#include "YuEngine/Resource/ResourceLoadState.h"
#include "YuEngine/Resource/ResourceRegistrationResult.h"
#include "YuEngine/Resource/ResourceRegistryDesc.h"
#include "YuEngine/Resource/ResourceResidencyBudgetDesc.h"
#include "YuEngine/Resource/ResourceResidencyOperation.h"
#include "YuEngine/Resource/ResourceResidencyRecord.h"
#include "YuEngine/Resource/ResourceResidencyRequest.h"
#include "YuEngine/Resource/ResourceResidencySnapshot.h"
#include "YuEngine/Resource/ResourceResidencyState.h"
#include "YuEngine/Resource/ResourceResidencyStatus.h"
#include "YuEngine/Resource/ResourceSlot.h"
#include "YuEngine/Resource/ResourceSnapshot.h"

namespace yuengine::resource {
struct ResourceDescriptorBatchResult final {
    ResourceStatus status = ResourceStatus::InvalidDescriptor;
    std::uint32_t committed_descriptor_count = 0U;
    std::uint32_t failed_descriptor_index = 0U;
    std::uint32_t required_resource_count = 0U;
    std::uint32_t required_type_count = 0U;
    std::uint32_t required_dependency_edge_count = 0U;

    /**
     * @comment 检查 batch 结果是否成功。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == ResourceStatus::Success;
    }
};

struct ResourceDependencyRequest final {
    ResourceHandle dependent{};
    ResourceHandle dependency{};
};

struct ResourceDependencyBatchResult final {
    ResourceStatus status = ResourceStatus::InvalidHandle;
    std::uint32_t committed_dependency_edge_count = 0U;
    std::uint32_t failed_dependency_edge_index = 0U;

    /**
     * @comment 检查 batch 结果是否成功。
     * @return 条件满足时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == ResourceStatus::Success;
    }
};

class ResourceRegistry final {
public:
    /**
     * @comment 构造 ResourceRegistry 实例。
     */
    ResourceRegistry();
    /**
     * @comment 构造 ResourceRegistry 实例。
     * @param desc 输入描述。
     */
    explicit ResourceRegistry(ResourceRegistryDesc desc);

    /**
     * @comment 注册 synthetic 描述.
     * @param descriptor 输入描述。
     * @return 显式操作结果。
     */
    ResourceRegistrationResult RegisterSyntheticDescriptor(const ResourceDescriptor& descriptor);
    /**
     * @comment 按顺序注册多个 synthetic 描述。
     * @param descriptors 输入描述数组。
     * @param descriptor_count 输入描述数量。
     * @return 显式 batch 操作结果。
     */
    ResourceDescriptorBatchResult RegisterSyntheticDescriptors(
        const ResourceDescriptor *descriptors,
        std::uint32_t descriptor_count);
    /**
     * @comment 按注册顺序枚举当前 synthetic 描述。
     * @param output_descriptors 输出描述存储。
     * @param output_descriptor_capacity 输出存储容量。
     * @param output_descriptor_count 输出描述数量。
     * @return 显式操作状态。
     */
    ResourceStatus EnumerateSyntheticDescriptors(
        ResourceDescriptor *output_descriptors,
        std::uint32_t output_descriptor_capacity,
        std::uint32_t *output_descriptor_count);
    /**
     * @comment 按 type/key 精确查找当前 synthetic 描述。
     * @param type 输入类型。
     * @param logical_key 输入逻辑 key。
     * @param output_descriptor 输出描述。
     * @return 显式操作状态。
     */
    ResourceStatus FindSyntheticDescriptor(
        ResourceTypeId type,
        const ResourceLogicalKey &logical_key,
        ResourceDescriptor *output_descriptor);
    /**
     * @comment 添加依赖。
     * @param dependent 输入 dependent。
     * @param dependency 输入 dependency。
     * @return 显式操作状态。
     */
    ResourceStatus AddDependency(ResourceHandle dependent, ResourceHandle dependency);
    /**
     * @comment 按顺序提交多个显式 Resource dependency 请求。
     * @param dependencies 输入 dependency 请求数组。
     * @param dependency_count 输入 dependency 请求数量。
     * @return 显式 batch 操作结果。
     */
    ResourceDependencyBatchResult AddDependencies(
        const ResourceDependencyRequest *dependencies,
        std::uint32_t dependency_count);
    /**
     * @comment 精确查找当前直接 Resource dependency 边。
     * @param dependent 输入 dependent。
     * @param dependency 输入 dependency。
     * @param output_dependency_edge_exists 输出直接 dependency 边存在标记。
     * @return 显式操作状态。
     */
    ResourceStatus FindDependencyEdge(
        ResourceHandle dependent,
        ResourceHandle dependency,
        bool *output_dependency_edge_exists);
    /**
     * @comment 返回当前直接 Resource dependency 边数量快照。
     * @param output_dependency_edge_count 输出 dependency 边数量。
     * @return 显式操作状态。
     */
    ResourceStatus CountDependencyEdges(std::uint32_t *output_dependency_edge_count);
    /**
     * @comment 按提交顺序枚举当前直接 Resource dependency 边。
     * @param output_dependencies 输出 dependency 请求存储。
     * @param output_dependency_capacity 输出存储容量。
     * @param output_dependency_count 输出 dependency 请求数量。
     * @return 显式操作状态。
     */
    ResourceStatus EnumerateDependencyEdges(
        ResourceDependencyRequest *output_dependencies,
        std::uint32_t output_dependency_capacity,
        std::uint32_t *output_dependency_count);
    /**
     * @comment 按确定顺序遍历 root Resource 的依赖闭包。
     * @param root 输入 root Resource。
     * @param output_dependencies 输出 Resource handle 存储。
     * @param output_dependency_capacity 输出存储容量。
     * @param output_dependency_count 输出 Resource 数量。
     * @return 显式操作状态。
     */
    ResourceStatus TraverseDependencies(
        ResourceHandle root,
        ResourceHandle *output_dependencies,
        std::uint32_t output_dependency_capacity,
        std::uint32_t *output_dependency_count);
    /**
     * @comment 按确定顺序遍历多个 root Resource 的去重依赖闭包。
     * @param roots 输入 root Resource 数组。
     * @param root_count 输入 root Resource 数量。
     * @param output_dependencies 输出 Resource handle 存储。
     * @param output_dependency_capacity 输出存储容量。
     * @param output_dependency_count 输出 Resource 数量。
     * @return 显式操作状态。
     */
    ResourceStatus TraverseDependencies(
        const ResourceHandle *roots,
        std::uint32_t root_count,
        ResourceHandle *output_dependencies,
        std::uint32_t output_dependency_capacity,
        std::uint32_t *output_dependency_count);
    /**
     * @comment 获取操作。
     * @param handle 输入句柄。
     * @param expected_type 输入期望类型。
     * @return 显式操作状态。
     */
    ResourceStatus Acquire(ResourceHandle handle, ResourceTypeId expected_type);
    /**
     * @comment 验证预计 acquire 可在不修改注册表状态的情况下成功。
     * @param handle 输入句柄。
     * @param expected_type 输入期望类型。
     * @param projected_acquire_count 输入 acquire 计数预算。
     * @return 显式操作状态。
     */
    ResourceStatus ValidateAcquire(
        ResourceHandle handle,
        ResourceTypeId expected_type,
        std::uint32_t projected_acquire_count) const;
    /**
     * @comment 提交 upload completion 产生的最终 Resource 加载状态。
     * @param request 输入提交请求。
     * @return 显式操作状态。
     */
    ResourceLoadCommitStatus CommitUploadCompletion(const ResourceLoadCommitRequest &request);
    /**
     * @comment 返回有效句柄的 Resource 加载状态。
     * @param handle 输入句柄。
     * @param expected_type 输入期望类型。
     * @param output_state 输出加载状态。
     * @return 显式操作状态。
     */
    ResourceLoadCommitStatus GetLoadState(
        ResourceHandle handle,
        ResourceTypeId expected_type,
        ResourceLoadState *output_state) const;
    /**
     * @comment 配置 Resource 自有驻留预算。
     * @param desc 输入预算描述。
     * @return 显式驻留操作状态。
     */
    ResourceResidencyStatus SetResidencyBudget(ResourceResidencyBudgetDesc desc);
    /**
     * @comment 将已上传的 Resource 槽位纳入 resident 状态。
     * @param request 输入驻留请求。
     * @return 显式驻留操作状态。
     */
    ResourceResidencyStatus AdmitResident(const ResourceResidencyRequest &request);
    /**
     * @comment 固定一个已驻留 Resource 槽位。
     * @param request 输入驻留请求。
     * @return 显式驻留操作状态。
     */
    ResourceResidencyStatus PinResident(const ResourceResidencyRequest &request);
    /**
     * @comment 取消固定 一个已固定 Resource 槽位。
     * @param request 输入驻留请求。
     * @return 显式驻留操作状态。
     */
    ResourceResidencyStatus UnpinResident(const ResourceResidencyRequest &request);
    /**
     * @comment 在 Resource 自有状态中将已驻留槽位标记为已驱逐。
     * @param request 输入驻留请求。
     * @return 显式驻留操作状态。
     */
    ResourceResidencyStatus EvictResident(const ResourceResidencyRequest &request);
    /**
     * @comment 选择确定性的 Resource 自有驱逐候选项。
     * @param output_handle 输出候选句柄。
     * @return 显式驻留操作状态。
     */
    ResourceResidencyStatus SelectEvictionCandidate(ResourceHandle *output_handle);
    /**
     * @comment 返回有效句柄的 Resource 自有驻留状态。
     * @param handle 输入句柄。
     * @param expected_type 输入期望类型。
     * @param output_state 输出驻留状态。
     * @return 显式驻留操作状态。
     */
    ResourceResidencyStatus GetResidencyState(
        ResourceHandle handle,
        ResourceTypeId expected_type,
        ResourceResidencyState *output_state) const;
    /**
     * @comment 返回 Resource 自有驻留计数器快照。
     * @return 驻留快照值。
     */
    ResourceResidencySnapshot ResidencySnapshot() const;
    /**
     * @comment 配置 Resource 自有缓存载荷预算。
     * @param desc 输入预算描述。
     * @return 显式缓存载荷操作状态。
     */
    ResourceCachePayloadStatus SetCachePayloadBudget(ResourceCachePayloadBudgetDesc desc);
    /**
     * @comment 将调用方提供的不透明字节存入 Resource 自有缓存载荷存储。
     * @param request 输入缓存载荷请求。
     * @return 显式缓存载荷操作状态。
     */
    ResourceCachePayloadStatus StoreCachePayload(const ResourceCachePayloadRequest &request);
    /**
     * @comment 将 Resource 自有不透明缓存载荷字节读取到调用方持有输出存储。
     * @param request 输入缓存载荷请求。
     * @param output_bytes 输出 字节 存储。
     * @param output_byte_capacity 输出 字节容量。
     * @param output_byte_count 输出 字节数。
     * @return 显式缓存载荷操作状态。
     */
    ResourceCachePayloadStatus ReadCachePayload(
        const ResourceCachePayloadRequest &request,
        std::uint8_t *output_bytes,
        std::uint32_t output_byte_capacity,
        std::uint32_t *output_byte_count);
    /**
     * @comment 释放 Resource 自有缓存载荷字节，不改变加载或驻留状态。
     * @param request 输入缓存载荷请求。
     * @return 显式缓存载荷操作状态。
     */
    ResourceCachePayloadStatus ReleaseCachePayload(const ResourceCachePayloadRequest &request);
    /**
     * @comment 返回 Resource 自有 cache payload 计数器 的快照。
     * @return 缓存载荷快照值。
     */
    ResourceCachePayloadSnapshot CachePayloadSnapshot() const;
    /**
     * @comment 配置 Resource 自有 decode plan budget.
     * @param desc 输入预算描述。
     * @return 显式解码计划操作状态。
     */
    ResourceDecodePlanStatus SetDecodePlanBudget(ResourceDecodePlanBudgetDesc desc);
    /**
     * @comment 基于已有 cache payload 记录创建一个 Resource 自有 decode plan。
     * @param request 输入解码计划请求。
     * @return 显式解码计划操作状态。
     */
    ResourceDecodePlanStatus CreateDecodePlan(const ResourceDecodePlanRequest &request);
    /**
     * @comment 查询一个 Resource 自有 decode plan 记录。
     * @param request 输入解码计划请求。
     * @param output_record 输出 decode plan 记录。
     * @return 显式解码计划操作状态。
     */
    ResourceDecodePlanStatus QueryDecodePlan(
        const ResourceDecodePlanRequest &request,
        ResourceDecodePlanRecord *output_record);
    /**
     * @comment 释放一个 Resource 自有 decode plan 记录，且不改变 payload 或 residency 状态。
     * @param request 输入解码计划请求。
     * @return 显式解码计划操作状态。
     */
    ResourceDecodePlanStatus ReleaseDecodePlan(const ResourceDecodePlanRequest &request);
    /**
     * @comment 返回 Resource 自有 decode plan 计数器 的快照。
     * @return 解码计划快照值。
     */
    ResourceDecodePlanSnapshot DecodePlanSnapshot() const;
    /**
     * @comment 配置 Resource 自有 decode 结果 budget.
     * @param desc 输入预算描述。
     * @return 显式 decode 结果 操作状态。
     */
    ResourceDecodeResultStatus SetDecodeResultBudget(ResourceDecodeResultBudgetDesc desc);
    /**
     * @comment 基于已有 decode plan 提交一个 Resource 自有 import-ready decode 结果。
     * @param request 输入解码结果请求。
     * @return 显式 decode 结果 操作状态。
     */
    ResourceDecodeResultStatus CommitDecodeResult(const ResourceDecodeResultRequest &request);
    /**
     * @comment 查询一个 Resource 自有 decode 结果记录。
     * @param request 输入解码结果请求。
     * @param output_record 输出 decode 结果 记录。
     * @return 显式 decode 结果 操作状态。
     */
    ResourceDecodeResultStatus QueryDecodeResult(
        const ResourceDecodeResultRequest &request,
        ResourceDecodeResultRecord *output_record);
    /**
     * @comment 释放一个 Resource 自有 decode 结果记录，且不改变 payload 或 residency 状态。
     * @param request 输入解码结果请求。
     * @return 显式 decode 结果 操作状态。
     */
    ResourceDecodeResultStatus ReleaseDecodeResult(const ResourceDecodeResultRequest &request);
    /**
     * @comment 返回 Resource 自有 decode 结果 计数器 的快照。
     * @return Decode 结果 快照 值。
     */
    ResourceDecodeResultSnapshot DecodeResultSnapshot() const;
    /**
     * @comment 配置 Resource 自有 decoded payload 字节 budget.
     * @param desc 输入预算描述。
     * @return 显式解码载荷操作状态。
     */
    ResourceDecodedPayloadStatus SetDecodedPayloadBudget(ResourceDecodedPayloadBudgetDesc desc);
    /**
     * @comment 存储 caller-provided decoded 字节 在 Resource 自有 decoded payload 存储。
     * @param request 输入解码载荷请求。
     * @return 显式解码载荷操作状态。
     */
    ResourceDecodedPayloadStatus StoreDecodedPayload(const ResourceDecodedPayloadRequest &request);
    /**
     * @comment 查询一个 Resource 自有 decoded payload 记录。
     * @param request 输入解码载荷请求。
     * @param output_record 输出 decoded payload 记录。
     * @return 显式解码载荷操作状态。
     */
    ResourceDecodedPayloadStatus QueryDecodedPayload(
        const ResourceDecodedPayloadRequest &request,
        ResourceDecodedPayloadRecord *output_record);
    /**
     * @comment 读取 Resource 自有 decoded payload 字节 写入 调用方持有 output 存储.
     * @param request 输入解码载荷请求。
     * @param output_bytes 输出 字节 存储。
     * @param output_byte_capacity 输出 字节容量。
     * @param output_byte_count 输出 字节数。
     * @return 显式解码载荷操作状态。
     */
    ResourceDecodedPayloadStatus ReadDecodedPayload(
        const ResourceDecodedPayloadRequest &request,
        std::uint8_t *output_bytes,
        std::uint32_t output_byte_capacity,
        std::uint32_t *output_byte_count);
    /**
     * @comment 释放 Resource 自有 decoded payload 字节 且不 changing earlier Resource 状态.
     * @param request 输入解码载荷请求。
     * @return 显式解码载荷操作状态。
     */
    ResourceDecodedPayloadStatus ReleaseDecodedPayload(const ResourceDecodedPayloadRequest &request);
    /**
     * @comment 返回 Resource 自有 decoded payload 计数器 的快照。
     * @return 解码载荷快照值。
     */
    ResourceDecodedPayloadSnapshot DecodedPayloadSnapshot() const;
    /**
     * @comment 释放操作。
     * @param handle 输入句柄。
     * @return 显式操作状态。
     */
    ResourceStatus Release(ResourceHandle handle);
    /**
     * @comment 回收 操作。
     * @param handle 输入句柄。
     * @return 显式操作状态。
     */
    ResourceStatus Retire(ResourceHandle handle);
    /**
     * @comment 返回当前状态快照。
     * @return 快照值。
     */
    ResourceSnapshot Snapshot() const;

private:
    ResourceStatus RecordFailure(ResourceStatus status);
    void RecordSuccess();
    ResourceLoadCommitStatus RecordLoadCommitRejected(
        const ResourceLoadCommitRequest &request,
        ResourceLoadCommitStatus status);
    void RecordLoadCommitSuccess(ResourceLoadState load_state);
    ResourceResidencyStatus RecordResidencyRejected(
        ResourceResidencyOperation operation,
        const ResourceResidencyRequest &request,
        ResourceResidencyStatus status);
    void RecordResidencySuccess(
        ResourceResidencyOperation operation,
        const ResourceResidencyRequest &request,
        ResourceResidencyState state);
    ResourceCachePayloadStatus RecordCachePayloadRejected(
        ResourceCachePayloadOperation operation,
        const ResourceCachePayloadRequest &request,
        ResourceCachePayloadStatus status);
    void RecordCachePayloadSuccess(
        ResourceCachePayloadOperation operation,
        const ResourceCachePayloadRequest &request,
        std::uint32_t cache_slot_index,
        std::uint32_t payload_byte_count);
    ResourceDecodePlanStatus RecordDecodePlanRejected(
        ResourceDecodePlanOperation operation,
        const ResourceDecodePlanRequest &request,
        ResourceDecodePlanStatus status);
    void RecordDecodePlanSuccess(
        ResourceDecodePlanOperation operation,
        const ResourceDecodePlanRequest &request,
        std::uint32_t cache_slot_index,
        std::uint32_t header_version);
    ResourceDecodeResultStatus RecordDecodeResultRejected(
        ResourceDecodeResultOperation operation,
        const ResourceDecodeResultRequest &request,
        ResourceDecodeResultStatus status);
    void RecordDecodeResultSuccess(
        ResourceDecodeResultOperation operation,
        const ResourceDecodeResultRequest &request,
        std::uint32_t decode_plan_slot_index);
    ResourceDecodedPayloadStatus RecordDecodedPayloadRejected(
        ResourceDecodedPayloadOperation operation,
        const ResourceDecodedPayloadRequest &request,
        ResourceDecodedPayloadStatus status);
    void RecordDecodedPayloadSuccess(
        ResourceDecodedPayloadOperation operation,
        const ResourceDecodedPayloadRequest &request,
        std::uint32_t decode_result_slot_index,
        std::uint32_t decoded_payload_slot_index,
        std::uint32_t decoded_byte_count);
    ResourceLoadCommitStatus ValidateLoadCommitRequest(
        const ResourceLoadCommitRequest &request,
        std::size_t *out_slot_index) const;
    ResourceResidencyStatus ValidateResidencyRequest(
        const ResourceResidencyRequest &request,
        std::size_t *out_slot_index) const;
    ResourceCachePayloadStatus ValidateCachePayloadRequest(
        const ResourceCachePayloadRequest &request,
        std::size_t *out_slot_index) const;
    ResourceCachePayloadStatus ValidateCachePayloadStoreWindow(
        const ResourceCachePayloadRequest &request) const;
    ResourceCachePayloadStatus ResolveCachePayloadReadWindow(
        const ResourceCachePayloadRequest &request,
        const ResourceCachePayloadRecord &record,
        std::uint32_t *out_byte_offset,
        std::uint32_t *out_byte_count) const;
    ResourceDecodePlanStatus ValidateDecodePlanRequest(
        const ResourceDecodePlanRequest &request,
        std::size_t *out_slot_index) const;
    ResourceDecodePlanStatus ValidateDecodePlanHeader(
        const ResourceDecodePlanRequest &request,
        const ResourceCachePayloadRecord &cache_payload_record,
        std::uint32_t *out_header_version) const;
    ResourceDecodeResultStatus ValidateDecodeResultRequest(
        const ResourceDecodeResultRequest &request,
        std::size_t *out_slot_index) const;
    ResourceDecodeResultStatus ValidateDecodeResultPlan(
        const ResourceDecodeResultRequest &request,
        const ResourceDecodePlanRecord &decode_plan_record) const;
    ResourceDecodedPayloadStatus ValidateDecodedPayloadRequest(
        const ResourceDecodedPayloadRequest &request,
        std::size_t *out_slot_index) const;
    ResourceDecodedPayloadStatus ValidateDecodedPayloadStoreWindow(
        const ResourceDecodedPayloadRequest &request) const;
    ResourceDecodedPayloadStatus ResolveDecodedPayloadReadWindow(
        const ResourceDecodedPayloadRequest &request,
        const ResourceDecodedPayloadRecord &record,
        std::uint32_t *out_byte_offset,
        std::uint32_t *out_byte_count) const;
    ResourceDecodedPayloadStatus ValidateDecodedPayloadResult(
        const ResourceDecodedPayloadRequest &request,
        const ResourceDecodeResultRecord &decode_result_record) const;
    ResourceStatus TraverseDependencyClosure(
        const ResourceHandle *roots,
        std::uint32_t root_count,
        ResourceHandle *output_dependencies,
        std::uint32_t output_dependency_capacity,
        std::uint32_t *output_dependency_count);
    ResourceLoadCommitStatus MapHandleStatus(ResourceStatus status) const;
    ResourceStatus MapLoadCommitStatus(ResourceLoadCommitStatus status) const;
    ResourceResidencyStatus MapHandleResidencyStatus(ResourceStatus status) const;
    ResourceStatus MapResidencyStatus(ResourceResidencyStatus status) const;
    ResourceCachePayloadStatus MapHandleCachePayloadStatus(ResourceStatus status) const;
    ResourceStatus MapCachePayloadStatus(ResourceCachePayloadStatus status) const;
    ResourceDecodePlanStatus MapHandleDecodePlanStatus(ResourceStatus status) const;
    ResourceStatus MapDecodePlanStatus(ResourceDecodePlanStatus status) const;
    ResourceDecodeResultStatus MapHandleDecodeResultStatus(ResourceStatus status) const;
    ResourceStatus MapDecodeResultStatus(ResourceDecodeResultStatus status) const;
    ResourceDecodedPayloadStatus MapHandleDecodedPayloadStatus(ResourceStatus status) const;
    ResourceStatus MapDecodedPayloadStatus(ResourceDecodedPayloadStatus status) const;
    ResourceStatus ResolveHandle(ResourceHandle handle, std::size_t& out_index) const;
    ResourceStatus RegisterTypeIfNeeded(ResourceTypeId type);
    bool HasType(ResourceTypeId type) const;
    bool HasDuplicateActiveResource(const ResourceDescriptor& descriptor) const;
    bool HasLoadCommitId(std::uint64_t commit_id) const;
    bool HasCachePayloadId(std::uint64_t payload_id) const;
    bool HasDecodePlanId(std::uint64_t decode_plan_id) const;
    bool HasDecodeResultId(std::uint64_t decode_result_id) const;
    bool HasDecodedPayloadId(std::uint64_t decoded_payload_id) const;
    bool FindCachePayloadRecord(
        ResourceHandle resource,
        std::uint64_t payload_id,
        std::size_t *out_record_index) const;
    bool FindFreeCachePayloadRecord(std::size_t *out_record_index) const;
    bool FindDecodePlanRecord(
        ResourceHandle resource,
        std::uint64_t payload_id,
        std::uint64_t decode_plan_id,
        std::size_t *out_record_index) const;
    bool FindFreeDecodePlanRecord(std::size_t *out_record_index) const;
    bool FindDecodeResultRecord(
        ResourceHandle resource,
        std::uint64_t payload_id,
        std::uint64_t decode_plan_id,
        std::uint64_t decode_result_id,
        std::size_t *out_record_index) const;
    bool FindFreeDecodeResultRecord(std::size_t *out_record_index) const;
    bool FindDecodedPayloadRecord(
        ResourceHandle resource,
        std::uint64_t payload_id,
        std::uint64_t decode_plan_id,
        std::uint64_t decode_result_id,
        std::uint64_t decoded_payload_id,
        std::size_t *out_record_index) const;
    bool FindFreeDecodedPayloadRecord(std::size_t *out_record_index) const;
    bool StoreLoadCommitRecord(const ResourceLoadCommitRequest &request);
    bool StoreResidencyRecord(
        ResourceResidencyOperation operation,
        const ResourceResidencyRequest &request,
        ResourceResidencyStatus status,
        ResourceResidencyState state);
    bool IsResidentState(ResourceResidencyState state) const;
    bool IsEvictionCandidate(const ResourceSlot &slot) const;
    void RemoveResidentCounters(const ResourceSlot &slot);
    void RefreshEvictableState(ResourceSlot &slot);
    void ClearResidencySlot(ResourceSlot &slot);
    void ClearDecodePlanRecord(std::size_t record_index);
    void ClearDecodePlansForCachePayload(ResourceHandle resource, std::uint64_t payload_id);
    void ClearDecodedPayloadRecord(std::size_t record_index);
    void ClearDecodedPayloadsForDecodeResult(
        ResourceHandle resource,
        std::uint64_t payload_id,
        std::uint64_t decode_plan_id,
        std::uint64_t decode_result_id);
    void ClearDecodeResultRecord(std::size_t record_index);
    void ClearDecodeResultsForDecodePlan(
        ResourceHandle resource,
        std::uint64_t payload_id,
        std::uint64_t decode_plan_id);
    void ClearDecodeResultsForCachePayload(ResourceHandle resource, std::uint64_t payload_id);
    void ClearCachePayloadRecord(std::size_t record_index);
    void ClearCachePayloadForSlot(std::size_t slot_index);
    bool HasInboundEdge(std::size_t slot_index) const;
    bool HasDependencyPath(std::size_t start_slot, std::size_t target_slot) const;
    void ClearOutboundEdges(std::size_t slot_index);
    void AdvanceGeneration(ResourceSlot& slot);

    std::array<ResourceSlot, MAX_RESOURCE_COUNT> slots_;
    std::array<ResourceDependencyEdge, MAX_DEPENDENCY_EDGE_COUNT> dependency_edges_;
    std::array<ResourceLoadCommitRecord, MAX_RESOURCE_LOAD_COMMIT_RECORD_COUNT> load_commit_records_;
    std::array<ResourceResidencyRecord, MAX_RESOURCE_RESIDENCY_RECORD_COUNT> residency_records_;
    std::array<ResourceCachePayloadRecord, MAX_RESOURCE_CACHE_PAYLOAD_RECORD_COUNT> cache_payload_records_;
    std::array<ResourceDecodePlanRecord, MAX_RESOURCE_DECODE_PLAN_RECORD_COUNT> decode_plan_records_;
    std::array<ResourceDecodeResultRecord, MAX_RESOURCE_DECODE_RESULT_RECORD_COUNT> decode_result_records_;
    std::array<ResourceDecodedPayloadRecord, MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT> decoded_payload_records_;
    std::array<
        std::array<std::uint8_t, MAX_RESOURCE_CACHE_PAYLOAD_BYTES_PER_RECORD>,
        MAX_RESOURCE_CACHE_PAYLOAD_RECORD_COUNT> cache_payload_bytes_;
    std::array<
        std::array<std::uint8_t, MAX_RESOURCE_DECODED_PAYLOAD_BYTES_PER_RECORD>,
        MAX_RESOURCE_DECODED_PAYLOAD_RECORD_COUNT> decoded_payload_bytes_;
    std::array<ResourceTypeId, MAX_RESOURCE_TYPE_COUNT> types_;
    ResourceSnapshot snapshot_;
    ResourceResidencySnapshot residency_snapshot_;
    ResourceCachePayloadSnapshot cache_payload_snapshot_;
    ResourceDecodePlanSnapshot decode_plan_snapshot_;
    ResourceDecodeResultSnapshot decode_result_snapshot_;
    ResourceDecodedPayloadSnapshot decoded_payload_snapshot_;
};
}
