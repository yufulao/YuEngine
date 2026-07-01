// 模块: Tests UiCore
// 文件: Tests/UiCore/UiDrawBatcherTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/UiCore/UiDrawBatch.h"
#include "YuEngine/UiCore/UiDrawBatchKey.h"
#include "YuEngine/UiCore/UiDrawBatcher.h"
#include "YuEngine/UiCore/UiDrawBatchResult.h"
#include "YuEngine/UiCore/UiDrawBatchStatus.h"
#include "YuEngine/UiCore/UiDrawElement.h"
#include "YuEngine/UiCore/UiDrawElementType.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiRect.h"

using UiDrawBatch = yuengine::uicore::UiDrawBatch;
using UiDrawBatchKey = yuengine::uicore::UiDrawBatchKey;
using UiDrawBatcher = yuengine::uicore::UiDrawBatcher;
using UiDrawBatcherSnapshot = yuengine::uicore::UiDrawBatcherSnapshot;
using UiDrawBatchResult = yuengine::uicore::UiDrawBatchResult;
using yuengine::uicore::UiDrawBatchStatus;
using UiDrawElement = yuengine::uicore::UiDrawElement;
using yuengine::uicore::UiDrawElementType;
using UiNodeId = yuengine::uicore::UiNodeId;
using UiRect = yuengine::uicore::UiRect;

namespace {
constexpr const char *TEST_FIXTURE_BATCH_COUNT =
    "UiCore_DrawBatcher_DeterministicFixtureBatchCount";
constexpr const char *TEST_TEXT_TEXTURE_STATE =
    "UiCore_DrawBatcher_SplitsTextAndTextureState";
constexpr const char *TEST_INVALID_ELEMENT =
    "UiCore_DrawBatcher_RejectsInvalidElementWithoutMutation";
constexpr const char *TEST_SMALL_OUTPUT =
    "UiCore_DrawBatcher_RejectsSmallOutputWithoutMutation";
constexpr const char *TEST_OUTPUT_CAPACITY_ENTRY =
    "UiCore_DrawBatcher_OutputCapacityEntryReportsRejectedBatch";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t SENTINEL_INDEX = 777U;
constexpr std::uint32_t SENTINEL_COUNT = 888U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

UiDrawElement DrawElement(
    std::uint32_t node_id,
    UiDrawElementType type,
    std::int32_t layer,
    std::uint32_t material_key) {
    UiDrawElement element{};
    element.node_id = UiNodeId{node_id};
    element.type = type;
    element.rect = UiRect{0.0F, 0.0F, 100.0F, 50.0F};
    element.clip_rect = UiRect{0.0F, 0.0F, 100.0F, 50.0F};
    element.layer = layer;
    element.sibling_order = node_id;
    element.style_key = 3U;
    element.material_key = material_key;
    element.texture_key = 0U;
    element.text_key = 0U;
    element.scissor_enabled = false;
    return element;
}

UiDrawBatch SentinelBatch() {
    UiDrawBatch batch{};
    batch.first_element_index = SENTINEL_INDEX;
    batch.element_count = SENTINEL_COUNT;
    batch.key.material_key = SENTINEL_INDEX;
    return batch;
}

bool BatchMatchesSentinel(const UiDrawBatch &batch) {
    if (batch.first_element_index != SENTINEL_INDEX) {
        return false;
    }

    if (batch.element_count != SENTINEL_COUNT) {
        return false;
    }

    return batch.key.material_key == SENTINEL_INDEX;
}

bool RectMatches(const UiRect &left, const UiRect &right) {
    if (left.x != right.x) {
        return false;
    }

    if (left.y != right.y) {
        return false;
    }

    if (left.width != right.width) {
        return false;
    }

    return left.height == right.height;
}

bool BatchKeyMatchesElement(const UiDrawBatchKey &key, const UiDrawElement &element) {
    if (key.type != element.type) {
        return false;
    }

    if (key.layer != element.layer) {
        return false;
    }

    if (key.style_key != element.style_key) {
        return false;
    }

    if (key.material_key != element.material_key) {
        return false;
    }

    if (key.texture_key != element.texture_key) {
        return false;
    }

    if (key.text_key != element.text_key) {
        return false;
    }

    if (!RectMatches(key.clip_rect, element.clip_rect)) {
        return false;
    }

    return key.scissor_enabled == element.scissor_enabled;
}

void SeedStaleCapacityFailure(UiDrawBatchResult *result) {
    if (result == nullptr) {
        return;
    }

    result->failed_batch_element_index = 19U;
    result->failed_batch_node_id = UiNodeId{20U};
    result->failed_batch_key.material_key = 21U;
    result->failed_batch_key.texture_key = 22U;
    result->capacity_entry_output_index = 23U;
    result->capacity_entry_output_capacity = 24U;
    result->capacity_entry_written_batch_count = 25U;
    result->required_output_batch_count = 26U;
}

int ExpectSnapshotCapacityEntryCleared(const UiDrawBatcherSnapshot &snapshot) {
    if (snapshot.last_capacity_entry_output_index != 0U) {
        return Fail("snapshot retained capacity output index");
    }

    if (snapshot.last_capacity_entry_node_id.value != 0U) {
        return Fail("snapshot retained capacity node id");
    }

    if (snapshot.last_capacity_entry_key.material_key != 0U ||
        snapshot.last_capacity_entry_key.texture_key != 0U ||
        snapshot.last_capacity_entry_key.text_key != 0U) {
        return Fail("snapshot retained capacity key");
    }

    if (snapshot.last_capacity_entry_output_capacity != 0U ||
        snapshot.last_capacity_entry_written_batch_count != 0U ||
        snapshot.last_required_output_batch_count != 0U) {
        return Fail("snapshot retained capacity counts");
    }

    return 0;
}

int UiCoreDrawBatcherDeterministicFixtureBatchCount() {
    std::array<UiDrawElement, 6U> elements{};
    elements[0U] = DrawElement(1U, UiDrawElementType::Rect, 0, 11U);
    elements[1U] = DrawElement(2U, UiDrawElementType::Rect, 0, 11U);
    elements[2U] = DrawElement(3U, UiDrawElementType::Rect, 0, 11U);
    elements[2U].scissor_enabled = true;
    elements[2U].clip_rect = UiRect{0.0F, 0.0F, 50.0F, 50.0F};
    elements[3U] = elements[2U];
    elements[3U].node_id = UiNodeId{4U};
    elements[4U] = DrawElement(5U, UiDrawElementType::TexturedQuad, 0, 11U);
    elements[4U].texture_key = 23U;
    elements[5U] = elements[4U];
    elements[5U].node_id = UiNodeId{6U};
    elements[5U].layer = 1;

    std::array<UiDrawBatch, 6U> batches{};
    UiDrawBatchResult result{};
    SeedStaleCapacityFailure(&result);
    UiDrawBatcher batcher{};
    const UiDrawBatchStatus status = batcher.Build(elements, batches, &result);
    if (status != UiDrawBatchStatus::Success || !result.Succeeded()) {
        return Fail("batcher rejected valid fixture elements");
    }

    if (result.batch_count != 4U || result.draw_element_count != 6U) {
        return Fail("deterministic fixture batch count mismatch");
    }

    if (batches[0U].first_element_index != 0U || batches[0U].element_count != 2U) {
        return Fail("first batch did not merge matching rects");
    }

    if (batches[1U].first_element_index != 2U || batches[1U].element_count != 2U) {
        return Fail("second batch did not merge matching clipped rects");
    }

    if (batches[2U].first_element_index != 4U || batches[2U].element_count != 1U) {
        return Fail("textured batch boundary mismatch");
    }

    if (batches[3U].first_element_index != 5U || batches[3U].element_count != 1U) {
        return Fail("layer batch boundary mismatch");
    }

    if (batches[2U].key.texture_key != 23U || batches[3U].key.layer != 1) {
        return Fail("batch keys did not preserve texture or layer");
    }

    if (result.failed_element_index != 0U || result.failed_node_id.value != 0U) {
        return Fail("successful batch build retained stale invalid element identity");
    }

    if (result.failed_batch_element_index != 0U ||
        result.failed_batch_node_id.value != 0U ||
        result.failed_batch_key.material_key != 0U ||
        result.capacity_entry_output_index != 0U ||
        result.capacity_entry_output_capacity != 0U ||
        result.capacity_entry_written_batch_count != 0U ||
        result.required_output_batch_count != 0U) {
        return Fail("successful batch build retained stale capacity identity");
    }

    return 0;
}

int UiCoreDrawBatcherSplitsTextAndTextureState() {
    std::array<UiDrawElement, 4U> elements{};
    elements[0U] = DrawElement(1U, UiDrawElementType::Text, 0, 17U);
    elements[0U].text_key = 41U;
    elements[1U] = elements[0U];
    elements[1U].node_id = UiNodeId{2U};
    elements[2U] = elements[1U];
    elements[2U].node_id = UiNodeId{3U};
    elements[2U].text_key = 43U;
    elements[3U] = DrawElement(4U, UiDrawElementType::TexturedQuad, 0, 17U);
    elements[3U].texture_key = 41U;

    std::array<UiDrawBatch, 4U> batches{};
    UiDrawBatchResult result{};
    SeedStaleCapacityFailure(&result);
    UiDrawBatcher batcher{};
    const UiDrawBatchStatus status = batcher.Build(elements, batches, &result);
    if (status != UiDrawBatchStatus::Success) {
        return Fail("batcher rejected text and texture fixture");
    }

    if (result.batch_count != 3U) {
        return Fail("text and texture state batch count mismatch");
    }

    if (batches[0U].element_count != 2U || batches[1U].key.text_key != 43U) {
        return Fail("text batch state mismatch");
    }

    if (batches[2U].key.type != UiDrawElementType::TexturedQuad || batches[2U].key.texture_key != 41U) {
        return Fail("texture batch state mismatch");
    }

    return 0;
}

int UiCoreDrawBatcherRejectsInvalidElementWithoutMutation() {
    std::array<UiDrawElement, 1U> elements{};
    elements[0U] = DrawElement(0U, UiDrawElementType::Rect, 0, 11U);
    std::array<UiDrawBatch, 1U> batches{SentinelBatch()};
    UiDrawBatchResult result{};
    UiDrawBatcher batcher{};
    const UiDrawBatchStatus status = batcher.Build(elements, batches, &result);
    if (status != UiDrawBatchStatus::InvalidDrawElement) {
        return Fail("batcher accepted invalid element");
    }

    if (result.failed_element_index != 0U || result.failed_node_id.value != 0U) {
        return Fail("batcher did not report failed element");
    }

    if (result.failed_batch_node_id.value != 0U || result.failed_batch_key.material_key != 0U) {
        return Fail("invalid element retained stale capacity identity");
    }

    if (!BatchMatchesSentinel(batches[0U])) {
        return Fail("batcher mutated output after invalid element");
    }

    return 0;
}

int UiCoreDrawBatcherRejectsSmallOutputWithoutMutation() {
    std::array<UiDrawElement, 3U> elements{};
    elements[0U] = DrawElement(1U, UiDrawElementType::Rect, 0, 11U);
    elements[1U] = DrawElement(2U, UiDrawElementType::TexturedQuad, 0, 11U);
    elements[1U].texture_key = 23U;
    elements[2U] = DrawElement(3U, UiDrawElementType::Text, 0, 11U);
    elements[2U].text_key = 29U;

    std::array<UiDrawBatch, 3U> batches{SentinelBatch(), SentinelBatch(), SentinelBatch()};
    std::span<UiDrawBatch> limited_batches(batches.data(), 2U);
    UiDrawBatchResult result{};
    UiDrawBatcher batcher{};
    const UiDrawBatchStatus status = batcher.Build(elements, limited_batches, &result);
    if (status != UiDrawBatchStatus::OutputCapacityExceeded) {
        return Fail("batcher accepted undersized output storage");
    }

    if (result.batch_count != 3U) {
        return Fail("batcher did not report required batch count");
    }

    if (result.failed_batch_element_index != 2U || result.failed_batch_node_id.value != 3U) {
        return Fail("batcher did not report first rejected batch element");
    }

    if (!BatchKeyMatchesElement(result.failed_batch_key, elements[2U])) {
        return Fail("batcher did not report rejected batch key");
    }

    if (result.capacity_entry_output_index != 2U ||
        result.capacity_entry_output_capacity != 2U ||
        result.capacity_entry_written_batch_count != 2U ||
        result.required_output_batch_count != 3U) {
        return Fail("batcher did not report rejected batch capacity counts");
    }

    if (result.failed_element_index != 0U || result.failed_node_id.value != 0U) {
        return Fail("capacity failure retained invalid element identity");
    }

    if (!BatchMatchesSentinel(batches[0U]) ||
        !BatchMatchesSentinel(batches[1U]) ||
        !BatchMatchesSentinel(batches[2U])) {
        return Fail("batcher mutated output after undersized storage");
    }

    SeedStaleCapacityFailure(&result);
    std::span<UiDrawBatch> invalid_batches(static_cast<UiDrawBatch *>(nullptr), 3U);
    const UiDrawBatchStatus invalid_status = batcher.Build(elements, invalid_batches, &result);
    if (invalid_status != UiDrawBatchStatus::OutputCapacityExceeded) {
        return Fail("batcher did not reject invalid output storage");
    }

    if (result.failed_element_index != 0U || result.failed_node_id.value != 0U) {
        return Fail("invalid output storage retained invalid element identity");
    }

    if (result.failed_batch_element_index != 0U ||
        result.failed_batch_node_id.value != 0U ||
        result.failed_batch_key.material_key != 0U ||
        result.capacity_entry_output_index != 0U ||
        result.capacity_entry_output_capacity != 0U ||
        result.capacity_entry_written_batch_count != 0U ||
        result.required_output_batch_count != 0U) {
        return Fail("invalid output storage retained stale capacity identity");
    }

    return 0;
}

int UiCoreDrawBatcherOutputCapacityEntryReportsRejectedBatch() {
    std::array<UiDrawElement, 4U> elements{};
    elements[0U] = DrawElement(1U, UiDrawElementType::Rect, 0, 11U);
    elements[1U] = elements[0U];
    elements[1U].node_id = UiNodeId{2U};
    elements[2U] = DrawElement(3U, UiDrawElementType::TexturedQuad, 0, 13U);
    elements[2U].texture_key = 33U;
    elements[3U] = DrawElement(4U, UiDrawElementType::Text, 0, 17U);
    elements[3U].text_key = 44U;

    std::array<UiDrawBatch, 4U> batches{
        SentinelBatch(),
        SentinelBatch(),
        SentinelBatch(),
        SentinelBatch()};
    std::span<UiDrawBatch> limited_batches(batches.data(), 2U);
    UiDrawBatchResult result{};
    UiDrawBatcher batcher{};
    const UiDrawBatchStatus status = batcher.Build(elements, limited_batches, &result);
    if (status != UiDrawBatchStatus::OutputCapacityExceeded) {
        return Fail("batcher did not reject small output for capacity entry");
    }

    if (result.batch_count != 3U ||
        result.capacity_entry_output_index != 2U ||
        result.capacity_entry_output_capacity != 2U ||
        result.capacity_entry_written_batch_count != 2U ||
        result.required_output_batch_count != 3U) {
        return Fail("result did not record output capacity entry counts");
    }

    if (result.failed_batch_element_index != 3U || result.failed_batch_node_id.value != 4U) {
        return Fail("result did not record rejected output identity");
    }

    if (!BatchKeyMatchesElement(result.failed_batch_key, elements[3U])) {
        return Fail("result did not record rejected output data");
    }

    const UiDrawBatcherSnapshot capacity_snapshot = batcher.Snapshot();
    if (capacity_snapshot.last_status != UiDrawBatchStatus::OutputCapacityExceeded ||
        capacity_snapshot.draw_element_count != 4U ||
        capacity_snapshot.batch_count != 3U ||
        capacity_snapshot.failed_operation_count != 1U) {
        return Fail("snapshot did not record capacity failure status");
    }

    if (capacity_snapshot.last_capacity_entry_output_index != 2U ||
        capacity_snapshot.last_capacity_entry_output_capacity != 2U ||
        capacity_snapshot.last_capacity_entry_written_batch_count != 2U ||
        capacity_snapshot.last_required_output_batch_count != 3U) {
        return Fail("snapshot did not record output capacity entry counts");
    }

    if (capacity_snapshot.last_capacity_entry_node_id.value != 4U ||
        !BatchKeyMatchesElement(capacity_snapshot.last_capacity_entry_key, elements[3U])) {
        return Fail("snapshot did not record rejected output identity");
    }

    if (!BatchMatchesSentinel(batches[0U]) ||
        !BatchMatchesSentinel(batches[1U]) ||
        !BatchMatchesSentinel(batches[2U]) ||
        !BatchMatchesSentinel(batches[3U])) {
        return Fail("capacity entry failure mutated output batches");
    }

    std::array<UiDrawBatch, 4U> valid_batches{};
    const UiDrawBatchStatus success_status = batcher.Build(elements, valid_batches, &result);
    if (success_status != UiDrawBatchStatus::Success) {
        return Fail("batcher did not recover after capacity failure");
    }

    const UiDrawBatcherSnapshot success_snapshot = batcher.Snapshot();
    if (success_snapshot.accepted_operation_count != 1U ||
        success_snapshot.failed_operation_count != 1U ||
        success_snapshot.last_status != UiDrawBatchStatus::Success) {
        return Fail("snapshot did not record success after capacity failure");
    }

    int ret_code = ExpectSnapshotCapacityEntryCleared(success_snapshot);
    if (ret_code != 0) {
        return ret_code;
    }

    elements[0U].node_id = UiNodeId{};
    const UiDrawBatchStatus invalid_status = batcher.Build(elements, valid_batches, &result);
    if (invalid_status != UiDrawBatchStatus::InvalidDrawElement) {
        return Fail("invalid element did not fail after capacity recovery");
    }

    ret_code = ExpectSnapshotCapacityEntryCleared(batcher.Snapshot());
    if (ret_code != 0) {
        return ret_code;
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_FIXTURE_BATCH_COUNT) {
        return UiCoreDrawBatcherDeterministicFixtureBatchCount();
    }

    if (name == TEST_TEXT_TEXTURE_STATE) {
        return UiCoreDrawBatcherSplitsTextAndTextureState();
    }

    if (name == TEST_INVALID_ELEMENT) {
        return UiCoreDrawBatcherRejectsInvalidElementWithoutMutation();
    }

    if (name == TEST_SMALL_OUTPUT) {
        return UiCoreDrawBatcherRejectsSmallOutputWithoutMutation();
    }

    if (name == TEST_OUTPUT_CAPACITY_ENTRY) {
        return UiCoreDrawBatcherOutputCapacityEntryReportsRejectedBatch();
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
