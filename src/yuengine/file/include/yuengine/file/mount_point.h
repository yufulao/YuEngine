#pragma once

#include "yuengine/file/loose_file_source.h"
#include "yuengine/file/mount_id.h"

namespace yuengine::file {
class MountPoint final {
public:
    MountPoint();
    MountPoint(MountId id, LooseFileSource source);

    const MountId& Id() const;
    const LooseFileSource& Source() const;

private:
    MountId _id;
    LooseFileSource _source;
};
}
