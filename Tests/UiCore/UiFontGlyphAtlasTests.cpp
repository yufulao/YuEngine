// 模块: Tests UiCore
// 文件: Tests/UiCore/UiFontGlyphAtlasTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiCore/UiFontGlyphAtlas.h"

using UiDynamicAtlasPackPhase = yuengine::uicore::UiDynamicAtlasPackPhase;
using UiFontAssetDesc = yuengine::uicore::UiFontAssetDesc;
using UiFontGlyphAtlas = yuengine::uicore::UiFontGlyphAtlas;
using UiFontGlyphAtlasDesc = yuengine::uicore::UiFontGlyphAtlasDesc;
using UiFontGlyphAtlasStatus = yuengine::uicore::UiFontGlyphAtlasStatus;
using UiFontGlyphBuildDesc = yuengine::uicore::UiFontGlyphBuildDesc;
using UiFontGlyphBuildRequest = yuengine::uicore::UiFontGlyphBuildRequest;
using UiFontGlyphBuildResult = yuengine::uicore::UiFontGlyphBuildResult;
using UiFontGlyphDesc = yuengine::uicore::UiFontGlyphDesc;
using UiFontGlyphKey = yuengine::uicore::UiFontGlyphKey;
using UiFontGlyphResolveRequest = yuengine::uicore::UiFontGlyphResolveRequest;
using UiFontGlyphResolveResult = yuengine::uicore::UiFontGlyphResolveResult;
using UiStaticAtlasPageDesc = yuengine::uicore::UiStaticAtlasPageDesc;
using UiStaticAtlasSpriteDesc = yuengine::uicore::UiStaticAtlasSpriteDesc;

namespace {
constexpr const char *TEST_SAFE_POINT =
    "UiCore_FontGlyphAtlas_SafePointBuildResolvesPrimaryGlyph";
constexpr const char *TEST_FALLBACK =
    "UiCore_FontGlyphAtlas_TextFixtureResolvesFallbackGlyph";
constexpr const char *TEST_MISSING =
    "UiCore_FontGlyphAtlas_TextFixtureReportsMissingGlyph";
constexpr const char *TEST_PAINT_PATH =
    "UiCore_FontGlyphAtlas_RejectsPaintPathPackAndDuplicateGlyph";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t FONT_PRIMARY = 100U;
constexpr std::uint32_t FONT_FALLBACK = 200U;
constexpr std::uint32_t CODEPOINT_A = 65U;
constexpr std::uint32_t CODEPOINT_B = 66U;
constexpr std::uint32_t SPRITE_SENTINEL = 9999U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

UiStaticAtlasPageDesc MakePage(
    std::uint32_t page_key,
    std::uint32_t texture_key,
    std::uint32_t width,
    std::uint32_t height) {
    UiStaticAtlasPageDesc desc;
    desc.page_key = page_key;
    desc.texture_key = texture_key;
    desc.width = width;
    desc.height = height;
    return desc;
}

UiFontAssetDesc MakeFont(std::uint32_t font_key) {
    UiFontAssetDesc desc;
    desc.font_key = font_key;
    return desc;
}

UiFontGlyphKey MakeGlyphKey(
    std::uint32_t font_key,
    std::uint32_t codepoint,
    std::uint32_t size_px) {
    UiFontGlyphKey key;
    key.font_key = font_key;
    key.codepoint = codepoint;
    key.size_px = size_px;
    return key;
}

UiFontGlyphBuildRequest MakeBuildRequest(
    std::uint32_t font_key,
    std::uint32_t codepoint,
    std::uint32_t sprite_key,
    std::uint32_t width,
    std::uint32_t height) {
    UiFontGlyphBuildRequest request;
    request.key = MakeGlyphKey(font_key, codepoint, 16U);
    request.sprite_key = sprite_key;
    request.width = width;
    request.height = height;
    return request;
}

UiFontGlyphDesc MakeGlyph(
    const UiFontGlyphKey &key,
    const UiStaticAtlasSpriteDesc &sprite,
    std::int32_t advance_x) {
    UiFontGlyphDesc glyph;
    glyph.key = key;
    glyph.sprite = sprite;
    glyph.advance_x = advance_x;
    glyph.bearing_x = 1;
    glyph.bearing_y = 12;
    return glyph;
}

UiStaticAtlasSpriteDesc SentinelSprite() {
    UiStaticAtlasSpriteDesc sprite;
    sprite.sprite_key = SPRITE_SENTINEL;
    sprite.page_key = SPRITE_SENTINEL;
    sprite.x = SPRITE_SENTINEL;
    return sprite;
}

bool SpriteMatchesSentinel(const UiStaticAtlasSpriteDesc &sprite) {
    if (sprite.sprite_key != SPRITE_SENTINEL) {
        return false;
    }

    if (sprite.page_key != SPRITE_SENTINEL) {
        return false;
    }

    return sprite.x == SPRITE_SENTINEL;
}

bool FloatClose(float left, float right) {
    float diff = left - right;
    if (diff < 0.0F) {
        diff = -diff;
    }

    return diff < 0.0001F;
}

int UiCoreFontGlyphAtlasSafePointBuildResolvesPrimaryGlyph() {
    const std::array<UiStaticAtlasPageDesc, 1U> pages{
        MakePage(7U, 77U, 64U, 64U)};
    const std::array<UiFontGlyphBuildRequest, 1U> requests{
        MakeBuildRequest(FONT_PRIMARY, CODEPOINT_A, 1001U, 8U, 16U)};
    std::array<UiStaticAtlasSpriteDesc, 1U> sprites{};
    UiFontGlyphBuildResult build_result{};
    UiFontGlyphAtlas atlas{};
    const UiFontGlyphBuildDesc build_desc{pages, UiDynamicAtlasPackPhase::SafePoint, 0U};
    const UiFontGlyphAtlasStatus build_status = atlas.BuildGlyphSprites(build_desc, requests, sprites, &build_result);
    if (build_status != UiFontGlyphAtlasStatus::Success || !build_result.Succeeded()) {
        return Fail("font glyph safe-point build failed");
    }

    if (build_result.allocation_count != 1U || build_result.used_page_count != 1U) {
        return Fail("font glyph build counters mismatch");
    }

    const std::array<UiFontAssetDesc, 1U> fonts{
        MakeFont(FONT_PRIMARY)};
    const std::array<UiFontGlyphDesc, 1U> glyphs{
        MakeGlyph(requests[0U].key, sprites[0U], 9)};
    const UiFontGlyphAtlasDesc atlas_desc{pages, fonts, glyphs};
    UiFontGlyphResolveRequest resolve_request;
    resolve_request.key = MakeGlyphKey(FONT_PRIMARY, CODEPOINT_A, 16U);
    const UiFontGlyphResolveResult resolve_result = atlas.ResolveGlyph(atlas_desc, resolve_request);
    if (!resolve_result.Succeeded()) {
        return Fail("font glyph primary resolve failed");
    }

    if (resolve_result.resolved_font_key != FONT_PRIMARY || resolve_result.texture_key != 77U) {
        return Fail("font glyph primary resolve metadata mismatch");
    }

    if (resolve_result.paint_path_pack_count != 0U || resolve_result.atlas_repack_count != 0U) {
        return Fail("font glyph primary resolve touched pack counters");
    }

    if (!FloatClose(resolve_result.uv_rect.u_max, 0.125F) || !FloatClose(resolve_result.uv_rect.v_max, 0.25F)) {
        return Fail("font glyph primary resolve uv mismatch");
    }

    return 0;
}

int UiCoreFontGlyphAtlasTextFixtureResolvesFallbackGlyph() {
    const std::array<UiStaticAtlasPageDesc, 1U> pages{
        MakePage(7U, 77U, 64U, 64U)};
    UiStaticAtlasSpriteDesc fallback_sprite;
    fallback_sprite.sprite_key = 2001U;
    fallback_sprite.page_key = 7U;
    fallback_sprite.x = 16U;
    fallback_sprite.y = 0U;
    fallback_sprite.width = 8U;
    fallback_sprite.height = 16U;
    const std::array<UiFontAssetDesc, 2U> fonts{
        MakeFont(FONT_PRIMARY),
        MakeFont(FONT_FALLBACK)};
    const std::array<UiFontGlyphDesc, 1U> glyphs{
        MakeGlyph(MakeGlyphKey(FONT_FALLBACK, CODEPOINT_B, 16U), fallback_sprite, 10)};
    const UiFontGlyphAtlasDesc atlas_desc{pages, fonts, glyphs};
    const std::array<std::uint32_t, 1U> fallback_fonts{FONT_FALLBACK};

    UiFontGlyphResolveRequest request;
    request.key = MakeGlyphKey(FONT_PRIMARY, CODEPOINT_B, 16U);
    request.fallback_font_keys = fallback_fonts;
    UiFontGlyphAtlas atlas{};
    const UiFontGlyphResolveResult result = atlas.ResolveGlyph(atlas_desc, request);
    if (!result.Succeeded()) {
        return Fail("font glyph fallback resolve failed");
    }

    if (!result.used_fallback || result.fallback_depth != 1U) {
        return Fail("font glyph fallback depth mismatch");
    }

    if (result.resolved_font_key != FONT_FALLBACK || result.glyph_lookup_count != 2U) {
        return Fail("font glyph fallback lookup counters mismatch");
    }

    if (result.fallback_lookup_count != 1U || result.paint_path_pack_count != 0U) {
        return Fail("font glyph fallback paint path counters mismatch");
    }

    return 0;
}

int UiCoreFontGlyphAtlasTextFixtureReportsMissingGlyph() {
    const std::array<UiStaticAtlasPageDesc, 1U> pages{
        MakePage(7U, 77U, 64U, 64U)};
    const std::array<UiFontAssetDesc, 1U> fonts{
        MakeFont(FONT_PRIMARY)};
    const std::array<UiFontGlyphDesc, 0U> glyphs{};
    const UiFontGlyphAtlasDesc atlas_desc{pages, fonts, glyphs};
    UiFontGlyphResolveRequest request;
    request.key = MakeGlyphKey(FONT_PRIMARY, CODEPOINT_A, 16U);

    UiFontGlyphAtlas atlas{};
    const UiFontGlyphResolveResult result = atlas.ResolveGlyph(atlas_desc, request);
    if (result.status != UiFontGlyphAtlasStatus::MissingGlyph) {
        return Fail("font glyph missing status mismatch");
    }

    if (result.glyph_lookup_count != 1U || result.fallback_lookup_count != 0U) {
        return Fail("font glyph missing lookup counters mismatch");
    }

    if (result.paint_path_pack_count != 0U || result.atlas_repack_count != 0U) {
        return Fail("font glyph missing touched pack counters");
    }

    return 0;
}

int UiCoreFontGlyphAtlasRejectsPaintPathPackAndDuplicateGlyph() {
    const std::array<UiStaticAtlasPageDesc, 1U> pages{
        MakePage(7U, 77U, 64U, 64U)};
    const std::array<UiFontGlyphBuildRequest, 1U> requests{
        MakeBuildRequest(FONT_PRIMARY, CODEPOINT_A, 1001U, 8U, 16U)};
    std::array<UiStaticAtlasSpriteDesc, 1U> sprites{SentinelSprite()};
    UiFontGlyphBuildResult build_result{};
    UiFontGlyphAtlas atlas{};
    const UiFontGlyphBuildDesc build_desc{pages, UiDynamicAtlasPackPhase::PaintPath, 0U};
    UiFontGlyphAtlasStatus status = atlas.BuildGlyphSprites(build_desc, requests, sprites, &build_result);
    if (status != UiFontGlyphAtlasStatus::PaintPathPackRejected) {
        return Fail("font glyph accepted paint-path pack");
    }

    if (build_result.paint_path_pack_count != 1U || !SpriteMatchesSentinel(sprites[0U])) {
        return Fail("font glyph paint-path rejection result mismatch");
    }

    const UiFontGlyphBuildDesc safe_point_build_desc{pages, UiDynamicAtlasPackPhase::SafePoint, 0U};
    const std::array<UiFontGlyphBuildRequest, 2U> invalid_requests{
        MakeBuildRequest(FONT_PRIMARY, CODEPOINT_A, 3001U, 8U, 16U),
        MakeBuildRequest(FONT_PRIMARY, CODEPOINT_B, 3002U, 0U, 16U)};
    std::array<UiStaticAtlasSpriteDesc, 2U> invalid_sprites{
        SentinelSprite(),
        SentinelSprite()};
    build_result = UiFontGlyphBuildResult{};
    status = atlas.BuildGlyphSprites(safe_point_build_desc, invalid_requests, invalid_sprites, &build_result);
    if (status != UiFontGlyphAtlasStatus::InvalidGlyphMetadata) {
        return Fail("font glyph accepted invalid build request");
    }

    if (build_result.failed_codepoint != CODEPOINT_B) {
        return Fail("font glyph invalid build request codepoint mismatch");
    }

    if (!SpriteMatchesSentinel(invalid_sprites[0U]) || !SpriteMatchesSentinel(invalid_sprites[1U])) {
        return Fail("font glyph invalid build request mutated output");
    }

    const std::array<UiFontGlyphBuildRequest, 2U> duplicate_sprite_requests{
        MakeBuildRequest(FONT_PRIMARY, CODEPOINT_A, 4001U, 8U, 16U),
        MakeBuildRequest(FONT_PRIMARY, CODEPOINT_B, 4001U, 8U, 16U)};
    std::array<UiStaticAtlasSpriteDesc, 2U> duplicate_sprite_sprites{
        SentinelSprite(),
        SentinelSprite()};
    build_result = UiFontGlyphBuildResult{};
    status = atlas.BuildGlyphSprites(safe_point_build_desc, duplicate_sprite_requests, duplicate_sprite_sprites, &build_result);
    if (status != UiFontGlyphAtlasStatus::DuplicateGlyph) {
        return Fail("font glyph accepted duplicate sprite build request");
    }

    if (build_result.failed_codepoint != CODEPOINT_B) {
        return Fail("font glyph duplicate sprite codepoint mismatch");
    }

    if (!SpriteMatchesSentinel(duplicate_sprite_sprites[0U]) || !SpriteMatchesSentinel(duplicate_sprite_sprites[1U])) {
        return Fail("font glyph duplicate sprite mutated output");
    }

    const std::array<UiFontGlyphBuildRequest, 2U> duplicate_glyph_requests{
        MakeBuildRequest(FONT_PRIMARY, CODEPOINT_A, 5001U, 8U, 16U),
        MakeBuildRequest(FONT_PRIMARY, CODEPOINT_A, 5002U, 8U, 16U)};
    std::array<UiStaticAtlasSpriteDesc, 2U> duplicate_glyph_sprites{
        SentinelSprite(),
        SentinelSprite()};
    build_result = UiFontGlyphBuildResult{};
    status = atlas.BuildGlyphSprites(safe_point_build_desc, duplicate_glyph_requests, duplicate_glyph_sprites, &build_result);
    if (status != UiFontGlyphAtlasStatus::DuplicateGlyph) {
        return Fail("font glyph accepted duplicate glyph build request");
    }

    if (build_result.failed_codepoint != CODEPOINT_A) {
        return Fail("font glyph duplicate glyph codepoint mismatch");
    }

    if (!SpriteMatchesSentinel(duplicate_glyph_sprites[0U]) || !SpriteMatchesSentinel(duplicate_glyph_sprites[1U])) {
        return Fail("font glyph duplicate glyph mutated output");
    }

    UiStaticAtlasSpriteDesc sprite;
    sprite.sprite_key = 2001U;
    sprite.page_key = 7U;
    sprite.width = 8U;
    sprite.height = 16U;
    const std::array<UiFontAssetDesc, 1U> fonts{
        MakeFont(FONT_PRIMARY)};
    const std::array<UiFontGlyphDesc, 2U> glyphs{
        MakeGlyph(MakeGlyphKey(FONT_PRIMARY, CODEPOINT_A, 16U), sprite, 9),
        MakeGlyph(MakeGlyphKey(FONT_PRIMARY, CODEPOINT_A, 16U), sprite, 9)};
    const UiFontGlyphAtlasDesc atlas_desc{pages, fonts, glyphs};
    status = atlas.Validate(atlas_desc);
    if (status != UiFontGlyphAtlasStatus::DuplicateGlyph) {
        return Fail("font glyph did not reject duplicate glyph metadata");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_SAFE_POINT) {
        return UiCoreFontGlyphAtlasSafePointBuildResolvesPrimaryGlyph();
    }

    if (name == TEST_FALLBACK) {
        return UiCoreFontGlyphAtlasTextFixtureResolvesFallbackGlyph();
    }

    if (name == TEST_MISSING) {
        return UiCoreFontGlyphAtlasTextFixtureReportsMissingGlyph();
    }

    if (name == TEST_PAINT_PATH) {
        return UiCoreFontGlyphAtlasRejectsPaintPathPackAndDuplicateGlyph();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
