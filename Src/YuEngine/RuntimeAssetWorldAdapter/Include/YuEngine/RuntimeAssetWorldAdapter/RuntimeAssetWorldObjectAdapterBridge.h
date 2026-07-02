// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterBridge.h

#pragma once

#include <cstdint>

#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterIdentityRecord.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterResult.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterSnapshot.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectProducerPlaybackRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectTimelineTransformSampleRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectTransformApplicationRequest.h"
#include "YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectTransformSamplerBridgeRequest.h"
#include "YuEngine/World/WorldObjectId.h"

namespace yuengine::animation {
enum class AnimationRuntimeStatus;
}

namespace yuengine::runtimeasset {
struct RuntimeAssetRuntimeInstanceMappingRecord;
}

namespace yuengine::runtimeassetworldadapter {
class RuntimeAssetWorldObjectAdapterBridge final {
public:
    /**
     * @comment 从 RuntimeAsset runtime instance mapping 生成 World restore records。
     * @param request 调用方持有的输入和输出数组。
     * @return 显式操作结果。
     */
    RuntimeAssetWorldObjectAdapterResult BuildRestoreRecords(
        const RuntimeAssetWorldObjectAdapterRequest &request);
    /**
     * @comment 从 producer/export playback 输入装配 timeline transform application 请求。
     * @param request RuntimeAsset mapping、identity、export playback 输入和 sampled value 输出。
     * @param out_request 调用方持有的 transform application request 输出。
     * @return 显式操作结果。
     */
    RuntimeAssetWorldObjectAdapterResult BuildProducerPlaybackTransformApplicationRequest(
        const RuntimeAssetWorldObjectProducerPlaybackRequest &request,
        RuntimeAssetWorldObjectTransformApplicationRequest *out_request);
    /**
     * @comment 从 caller-owned producer playback 批量输入原子装配 transform application 请求。
     * @param request producer playback 输入数组和 batch scratch/output。
     * @param out_request 调用方持有的 transform application request 输出。
     * @return 显式操作结果。
     */
    RuntimeAssetWorldObjectAdapterResult BuildProducerPlaybackBatchTransformApplicationRequest(
        const RuntimeAssetWorldObjectProducerPlaybackBatchRequest &request,
        RuntimeAssetWorldObjectTransformApplicationRequest *out_request);
    /**
     * @comment 预检 producer playback 批量输入并返回最终 sampled output 所需数量。
     * @param request producer playback 输入数组和 batch scratch。
     * @return 显式操作结果。
     */
    RuntimeAssetWorldObjectAdapterResult SnapshotProducerPlaybackBatchTransformApplicationCount(
        const RuntimeAssetWorldObjectProducerPlaybackBatchRequest &request);
    /**
     * @comment 从 caller-owned timeline/export transform sample 输入装配 transform application 请求。
     * @param request RuntimeAsset mapping、identity、timeline sample 输入和 sampled value 输出。
     * @param out_request 调用方持有的 transform application request 输出。
     * @return 显式操作结果。
     */
    RuntimeAssetWorldObjectAdapterResult BuildTimelineTransformApplicationRequest(
        const RuntimeAssetWorldObjectTimelineTransformSampleRequest &request,
        RuntimeAssetWorldObjectTransformApplicationRequest *out_request);
    /**
     * @comment 预检 timeline/export transform sample 输入并返回 sampled output 所需数量。
     * @param request RuntimeAsset mapping、identity、timeline sample 输入和 scratch/output 容量。
     * @return 显式操作结果。
     */
    RuntimeAssetWorldObjectAdapterResult SnapshotTimelineTransformApplicationCount(
        const RuntimeAssetWorldObjectTimelineTransformSampleRequest &request);
    /**
     * @comment 从 Animation runtime sampler 输出装配 RuntimeAssetWorldObject transform application 请求。
     * @param request RuntimeAsset mapping、identity、sample request 和 sampled value 输出。
     * @param out_request 调用方持有的 transform application request 输出。
     * @return 显式操作结果。
     */
    RuntimeAssetWorldObjectAdapterResult BuildTransformApplicationRequest(
        const RuntimeAssetWorldObjectTransformSamplerBridgeRequest &request,
        RuntimeAssetWorldObjectTransformApplicationRequest *out_request);
    /**
     * @comment 将 RuntimeAsset sampled transform values 直接应用到调用方持有的 World transform bridge。
     * @param request RuntimeAsset mapping、identity 和 sampled transform 输入。
     * @return 显式操作结果。
     */
    RuntimeAssetWorldObjectAdapterResult ApplySampledTransforms(
        const RuntimeAssetWorldObjectTransformApplicationRequest &request);
    /**
     * @comment Applies RuntimeAsset target id sampled transform values to caller-owned World transform bridge.
     * @param request RuntimeAsset mapping, identity, and target id sampled transform inputs.
     * @return Explicit operation result.
     */
    RuntimeAssetWorldObjectAdapterResult ApplyRuntimeSampledTransforms(
        const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request);
    /**
     * @comment 在调用方持有的 World transform records 写入前校验 RuntimeAsset sampled transform 目标。
     * @param request RuntimeAsset mapping、identity 和 sampled transform 输入。
     * @return 显式操作结果。
     */
    RuntimeAssetWorldObjectAdapterResult PreflightSampledTransforms(
        const RuntimeAssetWorldObjectTransformApplicationRequest &request);

    /**
     * @comment 返回当前 bridge 状态快照。
     * @return 快照值。
     */
    RuntimeAssetWorldObjectAdapterSnapshot Snapshot() const;

private:
    RuntimeAssetWorldObjectAdapterResult RecordFailure(
        RuntimeAssetWorldObjectAdapterStatus status);
    RuntimeAssetWorldObjectAdapterResult RecordFailure(
        RuntimeAssetWorldObjectAdapterStatus status,
        std::uint32_t failed_mapping_index,
        std::uint64_t failed_target_id,
        std::uint32_t required_identity_output_count,
        std::uint32_t required_transform_output_count);
    RuntimeAssetWorldObjectAdapterResult RecordSuccess(
        const RuntimeAssetWorldObjectAdapterState &state);
    RuntimeAssetWorldObjectAdapterResult RecordTransformApplicationRequestSuccess(
        const RuntimeAssetWorldObjectAdapterState &state);
    RuntimeAssetWorldObjectAdapterResult RecordTransformApplicationSuccess(
        const RuntimeAssetWorldObjectAdapterState &state);
    RuntimeAssetWorldObjectAdapterStatus ValidateRequest(
        const RuntimeAssetWorldObjectAdapterRequest &request,
        std::uint32_t *out_failed_mapping_index,
        std::uint64_t *out_failed_target_id,
        std::uint32_t *out_required_identity_output_count,
        std::uint32_t *out_required_transform_output_count) const;
    RuntimeAssetWorldObjectAdapterStatus ValidateTransformApplicationRequest(
        const RuntimeAssetWorldObjectTransformApplicationRequest &request) const;
    RuntimeAssetWorldObjectAdapterStatus ValidateRuntimeSampledTransformApplicationRequest(
        const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
        std::uint64_t *out_failed_target_id) const;
    RuntimeAssetWorldObjectAdapterStatus ValidateTransformSamplerRequest(
        const RuntimeAssetWorldObjectTransformSamplerBridgeRequest &request,
        const RuntimeAssetWorldObjectTransformApplicationRequest *out_request) const;
    RuntimeAssetWorldObjectAdapterStatus ValidateProducerPlaybackBatchRequest(
        const RuntimeAssetWorldObjectProducerPlaybackBatchRequest &request,
        const RuntimeAssetWorldObjectTransformApplicationRequest *out_request) const;
    RuntimeAssetWorldObjectAdapterStatus ValidateProducerPlaybackBatchCountRequest(
        const RuntimeAssetWorldObjectProducerPlaybackBatchRequest &request) const;
    RuntimeAssetWorldObjectAdapterStatus ValidateProducerPlaybackRequest(
        const RuntimeAssetWorldObjectProducerPlaybackRequest &request) const;
    RuntimeAssetWorldObjectAdapterStatus ValidateTimelineTransformSampleRequest(
        const RuntimeAssetWorldObjectTimelineTransformSampleRequest &request,
        const RuntimeAssetWorldObjectTransformApplicationRequest *out_request) const;
    RuntimeAssetWorldObjectAdapterStatus ValidateTimelineTransformSampleCountRequest(
        const RuntimeAssetWorldObjectTimelineTransformSampleRequest &request) const;
    RuntimeAssetWorldObjectAdapterResult PreflightSampledTransformsWithTargetDiagnostics(
        const RuntimeAssetWorldObjectTransformApplicationRequest &request);
    RuntimeAssetWorldObjectAdapterStatus ValidateRuntimeInstanceMapping(
        const RuntimeAssetWorldObjectAdapterRequest &request,
        std::uint32_t mapping_index) const;
    RuntimeAssetWorldObjectAdapterStatus ValidateRuntimeInstanceMapping(
        const RuntimeAssetWorldObjectTransformApplicationRequest &request,
        std::uint32_t mapping_index) const;
    RuntimeAssetWorldObjectAdapterStatus ValidateRuntimeInstanceMapping(
        const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
        std::uint32_t mapping_index) const;
    RuntimeAssetWorldObjectAdapterStatus ValidateSampledTransformTargets(
        const RuntimeAssetWorldObjectTransformApplicationRequest &request) const;
    RuntimeAssetWorldObjectAdapterStatus ValidateRuntimeSampledTransformTargets(
        const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
        std::uint64_t *out_failed_target_id) const;
    const RuntimeAssetWorldObjectAdapterIdentityRecord *FindIdentityRecord(
        const RuntimeAssetWorldObjectAdapterRequest &request,
        std::uint64_t target_id) const;
    const RuntimeAssetWorldObjectAdapterIdentityRecord *FindIdentityRecord(
        const RuntimeAssetWorldObjectTransformApplicationRequest &request,
        std::uint64_t target_id) const;
    const RuntimeAssetWorldObjectAdapterIdentityRecord *FindIdentityRecord(
        const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
        std::uint64_t target_id) const;
    const yuengine::runtimeasset::RuntimeAssetRuntimeInstanceMappingRecord *FindRuntimeInstanceMapping(
        const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
        std::uint64_t target_id) const;
    bool HasDuplicateMappingTargetId(
        const RuntimeAssetWorldObjectAdapterRequest &request,
        std::uint32_t mapping_index) const;
    bool HasDuplicateMappingTargetId(
        const RuntimeAssetWorldObjectTransformApplicationRequest &request,
        std::uint32_t mapping_index) const;
    bool HasDuplicateMappingTargetId(
        const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
        std::uint32_t mapping_index) const;
    bool HasDuplicateWorldObjectId(
        const RuntimeAssetWorldObjectAdapterRequest &request,
        std::uint32_t mapping_index,
        yuengine::world::WorldObjectId world_object_id) const;
    bool HasDuplicateWorldObjectId(
        const RuntimeAssetWorldObjectTransformApplicationRequest &request,
        std::uint32_t mapping_index,
        yuengine::world::WorldObjectId world_object_id) const;
    bool HasDuplicateWorldObjectId(
        const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
        std::uint32_t mapping_index,
        yuengine::world::WorldObjectId world_object_id) const;
    bool HasDuplicateObjectHandle(
        const RuntimeAssetWorldObjectAdapterRequest &request,
        std::uint32_t mapping_index,
        yuengine::object::ObjectHandle object_handle) const;
    bool HasDuplicateObjectHandle(
        const RuntimeAssetWorldObjectTransformApplicationRequest &request,
        std::uint32_t mapping_index,
        yuengine::object::ObjectHandle object_handle) const;
    bool HasDuplicateObjectHandle(
        const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
        std::uint32_t mapping_index,
        yuengine::object::ObjectHandle object_handle) const;
    bool HasMappedWorldObject(
        const RuntimeAssetWorldObjectTransformApplicationRequest &request,
        yuengine::world::WorldObjectId world_object_id) const;
    bool ResolveRuntimeSampledTransformTarget(
        const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
        std::uint64_t target_id,
        yuengine::world::WorldObjectId *out_world_object_id) const;
    bool IsFirstRuntimeSampledTransformTargetOccurrence(
        const RuntimeAssetWorldObjectRuntimeSampledTransformApplicationRequest &request,
        std::uint32_t sampled_value_index) const;
    bool ObjectHandlesMatch(
        yuengine::object::ObjectHandle left,
        yuengine::object::ObjectHandle right) const;
    RuntimeAssetWorldObjectAdapterStatus MapAnimationStatus(
        yuengine::animation::AnimationRuntimeStatus status) const;
    RuntimeAssetWorldObjectAdapterStatus MapAnimationSampleStatus(
        yuengine::animation::AnimationRuntimeStatus status) const;

    RuntimeAssetWorldObjectAdapterSnapshot snapshot_;
};
}
