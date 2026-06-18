// Module: YuEngine File
// File: Src/YuEngine/File/Include/YuEngine/File/LooseFileSource.h

#pragma once

#include <filesystem>
#include <cstddef>
#include <cstdint>

#include "YuEngine/File/FileReadResult.h"
#include "YuEngine/File/FileWriteResult.h"
#include "YuEngine/File/NormalizedPath.h"

namespace yuengine::file {
class LooseFileSource final {
public:
    /**
     * @comment Constructs a LooseFileSource instance.
     */
    LooseFileSource();
    /**
     * @comment Constructs a LooseFileSource instance.
     * @param root_path Input root path.
     */
    explicit LooseFileSource(std::filesystem::path root_path);

    /**
     * @comment Reads the operation.
     * @param path Input path.
     * @return Explicit operation result.
     */
    FileReadResult Read(NormalizedPath path) const;
    /**
     * @comment Writes bytes to the operation.
     * @param path Input path.
     * @param bytes Input bytes.
     * @param byte_count Input byte count.
     * @return Explicit operation result.
     */
    FileWriteResult Write(NormalizedPath path, const std::uint8_t *bytes, std::size_t byte_count) const;
    /**
     * @comment Returns the root filesystem path.
     * @return Reference to the requested object.
     */
    const std::filesystem::path& RootPath() const;

private:
    std::filesystem::path root_path_;
};
}
