// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/FileReadRequest.h

#pragma once

#include "YuEngine/File/MountId.h"
#include "YuEngine/File/VirtualPath.h"

namespace yuengine::file {
struct FileReadRequest {
    MountId mount;
    VirtualPath path;
};
}
