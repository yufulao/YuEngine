// Module: YuEngine File
// File: Src/YuEngine/File/Src/MountPoint.cpp

#include "YuEngine/File/MountPoint.h"

#include <utility>

namespace yuengine::file {
MountPoint::MountPoint()
    : id_(),
      source_() {
}

MountPoint::MountPoint(MountId id, LooseFileSource source)
    : id_(std::move(id)),
      source_(std::move(source)) {
}

const MountId& MountPoint::Id() const {
    return id_;
}

const LooseFileSource& MountPoint::Source() const {
    return source_;
}
}
