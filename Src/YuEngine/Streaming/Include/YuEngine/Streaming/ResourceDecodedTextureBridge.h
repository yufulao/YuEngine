// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceDecodedTextureBridge.h

#pragma once

#include <cstdint>

#include "YuEngine/Streaming/ResourceDecodedTextureBridgeRequest.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeResult.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeSnapshot.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeStatus.h"
#include "YuEngine/Streaming/ResourceUploadQueue.h"

namespace yuengine::streaming {
class ResourceDecodedTextureBridge final {
public:
    /**
     * @comment 构造带固定容量 upload 存储的 Resource decoded texture bridge。
     */
    ResourceDecodedTextureBridge();

    /**
     * @comment 读取 Resource 持有的 decoded texture payload，并上传为 RHI texture。
     * @param request 输入 decoded payload 和 upload 请求。
     * @return 显式 bridge 结果和 texture 绑定值。
     */
    ResourceDecodedTextureBridgeResult UploadTexture(const ResourceDecodedTextureBridgeRequest &request);
    /**
     * @comment 返回 bridge 计数器和最近状态。
     * @return 快照值。
     */
    ResourceDecodedTextureBridgeSnapshot Snapshot() const;

private:
    ResourceDecodedTextureBridgeStatus ValidateRequest(
        const ResourceDecodedTextureBridgeRequest &request) const;
    ResourceDecodedTextureBridgeStatus ValidateTextureByteCount(
        const ResourceDecodedTextureBridgeRequest &request,
        std::uint32_t decoded_byte_count) const;
    ResourceDecodedTextureBridgeResult BuildResult(const ResourceDecodedTextureBridgeRequest &request) const;
    ResourceUploadRequest BuildUploadRequest(
        const ResourceDecodedTextureBridgeRequest &request,
        std::uint32_t decoded_byte_count) const;
    ResourceDecodedTextureBridgeResult RecordRejected(ResourceDecodedTextureBridgeResult result);
    ResourceDecodedTextureBridgeResult RecordFailed(ResourceDecodedTextureBridgeResult result);
    ResourceDecodedTextureBridgeResult RecordCompleted(ResourceDecodedTextureBridgeResult result);
    void StoreLastResult(const ResourceDecodedTextureBridgeResult &result);

    ResourceUploadQueue upload_queue_;
    ResourceDecodedTextureBridgeSnapshot snapshot_;
};
}
