// Module: Tests File
// File: Tests/File/FileTests.cpp

#include <array>
#include <filesystem>
#include <cstdio>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "YuEngine/File/AsyncFileReadQueue.h"
#include "YuEngine/File/FileConstants.h"
#include "YuEngine/File/LooseFileSource.h"
#include "YuEngine/File/MountTable.h"
#include "YuEngine/File/NormalizedPath.h"
#include "YuEngine/Memory/MemoryAccountingStatus.h"

using yuengine::file::FileStatus;
using AsyncFileReadQueue = yuengine::file::AsyncFileReadQueue;
using AsyncFileReadRequest = yuengine::file::AsyncFileReadRequest;
using AsyncFileReadResult = yuengine::file::AsyncFileReadResult;
using yuengine::file::AsyncFileReadStatus;
using LooseFileSource = yuengine::file::LooseFileSource;
using yuengine::memory::MemoryAccountingStatus;
using MountId = yuengine::file::MountId;
using MountTable = yuengine::file::MountTable;
using NormalizedPath = yuengine::file::NormalizedPath;
using VirtualPath = yuengine::file::VirtualPath;
using yuengine::file::MAX_VIRTUAL_PATH_LENGTH;

namespace {
constexpr const char* TEST_NORMALIZE = "File_PathNormalize_RemovesDotAndRepeatedSeparators";
constexpr const char* TEST_TRAVERSAL = "File_PathNormalize_RejectsTraversalOutsideRoot";
constexpr const char* TEST_EMPTY_ABSOLUTE = "File_PathNormalize_RejectsEmptyAndAbsolutePath";
constexpr const char* TEST_DUPLICATE_MOUNT = "File_MountTable_RejectsDuplicateMount";
constexpr const char* TEST_PRIORITY_ORDER = "File_MountTable_UsesDeterministicPriorityOrder";
constexpr const char* TEST_MISSING = "File_MountTable_ReportsMissingMountOrFile";
constexpr const char* TEST_READ = "File_LooseFixtureRead_ReturnsExactBytes";
constexpr const char* TEST_FORGED_NORMALIZED_PATH = "File_LooseFileSourceRejectsForgedNormalizedPathEscape";
constexpr const char* TEST_SNAPSHOT = "File_ReadSnapshot_RecordsCountsAndBytes";
constexpr const char* TEST_DISABLED_DIAGNOSTICS = "File_DiagnosticsDisabled_DoesNotChangeBehavior";
constexpr const char* TEST_ASYNC_READ = "File_AsyncReadQueue_ReadsFixtureIntoCallerStorage";
constexpr const char* TEST_ASYNC_CAPACITY = "File_AsyncReadQueue_RejectsCapacityOverflow";
constexpr const char* TEST_ASYNC_FAILURE = "File_AsyncReadQueue_ReportsReadFailureCompletion";
constexpr const char* TEST_ASYNC_SMALL_OUTPUT = "File_AsyncReadQueue_RejectsSmallOutputWithoutOverrun";
constexpr const char* TEST_ASYNC_SHUTDOWN = "File_AsyncReadQueue_ShutdownRejectsSubmission";
constexpr const char* TEST_ASYNC_SNAPSHOT = "File_AsyncReadQueue_SnapshotReportsBoundedCounters";
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr const char* PRIMARY_MOUNT = "Primary";
constexpr const char* SECONDARY_MOUNT = "Secondary";
constexpr const char* THIRD_MOUNT = "third";
constexpr const char* FOURTH_MOUNT = "fourth";
constexpr const char* OVERFLOW_MOUNT = "overflow";
constexpr const char* MISSING_MOUNT = "missing";
constexpr const char* NORMALIZED_PATH = "Nested/FixtureFile.txt";
constexpr const char* FIXTURE_TEXT = "yuengine file fixture\n";
constexpr const char* MISSING_PATH = "missing.txt";
constexpr std::size_t ASYNC_OUTPUT_CAPACITY = 64U;
using TestFunction = int (*)();

std::filesystem::path FixtureRoot() {
    return std::filesystem::path(YUENGINE_FILE_FIXTURE_ROOT);
}

int Fail(const std::string& message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

MountTable CreateMountedTable() {
    MountTable table;
    const FileStatus primary_status = table.RegisterLooseMount(MountId(PRIMARY_MOUNT), FixtureRoot() / "Primary");
    if (primary_status != FileStatus::Success) {
        return table;
    }

    const FileStatus secondary_status = table.RegisterLooseMount(MountId(SECONDARY_MOUNT), FixtureRoot() / "Secondary");
    if (secondary_status != FileStatus::Success) {
        return table;
    }

    return table;
}

AsyncFileReadRequest CreateAsyncRequest(
    MountTable &table,
    std::uint64_t request_index,
    const char *path,
    std::uint8_t *output_bytes,
    std::size_t output_capacity) {
    AsyncFileReadRequest request;
    request.mount_table = &table;
    request.read_request = {MountId(PRIMARY_MOUNT), VirtualPath(path)};
    request.request_index = request_index;
    request.output_bytes = output_bytes;
    request.output_capacity = output_capacity;
    return request;
}

int FilePathNormalizeRemovesDotAndRepeatedSeparators() {
    MountTable table;
    const auto result = table.Normalize(VirtualPath("Nested//./FixtureFile.txt"));
    if (!result.Succeeded()) {
        return Fail("normal path did not normalize successfully");
    }

    if (result.path.Value() != NORMALIZED_PATH) {
        return Fail("normalized path removed the wrong segments");
    }

    const auto snapshot = table.Snapshot();
    if (snapshot.path_normalization_count != 1U) {
        return Fail("normalization count was not recorded");
    }

    if (snapshot.rejected_path_count != 0U) {
        return Fail("valid path was counted as rejected");
    }

    return 0;
}

int FilePathNormalizeRejectsTraversalOutsideRoot() {
    MountTable table;
    const auto result = table.Normalize(VirtualPath("../FixtureFile.txt"));
    if (result.status != FileStatus::PathEscape) {
        return Fail("traversal outside root was not rejected");
    }

    const auto snapshot = table.Snapshot();
    if (snapshot.rejected_path_count != 1U) {
        return Fail("path escape rejection was not counted");
    }

    return 0;
}

int FilePathNormalizeRejectsEmptyAndAbsolutePath() {
    MountTable table;
    const auto empty_result = table.Normalize(VirtualPath(""));
    if (empty_result.status != FileStatus::InvalidPath) {
        return Fail("empty path was not rejected");
    }

    const auto absolute_result = table.Normalize(VirtualPath("/absolute/path.txt"));
    if (absolute_result.status != FileStatus::InvalidPath) {
        return Fail("absolute path was not rejected");
    }

    const auto drive_result = table.Normalize(VirtualPath("C:/absolute/path.txt"));
    if (drive_result.status != FileStatus::InvalidPath) {
        return Fail("drive absolute path was not rejected");
    }

    const std::string long_path(MAX_VIRTUAL_PATH_LENGTH + 1U, 'a');
    const auto long_path_result = table.Normalize(VirtualPath(long_path));
    if (long_path_result.status != FileStatus::PathTooLong) {
        return Fail("overlong path did not return bounds status");
    }

    const auto snapshot = table.Snapshot();
    if (snapshot.rejected_path_count != 4U) {
        return Fail("invalid path rejection count was wrong");
    }

    return 0;
}

int FileMountTableRejectsDuplicateMount() {
    MountTable table;
    const FileStatus first_status = table.RegisterLooseMount(MountId(PRIMARY_MOUNT), FixtureRoot() / "Primary");
    if (first_status != FileStatus::Success) {
        return Fail("first mount registration failed");
    }

    const FileStatus duplicate_status = table.RegisterLooseMount(MountId(PRIMARY_MOUNT), FixtureRoot() / "Secondary");
    if (duplicate_status != FileStatus::DuplicateMount) {
        return Fail("duplicate mount was not rejected");
    }

    const auto snapshot = table.Snapshot();
    if (snapshot.mount_count != 1U) {
        return Fail("duplicate mount changed mount count");
    }

    const FileStatus second_status = table.RegisterLooseMount(MountId(SECONDARY_MOUNT), FixtureRoot() / "Secondary");
    const FileStatus third_status = table.RegisterLooseMount(MountId(THIRD_MOUNT), FixtureRoot() / "Secondary");
    const FileStatus fourth_status = table.RegisterLooseMount(MountId(FOURTH_MOUNT), FixtureRoot() / "Secondary");
    if (second_status != FileStatus::Success) {
        return Fail("second mount registration failed");
    }

    if (third_status != FileStatus::Success) {
        return Fail("third mount registration failed");
    }

    if (fourth_status != FileStatus::Success) {
        return Fail("fourth mount registration failed");
    }

    const FileStatus overflow_status = table.RegisterLooseMount(MountId(OVERFLOW_MOUNT), FixtureRoot() / "Secondary");
    if (overflow_status != FileStatus::MountTableFull) {
        return Fail("mount table did not enforce capacity");
    }

    return 0;
}

int FileMountTableUsesDeterministicPriorityOrder() {
    MountTable table = CreateMountedTable();
    const std::vector<MountId> order = table.MountOrder();
    if (order.size() != 2U) {
        return Fail("mount order size was wrong");
    }

    if (order[0U].Value() != PRIMARY_MOUNT) {
        return Fail("primary mount was not first");
    }

    if (order[1U].Value() != SECONDARY_MOUNT) {
        return Fail("secondary mount was not second");
    }

    return 0;
}

int FileMountTableReportsMissingMountOrFile() {
    MountTable table = CreateMountedTable();
    const auto missing_mount = table.Read({MountId(MISSING_MOUNT), VirtualPath(NORMALIZED_PATH)});
    if (missing_mount.status != FileStatus::MountNotFound) {
        return Fail("missing mount did not return explicit status");
    }

    const auto missing_file = table.Read({MountId(PRIMARY_MOUNT), VirtualPath(MISSING_PATH)});
    if (missing_file.status != FileStatus::FileNotFound) {
        return Fail("missing file did not return explicit status");
    }

    return 0;
}

int FileLooseFixtureReadReturnsExactBytes() {
    MountTable table = CreateMountedTable();
    const auto result = table.Read({MountId(PRIMARY_MOUNT), VirtualPath(NORMALIZED_PATH)});
    if (!result.Succeeded()) {
        return Fail("fixture read failed");
    }

    const std::string text(result.bytes.begin(), result.bytes.end());
    if (text != FIXTURE_TEXT) {
        return Fail("fixture bytes did not match expected content");
    }

    return 0;
}

int FileLooseFileSourceRejectsForgedNormalizedPathEscape() {
    LooseFileSource source(FixtureRoot() / "Secondary");
    const auto traversal_result = source.Read(NormalizedPath("../Primary/Nested/FixtureFile.txt"));
    if (traversal_result.status != FileStatus::PathEscape) {
        return Fail("forged traversal normalized path did not return path escape status");
    }

    const std::filesystem::path absolute_fixture_path = FixtureRoot() / "Primary" / "Nested" / "FixtureFile.txt";
    const auto absolute_result = source.Read(NormalizedPath(absolute_fixture_path.generic_string()));
    if (absolute_result.status != FileStatus::InvalidPath) {
        return Fail("forged absolute normalized path did not return invalid path status");
    }

    return 0;
}

int FileReadSnapshotRecordsCountsAndBytes() {
    MountTable table = CreateMountedTable();
    const auto result = table.Read({MountId(PRIMARY_MOUNT), VirtualPath("Nested//./FixtureFile.txt")});
    if (!result.Succeeded()) {
        return Fail("snapshot fixture read failed");
    }

    const auto snapshot = table.Snapshot();
    if (snapshot.path_normalization_count != 1U) {
        return Fail("snapshot did not record normalization count");
    }

    if (snapshot.lookup_count != 1U) {
        return Fail("snapshot did not record lookup count");
    }

    if (snapshot.read_byte_count != result.bytes.size()) {
        return Fail("snapshot did not record read byte count");
    }

    if (snapshot.max_fixture_path_length != std::string(NORMALIZED_PATH).size()) {
        return Fail("snapshot did not record max fixture path length");
    }

    if (snapshot.allocation_accounting_status != MemoryAccountingStatus::ExplicitlyTrackedOnly) {
        return Fail("snapshot did not use YuMemory allocation accounting vocabulary");
    }

    if (snapshot.last_read_status != FileStatus::Success) {
        return Fail("snapshot did not record sync read status");
    }

    return 0;
}

int FileDiagnosticsDisabledDoesNotChangeBehavior() {
    MountTable recording_table = CreateMountedTable();
    MountTable diagnostics_disabled_table = CreateMountedTable();

    const auto recording_result = recording_table.Read({MountId(PRIMARY_MOUNT), VirtualPath(NORMALIZED_PATH)});
    const auto disabled_result = diagnostics_disabled_table.Read({MountId(PRIMARY_MOUNT), VirtualPath(NORMALIZED_PATH)});
    if (recording_result.status != disabled_result.status) {
        return Fail("disabled diagnostics changed read status");
    }

    if (recording_result.bytes != disabled_result.bytes) {
        return Fail("disabled diagnostics changed read bytes");
    }

    if (recording_table.Snapshot().read_byte_count != diagnostics_disabled_table.Snapshot().read_byte_count) {
        return Fail("disabled diagnostics changed read byte count");
    }

    return 0;
}

int FileAsyncReadQueueReadsFixtureIntoCallerStorage() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue queue;
    const AsyncFileReadStatus init_status = queue.Initialize(2U, 2U);
    if (init_status != AsyncFileReadStatus::Success) {
        return Fail("async queue initialize failed");
    }

    const AsyncFileReadStatus start_status = queue.Start();
    if (start_status != AsyncFileReadStatus::Success) {
        return Fail("async queue start failed");
    }

    std::array<std::uint8_t, ASYNC_OUTPUT_CAPACITY> output_bytes{};
    AsyncFileReadRequest request = CreateAsyncRequest(
        table,
        1U,
        NORMALIZED_PATH,
        output_bytes.data(),
        output_bytes.size());

    const AsyncFileReadStatus submit_status = queue.Submit(request);
    if (submit_status != AsyncFileReadStatus::Queued) {
        return Fail("async queue did not accept read request");
    }

    const AsyncFileReadStatus shutdown_status = queue.Shutdown(false);
    if (shutdown_status != AsyncFileReadStatus::ShutdownComplete) {
        return Fail("async queue shutdown failed");
    }

    std::array<AsyncFileReadResult, 1U> results{};
    std::size_t written_count = 0U;
    const AsyncFileReadStatus drain_status = queue.DrainCompletions(
        results.data(),
        results.size(),
        &written_count);
    if (drain_status != AsyncFileReadStatus::Success) {
        return Fail("async completion drain failed");
    }

    if (written_count != 1U) {
        return Fail("async completion count was wrong");
    }

    if (results[0U].status != AsyncFileReadStatus::Success) {
        return Fail("async read did not return success");
    }

    const std::string text(output_bytes.begin(), output_bytes.begin() + results[0U].byte_count);
    if (text != FIXTURE_TEXT) {
        return Fail("async read bytes did not match fixture");
    }

    return 0;
}

int FileAsyncReadQueueRejectsCapacityOverflow() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue queue;
    queue.Initialize(1U, 1U);
    queue.Start();

    std::array<std::uint8_t, ASYNC_OUTPUT_CAPACITY> first_output{};
    std::array<std::uint8_t, ASYNC_OUTPUT_CAPACITY> second_output{};
    AsyncFileReadRequest first_request = CreateAsyncRequest(
        table,
        1U,
        NORMALIZED_PATH,
        first_output.data(),
        first_output.size());
    AsyncFileReadRequest second_request = CreateAsyncRequest(
        table,
        2U,
        NORMALIZED_PATH,
        second_output.data(),
        second_output.size());

    const AsyncFileReadStatus first_status = queue.Submit(first_request);
    if (first_status != AsyncFileReadStatus::Queued) {
        return Fail("first async submit failed");
    }

    const AsyncFileReadStatus second_status = queue.Submit(second_request);
    if (second_status != AsyncFileReadStatus::QueueFull) {
        return Fail("async queue capacity overflow was not rejected");
    }

    queue.Shutdown(false);

    const auto snapshot = queue.Snapshot();
    if (snapshot.rejected_count != 1U) {
        return Fail("async queue rejection count was wrong");
    }

    if (snapshot.submitted_count != 1U) {
        return Fail("async queue overflow changed submitted count");
    }

    return 0;
}

int FileAsyncReadQueueReportsReadFailureCompletion() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue queue;
    queue.Initialize(2U, 2U);
    queue.Start();

    std::array<std::uint8_t, ASYNC_OUTPUT_CAPACITY> output_bytes{};
    AsyncFileReadRequest request = CreateAsyncRequest(
        table,
        7U,
        MISSING_PATH,
        output_bytes.data(),
        output_bytes.size());
    queue.Submit(request);
    queue.Shutdown(false);

    std::array<AsyncFileReadResult, 1U> results{};
    std::size_t written_count = 0U;
    queue.DrainCompletions(results.data(), results.size(), &written_count);
    if (written_count != 1U) {
        return Fail("read failure completion count was wrong");
    }

    if (results[0U].status != AsyncFileReadStatus::ReadFailure) {
        return Fail("missing file did not return async read failure");
    }

    if (results[0U].file_status != FileStatus::FileNotFound) {
        return Fail("missing file status was not preserved");
    }

    if (results[0U].request_index != 7U) {
        return Fail("async failure request index was not preserved");
    }

    return 0;
}

int FileAsyncReadQueueRejectsSmallOutputWithoutOverrun() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue queue;
    queue.Initialize(2U, 2U);
    queue.Start();

    std::array<std::uint8_t, 4U> output_bytes{};
    output_bytes[0U] = 11U;
    output_bytes[1U] = 22U;
    output_bytes[2U] = 33U;
    output_bytes[3U] = 44U;

    AsyncFileReadRequest request = CreateAsyncRequest(
        table,
        9U,
        NORMALIZED_PATH,
        output_bytes.data(),
        output_bytes.size());
    queue.Submit(request);
    queue.Shutdown(false);

    std::array<AsyncFileReadResult, 1U> results{};
    std::size_t written_count = 0U;
    queue.DrainCompletions(results.data(), results.size(), &written_count);
    if (written_count != 1U) {
        return Fail("small output completion count was wrong");
    }

    if (results[0U].status != AsyncFileReadStatus::OutputTooSmall) {
        return Fail("small output did not return output-too-small");
    }

    if (output_bytes[0U] != 11U) {
        return Fail("small output buffer was overwritten");
    }

    if (output_bytes[3U] != 44U) {
        return Fail("small output buffer tail was overwritten");
    }

    return 0;
}

int FileAsyncReadQueueShutdownRejectsSubmission() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue queue;
    queue.Initialize(2U, 2U);
    queue.Start();

    const AsyncFileReadStatus shutdown_status = queue.Shutdown(true);
    if (shutdown_status != AsyncFileReadStatus::ShutdownComplete) {
        return Fail("async shutdown failed");
    }

    std::array<std::uint8_t, ASYNC_OUTPUT_CAPACITY> output_bytes{};
    AsyncFileReadRequest request = CreateAsyncRequest(
        table,
        3U,
        NORMALIZED_PATH,
        output_bytes.data(),
        output_bytes.size());

    const AsyncFileReadStatus submit_status = queue.Submit(request);
    if (submit_status != AsyncFileReadStatus::ShutdownRequested) {
        return Fail("async submit after shutdown was not rejected");
    }

    return 0;
}

int FileAsyncReadQueueSnapshotReportsBoundedCounters() {
    MountTable table = CreateMountedTable();
    AsyncFileReadQueue queue;
    queue.Initialize(2U, 2U);
    queue.Start();

    std::array<std::uint8_t, ASYNC_OUTPUT_CAPACITY> output_bytes{};
    AsyncFileReadRequest request = CreateAsyncRequest(
        table,
        4U,
        NORMALIZED_PATH,
        output_bytes.data(),
        output_bytes.size());
    queue.Submit(request);
    queue.Shutdown(false);

    std::array<AsyncFileReadResult, 1U> results{};
    std::size_t written_count = 0U;
    queue.DrainCompletions(results.data(), results.size(), &written_count);

    const auto snapshot = queue.Snapshot();
    if (snapshot.work_capacity != 2U) {
        return Fail("async snapshot work capacity was wrong");
    }

    if (snapshot.completion_capacity != 2U) {
        return Fail("async snapshot completion capacity was wrong");
    }

    if (snapshot.completed_count != 1U) {
        return Fail("async snapshot completed count was wrong");
    }

    if (snapshot.read_byte_count != std::string(FIXTURE_TEXT).size()) {
        return Fail("async snapshot read byte count was wrong");
    }

    if (snapshot.max_queue_depth > snapshot.work_capacity) {
        return Fail("async snapshot max queue depth exceeded capacity");
    }

    if (snapshot.max_completion_depth > snapshot.completion_capacity) {
        return Fail("async snapshot max completion depth exceeded capacity");
    }

    return 0;
}
}

int main(int argc, char** argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    const std::unordered_map<std::string_view, TestFunction> test_registry{
        {TEST_NORMALIZE, FilePathNormalizeRemovesDotAndRepeatedSeparators},
        {TEST_TRAVERSAL, FilePathNormalizeRejectsTraversalOutsideRoot},
        {TEST_EMPTY_ABSOLUTE, FilePathNormalizeRejectsEmptyAndAbsolutePath},
        {TEST_DUPLICATE_MOUNT, FileMountTableRejectsDuplicateMount},
        {TEST_PRIORITY_ORDER, FileMountTableUsesDeterministicPriorityOrder},
        {TEST_MISSING, FileMountTableReportsMissingMountOrFile},
        {TEST_READ, FileLooseFixtureReadReturnsExactBytes},
        {TEST_FORGED_NORMALIZED_PATH, FileLooseFileSourceRejectsForgedNormalizedPathEscape},
        {TEST_SNAPSHOT, FileReadSnapshotRecordsCountsAndBytes},
        {TEST_DISABLED_DIAGNOSTICS, FileDiagnosticsDisabledDoesNotChangeBehavior},
        {TEST_ASYNC_READ, FileAsyncReadQueueReadsFixtureIntoCallerStorage},
        {TEST_ASYNC_CAPACITY, FileAsyncReadQueueRejectsCapacityOverflow},
        {TEST_ASYNC_FAILURE, FileAsyncReadQueueReportsReadFailureCompletion},
        {TEST_ASYNC_SMALL_OUTPUT, FileAsyncReadQueueRejectsSmallOutputWithoutOverrun},
        {TEST_ASYNC_SHUTDOWN, FileAsyncReadQueueShutdownRejectsSubmission},
        {TEST_ASYNC_SNAPSHOT, FileAsyncReadQueueSnapshotReportsBoundedCounters}};

    const std::string_view test_name(argv[1]);
    const auto test_iterator = test_registry.find(test_name);
    if (test_iterator == test_registry.end()) {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return test_iterator->second();
}
