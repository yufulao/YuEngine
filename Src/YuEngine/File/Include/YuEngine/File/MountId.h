// Module: YuEngine File
// File: Src/YuEngine/File/Include/YuEngine/File/MountId.h

#pragma once

#include <string>
#include <string_view>

namespace yuengine::file {
class MountId final {
public:
    /**
     * @comment Constructs a MountId instance.
     */
    MountId();
    /**
     * @comment Constructs a MountId instance.
     * @param value Input value.
     */
    explicit MountId(std::string value);

    /**
     * @comment Returns the stored value.
     * @return Value value.
     */
    std::string_view Value() const;
    /**
     * @comment Checks whether the value is valid.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool IsValid() const;

private:
    std::string value_;
};
}
