// Module: YuEngine Streaming
// File: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceDecodedTextureBridge.h

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
     * @comment Constructs a Resource decoded texture bridge with bounded upload storage.
     */
    ResourceDecodedTextureBridge();

    /**
     * @comment Reads a Resource-owned decoded texture payload and uploads it as an RHI texture.
     * @param request Input decoded payload and upload request.
     * @return Explicit bridge result and texture binding values.
     */
    ResourceDecodedTextureBridgeResult UploadTexture(const ResourceDecodedTextureBridgeRequest &request);
    /**
     * @comment Returns bridge counters and last statuses.
     * @return Snapshot value.
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
