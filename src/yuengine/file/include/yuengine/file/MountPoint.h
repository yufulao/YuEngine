#pragma once

#include "yuengine/file/LooseFileSource.h"
#include "yuengine/file/MountId.h"

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
