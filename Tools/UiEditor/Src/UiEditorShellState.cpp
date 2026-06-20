// 模块: Tools UiEditor
// 文件: Tools/UiEditor/Src/UiEditorShellState.cpp

#include "YuEngine/UiEditor/UiEditorShellState.h"

#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>

#include "YuEngine/RenderCore/RenderFixturePass.h"
#include "YuEngine/RenderCore/RenderFixturePassRequest.h"
#include "YuEngine/RenderCore/RenderFixturePassResult.h"
#include "YuEngine/RenderCore/RenderSubmissionBatchFixture.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/NullRhiDevice.h"
#include "YuEngine/Rhi/RhiBufferDesc.h"
#include "YuEngine/Rhi/RhiBufferHandle.h"
#include "YuEngine/Rhi/RhiBufferUsage.h"
#include "YuEngine/Rhi/RhiColorTargetDesc.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiDeviceSnapshot.h"
#include "YuEngine/Rhi/RhiDrawIndexedDesc.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiIndexFormat.h"
#include "YuEngine/Rhi/RhiInputLayoutDesc.h"
#include "YuEngine/Rhi/RhiPipelineDesc.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiPrimitiveTopology.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"
#include "YuEngine/Rhi/RhiSamplerDesc.h"
#include "YuEngine/Rhi/RhiSamplerHandle.h"
#include "YuEngine/Rhi/RhiShaderModuleDesc.h"
#include "YuEngine/Rhi/RhiShaderModuleHandle.h"
#include "YuEngine/Rhi/RhiShaderStage.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"
#include "YuEngine/UiCore/UiDrawElement.h"
#include "YuEngine/UiCore/UiDrawElementDesc.h"
#include "YuEngine/UiCore/UiDrawElementType.h"
#include "YuEngine/UiCore/UiDrawListBuilder.h"
#include "YuEngine/UiCore/UiDrawListResult.h"
#include "YuEngine/UiCore/UiDrawListStatus.h"
#include "YuEngine/UiCore/UiLayoutContainerDesc.h"
#include "YuEngine/UiCore/UiLayoutContainerType.h"
#include "YuEngine/UiCore/UiLayoutPass.h"
#include "YuEngine/UiCore/UiLayoutPassResult.h"
#include "YuEngine/UiCore/UiNodeDesc.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiNodeTree.h"
#include "YuEngine/UiCore/UiNodeTreeDesc.h"
#include "YuEngine/UiCore/UiNodeTreeResult.h"
#include "YuEngine/UiCore/UiRect.h"
#include "YuEngine/UiCore/UiRectTransform.h"
#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridge.h"
#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeRequest.h"
#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeResult.h"
#include "YuEngine/UiRenderCoreBridge/UiRenderCoreBridgeStatus.h"

namespace yuengine::uieditor {
namespace {
constexpr const char *PANEL_HIERARCHY_NAME = "Hierarchy";
constexpr const char *PANEL_INSPECTOR_NAME = "Inspector";
constexpr const char *PANEL_PREVIEW_NAME = "Preview";
constexpr const char *LAYOUT_SCHEMA = "YuEngine.UI.Layout";
constexpr const char *PREVIEW_NODE_TYPE_ROOT = "Root";
constexpr const char *PREVIEW_NODE_TYPE_TITLE_BAR = "TitleBar";
constexpr std::uint32_t PREVIEW_STYLE_KEY_BASE = 700U;
constexpr std::uint32_t PREVIEW_MATERIAL_KEY_BASE = 900U;
constexpr std::uint32_t PREVIEW_PASS_ID_BASE = 1200U;
constexpr std::uint32_t PREVIEW_MAX_RHI_EXTENT = 65535U;
constexpr std::uint32_t TRIANGLE_VERTEX_COUNT = 3U;
constexpr std::uint32_t TRIANGLE_INDEX_COUNT = 3U;
constexpr std::size_t TRIANGLE_VERTEX_STRIDE_BYTES = sizeof(float) * 6U;
constexpr std::size_t TRIANGLE_VERTEX_BUFFER_BYTES = TRIANGLE_VERTEX_STRIDE_BYTES * TRIANGLE_VERTEX_COUNT;
constexpr std::size_t TRIANGLE_INDEX_BUFFER_BYTES = sizeof(std::uint16_t) * TRIANGLE_INDEX_COUNT;
constexpr std::size_t PREVIEW_CAPTURE_BYTES = 16U;

using RenderFixturePass = yuengine::rendercore::RenderFixturePass;
using RenderFixturePassRequest = yuengine::rendercore::RenderFixturePassRequest;
using RenderFixturePassResult = yuengine::rendercore::RenderFixturePassResult;
using RenderSubmissionBatchFixture = yuengine::rendercore::RenderSubmissionBatchFixture;
using IRhiDevice = yuengine::rhi::IRhiDevice;
using NullRhiDevice = yuengine::rhi::NullRhiDevice;
using RhiBufferDesc = yuengine::rhi::RhiBufferDesc;
using RhiBufferHandle = yuengine::rhi::RhiBufferHandle;
using yuengine::rhi::RhiBufferUsage;
using RhiColor = yuengine::rhi::RhiColor;
using RhiColorTargetDesc = yuengine::rhi::RhiColorTargetDesc;
using RhiDeviceDesc = yuengine::rhi::RhiDeviceDesc;
using RhiDeviceSnapshot = yuengine::rhi::RhiDeviceSnapshot;
using RhiDrawIndexedDesc = yuengine::rhi::RhiDrawIndexedDesc;
using RhiIndexBufferView = yuengine::rhi::RhiIndexBufferView;
using yuengine::rhi::RhiFormat;
using yuengine::rhi::RhiIndexFormat;
using yuengine::rhi::RhiInputElementFormat;
using yuengine::rhi::RhiInputElementSemantic;
using RhiInputLayoutDesc = yuengine::rhi::RhiInputLayoutDesc;
using RhiPipelineDesc = yuengine::rhi::RhiPipelineDesc;
using RhiPipelineHandle = yuengine::rhi::RhiPipelineHandle;
using yuengine::rhi::RhiPrimitiveTopology;
using RhiSampledTextureBinding = yuengine::rhi::RhiSampledTextureBinding;
using RhiSamplerBinding = yuengine::rhi::RhiSamplerBinding;
using RhiSamplerDesc = yuengine::rhi::RhiSamplerDesc;
using RhiSamplerHandle = yuengine::rhi::RhiSamplerHandle;
using RhiShaderModuleDesc = yuengine::rhi::RhiShaderModuleDesc;
using RhiShaderModuleHandle = yuengine::rhi::RhiShaderModuleHandle;
using yuengine::rhi::RhiShaderStage;
using yuengine::rhi::RhiStatus;
using RhiTextureDesc = yuengine::rhi::RhiTextureDesc;
using RhiTextureHandle = yuengine::rhi::RhiTextureHandle;
using RhiVertexBufferView = yuengine::rhi::RhiVertexBufferView;
using UiDrawElement = yuengine::uicore::UiDrawElement;
using UiDrawElementDesc = yuengine::uicore::UiDrawElementDesc;
using yuengine::uicore::UiDrawElementType;
using UiDrawListBuilder = yuengine::uicore::UiDrawListBuilder;
using UiDrawListResult = yuengine::uicore::UiDrawListResult;
using yuengine::uicore::UiDrawListStatus;
using UiLayoutContainerDesc = yuengine::uicore::UiLayoutContainerDesc;
using yuengine::uicore::UiLayoutContainerType;
using UiLayoutPass = yuengine::uicore::UiLayoutPass;
using UiLayoutPassResult = yuengine::uicore::UiLayoutPassResult;
using UiNodeDesc = yuengine::uicore::UiNodeDesc;
using UiNodeId = yuengine::uicore::UiNodeId;
using UiNodeTree = yuengine::uicore::UiNodeTree;
using UiNodeTreeDesc = yuengine::uicore::UiNodeTreeDesc;
using UiNodeTreeResult = yuengine::uicore::UiNodeTreeResult;
using UiRect = yuengine::uicore::UiRect;
using UiRectTransform = yuengine::uicore::UiRectTransform;
using UiRenderCoreBridge = yuengine::uirendercorebridge::UiRenderCoreBridge;
using UiRenderCoreBridgeRequest = yuengine::uirendercorebridge::UiRenderCoreBridgeRequest;
using UiRenderCoreBridgeResult = yuengine::uirendercorebridge::UiRenderCoreBridgeResult;
using yuengine::uirendercorebridge::UiRenderCoreBridgeStatus;

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

UiNodeId NodeId(std::uint32_t value) {
    return UiNodeId{value};
}

float SmallerFloat(float left, float right) {
    if (left < right) {
        return left;
    }

    return right;
}

UiRectTransform FullStretchTransform() {
    UiRectTransform transform;
    transform.anchor_min = {0.0F, 0.0F};
    transform.anchor_max = {1.0F, 1.0F};
    transform.pivot = {0.5F, 0.5F};
    return transform;
}

UiRectTransform PreviewWindowTransform(const UiEditorPreviewViewportDesc &desc) {
    const float width = static_cast<float>(desc.viewport_width);
    const float height = static_cast<float>(desc.viewport_height);
    const float margin_x = SmallerFloat(width * 0.10F, 120.0F);
    const float bottom_margin = SmallerFloat(height * 0.12F, 120.0F);
    const float top_margin = SmallerFloat(height * 0.18F, 160.0F);

    UiRectTransform transform = FullStretchTransform();
    transform.offset_min = {margin_x, bottom_margin};
    transform.offset_max = {-margin_x, -top_margin};
    return transform;
}

UiRectTransform PreviewTitleBarTransform(const UiEditorPreviewViewportDesc &desc) {
    const float height = static_cast<float>(desc.viewport_height);
    const float bar_height = SmallerFloat(height * 0.08F, 48.0F);

    UiRectTransform transform;
    transform.anchor_min = {0.0F, 1.0F};
    transform.anchor_max = {1.0F, 1.0F};
    transform.pivot = {0.5F, 1.0F};
    transform.offset_min = {0.0F, -bar_height};
    transform.offset_max = {0.0F, 0.0F};
    return transform;
}

bool IsRootNode(const UiEditorLayoutNodeRecord &node) {
    if (node.parent_node_id == 0U) {
        return true;
    }

    return std::string_view(node.type) == PREVIEW_NODE_TYPE_ROOT;
}

bool IsTitleBarNode(const UiEditorLayoutNodeRecord &node) {
    return std::string_view(node.type) == PREVIEW_NODE_TYPE_TITLE_BAR;
}

UiNodeDesc MakePreviewNodeDesc(
    const UiEditorLayoutNodeRecord &node,
    const UiEditorPreviewViewportDesc &viewport_desc) {
    UiNodeDesc desc;
    desc.node_id = NodeId(node.node_id);
    desc.parent_id = NodeId(node.parent_node_id);
    desc.rect_transform = FullStretchTransform();
    desc.sibling_order = node.order;
    desc.layer = 10;
    desc.is_visible = true;
    desc.is_enabled = true;
    desc.is_hit_testable = true;

    if (node.parent_node_id == 0U) {
        desc.parent_id = UiNodeId{};
        desc.layer = 0;
        return desc;
    }

    if (IsTitleBarNode(node)) {
        desc.rect_transform = PreviewTitleBarTransform(viewport_desc);
        desc.layer = 20;
        return desc;
    }

    desc.rect_transform = PreviewWindowTransform(viewport_desc);
    return desc;
}

bool CreatePreviewNode(UiNodeTree *tree, const UiNodeDesc &desc) {
    if (tree == nullptr) {
        return false;
    }

    const UiNodeTreeResult result = tree->CreateNode(desc);
    return result.Succeeded();
}

bool IsCreatedNode(
    const UiEditorLayoutDocument &document,
    const std::array<bool, UI_EDITOR_LAYOUT_MAX_NODE_COUNT> &created,
    std::uint32_t node_id) {
    if (node_id == 0U) {
        return true;
    }

    for (std::uint32_t index = 0U; index < document.node_count; ++index) {
        if (document.nodes[index].node_id != node_id) {
            continue;
        }

        return created[index];
    }

    return false;
}

bool BuildPreviewNodeTree(
    const UiEditorLayoutDocument &document,
    const UiEditorPreviewViewportDesc &viewport_desc,
    UiNodeTree *tree) {
    if (tree == nullptr) {
        return false;
    }

    std::array<bool, UI_EDITOR_LAYOUT_MAX_NODE_COUNT> created{};
    std::uint32_t created_count = 0U;
    while (created_count < document.node_count) {
        bool made_progress = false;
        for (std::uint32_t index = 0U; index < document.node_count; ++index) {
            if (created[index]) {
                continue;
            }

            const UiEditorLayoutNodeRecord &node = document.nodes[index];
            if (!IsCreatedNode(document, created, node.parent_node_id)) {
                continue;
            }

            const UiNodeDesc desc = MakePreviewNodeDesc(node, viewport_desc);
            if (!CreatePreviewNode(tree, desc)) {
                return false;
            }

            created[index] = true;
            ++created_count;
            made_progress = true;
        }

        if (!made_progress) {
            return false;
        }
    }

    return true;
}

bool HasLayoutChild(const UiEditorLayoutDocument &document, std::uint32_t node_id) {
    for (std::uint32_t index = 0U; index < document.node_count; ++index) {
        if (document.nodes[index].parent_node_id == node_id) {
            return true;
        }
    }

    return false;
}

bool BuildPreviewLayoutContainers(
    const UiEditorLayoutDocument &document,
    std::array<UiLayoutContainerDesc, UI_EDITOR_LAYOUT_MAX_NODE_COUNT> *containers,
    std::uint32_t *out_count) {
    if (containers == nullptr || out_count == nullptr) {
        return false;
    }

    *out_count = 0U;
    for (std::uint32_t index = 0U; index < document.node_count; ++index) {
        const UiEditorLayoutNodeRecord &node = document.nodes[index];
        if (!HasLayoutChild(document, node.node_id)) {
            continue;
        }

        UiLayoutContainerDesc desc;
        desc.container_id = NodeId(node.node_id);
        desc.type = UiLayoutContainerType::Absolute;
        (*containers)[*out_count] = desc;
        ++(*out_count);
    }

    return true;
}

bool ApplyPreviewLayout(
    UiNodeTree *tree,
    const UiEditorLayoutDocument &document,
    std::uint32_t *out_container_count) {
    if (tree == nullptr || out_container_count == nullptr) {
        return false;
    }

    std::array<UiLayoutContainerDesc, UI_EDITOR_LAYOUT_MAX_NODE_COUNT> containers{};
    std::uint32_t container_count = 0U;
    if (!BuildPreviewLayoutContainers(document, &containers, &container_count)) {
        return false;
    }

    UiLayoutPass pass;
    const std::span<const UiLayoutContainerDesc> container_span(containers.data(), container_count);
    const UiLayoutPassResult result = pass.Apply(tree, container_span);
    if (!result.Succeeded()) {
        return false;
    }

    if (result.container_count != container_count) {
        return false;
    }

    *out_container_count = result.container_count;
    return true;
}

bool ShouldEmitPreviewDrawElement(const UiEditorLayoutNodeRecord &node) {
    if (IsRootNode(node)) {
        return false;
    }

    return node.parent_node_id != 0U;
}

bool BuildPreviewDrawDescs(
    const UiEditorLayoutDocument &document,
    std::array<UiDrawElementDesc, UI_EDITOR_PREVIEW_MAX_DRAW_ELEMENT_COUNT> *descs,
    std::uint32_t *out_count) {
    if (descs == nullptr || out_count == nullptr) {
        return false;
    }

    *out_count = 0U;
    for (std::uint32_t index = 0U; index < document.node_count; ++index) {
        const UiEditorLayoutNodeRecord &node = document.nodes[index];
        if (!ShouldEmitPreviewDrawElement(node)) {
            continue;
        }

        UiDrawElementDesc desc;
        desc.node_id = NodeId(node.node_id);
        desc.type = UiDrawElementType::Rect;
        desc.style_key = PREVIEW_STYLE_KEY_BASE + index;
        desc.material_key = PREVIEW_MATERIAL_KEY_BASE + index;
        desc.scissor_enabled = true;
        (*descs)[*out_count] = desc;
        ++(*out_count);
    }

    return true;
}

bool BuildPreviewDrawElements(
    const UiNodeTree &tree,
    const UiEditorLayoutDocument &document,
    std::array<UiDrawElement, UI_EDITOR_PREVIEW_MAX_DRAW_ELEMENT_COUNT> *elements,
    std::uint32_t *out_count) {
    if (elements == nullptr || out_count == nullptr) {
        return false;
    }

    std::array<UiDrawElementDesc, UI_EDITOR_PREVIEW_MAX_DRAW_ELEMENT_COUNT> descs{};
    std::uint32_t desc_count = 0U;
    if (!BuildPreviewDrawDescs(document, &descs, &desc_count)) {
        return false;
    }

    if (desc_count == 0U) {
        return false;
    }

    UiDrawListResult result;
    UiDrawListBuilder builder;
    const std::span<const UiDrawElementDesc> desc_span(descs.data(), desc_count);
    const std::span<UiDrawElement> element_span(elements->data(), elements->size());
    const UiDrawListStatus status = builder.Build(tree, desc_span, element_span, &result);
    if (status != UiDrawListStatus::Success) {
        return false;
    }

    if (result.element_count != desc_count) {
        return false;
    }

    *out_count = result.element_count;
    return true;
}

RhiColorTargetDesc PreviewTargetDesc() {
    RhiColorTargetDesc target_desc{};
    target_desc.format = RhiFormat::Rgba8Unorm;
    target_desc.extent.width = static_cast<std::uint16_t>(2U);
    target_desc.extent.height = static_cast<std::uint16_t>(2U);
    return target_desc;
}

RhiBufferDesc TriangleVertexBufferDesc() {
    return RhiBufferDesc{RhiBufferUsage::Vertex, TRIANGLE_VERTEX_BUFFER_BYTES};
}

RhiBufferDesc TriangleIndexBufferDesc() {
    return RhiBufferDesc{RhiBufferUsage::Index, TRIANGLE_INDEX_BUFFER_BYTES};
}

RhiTextureDesc SmallTextureDesc() {
    RhiTextureDesc desc{};
    desc.format = RhiFormat::Rgba8Unorm;
    desc.extent.width = static_cast<std::uint16_t>(2U);
    desc.extent.height = static_cast<std::uint16_t>(2U);
    return desc;
}

RhiInputLayoutDesc TriangleInputLayoutDesc() {
    RhiInputLayoutDesc desc{};
    desc.elements[0U].semantic = RhiInputElementSemantic::Position;
    desc.elements[0U].format = RhiInputElementFormat::Float32x2;
    desc.elements[0U].offset_bytes = 0U;
    desc.elements[1U].semantic = RhiInputElementSemantic::Color;
    desc.elements[1U].format = RhiInputElementFormat::Float32x4;
    desc.elements[1U].offset_bytes = sizeof(float) * 2U;
    desc.element_count = 2U;
    desc.stride_bytes = TRIANGLE_VERTEX_STRIDE_BYTES;
    return desc;
}

RhiDrawIndexedDesc TriangleDrawIndexedDesc() {
    RhiDrawIndexedDesc desc{};
    desc.topology = RhiPrimitiveTopology::TriangleList;
    desc.index_count = TRIANGLE_INDEX_COUNT;
    desc.first_index = 0U;
    desc.vertex_offset = 0;
    return desc;
}

RhiVertexBufferView TriangleVertexBufferViewFor(RhiBufferHandle handle) {
    RhiVertexBufferView view{};
    view.buffer = handle;
    view.offset_bytes = 0U;
    view.stride_bytes = TRIANGLE_VERTEX_STRIDE_BYTES;
    view.size_bytes = TRIANGLE_VERTEX_BUFFER_BYTES;
    return view;
}

RhiIndexBufferView TriangleIndexBufferViewFor(RhiBufferHandle handle) {
    RhiIndexBufferView view{};
    view.buffer = handle;
    view.offset_bytes = 0U;
    view.size_bytes = TRIANGLE_INDEX_BUFFER_BYTES;
    view.format = RhiIndexFormat::Uint16;
    return view;
}

RhiSampledTextureBinding SampledTextureBindingFor(RhiTextureHandle texture) {
    RhiSampledTextureBinding binding{};
    binding.texture = texture;
    binding.slot = 0U;
    return binding;
}

RhiSamplerBinding SamplerBindingFor(RhiSamplerHandle sampler) {
    RhiSamplerBinding binding{};
    binding.sampler = sampler;
    binding.slot = 0U;
    return binding;
}

std::array<std::uint8_t, 4U> SmallShaderBytes() {
    return std::array<std::uint8_t, 4U>{1U, 2U, 3U, 4U};
}

bool CreateTarget(IRhiDevice *device, RhiTextureHandle *out_handle) {
    if (device == nullptr || out_handle == nullptr) {
        return false;
    }

    const RhiColorTargetDesc target_desc = PreviewTargetDesc();
    return device->CreateColorTarget(target_desc, *out_handle) == RhiStatus::Success;
}

bool CreateShaderModule(IRhiDevice *device, RhiShaderStage stage, RhiShaderModuleHandle *out_handle) {
    if (device == nullptr || out_handle == nullptr) {
        return false;
    }

    const std::array<std::uint8_t, 4U> bytes = SmallShaderBytes();
    const std::span<const std::uint8_t> byte_span(bytes.data(), bytes.size());
    const RhiShaderModuleDesc desc{stage, byte_span};
    return device->CreateShaderModule(desc, *out_handle) == RhiStatus::Success;
}

bool CreateTrianglePipeline(IRhiDevice *device, RhiPipelineHandle *out_handle) {
    if (device == nullptr || out_handle == nullptr) {
        return false;
    }

    RhiShaderModuleHandle vertex_shader{};
    if (!CreateShaderModule(device, RhiShaderStage::Vertex, &vertex_shader)) {
        return false;
    }

    RhiShaderModuleHandle pixel_shader{};
    if (!CreateShaderModule(device, RhiShaderStage::Pixel, &pixel_shader)) {
        return false;
    }

    RhiPipelineDesc desc{};
    desc.vertex_shader = vertex_shader;
    desc.pixel_shader = pixel_shader;
    desc.input_layout = TriangleInputLayoutDesc();
    return device->CreatePipeline(desc, *out_handle) == RhiStatus::Success;
}

bool CreateTriangleBuffer(IRhiDevice *device, RhiBufferHandle *out_handle) {
    if (device == nullptr || out_handle == nullptr) {
        return false;
    }

    const std::span<const std::uint8_t> empty_bytes{};
    return device->CreateBuffer(TriangleVertexBufferDesc(), empty_bytes, *out_handle) == RhiStatus::Success;
}

bool CreateTriangleIndexBuffer(IRhiDevice *device, RhiBufferHandle *out_handle) {
    if (device == nullptr || out_handle == nullptr) {
        return false;
    }

    const std::array<std::uint16_t, TRIANGLE_INDEX_COUNT> indices{
        static_cast<std::uint16_t>(0U),
        static_cast<std::uint16_t>(1U),
        static_cast<std::uint16_t>(2U)};
    const auto *index_byte_pointer = reinterpret_cast<const std::uint8_t *>(indices.data());
    const std::span<const std::uint8_t> index_bytes(index_byte_pointer, TRIANGLE_INDEX_BUFFER_BYTES);
    return device->CreateBuffer(TriangleIndexBufferDesc(), index_bytes, *out_handle) == RhiStatus::Success;
}

bool CreateSampledTexture(IRhiDevice *device, RhiTextureHandle *out_handle) {
    if (device == nullptr || out_handle == nullptr) {
        return false;
    }

    const std::array<std::uint8_t, PREVIEW_CAPTURE_BYTES> texture_bytes{
        255U, 0U, 0U, 255U,
        0U, 255U, 0U, 255U,
        0U, 0U, 255U, 255U,
        255U, 255U, 255U, 255U};
    const std::span<const std::uint8_t> texture_span(texture_bytes.data(), texture_bytes.size());
    return device->CreateTexture(SmallTextureDesc(), texture_span, *out_handle) == RhiStatus::Success;
}

bool CreateSamplerPrimitive(IRhiDevice *device, RhiSamplerHandle *out_handle) {
    if (device == nullptr || out_handle == nullptr) {
        return false;
    }

    RhiSamplerDesc desc{};
    desc.linear_filter = false;
    desc.clamp_to_edge = true;
    return device->CreateSampler(desc, *out_handle) == RhiStatus::Success;
}

bool FillPreviewRenderTemplateRequest(
    NullRhiDevice *device,
    std::array<std::uint8_t, PREVIEW_CAPTURE_BYTES> *capture,
    RenderFixturePassRequest *request) {
    if (device == nullptr || capture == nullptr || request == nullptr) {
        return false;
    }

    IRhiDevice *device_interface = device;
    RhiTextureHandle target{};
    if (!CreateTarget(device_interface, &target)) {
        return false;
    }

    RhiPipelineHandle pipeline{};
    if (!CreateTrianglePipeline(device_interface, &pipeline)) {
        return false;
    }

    RhiBufferHandle vertex_buffer{};
    if (!CreateTriangleBuffer(device_interface, &vertex_buffer)) {
        return false;
    }

    RhiBufferHandle index_buffer{};
    if (!CreateTriangleIndexBuffer(device_interface, &index_buffer)) {
        return false;
    }

    RhiTextureHandle sampled_texture{};
    if (!CreateSampledTexture(device_interface, &sampled_texture)) {
        return false;
    }

    RhiSamplerHandle sampler{};
    if (!CreateSamplerPrimitive(device_interface, &sampler)) {
        return false;
    }

    request->rhi_device = device_interface;
    request->target = target;
    request->pipeline = pipeline;
    request->vertex_buffer = TriangleVertexBufferViewFor(vertex_buffer);
    request->index_buffer = TriangleIndexBufferViewFor(index_buffer);
    request->sampled_texture = SampledTextureBindingFor(sampled_texture);
    request->sampler = SamplerBindingFor(sampler);
    request->draw = TriangleDrawIndexedDesc();
    request->clear_color = RhiColor{7U, 16U, 28U, 255U};
    request->capture_output = std::span<std::uint8_t>(capture->data(), capture->size());
    request->capture_byte_budget = capture->size();
    request->pass_id = 1U;
    request->material_id = PREVIEW_MATERIAL_KEY_BASE;
    return true;
}

bool SubmitPreviewDrawElements(
    const std::array<UiDrawElement, UI_EDITOR_PREVIEW_MAX_DRAW_ELEMENT_COUNT> &elements,
    std::uint32_t element_count,
    UiEditorPreviewViewportRecord *record) {
    if (record == nullptr) {
        return false;
    }

    NullRhiDevice device;
    if (device.Initialize(RhiDeviceDesc{}) != RhiStatus::Success) {
        return false;
    }

    std::array<std::uint8_t, PREVIEW_CAPTURE_BYTES> capture{};
    RenderFixturePassRequest template_request{};
    if (!FillPreviewRenderTemplateRequest(&device, &capture, &template_request)) {
        return false;
    }

    std::array<RenderFixturePassRequest, UI_EDITOR_PREVIEW_MAX_DRAW_ELEMENT_COUNT> pass_requests{};
    std::array<RenderFixturePassResult, UI_EDITOR_PREVIEW_MAX_DRAW_ELEMENT_COUNT> pass_results{};
    RenderFixturePass pass;
    RenderSubmissionBatchFixture batch;
    UiRenderCoreBridge bridge;
    const std::span<const UiDrawElement> element_span(elements.data(), element_count);
    const std::span<RenderFixturePassRequest> request_span(pass_requests.data(), element_count);
    const std::span<RenderFixturePassResult> result_span(pass_results.data(), element_count);

    UiRenderCoreBridgeRequest request{};
    request.pass = &pass;
    request.submission_batch = &batch;
    request.draw_elements = element_span;
    request.template_pass_request = template_request;
    request.pass_requests = request_span;
    request.pass_results = result_span;
    request.pass_id_base = PREVIEW_PASS_ID_BASE;

    const UiRenderCoreBridgeResult bridge_result = bridge.Submit(request);
    if (bridge_result.status != UiRenderCoreBridgeStatus::Success) {
        return false;
    }

    const RhiDeviceSnapshot device_snapshot = device.Snapshot();
    const std::uint64_t expected_submit_count = static_cast<std::uint64_t>(element_count);
    if (device_snapshot.submit_count != expected_submit_count) {
        return false;
    }

    if (device_snapshot.present_count != expected_submit_count) {
        return false;
    }

    record->submitted_entry_count = bridge_result.completed_entry_count;
    record->render_submit_count = device_snapshot.submit_count;
    return true;
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
      preview_variants_{},
      preview_viewport_record_{},
      snapshot_{
          UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT,
          0U,
          UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT,
          UI_EDITOR_SHELL_REQUIRED_PANEL_COUNT,
          0U,
          0U,
          0U,
          false,
          false,
          false,
          0U,
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

UiEditorShellStatus UiEditorShellState::RegisterPreviewVariant(const UiEditorPreviewVariantDesc &desc) {
    if (!layout_document_.is_loaded) {
        return RecordStatus(UiEditorShellStatus::LayoutNotLoaded);
    }

    if (snapshot_.preview_variant_count >= UI_EDITOR_PREVIEW_VARIANT_CAPACITY) {
        return RecordStatus(UiEditorShellStatus::PreviewVariantCapacityExceeded);
    }

    if (HasPreviewVariantId(desc.variant_id)) {
        return RecordStatus(UiEditorShellStatus::DuplicatePreviewVariantId);
    }

    UiEditorPreviewVariantRecord record;
    const UiEditorShellStatus build_status = BuildPreviewVariant(desc, &record);
    if (build_status != UiEditorShellStatus::Success) {
        return RecordStatus(build_status);
    }

    preview_variants_[snapshot_.preview_variant_count] = record;
    ++snapshot_.preview_variant_count;
    return RecordStatus(UiEditorShellStatus::Success);
}

UiEditorShellStatus UiEditorShellState::ExportPreviewVariants(
    UiEditorPreviewVariantRecord *output_records,
    std::uint32_t output_capacity,
    std::uint32_t *out_count) {
    if (out_count == nullptr) {
        return RecordStatus(UiEditorShellStatus::InvalidOutput);
    }

    *out_count = snapshot_.preview_variant_count;
    if (output_capacity < snapshot_.preview_variant_count) {
        return RecordStatus(UiEditorShellStatus::OutputCapacityExceeded);
    }

    if (output_records == nullptr) {
        return RecordStatus(UiEditorShellStatus::InvalidOutput);
    }

    for (std::uint32_t index = 0U; index < snapshot_.preview_variant_count; ++index) {
        output_records[index] = preview_variants_[index];
    }

    return RecordStatus(UiEditorShellStatus::Success);
}

UiEditorShellStatus UiEditorShellState::BuildRuntimePreviewViewport(
    const UiEditorPreviewViewportDesc &desc,
    UiEditorPreviewViewportRecord *out_record) {
    if (out_record == nullptr) {
        return RecordStatus(UiEditorShellStatus::InvalidOutput);
    }

    *out_record = {};
    ResetPreviewState();

    if (!layout_document_.is_loaded) {
        return RecordStatus(UiEditorShellStatus::LayoutNotLoaded);
    }

    if (desc.viewport_width == 0U || desc.viewport_height == 0U) {
        return RecordStatus(UiEditorShellStatus::InvalidPreviewViewport);
    }

    if (desc.viewport_width > PREVIEW_MAX_RHI_EXTENT || desc.viewport_height > PREVIEW_MAX_RHI_EXTENT) {
        return RecordStatus(UiEditorShellStatus::InvalidPreviewViewport);
    }

    UiNodeTreeDesc tree_desc;
    tree_desc.node_capacity = UI_EDITOR_LAYOUT_MAX_NODE_COUNT;
    tree_desc.viewport_rect = UiRect{
        0.0F,
        0.0F,
        static_cast<float>(desc.viewport_width),
        static_cast<float>(desc.viewport_height)};
    UiNodeTree tree(tree_desc);
    if (!BuildPreviewNodeTree(layout_document_, desc, &tree)) {
        return RecordStatus(UiEditorShellStatus::PreviewNodeTreeFailed);
    }

    std::uint32_t layout_container_count = 0U;
    if (!ApplyPreviewLayout(&tree, layout_document_, &layout_container_count)) {
        return RecordStatus(UiEditorShellStatus::PreviewLayoutPassFailed);
    }

    std::array<UiDrawElement, UI_EDITOR_PREVIEW_MAX_DRAW_ELEMENT_COUNT> elements{};
    std::uint32_t draw_element_count = 0U;
    if (!BuildPreviewDrawElements(tree, layout_document_, &elements, &draw_element_count)) {
        return RecordStatus(UiEditorShellStatus::PreviewDrawListFailed);
    }

    UiEditorPreviewViewportRecord record;
    record.viewport_width = desc.viewport_width;
    record.viewport_height = desc.viewport_height;
    record.layout_node_count = layout_document_.node_count;
    record.layout_container_count = layout_container_count;
    record.draw_element_count = draw_element_count;
    if (!CopyString(std::string_view(layout_document_.layout_id), record.layout_id, UI_EDITOR_LAYOUT_ID_CAPACITY)) {
        return RecordStatus(UiEditorShellStatus::InvalidLayoutAsset);
    }

    if (!SubmitPreviewDrawElements(elements, draw_element_count, &record)) {
        return RecordStatus(UiEditorShellStatus::PreviewRenderSubmitFailed);
    }

    record.is_ready = true;
    record.uses_headless_rendercore_path = true;
    preview_viewport_record_ = record;
    snapshot_.preview_ready = true;
    snapshot_.preview_draw_element_count = record.draw_element_count;

    UiEditorShellPanelRecord *preview_panel = FindPanel(UiEditorShellPanelId::Preview);
    if (preview_panel == nullptr) {
        ResetPreviewState();
        return RecordStatus(UiEditorShellStatus::InvalidPanel);
    }

    preview_panel->is_placeholder = false;
    RecountPanels();
    *out_record = preview_viewport_record_;
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

bool UiEditorShellState::HasPreviewVariantId(std::uint32_t variant_id) const {
    if (variant_id == 0U) {
        return false;
    }

    for (std::uint32_t index = 0U; index < snapshot_.preview_variant_count; ++index) {
        if (preview_variants_[index].variant_id == variant_id) {
            return true;
        }
    }

    return false;
}

UiEditorShellStatus UiEditorShellState::BuildPreviewVariant(
    const UiEditorPreviewVariantDesc &desc,
    UiEditorPreviewVariantRecord *out_record) const {
    if (out_record == nullptr) {
        return UiEditorShellStatus::InvalidOutput;
    }

    if (desc.variant_id == 0U || desc.target_width == 0U || desc.target_height == 0U) {
        return UiEditorShellStatus::InvalidPreviewVariant;
    }

    if (desc.dpi_scale_percent == 0U) {
        return UiEditorShellStatus::InvalidPreviewVariant;
    }

    const std::uint32_t horizontal_safe_area = desc.safe_area.left + desc.safe_area.right;
    const std::uint32_t vertical_safe_area = desc.safe_area.top + desc.safe_area.bottom;
    if (horizontal_safe_area >= desc.target_width) {
        return UiEditorShellStatus::InvalidPreviewVariant;
    }

    if (vertical_safe_area >= desc.target_height) {
        return UiEditorShellStatus::InvalidPreviewVariant;
    }

    const float dpi_scale = static_cast<float>(desc.dpi_scale_percent) / 100.0F;
    const float safe_width = static_cast<float>(desc.target_width - horizontal_safe_area);
    const float safe_height = static_cast<float>(desc.target_height - vertical_safe_area);
    UiEditorPreviewVariantRecord record;
    record.variant_id = desc.variant_id;
    record.target_width = desc.target_width;
    record.target_height = desc.target_height;
    record.dpi_scale_percent = desc.dpi_scale_percent;
    record.safe_area = desc.safe_area;
    record.logical_x = static_cast<float>(desc.safe_area.left) / dpi_scale;
    record.logical_y = static_cast<float>(desc.safe_area.top) / dpi_scale;
    record.logical_width = safe_width / dpi_scale;
    record.logical_height = safe_height / dpi_scale;
    record.previewed_node_count = layout_document_.node_count;
    *out_record = record;
    return UiEditorShellStatus::Success;
}

void UiEditorShellState::ResetLayoutState() {
    layout_document_ = {};
    inspector_record_ = {};
    preview_variants_ = {};
    snapshot_.layout_loaded = false;
    snapshot_.loaded_node_count = 0U;
    snapshot_.selected_node_id = 0U;
    snapshot_.preview_variant_count = 0U;
    ResetPreviewState();

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

void UiEditorShellState::ResetPreviewState() {
    preview_viewport_record_ = {};
    snapshot_.preview_ready = false;
    snapshot_.preview_draw_element_count = 0U;

    UiEditorShellPanelRecord *preview_panel = FindPanel(UiEditorShellPanelId::Preview);
    if (preview_panel != nullptr) {
        preview_panel->is_placeholder = true;
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
