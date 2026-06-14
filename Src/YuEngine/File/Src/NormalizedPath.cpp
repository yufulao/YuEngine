#include "YuEngine/File/NormalizedPath.h"

#include <utility>

namespace yuengine::file {
NormalizedPath::NormalizedPath()
    : value_() {
}

NormalizedPath::NormalizedPath(std::string value)
    : value_(std::move(value)) {
}

std::string_view NormalizedPath::Value() const {
    return value_;
}

std::size_t NormalizedPath::ByteLength() const {
    return value_.size();
}

bool NormalizedPath::IsValid() const {
    return !value_.empty();
}
}
