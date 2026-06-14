#include "YuEngine/File/VirtualPath.h"

#include <utility>

namespace yuengine::file {
VirtualPath::VirtualPath()
    : _value() {
}

VirtualPath::VirtualPath(std::string value)
    : _value(std::move(value)) {
}

std::string_view VirtualPath::Value() const {
    return _value;
}

std::size_t VirtualPath::ByteLength() const {
    return _value.size();
}
}
