// 模块: Tools UiEditor
// 文件: Tools/UiEditor/Src/UiEditorShellState.cpp

#include "YuEngine/UiEditor/UiEditorShellState.h"

#include <charconv>
#include <cstring>

namespace yuengine::uieditor {
namespace {
constexpr const char *PANEL_HIERARCHY_NAME = "Hierarchy";
constexpr const char *PANEL_INSPECTOR_NAME = "Inspector";
constexpr const char *PANEL_PREVIEW_NAME = "Preview";
constexpr const char *LAYOUT_SCHEMA = "YuEngine.UI.Layout";

UiEditorShellPanelRecord MakePanelRecord(UiEditorShellPanelId panel_id, const char *panel_name) {
    UiEditorShellPanelRecord record;
    record.panel_id = panel_id;
    record.panel_name = panel_name;
    record.is_open = false;
    record.is_placeholder = true;
    return record;
}

bool IsWhitespace(char value) {
    if (value == ' ') {
        return true;
    }

    if (value == '\n') {
        return true;
    }

    if (value == '\r') {
        return true;
    }

    return value == '\t';
}

std::size_t SkipWhitespace(std::string_view text, std::size_t offset) {
    std::size_t current = offset;
    while (current < text.size()) {
        if (!IsWhitespace(text[current])) {
            return current;
        }

        ++current;
    }

    return current;
}

bool FindValueStart(std::string_view text, std::string_view key, std::size_t *out_offset) {
    if (out_offset == nullptr) {
        return false;
    }

    const std::size_t key_offset = text.find(key);
    if (key_offset == std::string_view::npos) {
        return false;
    }

    const std::size_t colon_offset = text.find(':', key_offset + key.size());
    if (colon_offset == std::string_view::npos) {
        return false;
    }

    *out_offset = SkipWhitespace(text, colon_offset + 1U);
    return *out_offset < text.size();
}

bool CopyString(std::string_view input, char *output, std::uint32_t output_capacity) {
    if (output == nullptr) {
        return false;
    }

    if (output_capacity == 0U) {
        return false;
    }

    if ((input.size() + 1U) > output_capacity) {
        return false;
    }

    std::memset(output, 0, output_capacity);
    if (input.empty()) {
        return true;
    }

    std::memcpy(output, input.data(), input.size());
    return true;
}

bool ReadStringField(std::string_view text, std::string_view key, char *output, std::uint32_t output_capacity) {
    std::size_t value_offset = 0U;
    if (!FindValueStart(text, key, &value_offset)) {
        return false;
    }

    if (text[value_offset] != '"') {
        return false;
    }

    const std::size_t begin_offset = value_offset + 1U;
    const std::size_t end_offset = text.find('"', begin_offset);
    if (end_offset == std::string_view::npos) {
        return false;
    }

    const std::string_view value = text.substr(begin_offset, end_offset - begin_offset);
    return CopyString(value, output, output_capacity);
}

bool ReadUIntField(std::string_view text, std::string_view key, std::uint32_t *output) {
    if (output == nullptr) {
        return false;
    }

    std::size_t value_offset = 0U;
    if (!FindValueStart(text, key, &value_offset)) {
        return false;
    }

    std::size_t end_offset = value_offset;
    while (end_offset < text.size()) {
        const char value = text[end_offset];
        if (value < '0' || value > '9') {
            break;
        }

        ++end_offset;
    }

    if (end_offset == value_offset) {
        return false;
    }

    const char *first = text.data() + value_offset;
    const char *last = text.data() + end_offset;
    const std::from_chars_result result = std::from_chars(first, last, *output);
    return result.ec == std::errc{};
}

bool FindMatchingToken(
    std::string_view text,
    std::size_t open_offset,
    char open_token,
    char close_token,
    std::size_t *out_close_offset) {
    if (out_close_offset == nullptr) {
        return false;
    }

    if (open_offset >= text.size()) {
        return false;
    }

    if (text[open_offset] != open_token) {
        return false;
    }

    std::uint32_t depth = 0U;
    bool in_string = false;
    bool escaped = false;
    for (std::size_t index = open_offset; index < text.size(); ++index) {
        const char value = text[index];
        if (escaped) {
            escaped = false;
            continue;
        }

        if (value == '\\') {
            escaped = in_string;
            continue;
        }

        if (value == '"') {
            in_string = !in_string;
            continue;
        }

        if (in_string) {
            continue;
        }

        if (value == open_token) {
            ++depth;
            continue;
        }

        if (value != close_token) {
            continue;
        }

        if (depth == 0U) {
            return false;
        }

        --depth;
        if (depth == 0U) {
            *out_close_offset = index;
            return true;
        }
    }

    return false;
}

bool FindNodesArray(std::string_view text, std::string_view *out_nodes) {
    if (out_nodes == nullptr) {
        return false;
    }

    const std::size_t key_offset = text.find("\"nodes\"");
    if (key_offset == std::string_view::npos) {
        return false;
    }

    const std::size_t open_offset = text.find('[', key_offset);
    if (open_offset == std::string_view::npos) {
        return false;
    }

    std::size_t close_offset = 0U;
    if (!FindMatchingToken(text, open_offset, '[', ']', &close_offset)) {
        return false;
    }

    const std::size_t content_offset = open_offset + 1U;
    *out_nodes = text.substr(content_offset, close_offset - content_offset);
    return true;
}

bool FindNextObject(
    std::string_view text,
    std::size_t search_offset,
    std::string_view *out_object,
    std::size_t *out_next_offset) {
    if (out_object == nullptr || out_next_offset == nullptr) {
        return false;
    }

    const std::size_t open_offset = text.find('{', search_offset);
    if (open_offset == std::string_view::npos) {
        return false;
    }

    std::size_t close_offset = 0U;
    if (!FindMatchingToken(text, open_offset, '{', '}', &close_offset)) {
        return false;
    }

    *out_object = text.substr(open_offset, close_offset - open_offset + 1U);
    *out_next_offset = close_offset + 1U;
    return true;
}

bool HasNodeId(const UiEditorLayoutDocument &document, std::uint32_t node_id) {
    for (std::uint32_t index = 0U; index < document.node_count; ++index) {
        if (document.nodes[index].node_id == node_id) {
            return true;
        }
    }

    return false;
}

UiEditorShellStatus ParseLayoutNode(std::string_view object_text, UiEditorLayoutNodeRecord *record) {
    if (record == nullptr) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    UiEditorLayoutNodeRecord parsed_record;
    if (!ReadUIntField(object_text, "\"nodeId\"", &parsed_record.node_id)) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    if (!ReadUIntField(object_text, "\"parentNodeId\"", &parsed_record.parent_node_id)) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    if (!ReadUIntField(object_text, "\"order\"", &parsed_record.order)) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    if (!ReadStringField(object_text, "\"name\"", parsed_record.name, UI_EDITOR_LAYOUT_NAME_CAPACITY)) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    if (!ReadStringField(object_text, "\"type\"", parsed_record.type, UI_EDITOR_LAYOUT_TYPE_CAPACITY)) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    if (parsed_record.node_id == 0U) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    *record = parsed_record;
    return UiEditorShellStatus::Success;
}

UiEditorShellStatus ValidateLayoutDocument(UiEditorLayoutDocument *document) {
    if (document == nullptr) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    if (document->node_count == 0U) {
        return UiEditorShellStatus::MissingRootNode;
    }

    std::uint32_t root_count = 0U;
    bool found_root_id = false;
    bool root_id_is_root = false;
    for (std::uint32_t index = 0U; index < document->node_count; ++index) {
        const UiEditorLayoutNodeRecord &node = document->nodes[index];
        if (node.parent_node_id == 0U) {
            ++root_count;
        }

        if (node.node_id == document->root_node_id) {
            found_root_id = true;
            root_id_is_root = node.parent_node_id == 0U;
        }
    }

    if (!found_root_id) {
        return UiEditorShellStatus::MissingRootNode;
    }

    if (!root_id_is_root) {
        return UiEditorShellStatus::MissingRootNode;
    }

    if (root_count != 1U) {
        return UiEditorShellStatus::MissingRootNode;
    }

    for (std::uint32_t index = 0U; index < document->node_count; ++index) {
        const UiEditorLayoutNodeRecord &node = document->nodes[index];
        if (node.parent_node_id == 0U) {
            continue;
        }

        if (!HasNodeId(*document, node.parent_node_id)) {
            return UiEditorShellStatus::MissingParentNode;
        }
    }

    return UiEditorShellStatus::Success;
}

UiEditorShellStatus ParseLayoutAsset(std::string_view layout_text, UiEditorLayoutDocument *document) {
    if (document == nullptr) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    if (layout_text.empty()) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    UiEditorLayoutDocument parsed_document;
    char schema[UI_EDITOR_LAYOUT_ID_CAPACITY] = {};
    if (!ReadStringField(layout_text, "\"schema\"", schema, UI_EDITOR_LAYOUT_ID_CAPACITY)) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    if (std::string_view(schema) != LAYOUT_SCHEMA) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    if (!ReadUIntField(layout_text, "\"version\"", &parsed_document.version)) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    if (parsed_document.version == 0U) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    if (!ReadStringField(layout_text, "\"layoutId\"", parsed_document.layout_id, UI_EDITOR_LAYOUT_ID_CAPACITY)) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    if (!ReadUIntField(layout_text, "\"rootNodeId\"", &parsed_document.root_node_id)) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    std::string_view nodes_text;
    if (!FindNodesArray(layout_text, &nodes_text)) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    std::size_t next_offset = 0U;
    while (true) {
        std::string_view object_text;
        if (!FindNextObject(nodes_text, next_offset, &object_text, &next_offset)) {
            break;
        }

        if (parsed_document.node_count >= UI_EDITOR_LAYOUT_MAX_NODE_COUNT) {
            return UiEditorShellStatus::LayoutNodeCapacityExceeded;
        }

        UiEditorLayoutNodeRecord node;
        const UiEditorShellStatus node_status = ParseLayoutNode(object_text, &node);
        if (node_status != UiEditorShellStatus::Success) {
            return node_status;
        }

        if (HasNodeId(parsed_document, node.node_id)) {
            return UiEditorShellStatus::DuplicateNodeId;
        }

        parsed_document.nodes[parsed_document.node_count] = node;
        ++parsed_document.node_count;
    }

    const UiEditorShellStatus validate_status = ValidateLayoutDocument(&parsed_document);
    if (validate_status != UiEditorShellStatus::Success) {
        return validate_status;
    }

    parsed_document.is_loaded = true;
    *document = parsed_document;
    return UiEditorShellStatus::Success;
}
}

UiEditorShellState::UiEditorShellState()
    : panels_{
          MakePanelRecord(UiEditorShellPanelId::Hierarchy, PANEL_HIERARCHY_NAME),
          MakePanelRecord(UiEditorShellPanelId::Inspector, PANEL_INSPECTOR_NAME),
          MakePanelRecord(UiEditorShellPanelId::Preview, PANEL_PREVIEW_NAME)},
      layout_document_{},
      inspector_record_{},
      snapshot_{
          UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT,
          0U,
          UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT,
          UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT,
          0U,
          0U,
          false,
          false,
          UiEditorShellStatus::Success} {
}

UiEditorShellStatus UiEditorShellState::OpenDefaultPlaceholderPanels() {
    for (UiEditorShellPanelRecord &panel : panels_) {
        panel.is_open = true;
    }

    RecountPanels();
    return RecordStatus(UiEditorShellStatus::Success);
}

UiEditorShellStatus UiEditorShellState::SetPanelOpen(UiEditorShellPanelId panel_id, bool is_open) {
    UiEditorShellPanelRecord *panel = FindPanel(panel_id);
    if (panel == nullptr) {
        return RecordStatus(UiEditorShellStatus::InvalidPanel);
    }

    panel->is_open = is_open;
    RecountPanels();
    return RecordStatus(UiEditorShellStatus::Success);
}

UiEditorShellStatus UiEditorShellState::ExportPanels(
    UiEditorShellPanelRecord *output_records,
    std::uint32_t output_capacity,
    std::uint32_t *out_count) {
    if (out_count == nullptr) {
        return RecordStatus(UiEditorShellStatus::InvalidOutput);
    }

    *out_count = snapshot_.panel_count;
    if (output_capacity < snapshot_.panel_count) {
        return RecordStatus(UiEditorShellStatus::OutputCapacityExceeded);
    }

    if (output_records == nullptr) {
        return RecordStatus(UiEditorShellStatus::InvalidOutput);
    }

    for (std::uint32_t index = 0U; index < snapshot_.panel_count; ++index) {
        output_records[index] = panels_[index];
    }

    return RecordStatus(UiEditorShellStatus::Success);
}

UiEditorShellStatus UiEditorShellState::LoadLayoutAsset(std::string_view layout_text) {
    ResetLayoutState();

    UiEditorLayoutDocument document;
    const UiEditorShellStatus parse_status = ParseLayoutAsset(layout_text, &document);
    if (parse_status != UiEditorShellStatus::Success) {
        return RecordStatus(parse_status);
    }

    layout_document_ = document;
    snapshot_.layout_loaded = true;
    snapshot_.loaded_node_count = layout_document_.node_count;

    UiEditorShellPanelRecord *hierarchy_panel = FindPanel(UiEditorShellPanelId::Hierarchy);
    UiEditorShellPanelRecord *inspector_panel = FindPanel(UiEditorShellPanelId::Inspector);
    UiEditorShellPanelRecord *preview_panel = FindPanel(UiEditorShellPanelId::Preview);
    if (hierarchy_panel == nullptr || inspector_panel == nullptr || preview_panel == nullptr) {
        ResetLayoutState();
        return RecordStatus(UiEditorShellStatus::InvalidPanel);
    }

    hierarchy_panel->is_placeholder = false;
    inspector_panel->is_placeholder = false;
    preview_panel->is_placeholder = true;
    RecountPanels();

    const UiEditorLayoutNodeRecord *root_node = FindLayoutNode(layout_document_.root_node_id);
    if (root_node == nullptr) {
        ResetLayoutState();
        return RecordStatus(UiEditorShellStatus::MissingRootNode);
    }

    const UiEditorShellStatus inspector_status = LoadInspectorFromNode(*root_node);
    if (inspector_status != UiEditorShellStatus::Success) {
        ResetLayoutState();
        return RecordStatus(inspector_status);
    }

    return RecordStatus(UiEditorShellStatus::Success);
}

UiEditorShellStatus UiEditorShellState::ExportHierarchyNodes(
    UiEditorLayoutNodeRecord *output_records,
    std::uint32_t output_capacity,
    std::uint32_t *out_count) {
    if (out_count == nullptr) {
        return RecordStatus(UiEditorShellStatus::InvalidOutput);
    }

    *out_count = layout_document_.node_count;
    if (!layout_document_.is_loaded) {
        *out_count = 0U;
        return RecordStatus(UiEditorShellStatus::LayoutNotLoaded);
    }

    if (output_capacity < layout_document_.node_count) {
        return RecordStatus(UiEditorShellStatus::OutputCapacityExceeded);
    }

    if (output_records == nullptr) {
        return RecordStatus(UiEditorShellStatus::InvalidOutput);
    }

    for (std::uint32_t index = 0U; index < layout_document_.node_count; ++index) {
        output_records[index] = layout_document_.nodes[index];
    }

    return RecordStatus(UiEditorShellStatus::Success);
}

UiEditorShellStatus UiEditorShellState::SelectLayoutNode(std::uint32_t node_id) {
    if (!layout_document_.is_loaded) {
        return RecordStatus(UiEditorShellStatus::LayoutNotLoaded);
    }

    const UiEditorLayoutNodeRecord *node = FindLayoutNode(node_id);
    if (node == nullptr) {
        return RecordStatus(UiEditorShellStatus::NodeNotFound);
    }

    const UiEditorShellStatus inspector_status = LoadInspectorFromNode(*node);
    if (inspector_status != UiEditorShellStatus::Success) {
        return RecordStatus(inspector_status);
    }

    return RecordStatus(UiEditorShellStatus::Success);
}

UiEditorShellStatus UiEditorShellState::GetInspectorRecord(UiEditorInspectorRecord *out_record) {
    if (out_record == nullptr) {
        return RecordStatus(UiEditorShellStatus::InvalidOutput);
    }

    if (!layout_document_.is_loaded) {
        return RecordStatus(UiEditorShellStatus::LayoutNotLoaded);
    }

    if (!inspector_record_.has_selection) {
        return RecordStatus(UiEditorShellStatus::NodeNotFound);
    }

    *out_record = inspector_record_;
    return RecordStatus(UiEditorShellStatus::Success);
}

UiEditorShellStatus UiEditorShellState::GetVisualBackendStatus() const {
    return UiEditorShellStatus::DearImGuiUnavailable;
}

UiEditorShellSnapshot UiEditorShellState::Snapshot() const {
    return snapshot_;
}

UiEditorShellStatus UiEditorShellState::RecordStatus(UiEditorShellStatus status) {
    snapshot_.last_status = status;
    return status;
}

void UiEditorShellState::RecountPanels() {
    std::uint32_t open_panel_count = 0U;
    std::uint32_t placeholder_panel_count = 0U;

    for (const UiEditorShellPanelRecord &panel : panels_) {
        if (panel.is_open) {
            ++open_panel_count;
        }

        if (panel.is_placeholder) {
            ++placeholder_panel_count;
        }
    }

    snapshot_.open_panel_count = open_panel_count;
    snapshot_.placeholder_panel_count = placeholder_panel_count;
}

UiEditorShellPanelRecord *UiEditorShellState::FindPanel(UiEditorShellPanelId panel_id) {
    if (panel_id == UiEditorShellPanelId::Invalid) {
        return nullptr;
    }

    for (UiEditorShellPanelRecord &panel : panels_) {
        if (panel.panel_id == panel_id) {
            return &panel;
        }
    }

    return nullptr;
}

const UiEditorLayoutNodeRecord *UiEditorShellState::FindLayoutNode(std::uint32_t node_id) const {
    if (node_id == 0U) {
        return nullptr;
    }

    for (std::uint32_t index = 0U; index < layout_document_.node_count; ++index) {
        const UiEditorLayoutNodeRecord &node = layout_document_.nodes[index];
        if (node.node_id == node_id) {
            return &node;
        }
    }

    return nullptr;
}

void UiEditorShellState::ResetLayoutState() {
    layout_document_ = {};
    inspector_record_ = {};
    snapshot_.layout_loaded = false;
    snapshot_.loaded_node_count = 0U;
    snapshot_.selected_node_id = 0U;

    UiEditorShellPanelRecord *hierarchy_panel = FindPanel(UiEditorShellPanelId::Hierarchy);
    UiEditorShellPanelRecord *inspector_panel = FindPanel(UiEditorShellPanelId::Inspector);
    if (hierarchy_panel != nullptr) {
        hierarchy_panel->is_placeholder = true;
    }

    if (inspector_panel != nullptr) {
        inspector_panel->is_placeholder = true;
    }

    RecountPanels();
}

UiEditorShellStatus UiEditorShellState::LoadInspectorFromNode(const UiEditorLayoutNodeRecord &node) {
    UiEditorInspectorRecord record;
    record.has_selection = true;
    record.node_id = node.node_id;
    record.parent_node_id = node.parent_node_id;
    record.order = node.order;

    if (!CopyString(std::string_view(layout_document_.layout_id), record.layout_id, UI_EDITOR_LAYOUT_ID_CAPACITY)) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    if (!CopyString(std::string_view(node.name), record.name, UI_EDITOR_LAYOUT_NAME_CAPACITY)) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    if (!CopyString(std::string_view(node.type), record.type, UI_EDITOR_LAYOUT_TYPE_CAPACITY)) {
        return UiEditorShellStatus::InvalidLayoutAsset;
    }

    inspector_record_ = record;
    snapshot_.selected_node_id = node.node_id;
    return UiEditorShellStatus::Success;
}
}
