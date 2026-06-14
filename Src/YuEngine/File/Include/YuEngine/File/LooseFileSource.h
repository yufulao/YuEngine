#pragma once

#include <filesystem>

#include "YuEngine/File/FileReadResult.h"
#include "YuEngine/File/NormalizedPath.h"

namespace yuengine::file {
class LooseFileSource final {
public:
    LooseFileSource();
    explicit LooseFileSource(std::filesystem::path rootPath);

    FileReadResult Read(NormalizedPath path) const;
    const std::filesystem::path& RootPath() const;

private:
    std::filesystem::path root_path_;
};
}
