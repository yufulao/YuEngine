// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiRectMath.cpp

#include "YuEngine/UiCore/UiRectMath.h"

namespace yuengine::uicore {
namespace {
bool IsUnitRange(float value) {
    if (value < 0.0F) {
        return false;
    }

    return value <= 1.0F;
}

bool IsNonNegative(float value) {
    return value >= 0.0F;
}

UiRectMathResult Failure(UiRectMathStatus status) {
    UiRectMathResult result;
    result.status = status;
    return result;
}
}

UiRectMathResult UiRectMath::Resolve(const UiRect &parent_rect, const UiRectTransform &transform) {
    if (!IsNonNegative(parent_rect.width) || !IsNonNegative(parent_rect.height)) {
        return Failure(UiRectMathStatus::InvalidParentRect);
    }

    if (!IsUnitRange(transform.anchor_min.x) || !IsUnitRange(transform.anchor_min.y)) {
        return Failure(UiRectMathStatus::InvalidAnchor);
    }

    if (!IsUnitRange(transform.anchor_max.x) || !IsUnitRange(transform.anchor_max.y)) {
        return Failure(UiRectMathStatus::InvalidAnchor);
    }

    if ((transform.anchor_min.x > transform.anchor_max.x) || (transform.anchor_min.y > transform.anchor_max.y)) {
        return Failure(UiRectMathStatus::InvalidAnchor);
    }

    if (!IsUnitRange(transform.pivot.x) || !IsUnitRange(transform.pivot.y)) {
        return Failure(UiRectMathStatus::InvalidPivot);
    }

    if (transform.dpi_scale <= 0.0F) {
        return Failure(UiRectMathStatus::InvalidDpiScale);
    }

    const UiThickness &margin = transform.margin;
    if (!IsNonNegative(margin.left) || !IsNonNegative(margin.top) ||
        !IsNonNegative(margin.right) || !IsNonNegative(margin.bottom)) {
        return Failure(UiRectMathStatus::InvalidMargin);
    }

    const UiThickness &padding = transform.padding;
    if (!IsNonNegative(padding.left) || !IsNonNegative(padding.top) ||
        !IsNonNegative(padding.right) || !IsNonNegative(padding.bottom)) {
        return Failure(UiRectMathStatus::InvalidPadding);
    }

    const float dpi_scale = transform.dpi_scale;
    const float left_anchor = parent_rect.x + (parent_rect.width * transform.anchor_min.x);
    const float bottom_anchor = parent_rect.y + (parent_rect.height * transform.anchor_min.y);
    const float right_anchor = parent_rect.x + (parent_rect.width * transform.anchor_max.x);
    const float top_anchor = parent_rect.y + (parent_rect.height * transform.anchor_max.y);
    const float left = left_anchor + (transform.offset_min.x * dpi_scale) + (margin.left * dpi_scale);
    const float bottom = bottom_anchor + (transform.offset_min.y * dpi_scale) + (margin.bottom * dpi_scale);
    const float right = right_anchor + (transform.offset_max.x * dpi_scale) - (margin.right * dpi_scale);
    const float top = top_anchor + (transform.offset_max.y * dpi_scale) - (margin.top * dpi_scale);
    const float width = right - left;
    const float height = top - bottom;
    if ((width < 0.0F) || (height < 0.0F)) {
        return Failure(UiRectMathStatus::InvalidOutputRect);
    }

    const float content_left = left + (padding.left * dpi_scale);
    const float content_bottom = bottom + (padding.bottom * dpi_scale);
    const float content_width = width - ((padding.left + padding.right) * dpi_scale);
    const float content_height = height - ((padding.top + padding.bottom) * dpi_scale);
    if ((content_width < 0.0F) || (content_height < 0.0F)) {
        return Failure(UiRectMathStatus::InvalidPadding);
    }

    UiRectMathResult result;
    result.status = UiRectMathStatus::Success;
    result.rect = UiRect{left, bottom, width, height};
    result.content_rect = UiRect{content_left, content_bottom, content_width, content_height};
    result.pivot_point = UiVector2{
        left + (width * transform.pivot.x),
        bottom + (height * transform.pivot.y)};
    return result;
}
}
