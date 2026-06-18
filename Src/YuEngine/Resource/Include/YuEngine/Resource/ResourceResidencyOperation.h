// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceResidencyOperation.h

#pragma once

namespace yuengine::resource {
enum class ResourceResidencyOperation {
    None,
    ConfigureBudget,
    Admit,
    Pin,
    Unpin,
    Evict,
    SelectCandidate
};
}
