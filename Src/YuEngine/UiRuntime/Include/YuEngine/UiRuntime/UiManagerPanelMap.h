// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiManagerPanelMap.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/UiRuntime/BaseUiController.h"
#include "YuEngine/UiRuntime/UiManagerLayerModel.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapConstants.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapDesc.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapRecord.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapResult.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapSnapshot.h"
#include "YuEngine/UiRuntime/UiManagerPanelMapStatus.h"
#include "YuEngine/UiRuntime/UiPanelRegistry.h"

namespace yuengine::uiruntime {
class UiManagerPanelMap final {
public:
    /**
     * @comment 构造 UIManager panel map。
     */
    UiManagerPanelMap();

    /**
     * @comment 构造 UIManager panel map。
     * @param desc panel map 描述。
     */
    explicit UiManagerPanelMap(UiManagerPanelMapDesc desc);

    /**
     * @comment 打开已注册 panel，并写入 loaded 与 active map。
     * @param panel_id 输入 panel id。
     * @param registry 输入 panel registry。
     * @param layer_model 输入 layer model。
     * @param controller 首次加载时由调用方持有的 controller。
     * @return 显式打开结果。
     */
    UiManagerPanelMapResult OpenPanel(
        UiPanelId panel_id,
        const UiPanelRegistry &registry,
        const UiManagerLayerModel &layer_model,
        BaseUiController *controller);

    /**
     * @comment 携带 open 参数打开已注册 panel，并写入 loaded 与 active map。
     * @param panel_id 输入 panel id。
     * @param registry 输入 panel registry。
     * @param layer_model 输入 layer model。
     * @param controller 首次加载时由调用方持有的 controller。
     * @param open_args 调用方提供的 open 参数视图，仅在本次调用期间有效。
     * @return 显式打开结果。
     */
    UiManagerPanelMapResult OpenPanelWithArgs(
        UiPanelId panel_id,
        const UiPanelRegistry &registry,
        const UiManagerLayerModel &layer_model,
        BaseUiController *controller,
        const UiPanelOpenArgs &open_args);

    /**
     * @comment 关闭 active panel，并保留 loaded map 记录。
     * @param panel_id 输入 panel id。
     * @return 显式关闭结果。
     */
    UiManagerPanelMapResult ClosePanel(UiPanelId panel_id);

    /**
     * @comment 释放 loaded panel，并清理 loaded map 记录与 open 参数快照。
     * @param panel_id 输入 panel id。
     * @return 显式释放结果。
     */
    UiManagerPanelMapResult ReleasePanel(UiPanelId panel_id);

    /**
     * @comment 解析 loaded panel 记录。
     * @param panel_id 输入 panel id。
     * @param out_record 输出 loaded panel 记录。
     * @return 显式解析状态。
     */
    UiManagerPanelMapStatus ResolveLoadedPanel(
        UiPanelId panel_id,
        UiManagerPanelMapRecord *out_record) const;

    /**
     * @comment 解析 active panel 记录。
     * @param panel_id 输入 panel id。
     * @param out_record 输出 active panel 记录。
     * @return 显式解析状态。
     */
    UiManagerPanelMapStatus ResolveActivePanel(
        UiPanelId panel_id,
        UiManagerPanelMapRecord *out_record) const;

    /**
     * @comment 查询 panel 是否已加载。
     * @param panel_id 输入 panel id。
     * @return 已加载时返回 true，否则返回 false。
     */
    bool IsLoaded(UiPanelId panel_id) const;

    /**
     * @comment 查询 panel 是否处于 active map。
     * @param panel_id 输入 panel id。
     * @return active 时返回 true，否则返回 false。
     */
    bool IsActive(UiPanelId panel_id) const;

    /**
     * @comment 获取 panel map 快照。
     * @return 当前 panel map 快照。
     */
    UiManagerPanelMapSnapshot Snapshot() const;

private:
    UiManagerPanelMapStatus ResolveManifestAndLayer(
        UiPanelId panel_id,
        const UiPanelRegistry &registry,
        const UiManagerLayerModel &layer_model,
        UiPanelManifestRecord *out_manifest_record,
        UiManagerLayerRecord *out_layer_record) const;
    UiManagerPanelMapStatus TranslateRegistryStatus(UiPanelRegistryStatus status) const;
    UiManagerPanelMapStatus TranslateLayerStatus(UiManagerLayerModelStatus status) const;
    UiManagerPanelMapStatus ValidateOpenArgs(const UiPanelOpenArgs &open_args) const;
    UiPanelOpenArgsSnapshot MakeOpenArgsSnapshot(const UiPanelOpenArgs &open_args) const;
    UiManagerPanelMapRecord *FindLoadedRecord(UiPanelId panel_id);
    const UiManagerPanelMapRecord *FindLoadedRecord(UiPanelId panel_id) const;
    std::uint32_t FindLoadedRecordIndex(UiPanelId panel_id) const;
    std::uint32_t FindFreeRecordIndex() const;
    UiManagerPanelMapResult MakeResult(
        UiManagerPanelMapStatus status,
        const UiManagerPanelMapRecord &record,
        bool reused_loaded,
        bool already_active,
        bool already_inactive,
        bool released_loaded=false,
        bool released_active=false) const;
    UiManagerPanelMapResult MakeCapacityResult(
        UiPanelId panel_id,
        const UiPanelOpenArgsSnapshot &open_args_snapshot);
    UiManagerPanelMapStatus RecordCapacityFailure(
        UiPanelId panel_id,
        const UiPanelOpenArgsSnapshot &open_args_snapshot);
    UiManagerPanelMapStatus RecordFailure(UiManagerPanelMapStatus status);
    void RecordOpenArgsAccepted(const UiPanelOpenArgsSnapshot &open_args_snapshot, bool reused_loaded);
    void ClearCapacityEntry();
    void RecordSuccess();

    std::array<UiManagerPanelMapRecord, MAX_UI_MANAGER_PANEL_MAP_RECORD_COUNT> records_;
    UiManagerPanelMapSnapshot snapshot_;
};
}
