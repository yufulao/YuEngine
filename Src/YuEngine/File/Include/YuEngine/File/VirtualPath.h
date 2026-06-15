// Module: YuEngine File
// File: Src/YuEngine/File/Include/YuEngine/File/VirtualPath.h

#pragma once

#include <string>
#include <string_view>

namespace yuengine::file {
class VirtualPath final {
public:
    /**
     * @comment Constructs a VirtualPath instance.
     */
    VirtualPath();
    /**
     * @comment Constructs a VirtualPath instance.
     * @param value Input value.
     */
    explicit VirtualPath(std::string value);

    /**
     * @comment Returns the stored value.
     * @return Value value.
     */
    std::string_view Value() const;
    /**
     * @comment Returns the stored byte length.
     * @return Byte length value.
     */
    std::size_t ByteLength() const;

private:
    std::string value_;
};
}
