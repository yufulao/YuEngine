// 模块: YuEngine File
// 文件: Src/YuEngine/File/Src/MountId.cpp

#include "YuEngine/File/MountId.h"

#include <utility>

namespace yuengine::file {
MountId::MountId()
    : value_() {
}

MountId::MountId(std::string value)
    : value_(std::move(value)) {
}

std::string_view MountId::Value() const {
    return value_;
}

bool MountId::IsValid() const {
    return !value_.empty();
}
}
