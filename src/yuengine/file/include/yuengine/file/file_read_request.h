#pragma once

#include "yuengine/file/mount_id.h"
#include "yuengine/file/virtual_path.h"

namespace yuengine::file {
struct FileReadRequest {
    MountId Mount;
    VirtualPath Path;
};
}
