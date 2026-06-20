// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiFontGlyphAtlas.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/UiCore/UiDynamicAtlasPacker.h"
#include "YuEngine/UiCore/UiStaticAtlasMetadata.h"

namespace yuengine::uicore {
constexpr std::uint32_t MAX_UI_FONT_FALLBACK_COUNT = 8U;

enum class UiFontStyle {
    Regular,
    Bold,
    Italic,
    BoldItalic
};

enum class UiFontGlyphAtlasStatus {
    Success,
    InvalidOutput,
    InvalidFontAsset,
    DuplicateFontAsset,
    InvalidGlyphKey,
    InvalidGlyphMetadata,
    DuplicateGlyph,
    InvalidAtlasPage,
    DuplicateAtlasPage,
    AtlasPageNotFound,
    MissingGlyph,
    FallbackCapacityExceeded,
    OutputCapacityExceeded,
    PaintPathPackRejected
};

struct UiFontAssetDesc final {
    std::uint32_t font_key = 0U;
};

struct UiFontGlyphKey final {
    std::uint32_t font_key = 0U;
    std::uint32_t codepoint = 0U;
    std::uint32_t size_px = 0U;
    UiFontStyle style = UiFontStyle::Regular;
};

struct UiFontGlyphDesc final {
    UiFontGlyphKey key;
    UiStaticAtlasSpriteDesc sprite;
    std::int32_t advance_x = 0;
    std::int32_t bearing_x = 0;
    std::int32_t bearing_y = 0;
};

struct UiFontGlyphAtlasDesc final {
    std::span<const UiStaticAtlasPageDesc> pages;
    std::span<const UiFontAssetDesc> fonts;
    std::span<const UiFontGlyphDesc> glyphs;
};

struct UiFontGlyphResolveRequest final {
    UiFontGlyphKey key;
    std::span<const std::uint32_t> fallback_font_keys;
};

struct UiFontGlyphResolveResult final {
    UiFontGlyphAtlasStatus status = UiFontGlyphAtlasStatus::MissingGlyph;
    std::uint32_t requested_font_key = 0U;
    std::uint32_t resolved_font_key = 0U;
    std::uint32_t requested_codepoint = 0U;
    std::uint32_t resolved_codepoint = 0U;
    std::uint32_t size_px = 0U;
    UiFontStyle style = UiFontStyle::Regular;
    bool used_fallback = false;
    std::uint32_t fallback_depth = 0U;
    std::uint32_t glyph_lookup_count = 0U;
    std::uint32_t fallback_lookup_count = 0U;
    std::uint32_t paint_path_pack_count = 0U;
    std::uint32_t atlas_repack_count = 0U;
    std::uint32_t sprite_key = 0U;
    std::uint32_t page_key = 0U;
    std::uint32_t texture_key = 0U;
    std::uint32_t x = 0U;
    std::uint32_t y = 0U;
    std::uint32_t width = 0U;
    std::uint32_t height = 0U;
    UiStaticAtlasUvRect uv_rect;
    std::int32_t advance_x = 0;
    std::int32_t bearing_x = 0;
    std::int32_t bearing_y = 0;

    /**
     * @comment 检查 glyph resolve 是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiFontGlyphAtlasStatus::Success;
    }
};

struct UiFontGlyphBuildRequest final {
    UiFontGlyphKey key;
    std::uint32_t sprite_key = 0U;
    std::uint32_t width = 0U;
    std::uint32_t height = 0U;
};

struct UiFontGlyphBuildDesc final {
    std::span<const UiStaticAtlasPageDesc> pages;
    UiDynamicAtlasPackPhase phase = UiDynamicAtlasPackPhase::SafePoint;
    std::uint32_t padding = 0U;
};

struct UiFontGlyphBuildResult final {
    UiFontGlyphAtlasStatus status = UiFontGlyphAtlasStatus::InvalidOutput;
    UiDynamicAtlasStatus dynamic_status = UiDynamicAtlasStatus::InvalidOutputBuffer;
    std::uint32_t request_count = 0U;
    std::uint32_t allocation_count = 0U;
    std::uint32_t required_allocation_count = 0U;
    std::uint32_t used_page_count = 0U;
    std::uint32_t failed_codepoint = 0U;
    std::uint32_t paint_path_pack_count = 0U;

    /**
     * @comment 检查 glyph atlas build 是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiFontGlyphAtlasStatus::Success;
    }
};

class UiFontGlyphAtlas final {
public:
    /**
     * @comment 在 safe point 将 glyph 请求打包为 atlas sprite metadata。
     * @param desc glyph build 描述。
     * @param requests glyph build 请求。
     * @param out_sprites 调用方持有的 sprite metadata 输出 buffer。
     * @param out_result 输出 build result。
     * @return 显式 build 状态。
     */
    UiFontGlyphAtlasStatus BuildGlyphSprites(
        const UiFontGlyphBuildDesc &desc,
        std::span<const UiFontGlyphBuildRequest> requests,
        std::span<UiStaticAtlasSpriteDesc> out_sprites,
        UiFontGlyphBuildResult *out_result) const;

    /**
     * @comment 校验 font glyph atlas metadata。
     * @param desc font glyph atlas 描述。
     * @return 显式校验状态。
     */
    UiFontGlyphAtlasStatus Validate(const UiFontGlyphAtlasDesc &desc) const;

    /**
     * @comment 解析 font、size、style 和 codepoint 到 glyph atlas metadata。
     * @param desc font glyph atlas 描述。
     * @param request glyph resolve 请求。
     * @return glyph resolve 结果。
     */
    UiFontGlyphResolveResult ResolveGlyph(
        const UiFontGlyphAtlasDesc &desc,
        const UiFontGlyphResolveRequest &request) const;

private:
    UiFontGlyphAtlasStatus ValidateBuildRequests(std::span<const UiFontGlyphBuildRequest> requests) const;
    UiFontGlyphAtlasStatus ValidateBuildRequest(const UiFontGlyphBuildRequest &request) const;
    UiFontGlyphAtlasStatus ValidatePage(const UiStaticAtlasPageDesc &page) const;
    UiFontGlyphAtlasStatus ValidateFont(const UiFontAssetDesc &font) const;
    UiFontGlyphAtlasStatus ValidateGlyph(const UiFontGlyphAtlasDesc &desc, const UiFontGlyphDesc &glyph) const;
    UiFontGlyphAtlasStatus ValidateUniquePages(std::span<const UiStaticAtlasPageDesc> pages) const;
    UiFontGlyphAtlasStatus ValidateUniqueFonts(std::span<const UiFontAssetDesc> fonts) const;
    UiFontGlyphAtlasStatus ValidateUniqueGlyphs(std::span<const UiFontGlyphDesc> glyphs) const;
    UiFontGlyphAtlasStatus ValidateRequest(const UiFontGlyphResolveRequest &request) const;
    const UiStaticAtlasPageDesc *FindPage(
        std::span<const UiStaticAtlasPageDesc> pages,
        std::uint32_t page_key) const;
    const UiFontAssetDesc *FindFont(
        std::span<const UiFontAssetDesc> fonts,
        std::uint32_t font_key) const;
    const UiFontGlyphDesc *FindGlyph(
        std::span<const UiFontGlyphDesc> glyphs,
        const UiFontGlyphKey &key) const;
    UiFontGlyphResolveResult BuildResolveResult(
        const UiFontGlyphResolveRequest &request,
        const UiFontGlyphDesc &glyph,
        const UiStaticAtlasPageDesc &page,
        bool used_fallback,
        std::uint32_t fallback_depth,
        std::uint32_t glyph_lookup_count,
        std::uint32_t fallback_lookup_count) const;
    UiStaticAtlasUvRect BuildUvRect(
        const UiStaticAtlasPageDesc &page,
        const UiStaticAtlasSpriteDesc &sprite) const;
};
}
