#pragma once

#include "yuengine/file/mount_id.h"
#include "yuengine/file/virtual_path.h"

namespace yuengine::file {
struct file_read_request_t {
    MountId Mount;
    VirtualPath Path;
};
}
