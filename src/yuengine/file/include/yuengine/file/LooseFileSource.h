#pragma once

#include <filesystem>

#include "yuengine/file/FileReadResult.h"
#include "yuengine/file/NormalizedPath.h"

namespace yuengine::file
{
class LooseFileSource final
{
public:
    LooseFileSource();
    explicit LooseFileSource(std::filesystem::path rootPath);

    FileReadResult Read(NormalizedPath path) const;
    const std::filesystem::path& RootPath() const;

private:
    std::filesystem::path _rootPath;
};
}
