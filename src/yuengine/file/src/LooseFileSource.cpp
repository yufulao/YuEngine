#include "yuengine/file/LooseFileSource.h"

#include <fstream>
#include <utility>

#include "yuengine/file/FileConstants.h"

namespace yuengine::file
{
LooseFileSource::LooseFileSource()
    : _rootPath()
{
}

LooseFileSource::LooseFileSource(std::filesystem::path rootPath)
    : _rootPath(std::move(rootPath))
{
}

FileReadResult LooseFileSource::Read(NormalizedPath path) const
{
    const std::filesystem::path resolvedPath = _rootPath / std::filesystem::path(std::string(path.Value()));
    if (!std::filesystem::exists(resolvedPath))
    {
        return FileReadResult::Failure(FileStatus::FileNotFound);
    }

    if (!std::filesystem::is_regular_file(resolvedPath))
    {
        return FileReadResult::Failure(FileStatus::ReadFailure);
    }

    const auto fileSize = std::filesystem::file_size(resolvedPath);
    if (fileSize > MAX_FIXTURE_READ_SIZE)
    {
        return FileReadResult::Failure(FileStatus::ReadTooLarge);
    }

    std::ifstream file(resolvedPath, std::ios::binary);
    if (!file.is_open())
    {
        return FileReadResult::Failure(FileStatus::ReadFailure);
    }

    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(fileSize));
    if (!bytes.empty())
    {
        file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }

    if (!file.good())
    {
        return FileReadResult::Failure(FileStatus::ReadFailure);
    }

    return FileReadResult::Success(std::move(bytes));
}

const std::filesystem::path& LooseFileSource::RootPath() const
{
    return _rootPath;
}
}
