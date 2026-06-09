#include "yuengine/file/NormalizedPath.h"

#include <utility>

namespace yuengine::file
{
NormalizedPath::NormalizedPath()
    : _value()
{
}

NormalizedPath::NormalizedPath(std::string value)
    : _value(std::move(value))
{
}

std::string_view NormalizedPath::Value() const
{
    return _value;
}

std::size_t NormalizedPath::ByteLength() const
{
    return _value.size();
}

bool NormalizedPath::IsValid() const
{
    return !_value.empty();
}
}
