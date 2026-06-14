#pragma once

#include <string>
#include <string_view>

namespace yuengine::file {
class NormalizedPath final {
public:
    NormalizedPath();
    explicit NormalizedPath(std::string value);

    std::string_view Value() const;
    std::size_t ByteLength() const;
    bool IsValid() const;

private:
    std::string value_;
};
}
