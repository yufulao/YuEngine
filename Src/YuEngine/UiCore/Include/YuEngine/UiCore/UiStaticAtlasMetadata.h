// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiStaticAtlasMetadata.h

#pragma once

#include <cstdint>
#include <span>

namespace yuengine::uicore {
enum class UiStaticAtlasStatus {
    Success,
    InvalidSpriteKey,
    InvalidAtlasPage,
    InvalidSpriteRect,
    InvalidNineSlice,
    DuplicateAtlasPage,
    DuplicateSprite,
    AtlasPageNotFound,
    SpriteNotFound
};

struct UiStaticAtlasPageDesc final {
    std::uint32_t page_key = 0U;
    std::uint32_t texture_key = 0U;
    std::uint32_t width = 0U;
    std::uint32_t height = 0U;
};

struct UiStaticAtlasSpriteDesc final {
    std::uint32_t sprite_key = 0U;
    std::uint32_t page_key = 0U;
    std::uint32_t x = 0U;
    std::uint32_t y = 0U;
    std::uint32_t width = 0U;
    std::uint32_t height = 0U;
    bool nine_slice_enabled = false;
    std::uint32_t border_left = 0U;
    std::uint32_t border_top = 0U;
    std::uint32_t border_right = 0U;
    std::uint32_t border_bottom = 0U;
};

struct UiStaticAtlasMetadataDesc final {
    std::span<const UiStaticAtlasPageDesc> pages;
    std::span<const UiStaticAtlasSpriteDesc> sprites;
};

struct UiStaticAtlasUvRect final {
    float u_min = 0.0F;
    float v_min = 0.0F;
    float u_max = 0.0F;
    float v_max = 0.0F;
};

struct UiStaticAtlasNineSlice final {
    bool enabled = false;
    std::uint32_t border_left = 0U;
    std::uint32_t border_top = 0U;
    std::uint32_t border_right = 0U;
    std::uint32_t border_bottom = 0U;
};

struct UiStaticAtlasResolveResult final {
    UiStaticAtlasStatus status = UiStaticAtlasStatus::SpriteNotFound;
    std::uint32_t sprite_key = 0U;
    std::uint32_t page_key = 0U;
    std::uint32_t texture_key = 0U;
    std::uint32_t x = 0U;
    std::uint32_t y = 0U;
    std::uint32_t width = 0U;
    std::uint32_t height = 0U;
    UiStaticAtlasUvRect uv_rect;
    UiStaticAtlasNineSlice nine_slice;

    /**
     * @comment 检查 atlas sprite resolve 是否成功。
     * @return 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiStaticAtlasStatus::Success;
    }
};

class UiStaticAtlasMetadata final {
public:
    /**
     * @comment 校验静态 atlas metadata。
     * @param desc atlas metadata 描述。
     * @return 显式校验状态。
     */
    UiStaticAtlasStatus Validate(const UiStaticAtlasMetadataDesc &desc) const;
    /**
     * @comment 解析 sprite key 到 atlas page、texture、UV 和 nine-slice metadata。
     * @param desc atlas metadata 描述。
     * @param sprite_key sprite key。
     * @return 解析结果。
     */
    UiStaticAtlasResolveResult ResolveSprite(
        const UiStaticAtlasMetadataDesc &desc,
        std::uint32_t sprite_key) const;

private:
    const UiStaticAtlasPageDesc *FindPage(
        std::span<const UiStaticAtlasPageDesc> pages,
        std::uint32_t page_key) const;
    const UiStaticAtlasSpriteDesc *FindSprite(
        std::span<const UiStaticAtlasSpriteDesc> sprites,
        std::uint32_t sprite_key) const;
    UiStaticAtlasStatus ValidatePage(const UiStaticAtlasPageDesc &page) const;
    UiStaticAtlasStatus ValidateSprite(
        const UiStaticAtlasSpriteDesc &sprite,
        std::span<const UiStaticAtlasPageDesc> pages) const;
    UiStaticAtlasStatus ValidateUniquePages(std::span<const UiStaticAtlasPageDesc> pages) const;
    UiStaticAtlasStatus ValidateUniqueSprites(std::span<const UiStaticAtlasSpriteDesc> sprites) const;
    UiStaticAtlasUvRect BuildUvRect(
        const UiStaticAtlasPageDesc &page,
        const UiStaticAtlasSpriteDesc &sprite) const;
};
}
