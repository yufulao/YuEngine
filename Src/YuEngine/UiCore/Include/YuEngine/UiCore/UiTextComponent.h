// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiTextComponent.h

#pragma once

#include <span>

#include "YuEngine/UiCore/UiFontGlyphAtlas.h"
#include "YuEngine/UiCore/UiNodeTree.h"
#include "YuEngine/UiCore/UiTextComponentDesc.h"
#include "YuEngine/UiCore/UiTextComponentResult.h"
#include "YuEngine/UiCore/UiTextComponentStatus.h"
#include "YuEngine/UiCore/UiTextDrawRecord.h"

namespace yuengine::uicore {
class UiTextComponent final {
public:
    /**
     * @comment 将 Text component 描述转换成 text draw records。
     * @param tree UI node tree。
     * @param font_atlas font glyph atlas metadata。
     * @param desc Text component 描述。
     * @param out_records 调用方持有的输出 draw record buffer。
     * @param out_result 输出 component result。
     * @return 显式 component 状态。
     */
    UiTextComponentStatus Build(
        const UiNodeTree &tree,
        const UiFontGlyphAtlasDesc &font_atlas,
        const UiTextComponentDesc &desc,
        std::span<UiTextDrawRecord> out_records,
        UiTextComponentResult *out_result) const;

private:
    UiTextComponentStatus ValidateDesc(const UiTextComponentDesc &desc) const;
    UiDirtyChangeType ResolveDirtyChangeType(UiTextChangeReason reason) const;
};
}
