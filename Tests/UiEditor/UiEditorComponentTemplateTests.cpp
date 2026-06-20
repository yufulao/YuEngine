// 模块: Tests UiEditor
// 文件: Tests/UiEditor/UiEditorComponentTemplateTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string>
#include <string_view>

#include "YuEngine/UiCore/UiStaticAtlasMetadata.h"
#include "YuEngine/UiEditor/UiEditorComponentTemplate.h"
#include "YuEngine/UiEditor/UiEditorIdEventValidator.h"
#include "YuEngine/UiEditor/UiEditorResourceReferenceValidator.h"
#include "YuEngine/UiEditor/UiEditorShellState.h"

using yuengine::uicore::UiStaticAtlasMetadataDesc;
using yuengine::uicore::UiStaticAtlasPageDesc;
using yuengine::uicore::UiStaticAtlasSpriteDesc;
using yuengine::uieditor::UiEditorComponentTemplateDefaultState;
using yuengine::uieditor::UiEditorComponentTemplateDesc;
using yuengine::uieditor::UiEditorComponentTemplateFactory;
using yuengine::uieditor::UiEditorComponentTemplateKind;
using yuengine::uieditor::UiEditorComponentTemplateRecord;
using yuengine::uieditor::UiEditorComponentTemplateResult;
using yuengine::uieditor::UiEditorComponentTemplateStatus;
using yuengine::uieditor::UiEditorEventBinding;
using yuengine::uieditor::UiEditorIdEventValidationRecord;
using yuengine::uieditor::UiEditorIdEventValidationResult;
using yuengine::uieditor::UiEditorIdEventValidationStatus;
using yuengine::uieditor::UiEditorIdEventValidator;
using yuengine::uieditor::UiEditorIdValidationNode;
using yuengine::uieditor::UiEditorLayoutNodeRecord;
using yuengine::uieditor::UiEditorResourceCatalog;
using yuengine::uieditor::UiEditorResourceReference;
using yuengine::uieditor::UiEditorResourceReferenceKind;
using yuengine::uieditor::UiEditorResourceReferenceValidator;
using yuengine::uieditor::UiEditorResourceValidationRecord;
using yuengine::uieditor::UiEditorResourceValidationResult;
using yuengine::uieditor::UiEditorResourceValidationStatus;
using yuengine::uieditor::UiEditorShellState;
using yuengine::uieditor::UiEditorShellStatus;
using yuengine::uieditor::UI_EDITOR_LAYOUT_MAX_NODE_COUNT;

namespace {
constexpr const char *TEST_CREATE_ALL =
    "UiEditor_ComponentTemplate_CreatesAllValidLayoutNodes";
constexpr const char *TEST_INVALID_TEMPLATE =
    "UiEditor_ComponentTemplate_RejectsInvalidTemplateWithoutMutation";
constexpr const char *TEST_SMALL_OUTPUT =
    "UiEditor_ComponentTemplate_RejectsInsufficientOutputWithoutMutation";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t ROOT_NODE_ID = 1U;
constexpr std::uint32_t FIRST_TEMPLATE_NODE_ID = 10U;
constexpr std::uint32_t DEFAULT_FONT_KEY = 1001U;
constexpr std::uint32_t DEFAULT_LOCALIZATION_KEY = 2001U;
constexpr std::uint32_t DEFAULT_SPRITE_KEY = 3001U;
constexpr std::uint32_t DEFAULT_STYLE_KEY = 4001U;
constexpr std::uint32_t SENTINEL_NODE_ID = 777U;
constexpr std::uint32_t SENTINEL_RESOURCE_KEY = 888U;
constexpr std::uint32_t SENTINEL_EVENT_ID = 999U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

UiEditorComponentTemplateDesc MakeDesc(
    UiEditorComponentTemplateKind kind,
    std::uint32_t node_id,
    std::uint32_t order) {
    UiEditorComponentTemplateDesc desc{};
    desc.kind = kind;
    desc.node_id = node_id;
    desc.parent_node_id = ROOT_NODE_ID;
    desc.order = order;
    return desc;
}

UiStaticAtlasPageDesc MakeAtlasPage() {
    UiStaticAtlasPageDesc desc{};
    desc.page_key = 1U;
    desc.texture_key = 100U;
    desc.width = 256U;
    desc.height = 256U;
    return desc;
}

UiStaticAtlasSpriteDesc MakeAtlasSprite() {
    UiStaticAtlasSpriteDesc desc{};
    desc.sprite_key = DEFAULT_SPRITE_KEY;
    desc.page_key = 1U;
    desc.width = 32U;
    desc.height = 32U;
    return desc;
}

UiEditorResourceCatalog MakeCatalog(
    const std::array<UiStaticAtlasPageDesc, 1U> &pages,
    const std::array<UiStaticAtlasSpriteDesc, 1U> &sprites,
    const std::array<std::uint32_t, 1U> &font_keys,
    const std::array<std::uint32_t, 1U> &style_keys,
    const std::array<std::uint32_t, 1U> &localization_keys) {
    UiEditorResourceCatalog catalog{};
    catalog.static_atlas = UiStaticAtlasMetadataDesc{pages, sprites};
    catalog.font_keys = font_keys;
    catalog.style_keys = style_keys;
    catalog.localization_keys = localization_keys;
    return catalog;
}

UiEditorComponentTemplateRecord SentinelRecord() {
    UiEditorComponentTemplateRecord record{};
    record.kind = UiEditorComponentTemplateKind::GridView;
    record.default_state = UiEditorComponentTemplateDefaultState::GridViewReady;
    record.layout_node.node_id = SENTINEL_NODE_ID;
    record.layout_node.parent_node_id = ROOT_NODE_ID;
    record.layout_node.order = 9U;
    return record;
}

UiEditorResourceReference SentinelResource() {
    UiEditorResourceReference reference{};
    reference.node_id = SENTINEL_NODE_ID;
    reference.kind = UiEditorResourceReferenceKind::Style;
    reference.key = SENTINEL_RESOURCE_KEY;
    return reference;
}

UiEditorEventBinding SentinelEvent() {
    UiEditorEventBinding event_binding{};
    event_binding.node_id = SENTINEL_NODE_ID;
    event_binding.event_id = SENTINEL_EVENT_ID;
    event_binding.event_name = "SentinelEvent";
    return event_binding;
}

bool RecordMatchesSentinel(const UiEditorComponentTemplateRecord &record) {
    if (record.kind != UiEditorComponentTemplateKind::GridView) {
        return false;
    }

    if (record.default_state != UiEditorComponentTemplateDefaultState::GridViewReady) {
        return false;
    }

    return record.layout_node.node_id == SENTINEL_NODE_ID;
}

bool ResourceMatchesSentinel(const UiEditorResourceReference &reference) {
    if (reference.node_id != SENTINEL_NODE_ID) {
        return false;
    }

    if (reference.key != SENTINEL_RESOURCE_KEY) {
        return false;
    }

    return reference.kind == UiEditorResourceReferenceKind::Style;
}

bool EventMatchesSentinel(const UiEditorEventBinding &event_binding) {
    if (event_binding.node_id != SENTINEL_NODE_ID) {
        return false;
    }

    if (event_binding.event_id != SENTINEL_EVENT_ID) {
        return false;
    }

    return event_binding.event_name == "SentinelEvent";
}

std::string LayoutNodeJson(const UiEditorLayoutNodeRecord &node) {
    std::string text = "{ \"nodeId\": ";
    text += std::to_string(node.node_id);
    text += ", \"name\": \"";
    text += node.name;
    text += "\", \"type\": \"";
    text += node.type;
    text += "\", \"parentNodeId\": ";
    text += std::to_string(node.parent_node_id);
    text += ", \"order\": ";
    text += std::to_string(node.order);
    text += " }";
    return text;
}

std::string BuildLayoutAsset(std::span<const UiEditorComponentTemplateRecord> records) {
    std::string text = "{\n";
    text += "  \"schema\": \"YuEngine.UI.Layout\",\n";
    text += "  \"version\": 1,\n";
    text += "  \"layoutId\": \"UiEditor.ComponentTemplates\",\n";
    text += "  \"rootNodeId\": 1,\n";
    text += "  \"nodes\": [\n";
    text += "    { \"nodeId\": 1, \"name\": \"Root\", \"type\": \"Root\", \"parentNodeId\": 0, \"order\": 0 }";
    for (const UiEditorComponentTemplateRecord &record : records) {
        text += ",\n    ";
        text += LayoutNodeJson(record.layout_node);
    }

    text += "\n  ],\n";
    text += "  \"resources\": []\n";
    text += "}";
    return text;
}

int ValidateLayoutLoad(std::span<const UiEditorComponentTemplateRecord> records) {
    const std::string layout_text = BuildLayoutAsset(records);
    UiEditorShellState shell_state;
    const UiEditorShellStatus status = shell_state.LoadLayoutAsset(layout_text);
    if (status != UiEditorShellStatus::Success) {
        return Fail("component template layout asset did not load");
    }

    std::array<UiEditorLayoutNodeRecord, UI_EDITOR_LAYOUT_MAX_NODE_COUNT> nodes{};
    std::uint32_t node_count = 0U;
    const UiEditorShellStatus export_status = shell_state.ExportHierarchyNodes(
        nodes.data(),
        static_cast<std::uint32_t>(nodes.size()),
        &node_count);
    if (export_status != UiEditorShellStatus::Success) {
        return Fail("component template hierarchy export failed");
    }

    const std::uint32_t expected_count = static_cast<std::uint32_t>(records.size()) + 1U;
    if (node_count != expected_count) {
        return Fail("component template layout node count mismatch");
    }

    return 0;
}

int ValidateIdEvents(
    std::span<const UiEditorComponentTemplateRecord> records,
    std::span<const UiEditorEventBinding> events) {
    std::array<UiEditorIdValidationNode, 5U> nodes{};
    for (std::size_t index = 0U; index < records.size(); ++index) {
        nodes[index].node_id = records[index].layout_node.node_id;
    }

    std::array<UiEditorIdEventValidationRecord, 1U> reports{};
    UiEditorIdEventValidationResult result{};
    const UiEditorIdEventValidator validator{};
    const UiEditorIdEventValidationStatus status = validator.Validate(nodes, events, reports, &result);
    if (status != UiEditorIdEventValidationStatus::Success || !result.Succeeded()) {
        return Fail("component template id event output did not validate");
    }

    if (result.checked_node_count != static_cast<std::uint32_t>(records.size())) {
        return Fail("component template id validation node count mismatch");
    }

    if (result.checked_event_count != static_cast<std::uint32_t>(events.size())) {
        return Fail("component template event validation count mismatch");
    }

    return 0;
}

int ValidateResources(std::span<const UiEditorResourceReference> resources) {
    const std::array<UiStaticAtlasPageDesc, 1U> pages{MakeAtlasPage()};
    const std::array<UiStaticAtlasSpriteDesc, 1U> sprites{MakeAtlasSprite()};
    const std::array<std::uint32_t, 1U> font_keys{DEFAULT_FONT_KEY};
    const std::array<std::uint32_t, 1U> style_keys{DEFAULT_STYLE_KEY};
    const std::array<std::uint32_t, 1U> localization_keys{DEFAULT_LOCALIZATION_KEY};
    const UiEditorResourceCatalog catalog = MakeCatalog(pages, sprites, font_keys, style_keys, localization_keys);
    std::array<UiEditorResourceValidationRecord, 1U> reports{};
    UiEditorResourceValidationResult result{};
    const UiEditorResourceReferenceValidator validator{};
    const UiEditorResourceValidationStatus status = validator.Validate(resources, catalog, reports, &result);
    if (status != UiEditorResourceValidationStatus::Success || !result.Succeeded()) {
        return Fail("component template resource references did not validate");
    }

    if (result.checked_reference_count != static_cast<std::uint32_t>(resources.size())) {
        return Fail("component template resource validation count mismatch");
    }

    return 0;
}

int UiEditorComponentTemplateCreatesAllValidLayoutNodes() {
    constexpr std::array<UiEditorComponentTemplateKind, 5U> kinds{
        UiEditorComponentTemplateKind::Text,
        UiEditorComponentTemplateKind::Image,
        UiEditorComponentTemplateKind::Button,
        UiEditorComponentTemplateKind::Slider,
        UiEditorComponentTemplateKind::GridView};
    constexpr std::array<UiEditorComponentTemplateDefaultState, 5U> expected_states{
        UiEditorComponentTemplateDefaultState::TextReady,
        UiEditorComponentTemplateDefaultState::ImageReady,
        UiEditorComponentTemplateDefaultState::ButtonNormal,
        UiEditorComponentTemplateDefaultState::SliderIdle,
        UiEditorComponentTemplateDefaultState::GridViewReady};

    std::array<UiEditorComponentTemplateRecord, 5U> records{};
    std::array<UiEditorResourceReference, 9U> resources{};
    std::array<UiEditorEventBinding, 3U> events{};
    std::uint32_t resource_count = 0U;
    std::uint32_t event_count = 0U;
    const UiEditorComponentTemplateFactory factory{};

    for (std::size_t index = 0U; index < kinds.size(); ++index) {
        const std::uint32_t node_id = FIRST_TEMPLATE_NODE_ID + static_cast<std::uint32_t>(index);
        const UiEditorComponentTemplateDesc desc = MakeDesc(kinds[index], node_id, static_cast<std::uint32_t>(index));
        const std::size_t resource_offset = static_cast<std::size_t>(resource_count);
        const std::size_t event_offset = static_cast<std::size_t>(event_count);
        const std::size_t resource_remaining = resources.size() - resource_offset;
        const std::size_t event_remaining = events.size() - event_offset;
        std::span<UiEditorResourceReference> resource_span(resources.data() + resource_offset, resource_remaining);
        std::span<UiEditorEventBinding> event_span(events.data() + event_offset, event_remaining);
        UiEditorComponentTemplateResult result{};
        const UiEditorComponentTemplateStatus status = factory.Create(
            desc,
            &records[index],
            resource_span,
            event_span,
            &result);
        if (status != UiEditorComponentTemplateStatus::Success || !result.Succeeded()) {
            return Fail("component template creation failed");
        }

        if (records[index].kind != kinds[index] || records[index].default_state != expected_states[index]) {
            return Fail("component template default state mismatch");
        }

        if (records[index].layout_node.node_id != node_id || records[index].layout_node.parent_node_id != ROOT_NODE_ID) {
            return Fail("component template layout identity mismatch");
        }

        resource_count += result.written_resource_count;
        event_count += result.written_event_count;
    }

    if (resource_count != 9U || event_count != 3U) {
        return Fail("component template resource or event counts mismatch");
    }

    std::span<const UiEditorComponentTemplateRecord> record_span(records.data(), records.size());
    int ret_code = ValidateLayoutLoad(record_span);
    if (ret_code != 0) {
        return ret_code;
    }

    std::span<const UiEditorEventBinding> event_span(events.data(), static_cast<std::size_t>(event_count));
    ret_code = ValidateIdEvents(record_span, event_span);
    if (ret_code != 0) {
        return ret_code;
    }

    std::span<const UiEditorResourceReference> resource_span(resources.data(), static_cast<std::size_t>(resource_count));
    return ValidateResources(resource_span);
}

int UiEditorComponentTemplateRejectsInvalidTemplateWithoutMutation() {
    UiEditorComponentTemplateRecord record = SentinelRecord();
    std::array<UiEditorResourceReference, 2U> resources{SentinelResource(), SentinelResource()};
    std::array<UiEditorEventBinding, 1U> events{SentinelEvent()};
    UiEditorComponentTemplateResult result{};
    UiEditorComponentTemplateDesc desc = MakeDesc(
        UiEditorComponentTemplateKind::Invalid,
        FIRST_TEMPLATE_NODE_ID,
        0U);

    const UiEditorComponentTemplateFactory factory{};
    const UiEditorComponentTemplateStatus status = factory.Create(desc, &record, resources, events, &result);
    if (status != UiEditorComponentTemplateStatus::InvalidTemplateKind) {
        return Fail("invalid component template kind was not rejected");
    }

    if (result.status != UiEditorComponentTemplateStatus::InvalidTemplateKind) {
        return Fail("invalid template result status mismatch");
    }

    if (!RecordMatchesSentinel(record)) {
        return Fail("invalid component template mutated record");
    }

    if (!ResourceMatchesSentinel(resources[0U]) || !EventMatchesSentinel(events[0U])) {
        return Fail("invalid component template mutated outputs");
    }

    return 0;
}

int UiEditorComponentTemplateRejectsInsufficientOutputWithoutMutation() {
    UiEditorComponentTemplateRecord record = SentinelRecord();
    std::array<UiEditorResourceReference, 1U> resources{SentinelResource()};
    std::array<UiEditorEventBinding, 1U> events{SentinelEvent()};
    UiEditorComponentTemplateResult result{};
    const UiEditorComponentTemplateDesc desc = MakeDesc(
        UiEditorComponentTemplateKind::Button,
        FIRST_TEMPLATE_NODE_ID,
        0U);

    const UiEditorComponentTemplateFactory factory{};
    const UiEditorComponentTemplateStatus status = factory.Create(desc, &record, resources, events, &result);
    if (status != UiEditorComponentTemplateStatus::OutputCapacityExceeded) {
        return Fail("insufficient component template output was not rejected");
    }

    if (result.required_resource_count != 2U || result.required_event_count != 1U) {
        return Fail("insufficient output did not report required counts");
    }

    if (!RecordMatchesSentinel(record)) {
        return Fail("insufficient output mutated record");
    }

    if (!ResourceMatchesSentinel(resources[0U]) || !EventMatchesSentinel(events[0U])) {
        return Fail("insufficient output mutated buffers");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_CREATE_ALL) {
        return UiEditorComponentTemplateCreatesAllValidLayoutNodes();
    }

    if (name == TEST_INVALID_TEMPLATE) {
        return UiEditorComponentTemplateRejectsInvalidTemplateWithoutMutation();
    }

    if (name == TEST_SMALL_OUTPUT) {
        return UiEditorComponentTemplateRejectsInsufficientOutputWithoutMutation();
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
