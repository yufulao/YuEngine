// 模块: Tests UiEditor
// 文件: Tests/UiEditor/UiEditorStatePreviewTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiCore/UiButtonState.h"
#include "YuEngine/UiEditor/UiEditorComponentTemplate.h"
#include "YuEngine/UiEditor/UiEditorStatePreview.h"

using yuengine::uicore::UiButtonState;
using yuengine::uieditor::UiEditorComponentTemplateDefaultState;
using yuengine::uieditor::UiEditorComponentTemplateDesc;
using yuengine::uieditor::UiEditorComponentTemplateFactory;
using yuengine::uieditor::UiEditorComponentTemplateKind;
using yuengine::uieditor::UiEditorComponentTemplateRecord;
using yuengine::uieditor::UiEditorComponentTemplateResult;
using yuengine::uieditor::UiEditorComponentTemplateStatus;
using yuengine::uieditor::UiEditorEventBinding;
using yuengine::uieditor::UiEditorResourceReference;
using yuengine::uieditor::UiEditorStatePreviewDesc;
using yuengine::uieditor::UiEditorStatePreviewFactory;
using yuengine::uieditor::UiEditorStatePreviewRecord;
using yuengine::uieditor::UiEditorStatePreviewResult;
using yuengine::uieditor::UiEditorStatePreviewStatus;

namespace {
constexpr const char *TEST_EXPORTS_STATES =
    "UiEditor_StatePreview_ExportsButtonStates";
constexpr const char *TEST_REJECTS_NON_BUTTON =
    "UiEditor_StatePreview_RejectsNonButtonTemplateWithoutMutation";
constexpr const char *TEST_REJECTS_INVALID_STATE =
    "UiEditor_StatePreview_RejectsInvalidStateWithoutMutation";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t ROOT_NODE_ID = 1U;
constexpr std::uint32_t BUTTON_NODE_ID = 10U;
constexpr std::uint32_t SENTINEL_NODE_ID = 777U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

UiEditorComponentTemplateDesc MakeButtonTemplateDesc() {
    UiEditorComponentTemplateDesc desc{};
    desc.kind = UiEditorComponentTemplateKind::Button;
    desc.node_id = BUTTON_NODE_ID;
    desc.parent_node_id = ROOT_NODE_ID;
    desc.order = 0U;
    return desc;
}

int CreateButtonTemplate(UiEditorComponentTemplateRecord *out_record) {
    if (out_record == nullptr) {
        return Fail("button template output was null");
    }

    std::array<UiEditorResourceReference, 2U> resources{};
    std::array<UiEditorEventBinding, 1U> events{};
    UiEditorComponentTemplateResult result{};
    const UiEditorComponentTemplateFactory factory{};
    const UiEditorComponentTemplateDesc desc = MakeButtonTemplateDesc();
    const UiEditorComponentTemplateStatus status = factory.Create(desc, out_record, resources, events, &result);
    if (status != UiEditorComponentTemplateStatus::Success || !result.Succeeded()) {
        return Fail("button component template setup failed");
    }

    return 0;
}

UiEditorStatePreviewRecord SentinelPreviewRecord() {
    UiEditorStatePreviewRecord record{};
    record.component_kind = UiEditorComponentTemplateKind::GridView;
    record.button_state = UiButtonState::Selected;
    record.node_id = SENTINEL_NODE_ID;
    record.state_badge_visible = false;
    record.disabled_overlay_visible = true;
    record.selected_outline_visible = true;
    record.interaction_enabled = false;
    record.state_name[0U] = 'X';
    return record;
}

bool PreviewRecordMatchesSentinel(const UiEditorStatePreviewRecord &record) {
    if (record.component_kind != UiEditorComponentTemplateKind::GridView) {
        return false;
    }

    if (record.button_state != UiButtonState::Selected) {
        return false;
    }

    if (record.node_id != SENTINEL_NODE_ID) {
        return false;
    }

    if (record.state_name[0U] != 'X') {
        return false;
    }

    if (record.state_badge_visible) {
        return false;
    }

    if (!record.disabled_overlay_visible || !record.selected_outline_visible) {
        return false;
    }

    return !record.interaction_enabled;
}

int VerifyStatePreview(
    const UiEditorComponentTemplateRecord &button_template,
    UiButtonState state,
    std::string_view expected_name,
    bool expected_disabled_overlay,
    bool expected_selected_outline,
    bool expected_interaction_enabled) {
    UiEditorStatePreviewDesc desc{};
    desc.component_template = button_template;
    desc.button_state = state;

    UiEditorStatePreviewRecord record{};
    UiEditorStatePreviewResult result{};
    const UiEditorStatePreviewFactory factory{};
    const UiEditorStatePreviewStatus status = factory.CreateButtonStatePreview(desc, &record, &result);
    if (status != UiEditorStatePreviewStatus::Success || !result.Succeeded()) {
        return Fail("state preview creation failed");
    }

    if (record.component_kind != UiEditorComponentTemplateKind::Button) {
        return Fail("state preview component kind mismatch");
    }

    if (record.node_id != BUTTON_NODE_ID || record.button_state != state) {
        return Fail("state preview identity mismatch");
    }

    if (std::string_view(record.state_name) != expected_name) {
        return Fail("state preview name mismatch");
    }

    if (!record.state_badge_visible) {
        return Fail("state preview badge was not visible");
    }

    if (record.disabled_overlay_visible != expected_disabled_overlay) {
        return Fail("state preview disabled overlay mismatch");
    }

    if (record.selected_outline_visible != expected_selected_outline) {
        return Fail("state preview selected outline mismatch");
    }

    if (record.interaction_enabled != expected_interaction_enabled) {
        return Fail("state preview interaction flag mismatch");
    }

    return 0;
}

int UiEditorStatePreviewExportsButtonStates() {
    UiEditorComponentTemplateRecord button_template{};
    int ret_code = CreateButtonTemplate(&button_template);
    if (ret_code != 0) {
        return ret_code;
    }

    ret_code = VerifyStatePreview(button_template, UiButtonState::Normal, "Normal", false, false, true);
    if (ret_code != 0) {
        return ret_code;
    }

    ret_code = VerifyStatePreview(button_template, UiButtonState::Hover, "Hover", false, false, true);
    if (ret_code != 0) {
        return ret_code;
    }

    ret_code = VerifyStatePreview(button_template, UiButtonState::Pressed, "Pressed", false, false, true);
    if (ret_code != 0) {
        return ret_code;
    }

    ret_code = VerifyStatePreview(button_template, UiButtonState::Disabled, "Disabled", true, false, false);
    if (ret_code != 0) {
        return ret_code;
    }

    return VerifyStatePreview(button_template, UiButtonState::Selected, "Selected", false, true, true);
}

int UiEditorStatePreviewRejectsNonButtonTemplateWithoutMutation() {
    UiEditorStatePreviewDesc desc{};
    desc.component_template.kind = UiEditorComponentTemplateKind::Text;
    desc.component_template.default_state = UiEditorComponentTemplateDefaultState::TextReady;
    desc.component_template.layout_node.node_id = BUTTON_NODE_ID;
    desc.button_state = UiButtonState::Normal;

    UiEditorStatePreviewRecord record = SentinelPreviewRecord();
    UiEditorStatePreviewResult result{};
    const UiEditorStatePreviewFactory factory{};
    const UiEditorStatePreviewStatus status = factory.CreateButtonStatePreview(desc, &record, &result);
    if (status != UiEditorStatePreviewStatus::InvalidTemplate) {
        return Fail("non button template was not rejected");
    }

    if (result.status != UiEditorStatePreviewStatus::InvalidTemplate) {
        return Fail("non button template result status mismatch");
    }

    if (!PreviewRecordMatchesSentinel(record)) {
        return Fail("non button template mutated preview record");
    }

    return 0;
}

int UiEditorStatePreviewRejectsInvalidStateWithoutMutation() {
    UiEditorComponentTemplateRecord button_template{};
    int ret_code = CreateButtonTemplate(&button_template);
    if (ret_code != 0) {
        return ret_code;
    }

    UiEditorStatePreviewDesc desc{};
    desc.component_template = button_template;
    desc.button_state = static_cast<UiButtonState>(99);

    UiEditorStatePreviewRecord record = SentinelPreviewRecord();
    UiEditorStatePreviewResult result{};
    const UiEditorStatePreviewFactory factory{};
    const UiEditorStatePreviewStatus status = factory.CreateButtonStatePreview(desc, &record, &result);
    if (status != UiEditorStatePreviewStatus::InvalidState) {
        return Fail("invalid button state was not rejected");
    }

    if (result.status != UiEditorStatePreviewStatus::InvalidState) {
        return Fail("invalid button state result status mismatch");
    }

    if (!PreviewRecordMatchesSentinel(record)) {
        return Fail("invalid button state mutated preview record");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_EXPORTS_STATES) {
        return UiEditorStatePreviewExportsButtonStates();
    }

    if (name == TEST_REJECTS_NON_BUTTON) {
        return UiEditorStatePreviewRejectsNonButtonTemplateWithoutMutation();
    }

    if (name == TEST_REJECTS_INVALID_STATE) {
        return UiEditorStatePreviewRejectsInvalidStateWithoutMutation();
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
