// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerLayerModel.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/UiRuntime/UiManagerLayerModelConstants.h"
#include "YuEngine/UiRuntime/UiManagerLayerModelDesc.h"
#include "YuEngine/UiRuntime/UiManagerLayerModelResult.h"
#include "YuEngine/UiRuntime/UiManagerLayerModelSnapshot.h"
#include "YuEngine/UiRuntime/UiManagerLayerModelStatus.h"
#include "YuEngine/UiRuntime/UiManagerLayerRecord.h"
#include "YuEngine/UiRuntime/UiManagerPanelLayerBinding.h"
#include "YuEngine/UiRuntime/UiPanelId.h"

namespace yuengine::uiruntime {
class UiManagerLayerModel final {
public:
    /**
     * @comment 构造 UIManager layer model。
     */
    UiManagerLayerModel();

    /**
     * @comment 构造 UIManager layer model。
     * @param desc layer model 描述。
     */
    explicit UiManagerLayerModel(UiManagerLayerModelDesc desc);

    /**
     * @comment 注册单个 UIManager layer root 记录。
     * @param record 输入 layer 记录。
     * @return 显式注册结果。
     */
    UiManagerLayerModelResult RegisterLayer(const UiManagerLayerRecord &record);

    /**
     * @comment 原子注册一组 UIManager layer root 记录。
     * @param layer_set 输入 layer 记录集合。
     * @return 显式注册结果。
     */
    UiManagerLayerModelResult RegisterLayerSet(const UiManagerLayerSet &layer_set);

    /**
     * @comment 将 panel id 绑定到已注册 layer。
     * @param binding 输入 panel-layer 绑定。
     * @return 显式绑定结果。
     */
    UiManagerLayerModelResult BindPanelToLayer(const UiManagerPanelLayerBinding &binding);

    /**
     * @comment 解析 layer id 到 layer root 记录。
     * @param layer_id 输入 layer id。
     * @param out_record 输出 layer 记录。
     * @return 显式解析状态。
     */
    UiManagerLayerModelStatus ResolveLayer(
        UiManagerLayerId layer_id,
        UiManagerLayerRecord *out_record) const;

    /**
     * @comment 解析 layer type 到 layer root 记录。
     * @param type 输入 layer type。
     * @param out_record 输出 layer 记录。
     * @return 显式解析状态。
     */
    UiManagerLayerModelStatus ResolveLayerByType(
        UiManagerLayerType type,
        UiManagerLayerRecord *out_record) const;

    /**
     * @comment 解析 panel id 到 layer root 记录。
     * @param panel_id 输入 panel id。
     * @param out_record 输出 layer 记录。
     * @return 显式解析状态。
     */
    UiManagerLayerModelStatus ResolvePanelLayer(
        UiPanelId panel_id,
        UiManagerLayerRecord *out_record) const;

    /**
     * @comment 按稳定 order 导出 layer root 记录。
     * @param output_records 输出 layer 记录。
     * @param output_capacity 输出容量。
     * @return 显式导出结果。
     */
    UiManagerLayerModelResult ExportLayers(
        UiManagerLayerRecord *output_records,
        std::uint32_t output_capacity) const;

    /**
     * @comment 获取 layer model 快照。
     * @return 当前 layer model 快照。
     */
    UiManagerLayerModelSnapshot Snapshot() const;

private:
    UiManagerLayerModelStatus ValidateLayerType(UiManagerLayerType type) const;
    UiManagerLayerModelStatus ValidateLayerRecord(const UiManagerLayerRecord &record) const;
    UiManagerLayerModelStatus ValidateLayerSet(const UiManagerLayerSet &layer_set) const;
    UiManagerLayerModelStatus ValidateBinding(const UiManagerPanelLayerBinding &binding) const;
    bool HasLayer(UiManagerLayerId layer_id) const;
    bool HasLayerType(UiManagerLayerType type) const;
    bool HasPanelBinding(UiPanelId panel_id) const;
    const UiManagerLayerRecord *FindLayer(UiManagerLayerId layer_id) const;
    const UiManagerLayerRecord *FindLayerByType(UiManagerLayerType type) const;
    const UiManagerPanelLayerBinding *FindBinding(UiPanelId panel_id) const;
    std::uint32_t FindFreeLayerRecordIndex() const;
    std::uint32_t FindFreeBindingRecordIndex() const;
    UiManagerLayerModelResult InsertLayerRecord(const UiManagerLayerRecord &record);
    UiManagerLayerModelResult InsertBindingRecord(const UiManagerPanelLayerBinding &binding);
    void InsertSortedLayerRecord(
        const UiManagerLayerRecord &record,
        std::array<UiManagerLayerRecord, MAX_UI_MANAGER_LAYER_COUNT> &sorted_records,
        std::uint32_t *sorted_count) const;
    UiManagerLayerModelResult MakeResult(
        UiManagerLayerModelStatus status,
        const UiManagerLayerRecord &layer_record,
        const UiManagerPanelLayerBinding &binding_record,
        std::uint32_t record_index) const;
    UiManagerLayerModelResult MakeCapacityResult(
        UiManagerLayerModelOperationKind operation_kind,
        const UiManagerLayerRecord &layer_record,
        const UiManagerPanelLayerBinding &binding_record,
        std::uint32_t failed_record_index,
        std::uint32_t required_layer_count,
        std::uint32_t required_binding_count);
    UiManagerLayerModelResult MakeLayerSetCapacityResult(
        const UiManagerLayerSet &layer_set,
        std::uint32_t layer_count);
    UiManagerLayerModelStatus RecordFailure(UiManagerLayerModelStatus status);
    void RecordSuccess();
    void ClearCapacityEntry();
    void RecordCapacityEntry(
        UiManagerLayerModelOperationKind operation_kind,
        UiManagerLayerId failed_layer_id,
        UiPanelId failed_panel_id,
        std::uint32_t failed_record_index,
        std::uint32_t required_layer_count,
        std::uint32_t required_binding_count);

    std::array<UiManagerLayerRecord, MAX_UI_MANAGER_LAYER_COUNT> layer_records_;
    std::array<bool, MAX_UI_MANAGER_LAYER_COUNT> used_layer_records_;
    std::array<UiManagerPanelLayerBinding, MAX_UI_MANAGER_PANEL_LAYER_BINDING_COUNT> binding_records_;
    std::array<bool, MAX_UI_MANAGER_PANEL_LAYER_BINDING_COUNT> used_binding_records_;
    UiManagerLayerModelSnapshot snapshot_;
};
}
