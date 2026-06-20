// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Include/YuEngine/UiRuntime/UiPanelRegistry.h

#pragma once

#include <array>
#include <cstdint>

#include "YuEngine/UiRuntime/UiPanelId.h"
#include "YuEngine/UiRuntime/UiPanelManifestRecord.h"
#include "YuEngine/UiRuntime/UiPanelRegistryDesc.h"
#include "YuEngine/UiRuntime/UiPanelRegistryResult.h"
#include "YuEngine/UiRuntime/UiPanelRegistrySnapshot.h"
#include "YuEngine/UiRuntime/UiPanelRegistryStatus.h"

namespace yuengine::uiruntime {
class UiPanelRegistry final {
public:
    /**
     * @comment 构造 panel registry。
     */
    UiPanelRegistry();

    /**
     * @comment 构造 panel registry。
     * @param desc registry 描述。
     */
    explicit UiPanelRegistry(UiPanelRegistryDesc desc);

    /**
     * @comment 注册单个显式 panel manifest record。
     * @param record 输入 manifest record。
     * @return 显式注册结果。
     */
    UiPanelRegistryResult RegisterPanel(const UiPanelManifestRecord &record);

    /**
     * @comment 原子注册测试 manifest 中的 panel records。
     * @param manifest caller-provided 测试 manifest。
     * @return 显式注册结果。
     */
    UiPanelRegistryResult RegisterManifest(const UiPanelTestManifest &manifest);

    /**
     * @comment 解析 panel id 到 layout/controller/resource refs。
     * @param panel_id 输入 panel id。
     * @param out_record 输出 manifest record。
     * @return 显式解析状态。
     */
    UiPanelRegistryStatus ResolvePanel(UiPanelId panel_id, UiPanelManifestRecord *out_record) const;

    /**
     * @comment 将 registry manifest 复制到 caller-owned 输出存储。
     * @param output_records 输出 manifest records。
     * @param output_capacity 输出容量。
     * @return 显式导出结果。
     */
    UiPanelRegistryResult ExportManifest(
        UiPanelManifestRecord *output_records,
        std::uint32_t output_capacity) const;

    /**
     * @comment 获取 registry 快照。
     * @return 当前 registry 快照。
     */
    UiPanelRegistrySnapshot Snapshot() const;

private:
    UiPanelRegistryStatus ValidateRecord(const UiPanelManifestRecord &record) const;
    UiPanelRegistryStatus ValidateManifest(const UiPanelTestManifest &manifest) const;
    UiPanelRegistryStatus ValidateResourceRefs(const UiPanelManifestRecord &record) const;
    UiPanelRegistryStatus ValidateNoManifestDuplicate(const UiPanelTestManifest &manifest) const;
    bool HasPanel(UiPanelId panel_id) const;
    const UiPanelManifestRecord *FindRecord(UiPanelId panel_id) const;
    std::uint32_t FindFreeRecordIndex() const;
    UiPanelRegistryResult InsertRecord(const UiPanelManifestRecord &record);
    UiPanelRegistryResult MakeResult(
        UiPanelRegistryStatus status,
        const UiPanelManifestRecord &record,
        std::uint32_t record_index) const;
    UiPanelRegistryStatus RecordFailure(UiPanelRegistryStatus status);
    void RecordSuccess();

    std::array<UiPanelManifestRecord, MAX_UI_PANEL_REGISTRY_RECORD_COUNT> records_;
    std::array<bool, MAX_UI_PANEL_REGISTRY_RECORD_COUNT> active_records_;
    UiPanelRegistrySnapshot snapshot_;
};
}
