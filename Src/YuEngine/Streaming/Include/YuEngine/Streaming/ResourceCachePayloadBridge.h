// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceCachePayloadBridge.h
#pragma once

#include "YuEngine/Streaming/ResourceCachePayloadBridgeRequest.h"
#include "YuEngine/Streaming/ResourceCachePayloadBridgeResult.h"
#include "YuEngine/Streaming/ResourceCachePayloadBridgeSnapshot.h"
#include "YuEngine/Streaming/ResourceCachePayloadBridgeStatus.h"

namespace yuengine::streaming {

class ResourceCachePayloadBridge final {
public:
    /**
     * @comment 创建 Resource cache payload bridge。
     */
    ResourceCachePayloadBridge();

    /**
     * @comment 把 Package staging completion 中的 payload window 写入 Resource cache payload。
     * @param request Streaming 侧桥接请求。
     * @return 桥接结果。
     */
    ResourceCachePayloadBridgeResult StorePayload(const ResourceCachePayloadBridgeRequest &request);

    /**
     * @comment 获取桥接统计快照。
     * @return 当前快照。
     */
    ResourceCachePayloadBridgeSnapshot Snapshot() const;

private:
    ResourceCachePayloadBridgeStatus ValidateRequest(const ResourceCachePayloadBridgeRequest &request) const;
    ResourceCachePayloadBridgeResult BuildResult(const ResourceCachePayloadBridgeRequest &request) const;
    void RecordRejected(const ResourceCachePayloadBridgeResult &result);
    void RecordFailed(const ResourceCachePayloadBridgeResult &result);
    void RecordCompleted(const ResourceCachePayloadBridgeResult &result);
    void StoreLastResult(const ResourceCachePayloadBridgeResult &result);

private:
    ResourceCachePayloadBridgeSnapshot snapshot_;
};

} // namespace yuengine::streaming
