// 模块: Tests UiCore
// 文件: Tests/UiCore/UiDynamicAtlasTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiCore/UiDynamicAtlasPacker.h"
#include "YuEngine/UiCore/UiStaticAtlasMetadata.h"

using UiDynamicAtlasPacker = yuengine::uicore::UiDynamicAtlasPacker;
using UiDynamicAtlasPackDesc = yuengine::uicore::UiDynamicAtlasPackDesc;
using UiDynamicAtlasPackPhase = yuengine::uicore::UiDynamicAtlasPackPhase;
using UiDynamicAtlasPackResult = yuengine::uicore::UiDynamicAtlasPackResult;
using UiDynamicAtlasSpriteRequest = yuengine::uicore::UiDynamicAtlasSpriteRequest;
using yuengine::uicore::UiDynamicAtlasStatus;
using UiStaticAtlasMetadata = yuengine::uicore::UiStaticAtlasMetadata;
using UiStaticAtlasMetadataDesc = yuengine::uicore::UiStaticAtlasMetadataDesc;
using UiStaticAtlasPageDesc = yuengine::uicore::UiStaticAtlasPageDesc;
using UiStaticAtlasResolveResult = yuengine::uicore::UiStaticAtlasResolveResult;
using UiStaticAtlasSpriteDesc = yuengine::uicore::UiStaticAtlasSpriteDesc;

namespace {
constexpr const char *TEST_SAFE_POINT =
    "UiCore_DynamicAtlasPacker_SafePointPacksAndResolves";
constexpr const char *TEST_PAINT_PATH =
    "UiCore_DynamicAtlasPacker_RejectsPaintPathPacking";
constexpr const char *TEST_OVERFLOW =
    "UiCore_DynamicAtlasPacker_ReportsOverflowWithoutMutation";
constexpr const char *TEST_SMALL_OUTPUT =
    "UiCore_DynamicAtlasPacker_RejectsSmallOutputAndInvalidRequest";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t SENTINEL_SPRITE_KEY = 9999U;

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

UiDynamicAtlasSpriteRequest MakeRequest(
    std::uint32_t sprite_key,
    std::uint32_t width,
    std::uint32_t height) {
    UiDynamicAtlasSpriteRequest request;
    request.sprite_key = sprite_key;
    request.width = width;
    request.height = height;
    return request;
}

UiStaticAtlasSpriteDesc SentinelSprite() {
    UiStaticAtlasSpriteDesc sprite;
    sprite.sprite_key = SENTINEL_SPRITE_KEY;
    sprite.page_key = SENTINEL_SPRITE_KEY;
    sprite.x = SENTINEL_SPRITE_KEY;
    return sprite;
}

bool SpriteMatchesSentinel(const UiStaticAtlasSpriteDesc &sprite) {
    if (sprite.sprite_key != SENTINEL_SPRITE_KEY) {
        return false;
    }

    if (sprite.page_key != SENTINEL_SPRITE_KEY) {
        return false;
    }

    return sprite.x == SENTINEL_SPRITE_KEY;
}

bool FloatClose(float left, float right) {
    float diff = left - right;
    if (diff < 0.0F) {
        diff = -diff;
    }

    return diff < 0.0001F;
}

int UiCoreDynamicAtlasPackerSafePointPacksAndResolves() {
    const std::array<UiStaticAtlasPageDesc, 1U> pages{
        MakePage(7U, 77U, 64U, 64U)};
    std::array<UiDynamicAtlasSpriteRequest, 3U> requests{
        MakeRequest(11U, 32U, 16U),
        MakeRequest(12U, 16U, 16U),
        MakeRequest(13U, 32U, 32U)};
    requests[0U].nine_slice_enabled = true;
    requests[0U].border_left = 4U;
    requests[0U].border_top = 2U;
    requests[0U].border_right = 4U;
    requests[0U].border_bottom = 2U;

    std::array<UiStaticAtlasSpriteDesc, 3U> sprites{};
    UiDynamicAtlasPackResult pack_result{};
    UiDynamicAtlasPacker packer{};
    const UiDynamicAtlasPackDesc pack_desc{pages, UiDynamicAtlasPackPhase::SafePoint, 0U};
    const UiDynamicAtlasStatus pack_status = packer.Pack(pack_desc, requests, sprites, &pack_result);
    if (pack_status != UiDynamicAtlasStatus::Success || !pack_result.Succeeded()) {
        return Fail("dynamic atlas safe-point pack failed");
    }

    if (pack_result.allocation_count != 3U || pack_result.used_page_count != 1U) {
        return Fail("dynamic atlas pack result counters mismatch");
    }

    if (sprites[0U].x != 0U || sprites[1U].x != 32U || sprites[2U].y != 16U) {
        return Fail("dynamic atlas shelf positions mismatch");
    }

    if (!sprites[0U].nine_slice_enabled || sprites[0U].border_left != 4U || sprites[0U].border_right != 4U) {
        return Fail("dynamic atlas did not preserve nine-slice metadata");
    }

    UiStaticAtlasMetadata metadata{};
    const UiStaticAtlasMetadataDesc metadata_desc{pages, sprites};
    const UiStaticAtlasResolveResult resolve_result = metadata.ResolveSprite(metadata_desc, 13U);
    if (!resolve_result.Succeeded()) {
        return Fail("dynamic atlas output did not resolve through static metadata");
    }

    if (resolve_result.page_key != 7U || resolve_result.texture_key != 77U) {
        return Fail("dynamic atlas resolve page or texture mismatch");
    }

    if (!FloatClose(resolve_result.uv_rect.u_min, 0.0F) || !FloatClose(resolve_result.uv_rect.v_min, 0.25F)) {
        return Fail("dynamic atlas resolve uv min mismatch");
    }

    if (!FloatClose(resolve_result.uv_rect.u_max, 0.5F) || !FloatClose(resolve_result.uv_rect.v_max, 0.75F)) {
        return Fail("dynamic atlas resolve uv max mismatch");
    }

    return 0;
}

int UiCoreDynamicAtlasPackerRejectsPaintPathPacking() {
    const std::array<UiStaticAtlasPageDesc, 1U> pages{
        MakePage(7U, 77U, 64U, 64U)};
    const std::array<UiDynamicAtlasSpriteRequest, 1U> requests{
        MakeRequest(11U, 16U, 16U)};
    std::array<UiStaticAtlasSpriteDesc, 1U> sprites{SentinelSprite()};
    UiDynamicAtlasPackResult result{};
    UiDynamicAtlasPacker packer{};
    const UiDynamicAtlasPackDesc desc{pages, UiDynamicAtlasPackPhase::PaintPath, 0U};
    const UiDynamicAtlasStatus status = packer.Pack(desc, requests, sprites, &result);
    if (status != UiDynamicAtlasStatus::InvalidPackPhase) {
        return Fail("dynamic atlas accepted paint-path packing");
    }

    if (!SpriteMatchesSentinel(sprites[0U])) {
        return Fail("dynamic atlas mutated output after paint-path rejection");
    }

    return 0;
}

int UiCoreDynamicAtlasPackerReportsOverflowWithoutMutation() {
    const std::array<UiStaticAtlasPageDesc, 1U> pages{
        MakePage(7U, 77U, 32U, 32U)};
    const std::array<UiDynamicAtlasSpriteRequest, 2U> requests{
        MakeRequest(11U, 32U, 32U),
        MakeRequest(12U, 32U, 32U)};
    std::array<UiStaticAtlasSpriteDesc, 2U> sprites{SentinelSprite(), SentinelSprite()};
    UiDynamicAtlasPackResult result{};
    UiDynamicAtlasPacker packer{};
    const UiDynamicAtlasPackDesc desc{pages, UiDynamicAtlasPackPhase::SafePoint, 0U};
    const UiDynamicAtlasStatus status = packer.Pack(desc, requests, sprites, &result);
    if (status != UiDynamicAtlasStatus::AtlasCapacityExceeded) {
        return Fail("dynamic atlas did not report atlas capacity overflow");
    }

    if (result.failed_sprite_key != 12U || result.required_allocation_count != 2U) {
        return Fail("dynamic atlas overflow result mismatch");
    }

    if (!SpriteMatchesSentinel(sprites[0U]) || !SpriteMatchesSentinel(sprites[1U])) {
        return Fail("dynamic atlas mutated output after overflow");
    }

    return 0;
}

int UiCoreDynamicAtlasPackerRejectsSmallOutputAndInvalidRequest() {
    const std::array<UiStaticAtlasPageDesc, 1U> pages{
        MakePage(7U, 77U, 64U, 64U)};
    const std::array<UiDynamicAtlasSpriteRequest, 2U> requests{
        MakeRequest(11U, 16U, 16U),
        MakeRequest(12U, 16U, 16U)};
    std::array<UiStaticAtlasSpriteDesc, 1U> sprites{SentinelSprite()};
    UiDynamicAtlasPackResult result{};
    UiDynamicAtlasPacker packer{};
    const UiDynamicAtlasPackDesc desc{pages, UiDynamicAtlasPackPhase::SafePoint, 0U};
    UiDynamicAtlasStatus status = packer.Pack(desc, requests, sprites, &result);
    if (status != UiDynamicAtlasStatus::OutputCapacityExceeded) {
        return Fail("dynamic atlas accepted undersized output storage");
    }

    if (result.required_allocation_count != 2U || !SpriteMatchesSentinel(sprites[0U])) {
        return Fail("dynamic atlas small output result mismatch");
    }

    UiDynamicAtlasSpriteRequest invalid_request = MakeRequest(13U, 16U, 16U);
    invalid_request.nine_slice_enabled = true;
    invalid_request.border_left = 12U;
    invalid_request.border_right = 12U;
    const std::array<UiDynamicAtlasSpriteRequest, 1U> invalid_requests{invalid_request};
    status = packer.Pack(desc, invalid_requests, sprites, &result);
    if (status != UiDynamicAtlasStatus::InvalidSpriteRequest) {
        return Fail("dynamic atlas accepted invalid request metadata");
    }

    if (result.failed_sprite_key != 13U || !SpriteMatchesSentinel(sprites[0U])) {
        return Fail("dynamic atlas invalid request diagnostics changed");
    }

    const std::array<UiDynamicAtlasSpriteRequest, 2U> duplicate_requests{
        MakeRequest(15U, 16U, 16U),
        MakeRequest(15U, 8U, 8U)};
    status = packer.Pack(desc, duplicate_requests, sprites, &result);
    if (status != UiDynamicAtlasStatus::DuplicateSprite) {
        return Fail("dynamic atlas accepted duplicate sprite key");
    }

    if (result.failed_sprite_key != 15U || !SpriteMatchesSentinel(sprites[0U])) {
        return Fail("dynamic atlas duplicate request diagnostics changed");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_SAFE_POINT) {
        return UiCoreDynamicAtlasPackerSafePointPacksAndResolves();
    }

    if (name == TEST_PAINT_PATH) {
        return UiCoreDynamicAtlasPackerRejectsPaintPathPacking();
    }

    if (name == TEST_OVERFLOW) {
        return UiCoreDynamicAtlasPackerReportsOverflowWithoutMutation();
    }

    if (name == TEST_SMALL_OUTPUT) {
        return UiCoreDynamicAtlasPackerRejectsSmallOutputAndInvalidRequest();
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
