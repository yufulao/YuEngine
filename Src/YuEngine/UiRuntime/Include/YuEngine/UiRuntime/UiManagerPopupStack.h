// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerPopupStack.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/BaseUiController.h"
#include "YuEngine/UiRuntime/UiManagerLayerModel.h"
#include "YuEngine/UiRuntime/UiManagerPanelMap.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackResult.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackSnapshot.h"
#include "YuEngine/UiRuntime/UiManagerPopupStackStatus.h"
#include "YuEngine/UiRuntime/UiPanelRegistry.h"

namespace yuengine::uiruntime {
class UiManagerPopupStack final {
public:
    /**
     * @comment 构造 UIManager popup stack。
     */
    UiManagerPopupStack();

    /**
     * @comment 打开 popup panel，并把它移动到 stack 顶部。
     * @param panel_id 输入 panel id。
     * @param registry 输入 panel registry。
     * @param layer_model 输入 layer model。
     * @param panel_map 输入输出 panel map。
     * @param controller 首次加载时由调用方持有的 controller。
     * @return 显式 popup 打开结果。
     */
    UiManagerPopupStackResult OpenPopupPanel(
        UiPanelId panel_id,
        const UiPanelRegistry &registry,
        const UiManagerLayerModel &layer_model,
        UiManagerPanelMap *panel_map,
        BaseUiController *controller);

    /**
     * @comment 携带 open 参数打开 popup panel，并把它移动到 stack 顶部。
     * @param panel_id 输入 panel id。
     * @param registry 输入 panel registry。
     * @param layer_model 输入 layer model。
     * @param panel_map 输入输出 panel map。
     * @param controller 首次加载时由调用方持有的 controller。
     * @param open_args 调用方提供的 open 参数视图，仅在本次调用期间有效。
     * @return 显式 popup 打开结果。
     */
    UiManagerPopupStackResult OpenPopupPanelWithArgs(
        UiPanelId panel_id,
        const UiPanelRegistry &registry,
        const UiManagerLayerModel &layer_model,
        UiManagerPanelMap *panel_map,
        BaseUiController *controller,
        const UiPanelOpenArgs &open_args);

    /**
     * @comment 将已打开 popup 移动到 stack 顶部。
     * @param panel_id 输入 panel id。
     * @param panel_map 输入 panel map。
     * @return 显式置顶结果。
     */
    UiManagerPopupStackResult BringPopupToTop(
        UiPanelId panel_id,
        const UiManagerPanelMap &panel_map);

    /**
     * @comment 关闭 popup panel，并从 stack 移除。
     * @param panel_id 输入 panel id。
     * @param panel_map 输入输出 panel map。
     * @return 显式关闭结果。
     */
    UiManagerPopupStackResult ClosePopupPanel(UiPanelId panel_id, UiManagerPanelMap *panel_map);

    /**
     * @comment 导出 popup 顺序，索引越大越靠近顶部。
     * @param output_panel_ids 输出 panel id。
     * @param output_capacity 输出容量。
     * @return 显式导出状态。
     */
    UiManagerPopupStackStatus ExportPopupOrder(
        UiPanelId *output_panel_ids,
        std::uint32_t output_capacity) const;

    /**
     * @comment 查询 popup 是否在 stack 中。
     * @param panel_id 输入 panel id。
     * @return 在 stack 中返回 true，否则返回 false。
     */
    bool IsPopupStacked(UiPanelId panel_id) const;

    /**
     * @comment 获取 popup stack 快照。
     * @return 当前 popup stack 快照。
     */
    UiManagerPopupStackSnapshot Snapshot() const;

private:
    UiManagerPopupStackStatus ValidatePopupLayer(
        UiPanelId panel_id,
        const UiPanelRegistry &registry,
        const UiManagerLayerModel &layer_model,
        UiManagerLayerRecord *out_layer_record) const;
    UiManagerPopupStackStatus ResolveLoadedPopupRecord(
        UiPanelId panel_id,
        const UiManagerPanelMap &panel_map,
        UiManagerPanelMapRecord *out_record,
        UiManagerPanelMapStatus *out_panel_map_status) const;
    UiManagerPopupStackStatus ResolveActivePopupRecord(
        UiPanelId panel_id,
        const UiManagerPanelMap &panel_map,
        UiManagerPanelMapRecord *out_record,
        UiManagerPanelMapStatus *out_panel_map_status) const;
    UiManagerPopupStackStatus TranslateRegistryStatus(UiPanelRegistryStatus status) const;
    UiManagerPopupStackStatus TranslateLayerStatus(UiManagerLayerModelStatus status) const;
    UiManagerPopupStackStatus TranslatePanelMapStatus(UiManagerPanelMapStatus status) const;
    std::uint32_t FindStackIndex(UiPanelId panel_id) const;
    void PushPopup(UiPanelId panel_id);
    void MovePopupToTop(std::uint32_t stack_index);
    void RemovePopupAt(std::uint32_t stack_index);
    void RefreshTopPanel();
    UiManagerPopupStackResult MakeResult(
        UiManagerPopupStackStatus status,
        UiManagerPanelMapStatus panel_map_status,
        const UiManagerPanelMapRecord &record,
        UiPanelId panel_id,
        bool pushed,
        bool brought_to_top,
        bool already_top,
        bool already_in_stack,
        bool already_inactive,
        bool removed_from_stack) const;
    UiManagerPopupStackStatus RecordFailure(UiManagerPopupStackStatus status);
    void RecordSuccess();

    UiManagerPopupStackSnapshot snapshot_;
};
}
