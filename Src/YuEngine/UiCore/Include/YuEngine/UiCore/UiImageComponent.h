// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiImageComponent.h

#pragma once

#include <span>

#include "YuEngine/UiCore/UiImageComponentDesc.h"
#include "YuEngine/UiCore/UiImageComponentResult.h"
#include "YuEngine/UiCore/UiImageComponentStatus.h"
#include "YuEngine/UiCore/UiImageDrawRecord.h"
#include "YuEngine/UiCore/UiNodeRecord.h"
#include "YuEngine/UiCore/UiNodeTree.h"
#include "YuEngine/UiCore/UiRect.h"
#include "YuEngine/UiCore/UiStaticAtlasMetadata.h"

namespace yuengine::uicore {
class UiImageComponent final {
public:
    /**
     * @comment 将 Image component 描述转换成 sprite draw records。
     * @param tree UI node tree。
     * @param atlas_metadata static atlas metadata。
     * @param desc Image component 描述。
     * @param out_records 调用方持有的输出 draw record buffer。
     * @param out_result 输出 component result。
     * @return 显式 component 状态。
     */
    UiImageComponentStatus Build(
        const UiNodeTree &tree,
        const UiStaticAtlasMetadataDesc &atlas_metadata,
        const UiImageComponentDesc &desc,
        std::span<UiImageDrawRecord> out_records,
        UiImageComponentResult *out_result) const;

private:
    UiImageComponentStatus ValidateDesc(const UiImageComponentDesc &desc) const;
    UiImageComponentStatus ResolveSprite(
        const UiStaticAtlasMetadataDesc &atlas_metadata,
        const UiImageComponentDesc &desc,
        UiStaticAtlasResolveResult *out_sprite) const;
    UiImageComponentStatus WriteRecords(
        const UiNodeRecord &node_record,
        const UiStaticAtlasResolveResult &sprite,
        const UiImageComponentDesc &desc,
        std::span<UiImageDrawRecord> out_records,
        UiImageComponentResult *out_result) const;
    UiImageComponentStatus WriteSimpleRecord(
        const UiNodeRecord &node_record,
        const UiStaticAtlasResolveResult &sprite,
        const UiImageComponentDesc &desc,
        std::span<UiImageDrawRecord> out_records,
        UiImageComponentResult *out_result) const;
    UiImageComponentStatus WriteNineSliceRecords(
        const UiNodeRecord &node_record,
        const UiStaticAtlasResolveResult &sprite,
        const UiImageComponentDesc &desc,
        std::span<UiImageDrawRecord> out_records,
        UiImageComponentResult *out_result) const;
    UiImageDrawRecord BuildRecord(
        const UiNodeRecord &node_record,
        const UiStaticAtlasResolveResult &sprite,
        const UiImageComponentDesc &desc,
        UiRect rect,
        UiStaticAtlasUvRect uv_rect,
        std::uint32_t slice_index) const;
    UiStaticAtlasUvRect BuildUvRect(
        const UiStaticAtlasUvRect &base_rect,
        float u_min_ratio,
        float v_min_ratio,
        float u_max_ratio,
        float v_max_ratio) const;
};
}
