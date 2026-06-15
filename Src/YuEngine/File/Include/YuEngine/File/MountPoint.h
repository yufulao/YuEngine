// Module: YuEngine File
// File: Src/YuEngine/File/Include/YuEngine/File/MountPoint.h

#pragma once

#include "YuEngine/File/LooseFileSource.h"
#include "YuEngine/File/MountId.h"

namespace yuengine::file {
class MountPoint final {
public:
    /**
     * @comment Constructs a MountPoint instance.
     */
    MountPoint();
    /**
     * @comment Constructs a MountPoint instance.
     * @param id Input id.
     * @param source Input source.
     */
    MountPoint(MountId id, LooseFileSource source);

    /**
     * @comment Returns the identifier.
     * @return Reference to the requested object.
     */
    const MountId& Id() const;
    /**
     * @comment Returns the file source.
     * @return Reference to the requested object.
     */
    const LooseFileSource& Source() const;

private:
    MountId id_;
    LooseFileSource source_;
};
}
