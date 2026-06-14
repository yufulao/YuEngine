#pragma once

#include "yuengine/file/MountId.h"
#include "yuengine/file/VirtualPath.h"

namespace yuengine::file {
struct FileReadRequest {
    MountId Mount;
    VirtualPath Path;
};
}
