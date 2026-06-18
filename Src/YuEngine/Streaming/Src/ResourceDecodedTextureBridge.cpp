// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Src/ResourceDecodedTextureBridge.cpp

#include "YuEngine/Streaming/ResourceDecodedTextureBridge.h"

#include <array>
#include <limits>
#include <span>

#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiFormat.h"
#include "YuEngine/Streaming/PackageResourceStagingCompletion.h"

namespace yuengine::streaming {
namespace {
bool TextureByteCountOverflows(const rhi::RhiTextureDesc &desc) {
    const std::uint64_t width = static_cast<std::uint64_t>(desc.extent.width);
    const std::uint64_t height = static_cast<std::uint64_t>(desc.extent.height);
    const std::uint64_t pixel_bytes = static_cast<std::uint64_t>(rhi::RGBA8_BYTES_PER_PIXEL);
    const std::uint64_t byte_count = width * height * pixel_bytes;
    return byte_count > static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max());
}

std::uint32_t TextureByteCount(const rhi::RhiTextureDesc &desc) {
    const std::uint32_t width = static_cast<std::uint32_t>(desc.extent.width);
    const std::uint32_t height = static_cast<std::uint32_t>(desc.extent.height);
    return width * height * static_cast<std::uint32_t>(rhi::RGBA8_BYTES_PER_PIXEL);
}
}

ResourceDecodedTextureBridge::ResourceDecodedTextureBridge()
    : upload_queue_(),
      snapshot_() {
}

ResourceDecodedTextureBridgeResult ResourceDecodedTextureBridge::UploadTexture(
    const ResourceDecodedTextureBridgeRequest &request) {
    ResourceDecodedTextureBridgeResult result = BuildResult(request);
    result.status = ValidateRequest(request);
    if (result.status != ResourceDecodedTextureBridgeStatus::Success) {
        return RecordRejected(result);
    }

    resource::ResourceDecodedPayloadRecord decoded_record;
    result.decoded_payload_status = request.resource_registry->QueryDecodedPayload(
        request.decoded_payload,
        &decoded_record);
    if (result.decoded_payload_status != resource::ResourceDecodedPayloadStatus::Success) {
        result.status = ResourceDecodedTextureBridgeStatus::ResourceQueryFailed;
        return RecordFailed(result);
    }

    result.decoded_payload_record = decoded_record;
    result.decoded_byte_count = decoded_record.decoded_byte_count;
    result.status = ValidateTextureByteCount(request, decoded_record.decoded_byte_count);
    if (result.status != ResourceDecodedTextureBridgeStatus::Success) {
        return RecordFailed(result);
    }

    if (request.scratch_bytes.size() < decoded_record.decoded_byte_count) {
        result.status = ResourceDecodedTextureBridgeStatus::ScratchBufferTooSmall;
        return RecordFailed(result);
    }

    std::uint32_t read_byte_count = 0U;
    result.decoded_payload_status = request.resource_registry->ReadDecodedPayload(
        request.decoded_payload,
        request.scratch_bytes.data(),
        static_cast<std::uint32_t>(request.scratch_bytes.size()),
        &read_byte_count);
    if (result.decoded_payload_status != resource::ResourceDecodedPayloadStatus::Success) {
        result.status = ResourceDecodedTextureBridgeStatus::ResourceReadFailed;
        return RecordFailed(result);
    }

    result.decoded_byte_count = read_byte_count;
    const ResourceUploadRequest upload_request = BuildUploadRequest(request, read_byte_count);
    result.upload_status = upload_queue_.Submit(upload_request);
    if (result.upload_status != ResourceUploadStatus::Queued) {
        result.status = ResourceDecodedTextureBridgeStatus::UploadSubmitFailed;
        return RecordFailed(result);
    }

    ++snapshot_.submitted_count;
    result.upload_status = upload_queue_.ProcessNext();

    std::array<ResourceUploadCompletion, 1U> completions{};
    std::uint32_t written_count = 0U;
    const ResourceUploadStatus drain_status = upload_queue_.DrainCompletions(
        completions.data(),
        static_cast<std::uint32_t>(completions.size()),
        &written_count);
    if (drain_status != ResourceUploadStatus::Success || written_count != 1U) {
        result.upload_status = drain_status;
        result.status = ResourceDecodedTextureBridgeStatus::UploadCompletionMissing;
        return RecordFailed(result);
    }

    result.upload_completion = completions[0U];
    result.rhi_status = result.upload_completion.rhi_status;
    result.uploaded_byte_count = result.upload_completion.upload_byte_count;
    if (result.upload_status != ResourceUploadStatus::Success) {
        result.status = ResourceDecodedTextureBridgeStatus::UploadProcessFailed;
        return RecordFailed(result);
    }

    result.texture_handle = result.upload_completion.texture_handle;
    result.sampled_texture.texture = result.texture_handle;
    result.sampled_texture.slot = request.sampled_texture_slot;
    result.status = ResourceDecodedTextureBridgeStatus::Success;
    return RecordCompleted(result);
}

ResourceDecodedTextureBridgeSnapshot ResourceDecodedTextureBridge::Snapshot() const {
    return snapshot_;
}

ResourceDecodedTextureBridgeStatus ResourceDecodedTextureBridge::ValidateRequest(
    const ResourceDecodedTextureBridgeRequest &request) const {
    if (request.resource_registry == nullptr) {
        return ResourceDecodedTextureBridgeStatus::InvalidArgument;
    }

    if (request.rhi_device == nullptr) {
        return ResourceDecodedTextureBridgeStatus::InvalidArgument;
    }

    if (request.output_texture_handle == nullptr) {
        return ResourceDecodedTextureBridgeStatus::InvalidArgument;
    }

    if (request.staging_request_id == 0U) {
        return ResourceDecodedTextureBridgeStatus::InvalidArgument;
    }

    if (request.upload_id == 0U) {
        return ResourceDecodedTextureBridgeStatus::InvalidArgument;
    }

    if (request.scratch_bytes.data() == nullptr) {
        return ResourceDecodedTextureBridgeStatus::InvalidArgument;
    }

    if (request.scratch_bytes.size() > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max())) {
        return ResourceDecodedTextureBridgeStatus::InvalidArgument;
    }

    if (request.texture_desc.format != rhi::RhiFormat::Rgba8Unorm) {
        return ResourceDecodedTextureBridgeStatus::InvalidArgument;
    }

    if (request.texture_desc.extent.width == 0U) {
        return ResourceDecodedTextureBridgeStatus::InvalidArgument;
    }

    if (request.texture_desc.extent.height == 0U) {
        return ResourceDecodedTextureBridgeStatus::InvalidArgument;
    }

    if (request.sampled_texture_slot >= rhi::MAX_RHI_SAMPLED_TEXTURE_SLOTS) {
        return ResourceDecodedTextureBridgeStatus::SampledTextureSlotOutOfRange;
    }

    if (TextureByteCountOverflows(request.texture_desc)) {
        return ResourceDecodedTextureBridgeStatus::InvalidArgument;
    }

    return ResourceDecodedTextureBridgeStatus::Success;
}

ResourceDecodedTextureBridgeStatus ResourceDecodedTextureBridge::ValidateTextureByteCount(
    const ResourceDecodedTextureBridgeRequest &request,
    std::uint32_t decoded_byte_count) const {
    const std::uint32_t texture_byte_count = TextureByteCount(request.texture_desc);
    if (texture_byte_count != decoded_byte_count) {
        return ResourceDecodedTextureBridgeStatus::TextureByteCountMismatch;
    }

    return ResourceDecodedTextureBridgeStatus::Success;
}

ResourceDecodedTextureBridgeResult ResourceDecodedTextureBridge::BuildResult(
    const ResourceDecodedTextureBridgeRequest &request) const {
    ResourceDecodedTextureBridgeResult result;
    result.texture_handle = rhi::RhiTextureHandle{};
    result.sampled_texture.texture = result.texture_handle;
    result.sampled_texture.slot = request.sampled_texture_slot;
    return result;
}

ResourceUploadRequest ResourceDecodedTextureBridge::BuildUploadRequest(
    const ResourceDecodedTextureBridgeRequest &request,
    std::uint32_t decoded_byte_count) const {
    PackageResourceStagingCompletion staging_completion;
    staging_completion.status = PackageResourceStagingStatus::Success;
    staging_completion.resource = request.decoded_payload.resource;
    staging_completion.expected_type = request.decoded_payload.expected_type;
    staging_completion.request_id = request.staging_request_id;
    staging_completion.file_byte_count = decoded_byte_count;
    staging_completion.staged_byte_offset = 0U;
    staging_completion.staged_byte_count = decoded_byte_count;

    ResourceUploadRequest upload_request;
    upload_request.resource_registry = request.resource_registry;
    upload_request.rhi_device = request.rhi_device;
    upload_request.staging_completion = staging_completion;
    upload_request.resource = request.decoded_payload.resource;
    upload_request.expected_type = request.decoded_payload.expected_type;
    upload_request.staged_bytes = std::span<const std::uint8_t>(request.scratch_bytes.data(), decoded_byte_count);
    upload_request.upload_byte_count = decoded_byte_count;
    upload_request.upload_kind = ResourceUploadKind::CreateTexture;
    upload_request.texture_desc = request.texture_desc;
    upload_request.output_texture_handle = request.output_texture_handle;
    upload_request.upload_id = request.upload_id;
    return upload_request;
}

ResourceDecodedTextureBridgeResult ResourceDecodedTextureBridge::RecordRejected(
    ResourceDecodedTextureBridgeResult result) {
    ++snapshot_.rejected_count;
    StoreLastResult(result);
    return result;
}

ResourceDecodedTextureBridgeResult ResourceDecodedTextureBridge::RecordFailed(
    ResourceDecodedTextureBridgeResult result) {
    ++snapshot_.failed_count;
    StoreLastResult(result);
    return result;
}

ResourceDecodedTextureBridgeResult ResourceDecodedTextureBridge::RecordCompleted(
    ResourceDecodedTextureBridgeResult result) {
    ++snapshot_.completed_count;
    StoreLastResult(result);
    return result;
}

void ResourceDecodedTextureBridge::StoreLastResult(const ResourceDecodedTextureBridgeResult &result) {
    snapshot_.last_decoded_byte_count = result.decoded_byte_count;
    snapshot_.last_uploaded_byte_count = result.uploaded_byte_count;
    snapshot_.last_status = result.status;
    snapshot_.last_decoded_payload_status = result.decoded_payload_status;
    snapshot_.last_upload_status = result.upload_status;
    snapshot_.last_rhi_status = result.rhi_status;
}
}
