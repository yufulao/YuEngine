#include "YuEngine/File/VirtualPath.h"

#include <utility>

namespace yuengine::file {
VirtualPath::VirtualPath()
    : value_() {
}

VirtualPath::VirtualPath(std::string value)
    : value_(std::move(value)) {
}

std::string_view VirtualPath::Value() const {
    return value_;
}

std::size_t VirtualPath::ByteLength() const {
    return value_.size();
}
}
