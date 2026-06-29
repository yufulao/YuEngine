// 模块：Tests Streaming
// 文件：Tests/Streaming/StreamingTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>

#include "YuEngine/File/AsyncFileReadQueue.h"
#include "YuEngine/File/MountTable.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Package/PackageArtifact.h"
#include "YuEngine/Package/PackageRegistry.h"
#include "YuEngine/Package/PackageStatus.h"
#include "YuEngine/Resource/ResourceCachePayloadRequest.h"
#include "YuEngine/Resource/ResourceCachePayloadSnapshot.h"
#include "YuEngine/Resource/ResourceCachePayloadStatus.h"
#include "YuEngine/Resource/ResourceDescriptor.h"
#include "YuEngine/Resource/ResourceLoadCommitRequest.h"
#include "YuEngine/Resource/ResourceLoadCommitStatus.h"
#include "YuEngine/Resource/ResourceLoadState.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/Resource/ResourceResidencyBudgetDesc.h"
#include "YuEngine/Resource/ResourceResidencyRequest.h"
#include "YuEngine/Resource/ResourceResidencyStatus.h"
#include "YuEngine/Rhi/NullRhiDevice.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiBufferUsage.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiFenceHandle.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Streaming/PackageResourceStagingQueue.h"
#include "YuEngine/Streaming/ResourceCachePayloadBridge.h"
#include "YuEngine/Streaming/ResourceCachePayloadBridgeRequest.h"
#include "YuEngine/Streaming/ResourceCachePayloadBridgeResult.h"
#include "YuEngine/Streaming/ResourceCachePayloadBridgeSnapshot.h"
#include "YuEngine/Streaming/ResourceCachePayloadBridgeStatus.h"
#include "YuEngine/Streaming/ResourceStreamingPipeline.h"
#include "YuEngine/Streaming/ResourceStreamingPipelineRequest.h"
#include "YuEngine/Streaming/ResourceStreamingPipelineSnapshot.h"
#include "YuEngine/Streaming/ResourceStreamingPipelineStatus.h"
#include "YuEngine/Streaming/ResourceUploadCommitQueue.h"
#include "YuEngine/Streaming/ResourceUploadQueue.h"

using yuengine::file::AsyncFileReadQueue;
using yuengine::file::AsyncFileReadRequest;
using yuengine::file::AsyncFileReadResult;
using yuengine::file::AsyncFileReadStatus;
using yuengine::file::FileStatus;
using yuengine::file::MountId;
using yuengine::file::MountTable;
using yuengine::file::VirtualPath;
using yuengine::memory::MemoryAccountingStatus;
using yuengine::package::PackageArtifactReadRequest;
using yuengine::package::PackageArtifactResult;
using yuengine::package::PackageArtifactWriteRequest;
using yuengine::package::PackageEntryDescriptor;
using yuengine::package::PackageEntryId;
using yuengine::package::PackageId;
using yuengine::package::PackageLoadPlanRecord;
using yuengine::package::PackageLoadPlanResult;
using yuengine::package::PackageRegistry;
using yuengine::package::PackageRegistrationResult;
using yuengine::package::PackageSourceKey;
using yuengine::package::PackageStatus;
using yuengine::package::ReadPackageArtifact;
using yuengine::package::WritePackageArtifact;
using yuengine::resource::ResourceCachePayloadRequest;
using yuengine::resource::ResourceCachePayloadSnapshot;
using yuengine::resource::ResourceCachePayloadStatus;
using yuengine::resource::ResourceDescriptor;
using yuengine::resource::ResourceHandle;
using yuengine::resource::ResourceLoadCommitRequest;
using yuengine::resource::ResourceLoadCommitStatus;
using yuengine::resource::ResourceLoadState;
using yuengine::resource::ResourceLogicalKey;
using yuengine::resource::ResourceRegistrationResult;
using yuengine::resource::ResourceRegistry;
using yuengine::resource::ResourceResidencyBudgetDesc;
using yuengine::resource::ResourceResidencyRequest;
using yuengine::resource::ResourceResidencyStatus;
using yuengine::resource::ResourceStatus;
using yuengine::resource::ResourceTypeId;
using yuengine::rhi::NullRhiDevice;
using yuengine::rhi::RhiBufferDesc;
using yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiBufferUsage;
using yuengine::rhi::RhiDeviceDesc;
using yuengine::rhi::RhiFenceHandle;
using yuengine::rhi::RhiStatus;
using yuengine::rhi::RhiTextureDesc;
using yuengine::rhi::RhiTextureHandle;
using yuengine::streaming::PackageResourceStagingCompletion;
using yuengine::streaming::PackageResourceStagingQueue;
using yuengine::streaming::PackageResourceStagingQueueDesc;
using yuengine::streaming::PackageResourceStagingRequest;
using yuengine::streaming::PackageResourceStagingSnapshot;
using yuengine::streaming::PackageResourceStagingStatus;
using yuengine::streaming::ResourceCachePayloadBridge;
using yuengine::streaming::ResourceCachePayloadBridgeRequest;
using yuengine::streaming::ResourceCachePayloadBridgeResult;
using yuengine::streaming::ResourceCachePayloadBridgeSnapshot;
using yuengine::streaming::ResourceCachePayloadBridgeStatus;
using yuengine::streaming::ResourceStreamingPipeline;
using yuengine::streaming::ResourceStreamingPipelineRequest;
using yuengine::streaming::ResourceStreamingPipelineSnapshot;
using yuengine::streaming::ResourceStreamingPipelineStatus;
using yuengine::streaming::ResourceUploadCommitCompletion;
using yuengine::streaming::ResourceUploadCommitQueue;
using yuengine::streaming::ResourceUploadCommitQueueDesc;
using yuengine::streaming::ResourceUploadCommitRequest;
using yuengine::streaming::ResourceUploadCommitSnapshot;
using yuengine::streaming::ResourceUploadCommitStatus;
using yuengine::streaming::ResourceUploadCompletion;
using yuengine::streaming::ResourceUploadKind;
using yuengine::streaming::ResourceUploadQueue;
using yuengine::streaming::ResourceUploadQueueDesc;
using yuengine::streaming::ResourceUploadRequest;
using yuengine::streaming::ResourceUploadSnapshot;
using yuengine::streaming::ResourceUploadStatus;

namespace {
constexpr const char *TEST_SUBMIT_COMPLETE =
    "Streaming_PackageResourceStaging_SubmitsAndCompletesFixtureRead";
constexpr const char *TEST_INVALID_HANDLE =
    "Streaming_PackageResourceStaging_RejectsInvalidResourceHandleWithoutMutation";
constexpr const char *TEST_TYPE_MISMATCH =
    "Streaming_PackageResourceStaging_RejectsResourceTypeMismatchWithoutMutation";
constexpr const char *TEST_ARCHIVE_RANGE =
    "Streaming_PackageResourceStaging_UsesArchiveByteRangeForFileRead";
constexpr const char *TEST_LEGACY_MIRROR =
    "Streaming_PackageResourceStaging_PreservesLegacyMirrorAsMetadataOnly";
constexpr const char *TEST_OUTPUT_CAPACITY =
    "Streaming_PackageResourceStaging_RejectsArchiveByteSizeOverOutputCapacity";
constexpr const char *TEST_BYTE_RANGE =
    "Streaming_PackageResourceStaging_RejectsByteRangeOverflowWithoutMutation";
constexpr const char *TEST_FILE_BYTE_MISMATCH =
    "Streaming_PackageResourceStaging_ReportsFileByteCountMismatch";
constexpr const char *TEST_MISSING_COMPLETION =
    "Streaming_PackageResourceStaging_ReportsMissingFileCompletion";
constexpr const char *TEST_QUEUE_OVERFLOW =
    "Streaming_PackageResourceStaging_RejectsQueueOverflowWithoutMutation";
constexpr const char *TEST_COMPLETION_OVERFLOW =
    "Streaming_PackageResourceStaging_ReportsCompletionOverflowWithoutDroppingPending";
constexpr const char *TEST_DUPLICATE_REQUEST =
    "Streaming_PackageResourceStaging_RejectsDuplicateRequestId";
constexpr const char *TEST_SNAPSHOT =
    "Streaming_PackageResourceStaging_SnapshotReportsBoundedCounters";
constexpr const char *TEST_NO_UPPER_DEPENDENCY =
    "Streaming_PackageResourceStaging_NoUpperRuntimeDependency";
constexpr const char *TEST_CACHE_PAYLOAD_BRIDGE_STORE =
    "Streaming_ResourceCachePayloadBridge_StoresPackageWindowWithU64LogicalTotal";
constexpr const char *TEST_CACHE_PAYLOAD_BRIDGE_OFFSET =
    "Streaming_ResourceCachePayloadBridge_SeparatesArchiveAndPayloadOffsets";
constexpr const char *TEST_CACHE_PAYLOAD_BRIDGE_FAILED_STAGING =
    "Streaming_ResourceCachePayloadBridge_RejectsFailedStagingWithoutResourceMutation";
constexpr const char *TEST_CACHE_PAYLOAD_BRIDGE_WINDOW_OUT_OF_BOUNDS =
    "Streaming_ResourceCachePayloadBridge_RejectsPayloadWindowOutOfBoundsBeforeStore";
constexpr const char *TEST_CACHE_PAYLOAD_BRIDGE_BYTE_MISMATCH =
    "Streaming_ResourceCachePayloadBridge_RejectsLocalByteCountMismatchBeforeStore";
constexpr const char *TEST_UPLOAD_CREATE_BUFFER =
    "Streaming_ResourceUpload_CreateBufferFromStagingCompletion";
constexpr const char *TEST_UPLOAD_UPDATE_BUFFER =
    "Streaming_ResourceUpload_UpdateBufferSignalsFence";
constexpr const char *TEST_UPLOAD_UPDATE_BUFFER_DESTINATION_RANGE =
    "Streaming_ResourceUpload_UpdateBufferUsesDestinationOffsetAndRejectsOverflow";
constexpr const char *TEST_UPLOAD_CREATE_TEXTURE =
    "Streaming_ResourceUpload_CreateTextureFromStagingCompletion";
constexpr const char *TEST_UPLOAD_UPDATE_TEXTURE =
    "Streaming_ResourceUpload_UpdateTextureSignalsFence";
constexpr const char *TEST_UPLOAD_UPDATE_TEXTURE_DESTINATION_RANGE =
    "Streaming_ResourceUpload_UpdateTextureUsesDestinationOffsetAndRejectsOverflow";
constexpr const char *TEST_UPLOAD_INVALID_HANDLE =
    "Streaming_ResourceUpload_RejectsInvalidResourceHandleWithoutMutation";
constexpr const char *TEST_UPLOAD_TYPE_MISMATCH =
    "Streaming_ResourceUpload_RejectsResourceTypeMismatchWithoutMutation";
constexpr const char *TEST_UPLOAD_FAILED_STAGING =
    "Streaming_ResourceUpload_RejectsFailedStagingCompletion";
constexpr const char *TEST_UPLOAD_EMPTY_BYTES =
    "Streaming_ResourceUpload_RejectsEmptyUploadBytes";
constexpr const char *TEST_UPLOAD_BYTE_RANGE =
    "Streaming_ResourceUpload_RejectsByteRangeOverflow";
constexpr const char *TEST_UPLOAD_NULL_DEVICE =
    "Streaming_ResourceUpload_RejectsNullRhiDevice";
constexpr const char *TEST_UPLOAD_NULL_OUTPUT =
    "Streaming_ResourceUpload_RejectsNullOutputStorage";
constexpr const char *TEST_UPLOAD_UNSUPPORTED_KIND =
    "Streaming_ResourceUpload_RejectsUnsupportedUploadKind";
constexpr const char *TEST_UPLOAD_QUEUE_OVERFLOW =
    "Streaming_ResourceUpload_RejectsQueueOverflowWithoutMutation";
constexpr const char *TEST_UPLOAD_COMPLETION_OVERFLOW =
    "Streaming_ResourceUpload_ReportsCompletionOverflowWithoutProcessingPending";
constexpr const char *TEST_UPLOAD_DUPLICATE_ID =
    "Streaming_ResourceUpload_RejectsDuplicateUploadId";
constexpr const char *TEST_UPLOAD_SNAPSHOT =
    "Streaming_ResourceUpload_SnapshotReportsBoundedCounters";
constexpr const char *TEST_UPLOAD_RHI_FAILURE =
    "Streaming_ResourceUpload_ReportsRhiFailureWithoutWritingOutput";
constexpr const char *TEST_UPLOAD_COMMIT_SUCCESS =
    "Streaming_ResourceUploadCommit_CommitsSuccessfulUpload";
constexpr const char *TEST_UPLOAD_COMMIT_FAILED_UPLOAD =
    "Streaming_ResourceUploadCommit_CommitsFailedUpload";
constexpr const char *TEST_UPLOAD_COMMIT_INVALID_HANDLE =
    "Streaming_ResourceUploadCommit_RejectsInvalidResourceHandleWithoutMutation";
constexpr const char *TEST_UPLOAD_COMMIT_TYPE_MISMATCH =
    "Streaming_ResourceUploadCommit_RejectsTypeMismatchWithoutMutation";
constexpr const char *TEST_UPLOAD_COMMIT_DUPLICATE_ID =
    "Streaming_ResourceUploadCommit_RejectsDuplicateCommitId";
constexpr const char *TEST_UPLOAD_COMMIT_QUEUE_OVERFLOW =
    "Streaming_ResourceUploadCommit_RejectsQueueOverflowWithoutMutation";
constexpr const char *TEST_UPLOAD_COMMIT_COMPLETION_OVERFLOW =
    "Streaming_ResourceUploadCommit_ReportsCompletionOverflowWithoutProcessingPending";
constexpr const char *TEST_UPLOAD_COMMIT_SLOT_REUSE_ORDER =
    "Streaming_ResourceUploadCommit_PreservesOldestOrderAfterSlotReuse";
constexpr const char *TEST_UPLOAD_COMMIT_SNAPSHOT =
    "Streaming_ResourceUploadCommit_SnapshotReportsBoundedCounters";
constexpr const char *TEST_PIPELINE_FIXTURE_BUFFER =
    "Streaming_ResourceStreamingPipeline_FixtureBufferReadUploadCommit";
constexpr const char *TEST_PIPELINE_UPDATE_BUFFER_DESTINATION_RANGE =
    "Streaming_ResourceStreamingPipeline_UpdateBufferUsesPayloadWindowDestinationOffset";
constexpr const char *TEST_PIPELINE_CACHE_PAYLOAD =
    "Streaming_ResourceStreamingPipeline_CachePayloadOnlyStoresPackageWindow";
constexpr const char *TEST_PIPELINE_CACHE_PAYLOAD_STORE_FAILURE =
    "Streaming_ResourceStreamingPipeline_CachePayloadRejectsStoreFailureWithoutMutation";
constexpr const char *TEST_PIPELINE_PACKAGE_ARTIFACT_CACHE_PAYLOAD =
    "Streaming_ResourceStreamingPipeline_PackageArtifactFeedsCachePayloadU64Window";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr const char *PRIMARY_MOUNT = "Primary";
constexpr const char *PACKAGE_ARTIFACT_MOUNT = "PackageArtifact";
constexpr const char *PACKAGE_ARTIFACT_PATH = "PackageToStreamingArtifact.txt";
constexpr const char *FIXTURE_PATH = "Nested/FixtureFile.txt";
constexpr const char *PACKAGE_SOURCE_KEY = "fixtures/fixture_file.txt";
constexpr const char *FIXTURE_TEXT = "yuengine file fixture\n";
constexpr const char *RESOURCE_KEY = "texture_fixture";
constexpr const char *RESOURCE_KEY_ALT = "texture_fixture_alt";
constexpr const char *RESOURCE_KEY_THIRD = "texture_fixture_third";
constexpr PackageId PACKAGE_A{1U};
constexpr PackageEntryId ENTRY_TEXTURE{1U};
constexpr ResourceTypeId TYPE_TEXTURE{1U};
constexpr ResourceTypeId TYPE_AUDIO{2U};
constexpr std::uint64_t REQUEST_ONE = 1U;
constexpr std::uint64_t REQUEST_TWO = 2U;
constexpr std::uint64_t REQUEST_THREE = 3U;
constexpr std::uint64_t ARCHIVE_WINDOW_OFFSET = 3ULL;
constexpr std::uint64_t ARCHIVE_WINDOW_SIZE = 8ULL;
constexpr std::uint64_t UPLOAD_ONE = 101U;
constexpr std::uint64_t UPLOAD_TWO = 102U;
constexpr std::uint64_t UPLOAD_THREE = 103U;
constexpr std::uint64_t COMMIT_ONE = 201U;
constexpr std::uint64_t COMMIT_TWO = 202U;
constexpr std::uint64_t COMMIT_THREE = 203U;
constexpr std::uint64_t CACHE_PAYLOAD_ONE = 301U;
constexpr std::uint64_t U64_PAYLOAD_LOGICAL_BYTE_COUNT = 0x100001000ULL;
constexpr std::uint64_t PAYLOAD_WINDOW_OFFSET = 0x100000040ULL;
constexpr std::uint32_t OUTPUT_CAPACITY = 64U;
using TestFunction = int (*)();

std::filesystem::path FixtureRoot() {
    return std::filesystem::path(YUENGINE_FILE_FIXTURE_ROOT);
}

int Fail(const std::string &message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

std::uint32_t FixtureByteCount() {
    return static_cast<std::uint32_t>(std::string_view(FIXTURE_TEXT).size());
}

std::string ArchiveWindowText(std::uint64_t byte_offset, std::uint64_t byte_size) {
    const std::size_t text_offset = static_cast<std::size_t>(byte_offset);
    const std::size_t text_size = static_cast<std::size_t>(byte_size);
    const std::string_view fixture_text(FIXTURE_TEXT);
    return std::string(fixture_text.substr(text_offset, text_size));
}

MountTable CreateMountedTable() {
    MountTable table;
    table.RegisterLooseMount(MountId(PRIMARY_MOUNT), FixtureRoot() / "Primary");
    return table;
}

std::filesystem::path StreamingPackageArtifactRoot(std::string_view test_name) {
    std::filesystem::path root = std::filesystem::temp_directory_path();
    root /= "YuEngineStreamingPackageArtifactTests";
    root /= std::string(test_name);
    return root;
}

bool AddPackageArtifactMount(MountTable *table, std::string_view test_name) {
    if (table == nullptr) {
        return false;
    }

    const std::filesystem::path root = StreamingPackageArtifactRoot(test_name);
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    const FileStatus mount_status = table->RegisterLooseMount(MountId(PACKAGE_ARTIFACT_MOUNT), root);
    return mount_status == FileStatus::Success;
}

bool BuildPackageRecord(PackageLoadPlanRecord *output_record, std::uint32_t byte_offset, std::uint32_t byte_size) {
    if (output_record == nullptr) {
        return false;
    }

    PackageRegistry registry;
    const PackageRegistrationResult manifest_result = registry.RegisterSyntheticManifest({PACKAGE_A});
    if (!manifest_result.Succeeded()) {
        return false;
    }

    const PackageEntryDescriptor descriptor{
        PACKAGE_A,
        ENTRY_TEXTURE,
        TYPE_TEXTURE,
        ResourceLogicalKey(RESOURCE_KEY),
        PackageSourceKey(PACKAGE_SOURCE_KEY),
        byte_offset,
        byte_size};
    const PackageRegistrationResult entry_result = registry.RegisterEntry(descriptor);
    if (!entry_result.Succeeded()) {
        return false;
    }

    const PackageLoadPlanResult load_plan =
        registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey(RESOURCE_KEY));
    if (!load_plan.Succeeded()) {
        return false;
    }

    if (load_plan.plan.record_count != 1U) {
        return false;
    }

    *output_record = load_plan.plan.records[0U];
    return true;
}

bool BuildPackageArtifactRecord(
    MountTable *table,
    PackageLoadPlanRecord *output_record,
    std::string_view test_name) {
    if (table == nullptr) {
        return false;
    }

    if (output_record == nullptr) {
        return false;
    }

    if (!AddPackageArtifactMount(table, test_name)) {
        return false;
    }

    const std::uint32_t archive_window_offset = static_cast<std::uint32_t>(ARCHIVE_WINDOW_OFFSET);
    const std::uint32_t archive_window_size = static_cast<std::uint32_t>(ARCHIVE_WINDOW_SIZE);
    PackageEntryDescriptor descriptor{
        PACKAGE_A,
        ENTRY_TEXTURE,
        TYPE_TEXTURE,
        ResourceLogicalKey(RESOURCE_KEY),
        PackageSourceKey(PACKAGE_SOURCE_KEY),
        archive_window_offset,
        archive_window_size};
    descriptor.payload_logical_byte_count = U64_PAYLOAD_LOGICAL_BYTE_COUNT;
    descriptor.payload_window_byte_offset = PAYLOAD_WINDOW_OFFSET;
    descriptor.payload_window_byte_size = ARCHIVE_WINDOW_SIZE;
    const std::array<PackageEntryDescriptor, 1U> entries{descriptor};

    PackageArtifactWriteRequest write_request{};
    write_request.mount_table = table;
    write_request.mount = MountId(PACKAGE_ARTIFACT_MOUNT);
    write_request.artifact_path = VirtualPath(PACKAGE_ARTIFACT_PATH);
    write_request.package = PACKAGE_A;
    write_request.entries = entries.data();
    write_request.entry_count = static_cast<std::uint32_t>(entries.size());
    const PackageArtifactResult write_result = WritePackageArtifact(write_request);
    if (write_result.status != PackageStatus::Success) {
        return false;
    }

    if (!write_result.wrote_artifact) {
        return false;
    }

    PackageRegistry artifact_registry;
    PackageArtifactReadRequest read_request{};
    read_request.mount_table = table;
    read_request.mount = MountId(PACKAGE_ARTIFACT_MOUNT);
    read_request.artifact_path = VirtualPath(PACKAGE_ARTIFACT_PATH);
    read_request.registry = &artifact_registry;
    const PackageArtifactResult read_result = ReadPackageArtifact(read_request);
    if (read_result.status != PackageStatus::Success) {
        return false;
    }

    if (!read_result.rebuilt_registry) {
        return false;
    }

    const PackageLoadPlanResult load_plan =
        artifact_registry.ResolveEntryByResourceKey(PACKAGE_A, TYPE_TEXTURE, ResourceLogicalKey(RESOURCE_KEY));
    if (!load_plan.Succeeded()) {
        return false;
    }

    if (load_plan.plan.record_count != 1U) {
        return false;
    }

    *output_record = load_plan.plan.records[0U];
    return true;
}

ResourceRegistrationResult RegisterResource(ResourceRegistry &registry, ResourceTypeId type = TYPE_TEXTURE) {
    const ResourceDescriptor descriptor{type, ResourceLogicalKey(RESOURCE_KEY), 0U};
    return registry.RegisterSyntheticDescriptor(descriptor);
}

ResourceRegistrationResult RegisterResourceWithKey(
    ResourceRegistry &registry,
    ResourceTypeId type,
    const char *logical_key) {
    const ResourceDescriptor descriptor{type, ResourceLogicalKey(logical_key), 0U};
    return registry.RegisterSyntheticDescriptor(descriptor);
}

AsyncFileReadRequest BuildFileRequest(
    MountTable &table,
    std::uint8_t *output_bytes,
    std::size_t output_capacity) {
    AsyncFileReadRequest file_request;
    file_request.mount_table = &table;
    file_request.read_request = {MountId(PRIMARY_MOUNT), VirtualPath(FIXTURE_PATH)};
    file_request.output_bytes = output_bytes;
    file_request.output_capacity = output_capacity;
    return file_request;
}

PackageResourceStagingRequest BuildStagingRequest(
    const ResourceRegistry &resource_registry,
    AsyncFileReadQueue &file_queue,
    MountTable &table,
    const PackageLoadPlanRecord &record,
    ResourceHandle resource,
    std::uint64_t request_id,
    std::uint8_t *output_bytes,
    std::size_t output_capacity) {
    PackageResourceStagingRequest request;
    request.resource_registry = &resource_registry;
    request.file_queue = &file_queue;
    request.package_record = record;
    request.resource = resource;
    request.expected_type = record.type;
    request.file_request = BuildFileRequest(table, output_bytes, output_capacity);
    request.request_id = request_id;
    return request;
}

PackageResourceStagingRequest BuildValidRequest(
    ResourceRegistry &resource_registry,
    AsyncFileReadQueue &file_queue,
    MountTable &table,
    std::uint64_t request_id,
    std::array<std::uint8_t, OUTPUT_CAPACITY> &output_bytes) {
    PackageLoadPlanRecord record;
    BuildPackageRecord(&record, 0U, FixtureByteCount());
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    return BuildStagingRequest(
        resource_registry,
        file_queue,
        table,
        record,
        resource_result.handle,
        request_id,
        output_bytes.data(),
        output_bytes.size());
}

AsyncFileReadResult SuccessFileResult(std::uint64_t request_id) {
    AsyncFileReadResult result;
    result.status = AsyncFileReadStatus::Success;
    result.file_status = FileStatus::Success;
    result.request_index = request_id;
    result.byte_count = FixtureByteCount();
    return result;
}

bool DrainOneFileCompletion(AsyncFileReadQueue &file_queue, AsyncFileReadResult *output_result) {
    if (output_result == nullptr) {
        return false;
    }

    std::array<AsyncFileReadResult, 1U> file_results{};
    std::size_t written_count = 0U;
    const AsyncFileReadStatus drain_status = file_queue.DrainCompletions(
        file_results.data(),
        file_results.size(),
        &written_count);
    if (drain_status != AsyncFileReadStatus::Success) {
        return false;
    }

    if (written_count != 1U) {
        return false;
    }

    *output_result = file_results[0U];
    return true;
}

bool DrainOneStagingCompletion(PackageResourceStagingQueue &queue, PackageResourceStagingCompletion *completion) {
    if (completion == nullptr) {
        return false;
    }

    std::array<PackageResourceStagingCompletion, 1U> completions{};
    std::uint32_t written_count = 0U;
    const PackageResourceStagingStatus drain_status = queue.DrainCompletions(
        completions.data(),
        static_cast<std::uint32_t>(completions.size()),
        &written_count);
    if (drain_status != PackageResourceStagingStatus::Success) {
        return false;
    }

    if (written_count != 1U) {
        return false;
    }

    *completion = completions[0U];
    return true;
}

NullRhiDevice CreateInitializedUploadDevice() {
    NullRhiDevice device;
    device.Initialize(RhiDeviceDesc{});
    return device;
}

RhiBufferDesc UploadBufferDesc(std::size_t size_bytes) {
    RhiBufferDesc desc;
    desc.usage = RhiBufferUsage::Vertex;
    desc.size_bytes = size_bytes;
    return desc;
}

RhiTextureDesc UploadTextureDesc() {
    RhiTextureDesc desc;
    desc.extent.width = 2U;
    desc.extent.height = 2U;
    return desc;
}

std::array<std::uint8_t, 4U> BufferUploadBytes() {
    return std::array<std::uint8_t, 4U>{1U, 2U, 3U, 4U};
}

std::array<std::uint8_t, 16U> TextureUploadBytes() {
    return std::array<std::uint8_t, 16U>{
        1U, 2U, 3U, 4U,
        5U, 6U, 7U, 8U,
        9U, 10U, 11U, 12U,
        13U, 14U, 15U, 16U};
}

PackageResourceStagingCompletion BuildSuccessfulStagingCompletion(
    ResourceHandle resource,
    ResourceTypeId type,
    std::uint64_t request_id,
    std::uint32_t byte_count) {
    PackageResourceStagingCompletion completion;
    completion.status = PackageResourceStagingStatus::Success;
    completion.resource = resource;
    completion.expected_type = type;
    completion.request_id = request_id;
    completion.file_byte_count = byte_count;
    completion.staged_byte_offset = 0U;
    completion.staged_byte_count = byte_count;
    return completion;
}

ResourceUploadRequest BuildBaseUploadRequest(
    const ResourceRegistry &resource_registry,
    NullRhiDevice *device,
    const PackageResourceStagingCompletion &completion,
    std::span<const std::uint8_t> bytes,
    std::uint64_t upload_id) {
    ResourceUploadRequest request;
    request.resource_registry = &resource_registry;
    request.rhi_device = device;
    request.staging_completion = completion;
    request.resource = completion.resource;
    request.expected_type = completion.expected_type;
    request.staged_bytes = bytes;
    request.upload_byte_count = static_cast<std::uint32_t>(bytes.size());
    request.upload_id = upload_id;
    return request;
}

bool DrainOneUploadCompletion(ResourceUploadQueue &queue, ResourceUploadCompletion *completion) {
    if (completion == nullptr) {
        return false;
    }

    std::array<ResourceUploadCompletion, 1U> completions{};
    std::uint32_t written_count = 0U;
    const ResourceUploadStatus drain_status = queue.DrainCompletions(
        completions.data(),
        static_cast<std::uint32_t>(completions.size()),
        &written_count);
    if (drain_status != ResourceUploadStatus::Success) {
        return false;
    }

    if (written_count != 1U) {
        return false;
    }

    *completion = completions[0U];
    return true;
}

ResourceUploadCompletion BuildUploadCompletion(
    ResourceHandle resource,
    ResourceTypeId type,
    ResourceUploadStatus status,
    std::uint64_t upload_id,
    std::uint64_t staging_request_id,
    std::uint32_t byte_count) {
    ResourceUploadCompletion completion;
    completion.status = status;
    completion.upload_kind = ResourceUploadKind::CreateBuffer;
    completion.resource = resource;
    completion.expected_type = type;
    completion.upload_id = upload_id;
    completion.staging_request_id = staging_request_id;
    completion.upload_byte_count = byte_count;
    if (status == ResourceUploadStatus::RhiUploadFailed) {
        completion.rhi_status = RhiStatus::InvalidDescriptor;
    }

    return completion;
}

ResourceUploadCommitRequest BuildUploadCommitRequest(
    ResourceRegistry &resource_registry,
    const ResourceUploadCompletion &upload_completion,
    std::uint64_t commit_id) {
    ResourceUploadCommitRequest request;
    request.resource_registry = &resource_registry;
    request.upload_completion = upload_completion;
    request.commit_id = commit_id;
    return request;
}

bool DrainOneUploadCommitCompletion(
    ResourceUploadCommitQueue &queue,
    ResourceUploadCommitCompletion *completion) {
    if (completion == nullptr) {
        return false;
    }

    std::array<ResourceUploadCommitCompletion, 1U> completions{};
    std::uint32_t written_count = 0U;
    const ResourceUploadCommitStatus drain_status = queue.DrainCompletions(
        completions.data(),
        static_cast<std::uint32_t>(completions.size()),
        &written_count);
    if (drain_status != ResourceUploadCommitStatus::Success) {
        return false;
    }

    if (written_count != 1U) {
        return false;
    }

    *completion = completions[0U];
    return true;
}

bool ResourceLoadStateMatches(
    const ResourceRegistry &registry,
    ResourceHandle resource,
    ResourceTypeId expected_type,
    ResourceLoadState expected_state) {
    ResourceLoadState load_state = ResourceLoadState::Unloaded;
    const ResourceLoadCommitStatus status = registry.GetLoadState(resource, expected_type, &load_state);
    if (status != ResourceLoadCommitStatus::Success) {
        return false;
    }

    return load_state == expected_state;
}

ResourceLoadCommitRequest BuildResourceLoadCommitRequest(
    ResourceHandle resource,
    ResourceTypeId expected_type,
    std::uint32_t loaded_byte_count) {
    ResourceLoadCommitRequest request;
    request.resource = resource;
    request.expected_type = expected_type;
    request.load_state = ResourceLoadState::Uploaded;
    request.commit_id = COMMIT_ONE;
    request.upload_id = UPLOAD_ONE;
    request.staging_request_id = REQUEST_ONE;
    request.upload_byte_count = loaded_byte_count;
    return request;
}

bool AdmitCachePayloadResource(
    ResourceRegistry &registry,
    ResourceHandle resource,
    ResourceTypeId expected_type,
    std::uint32_t loaded_byte_count) {
    ResourceResidencyBudgetDesc budget;
    budget.byte_capacity = OUTPUT_CAPACITY;
    if (registry.SetResidencyBudget(budget) != ResourceResidencyStatus::Success) {
        return false;
    }

    const ResourceLoadCommitRequest commit_request =
        BuildResourceLoadCommitRequest(resource, expected_type, loaded_byte_count);
    if (registry.CommitUploadCompletion(commit_request) != ResourceLoadCommitStatus::Success) {
        return false;
    }

    ResourceResidencyRequest residency_request;
    residency_request.resource = resource;
    residency_request.expected_type = expected_type;
    return registry.AdmitResident(residency_request) == ResourceResidencyStatus::Success;
}

ResourceCachePayloadBridgeRequest BuildCachePayloadBridgeRequest(
    ResourceRegistry &registry,
    const PackageResourceStagingCompletion &completion,
    std::span<const std::uint8_t> staged_bytes) {
    ResourceCachePayloadBridgeRequest request;
    request.resource_registry = &registry;
    request.staging_completion = completion;
    request.staged_bytes = staged_bytes;
    request.payload_id = CACHE_PAYLOAD_ONE;
    request.payload_logical_byte_count = U64_PAYLOAD_LOGICAL_BYTE_COUNT;
    request.payload_window_byte_offset = PAYLOAD_WINDOW_OFFSET;
    request.payload_window_byte_size = completion.staged_byte_count;
    return request;
}

ResourceCachePayloadRequest BuildCachePayloadReadRequest(
    ResourceHandle resource,
    ResourceTypeId expected_type,
    std::uint64_t payload_id,
    std::uint64_t logical_byte_count,
    std::uint64_t window_byte_offset,
    std::uint64_t window_byte_size) {
    ResourceCachePayloadRequest request;
    request.resource = resource;
    request.expected_type = expected_type;
    request.payload_id = payload_id;
    request.payload_logical_byte_count = logical_byte_count;
    request.payload_window_byte_offset = window_byte_offset;
    request.payload_window_byte_size = window_byte_size;
    return request;
}

int StreamingResourceCachePayloadBridgeStoresPackageWindowWithU64LogicalTotal() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("cache payload bridge resource registration failed");
    }

    const std::array<std::uint8_t, 4U> payload_bytes = BufferUploadBytes();
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload_bytes.size());
    if (!AdmitCachePayloadResource(resource_registry, resource_result.handle, TYPE_TEXTURE, payload_byte_count)) {
        return Fail("cache payload bridge resource admission failed");
    }

    const PackageResourceStagingCompletion completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, payload_byte_count);
    ResourceCachePayloadBridge bridge;
    const std::span<const std::uint8_t> staged_bytes(payload_bytes.data(), payload_bytes.size());
    const ResourceCachePayloadBridgeRequest request =
        BuildCachePayloadBridgeRequest(resource_registry, completion, staged_bytes);
    const ResourceCachePayloadBridgeResult result = bridge.StorePayload(request);
    if (result.status != ResourceCachePayloadBridgeStatus::Success) {
        return Fail("cache payload bridge store did not succeed");
    }

    if (result.cache_payload_status != ResourceCachePayloadStatus::Success) {
        return Fail("cache payload bridge resource store status changed");
    }

    if (result.payload_logical_byte_count != U64_PAYLOAD_LOGICAL_BYTE_COUNT) {
        return Fail("cache payload bridge result logical byte count changed");
    }

    if (result.payload_window_byte_offset != PAYLOAD_WINDOW_OFFSET) {
        return Fail("cache payload bridge result window offset changed");
    }

    if (result.payload_byte_count != payload_byte_count) {
        return Fail("cache payload bridge result local byte count changed");
    }

    const ResourceCachePayloadBridgeSnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.submitted_count != 1U) {
        return Fail("cache payload bridge submitted count changed");
    }

    if (bridge_snapshot.completed_count != 1U) {
        return Fail("cache payload bridge completed count changed");
    }

    const ResourceCachePayloadSnapshot cache_snapshot = resource_registry.CachePayloadSnapshot();
    if (cache_snapshot.cached_payload_count != 1U) {
        return Fail("cache payload bridge did not store resource cache payload");
    }

    if (cache_snapshot.last_payload_logical_byte_count != U64_PAYLOAD_LOGICAL_BYTE_COUNT) {
        return Fail("cache payload bridge snapshot logical byte count changed");
    }

    if (cache_snapshot.last_payload_window_byte_offset != PAYLOAD_WINDOW_OFFSET) {
        return Fail("cache payload bridge snapshot window offset changed");
    }

    std::array<std::uint8_t, 2U> output_bytes{};
    std::uint32_t output_byte_count = 0U;
    const std::uint64_t read_window_offset = PAYLOAD_WINDOW_OFFSET + 1U;
    const ResourceCachePayloadRequest read_request = BuildCachePayloadReadRequest(
        resource_result.handle,
        TYPE_TEXTURE,
        CACHE_PAYLOAD_ONE,
        U64_PAYLOAD_LOGICAL_BYTE_COUNT,
        read_window_offset,
        static_cast<std::uint64_t>(output_bytes.size()));
    const ResourceCachePayloadStatus read_status = resource_registry.ReadCachePayload(
        read_request,
        output_bytes.data(),
        static_cast<std::uint32_t>(output_bytes.size()),
        &output_byte_count);
    if (read_status != ResourceCachePayloadStatus::Success) {
        return Fail("cache payload bridge stored window could not be read");
    }

    if (output_byte_count != static_cast<std::uint32_t>(output_bytes.size())) {
        return Fail("cache payload bridge read byte count changed");
    }

    if (output_bytes[0U] != payload_bytes[1U]) {
        return Fail("cache payload bridge read first byte changed");
    }

    if (output_bytes[1U] != payload_bytes[2U]) {
        return Fail("cache payload bridge read second byte changed");
    }

    return 0;
}

int StreamingResourceCachePayloadBridgeSeparatesArchiveAndPayloadOffsets() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("cache payload bridge offset resource registration failed");
    }

    const std::array<std::uint8_t, 4U> payload_bytes = BufferUploadBytes();
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload_bytes.size());
    if (!AdmitCachePayloadResource(resource_registry, resource_result.handle, TYPE_TEXTURE, payload_byte_count)) {
        return Fail("cache payload bridge offset resource admission failed");
    }

    PackageLoadPlanRecord package_record;
    if (!BuildPackageRecord(&package_record, static_cast<std::uint32_t>(ARCHIVE_WINDOW_OFFSET), payload_byte_count)) {
        return Fail("cache payload bridge offset package record build failed");
    }

    PackageResourceStagingCompletion completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, payload_byte_count);
    completion.package_record = package_record;

    ResourceCachePayloadBridge bridge;
    const std::span<const std::uint8_t> staged_bytes(payload_bytes.data(), payload_bytes.size());
    const ResourceCachePayloadBridgeRequest request =
        BuildCachePayloadBridgeRequest(resource_registry, completion, staged_bytes);
    const ResourceCachePayloadBridgeResult result = bridge.StorePayload(request);
    if (result.status != ResourceCachePayloadBridgeStatus::Success) {
        return Fail("cache payload bridge offset store did not succeed");
    }

    std::array<std::uint8_t, 2U> output_bytes{};
    std::uint32_t output_byte_count = 0U;
    const ResourceCachePayloadRequest archive_offset_request = BuildCachePayloadReadRequest(
        resource_result.handle,
        TYPE_TEXTURE,
        CACHE_PAYLOAD_ONE,
        U64_PAYLOAD_LOGICAL_BYTE_COUNT,
        ARCHIVE_WINDOW_OFFSET,
        static_cast<std::uint64_t>(output_bytes.size()));
    const ResourceCachePayloadStatus read_status = resource_registry.ReadCachePayload(
        archive_offset_request,
        output_bytes.data(),
        static_cast<std::uint32_t>(output_bytes.size()),
        &output_byte_count);
    if (read_status != ResourceCachePayloadStatus::PayloadWindowOutOfBounds) {
        return Fail("cache payload bridge treated archive offset as payload offset");
    }

    return 0;
}

int StreamingResourceCachePayloadBridgeRejectsFailedStagingWithoutResourceMutation() {
    ResourceRegistry resource_registry;
    const std::array<std::uint8_t, 4U> payload_bytes = BufferUploadBytes();
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload_bytes.size());
    PackageResourceStagingCompletion completion =
        BuildSuccessfulStagingCompletion(ResourceHandle{}, TYPE_TEXTURE, REQUEST_ONE, payload_byte_count);
    completion.status = PackageResourceStagingStatus::FileReadFailed;

    ResourceCachePayloadBridge bridge;
    const std::span<const std::uint8_t> staged_bytes(payload_bytes.data(), payload_bytes.size());
    const ResourceCachePayloadBridgeRequest request =
        BuildCachePayloadBridgeRequest(resource_registry, completion, staged_bytes);
    const ResourceCachePayloadBridgeResult result = bridge.StorePayload(request);
    if (result.status != ResourceCachePayloadBridgeStatus::StagingFailed) {
        return Fail("cache payload bridge did not reject failed staging");
    }

    const ResourceCachePayloadBridgeSnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.submitted_count != 0U) {
        return Fail("cache payload bridge submitted failed staging");
    }

    if (bridge_snapshot.rejected_count != 1U) {
        return Fail("cache payload bridge rejected count changed for failed staging");
    }

    const ResourceCachePayloadSnapshot cache_snapshot = resource_registry.CachePayloadSnapshot();
    if (cache_snapshot.cached_payload_count != 0U) {
        return Fail("cache payload bridge mutated resource cache for failed staging");
    }

    return 0;
}

int StreamingResourceCachePayloadBridgeRejectsPayloadWindowOutOfBoundsBeforeStore() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("cache payload bridge bounds resource registration failed");
    }

    const std::array<std::uint8_t, 4U> payload_bytes = BufferUploadBytes();
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload_bytes.size());
    if (!AdmitCachePayloadResource(resource_registry, resource_result.handle, TYPE_TEXTURE, payload_byte_count)) {
        return Fail("cache payload bridge bounds resource admission failed");
    }

    const PackageResourceStagingCompletion completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, payload_byte_count);
    ResourceCachePayloadBridge bridge;
    const std::span<const std::uint8_t> staged_bytes(payload_bytes.data(), payload_bytes.size());
    ResourceCachePayloadBridgeRequest request =
        BuildCachePayloadBridgeRequest(resource_registry, completion, staged_bytes);
    const std::uint64_t payload_window_end = PAYLOAD_WINDOW_OFFSET + payload_byte_count;
    request.payload_logical_byte_count = payload_window_end - 1U;
    const ResourceCachePayloadBridgeResult result = bridge.StorePayload(request);
    if (result.status != ResourceCachePayloadBridgeStatus::PayloadWindowOutOfBounds) {
        return Fail("cache payload bridge did not reject payload window bounds");
    }

    const ResourceCachePayloadBridgeSnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.submitted_count != 0U) {
        return Fail("cache payload bridge submitted bounds failure");
    }

    const ResourceCachePayloadSnapshot cache_snapshot = resource_registry.CachePayloadSnapshot();
    if (cache_snapshot.cached_payload_count != 0U) {
        return Fail("cache payload bridge mutated resource cache for bounds failure");
    }

    return 0;
}

int StreamingResourceCachePayloadBridgeRejectsLocalByteCountMismatchBeforeStore() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("cache payload bridge mismatch resource registration failed");
    }

    const std::array<std::uint8_t, 4U> payload_bytes = BufferUploadBytes();
    const std::uint32_t payload_byte_count = static_cast<std::uint32_t>(payload_bytes.size());
    if (!AdmitCachePayloadResource(resource_registry, resource_result.handle, TYPE_TEXTURE, payload_byte_count)) {
        return Fail("cache payload bridge mismatch resource admission failed");
    }

    const PackageResourceStagingCompletion completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, payload_byte_count);
    ResourceCachePayloadBridge bridge;
    const std::span<const std::uint8_t> staged_bytes(payload_bytes.data(), payload_bytes.size());
    ResourceCachePayloadBridgeRequest request =
        BuildCachePayloadBridgeRequest(resource_registry, completion, staged_bytes);
    request.payload_window_byte_size = static_cast<std::uint64_t>(payload_byte_count) + 1U;
    const ResourceCachePayloadBridgeResult result = bridge.StorePayload(request);
    if (result.status != ResourceCachePayloadBridgeStatus::StagingByteCountMismatch) {
        return Fail("cache payload bridge did not reject local byte mismatch");
    }

    const ResourceCachePayloadBridgeSnapshot bridge_snapshot = bridge.Snapshot();
    if (bridge_snapshot.submitted_count != 0U) {
        return Fail("cache payload bridge submitted local byte mismatch");
    }

    const ResourceCachePayloadSnapshot cache_snapshot = resource_registry.CachePayloadSnapshot();
    if (cache_snapshot.cached_payload_count != 0U) {
        return Fail("cache payload bridge mutated resource cache for local byte mismatch");
    }

    return 0;
}

int StreamingPackageResourceStagingSubmitsAndCompletesFixtureRead() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue file_queue;
    if (file_queue.Initialize(2U, 2U) != AsyncFileReadStatus::Success) {
        return Fail("file queue initialize failed");
    }

    if (file_queue.Start() != AsyncFileReadStatus::Success) {
        return Fail("file queue start failed");
    }

    ResourceRegistry resource_registry;
    std::array<std::uint8_t, OUTPUT_CAPACITY> output_bytes{};
    PackageResourceStagingRequest request =
        BuildValidRequest(resource_registry, file_queue, table, REQUEST_ONE, output_bytes);
    PackageResourceStagingQueue queue(PackageResourceStagingQueueDesc{2U, 2U});
    if (queue.Submit(request) != PackageResourceStagingStatus::Queued) {
        return Fail("staging queue did not accept request");
    }

    if (file_queue.Shutdown(false) != AsyncFileReadStatus::ShutdownComplete) {
        return Fail("file queue shutdown failed");
    }

    AsyncFileReadResult file_result;
    if (!DrainOneFileCompletion(file_queue, &file_result)) {
        return Fail("file completion drain failed");
    }

    if (queue.CompleteFileRead(file_result) != PackageResourceStagingStatus::Success) {
        return Fail("staging completion did not succeed");
    }

    PackageResourceStagingCompletion completion;
    if (!DrainOneStagingCompletion(queue, &completion)) {
        return Fail("staging completion drain failed");
    }

    if (completion.request_id != REQUEST_ONE) {
        return Fail("staging completion request id changed");
    }

    if (completion.staged_byte_count != FixtureByteCount()) {
        return Fail("staging completion byte count changed");
    }

    const std::string text(output_bytes.begin(), output_bytes.begin() + completion.file_byte_count);
    if (text != FIXTURE_TEXT) {
        return Fail("staging output bytes did not match fixture");
    }

    const PackageResourceStagingSnapshot snapshot = queue.Snapshot();
    if (snapshot.submitted_count != 1U) {
        return Fail("staging snapshot submitted count changed");
    }

    if (snapshot.completed_count != 1U) {
        return Fail("staging snapshot completed count changed");
    }

    if (snapshot.pending_count != 0U) {
        return Fail("staging snapshot left pending records");
    }

    return 0;
}

int StreamingPackageResourceStagingRejectsInvalidResourceHandleWithoutMutation() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue file_queue;
    ResourceRegistry resource_registry;
    std::array<std::uint8_t, OUTPUT_CAPACITY> output_bytes{};
    PackageLoadPlanRecord record;
    BuildPackageRecord(&record, 0U, FixtureByteCount());
    PackageResourceStagingRequest request = BuildStagingRequest(
        resource_registry,
        file_queue,
        table,
        record,
        ResourceHandle{},
        REQUEST_ONE,
        output_bytes.data(),
        output_bytes.size());
    PackageResourceStagingQueue queue(PackageResourceStagingQueueDesc{1U, 1U});
    const PackageResourceStagingStatus status = queue.Submit(request);
    if (status != PackageResourceStagingStatus::ResourceValidationFailed) {
        return Fail("invalid resource handle did not return resource validation failure");
    }

    const PackageResourceStagingSnapshot snapshot = queue.Snapshot();
    if (snapshot.pending_count != 0U) {
        return Fail("invalid handle changed pending count");
    }

    if (snapshot.submitted_count != 0U) {
        return Fail("invalid handle submitted file work");
    }

    if (snapshot.last_resource_status != ResourceStatus::InvalidHandle) {
        return Fail("invalid handle did not preserve resource status");
    }

    return 0;
}

int StreamingPackageResourceStagingRejectsResourceTypeMismatchWithoutMutation() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue file_queue;
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry, TYPE_TEXTURE);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    std::array<std::uint8_t, OUTPUT_CAPACITY> output_bytes{};
    PackageLoadPlanRecord record;
    BuildPackageRecord(&record, 0U, FixtureByteCount());
    record.type = TYPE_AUDIO;
    PackageResourceStagingRequest request = BuildStagingRequest(
        resource_registry,
        file_queue,
        table,
        record,
        resource_result.handle,
        REQUEST_ONE,
        output_bytes.data(),
        output_bytes.size());
    PackageResourceStagingQueue queue(PackageResourceStagingQueueDesc{1U, 1U});
    const PackageResourceStagingStatus status = queue.Submit(request);
    if (status != PackageResourceStagingStatus::ResourceValidationFailed) {
        return Fail("type mismatch did not return resource validation failure");
    }

    const PackageResourceStagingSnapshot snapshot = queue.Snapshot();
    if (snapshot.last_resource_status != ResourceStatus::TypeMismatch) {
        return Fail("type mismatch resource status was not preserved");
    }

    if (snapshot.pending_count != 0U) {
        return Fail("type mismatch changed pending count");
    }

    return 0;
}

int StreamingPackageResourceStagingUsesArchiveByteRangeForFileRead() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue file_queue;
    if (file_queue.Initialize(2U, 2U) != AsyncFileReadStatus::Success) {
        return Fail("archive range file queue initialize failed");
    }

    if (file_queue.Start() != AsyncFileReadStatus::Success) {
        return Fail("archive range file queue start failed");
    }

    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("archive range resource registration failed");
    }

    std::array<std::uint8_t, OUTPUT_CAPACITY> output_bytes{};
    PackageLoadPlanRecord record;
    BuildPackageRecord(&record, 0U, FixtureByteCount());
    record.archive_byte_offset = ARCHIVE_WINDOW_OFFSET;
    record.archive_byte_size = ARCHIVE_WINDOW_SIZE;
    PackageResourceStagingRequest request = BuildStagingRequest(
        resource_registry,
        file_queue,
        table,
        record,
        resource_result.handle,
        REQUEST_ONE,
        output_bytes.data(),
        output_bytes.size());
    PackageResourceStagingQueue queue(PackageResourceStagingQueueDesc{1U, 1U});
    if (queue.Submit(request) != PackageResourceStagingStatus::Queued) {
        return Fail("archive range request was not queued");
    }

    if (file_queue.Shutdown(false) != AsyncFileReadStatus::ShutdownComplete) {
        return Fail("archive range file queue shutdown failed");
    }

    AsyncFileReadResult file_result;
    if (!DrainOneFileCompletion(file_queue, &file_result)) {
        return Fail("archive range file completion drain failed");
    }

    if (queue.CompleteFileRead(file_result) != PackageResourceStagingStatus::Success) {
        return Fail("archive range completion did not succeed");
    }

    PackageResourceStagingCompletion completion;
    if (!DrainOneStagingCompletion(queue, &completion)) {
        return Fail("archive range staging completion drain failed");
    }

    const std::size_t expected_file_byte_count = static_cast<std::size_t>(ARCHIVE_WINDOW_SIZE);
    if (completion.file_byte_count != expected_file_byte_count) {
        return Fail("archive range file byte count changed");
    }

    if (completion.staged_byte_offset != 0U) {
        return Fail("archive range staged byte offset was not buffer local");
    }

    if (completion.staged_byte_count != static_cast<std::uint32_t>(ARCHIVE_WINDOW_SIZE)) {
        return Fail("archive range staged byte count changed");
    }

    const std::string text(output_bytes.begin(), output_bytes.begin() + completion.file_byte_count);
    const std::string expected_text = ArchiveWindowText(ARCHIVE_WINDOW_OFFSET, ARCHIVE_WINDOW_SIZE);
    if (text != expected_text) {
        return Fail("archive range output bytes did not match fixture window");
    }

    return 0;
}

int StreamingPackageResourceStagingPreservesLegacyMirrorAsMetadataOnly() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue file_queue;
    if (file_queue.Initialize(2U, 2U) != AsyncFileReadStatus::Success) {
        return Fail("legacy mirror file queue initialize failed");
    }

    if (file_queue.Start() != AsyncFileReadStatus::Success) {
        return Fail("legacy mirror file queue start failed");
    }

    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("legacy mirror resource registration failed");
    }

    std::array<std::uint8_t, OUTPUT_CAPACITY> output_bytes{};
    PackageLoadPlanRecord record;
    BuildPackageRecord(&record, 0U, FixtureByteCount());
    record.byte_offset = std::numeric_limits<std::uint32_t>::max();
    record.byte_size = 1U;
    record.archive_byte_offset = 0ULL;
    record.archive_byte_size = FixtureByteCount();
    PackageResourceStagingRequest request = BuildStagingRequest(
        resource_registry,
        file_queue,
        table,
        record,
        resource_result.handle,
        REQUEST_ONE,
        output_bytes.data(),
        output_bytes.size());
    PackageResourceStagingQueue queue(PackageResourceStagingQueueDesc{1U, 1U});
    if (queue.Submit(request) != PackageResourceStagingStatus::Queued) {
        return Fail("legacy mirror request was not queued");
    }

    if (file_queue.Shutdown(false) != AsyncFileReadStatus::ShutdownComplete) {
        return Fail("legacy mirror file queue shutdown failed");
    }

    AsyncFileReadResult file_result;
    if (!DrainOneFileCompletion(file_queue, &file_result)) {
        return Fail("legacy mirror file completion drain failed");
    }

    if (queue.CompleteFileRead(file_result) != PackageResourceStagingStatus::Success) {
        return Fail("legacy mirror completion did not succeed");
    }

    PackageResourceStagingCompletion completion;
    if (!DrainOneStagingCompletion(queue, &completion)) {
        return Fail("legacy mirror staging completion drain failed");
    }

    if (completion.package_record.byte_offset != std::numeric_limits<std::uint32_t>::max()) {
        return Fail("legacy mirror byte offset was not preserved");
    }

    const std::uint64_t fixture_byte_count = static_cast<std::uint64_t>(FixtureByteCount());
    if (completion.package_record.archive_byte_size != fixture_byte_count) {
        return Fail("legacy mirror archive byte size changed");
    }

    if (completion.staged_byte_offset != 0U) {
        return Fail("legacy mirror staged byte offset was not buffer local");
    }

    if (completion.staged_byte_count != FixtureByteCount()) {
        return Fail("legacy mirror staged byte count changed");
    }

    const std::string text(output_bytes.begin(), output_bytes.begin() + completion.file_byte_count);
    if (text != FIXTURE_TEXT) {
        return Fail("legacy mirror output bytes did not match fixture");
    }

    return 0;
}

int StreamingPackageResourceStagingRejectsArchiveByteSizeOverOutputCapacity() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue file_queue;
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("output capacity resource registration failed");
    }

    std::array<std::uint8_t, 1U> output_bytes{};
    PackageLoadPlanRecord record;
    BuildPackageRecord(&record, 0U, FixtureByteCount());
    PackageResourceStagingRequest request = BuildStagingRequest(
        resource_registry,
        file_queue,
        table,
        record,
        resource_result.handle,
        REQUEST_ONE,
        output_bytes.data(),
        output_bytes.size());
    PackageResourceStagingQueue queue(PackageResourceStagingQueueDesc{1U, 1U});
    const PackageResourceStagingStatus status = queue.Submit(request);
    if (status != PackageResourceStagingStatus::OutputTooSmall) {
        return Fail("archive byte size over output capacity did not return explicit status");
    }

    const PackageResourceStagingSnapshot snapshot = queue.Snapshot();
    if (snapshot.pending_count != 0U) {
        return Fail("archive byte size over output capacity changed pending count");
    }

    if (snapshot.submitted_count != 0U) {
        return Fail("archive byte size over output capacity submitted file work");
    }

    return 0;
}

int StreamingPackageResourceStagingRejectsByteRangeOverflowWithoutMutation() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue file_queue;
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    std::array<std::uint8_t, OUTPUT_CAPACITY> output_bytes{};
    PackageLoadPlanRecord record;
    BuildPackageRecord(&record, 0U, FixtureByteCount());
    record.archive_byte_offset = std::numeric_limits<std::uint64_t>::max();
    record.archive_byte_size = 1ULL;
    PackageResourceStagingRequest request = BuildStagingRequest(
        resource_registry,
        file_queue,
        table,
        record,
        resource_result.handle,
        REQUEST_ONE,
        output_bytes.data(),
        output_bytes.size());
    PackageResourceStagingQueue queue(PackageResourceStagingQueueDesc{1U, 1U});
    const PackageResourceStagingStatus status = queue.Submit(request);
    if (status != PackageResourceStagingStatus::ByteRangeOutOfBounds) {
        return Fail("byte range overflow did not return explicit status");
    }

    const PackageResourceStagingSnapshot snapshot = queue.Snapshot();
    if (snapshot.pending_count != 0U) {
        return Fail("byte range overflow changed pending count");
    }

    if (snapshot.submitted_count != 0U) {
        return Fail("byte range overflow submitted file work");
    }

    return 0;
}

int StreamingPackageResourceStagingReportsFileByteCountMismatch() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue file_queue;
    file_queue.Initialize(2U, 2U);
    file_queue.Start();

    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("file byte mismatch resource registration failed");
    }

    std::array<std::uint8_t, OUTPUT_CAPACITY> output_bytes{};
    PackageLoadPlanRecord record;
    BuildPackageRecord(&record, 0U, FixtureByteCount());
    PackageResourceStagingRequest request = BuildStagingRequest(
        resource_registry,
        file_queue,
        table,
        record,
        resource_result.handle,
        REQUEST_ONE,
        output_bytes.data(),
        output_bytes.size());
    PackageResourceStagingQueue queue(PackageResourceStagingQueueDesc{1U, 1U});
    if (queue.Submit(request) != PackageResourceStagingStatus::Queued) {
        return Fail("file byte mismatch request was not queued");
    }

    AsyncFileReadResult file_result = SuccessFileResult(REQUEST_ONE);
    --file_result.byte_count;
    const PackageResourceStagingStatus status = queue.CompleteFileRead(file_result);
    if (status != PackageResourceStagingStatus::FileByteCountMismatch) {
        return Fail("file byte mismatch did not return explicit status");
    }

    file_queue.Shutdown(false);

    PackageResourceStagingCompletion completion;
    if (!DrainOneStagingCompletion(queue, &completion)) {
        return Fail("file byte mismatch staging completion drain failed");
    }

    if (completion.status != PackageResourceStagingStatus::FileByteCountMismatch) {
        return Fail("file byte mismatch completion status changed");
    }

    const PackageResourceStagingSnapshot snapshot = queue.Snapshot();
    if (snapshot.failed_count != 1U) {
        return Fail("file byte mismatch failed count changed");
    }

    if (snapshot.pending_count != 0U) {
        return Fail("file byte mismatch left pending record");
    }

    return 0;
}

int StreamingPackageResourceStagingReportsMissingFileCompletion() {
    PackageResourceStagingQueue queue(PackageResourceStagingQueueDesc{1U, 1U});
    AsyncFileReadResult file_result = SuccessFileResult(REQUEST_ONE);
    const PackageResourceStagingStatus status = queue.CompleteFileRead(file_result);
    if (status != PackageResourceStagingStatus::MissingFileCompletion) {
        return Fail("missing file completion did not return explicit status");
    }

    const PackageResourceStagingSnapshot snapshot = queue.Snapshot();
    if (snapshot.missing_completion_count != 1U) {
        return Fail("missing completion count was not recorded");
    }

    if (snapshot.completion_count != 0U) {
        return Fail("missing completion wrote a staging completion");
    }

    return 0;
}

int StreamingPackageResourceStagingRejectsQueueOverflowWithoutMutation() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue file_queue;
    file_queue.Initialize(2U, 2U);
    file_queue.Start();

    ResourceRegistry resource_registry;
    PackageLoadPlanRecord record;
    BuildPackageRecord(&record, 0U, FixtureByteCount());
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    std::array<std::uint8_t, OUTPUT_CAPACITY> first_output{};
    std::array<std::uint8_t, OUTPUT_CAPACITY> second_output{};
    PackageResourceStagingRequest first_request = BuildStagingRequest(
        resource_registry,
        file_queue,
        table,
        record,
        resource_result.handle,
        REQUEST_ONE,
        first_output.data(),
        first_output.size());
    PackageResourceStagingRequest second_request = BuildStagingRequest(
        resource_registry,
        file_queue,
        table,
        record,
        resource_result.handle,
        REQUEST_TWO,
        second_output.data(),
        second_output.size());
    PackageResourceStagingQueue queue(PackageResourceStagingQueueDesc{1U, 1U});
    if (queue.Submit(first_request) != PackageResourceStagingStatus::Queued) {
        return Fail("first request was not queued");
    }

    const PackageResourceStagingStatus overflow_status = queue.Submit(second_request);
    if (overflow_status != PackageResourceStagingStatus::QueueFull) {
        return Fail("queue overflow did not return explicit status");
    }

    file_queue.Shutdown(false);

    const PackageResourceStagingSnapshot snapshot = queue.Snapshot();
    if (snapshot.pending_count != 1U) {
        return Fail("queue overflow changed pending count");
    }

    if (snapshot.submitted_count != 1U) {
        return Fail("queue overflow submitted extra file work");
    }

    return 0;
}

int StreamingPackageResourceStagingReportsCompletionOverflowWithoutDroppingPending() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue file_queue;
    file_queue.Initialize(2U, 2U);
    file_queue.Start();

    ResourceRegistry resource_registry;
    PackageLoadPlanRecord record;
    BuildPackageRecord(&record, 0U, FixtureByteCount());
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    std::array<std::uint8_t, OUTPUT_CAPACITY> first_output{};
    std::array<std::uint8_t, OUTPUT_CAPACITY> second_output{};
    PackageResourceStagingRequest first_request = BuildStagingRequest(
        resource_registry,
        file_queue,
        table,
        record,
        resource_result.handle,
        REQUEST_ONE,
        first_output.data(),
        first_output.size());
    PackageResourceStagingRequest second_request = BuildStagingRequest(
        resource_registry,
        file_queue,
        table,
        record,
        resource_result.handle,
        REQUEST_TWO,
        second_output.data(),
        second_output.size());
    PackageResourceStagingQueue queue(PackageResourceStagingQueueDesc{2U, 1U});
    queue.Submit(first_request);
    queue.Submit(second_request);

    if (queue.CompleteFileRead(SuccessFileResult(REQUEST_ONE)) != PackageResourceStagingStatus::Success) {
        return Fail("first completion did not succeed");
    }

    const PackageResourceStagingStatus overflow_status = queue.CompleteFileRead(SuccessFileResult(REQUEST_TWO));
    if (overflow_status != PackageResourceStagingStatus::CompletionQueueFull) {
        return Fail("completion overflow did not return explicit status");
    }

    file_queue.Shutdown(false);

    const PackageResourceStagingSnapshot snapshot = queue.Snapshot();
    if (snapshot.pending_count != 1U) {
        return Fail("completion overflow dropped pending record");
    }

    if (snapshot.completion_count != 1U) {
        return Fail("completion overflow changed completion count");
    }

    return 0;
}

int StreamingPackageResourceStagingRejectsDuplicateRequestId() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue file_queue;
    file_queue.Initialize(2U, 2U);
    file_queue.Start();

    ResourceRegistry resource_registry;
    PackageLoadPlanRecord record;
    BuildPackageRecord(&record, 0U, FixtureByteCount());
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    std::array<std::uint8_t, OUTPUT_CAPACITY> first_output{};
    std::array<std::uint8_t, OUTPUT_CAPACITY> second_output{};
    PackageResourceStagingRequest first_request = BuildStagingRequest(
        resource_registry,
        file_queue,
        table,
        record,
        resource_result.handle,
        REQUEST_ONE,
        first_output.data(),
        first_output.size());
    PackageResourceStagingRequest second_request = BuildStagingRequest(
        resource_registry,
        file_queue,
        table,
        record,
        resource_result.handle,
        REQUEST_ONE,
        second_output.data(),
        second_output.size());
    PackageResourceStagingQueue queue(PackageResourceStagingQueueDesc{2U, 2U});
    if (queue.Submit(first_request) != PackageResourceStagingStatus::Queued) {
        return Fail("first duplicate fixture request did not queue");
    }

    const PackageResourceStagingStatus duplicate_status = queue.Submit(second_request);
    if (duplicate_status != PackageResourceStagingStatus::DuplicateRequestId) {
        return Fail("duplicate request id did not return explicit status");
    }

    file_queue.Shutdown(false);

    const PackageResourceStagingSnapshot snapshot = queue.Snapshot();
    if (snapshot.duplicate_request_count != 1U) {
        return Fail("duplicate request count was not recorded");
    }

    if (snapshot.submitted_count != 1U) {
        return Fail("duplicate request submitted extra file work");
    }

    return 0;
}

int StreamingPackageResourceStagingSnapshotReportsBoundedCounters() {
    PackageResourceStagingQueue queue(PackageResourceStagingQueueDesc{2U, 2U});
    const PackageResourceStagingSnapshot initial_snapshot = queue.Snapshot();
    if (initial_snapshot.request_capacity != 2U) {
        return Fail("snapshot request capacity changed");
    }

    if (initial_snapshot.completion_capacity != 2U) {
        return Fail("snapshot completion capacity changed");
    }

    MountTable table = CreateMountedTable();
    AsyncFileReadQueue file_queue;
    file_queue.Initialize(1U, 1U);
    file_queue.Start();

    ResourceRegistry resource_registry;
    std::array<std::uint8_t, OUTPUT_CAPACITY> output_bytes{};
    PackageResourceStagingRequest request =
        BuildValidRequest(resource_registry, file_queue, table, REQUEST_ONE, output_bytes);
    queue.Submit(request);
    queue.CompleteFileRead(SuccessFileResult(REQUEST_ONE));
    file_queue.Shutdown(false);

    const PackageResourceStagingSnapshot final_snapshot = queue.Snapshot();
    if (final_snapshot.max_pending_count != 1U) {
        return Fail("snapshot max pending count changed");
    }

    if (final_snapshot.max_completion_count != 1U) {
        return Fail("snapshot max completion count changed");
    }

    if (final_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("snapshot allocation accounting vocabulary changed");
    }

    return 0;
}

int StreamingPackageResourceStagingNoUpperRuntimeDependency() {
    PackageResourceStagingQueue queue;
    const PackageResourceStagingSnapshot snapshot = queue.Snapshot();
    if (snapshot.request_capacity == 0U) {
        return Fail("staging queue did not expose bounded request capacity");
    }

    if (snapshot.completion_capacity == 0U) {
        return Fail("staging queue did not expose bounded completion capacity");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("staging queue allocation vocabulary changed");
    }

    return 0;
}

int StreamingResourceUploadCreateBufferFromStagingCompletion() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::array<std::uint8_t, 4U> bytes = BufferUploadBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion staging_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, 4U);
    ResourceUploadRequest request =
        BuildBaseUploadRequest(resource_registry, &device, staging_completion, byte_span, UPLOAD_ONE);
    RhiBufferHandle output_handle{};
    request.upload_kind = ResourceUploadKind::CreateBuffer;
    request.buffer_desc = UploadBufferDesc(bytes.size());
    request.output_buffer_handle = &output_handle;

    ResourceUploadQueue queue(ResourceUploadQueueDesc{2U, 2U});
    if (queue.Submit(request) != ResourceUploadStatus::Queued) {
        return Fail("buffer upload request was not queued");
    }

    if (queue.ProcessNext() != ResourceUploadStatus::Success) {
        return Fail("buffer upload did not succeed");
    }

    ResourceUploadCompletion completion;
    if (!DrainOneUploadCompletion(queue, &completion)) {
        return Fail("buffer upload completion drain failed");
    }

    if (output_handle.generation == 0U) {
        return Fail("buffer upload did not write output handle");
    }

    if (completion.buffer_handle.generation != output_handle.generation) {
        return Fail("buffer completion handle did not match output");
    }

    const ResourceUploadSnapshot snapshot = queue.Snapshot();
    if (snapshot.completed_count != 1U) {
        return Fail("buffer upload completion count was wrong");
    }

    if (snapshot.pending_count != 0U) {
        return Fail("buffer upload left pending record");
    }

    return 0;
}

int StreamingResourceUploadUpdateBufferSignalsFence() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::span<const std::uint8_t> empty_bytes{};
    RhiBufferHandle input_handle{};
    if (device.CreateBuffer(UploadBufferDesc(4U), empty_bytes, input_handle) != RhiStatus::Success) {
        return Fail("buffer primitive creation failed");
    }

    const std::array<std::uint8_t, 4U> bytes = BufferUploadBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion staging_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, 4U);
    ResourceUploadRequest request =
        BuildBaseUploadRequest(resource_registry, &device, staging_completion, byte_span, UPLOAD_ONE);
    RhiFenceHandle output_fence{};
    request.upload_kind = ResourceUploadKind::UpdateBuffer;
    request.input_buffer_handle = input_handle;
    request.output_fence = &output_fence;

    ResourceUploadQueue queue(ResourceUploadQueueDesc{2U, 2U});
    if (queue.Submit(request) != ResourceUploadStatus::Queued) {
        return Fail("buffer update request was not queued");
    }

    if (queue.ProcessNext() != ResourceUploadStatus::Success) {
        return Fail("buffer update did not succeed");
    }

    if (output_fence.generation == 0U) {
        return Fail("buffer update did not write fence");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.resources.updated_primitive_count != 1U) {
        return Fail("buffer update count was not tracked");
    }

    if (snapshot.resources.last_update_bytes != bytes.size()) {
        return Fail("buffer update byte count was not tracked");
    }

    return 0;
}

int StreamingResourceUploadUpdateBufferUsesDestinationOffsetAndRejectsOverflow() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::span<const std::uint8_t> empty_bytes{};
    RhiBufferHandle input_handle{};
    if (device.CreateBuffer(UploadBufferDesc(4U), empty_bytes, input_handle) != RhiStatus::Success) {
        return Fail("buffer primitive creation failed");
    }

    const std::array<std::uint8_t, 2U> bytes{7U, 8U};
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion staging_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, 2U);
    ResourceUploadRequest request =
        BuildBaseUploadRequest(resource_registry, &device, staging_completion, byte_span, UPLOAD_ONE);
    RhiFenceHandle output_fence{};
    request.upload_kind = ResourceUploadKind::UpdateBuffer;
    request.input_buffer_handle = input_handle;
    request.output_fence = &output_fence;
    request.destination_byte_offset = 2ULL;

    ResourceUploadQueue success_queue(ResourceUploadQueueDesc{1U, 1U});
    if (success_queue.Submit(request) != ResourceUploadStatus::Queued) {
        return Fail("buffer update destination request was not queued");
    }

    if (success_queue.ProcessNext() != ResourceUploadStatus::Success) {
        return Fail("buffer update destination request did not succeed");
    }

    if (output_fence.generation == 0U) {
        return Fail("buffer update destination request did not write fence");
    }

    const auto before_reject_snapshot = device.Snapshot();
    const PackageResourceStagingCompletion reject_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_TWO, 2U);
    ResourceUploadRequest reject_request =
        BuildBaseUploadRequest(resource_registry, &device, reject_completion, byte_span, UPLOAD_TWO);
    RhiFenceHandle rejected_fence{};
    reject_request.upload_kind = ResourceUploadKind::UpdateBuffer;
    reject_request.input_buffer_handle = input_handle;
    reject_request.output_fence = &rejected_fence;
    reject_request.destination_byte_offset = 3ULL;

    ResourceUploadQueue reject_queue(ResourceUploadQueueDesc{1U, 1U});
    if (reject_queue.Submit(reject_request) != ResourceUploadStatus::Queued) {
        return Fail("buffer update overflow request was not queued");
    }

    if (reject_queue.ProcessNext() != ResourceUploadStatus::RhiUploadFailed) {
        return Fail("buffer update overflow did not report RHI upload failure");
    }

    ResourceUploadCompletion completion;
    if (!DrainOneUploadCompletion(reject_queue, &completion)) {
        return Fail("buffer update overflow completion drain failed");
    }

    if (completion.rhi_status != RhiStatus::CapacityExceeded) {
        return Fail("buffer update overflow RHI status changed");
    }

    if (completion.fence.generation != 0U || rejected_fence.generation != 0U) {
        return Fail("buffer update overflow wrote fence");
    }

    const auto after_reject_snapshot = device.Snapshot();
    if (after_reject_snapshot.resources.updated_primitive_count !=
        before_reject_snapshot.resources.updated_primitive_count) {
        return Fail("buffer update overflow changed update count");
    }

    if (after_reject_snapshot.resources.signaled_fence_count !=
        before_reject_snapshot.resources.signaled_fence_count) {
        return Fail("buffer update overflow signaled fence");
    }

    return 0;
}

int StreamingResourceUploadCreateTextureFromStagingCompletion() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::array<std::uint8_t, 16U> bytes = TextureUploadBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion staging_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, 16U);
    ResourceUploadRequest request =
        BuildBaseUploadRequest(resource_registry, &device, staging_completion, byte_span, UPLOAD_ONE);
    RhiTextureHandle output_handle{};
    request.upload_kind = ResourceUploadKind::CreateTexture;
    request.texture_desc = UploadTextureDesc();
    request.output_texture_handle = &output_handle;

    ResourceUploadQueue queue(ResourceUploadQueueDesc{2U, 2U});
    if (queue.Submit(request) != ResourceUploadStatus::Queued) {
        return Fail("texture upload request was not queued");
    }

    if (queue.ProcessNext() != ResourceUploadStatus::Success) {
        return Fail("texture upload did not succeed");
    }

    if (output_handle.generation == 0U) {
        return Fail("texture upload did not write output handle");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.resources.texture_count != 1U) {
        return Fail("texture upload count was not tracked");
    }

    return 0;
}

int StreamingResourceUploadUpdateTextureSignalsFence() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::span<const std::uint8_t> empty_bytes{};
    RhiTextureHandle input_handle{};
    if (device.CreateTexture(UploadTextureDesc(), empty_bytes, input_handle) != RhiStatus::Success) {
        return Fail("texture primitive creation failed");
    }

    const std::array<std::uint8_t, 16U> bytes = TextureUploadBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion staging_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, 16U);
    ResourceUploadRequest request =
        BuildBaseUploadRequest(resource_registry, &device, staging_completion, byte_span, UPLOAD_ONE);
    RhiFenceHandle output_fence{};
    request.upload_kind = ResourceUploadKind::UpdateTexture;
    request.input_texture_handle = input_handle;
    request.output_fence = &output_fence;

    ResourceUploadQueue queue(ResourceUploadQueueDesc{2U, 2U});
    if (queue.Submit(request) != ResourceUploadStatus::Queued) {
        return Fail("texture update request was not queued");
    }

    if (queue.ProcessNext() != ResourceUploadStatus::Success) {
        return Fail("texture update did not succeed");
    }

    if (output_fence.generation == 0U) {
        return Fail("texture update did not write fence");
    }

    const auto snapshot = device.Snapshot();
    if (snapshot.resources.updated_primitive_count != 1U) {
        return Fail("texture update count was not tracked");
    }

    if (snapshot.resources.last_update_bytes != bytes.size()) {
        return Fail("texture update byte count was not tracked");
    }

    return 0;
}

int StreamingResourceUploadUpdateTextureUsesDestinationOffsetAndRejectsOverflow() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::span<const std::uint8_t> empty_bytes{};
    RhiTextureHandle input_handle{};
    if (device.CreateTexture(UploadTextureDesc(), empty_bytes, input_handle) != RhiStatus::Success) {
        return Fail("texture primitive creation failed");
    }

    const std::array<std::uint8_t, 8U> bytes{
        16U, 15U, 14U, 13U,
        12U, 11U, 10U, 9U};
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion staging_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, 8U);
    ResourceUploadRequest request =
        BuildBaseUploadRequest(resource_registry, &device, staging_completion, byte_span, UPLOAD_ONE);
    RhiFenceHandle output_fence{};
    request.upload_kind = ResourceUploadKind::UpdateTexture;
    request.input_texture_handle = input_handle;
    request.output_fence = &output_fence;
    request.destination_byte_offset = 8ULL;

    ResourceUploadQueue success_queue(ResourceUploadQueueDesc{1U, 1U});
    if (success_queue.Submit(request) != ResourceUploadStatus::Queued) {
        return Fail("texture update destination request was not queued");
    }

    if (success_queue.ProcessNext() != ResourceUploadStatus::Success) {
        return Fail("texture update destination request did not succeed");
    }

    if (output_fence.generation == 0U) {
        return Fail("texture update destination request did not write fence");
    }

    const auto before_reject_snapshot = device.Snapshot();
    const PackageResourceStagingCompletion reject_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_TWO, 8U);
    ResourceUploadRequest reject_request =
        BuildBaseUploadRequest(resource_registry, &device, reject_completion, byte_span, UPLOAD_TWO);
    RhiFenceHandle rejected_fence{};
    reject_request.upload_kind = ResourceUploadKind::UpdateTexture;
    reject_request.input_texture_handle = input_handle;
    reject_request.output_fence = &rejected_fence;
    reject_request.destination_byte_offset = 12ULL;

    ResourceUploadQueue reject_queue(ResourceUploadQueueDesc{1U, 1U});
    if (reject_queue.Submit(reject_request) != ResourceUploadStatus::Queued) {
        return Fail("texture update overflow request was not queued");
    }

    if (reject_queue.ProcessNext() != ResourceUploadStatus::RhiUploadFailed) {
        return Fail("texture update overflow did not report RHI upload failure");
    }

    ResourceUploadCompletion completion;
    if (!DrainOneUploadCompletion(reject_queue, &completion)) {
        return Fail("texture update overflow completion drain failed");
    }

    if (completion.rhi_status != RhiStatus::InvalidDescriptor) {
        return Fail("texture update overflow RHI status changed");
    }

    if (completion.fence.generation != 0U || rejected_fence.generation != 0U) {
        return Fail("texture update overflow wrote fence");
    }

    const auto after_reject_snapshot = device.Snapshot();
    if (after_reject_snapshot.resources.updated_primitive_count !=
        before_reject_snapshot.resources.updated_primitive_count) {
        return Fail("texture update overflow changed update count");
    }

    if (after_reject_snapshot.resources.signaled_fence_count !=
        before_reject_snapshot.resources.signaled_fence_count) {
        return Fail("texture update overflow signaled fence");
    }

    return 0;
}

int StreamingResourceUploadRejectsInvalidResourceHandleWithoutMutation() {
    ResourceRegistry resource_registry;
    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::array<std::uint8_t, 4U> bytes = BufferUploadBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion staging_completion =
        BuildSuccessfulStagingCompletion(ResourceHandle{}, TYPE_TEXTURE, REQUEST_ONE, 4U);
    ResourceUploadRequest request =
        BuildBaseUploadRequest(resource_registry, &device, staging_completion, byte_span, UPLOAD_ONE);
    RhiBufferHandle output_handle{};
    request.upload_kind = ResourceUploadKind::CreateBuffer;
    request.buffer_desc = UploadBufferDesc(bytes.size());
    request.output_buffer_handle = &output_handle;

    ResourceUploadQueue queue(ResourceUploadQueueDesc{1U, 1U});
    const ResourceUploadStatus status = queue.Submit(request);
    if (status != ResourceUploadStatus::ResourceValidationFailed) {
        return Fail("invalid resource handle did not return validation failure");
    }

    const ResourceUploadSnapshot snapshot = queue.Snapshot();
    if (snapshot.pending_count != 0U) {
        return Fail("invalid resource handle changed pending count");
    }

    if (snapshot.last_resource_status != ResourceStatus::InvalidHandle) {
        return Fail("invalid resource status was not preserved");
    }

    if (output_handle.generation != 0U) {
        return Fail("invalid resource handle wrote output handle");
    }

    return 0;
}

int StreamingResourceUploadRejectsResourceTypeMismatchWithoutMutation() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry, TYPE_TEXTURE);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::array<std::uint8_t, 4U> bytes = BufferUploadBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion staging_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_AUDIO, REQUEST_ONE, 4U);
    ResourceUploadRequest request =
        BuildBaseUploadRequest(resource_registry, &device, staging_completion, byte_span, UPLOAD_ONE);
    RhiBufferHandle output_handle{};
    request.upload_kind = ResourceUploadKind::CreateBuffer;
    request.buffer_desc = UploadBufferDesc(bytes.size());
    request.output_buffer_handle = &output_handle;

    ResourceUploadQueue queue(ResourceUploadQueueDesc{1U, 1U});
    const ResourceUploadStatus status = queue.Submit(request);
    if (status != ResourceUploadStatus::TypeMismatch) {
        return Fail("resource type mismatch did not return explicit status");
    }

    const ResourceUploadSnapshot snapshot = queue.Snapshot();
    if (snapshot.last_resource_status != ResourceStatus::TypeMismatch) {
        return Fail("type mismatch resource status was not preserved");
    }

    if (output_handle.generation != 0U) {
        return Fail("type mismatch wrote output handle");
    }

    return 0;
}

int StreamingResourceUploadRejectsFailedStagingCompletion() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::array<std::uint8_t, 4U> bytes = BufferUploadBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    PackageResourceStagingCompletion staging_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, 4U);
    staging_completion.status = PackageResourceStagingStatus::FileReadFailed;
    ResourceUploadRequest request =
        BuildBaseUploadRequest(resource_registry, &device, staging_completion, byte_span, UPLOAD_ONE);
    RhiBufferHandle output_handle{};
    request.upload_kind = ResourceUploadKind::CreateBuffer;
    request.buffer_desc = UploadBufferDesc(bytes.size());
    request.output_buffer_handle = &output_handle;

    ResourceUploadQueue queue(ResourceUploadQueueDesc{1U, 1U});
    const ResourceUploadStatus status = queue.Submit(request);
    if (status != ResourceUploadStatus::InvalidStagingCompletion) {
        return Fail("failed staging completion did not return explicit status");
    }

    if (queue.Snapshot().pending_count != 0U) {
        return Fail("failed staging completion changed pending count");
    }

    return 0;
}

int StreamingResourceUploadRejectsEmptyUploadBytes() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::array<std::uint8_t, 4U> bytes = BufferUploadBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion staging_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, 4U);
    ResourceUploadRequest request =
        BuildBaseUploadRequest(resource_registry, &device, staging_completion, byte_span, UPLOAD_ONE);
    RhiBufferHandle output_handle{};
    request.upload_kind = ResourceUploadKind::CreateBuffer;
    request.buffer_desc = UploadBufferDesc(bytes.size());
    request.output_buffer_handle = &output_handle;
    request.upload_byte_count = 0U;

    ResourceUploadQueue queue(ResourceUploadQueueDesc{1U, 1U});
    if (queue.Submit(request) != ResourceUploadStatus::EmptyUploadBytes) {
        return Fail("empty upload bytes did not return explicit status");
    }

    if (output_handle.generation != 0U) {
        return Fail("empty upload bytes wrote output handle");
    }

    return 0;
}

int StreamingResourceUploadRejectsByteRangeOverflow() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::array<std::uint8_t, 4U> bytes = BufferUploadBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion staging_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, 4U);
    ResourceUploadRequest request =
        BuildBaseUploadRequest(resource_registry, &device, staging_completion, byte_span, UPLOAD_ONE);
    RhiBufferHandle output_handle{};
    request.upload_kind = ResourceUploadKind::CreateBuffer;
    request.buffer_desc = UploadBufferDesc(bytes.size());
    request.output_buffer_handle = &output_handle;
    request.upload_byte_offset = 3U;
    request.upload_byte_count = 2U;

    ResourceUploadQueue queue(ResourceUploadQueueDesc{1U, 1U});
    if (queue.Submit(request) != ResourceUploadStatus::ByteRangeOutOfBounds) {
        return Fail("upload byte range overflow did not return explicit status");
    }

    if (queue.Snapshot().pending_count != 0U) {
        return Fail("byte range overflow changed pending count");
    }

    return 0;
}

int StreamingResourceUploadRejectsNullRhiDevice() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    const std::array<std::uint8_t, 4U> bytes = BufferUploadBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion staging_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, 4U);
    ResourceUploadRequest request =
        BuildBaseUploadRequest(resource_registry, nullptr, staging_completion, byte_span, UPLOAD_ONE);
    RhiBufferHandle output_handle{};
    request.upload_kind = ResourceUploadKind::CreateBuffer;
    request.buffer_desc = UploadBufferDesc(bytes.size());
    request.output_buffer_handle = &output_handle;

    ResourceUploadQueue queue(ResourceUploadQueueDesc{1U, 1U});
    if (queue.Submit(request) != ResourceUploadStatus::InvalidArgument) {
        return Fail("null RHI device did not return invalid argument");
    }

    if (output_handle.generation != 0U) {
        return Fail("null RHI device wrote output handle");
    }

    return 0;
}

int StreamingResourceUploadRejectsNullOutputStorage() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::array<std::uint8_t, 4U> bytes = BufferUploadBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion staging_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, 4U);
    ResourceUploadRequest request =
        BuildBaseUploadRequest(resource_registry, &device, staging_completion, byte_span, UPLOAD_ONE);
    request.upload_kind = ResourceUploadKind::CreateBuffer;
    request.buffer_desc = UploadBufferDesc(bytes.size());
    request.output_buffer_handle = nullptr;

    ResourceUploadQueue queue(ResourceUploadQueueDesc{1U, 1U});
    if (queue.Submit(request) != ResourceUploadStatus::InvalidArgument) {
        return Fail("null output storage did not return invalid argument");
    }

    if (queue.Snapshot().pending_count != 0U) {
        return Fail("null output storage changed pending count");
    }

    return 0;
}

int StreamingResourceUploadRejectsUnsupportedUploadKind() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::array<std::uint8_t, 4U> bytes = BufferUploadBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion staging_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, 4U);
    const ResourceUploadRequest request =
        BuildBaseUploadRequest(resource_registry, &device, staging_completion, byte_span, UPLOAD_ONE);
    ResourceUploadQueue queue(ResourceUploadQueueDesc{1U, 1U});
    if (queue.Submit(request) != ResourceUploadStatus::UnsupportedUploadKind) {
        return Fail("unsupported upload kind did not return explicit status");
    }

    if (queue.Snapshot().pending_count != 0U) {
        return Fail("unsupported upload kind changed pending count");
    }

    return 0;
}

int StreamingResourceUploadRejectsQueueOverflowWithoutMutation() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::array<std::uint8_t, 4U> bytes = BufferUploadBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion first_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, 4U);
    const PackageResourceStagingCompletion second_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_TWO, 4U);
    ResourceUploadRequest first_request =
        BuildBaseUploadRequest(resource_registry, &device, first_completion, byte_span, UPLOAD_ONE);
    ResourceUploadRequest second_request =
        BuildBaseUploadRequest(resource_registry, &device, second_completion, byte_span, UPLOAD_TWO);
    RhiBufferHandle first_handle{};
    RhiBufferHandle second_handle{};
    first_request.upload_kind = ResourceUploadKind::CreateBuffer;
    first_request.buffer_desc = UploadBufferDesc(bytes.size());
    first_request.output_buffer_handle = &first_handle;
    second_request.upload_kind = ResourceUploadKind::CreateBuffer;
    second_request.buffer_desc = UploadBufferDesc(bytes.size());
    second_request.output_buffer_handle = &second_handle;

    ResourceUploadQueue queue(ResourceUploadQueueDesc{1U, 1U});
    if (queue.Submit(first_request) != ResourceUploadStatus::Queued) {
        return Fail("first upload request was not queued");
    }

    if (queue.Submit(second_request) != ResourceUploadStatus::QueueFull) {
        return Fail("upload queue overflow did not return explicit status");
    }

    const ResourceUploadSnapshot snapshot = queue.Snapshot();
    if (snapshot.pending_count != 1U) {
        return Fail("upload queue overflow changed pending count");
    }

    if (second_handle.generation != 0U) {
        return Fail("upload queue overflow wrote output handle");
    }

    return 0;
}

int StreamingResourceUploadReportsCompletionOverflowWithoutProcessingPending() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::array<std::uint8_t, 4U> bytes = BufferUploadBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion first_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, 4U);
    const PackageResourceStagingCompletion second_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_TWO, 4U);
    ResourceUploadRequest first_request =
        BuildBaseUploadRequest(resource_registry, &device, first_completion, byte_span, UPLOAD_ONE);
    ResourceUploadRequest second_request =
        BuildBaseUploadRequest(resource_registry, &device, second_completion, byte_span, UPLOAD_TWO);
    RhiBufferHandle first_handle{};
    RhiBufferHandle second_handle{};
    first_request.upload_kind = ResourceUploadKind::CreateBuffer;
    first_request.buffer_desc = UploadBufferDesc(bytes.size());
    first_request.output_buffer_handle = &first_handle;
    second_request.upload_kind = ResourceUploadKind::CreateBuffer;
    second_request.buffer_desc = UploadBufferDesc(bytes.size());
    second_request.output_buffer_handle = &second_handle;

    ResourceUploadQueue queue(ResourceUploadQueueDesc{2U, 1U});
    queue.Submit(first_request);
    queue.Submit(second_request);
    if (queue.ProcessNext() != ResourceUploadStatus::Success) {
        return Fail("first upload did not succeed");
    }

    if (queue.ProcessNext() != ResourceUploadStatus::CompletionQueueFull) {
        return Fail("upload completion overflow did not return explicit status");
    }

    const ResourceUploadSnapshot snapshot = queue.Snapshot();
    if (snapshot.pending_count != 1U) {
        return Fail("upload completion overflow dropped pending record");
    }

    if (second_handle.generation != 0U) {
        return Fail("upload completion overflow processed pending record");
    }

    return 0;
}

int StreamingResourceUploadRejectsDuplicateUploadId() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::array<std::uint8_t, 4U> bytes = BufferUploadBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion first_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, 4U);
    const PackageResourceStagingCompletion second_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_TWO, 4U);
    ResourceUploadRequest first_request =
        BuildBaseUploadRequest(resource_registry, &device, first_completion, byte_span, UPLOAD_ONE);
    ResourceUploadRequest second_request =
        BuildBaseUploadRequest(resource_registry, &device, second_completion, byte_span, UPLOAD_ONE);
    RhiBufferHandle first_handle{};
    RhiBufferHandle second_handle{};
    first_request.upload_kind = ResourceUploadKind::CreateBuffer;
    first_request.buffer_desc = UploadBufferDesc(bytes.size());
    first_request.output_buffer_handle = &first_handle;
    second_request.upload_kind = ResourceUploadKind::CreateBuffer;
    second_request.buffer_desc = UploadBufferDesc(bytes.size());
    second_request.output_buffer_handle = &second_handle;

    ResourceUploadQueue queue(ResourceUploadQueueDesc{2U, 2U});
    if (queue.Submit(first_request) != ResourceUploadStatus::Queued) {
        return Fail("first upload request did not queue");
    }

    if (queue.Submit(second_request) != ResourceUploadStatus::DuplicateUploadId) {
        return Fail("duplicate upload id did not return explicit status");
    }

    const ResourceUploadSnapshot snapshot = queue.Snapshot();
    if (snapshot.duplicate_upload_count != 1U) {
        return Fail("duplicate upload count was not tracked");
    }

    if (second_handle.generation != 0U) {
        return Fail("duplicate upload id wrote output handle");
    }

    return 0;
}

int StreamingResourceUploadSnapshotReportsBoundedCounters() {
    ResourceUploadQueue queue(ResourceUploadQueueDesc{2U, 2U});
    const ResourceUploadSnapshot initial_snapshot = queue.Snapshot();
    if (initial_snapshot.request_capacity != 2U) {
        return Fail("upload snapshot request capacity changed");
    }

    if (initial_snapshot.completion_capacity != 2U) {
        return Fail("upload snapshot completion capacity changed");
    }

    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::array<std::uint8_t, 4U> bytes = BufferUploadBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion staging_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, 4U);
    ResourceUploadRequest request =
        BuildBaseUploadRequest(resource_registry, &device, staging_completion, byte_span, UPLOAD_ONE);
    RhiBufferHandle output_handle{};
    request.upload_kind = ResourceUploadKind::CreateBuffer;
    request.buffer_desc = UploadBufferDesc(bytes.size());
    request.output_buffer_handle = &output_handle;
    queue.Submit(request);
    queue.ProcessNext();

    const ResourceUploadSnapshot final_snapshot = queue.Snapshot();
    if (final_snapshot.max_pending_count != 1U) {
        return Fail("upload snapshot max pending count changed");
    }

    if (final_snapshot.max_completion_count != 1U) {
        return Fail("upload snapshot max completion count changed");
    }

    if (final_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("upload snapshot allocation vocabulary changed");
    }

    return 0;
}

int StreamingResourceUploadReportsRhiFailureWithoutWritingOutput() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::array<std::uint8_t, 4U> bytes = BufferUploadBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const PackageResourceStagingCompletion staging_completion =
        BuildSuccessfulStagingCompletion(resource_result.handle, TYPE_TEXTURE, REQUEST_ONE, 4U);
    ResourceUploadRequest request =
        BuildBaseUploadRequest(resource_registry, &device, staging_completion, byte_span, UPLOAD_ONE);
    RhiBufferHandle output_handle{};
    request.upload_kind = ResourceUploadKind::CreateBuffer;
    request.buffer_desc = RhiBufferDesc{};
    request.output_buffer_handle = &output_handle;

    ResourceUploadQueue queue(ResourceUploadQueueDesc{1U, 1U});
    if (queue.Submit(request) != ResourceUploadStatus::Queued) {
        return Fail("RHI failure fixture was not queued");
    }

    if (queue.ProcessNext() != ResourceUploadStatus::RhiUploadFailed) {
        return Fail("RHI failure did not return explicit status");
    }

    ResourceUploadCompletion completion;
    if (!DrainOneUploadCompletion(queue, &completion)) {
        return Fail("RHI failure completion drain failed");
    }

    if (completion.rhi_status != RhiStatus::InvalidDescriptor) {
        return Fail("RHI failure status was not preserved");
    }

    if (output_handle.generation != 0U) {
        return Fail("RHI failure wrote output handle");
    }

    const ResourceUploadSnapshot snapshot = queue.Snapshot();
    if (snapshot.rhi_upload_failed_count != 1U) {
        return Fail("RHI failure count was not tracked");
    }

    return 0;
}

int StreamingResourceUploadCommitCommitsSuccessfulUpload() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    const ResourceUploadCompletion upload_completion = BuildUploadCompletion(
        resource_result.handle,
        TYPE_TEXTURE,
        ResourceUploadStatus::Success,
        UPLOAD_ONE,
        REQUEST_ONE,
        4U);
    const ResourceUploadCommitRequest request =
        BuildUploadCommitRequest(resource_registry, upload_completion, COMMIT_ONE);
    ResourceUploadCommitQueue queue(ResourceUploadCommitQueueDesc{2U, 2U});
    if (queue.Submit(request) != ResourceUploadCommitStatus::Queued) {
        return Fail("upload commit request was not queued");
    }

    if (queue.ProcessNext() != ResourceUploadCommitStatus::Success) {
        return Fail("upload commit request did not succeed");
    }

    ResourceUploadCommitCompletion completion;
    if (!DrainOneUploadCommitCompletion(queue, &completion)) {
        return Fail("upload commit completion drain failed");
    }

    if (completion.load_state != ResourceLoadState::Uploaded) {
        return Fail("upload commit did not report uploaded state");
    }

    if (completion.commit_id != COMMIT_ONE) {
        return Fail("upload commit id was not preserved");
    }

    if (!ResourceLoadStateMatches(
            resource_registry,
            resource_result.handle,
            TYPE_TEXTURE,
            ResourceLoadState::Uploaded)) {
        return Fail("upload commit did not update resource load state");
    }

    const ResourceUploadCommitSnapshot snapshot = queue.Snapshot();
    if (snapshot.committed_count != 1U) {
        return Fail("upload commit snapshot committed count changed");
    }

    if (snapshot.pending_count != 0U) {
        return Fail("upload commit left pending records");
    }

    return 0;
}

int StreamingResourceUploadCommitCommitsFailedUpload() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    const ResourceUploadCompletion upload_completion = BuildUploadCompletion(
        resource_result.handle,
        TYPE_TEXTURE,
        ResourceUploadStatus::RhiUploadFailed,
        UPLOAD_ONE,
        REQUEST_ONE,
        4U);
    const ResourceUploadCommitRequest request =
        BuildUploadCommitRequest(resource_registry, upload_completion, COMMIT_ONE);
    ResourceUploadCommitQueue queue(ResourceUploadCommitQueueDesc{1U, 1U});
    if (queue.Submit(request) != ResourceUploadCommitStatus::Queued) {
        return Fail("failed upload commit request was not queued");
    }

    if (queue.ProcessNext() != ResourceUploadCommitStatus::Success) {
        return Fail("failed upload commit did not record terminal state");
    }

    ResourceUploadCommitCompletion completion;
    if (!DrainOneUploadCommitCompletion(queue, &completion)) {
        return Fail("failed upload commit completion drain failed");
    }

    if (completion.load_state != ResourceLoadState::Failed) {
        return Fail("failed upload commit did not report failed load state");
    }

    if (completion.upload_status != ResourceUploadStatus::RhiUploadFailed) {
        return Fail("failed upload status was not preserved");
    }

    if (completion.rhi_status != RhiStatus::InvalidDescriptor) {
        return Fail("failed upload RHI status was not preserved");
    }

    if (!ResourceLoadStateMatches(
            resource_registry,
            resource_result.handle,
            TYPE_TEXTURE,
            ResourceLoadState::Failed)) {
        return Fail("failed upload commit did not update resource load state");
    }

    const ResourceUploadCommitSnapshot snapshot = queue.Snapshot();
    if (snapshot.failed_upload_commit_count != 1U) {
        return Fail("failed upload commit count was not tracked");
    }

    return 0;
}

int StreamingResourceUploadCommitRejectsInvalidResourceHandleWithoutMutation() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    const ResourceUploadCompletion upload_completion = BuildUploadCompletion(
        ResourceHandle{},
        TYPE_TEXTURE,
        ResourceUploadStatus::Success,
        UPLOAD_ONE,
        REQUEST_ONE,
        4U);
    const ResourceUploadCommitRequest request =
        BuildUploadCommitRequest(resource_registry, upload_completion, COMMIT_ONE);
    ResourceUploadCommitQueue queue(ResourceUploadCommitQueueDesc{1U, 1U});
    if (queue.Submit(request) != ResourceUploadCommitStatus::Queued) {
        return Fail("invalid handle upload commit request was not queued");
    }

    if (queue.ProcessNext() != ResourceUploadCommitStatus::ResourceCommitFailed) {
        return Fail("invalid handle upload commit did not fail during resource commit");
    }

    ResourceUploadCommitCompletion completion;
    if (!DrainOneUploadCommitCompletion(queue, &completion)) {
        return Fail("invalid handle upload commit completion drain failed");
    }

    if (completion.resource_commit_status != ResourceLoadCommitStatus::InvalidHandle) {
        return Fail("invalid handle resource commit status was not preserved");
    }

    if (!ResourceLoadStateMatches(
            resource_registry,
            resource_result.handle,
            TYPE_TEXTURE,
            ResourceLoadState::Unloaded)) {
        return Fail("invalid handle upload commit mutated a valid resource");
    }

    return 0;
}

int StreamingResourceUploadCommitRejectsTypeMismatchWithoutMutation() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("resource registration failed");
    }

    const ResourceUploadCompletion upload_completion = BuildUploadCompletion(
        resource_result.handle,
        TYPE_AUDIO,
        ResourceUploadStatus::Success,
        UPLOAD_ONE,
        REQUEST_ONE,
        4U);
    const ResourceUploadCommitRequest request =
        BuildUploadCommitRequest(resource_registry, upload_completion, COMMIT_ONE);
    ResourceUploadCommitQueue queue(ResourceUploadCommitQueueDesc{1U, 1U});
    if (queue.Submit(request) != ResourceUploadCommitStatus::Queued) {
        return Fail("type mismatch upload commit request was not queued");
    }

    if (queue.ProcessNext() != ResourceUploadCommitStatus::ResourceCommitFailed) {
        return Fail("type mismatch upload commit did not fail during resource commit");
    }

    ResourceUploadCommitCompletion completion;
    if (!DrainOneUploadCommitCompletion(queue, &completion)) {
        return Fail("type mismatch upload commit completion drain failed");
    }

    if (completion.resource_commit_status != ResourceLoadCommitStatus::TypeMismatch) {
        return Fail("type mismatch resource commit status was not preserved");
    }

    if (!ResourceLoadStateMatches(
            resource_registry,
            resource_result.handle,
            TYPE_TEXTURE,
            ResourceLoadState::Unloaded)) {
        return Fail("type mismatch upload commit mutated the resource");
    }

    return 0;
}

int StreamingResourceUploadCommitRejectsDuplicateCommitId() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult first_resource = RegisterResource(resource_registry);
    if (!first_resource.Succeeded()) {
        return Fail("first resource registration failed");
    }

    const ResourceRegistrationResult second_resource =
        RegisterResourceWithKey(resource_registry, TYPE_TEXTURE, RESOURCE_KEY_ALT);
    if (!second_resource.Succeeded()) {
        return Fail("second resource registration failed");
    }

    const ResourceUploadCompletion first_completion = BuildUploadCompletion(
        first_resource.handle,
        TYPE_TEXTURE,
        ResourceUploadStatus::Success,
        UPLOAD_ONE,
        REQUEST_ONE,
        4U);
    const ResourceUploadCompletion second_completion = BuildUploadCompletion(
        second_resource.handle,
        TYPE_TEXTURE,
        ResourceUploadStatus::Success,
        UPLOAD_TWO,
        REQUEST_TWO,
        4U);
    const ResourceUploadCommitRequest first_request =
        BuildUploadCommitRequest(resource_registry, first_completion, COMMIT_ONE);
    const ResourceUploadCommitRequest second_request =
        BuildUploadCommitRequest(resource_registry, second_completion, COMMIT_ONE);
    ResourceUploadCommitQueue queue(ResourceUploadCommitQueueDesc{2U, 2U});
    if (queue.Submit(first_request) != ResourceUploadCommitStatus::Queued) {
        return Fail("first upload commit request was not queued");
    }

    if (queue.ProcessNext() != ResourceUploadCommitStatus::Success) {
        return Fail("first upload commit request did not succeed");
    }

    if (queue.Submit(second_request) != ResourceUploadCommitStatus::DuplicateCommitId) {
        return Fail("duplicate commit id did not return explicit status");
    }

    const ResourceUploadCommitSnapshot snapshot = queue.Snapshot();
    if (snapshot.duplicate_commit_count != 1U) {
        return Fail("duplicate commit count was not tracked");
    }

    if (!ResourceLoadStateMatches(
            resource_registry,
            second_resource.handle,
            TYPE_TEXTURE,
            ResourceLoadState::Unloaded)) {
        return Fail("duplicate commit id mutated second resource");
    }

    return 0;
}

int StreamingResourceUploadCommitRejectsQueueOverflowWithoutMutation() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult first_resource = RegisterResource(resource_registry);
    if (!first_resource.Succeeded()) {
        return Fail("first resource registration failed");
    }

    const ResourceRegistrationResult second_resource =
        RegisterResourceWithKey(resource_registry, TYPE_TEXTURE, RESOURCE_KEY_ALT);
    if (!second_resource.Succeeded()) {
        return Fail("second resource registration failed");
    }

    const ResourceUploadCompletion first_completion = BuildUploadCompletion(
        first_resource.handle,
        TYPE_TEXTURE,
        ResourceUploadStatus::Success,
        UPLOAD_ONE,
        REQUEST_ONE,
        4U);
    const ResourceUploadCompletion second_completion = BuildUploadCompletion(
        second_resource.handle,
        TYPE_TEXTURE,
        ResourceUploadStatus::Success,
        UPLOAD_TWO,
        REQUEST_TWO,
        4U);
    const ResourceUploadCommitRequest first_request =
        BuildUploadCommitRequest(resource_registry, first_completion, COMMIT_ONE);
    const ResourceUploadCommitRequest second_request =
        BuildUploadCommitRequest(resource_registry, second_completion, COMMIT_TWO);
    ResourceUploadCommitQueue queue(ResourceUploadCommitQueueDesc{1U, 1U});
    if (queue.Submit(first_request) != ResourceUploadCommitStatus::Queued) {
        return Fail("first upload commit request was not queued");
    }

    if (queue.Submit(second_request) != ResourceUploadCommitStatus::QueueFull) {
        return Fail("upload commit queue overflow did not return explicit status");
    }

    const ResourceUploadCommitSnapshot snapshot = queue.Snapshot();
    if (snapshot.pending_count != 1U) {
        return Fail("upload commit queue overflow changed pending count");
    }

    if (!ResourceLoadStateMatches(
            resource_registry,
            second_resource.handle,
            TYPE_TEXTURE,
            ResourceLoadState::Unloaded)) {
        return Fail("upload commit queue overflow mutated second resource");
    }

    return 0;
}

int StreamingResourceUploadCommitReportsCompletionOverflowWithoutProcessingPending() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult first_resource = RegisterResource(resource_registry);
    if (!first_resource.Succeeded()) {
        return Fail("first resource registration failed");
    }

    const ResourceRegistrationResult second_resource =
        RegisterResourceWithKey(resource_registry, TYPE_TEXTURE, RESOURCE_KEY_ALT);
    if (!second_resource.Succeeded()) {
        return Fail("second resource registration failed");
    }

    const ResourceUploadCompletion first_completion = BuildUploadCompletion(
        first_resource.handle,
        TYPE_TEXTURE,
        ResourceUploadStatus::Success,
        UPLOAD_ONE,
        REQUEST_ONE,
        4U);
    const ResourceUploadCompletion second_completion = BuildUploadCompletion(
        second_resource.handle,
        TYPE_TEXTURE,
        ResourceUploadStatus::Success,
        UPLOAD_TWO,
        REQUEST_TWO,
        4U);
    const ResourceUploadCommitRequest first_request =
        BuildUploadCommitRequest(resource_registry, first_completion, COMMIT_ONE);
    const ResourceUploadCommitRequest second_request =
        BuildUploadCommitRequest(resource_registry, second_completion, COMMIT_TWO);
    ResourceUploadCommitQueue queue(ResourceUploadCommitQueueDesc{2U, 1U});
    if (queue.Submit(first_request) != ResourceUploadCommitStatus::Queued) {
        return Fail("first upload commit request was not queued");
    }

    if (queue.Submit(second_request) != ResourceUploadCommitStatus::Queued) {
        return Fail("second upload commit request was not queued");
    }

    if (queue.ProcessNext() != ResourceUploadCommitStatus::Success) {
        return Fail("first upload commit request did not succeed");
    }

    if (queue.ProcessNext() != ResourceUploadCommitStatus::CompletionQueueFull) {
        return Fail("upload commit completion overflow did not return explicit status");
    }

    const ResourceUploadCommitSnapshot snapshot = queue.Snapshot();
    if (snapshot.pending_count != 1U) {
        return Fail("upload commit completion overflow dropped pending record");
    }

    if (!ResourceLoadStateMatches(
            resource_registry,
            second_resource.handle,
            TYPE_TEXTURE,
            ResourceLoadState::Unloaded)) {
        return Fail("upload commit completion overflow processed pending resource");
    }

    return 0;
}

int StreamingResourceUploadCommitPreservesOldestOrderAfterSlotReuse() {
    ResourceRegistry resource_registry;
    const ResourceRegistrationResult first_resource = RegisterResource(resource_registry);
    if (!first_resource.Succeeded()) {
        return Fail("first resource registration failed");
    }

    const ResourceRegistrationResult second_resource =
        RegisterResourceWithKey(resource_registry, TYPE_TEXTURE, RESOURCE_KEY_ALT);
    if (!second_resource.Succeeded()) {
        return Fail("second resource registration failed");
    }

    const ResourceRegistrationResult third_resource =
        RegisterResourceWithKey(resource_registry, TYPE_TEXTURE, RESOURCE_KEY_THIRD);
    if (!third_resource.Succeeded()) {
        return Fail("third resource registration failed");
    }

    const ResourceUploadCompletion first_completion = BuildUploadCompletion(
        first_resource.handle,
        TYPE_TEXTURE,
        ResourceUploadStatus::Success,
        UPLOAD_ONE,
        REQUEST_ONE,
        4U);
    const ResourceUploadCompletion second_completion = BuildUploadCompletion(
        second_resource.handle,
        TYPE_TEXTURE,
        ResourceUploadStatus::Success,
        UPLOAD_TWO,
        REQUEST_TWO,
        4U);
    const ResourceUploadCompletion third_completion = BuildUploadCompletion(
        third_resource.handle,
        TYPE_TEXTURE,
        ResourceUploadStatus::Success,
        UPLOAD_THREE,
        REQUEST_THREE,
        4U);
    const ResourceUploadCommitRequest first_request =
        BuildUploadCommitRequest(resource_registry, first_completion, COMMIT_ONE);
    const ResourceUploadCommitRequest second_request =
        BuildUploadCommitRequest(resource_registry, second_completion, COMMIT_TWO);
    const ResourceUploadCommitRequest third_request =
        BuildUploadCommitRequest(resource_registry, third_completion, COMMIT_THREE);
    ResourceUploadCommitQueue queue(ResourceUploadCommitQueueDesc{2U, 3U});
    if (queue.Submit(first_request) != ResourceUploadCommitStatus::Queued) {
        return Fail("first upload commit request was not queued");
    }

    if (queue.Submit(second_request) != ResourceUploadCommitStatus::Queued) {
        return Fail("second upload commit request was not queued");
    }

    if (queue.ProcessNext() != ResourceUploadCommitStatus::Success) {
        return Fail("first upload commit request did not succeed");
    }

    if (queue.Submit(third_request) != ResourceUploadCommitStatus::Queued) {
        return Fail("third upload commit request was not queued");
    }

    if (queue.ProcessNext() != ResourceUploadCommitStatus::Success) {
        return Fail("oldest pending upload commit request did not succeed");
    }

    std::array<ResourceUploadCommitCompletion, 2U> completions{};
    std::uint32_t written_count = 0U;
    const ResourceUploadCommitStatus drain_status = queue.DrainCompletions(
        completions.data(),
        static_cast<std::uint32_t>(completions.size()),
        &written_count);
    if (drain_status != ResourceUploadCommitStatus::Success) {
        return Fail("slot reuse upload commit drain failed");
    }

    if (written_count != 2U) {
        return Fail("slot reuse upload commit drain count changed");
    }

    if (completions[0U].commit_id != COMMIT_ONE) {
        return Fail("first upload commit completion order changed");
    }

    if (completions[1U].commit_id != COMMIT_TWO) {
        return Fail("oldest pending upload commit order changed");
    }

    if (!ResourceLoadStateMatches(
            resource_registry,
            second_resource.handle,
            TYPE_TEXTURE,
            ResourceLoadState::Uploaded)) {
        return Fail("second upload commit did not update resource load state");
    }

    if (!ResourceLoadStateMatches(
            resource_registry,
            third_resource.handle,
            TYPE_TEXTURE,
            ResourceLoadState::Unloaded)) {
        return Fail("third upload commit was processed before older pending request");
    }

    const ResourceUploadCommitSnapshot snapshot = queue.Snapshot();
    if (snapshot.pending_count != 1U) {
        return Fail("slot reuse upload commit pending count changed");
    }

    if (snapshot.committed_count != 2U) {
        return Fail("slot reuse upload commit committed count changed");
    }

    return 0;
}

int StreamingResourceUploadCommitSnapshotReportsBoundedCounters() {
    ResourceUploadCommitQueue queue(ResourceUploadCommitQueueDesc{2U, 2U});
    const ResourceUploadCommitSnapshot initial_snapshot = queue.Snapshot();
    if (initial_snapshot.request_capacity != 2U) {
        return Fail("upload commit snapshot request capacity changed");
    }

    if (initial_snapshot.completion_capacity != 2U) {
        return Fail("upload commit snapshot completion capacity changed");
    }

    ResourceRegistry resource_registry;
    const ResourceRegistrationResult first_resource = RegisterResource(resource_registry);
    if (!first_resource.Succeeded()) {
        return Fail("first resource registration failed");
    }

    const ResourceRegistrationResult second_resource =
        RegisterResourceWithKey(resource_registry, TYPE_TEXTURE, RESOURCE_KEY_ALT);
    if (!second_resource.Succeeded()) {
        return Fail("second resource registration failed");
    }

    const ResourceUploadCompletion first_completion = BuildUploadCompletion(
        first_resource.handle,
        TYPE_TEXTURE,
        ResourceUploadStatus::Success,
        UPLOAD_ONE,
        REQUEST_ONE,
        4U);
    const ResourceUploadCompletion second_completion = BuildUploadCompletion(
        second_resource.handle,
        TYPE_TEXTURE,
        ResourceUploadStatus::RhiUploadFailed,
        UPLOAD_TWO,
        REQUEST_TWO,
        4U);
    const ResourceUploadCommitRequest first_request =
        BuildUploadCommitRequest(resource_registry, first_completion, COMMIT_ONE);
    const ResourceUploadCommitRequest second_request =
        BuildUploadCommitRequest(resource_registry, second_completion, COMMIT_TWO);
    if (queue.Submit(first_request) != ResourceUploadCommitStatus::Queued) {
        return Fail("first snapshot upload commit request was not queued");
    }

    if (queue.Submit(second_request) != ResourceUploadCommitStatus::Queued) {
        return Fail("second snapshot upload commit request was not queued");
    }

    if (queue.ProcessNext() != ResourceUploadCommitStatus::Success) {
        return Fail("first snapshot upload commit request did not succeed");
    }

    if (queue.ProcessNext() != ResourceUploadCommitStatus::Success) {
        return Fail("second snapshot upload commit request did not succeed");
    }

    const ResourceUploadCommitSnapshot final_snapshot = queue.Snapshot();
    if (final_snapshot.max_pending_count != 2U) {
        return Fail("upload commit snapshot max pending count changed");
    }

    if (final_snapshot.max_completion_count != 2U) {
        return Fail("upload commit snapshot max completion count changed");
    }

    if (final_snapshot.submitted_count != 2U) {
        return Fail("upload commit snapshot submitted count changed");
    }

    if (final_snapshot.committed_count != 2U) {
        return Fail("upload commit snapshot committed count changed");
    }

    if (final_snapshot.failed_upload_commit_count != 1U) {
        return Fail("upload commit snapshot failed upload count changed");
    }

    if (final_snapshot.last_load_state != ResourceLoadState::Failed) {
        return Fail("upload commit snapshot last load state changed");
    }

    if (final_snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("upload commit snapshot allocation vocabulary changed");
    }

    return 0;
}

int StreamingResourceStreamingPipelineFixtureBufferReadUploadCommit() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue file_queue;
    if (file_queue.Initialize(2U, 2U) != AsyncFileReadStatus::Success) {
        return Fail("pipeline file queue initialize failed");
    }

    if (file_queue.Start() != AsyncFileReadStatus::Success) {
        return Fail("pipeline file queue start failed");
    }

    PackageLoadPlanRecord record;
    if (!BuildPackageRecord(&record, 0U, FixtureByteCount())) {
        return Fail("pipeline package record build failed");
    }

    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("pipeline resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    std::array<std::uint8_t, OUTPUT_CAPACITY> staged_bytes{};
    RhiBufferHandle output_handle{};
    ResourceStreamingPipelineRequest request;
    request.resource_registry = &resource_registry;
    request.file_queue = &file_queue;
    request.rhi_device = &device;
    request.package_record = record;
    request.resource = resource_result.handle;
    request.expected_type = TYPE_TEXTURE;
    request.file_request = BuildFileRequest(table, staged_bytes.data(), staged_bytes.size());
    request.staged_bytes = std::span<const std::uint8_t>(staged_bytes.data(), staged_bytes.size());
    request.upload_byte_count = FixtureByteCount();
    request.upload_kind = ResourceUploadKind::CreateBuffer;
    request.buffer_desc = UploadBufferDesc(FixtureByteCount());
    request.output_buffer_handle = &output_handle;
    request.staging_request_id = REQUEST_ONE;
    request.upload_id = UPLOAD_ONE;
    request.commit_id = COMMIT_ONE;

    ResourceStreamingPipeline pipeline;
    if (pipeline.Submit(request) != ResourceStreamingPipelineStatus::Queued) {
        return Fail("pipeline submit did not queue");
    }

    if (file_queue.Shutdown(false) != AsyncFileReadStatus::ShutdownComplete) {
        return Fail("pipeline file queue shutdown failed");
    }

    AsyncFileReadResult file_result;
    if (!DrainOneFileCompletion(file_queue, &file_result)) {
        return Fail("pipeline file completion drain failed");
    }

    if (pipeline.CompleteFileRead(file_result) != ResourceStreamingPipelineStatus::Queued) {
        return Fail("pipeline file completion did not queue upload");
    }

    if (pipeline.ProcessUpload() != ResourceStreamingPipelineStatus::Queued) {
        return Fail("pipeline upload did not queue commit");
    }

    if (pipeline.ProcessCommit() != ResourceStreamingPipelineStatus::Success) {
        return Fail("pipeline commit did not succeed");
    }

    if (output_handle.generation == 0U) {
        return Fail("pipeline did not create output buffer");
    }

    if (!ResourceLoadStateMatches(
            resource_registry,
            resource_result.handle,
            TYPE_TEXTURE,
            ResourceLoadState::Uploaded)) {
        return Fail("pipeline did not mark resource uploaded");
    }

    const ResourceUploadCommitCompletion commit_completion = pipeline.LastCommitCompletion();
    if (commit_completion.upload_byte_count != FixtureByteCount()) {
        return Fail("pipeline commit byte count changed");
    }

    const ResourceStreamingPipelineSnapshot snapshot = pipeline.Snapshot();
    if (snapshot.submitted_count != 1U) {
        return Fail("pipeline submitted count changed");
    }

    if (snapshot.completed_count != 1U) {
        return Fail("pipeline completed count changed");
    }

    if (snapshot.failed_count != 0U) {
        return Fail("pipeline failed count changed");
    }

    if (snapshot.has_active_request) {
        return Fail("pipeline active request was not cleared");
    }

    if (resource_registry.CachePayloadSnapshot().cached_payload_count != 0U) {
        return Fail("pipeline default path stored cache payload");
    }

    if (pipeline.LastCachePayloadResult().payload_id != 0U) {
        return Fail("pipeline default path mutated cache payload result");
    }

    return 0;
}

int StreamingResourceStreamingPipelineUpdateBufferUsesPayloadWindowDestinationOffset() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue file_queue;
    if (file_queue.Initialize(2U, 2U) != AsyncFileReadStatus::Success) {
        return Fail("pipeline update file queue initialize failed");
    }

    if (file_queue.Start() != AsyncFileReadStatus::Success) {
        return Fail("pipeline update file queue start failed");
    }

    PackageLoadPlanRecord record;
    if (!BuildPackageRecord(&record, 0U, 2U)) {
        return Fail("pipeline update package record build failed");
    }

    record.payload_logical_byte_count = 4ULL;
    record.payload_window_byte_offset = 3ULL;
    record.payload_window_byte_size = 2ULL;

    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("pipeline update resource registration failed");
    }

    NullRhiDevice device = CreateInitializedUploadDevice();
    const std::span<const std::uint8_t> empty_bytes{};
    RhiBufferHandle input_handle{};
    if (device.CreateBuffer(UploadBufferDesc(4U), empty_bytes, input_handle) != RhiStatus::Success) {
        return Fail("pipeline update input buffer creation failed");
    }

    std::array<std::uint8_t, OUTPUT_CAPACITY> staged_bytes{};
    RhiFenceHandle output_fence{};
    ResourceStreamingPipelineRequest request;
    request.resource_registry = &resource_registry;
    request.file_queue = &file_queue;
    request.rhi_device = &device;
    request.package_record = record;
    request.resource = resource_result.handle;
    request.expected_type = TYPE_TEXTURE;
    request.file_request = BuildFileRequest(table, staged_bytes.data(), staged_bytes.size());
    request.staged_bytes = std::span<const std::uint8_t>(staged_bytes.data(), staged_bytes.size());
    request.upload_byte_count = 2U;
    request.upload_kind = ResourceUploadKind::UpdateBuffer;
    request.input_buffer_handle = input_handle;
    request.output_fence = &output_fence;
    request.staging_request_id = REQUEST_ONE;
    request.upload_id = UPLOAD_ONE;
    request.commit_id = COMMIT_ONE;

    ResourceStreamingPipeline pipeline;
    if (pipeline.Submit(request) != ResourceStreamingPipelineStatus::Queued) {
        return Fail("pipeline update submit did not queue");
    }

    if (file_queue.Shutdown(false) != AsyncFileReadStatus::ShutdownComplete) {
        return Fail("pipeline update file queue shutdown failed");
    }

    AsyncFileReadResult file_result;
    if (!DrainOneFileCompletion(file_queue, &file_result)) {
        return Fail("pipeline update file completion drain failed");
    }

    if (pipeline.CompleteFileRead(file_result) != ResourceStreamingPipelineStatus::Queued) {
        return Fail("pipeline update file completion did not queue upload");
    }

    const auto before_upload_snapshot = device.Snapshot();
    if (pipeline.ProcessUpload() != ResourceStreamingPipelineStatus::Queued) {
        return Fail("pipeline update upload did not queue commit");
    }

    const ResourceUploadCompletion upload_completion = pipeline.LastUploadCompletion();
    if (upload_completion.status != ResourceUploadStatus::RhiUploadFailed) {
        return Fail("pipeline update did not preserve upload failure");
    }

    if (upload_completion.rhi_status != RhiStatus::CapacityExceeded) {
        return Fail("pipeline update did not use payload window destination offset");
    }

    if (upload_completion.fence.generation != 0U || output_fence.generation != 0U) {
        return Fail("pipeline update overflow wrote fence");
    }

    const auto after_upload_snapshot = device.Snapshot();
    if (after_upload_snapshot.resources.updated_primitive_count !=
        before_upload_snapshot.resources.updated_primitive_count) {
        return Fail("pipeline update overflow changed update count");
    }

    if (after_upload_snapshot.resources.signaled_fence_count !=
        before_upload_snapshot.resources.signaled_fence_count) {
        return Fail("pipeline update overflow signaled fence");
    }

    if (pipeline.ProcessCommit() != ResourceStreamingPipelineStatus::UploadProcessFailed) {
        return Fail("pipeline update commit did not surface upload failure");
    }

    const ResourceStreamingPipelineSnapshot pipeline_snapshot = pipeline.Snapshot();
    if (pipeline_snapshot.failed_count != 1U) {
        return Fail("pipeline update failure count changed");
    }

    if (pipeline_snapshot.has_active_request) {
        return Fail("pipeline update active request was not cleared");
    }

    return 0;
}

int StreamingResourceStreamingPipelineCachePayloadOnlyStoresPackageWindow() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue file_queue;
    if (file_queue.Initialize(2U, 2U) != AsyncFileReadStatus::Success) {
        return Fail("pipeline cache payload file queue initialize failed");
    }

    if (file_queue.Start() != AsyncFileReadStatus::Success) {
        return Fail("pipeline cache payload file queue start failed");
    }

    const std::uint32_t archive_window_offset = static_cast<std::uint32_t>(ARCHIVE_WINDOW_OFFSET);
    const std::uint32_t archive_window_size = static_cast<std::uint32_t>(ARCHIVE_WINDOW_SIZE);
    PackageLoadPlanRecord record;
    if (!BuildPackageRecord(&record, archive_window_offset, archive_window_size)) {
        return Fail("pipeline cache payload package record build failed");
    }

    record.payload_logical_byte_count = U64_PAYLOAD_LOGICAL_BYTE_COUNT;
    record.payload_window_byte_offset = PAYLOAD_WINDOW_OFFSET;
    record.payload_window_byte_size = ARCHIVE_WINDOW_SIZE;

    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("pipeline cache payload resource registration failed");
    }

    if (!AdmitCachePayloadResource(resource_registry, resource_result.handle, TYPE_TEXTURE, archive_window_size)) {
        return Fail("pipeline cache payload resource admission failed");
    }

    std::array<std::uint8_t, OUTPUT_CAPACITY> staged_bytes{};
    ResourceStreamingPipelineRequest request;
    request.resource_registry = &resource_registry;
    request.file_queue = &file_queue;
    request.package_record = record;
    request.resource = resource_result.handle;
    request.expected_type = TYPE_TEXTURE;
    request.file_request = BuildFileRequest(table, staged_bytes.data(), staged_bytes.size());
    request.staged_bytes = std::span<const std::uint8_t>(staged_bytes.data(), staged_bytes.size());
    request.store_cache_payload_only = true;
    request.cache_payload_id = CACHE_PAYLOAD_ONE;
    request.staging_request_id = REQUEST_ONE;

    ResourceStreamingPipeline pipeline;
    if (pipeline.Submit(request) != ResourceStreamingPipelineStatus::Queued) {
        return Fail("pipeline cache payload submit did not queue");
    }

    if (file_queue.Shutdown(false) != AsyncFileReadStatus::ShutdownComplete) {
        return Fail("pipeline cache payload file queue shutdown failed");
    }

    AsyncFileReadResult file_result;
    if (!DrainOneFileCompletion(file_queue, &file_result)) {
        return Fail("pipeline cache payload file completion drain failed");
    }

    if (pipeline.CompleteFileRead(file_result) != ResourceStreamingPipelineStatus::Success) {
        return Fail("pipeline cache payload file completion did not store cache payload");
    }

    const ResourceCachePayloadBridgeResult cache_result = pipeline.LastCachePayloadResult();
    if (cache_result.status != ResourceCachePayloadBridgeStatus::Success) {
        return Fail("pipeline cache payload bridge status changed");
    }

    if (cache_result.cache_payload_status != ResourceCachePayloadStatus::Success) {
        return Fail("pipeline cache payload resource status changed");
    }

    if (cache_result.payload_id != CACHE_PAYLOAD_ONE) {
        return Fail("pipeline cache payload id changed");
    }

    if (cache_result.payload_logical_byte_count != U64_PAYLOAD_LOGICAL_BYTE_COUNT) {
        return Fail("pipeline cache payload logical byte count changed");
    }

    if (cache_result.payload_window_byte_offset != PAYLOAD_WINDOW_OFFSET) {
        return Fail("pipeline cache payload window offset changed");
    }

    if (cache_result.payload_window_byte_size != ARCHIVE_WINDOW_SIZE) {
        return Fail("pipeline cache payload window size changed");
    }

    if (pipeline.LastUploadCompletion().upload_id != 0U) {
        return Fail("pipeline cache payload entered upload stage");
    }

    if (pipeline.LastCommitCompletion().commit_id != 0U) {
        return Fail("pipeline cache payload entered commit stage");
    }

    const ResourceStreamingPipelineSnapshot snapshot = pipeline.Snapshot();
    if (snapshot.completed_count != 1U) {
        return Fail("pipeline cache payload completed count changed");
    }

    if (snapshot.failed_count != 0U) {
        return Fail("pipeline cache payload failed count changed");
    }

    if (snapshot.last_cache_payload_status != ResourceCachePayloadBridgeStatus::Success) {
        return Fail("pipeline cache payload snapshot status changed");
    }

    if (snapshot.last_cache_payload_resource_status != ResourceCachePayloadStatus::Success) {
        return Fail("pipeline cache payload snapshot resource status changed");
    }

    if (snapshot.last_upload_status != ResourceUploadStatus::Success) {
        return Fail("pipeline cache payload mutated upload status");
    }

    if (snapshot.has_active_request) {
        return Fail("pipeline cache payload active request was not cleared");
    }

    const ResourceCachePayloadSnapshot cache_snapshot = resource_registry.CachePayloadSnapshot();
    if (cache_snapshot.cached_payload_count != 1U) {
        return Fail("pipeline cache payload did not store resource cache payload");
    }

    if (cache_snapshot.last_payload_logical_byte_count != U64_PAYLOAD_LOGICAL_BYTE_COUNT) {
        return Fail("pipeline cache payload snapshot logical byte count changed");
    }

    if (cache_snapshot.last_payload_window_byte_offset != PAYLOAD_WINDOW_OFFSET) {
        return Fail("pipeline cache payload snapshot window offset changed");
    }

    std::array<std::uint8_t, 4U> output_bytes{};
    std::uint32_t output_byte_count = 0U;
    const std::uint64_t read_window_size = static_cast<std::uint64_t>(output_bytes.size());
    const ResourceCachePayloadRequest read_request = BuildCachePayloadReadRequest(
        resource_result.handle,
        TYPE_TEXTURE,
        CACHE_PAYLOAD_ONE,
        U64_PAYLOAD_LOGICAL_BYTE_COUNT,
        PAYLOAD_WINDOW_OFFSET,
        read_window_size);
    const ResourceCachePayloadStatus read_status = resource_registry.ReadCachePayload(
        read_request,
        output_bytes.data(),
        static_cast<std::uint32_t>(output_bytes.size()),
        &output_byte_count);
    if (read_status != ResourceCachePayloadStatus::Success) {
        return Fail("pipeline cache payload read failed");
    }

    if (output_byte_count != static_cast<std::uint32_t>(output_bytes.size())) {
        return Fail("pipeline cache payload read byte count changed");
    }

    const std::string expected_text = ArchiveWindowText(ARCHIVE_WINDOW_OFFSET, read_window_size);
    const std::string output_text(output_bytes.begin(), output_bytes.end());
    if (output_text != expected_text) {
        return Fail("pipeline cache payload bytes changed");
    }

    return 0;
}

int StreamingResourceStreamingPipelinePackageArtifactFeedsCachePayloadU64Window() {
    MountTable table = CreateMountedTable();
    PackageLoadPlanRecord record;
    if (!BuildPackageArtifactRecord(&table, &record, TEST_PIPELINE_PACKAGE_ARTIFACT_CACHE_PAYLOAD)) {
        return Fail("pipeline package artifact record build failed");
    }

    if (record.archive_byte_offset != ARCHIVE_WINDOW_OFFSET) {
        return Fail("pipeline package artifact archive offset changed");
    }

    if (record.archive_byte_size != ARCHIVE_WINDOW_SIZE) {
        return Fail("pipeline package artifact archive size changed");
    }

    if (record.payload_logical_byte_count != U64_PAYLOAD_LOGICAL_BYTE_COUNT) {
        return Fail("pipeline package artifact payload logical count changed");
    }

    if (record.payload_window_byte_offset != PAYLOAD_WINDOW_OFFSET) {
        return Fail("pipeline package artifact payload window offset changed");
    }

    if (record.payload_window_byte_size != ARCHIVE_WINDOW_SIZE) {
        return Fail("pipeline package artifact payload window size changed");
    }

    AsyncFileReadQueue file_queue;
    if (file_queue.Initialize(2U, 2U) != AsyncFileReadStatus::Success) {
        return Fail("pipeline package artifact file queue initialize failed");
    }

    if (file_queue.Start() != AsyncFileReadStatus::Success) {
        return Fail("pipeline package artifact file queue start failed");
    }

    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("pipeline package artifact resource registration failed");
    }

    const std::uint32_t archive_window_size = static_cast<std::uint32_t>(record.archive_byte_size);
    if (!AdmitCachePayloadResource(resource_registry, resource_result.handle, TYPE_TEXTURE, archive_window_size)) {
        return Fail("pipeline package artifact resource admission failed");
    }

    std::array<std::uint8_t, OUTPUT_CAPACITY> staged_bytes{};
    ResourceStreamingPipelineRequest request;
    request.resource_registry = &resource_registry;
    request.file_queue = &file_queue;
    request.package_record = record;
    request.resource = resource_result.handle;
    request.expected_type = TYPE_TEXTURE;
    request.file_request = BuildFileRequest(table, staged_bytes.data(), staged_bytes.size());
    request.staged_bytes = std::span<const std::uint8_t>(staged_bytes.data(), staged_bytes.size());
    request.store_cache_payload_only = true;
    request.cache_payload_id = CACHE_PAYLOAD_ONE;
    request.staging_request_id = REQUEST_ONE;

    ResourceStreamingPipeline pipeline;
    if (pipeline.Submit(request) != ResourceStreamingPipelineStatus::Queued) {
        return Fail("pipeline package artifact submit did not queue");
    }

    if (file_queue.Shutdown(false) != AsyncFileReadStatus::ShutdownComplete) {
        return Fail("pipeline package artifact file queue shutdown failed");
    }

    AsyncFileReadResult file_result;
    if (!DrainOneFileCompletion(file_queue, &file_result)) {
        return Fail("pipeline package artifact file completion drain failed");
    }

    if (pipeline.CompleteFileRead(file_result) != ResourceStreamingPipelineStatus::Success) {
        return Fail("pipeline package artifact did not store cache payload");
    }

    const PackageResourceStagingCompletion staging_completion = pipeline.LastStagingCompletion();
    if (staging_completion.package_record.payload_logical_byte_count != U64_PAYLOAD_LOGICAL_BYTE_COUNT) {
        return Fail("pipeline package artifact staging logical count changed");
    }

    if (staging_completion.package_record.payload_window_byte_offset != PAYLOAD_WINDOW_OFFSET) {
        return Fail("pipeline package artifact staging window offset changed");
    }

    if (staging_completion.package_record.payload_window_byte_size != ARCHIVE_WINDOW_SIZE) {
        return Fail("pipeline package artifact staging window size changed");
    }

    const ResourceCachePayloadBridgeResult cache_result = pipeline.LastCachePayloadResult();
    if (cache_result.status != ResourceCachePayloadBridgeStatus::Success) {
        return Fail("pipeline package artifact cache bridge status changed");
    }

    if (cache_result.cache_payload_status != ResourceCachePayloadStatus::Success) {
        return Fail("pipeline package artifact resource status changed");
    }

    if (cache_result.payload_logical_byte_count != U64_PAYLOAD_LOGICAL_BYTE_COUNT) {
        return Fail("pipeline package artifact cache logical count changed");
    }

    if (cache_result.payload_window_byte_offset != PAYLOAD_WINDOW_OFFSET) {
        return Fail("pipeline package artifact cache window offset changed");
    }

    if (cache_result.payload_window_byte_size != ARCHIVE_WINDOW_SIZE) {
        return Fail("pipeline package artifact cache window size changed");
    }

    if (pipeline.LastUploadCompletion().upload_id != 0U) {
        return Fail("pipeline package artifact entered upload stage");
    }

    if (pipeline.LastCommitCompletion().commit_id != 0U) {
        return Fail("pipeline package artifact entered commit stage");
    }

    const ResourceStreamingPipelineSnapshot pipeline_snapshot = pipeline.Snapshot();
    if (pipeline_snapshot.completed_count != 1U) {
        return Fail("pipeline package artifact completed count changed");
    }

    if (pipeline_snapshot.failed_count != 0U) {
        return Fail("pipeline package artifact failed count changed");
    }

    if (pipeline_snapshot.has_active_request) {
        return Fail("pipeline package artifact active request was not cleared");
    }

    const ResourceCachePayloadSnapshot cache_snapshot = resource_registry.CachePayloadSnapshot();
    if (cache_snapshot.cached_payload_count != 1U) {
        return Fail("pipeline package artifact did not store cache payload");
    }

    if (cache_snapshot.last_payload_logical_byte_count != U64_PAYLOAD_LOGICAL_BYTE_COUNT) {
        return Fail("pipeline package artifact cache snapshot logical count changed");
    }

    if (cache_snapshot.last_payload_window_byte_offset != PAYLOAD_WINDOW_OFFSET) {
        return Fail("pipeline package artifact cache snapshot window offset changed");
    }

    std::array<std::uint8_t, 4U> output_bytes{};
    std::uint32_t output_byte_count = 0U;
    const std::uint64_t read_window_size = static_cast<std::uint64_t>(output_bytes.size());
    const ResourceCachePayloadRequest read_request = BuildCachePayloadReadRequest(
        resource_result.handle,
        TYPE_TEXTURE,
        CACHE_PAYLOAD_ONE,
        U64_PAYLOAD_LOGICAL_BYTE_COUNT,
        PAYLOAD_WINDOW_OFFSET,
        read_window_size);
    const ResourceCachePayloadStatus read_status = resource_registry.ReadCachePayload(
        read_request,
        output_bytes.data(),
        static_cast<std::uint32_t>(output_bytes.size()),
        &output_byte_count);
    if (read_status != ResourceCachePayloadStatus::Success) {
        return Fail("pipeline package artifact cache payload read failed");
    }

    if (output_byte_count != static_cast<std::uint32_t>(output_bytes.size())) {
        return Fail("pipeline package artifact read byte count changed");
    }

    const std::string expected_text = ArchiveWindowText(ARCHIVE_WINDOW_OFFSET, read_window_size);
    const std::string output_text(output_bytes.begin(), output_bytes.end());
    if (output_text != expected_text) {
        return Fail("pipeline package artifact cache bytes changed");
    }

    std::filesystem::remove_all(StreamingPackageArtifactRoot(TEST_PIPELINE_PACKAGE_ARTIFACT_CACHE_PAYLOAD));
    return 0;
}

int StreamingResourceStreamingPipelineCachePayloadRejectsStoreFailureWithoutMutation() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue file_queue;
    if (file_queue.Initialize(2U, 2U) != AsyncFileReadStatus::Success) {
        return Fail("pipeline cache payload failure file queue initialize failed");
    }

    if (file_queue.Start() != AsyncFileReadStatus::Success) {
        return Fail("pipeline cache payload failure file queue start failed");
    }

    const std::uint32_t archive_window_offset = static_cast<std::uint32_t>(ARCHIVE_WINDOW_OFFSET);
    const std::uint32_t archive_window_size = static_cast<std::uint32_t>(ARCHIVE_WINDOW_SIZE);
    PackageLoadPlanRecord record;
    if (!BuildPackageRecord(&record, archive_window_offset, archive_window_size)) {
        return Fail("pipeline cache payload failure package record build failed");
    }

    record.payload_logical_byte_count = U64_PAYLOAD_LOGICAL_BYTE_COUNT;
    record.payload_window_byte_offset = PAYLOAD_WINDOW_OFFSET;
    record.payload_window_byte_size = ARCHIVE_WINDOW_SIZE;

    ResourceRegistry resource_registry;
    const ResourceRegistrationResult resource_result = RegisterResource(resource_registry);
    if (!resource_result.Succeeded()) {
        return Fail("pipeline cache payload failure resource registration failed");
    }

    if (!AdmitCachePayloadResource(resource_registry, resource_result.handle, TYPE_TEXTURE, archive_window_size)) {
        return Fail("pipeline cache payload failure resource admission failed");
    }

    const std::array<std::uint8_t, 8U> existing_payload_bytes{
        10U, 20U, 30U, 40U,
        50U, 60U, 70U, 80U};
    ResourceCachePayloadRequest store_request = BuildCachePayloadReadRequest(
        resource_result.handle,
        TYPE_TEXTURE,
        CACHE_PAYLOAD_ONE,
        U64_PAYLOAD_LOGICAL_BYTE_COUNT,
        PAYLOAD_WINDOW_OFFSET,
        ARCHIVE_WINDOW_SIZE);
    store_request.payload_bytes = existing_payload_bytes.data();
    store_request.payload_byte_count = archive_window_size;
    if (resource_registry.StoreCachePayload(store_request) != ResourceCachePayloadStatus::Success) {
        return Fail("pipeline cache payload failure seed store failed");
    }

    const ResourceCachePayloadSnapshot before_cache_snapshot = resource_registry.CachePayloadSnapshot();
    std::array<std::uint8_t, OUTPUT_CAPACITY> staged_bytes{};
    ResourceStreamingPipelineRequest request;
    request.resource_registry = &resource_registry;
    request.file_queue = &file_queue;
    request.package_record = record;
    request.resource = resource_result.handle;
    request.expected_type = TYPE_TEXTURE;
    request.file_request = BuildFileRequest(table, staged_bytes.data(), staged_bytes.size());
    request.staged_bytes = std::span<const std::uint8_t>(staged_bytes.data(), staged_bytes.size());
    request.store_cache_payload_only = true;
    request.cache_payload_id = CACHE_PAYLOAD_ONE;
    request.staging_request_id = REQUEST_ONE;

    ResourceStreamingPipeline pipeline;
    if (pipeline.Submit(request) != ResourceStreamingPipelineStatus::Queued) {
        return Fail("pipeline cache payload failure submit did not queue");
    }

    if (file_queue.Shutdown(false) != AsyncFileReadStatus::ShutdownComplete) {
        return Fail("pipeline cache payload failure file queue shutdown failed");
    }

    AsyncFileReadResult file_result;
    if (!DrainOneFileCompletion(file_queue, &file_result)) {
        return Fail("pipeline cache payload failure file completion drain failed");
    }

    const ResourceStreamingPipelineStatus pipeline_status = pipeline.CompleteFileRead(file_result);
    if (pipeline_status != ResourceStreamingPipelineStatus::CachePayloadStoreFailed) {
        return Fail("pipeline cache payload failure did not return explicit pipeline failure");
    }

    const ResourceCachePayloadBridgeResult cache_result = pipeline.LastCachePayloadResult();
    if (cache_result.status != ResourceCachePayloadBridgeStatus::ResourceStoreFailed) {
        return Fail("pipeline cache payload failure bridge status changed");
    }

    if (cache_result.cache_payload_status != ResourceCachePayloadStatus::DuplicatePayloadId) {
        return Fail("pipeline cache payload failure resource status changed");
    }

    if (pipeline.LastUploadCompletion().upload_id != 0U) {
        return Fail("pipeline cache payload failure entered upload stage");
    }

    if (pipeline.LastCommitCompletion().commit_id != 0U) {
        return Fail("pipeline cache payload failure entered commit stage");
    }

    const ResourceStreamingPipelineSnapshot snapshot = pipeline.Snapshot();
    if (snapshot.completed_count != 0U) {
        return Fail("pipeline cache payload failure completed count changed");
    }

    if (snapshot.failed_count != 1U) {
        return Fail("pipeline cache payload failure failed count changed");
    }

    if (snapshot.last_status != ResourceStreamingPipelineStatus::CachePayloadStoreFailed) {
        return Fail("pipeline cache payload failure snapshot status changed");
    }

    if (snapshot.last_cache_payload_status != ResourceCachePayloadBridgeStatus::ResourceStoreFailed) {
        return Fail("pipeline cache payload failure snapshot bridge status changed");
    }

    if (snapshot.last_cache_payload_resource_status != ResourceCachePayloadStatus::DuplicatePayloadId) {
        return Fail("pipeline cache payload failure snapshot resource status changed");
    }

    if (snapshot.has_active_request) {
        return Fail("pipeline cache payload failure active request was not cleared");
    }

    const ResourceCachePayloadSnapshot after_cache_snapshot = resource_registry.CachePayloadSnapshot();
    if (after_cache_snapshot.cached_payload_count != before_cache_snapshot.cached_payload_count) {
        return Fail("pipeline cache payload failure changed cached payload count");
    }

    if (after_cache_snapshot.cached_byte_count != before_cache_snapshot.cached_byte_count) {
        return Fail("pipeline cache payload failure changed cached byte count");
    }

    if (after_cache_snapshot.cache_payload_record_count != before_cache_snapshot.cache_payload_record_count) {
        return Fail("pipeline cache payload failure changed cache payload record count");
    }

    std::array<std::uint8_t, 8U> output_bytes{};
    std::uint32_t output_byte_count = 0U;
    const ResourceCachePayloadRequest read_request = BuildCachePayloadReadRequest(
        resource_result.handle,
        TYPE_TEXTURE,
        CACHE_PAYLOAD_ONE,
        U64_PAYLOAD_LOGICAL_BYTE_COUNT,
        PAYLOAD_WINDOW_OFFSET,
        ARCHIVE_WINDOW_SIZE);
    const ResourceCachePayloadStatus read_status = resource_registry.ReadCachePayload(
        read_request,
        output_bytes.data(),
        static_cast<std::uint32_t>(output_bytes.size()),
        &output_byte_count);
    if (read_status != ResourceCachePayloadStatus::Success) {
        return Fail("pipeline cache payload failure existing payload read failed");
    }

    if (output_byte_count != static_cast<std::uint32_t>(existing_payload_bytes.size())) {
        return Fail("pipeline cache payload failure existing payload byte count changed");
    }

    if (output_bytes != existing_payload_bytes) {
        return Fail("pipeline cache payload failure mutated existing payload bytes");
    }

    return 0;
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_SUBMIT_COMPLETE, StreamingPackageResourceStagingSubmitsAndCompletesFixtureRead},
        {TEST_INVALID_HANDLE, StreamingPackageResourceStagingRejectsInvalidResourceHandleWithoutMutation},
        {TEST_TYPE_MISMATCH, StreamingPackageResourceStagingRejectsResourceTypeMismatchWithoutMutation},
        {TEST_ARCHIVE_RANGE, StreamingPackageResourceStagingUsesArchiveByteRangeForFileRead},
        {TEST_LEGACY_MIRROR, StreamingPackageResourceStagingPreservesLegacyMirrorAsMetadataOnly},
        {TEST_OUTPUT_CAPACITY, StreamingPackageResourceStagingRejectsArchiveByteSizeOverOutputCapacity},
        {TEST_BYTE_RANGE, StreamingPackageResourceStagingRejectsByteRangeOverflowWithoutMutation},
        {TEST_FILE_BYTE_MISMATCH, StreamingPackageResourceStagingReportsFileByteCountMismatch},
        {TEST_MISSING_COMPLETION, StreamingPackageResourceStagingReportsMissingFileCompletion},
        {TEST_QUEUE_OVERFLOW, StreamingPackageResourceStagingRejectsQueueOverflowWithoutMutation},
        {TEST_COMPLETION_OVERFLOW, StreamingPackageResourceStagingReportsCompletionOverflowWithoutDroppingPending},
        {TEST_DUPLICATE_REQUEST, StreamingPackageResourceStagingRejectsDuplicateRequestId},
        {TEST_SNAPSHOT, StreamingPackageResourceStagingSnapshotReportsBoundedCounters},
        {TEST_NO_UPPER_DEPENDENCY, StreamingPackageResourceStagingNoUpperRuntimeDependency},
        {TEST_CACHE_PAYLOAD_BRIDGE_STORE, StreamingResourceCachePayloadBridgeStoresPackageWindowWithU64LogicalTotal},
        {TEST_CACHE_PAYLOAD_BRIDGE_OFFSET, StreamingResourceCachePayloadBridgeSeparatesArchiveAndPayloadOffsets},
        {TEST_CACHE_PAYLOAD_BRIDGE_FAILED_STAGING,
         StreamingResourceCachePayloadBridgeRejectsFailedStagingWithoutResourceMutation},
        {TEST_CACHE_PAYLOAD_BRIDGE_WINDOW_OUT_OF_BOUNDS,
         StreamingResourceCachePayloadBridgeRejectsPayloadWindowOutOfBoundsBeforeStore},
        {TEST_CACHE_PAYLOAD_BRIDGE_BYTE_MISMATCH,
         StreamingResourceCachePayloadBridgeRejectsLocalByteCountMismatchBeforeStore},
        {TEST_UPLOAD_CREATE_BUFFER, StreamingResourceUploadCreateBufferFromStagingCompletion},
        {TEST_UPLOAD_UPDATE_BUFFER, StreamingResourceUploadUpdateBufferSignalsFence},
        {TEST_UPLOAD_UPDATE_BUFFER_DESTINATION_RANGE,
         StreamingResourceUploadUpdateBufferUsesDestinationOffsetAndRejectsOverflow},
        {TEST_UPLOAD_CREATE_TEXTURE, StreamingResourceUploadCreateTextureFromStagingCompletion},
        {TEST_UPLOAD_UPDATE_TEXTURE, StreamingResourceUploadUpdateTextureSignalsFence},
        {TEST_UPLOAD_UPDATE_TEXTURE_DESTINATION_RANGE,
         StreamingResourceUploadUpdateTextureUsesDestinationOffsetAndRejectsOverflow},
        {TEST_UPLOAD_INVALID_HANDLE, StreamingResourceUploadRejectsInvalidResourceHandleWithoutMutation},
        {TEST_UPLOAD_TYPE_MISMATCH, StreamingResourceUploadRejectsResourceTypeMismatchWithoutMutation},
        {TEST_UPLOAD_FAILED_STAGING, StreamingResourceUploadRejectsFailedStagingCompletion},
        {TEST_UPLOAD_EMPTY_BYTES, StreamingResourceUploadRejectsEmptyUploadBytes},
        {TEST_UPLOAD_BYTE_RANGE, StreamingResourceUploadRejectsByteRangeOverflow},
        {TEST_UPLOAD_NULL_DEVICE, StreamingResourceUploadRejectsNullRhiDevice},
        {TEST_UPLOAD_NULL_OUTPUT, StreamingResourceUploadRejectsNullOutputStorage},
        {TEST_UPLOAD_UNSUPPORTED_KIND, StreamingResourceUploadRejectsUnsupportedUploadKind},
        {TEST_UPLOAD_QUEUE_OVERFLOW, StreamingResourceUploadRejectsQueueOverflowWithoutMutation},
        {TEST_UPLOAD_COMPLETION_OVERFLOW, StreamingResourceUploadReportsCompletionOverflowWithoutProcessingPending},
        {TEST_UPLOAD_DUPLICATE_ID, StreamingResourceUploadRejectsDuplicateUploadId},
        {TEST_UPLOAD_SNAPSHOT, StreamingResourceUploadSnapshotReportsBoundedCounters},
        {TEST_UPLOAD_RHI_FAILURE, StreamingResourceUploadReportsRhiFailureWithoutWritingOutput},
        {TEST_UPLOAD_COMMIT_SUCCESS, StreamingResourceUploadCommitCommitsSuccessfulUpload},
        {TEST_UPLOAD_COMMIT_FAILED_UPLOAD, StreamingResourceUploadCommitCommitsFailedUpload},
        {TEST_UPLOAD_COMMIT_INVALID_HANDLE, StreamingResourceUploadCommitRejectsInvalidResourceHandleWithoutMutation},
        {TEST_UPLOAD_COMMIT_TYPE_MISMATCH, StreamingResourceUploadCommitRejectsTypeMismatchWithoutMutation},
        {TEST_UPLOAD_COMMIT_DUPLICATE_ID, StreamingResourceUploadCommitRejectsDuplicateCommitId},
        {TEST_UPLOAD_COMMIT_QUEUE_OVERFLOW, StreamingResourceUploadCommitRejectsQueueOverflowWithoutMutation},
        {TEST_UPLOAD_COMMIT_COMPLETION_OVERFLOW,
         StreamingResourceUploadCommitReportsCompletionOverflowWithoutProcessingPending},
        {TEST_UPLOAD_COMMIT_SLOT_REUSE_ORDER, StreamingResourceUploadCommitPreservesOldestOrderAfterSlotReuse},
        {TEST_UPLOAD_COMMIT_SNAPSHOT, StreamingResourceUploadCommitSnapshotReportsBoundedCounters},
        {TEST_PIPELINE_FIXTURE_BUFFER, StreamingResourceStreamingPipelineFixtureBufferReadUploadCommit},
        {TEST_PIPELINE_UPDATE_BUFFER_DESTINATION_RANGE,
         StreamingResourceStreamingPipelineUpdateBufferUsesPayloadWindowDestinationOffset},
        {TEST_PIPELINE_CACHE_PAYLOAD, StreamingResourceStreamingPipelineCachePayloadOnlyStoresPackageWindow},
        {TEST_PIPELINE_PACKAGE_ARTIFACT_CACHE_PAYLOAD,
         StreamingResourceStreamingPipelinePackageArtifactFeedsCachePayloadU64Window},
        {TEST_PIPELINE_CACHE_PAYLOAD_STORE_FAILURE,
         StreamingResourceStreamingPipelineCachePayloadRejectsStoreFailureWithoutMutation}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
