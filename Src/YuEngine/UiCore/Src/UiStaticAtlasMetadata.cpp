// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiStaticAtlasMetadata.cpp

#include "YuEngine/UiCore/UiStaticAtlasMetadata.h"

namespace yuengine::uicore {
namespace {
bool IsSpriteRectInsidePage(const UiStaticAtlasPageDesc &page, const UiStaticAtlasSpriteDesc &sprite) {
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

bool IsNineSliceInsideSprite(const UiStaticAtlasSpriteDesc &sprite) {
    if (!sprite.nine_slice_enabled) {
        return true;
    }

    if (sprite.border_left > sprite.width) {
        return false;
    }

    const std::uint32_t remaining_width = sprite.width - sprite.border_left;
    if (sprite.border_right > remaining_width) {
        return false;
    }

    if (sprite.border_top > sprite.height) {
        return false;
    }

    const std::uint32_t remaining_height = sprite.height - sprite.border_top;
    if (sprite.border_bottom > remaining_height) {
        return false;
    }

    return true;
}
}

UiStaticAtlasStatus UiStaticAtlasMetadata::Validate(const UiStaticAtlasMetadataDesc &desc) const {
    UiStaticAtlasStatus status = ValidateUniquePages(desc.pages);
    if (status != UiStaticAtlasStatus::Success) {
        return status;
    }

    status = ValidateUniqueSprites(desc.sprites);
    if (status != UiStaticAtlasStatus::Success) {
        return status;
    }

    for (const UiStaticAtlasPageDesc &page : desc.pages) {
        status = ValidatePage(page);
        if (status != UiStaticAtlasStatus::Success) {
            return status;
        }
    }

    for (const UiStaticAtlasSpriteDesc &sprite : desc.sprites) {
        status = ValidateSprite(sprite, desc.pages);
        if (status != UiStaticAtlasStatus::Success) {
            return status;
        }
    }

    return UiStaticAtlasStatus::Success;
}

UiStaticAtlasResolveResult UiStaticAtlasMetadata::ResolveSprite(
    const UiStaticAtlasMetadataDesc &desc,
    std::uint32_t sprite_key) const {
    UiStaticAtlasResolveResult result;
    if (sprite_key == 0U) {
        result.status = UiStaticAtlasStatus::InvalidSpriteKey;
        return result;
    }

    const UiStaticAtlasStatus validate_status = Validate(desc);
    if (validate_status != UiStaticAtlasStatus::Success) {
        result.status = validate_status;
        return result;
    }

    const UiStaticAtlasSpriteDesc *sprite = FindSprite(desc.sprites, sprite_key);
    if (sprite == nullptr) {
        result.status = UiStaticAtlasStatus::SpriteNotFound;
        return result;
    }

    const UiStaticAtlasPageDesc *page = FindPage(desc.pages, sprite->page_key);
    if (page == nullptr) {
        result.status = UiStaticAtlasStatus::AtlasPageNotFound;
        return result;
    }

    result.status = UiStaticAtlasStatus::Success;
    result.sprite_key = sprite->sprite_key;
    result.page_key = page->page_key;
    result.texture_key = page->texture_key;
    result.x = sprite->x;
    result.y = sprite->y;
    result.width = sprite->width;
    result.height = sprite->height;
    result.uv_rect = BuildUvRect(*page, *sprite);
    result.nine_slice.enabled = sprite->nine_slice_enabled;
    result.nine_slice.border_left = sprite->border_left;
    result.nine_slice.border_top = sprite->border_top;
    result.nine_slice.border_right = sprite->border_right;
    result.nine_slice.border_bottom = sprite->border_bottom;
    return result;
}

const UiStaticAtlasPageDesc *UiStaticAtlasMetadata::FindPage(
    std::span<const UiStaticAtlasPageDesc> pages,
    std::uint32_t page_key) const {
    for (const UiStaticAtlasPageDesc &page : pages) {
        if (page.page_key == page_key) {
            return &page;
        }
    }

    return nullptr;
}

const UiStaticAtlasSpriteDesc *UiStaticAtlasMetadata::FindSprite(
    std::span<const UiStaticAtlasSpriteDesc> sprites,
    std::uint32_t sprite_key) const {
    for (const UiStaticAtlasSpriteDesc &sprite : sprites) {
        if (sprite.sprite_key == sprite_key) {
            return &sprite;
        }
    }

    return nullptr;
}

UiStaticAtlasStatus UiStaticAtlasMetadata::ValidatePage(const UiStaticAtlasPageDesc &page) const {
    if (page.page_key == 0U || page.texture_key == 0U) {
        return UiStaticAtlasStatus::InvalidAtlasPage;
    }

    if (page.width == 0U || page.height == 0U) {
        return UiStaticAtlasStatus::InvalidAtlasPage;
    }

    return UiStaticAtlasStatus::Success;
}

UiStaticAtlasStatus UiStaticAtlasMetadata::ValidateSprite(
    const UiStaticAtlasSpriteDesc &sprite,
    std::span<const UiStaticAtlasPageDesc> pages) const {
    if (sprite.sprite_key == 0U) {
        return UiStaticAtlasStatus::InvalidSpriteKey;
    }

    const UiStaticAtlasPageDesc *page = FindPage(pages, sprite.page_key);
    if (page == nullptr) {
        return UiStaticAtlasStatus::AtlasPageNotFound;
    }

    if (!IsSpriteRectInsidePage(*page, sprite)) {
        return UiStaticAtlasStatus::InvalidSpriteRect;
    }

    if (!IsNineSliceInsideSprite(sprite)) {
        return UiStaticAtlasStatus::InvalidNineSlice;
    }

    return UiStaticAtlasStatus::Success;
}

UiStaticAtlasStatus UiStaticAtlasMetadata::ValidateUniquePages(
    std::span<const UiStaticAtlasPageDesc> pages) const {
    const std::size_t page_count = pages.size();
    for (std::size_t left_index = 0U; left_index < page_count; ++left_index) {
        for (std::size_t right_index = left_index + 1U; right_index < page_count; ++right_index) {
            if (pages[left_index].page_key == pages[right_index].page_key) {
                return UiStaticAtlasStatus::DuplicateAtlasPage;
            }
        }
    }

    return UiStaticAtlasStatus::Success;
}

UiStaticAtlasStatus UiStaticAtlasMetadata::ValidateUniqueSprites(
    std::span<const UiStaticAtlasSpriteDesc> sprites) const {
    const std::size_t sprite_count = sprites.size();
    for (std::size_t left_index = 0U; left_index < sprite_count; ++left_index) {
        for (std::size_t right_index = left_index + 1U; right_index < sprite_count; ++right_index) {
            if (sprites[left_index].sprite_key == sprites[right_index].sprite_key) {
                return UiStaticAtlasStatus::DuplicateSprite;
            }
        }
    }

    return UiStaticAtlasStatus::Success;
}

UiStaticAtlasUvRect UiStaticAtlasMetadata::BuildUvRect(
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
