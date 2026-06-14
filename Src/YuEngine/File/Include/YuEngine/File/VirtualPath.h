#pragma once

#include <string>
#include <string_view>

namespace yuengine::file {
class VirtualPath final {
public:
    VirtualPath();
    explicit VirtualPath(std::string value);

    std::string_view Value() const;
    std::size_t ByteLength() const;

private:
    std::string _value;
};
}
