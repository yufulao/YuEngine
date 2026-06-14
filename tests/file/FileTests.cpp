#include <filesystem>
#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "yuengine/file/FileConstants.h"
#include "yuengine/file/LooseFileSource.h"
#include "yuengine/file/MountTable.h"
#include "yuengine/file/NormalizedPath.h"
#include "yuengine/memory/MemoryAccountingStatus.h"

using FileStatus = yuengine::file::FileStatus;
using LooseFileSource = yuengine::file::LooseFileSource;
using MemoryAccountingStatus = yuengine::memory::MemoryAccountingStatus;
using MountId = yuengine::file::MountId;
using MountTable = yuengine::file::MountTable;
using NormalizedPath = yuengine::file::NormalizedPath;
using VirtualPath = yuengine::file::VirtualPath;
using yuengine::file::MAX_VIRTUAL_PATH_LENGTH;

namespace
{
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
constexpr const char* ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char* ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr const char* PRIMARY_MOUNT = "primary";
constexpr const char* SECONDARY_MOUNT = "secondary";
constexpr const char* THIRD_MOUNT = "third";
constexpr const char* FOURTH_MOUNT = "fourth";
constexpr const char* OVERFLOW_MOUNT = "overflow";
constexpr const char* MISSING_MOUNT = "missing";
constexpr const char* NORMALIZED_PATH = "nested/fixture.txt";
constexpr const char* FIXTURE_TEXT = "yuengine file fixture\n";
constexpr const char* MISSING_PATH = "missing.txt";
using TestFunction = int (*)();

std::filesystem::path FixtureRoot()
{
    return std::filesystem::path(YUENGINE_FILE_FIXTURE_ROOT);
}

int Fail(const std::string& message)
{
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

MountTable CreateMountedTable()
{
    MountTable table;
    const FileStatus primaryStatus = table.RegisterLooseMount(MountId(PRIMARY_MOUNT), FixtureRoot() / "primary");
    if (primaryStatus != FileStatus::Success)
    {
        return table;
    }

    const FileStatus secondaryStatus = table.RegisterLooseMount(MountId(SECONDARY_MOUNT), FixtureRoot() / "secondary");
    if (secondaryStatus != FileStatus::Success)
    {
        return table;
    }

    return table;
}

int FilePathNormalizeRemovesDotAndRepeatedSeparators()
{
    MountTable table;
    const auto result = table.Normalize(VirtualPath("nested//./fixture.txt"));
    if (!result.Succeeded())
    {
        return Fail("normal path did not normalize successfully");
    }

    if (result.Path.Value() != NORMALIZED_PATH)
    {
        return Fail("normalized path removed the wrong segments");
    }

    const auto snapshot = table.Snapshot();
    if (snapshot.PathNormalizationCount != 1U)
    {
        return Fail("normalization count was not recorded");
    }

    if (snapshot.RejectedPathCount != 0U)
    {
        return Fail("valid path was counted as rejected");
    }

    return 0;
}

int FilePathNormalizeRejectsTraversalOutsideRoot()
{
    MountTable table;
    const auto result = table.Normalize(VirtualPath("../fixture.txt"));
    if (result.Status != FileStatus::PathEscape)
    {
        return Fail("traversal outside root was not rejected");
    }

    const auto snapshot = table.Snapshot();
    if (snapshot.RejectedPathCount != 1U)
    {
        return Fail("path escape rejection was not counted");
    }

    return 0;
}

int FilePathNormalizeRejectsEmptyAndAbsolutePath()
{
    MountTable table;
    const auto emptyResult = table.Normalize(VirtualPath(""));
    if (emptyResult.Status != FileStatus::InvalidPath)
    {
        return Fail("empty path was not rejected");
    }

    const auto absoluteResult = table.Normalize(VirtualPath("/absolute/path.txt"));
    if (absoluteResult.Status != FileStatus::InvalidPath)
    {
        return Fail("absolute path was not rejected");
    }

    const auto driveResult = table.Normalize(VirtualPath("C:/absolute/path.txt"));
    if (driveResult.Status != FileStatus::InvalidPath)
    {
        return Fail("drive absolute path was not rejected");
    }

    const std::string longPath(MAX_VIRTUAL_PATH_LENGTH + 1U, 'a');
    const auto longPathResult = table.Normalize(VirtualPath(longPath));
    if (longPathResult.Status != FileStatus::PathTooLong)
    {
        return Fail("overlong path did not return bounds status");
    }

    const auto snapshot = table.Snapshot();
    if (snapshot.RejectedPathCount != 4U)
    {
        return Fail("invalid path rejection count was wrong");
    }

    return 0;
}

int FileMountTableRejectsDuplicateMount()
{
    MountTable table;
    const FileStatus firstStatus = table.RegisterLooseMount(MountId(PRIMARY_MOUNT), FixtureRoot() / "primary");
    if (firstStatus != FileStatus::Success)
    {
        return Fail("first mount registration failed");
    }

    const FileStatus duplicateStatus = table.RegisterLooseMount(MountId(PRIMARY_MOUNT), FixtureRoot() / "secondary");
    if (duplicateStatus != FileStatus::DuplicateMount)
    {
        return Fail("duplicate mount was not rejected");
    }

    const auto snapshot = table.Snapshot();
    if (snapshot.MountCount != 1U)
    {
        return Fail("duplicate mount changed mount count");
    }

    const FileStatus secondStatus = table.RegisterLooseMount(MountId(SECONDARY_MOUNT), FixtureRoot() / "secondary");
    const FileStatus thirdStatus = table.RegisterLooseMount(MountId(THIRD_MOUNT), FixtureRoot() / "secondary");
    const FileStatus fourthStatus = table.RegisterLooseMount(MountId(FOURTH_MOUNT), FixtureRoot() / "secondary");
    if (secondStatus != FileStatus::Success)
    {
        return Fail("second mount registration failed");
    }

    if (thirdStatus != FileStatus::Success)
    {
        return Fail("third mount registration failed");
    }

    if (fourthStatus != FileStatus::Success)
    {
        return Fail("fourth mount registration failed");
    }

    const FileStatus overflowStatus = table.RegisterLooseMount(MountId(OVERFLOW_MOUNT), FixtureRoot() / "secondary");
    if (overflowStatus != FileStatus::MountTableFull)
    {
        return Fail("mount table did not enforce capacity");
    }

    return 0;
}

int FileMountTableUsesDeterministicPriorityOrder()
{
    MountTable table = CreateMountedTable();
    const std::vector<MountId> order = table.MountOrder();
    if (order.size() != 2U)
    {
        return Fail("mount order size was wrong");
    }

    if (order[0U].Value() != PRIMARY_MOUNT)
    {
        return Fail("primary mount was not first");
    }

    if (order[1U].Value() != SECONDARY_MOUNT)
    {
        return Fail("secondary mount was not second");
    }

    return 0;
}

int FileMountTableReportsMissingMountOrFile()
{
    MountTable table = CreateMountedTable();
    const auto missingMount = table.Read({MountId(MISSING_MOUNT), VirtualPath(NORMALIZED_PATH)});
    if (missingMount.Status != FileStatus::MountNotFound)
    {
        return Fail("missing mount did not return explicit status");
    }

    const auto missingFile = table.Read({MountId(PRIMARY_MOUNT), VirtualPath(MISSING_PATH)});
    if (missingFile.Status != FileStatus::FileNotFound)
    {
        return Fail("missing file did not return explicit status");
    }

    return 0;
}

int FileLooseFixtureReadReturnsExactBytes()
{
    MountTable table = CreateMountedTable();
    const auto result = table.Read({MountId(PRIMARY_MOUNT), VirtualPath(NORMALIZED_PATH)});
    if (!result.Succeeded())
    {
        return Fail("fixture read failed");
    }

    const std::string text(result.Bytes.begin(), result.Bytes.end());
    if (text != FIXTURE_TEXT)
    {
        return Fail("fixture bytes did not match expected content");
    }

    return 0;
}

int FileLooseFileSourceRejectsForgedNormalizedPathEscape()
{
    LooseFileSource source(FixtureRoot() / "secondary");
    const auto traversalResult = source.Read(NormalizedPath("../primary/nested/fixture.txt"));
    if (traversalResult.Status != FileStatus::PathEscape)
    {
        return Fail("forged traversal normalized path did not return path escape status");
    }

    const std::filesystem::path absoluteFixturePath = FixtureRoot() / "primary" / "nested" / "fixture.txt";
    const auto absoluteResult = source.Read(NormalizedPath(absoluteFixturePath.generic_string()));
    if (absoluteResult.Status != FileStatus::InvalidPath)
    {
        return Fail("forged absolute normalized path did not return invalid path status");
    }

    return 0;
}

int FileReadSnapshotRecordsCountsAndBytes()
{
    MountTable table = CreateMountedTable();
    const auto result = table.Read({MountId(PRIMARY_MOUNT), VirtualPath("nested//./fixture.txt")});
    if (!result.Succeeded())
    {
        return Fail("snapshot fixture read failed");
    }

    const auto snapshot = table.Snapshot();
    if (snapshot.PathNormalizationCount != 1U)
    {
        return Fail("snapshot did not record normalization count");
    }

    if (snapshot.LookupCount != 1U)
    {
        return Fail("snapshot did not record lookup count");
    }

    if (snapshot.ReadByteCount != result.Bytes.size())
    {
        return Fail("snapshot did not record read byte count");
    }

    if (snapshot.MaxFixturePathLength != std::string(NORMALIZED_PATH).size())
    {
        return Fail("snapshot did not record max fixture path length");
    }

    if (snapshot.AllocationAccountingStatus != MemoryAccountingStatus::ExplicitlyTrackedOnly)
    {
        return Fail("snapshot did not use YuMemory allocation accounting vocabulary");
    }

    if (snapshot.LastReadStatus != FileStatus::Success)
    {
        return Fail("snapshot did not record sync read status");
    }

    return 0;
}

int FileDiagnosticsDisabledDoesNotChangeBehavior()
{
    MountTable recordingTable = CreateMountedTable();
    MountTable diagnosticsDisabledTable = CreateMountedTable();

    const auto recordingResult = recordingTable.Read({MountId(PRIMARY_MOUNT), VirtualPath(NORMALIZED_PATH)});
    const auto disabledResult = diagnosticsDisabledTable.Read({MountId(PRIMARY_MOUNT), VirtualPath(NORMALIZED_PATH)});
    if (recordingResult.Status != disabledResult.Status)
    {
        return Fail("disabled diagnostics changed read status");
    }

    if (recordingResult.Bytes != disabledResult.Bytes)
    {
        return Fail("disabled diagnostics changed read bytes");
    }

    if (recordingTable.Snapshot().ReadByteCount != diagnosticsDisabledTable.Snapshot().ReadByteCount)
    {
        return Fail("disabled diagnostics changed read byte count");
    }

    return 0;
}
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    static const std::unordered_map<std::string_view, TestFunction> testRegistry{
        {TEST_NORMALIZE, FilePathNormalizeRemovesDotAndRepeatedSeparators},
        {TEST_TRAVERSAL, FilePathNormalizeRejectsTraversalOutsideRoot},
        {TEST_EMPTY_ABSOLUTE, FilePathNormalizeRejectsEmptyAndAbsolutePath},
        {TEST_DUPLICATE_MOUNT, FileMountTableRejectsDuplicateMount},
        {TEST_PRIORITY_ORDER, FileMountTableUsesDeterministicPriorityOrder},
        {TEST_MISSING, FileMountTableReportsMissingMountOrFile},
        {TEST_READ, FileLooseFixtureReadReturnsExactBytes},
        {TEST_FORGED_NORMALIZED_PATH, FileLooseFileSourceRejectsForgedNormalizedPathEscape},
        {TEST_SNAPSHOT, FileReadSnapshotRecordsCountsAndBytes},
        {TEST_DISABLED_DIAGNOSTICS, FileDiagnosticsDisabledDoesNotChangeBehavior}};

    const std::string_view testName(argv[1]);
    const auto testIterator = testRegistry.find(testName);
    if (testIterator == testRegistry.end())
    {
        return Fail(ERROR_UNKNOWN_TEST_NAME);
    }

    return testIterator->second();
}
