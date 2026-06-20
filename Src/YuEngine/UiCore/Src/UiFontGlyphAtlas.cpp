// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiFontGlyphAtlas.cpp

#include "YuEngine/UiCore/UiFontGlyphAtlas.h"

#include <array>
#include <cstddef>

#include "YuEngine/UiCore/UiCoreConstants.h"

namespace yuengine::uicore {
namespace {
bool IsStyleKnown(UiFontStyle style) {
    switch (style) {
        case UiFontStyle::Regular:
            return true;
        case UiFontStyle::Bold:
            return true;
        case UiFontStyle::Italic:
            return true;
        case UiFontStyle::BoldItalic:
            return true;
        default:
            break;
    }

    return false;
}

bool AreGlyphKeysEqual(const UiFontGlyphKey &left, const UiFontGlyphKey &right) {
    if (left.font_key != right.font_key) {
        return false;
    }

    if (left.codepoint != right.codepoint) {
        return false;
    }

    if (left.size_px != right.size_px) {
        return false;
    }

    return left.style == right.style;
}

bool IsGlyphKeyValid(const UiFontGlyphKey &key) {
    if (key.font_key == 0U || key.codepoint == 0U || key.size_px == 0U) {
        return false;
    }

    return IsStyleKnown(key.style);
}

bool IsGlyphRectInsidePage(const UiStaticAtlasPageDesc &page, const UiStaticAtlasSpriteDesc &sprite) {
    if (sprite.width == 0U || sprite.height == 0U) {
        return false;
    }

    if (sprite.x >= page.width || sprite.y >= page.height) {
        return false;
    }

    const std::uint32_t remaining_width = page.width - sprite.x;
    if (sprite.width > remaining_width) {
        return false;
    }

    const std::uint32_t remaining_height = page.height - sprite.y;
    if (sprite.height > remaining_height) {
        return false;
    }

    return true;
}

UiDynamicAtlasSpriteRequest BuildSpriteRequest(const UiFontGlyphBuildRequest &request) {
    UiDynamicAtlasSpriteRequest sprite_request;
    sprite_request.sprite_key = request.sprite_key;
    sprite_request.width = request.width;
    sprite_request.height = request.height;
    return sprite_request;
}

UiFontGlyphAtlasStatus MapDynamicStatus(UiDynamicAtlasStatus status) {
    switch (status) {
        case UiDynamicAtlasStatus::Success:
            return UiFontGlyphAtlasStatus::Success;
        case UiDynamicAtlasStatus::InvalidOutputBuffer:
            return UiFontGlyphAtlasStatus::InvalidOutput;
        case UiDynamicAtlasStatus::InvalidPackPhase:
            return UiFontGlyphAtlasStatus::PaintPathPackRejected;
        case UiDynamicAtlasStatus::InvalidAtlasPage:
            return UiFontGlyphAtlasStatus::InvalidAtlasPage;
        case UiDynamicAtlasStatus::InvalidSpriteRequest:
            return UiFontGlyphAtlasStatus::InvalidGlyphMetadata;
        case UiDynamicAtlasStatus::DuplicateSprite:
            return UiFontGlyphAtlasStatus::DuplicateGlyph;
        case UiDynamicAtlasStatus::OutputCapacityExceeded:
            return UiFontGlyphAtlasStatus::OutputCapacityExceeded;
        case UiDynamicAtlasStatus::SpriteTooLarge:
            return UiFontGlyphAtlasStatus::InvalidGlyphMetadata;
        case UiDynamicAtlasStatus::AtlasCapacityExceeded:
            return UiFontGlyphAtlasStatus::OutputCapacityExceeded;
        default:
            break;
    }

    return UiFontGlyphAtlasStatus::InvalidGlyphMetadata;
}

void SetBuildResult(
    UiFontGlyphBuildResult *out_result,
    UiFontGlyphAtlasStatus status,
    UiDynamicAtlasStatus dynamic_status) {
    out_result->status = status;
    out_result->dynamic_status = dynamic_status;
}
}

UiFontGlyphAtlasStatus UiFontGlyphAtlas::BuildGlyphSprites(
    const UiFontGlyphBuildDesc &desc,
    std::span<const UiFontGlyphBuildRequest> requests,
    std::span<UiStaticAtlasSpriteDesc> out_sprites,
    UiFontGlyphBuildResult *out_result) const {
    if (out_result == nullptr) {
        return UiFontGlyphAtlasStatus::InvalidOutput;
    }

    *out_result = UiFontGlyphBuildResult{};
    out_result->request_count = static_cast<std::uint32_t>(requests.size());
    out_result->required_allocation_count = out_result->request_count;

    UiFontGlyphAtlasStatus status = ValidateBuildRequests(requests);
    if (status != UiFontGlyphAtlasStatus::Success) {
        SetBuildResult(out_result, status, UiDynamicAtlasStatus::InvalidSpriteRequest);
        return status;
    }

    if (requests.size() > static_cast<std::size_t>(MAX_UI_NODE_COUNT)) {
        SetBuildResult(out_result, UiFontGlyphAtlasStatus::OutputCapacityExceeded, UiDynamicAtlasStatus::OutputCapacityExceeded);
        return UiFontGlyphAtlasStatus::OutputCapacityExceeded;
    }

    std::array<UiDynamicAtlasSpriteRequest, MAX_UI_NODE_COUNT> sprite_requests{};
    const std::size_t request_count = requests.size();
    for (std::size_t index = 0U; index < request_count; ++index) {
        sprite_requests[index] = BuildSpriteRequest(requests[index]);
    }

    UiDynamicAtlasPacker packer;
    UiDynamicAtlasPackResult pack_result;
    const UiDynamicAtlasPackDesc pack_desc{desc.pages, desc.phase, desc.padding};
    const std::span<const UiDynamicAtlasSpriteRequest> request_span(sprite_requests.data(), request_count);
    const UiDynamicAtlasStatus pack_status = packer.Pack(pack_desc, request_span, out_sprites, &pack_result);
    status = MapDynamicStatus(pack_status);
    SetBuildResult(out_result, status, pack_status);
    out_result->allocation_count = pack_result.allocation_count;
    out_result->required_allocation_count = pack_result.required_allocation_count;
    out_result->used_page_count = pack_result.used_page_count;
    if (pack_status == UiDynamicAtlasStatus::InvalidPackPhase) {
        out_result->paint_path_pack_count = out_result->request_count;
    }

    if (pack_result.failed_sprite_key != 0U) {
        for (const UiFontGlyphBuildRequest &request : requests) {
            if (request.sprite_key == pack_result.failed_sprite_key) {
                out_result->failed_codepoint = request.key.codepoint;
                break;
            }
        }
    }

    return status;
}

UiFontGlyphAtlasStatus UiFontGlyphAtlas::Validate(const UiFontGlyphAtlasDesc &desc) const {
    UiFontGlyphAtlasStatus status = ValidateUniquePages(desc.pages);
    if (status != UiFontGlyphAtlasStatus::Success) {
        return status;
    }

    status = ValidateUniqueFonts(desc.fonts);
    if (status != UiFontGlyphAtlasStatus::Success) {
        return status;
    }

    status = ValidateUniqueGlyphs(desc.glyphs);
    if (status != UiFontGlyphAtlasStatus::Success) {
        return status;
    }

    for (const UiStaticAtlasPageDesc &page : desc.pages) {
        status = ValidatePage(page);
        if (status != UiFontGlyphAtlasStatus::Success) {
            return status;
        }
    }

    for (const UiFontAssetDesc &font : desc.fonts) {
        status = ValidateFont(font);
        if (status != UiFontGlyphAtlasStatus::Success) {
            return status;
        }
    }

    for (const UiFontGlyphDesc &glyph : desc.glyphs) {
        status = ValidateGlyph(desc, glyph);
        if (status != UiFontGlyphAtlasStatus::Success) {
            return status;
        }
    }

    return UiFontGlyphAtlasStatus::Success;
}

UiFontGlyphResolveResult UiFontGlyphAtlas::ResolveGlyph(
    const UiFontGlyphAtlasDesc &desc,
    const UiFontGlyphResolveRequest &request) const {
    UiFontGlyphResolveResult result;
    result.requested_font_key = request.key.font_key;
    result.requested_codepoint = request.key.codepoint;
    result.size_px = request.key.size_px;
    result.style = request.key.style;

    UiFontGlyphAtlasStatus status = ValidateRequest(request);
    if (status != UiFontGlyphAtlasStatus::Success) {
        result.status = status;
        return result;
    }

    status = Validate(desc);
    if (status != UiFontGlyphAtlasStatus::Success) {
        result.status = status;
        return result;
    }

    const UiFontAssetDesc *font = FindFont(desc.fonts, request.key.font_key);
    if (font == nullptr) {
        result.status = UiFontGlyphAtlasStatus::InvalidFontAsset;
        return result;
    }

    std::uint32_t glyph_lookup_count = 1U;
    std::uint32_t fallback_lookup_count = 0U;
    const UiFontGlyphDesc *glyph = FindGlyph(desc.glyphs, request.key);
    if (glyph != nullptr) {
        const UiStaticAtlasPageDesc *page = FindPage(desc.pages, glyph->sprite.page_key);
        if (page == nullptr) {
            result.status = UiFontGlyphAtlasStatus::AtlasPageNotFound;
            return result;
        }

        return BuildResolveResult(request, *glyph, *page, false, 0U, glyph_lookup_count, fallback_lookup_count);
    }

    const std::size_t fallback_count = request.fallback_font_keys.size();
    for (std::size_t index = 0U; index < fallback_count; ++index) {
        const std::uint32_t fallback_font_key = request.fallback_font_keys[index];
        font = FindFont(desc.fonts, fallback_font_key);
        if (font == nullptr) {
            result.status = UiFontGlyphAtlasStatus::InvalidFontAsset;
            return result;
        }

        UiFontGlyphKey fallback_key = request.key;
        fallback_key.font_key = fallback_font_key;
        ++glyph_lookup_count;
        ++fallback_lookup_count;
        glyph = FindGlyph(desc.glyphs, fallback_key);
        if (glyph == nullptr) {
            continue;
        }

        const UiStaticAtlasPageDesc *page = FindPage(desc.pages, glyph->sprite.page_key);
        if (page == nullptr) {
            result.status = UiFontGlyphAtlasStatus::AtlasPageNotFound;
            return result;
        }

        const std::uint32_t fallback_depth = static_cast<std::uint32_t>(index + 1U);
        return BuildResolveResult(request, *glyph, *page, true, fallback_depth, glyph_lookup_count, fallback_lookup_count);
    }

    result.status = UiFontGlyphAtlasStatus::MissingGlyph;
    result.glyph_lookup_count = glyph_lookup_count;
    result.fallback_lookup_count = fallback_lookup_count;
    return result;
}

UiFontGlyphAtlasStatus UiFontGlyphAtlas::ValidateBuildRequests(
    std::span<const UiFontGlyphBuildRequest> requests) const {
    if (requests.size() > static_cast<std::size_t>(MAX_UI_NODE_COUNT)) {
        return UiFontGlyphAtlasStatus::OutputCapacityExceeded;
    }

    for (const UiFontGlyphBuildRequest &request : requests) {
        const UiFontGlyphAtlasStatus status = ValidateBuildRequest(request);
        if (status != UiFontGlyphAtlasStatus::Success) {
            return status;
        }
    }

    const std::size_t request_count = requests.size();
    for (std::size_t left_index = 0U; left_index < request_count; ++left_index) {
        for (std::size_t right_index = left_index + 1U; right_index < request_count; ++right_index) {
            if (requests[left_index].sprite_key == requests[right_index].sprite_key) {
                return UiFontGlyphAtlasStatus::DuplicateGlyph;
            }

            if (AreGlyphKeysEqual(requests[left_index].key, requests[right_index].key)) {
                return UiFontGlyphAtlasStatus::DuplicateGlyph;
            }
        }
    }

    return UiFontGlyphAtlasStatus::Success;
}

UiFontGlyphAtlasStatus UiFontGlyphAtlas::ValidateBuildRequest(const UiFontGlyphBuildRequest &request) const {
    if (!IsGlyphKeyValid(request.key)) {
        return UiFontGlyphAtlasStatus::InvalidGlyphKey;
    }

    if (request.sprite_key == 0U || request.width == 0U || request.height == 0U) {
        return UiFontGlyphAtlasStatus::InvalidGlyphMetadata;
    }

    return UiFontGlyphAtlasStatus::Success;
}

UiFontGlyphAtlasStatus UiFontGlyphAtlas::ValidatePage(const UiStaticAtlasPageDesc &page) const {
    if (page.page_key == 0U || page.texture_key == 0U) {
        return UiFontGlyphAtlasStatus::InvalidAtlasPage;
    }

    if (page.width == 0U || page.height == 0U) {
        return UiFontGlyphAtlasStatus::InvalidAtlasPage;
    }

    return UiFontGlyphAtlasStatus::Success;
}

UiFontGlyphAtlasStatus UiFontGlyphAtlas::ValidateFont(const UiFontAssetDesc &font) const {
    if (font.font_key == 0U) {
        return UiFontGlyphAtlasStatus::InvalidFontAsset;
    }

    return UiFontGlyphAtlasStatus::Success;
}

UiFontGlyphAtlasStatus UiFontGlyphAtlas::ValidateGlyph(
    const UiFontGlyphAtlasDesc &desc,
    const UiFontGlyphDesc &glyph) const {
    if (!IsGlyphKeyValid(glyph.key)) {
        return UiFontGlyphAtlasStatus::InvalidGlyphKey;
    }

    const UiFontAssetDesc *font = FindFont(desc.fonts, glyph.key.font_key);
    if (font == nullptr) {
        return UiFontGlyphAtlasStatus::InvalidFontAsset;
    }

    if (glyph.sprite.sprite_key == 0U || glyph.sprite.nine_slice_enabled) {
        return UiFontGlyphAtlasStatus::InvalidGlyphMetadata;
    }

    const UiStaticAtlasPageDesc *page = FindPage(desc.pages, glyph.sprite.page_key);
    if (page == nullptr) {
        return UiFontGlyphAtlasStatus::AtlasPageNotFound;
    }

    if (!IsGlyphRectInsidePage(*page, glyph.sprite)) {
        return UiFontGlyphAtlasStatus::InvalidGlyphMetadata;
    }

    return UiFontGlyphAtlasStatus::Success;
}

UiFontGlyphAtlasStatus UiFontGlyphAtlas::ValidateUniquePages(std::span<const UiStaticAtlasPageDesc> pages) const {
    const std::size_t page_count = pages.size();
    for (std::size_t left_index = 0U; left_index < page_count; ++left_index) {
        for (std::size_t right_index = left_index + 1U; right_index < page_count; ++right_index) {
            if (pages[left_index].page_key == pages[right_index].page_key) {
                return UiFontGlyphAtlasStatus::DuplicateAtlasPage;
            }
        }
    }

    return UiFontGlyphAtlasStatus::Success;
}

UiFontGlyphAtlasStatus UiFontGlyphAtlas::ValidateUniqueFonts(std::span<const UiFontAssetDesc> fonts) const {
    const std::size_t font_count = fonts.size();
    for (std::size_t left_index = 0U; left_index < font_count; ++left_index) {
        for (std::size_t right_index = left_index + 1U; right_index < font_count; ++right_index) {
            if (fonts[left_index].font_key == fonts[right_index].font_key) {
                return UiFontGlyphAtlasStatus::DuplicateFontAsset;
            }
        }
    }

    return UiFontGlyphAtlasStatus::Success;
}

UiFontGlyphAtlasStatus UiFontGlyphAtlas::ValidateUniqueGlyphs(std::span<const UiFontGlyphDesc> glyphs) const {
    const std::size_t glyph_count = glyphs.size();
    for (std::size_t left_index = 0U; left_index < glyph_count; ++left_index) {
        for (std::size_t right_index = left_index + 1U; right_index < glyph_count; ++right_index) {
            if (AreGlyphKeysEqual(glyphs[left_index].key, glyphs[right_index].key)) {
                return UiFontGlyphAtlasStatus::DuplicateGlyph;
            }
        }
    }

    return UiFontGlyphAtlasStatus::Success;
}

UiFontGlyphAtlasStatus UiFontGlyphAtlas::ValidateRequest(const UiFontGlyphResolveRequest &request) const {
    if (!IsGlyphKeyValid(request.key)) {
        return UiFontGlyphAtlasStatus::InvalidGlyphKey;
    }

    if (request.fallback_font_keys.size() > static_cast<std::size_t>(MAX_UI_FONT_FALLBACK_COUNT)) {
        return UiFontGlyphAtlasStatus::FallbackCapacityExceeded;
    }

    for (const std::uint32_t font_key : request.fallback_font_keys) {
        if (font_key == 0U) {
            return UiFontGlyphAtlasStatus::InvalidFontAsset;
        }
    }

    return UiFontGlyphAtlasStatus::Success;
}

const UiStaticAtlasPageDesc *UiFontGlyphAtlas::FindPage(
    std::span<const UiStaticAtlasPageDesc> pages,
    std::uint32_t page_key) const {
    for (const UiStaticAtlasPageDesc &page : pages) {
        if (page.page_key == page_key) {
            return &page;
        }
    }

    return nullptr;
}

const UiFontAssetDesc *UiFontGlyphAtlas::FindFont(
    std::span<const UiFontAssetDesc> fonts,
    std::uint32_t font_key) const {
    for (const UiFontAssetDesc &font : fonts) {
        if (font.font_key == font_key) {
            return &font;
        }
    }

    return nullptr;
}

const UiFontGlyphDesc *UiFontGlyphAtlas::FindGlyph(
    std::span<const UiFontGlyphDesc> glyphs,
    const UiFontGlyphKey &key) const {
    for (const UiFontGlyphDesc &glyph : glyphs) {
        if (AreGlyphKeysEqual(glyph.key, key)) {
            return &glyph;
        }
    }

    return nullptr;
}

UiFontGlyphResolveResult UiFontGlyphAtlas::BuildResolveResult(
    const UiFontGlyphResolveRequest &request,
    const UiFontGlyphDesc &glyph,
    const UiStaticAtlasPageDesc &page,
    bool used_fallback,
    std::uint32_t fallback_depth,
    std::uint32_t glyph_lookup_count,
    std::uint32_t fallback_lookup_count) const {
    UiFontGlyphResolveResult result;
    result.status = UiFontGlyphAtlasStatus::Success;
    result.requested_font_key = request.key.font_key;
    result.resolved_font_key = glyph.key.font_key;
    result.requested_codepoint = request.key.codepoint;
    result.resolved_codepoint = glyph.key.codepoint;
    result.size_px = glyph.key.size_px;
    result.style = glyph.key.style;
    result.used_fallback = used_fallback;
    result.fallback_depth = fallback_depth;
    result.glyph_lookup_count = glyph_lookup_count;
    result.fallback_lookup_count = fallback_lookup_count;
    result.sprite_key = glyph.sprite.sprite_key;
    result.page_key = page.page_key;
    result.texture_key = page.texture_key;
    result.x = glyph.sprite.x;
    result.y = glyph.sprite.y;
    result.width = glyph.sprite.width;
    result.height = glyph.sprite.height;
    result.uv_rect = BuildUvRect(page, glyph.sprite);
    result.advance_x = glyph.advance_x;
    result.bearing_x = glyph.bearing_x;
    result.bearing_y = glyph.bearing_y;
    return result;
}

UiStaticAtlasUvRect UiFontGlyphAtlas::BuildUvRect(
    const UiStaticAtlasPageDesc &page,
    const UiStaticAtlasSpriteDesc &sprite) const {
    const float inv_width = 1.0F / static_cast<float>(page.width);
    const float inv_height = 1.0F / static_cast<float>(page.height);
    UiStaticAtlasUvRect rect;
    rect.u_min = static_cast<float>(sprite.x) * inv_width;
    rect.v_min = static_cast<float>(sprite.y) * inv_height;
    rect.u_max = static_cast<float>(sprite.x + sprite.width) * inv_width;
    rect.v_max = static_cast<float>(sprite.y + sprite.height) * inv_height;
    return rect;
}
}
