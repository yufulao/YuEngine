#pragma once

#include <string>
#include <string_view>

namespace yuengine::file
{
class MountId final
{
public:
    MountId();
    explicit MountId(std::string value);

    std::string_view Value() const;
    bool IsValid() const;

private:
    std::string _value;
};
}
