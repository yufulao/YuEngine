#include "yuengine/file/MountId.h"

#include <utility>

namespace yuengine::file
{
MountId::MountId()
    : _value()
{
}

MountId::MountId(std::string value)
    : _value(std::move(value))
{
}

std::string_view MountId::Value() const
{
    return _value;
}

bool MountId::IsValid() const
{
    return !_value.empty();
}
}
