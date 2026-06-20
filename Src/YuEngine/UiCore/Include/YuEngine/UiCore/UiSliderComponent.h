// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiSliderComponent.h

#pragma once

#include "YuEngine/UiCore/UiNodeRecord.h"
#include "YuEngine/UiCore/UiNodeTree.h"
#include "YuEngine/UiCore/UiSliderComponentDesc.h"
#include "YuEngine/UiCore/UiSliderComponentResult.h"
#include "YuEngine/UiCore/UiSliderComponentStatus.h"

namespace yuengine::uicore {
class UiSliderComponent final {
public:
    /**
     * @comment 解析 Slider value、fill、handle、input capture 和 value-change hook。
     * @param tree UI node tree。
     * @param desc Slider component 描述。
     * @param out_result 输出 component result。
     * @return 显式 component 状态。
     */
    UiSliderComponentStatus Build(
        const UiNodeTree &tree,
        const UiSliderComponentDesc &desc,
        UiSliderComponentResult *out_result) const;

private:
    UiSliderComponentStatus ValidateDesc(const UiSliderComponentDesc &desc) const;
    UiSliderComponentStatus ValidateGeometry(
        const UiSliderComponentDesc &desc,
        const UiNodeRecord &track_record,
        const UiNodeRecord &fill_record,
        const UiNodeRecord &handle_record) const;
    bool IsSliderEnabled(const UiSliderComponentDesc &desc, const UiNodeRecord &record) const;
    bool ShouldCapturePointer(const UiSliderComponentDesc &desc) const;
    float ResolveValueFromInput(
        const UiSliderComponentDesc &desc,
        const UiNodeRecord &track_record,
        UiSliderAdjustmentSource *out_source) const;
    UiSliderVisualUpdate BuildVisualUpdate(
        const UiSliderComponentDesc &desc,
        const UiNodeRecord &track_record,
        const UiNodeRecord &handle_record,
        float normalized_value) const;
};
}
