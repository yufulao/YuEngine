// Module: Tests Streaming
// File: Tests/Streaming/StreamingTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

#include "YuEngine/File/AsyncFileReadQueue.h"
#include "YuEngine/File/MountTable.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"
#include "YuEngine/Package/PackageRegistry.h"
#include "YuEngine/Resource/ResourceDescriptor.h"
#include "YuEngine/Resource/ResourceRegistry.h"
#include "YuEngine/Streaming/PackageResourceStagingQueue.h"

using yuengine::file::AsyncFileReadQueue;
using yuengine::file::AsyncFileReadRequest;
using yuengine::file::AsyncFileReadResult;
using yuengine::file::AsyncFileReadStatus;
using yuengine::file::FileStatus;
using yuengine::file::MountId;
using yuengine::file::MountTable;
using yuengine::file::VirtualPath;
using yuengine::memory::MemoryAccountingStatus;
using yuengine::package::PackageEntryDescriptor;
using yuengine::package::PackageEntryId;
using yuengine::package::PackageId;
using yuengine::package::PackageLoadPlanRecord;
using yuengine::package::PackageLoadPlanResult;
using yuengine::package::PackageRegistry;
using yuengine::package::PackageRegistrationResult;
using yuengine::package::PackageSourceKey;
using yuengine::resource::ResourceDescriptor;
using yuengine::resource::ResourceHandle;
using yuengine::resource::ResourceLogicalKey;
using yuengine::resource::ResourceRegistrationResult;
using yuengine::resource::ResourceRegistry;
using yuengine::resource::ResourceStatus;
using yuengine::resource::ResourceTypeId;
using yuengine::streaming::PackageResourceStagingCompletion;
using yuengine::streaming::PackageResourceStagingQueue;
using yuengine::streaming::PackageResourceStagingQueueDesc;
using yuengine::streaming::PackageResourceStagingRequest;
using yuengine::streaming::PackageResourceStagingSnapshot;
using yuengine::streaming::PackageResourceStagingStatus;

namespace {
constexpr const char *TEST_SUBMIT_COMPLETE =
    "Streaming_PackageResourceStaging_SubmitsAndCompletesFixtureRead";
constexpr const char *TEST_INVALID_HANDLE =
    "Streaming_PackageResourceStaging_RejectsInvalidResourceHandleWithoutMutation";
constexpr const char *TEST_TYPE_MISMATCH =
    "Streaming_PackageResourceStaging_RejectsResourceTypeMismatchWithoutMutation";
constexpr const char *TEST_BYTE_RANGE =
    "Streaming_PackageResourceStaging_RejectsByteRangeOverflowWithoutMutation";
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
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr const char *PRIMARY_MOUNT = "Primary";
constexpr const char *FIXTURE_PATH = "Nested/FixtureFile.txt";
constexpr const char *PACKAGE_SOURCE_KEY = "fixtures/fixture_file.txt";
constexpr const char *FIXTURE_TEXT = "yuengine file fixture\n";
constexpr const char *RESOURCE_KEY = "texture_fixture";
constexpr PackageId PACKAGE_A{1U};
constexpr PackageEntryId ENTRY_TEXTURE{1U};
constexpr ResourceTypeId TYPE_TEXTURE{1U};
constexpr ResourceTypeId TYPE_AUDIO{2U};
constexpr std::uint64_t REQUEST_ONE = 1U;
constexpr std::uint64_t REQUEST_TWO = 2U;
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

MountTable CreateMountedTable() {
    MountTable table;
    table.RegisterLooseMount(MountId(PRIMARY_MOUNT), FixtureRoot() / "Primary");
    return table;
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

ResourceRegistrationResult RegisterResource(ResourceRegistry &registry, ResourceTypeId type = TYPE_TEXTURE) {
    const ResourceDescriptor descriptor{type, ResourceLogicalKey(RESOURCE_KEY), 0U};
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
    record.byte_offset = 0xFFFFFFFFU;
    record.byte_size = 1U;
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
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_SUBMIT_COMPLETE, StreamingPackageResourceStagingSubmitsAndCompletesFixtureRead},
        {TEST_INVALID_HANDLE, StreamingPackageResourceStagingRejectsInvalidResourceHandleWithoutMutation},
        {TEST_TYPE_MISMATCH, StreamingPackageResourceStagingRejectsResourceTypeMismatchWithoutMutation},
        {TEST_BYTE_RANGE, StreamingPackageResourceStagingRejectsByteRangeOverflowWithoutMutation},
        {TEST_MISSING_COMPLETION, StreamingPackageResourceStagingReportsMissingFileCompletion},
        {TEST_QUEUE_OVERFLOW, StreamingPackageResourceStagingRejectsQueueOverflowWithoutMutation},
        {TEST_COMPLETION_OVERFLOW, StreamingPackageResourceStagingReportsCompletionOverflowWithoutDroppingPending},
        {TEST_DUPLICATE_REQUEST, StreamingPackageResourceStagingRejectsDuplicateRequestId},
        {TEST_SNAPSHOT, StreamingPackageResourceStagingSnapshotReportsBoundedCounters},
        {TEST_NO_UPPER_DEPENDENCY, StreamingPackageResourceStagingNoUpperRuntimeDependency}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
