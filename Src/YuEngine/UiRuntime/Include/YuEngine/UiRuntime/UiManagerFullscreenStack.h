// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerFullscreenStack.h

#pragma once

#include <cstdint>

#include "YuEngine/UiRuntime/BaseUiController.h"
#include "YuEngine/UiRuntime/UiManagerFullscreenStackResult.h"
#include "YuEngine/UiRuntime/UiManagerFullscreenStackSnapshot.h"
#include "YuEngine/UiRuntime/UiManagerFullscreenStackStatus.h"
#include "YuEngine/UiRuntime/UiManagerLayerModel.h"
#include "YuEngine/UiRuntime/UiManagerPanelMap.h"
#include "YuEngine/UiRuntime/UiPanelRegistry.h"

namespace yuengine::uiruntime {
class UiManagerFullscreenStack final {
public:
    /**
     * @comment 构造 UIManager fullscreen stack。
     */
    UiManagerFullscreenStack();

    /**
     * @comment 打开 fullscreen panel，并把它设为当前 fullscreen。
     * @param panel_id 输入 panel id。
     * @param registry 输入 panel registry。
     * @param layer_model 输入 layer model。
     * @param panel_map 输入输出 panel map。
     * @param controller 首次加载时由调用方持有的 controller。
     * @return 显式 fullscreen 打开结果。
     */
    UiManagerFullscreenStackResult OpenFullscreenPanel(
        UiPanelId panel_id,
        const UiPanelRegistry &registry,
        const UiManagerLayerModel &layer_model,
        UiManagerPanelMap *panel_map,
        BaseUiController *controller);

    /**
     * @comment 携带 open 参数打开 fullscreen panel，并把它设为当前 fullscreen。
     * @param panel_id 输入 panel id。
     * @param registry 输入 panel registry。
     * @param layer_model 输入 layer model。
     * @param panel_map 输入输出 panel map。
     * @param controller 首次加载时由调用方持有的 controller。
     * @param open_args 调用方提供的 open 参数视图，仅在本次调用期间有效。
     * @return 显式 fullscreen 打开结果。
     */
    UiManagerFullscreenStackResult OpenFullscreenPanelWithArgs(
        UiPanelId panel_id,
        const UiPanelRegistry &registry,
        const UiManagerLayerModel &layer_model,
        UiManagerPanelMap *panel_map,
        BaseUiController *controller,
        const UiPanelOpenArgs &open_args);

    /**
     * @comment 执行 fullscreen navigate-back，关闭当前 fullscreen 并恢复前序 fullscreen。
     * @param registry 输入 panel registry。
     * @param layer_model 输入 layer model。
     * @param panel_map 输入输出 panel map。
     * @return 显式 back navigation 结果。
     */
    UiManagerFullscreenStackResult NavigateBack(
        const UiPanelRegistry &registry,
        const UiManagerLayerModel &layer_model,
        UiManagerPanelMap *panel_map);

    /**
     * @comment 关闭 fullscreen panel，并在关闭当前 fullscreen 时恢复前序 fullscreen。
     * @param panel_id 输入 panel id。
     * @param registry 输入 panel registry。
     * @param layer_model 输入 layer model。
     * @param panel_map 输入输出 panel map。
     * @return 显式关闭结果。
     */
    UiManagerFullscreenStackResult CloseFullscreenPanel(
        UiPanelId panel_id,
        const UiPanelRegistry &registry,
        const UiManagerLayerModel &layer_model,
        UiManagerPanelMap *panel_map);

    /**
     * @comment 释放 fullscreen panel，并在释放当前 fullscreen 时恢复前序 fullscreen。
     * @param panel_id 输入 panel id。
     * @param registry 输入 panel registry。
     * @param layer_model 输入 layer model。
     * @param panel_map 输入输出 panel map。
     * @return 显式释放结果。
     */
    UiManagerFullscreenStackResult ReleaseFullscreenPanel(
        UiPanelId panel_id,
        const UiPanelRegistry &registry,
        const UiManagerLayerModel &layer_model,
        UiManagerPanelMap *panel_map);

    /**
     * @comment 导出 fullscreen 顺序，索引越大越靠近当前 fullscreen。
     * @param output_panel_ids 输出 panel id。
     * @param output_capacity 输出容量。
     * @return 显式导出结果。
     */
    UiManagerFullscreenStackResult ExportFullscreenOrder(
        UiPanelId *output_panel_ids,
        std::uint32_t output_capacity);

    /**
     * @comment 查询 fullscreen 是否在 stack 中。
     * @param panel_id 输入 panel id。
     * @return 在 stack 中返回 true，否则返回 false。
     */
    bool IsFullscreenStacked(UiPanelId panel_id) const;

    /**
     * @comment 获取 fullscreen stack 快照。
     * @return 当前 fullscreen stack 快照。
     */
    UiManagerFullscreenStackSnapshot Snapshot() const;

private:
    UiManagerFullscreenStackStatus ValidateFullscreenLayer(
        UiPanelId panel_id,
        const UiPanelRegistry &registry,
        const UiManagerLayerModel &layer_model,
        UiManagerLayerRecord *out_layer_record) const;
    UiManagerFullscreenStackStatus ResolveLoadedFullscreenRecord(
        UiPanelId panel_id,
        const UiManagerPanelMap &panel_map,
        UiManagerPanelMapRecord *out_record,
        UiManagerPanelMapStatus *out_panel_map_status) const;
    UiManagerFullscreenStackStatus TranslateRegistryStatus(UiPanelRegistryStatus status) const;
    UiManagerFullscreenStackStatus TranslateLayerStatus(UiManagerLayerModelStatus status) const;
    UiManagerFullscreenStackStatus TranslatePanelMapStatus(UiManagerPanelMapStatus status) const;
    UiManagerFullscreenStackResult RestoreTopFullscreen(
        const UiPanelRegistry &registry,
        const UiManagerLayerModel &layer_model,
        UiManagerPanelMap *panel_map,
        UiPanelId panel_id,
        UiPanelId closed_panel_id,
        const UiManagerPanelMapRecord &record,
        bool navigated_back,
        bool already_inactive,
        bool closed_current,
        bool closed_middle);
    std::uint32_t FindStackIndex(UiPanelId panel_id) const;
    void PushFullscreen(UiPanelId panel_id);
    void MoveFullscreenToTop(std::uint32_t stack_index);
    void RemoveFullscreenAt(std::uint32_t stack_index);
    void RefreshTopPanel();
    UiManagerFullscreenStackResult MakeResult(
        UiManagerFullscreenStackStatus status,
        UiManagerPanelMapStatus panel_map_status,
        const UiManagerPanelMapRecord &record,
        UiPanelId panel_id,
        UiPanelId closed_panel_id,
        UiPanelId restored_panel_id,
        bool pushed,
        bool moved_to_top,
        bool navigated_back,
        bool restored_previous,
        bool already_top,
        bool already_in_stack,
        bool already_inactive,
        bool removed_from_stack,
        bool closed_current,
        bool closed_middle) const;
    UiManagerFullscreenStackStatus RecordFailure(UiManagerFullscreenStackStatus status);
    UiManagerFullscreenStackStatus RecordFullscreenOrderCapacityFailure(UiPanelId panel_id);
    void RecordSuccess();
    void ClearFullscreenOrderCapacityFailure();

    UiManagerFullscreenStackSnapshot snapshot_;
};
}
