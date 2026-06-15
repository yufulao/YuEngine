// Module: YuEngine File
// File: Src/YuEngine/File/Include/YuEngine/File/NormalizedPath.h

#pragma once

#include <string>
#include <string_view>

namespace yuengine::file {
class NormalizedPath final {
public:
    /**
     * @comment Constructs a NormalizedPath instance.
     */
    NormalizedPath();
    /**
     * @comment Constructs a NormalizedPath instance.
     * @param value Input value.
     */
    explicit NormalizedPath(std::string value);

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
    /**
     * @comment Checks whether the value is valid.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool IsValid() const;

private:
    std::string value_;
};
}
