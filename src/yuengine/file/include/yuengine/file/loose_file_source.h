#pragma once

#include <filesystem>

#include "yuengine/file/file_read_result.h"
#include "yuengine/file/normalized_path.h"

namespace yuengine::file {
class LooseFileSource final {
public:
    LooseFileSource();
    explicit LooseFileSource(std::filesystem::path rootPath);

    FileReadResult Read(NormalizedPath path) const;
    const std::filesystem::path& RootPath() const;

private:
    std::filesystem::path _rootPath;
};
}
