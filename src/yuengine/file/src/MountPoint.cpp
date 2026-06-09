#include "yuengine/file/MountPoint.h"

#include <utility>

namespace yuengine::file
{
MountPoint::MountPoint()
    : _id(),
      _source()
{
}

MountPoint::MountPoint(MountId id, LooseFileSource source)
    : _id(std::move(id)),
      _source(std::move(source))
{
}

const MountId& MountPoint::Id() const
{
    return _id;
}

const LooseFileSource& MountPoint::Source() const
{
    return _source;
}
}
