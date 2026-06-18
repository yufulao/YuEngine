// 模块: YuEngine File
// 文件: Src/YuEngine/File/Include/YuEngine/File/MountPoint.h

#pragma once

#include "YuEngine/File/LooseFileSource.h"
#include "YuEngine/File/MountId.h"

namespace yuengine::file {
class MountPoint final {
public:
    /**
     * @comment 构造 MountPoint 实例。
     */
    MountPoint();
    /**
     * @comment 构造 MountPoint 实例。
     * @param id 输入 id。
     * @param source 输入 source。
     */
    MountPoint(MountId id, LooseFileSource source);

    /**
     * @comment 返回 identifier。
     * @return 请求对象的引用。
     */
    const MountId& Id() const;
    /**
     * @comment 返回 文件来源。
     * @return 请求对象的引用。
     */
    const LooseFileSource& Source() const;

private:
    MountId id_;
    LooseFileSource source_;
};
}
