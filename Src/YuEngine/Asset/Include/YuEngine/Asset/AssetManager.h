// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetManager.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/Asset/AssetDependencyEdge.h"
#include "YuEngine/Asset/AssetDescriptor.h"
#include "YuEngine/Asset/AssetManagerDesc.h"
#include "YuEngine/Asset/AssetRegistrationResult.h"
#include "YuEngine/Asset/AssetSlot.h"
#include "YuEngine/Asset/AssetSnapshot.h"
#include "YuEngine/AudioResource/AudioResourcePcmPacketImportRecord.h"
#include "YuEngine/Resource/ResourceDecodedPayloadRecord.h"
#include "YuEngine/Resource/ResourceResidencyState.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeResult.h"

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::asset {
class AssetManager final {
public:
    /**
     * @comment 构造 AssetManager 实例。
     */
    AssetManager();
    /**
     * @comment 构造 AssetManager 实例。
     * @param desc 输入描述。
     */
    explicit AssetManager(AssetManagerDesc desc);

    /**
     * @comment 注册 runtime asset，并在下层 Resource 上持有一份引用。
     * @param resource_registry 输入 Resource registry。
     * @param descriptor 输入 asset 描述。
     * @return 显式注册结果。
     */
    AssetRegistrationResult RegisterRuntimeAsset(
        yuengine::resource::ResourceRegistry *resource_registry,
        const AssetDescriptor &descriptor);
    /**
     * @comment 增加 asset 引用。
     * @param handle 输入 asset handle。
     * @return 显式操作状态。
     */
    AssetStatus AcquireAsset(AssetHandle handle);
    /**
     * @comment 释放 asset 引用。
     * @param handle 输入 asset handle。
     * @return 显式操作状态。
     */
    AssetStatus ReleaseAssetReference(AssetHandle handle);
    /**
     * @comment 释放 runtime asset，并释放注册时持有的 Resource 引用。
     * @param resource_registry 输入 Resource registry。
     * @param handle 输入 asset handle。
     * @return 显式操作状态。
     */
    AssetStatus ReleaseRuntimeAsset(
        yuengine::resource::ResourceRegistry *resource_registry,
        AssetHandle handle);
    /**
     * @comment 添加 asset 依赖边。
     * @param dependent 输入 dependent asset。
     * @param dependency 输入 dependency asset。
     * @return 显式操作状态。
     */
    AssetStatus AddDependency(AssetHandle dependent, AssetHandle dependency);
    /**
     * @comment 按确定顺序遍历 root asset 的依赖闭包。
     * @param root 输入 root asset。
     * @param output_assets 输出 asset handle 存储。
     * @param output_asset_capacity 输出存储容量。
     * @param output_asset_count 输出 asset 数量。
     * @return 显式操作状态。
     */
    AssetStatus TraverseDependencies(
        AssetHandle root,
        AssetHandle *output_assets,
        std::uint32_t output_asset_capacity,
        std::uint32_t *output_asset_count);
    /**
     * @comment 将 asset 标记为 loading。
     * @param handle 输入 asset handle。
     * @return 显式操作状态。
     */
    AssetStatus MarkAssetLoading(AssetHandle handle);
    /**
     * @comment 接收 Resource decoded payload 记录并将 asset 标记为 decoded。
     * @param handle 输入 asset handle。
     * @param decoded_payload 输入 decoded payload 记录。
     * @return 显式操作状态。
     */
    AssetStatus MarkAssetDecoded(
        AssetHandle handle,
        const yuengine::resource::ResourceDecodedPayloadRecord &decoded_payload);
    /**
     * @comment 接收 Streaming texture bridge 结果并记录 texture ready 值。
     * @param handle 输入 asset handle。
     * @param texture_result 输入 texture bridge 结果。
     * @return 显式操作状态。
     */
    AssetStatus MarkTextureReady(
        AssetHandle handle,
        const yuengine::streaming::ResourceDecodedTextureBridgeResult &texture_result);
    /**
     * @comment 接收 AudioResource PCM import 记录并记录 audio ready 值。
     * @param handle 输入 asset handle。
     * @param audio_record 输入 PCM import 记录。
     * @return 显式操作状态。
     */
    AssetStatus MarkAudioReady(
        AssetHandle handle,
        const yuengine::audioresource::AudioResourcePcmPacketImportRecord &audio_record);
    /**
     * @comment 从 Resource 加载和驻留状态刷新 asset 状态。
     * @param resource_registry 输入 Resource registry。
     * @param handle 输入 asset handle。
     * @return 显式操作状态。
     */
    AssetStatus RefreshStateFromResource(
        yuengine::resource::ResourceRegistry *resource_registry,
        AssetHandle handle);
    /**
     * @comment 查询 active asset 记录。
     * @param handle 输入 asset handle。
     * @param output_record 输出记录。
     * @return 显式操作状态。
     */
    AssetStatus QueryAsset(AssetHandle handle, AssetRecord *output_record);
    /**
     * @comment 返回当前状态快照。
     * @return 快照值。
     */
    AssetSnapshot Snapshot() const;

private:
    AssetStatus RecordFailure(AssetStatus status);
    AssetStatus RecordResourceFailure(
        AssetStatus status,
        yuengine::resource::ResourceStatus resource_status);
    AssetStatus RecordResourceLoadFailure(
        AssetStatus status,
        yuengine::resource::ResourceLoadCommitStatus resource_status);
    AssetStatus RecordResourceResidencyFailure(
        AssetStatus status,
        yuengine::resource::ResourceResidencyStatus resource_status);
    AssetStatus RecordTextureFailure(
        AssetStatus status,
        yuengine::streaming::ResourceDecodedTextureBridgeStatus texture_status);
    AssetStatus RecordAudioFailure(
        AssetStatus status,
        yuengine::audioresource::AudioResourcePcmPacketImportStatus audio_status);
    void RecordSuccess();
    AssetStatus ResolveHandle(AssetHandle handle, std::size_t &out_index) const;
    bool ValidateDescriptor(const AssetDescriptor &descriptor, AssetStatus *output_status) const;
    bool HasType(AssetTypeId type) const;
    bool HasDuplicateActiveAsset(const AssetDescriptor &descriptor) const;
    bool HasDependencyEdge(AssetHandle dependent, AssetHandle dependency) const;
    bool HasDependencyPath(std::uint32_t start_slot, std::uint32_t target_slot) const;
    bool IsResourceResidentState(yuengine::resource::ResourceResidencyState state) const;
    bool DoResourceHandlesMatch(
        yuengine::resource::ResourceHandle left,
        yuengine::resource::ResourceHandle right) const;
    bool DoResourceTypesMatch(
        yuengine::resource::ResourceTypeId left,
        yuengine::resource::ResourceTypeId right) const;
    AssetSlot *FindFreeSlot(std::uint32_t *out_slot_index);
    AssetDependencyEdge *FindFreeDependencyEdge();
    void RegisterTypeIfNeeded(AssetTypeId type);
    void ClearDependencyEdgesForSlot(std::uint32_t slot_index);
    void AdvanceGeneration(AssetSlot &slot);

    std::array<AssetSlot, MAX_ASSET_COUNT> slots_;
    std::array<AssetTypeId, MAX_ASSET_TYPE_COUNT> types_;
    std::array<AssetDependencyEdge, MAX_ASSET_DEPENDENCY_EDGE_COUNT> dependency_edges_;
    AssetSnapshot snapshot_;
};
}
