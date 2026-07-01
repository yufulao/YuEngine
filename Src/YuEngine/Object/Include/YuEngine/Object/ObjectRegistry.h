// 模块: YuEngine Object
// 文件: Src/YuEngine/Object/Include/YuEngine/Object/ObjectRegistry.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "YuEngine/Object/ObjectConstants.h"
#include "YuEngine/Object/ObjectDescriptor.h"
#include "YuEngine/Object/ObjectHandle.h"
#include "YuEngine/Object/ObjectRegistrationResult.h"
#include "YuEngine/Object/ObjectRegistryDesc.h"
#include "YuEngine/Object/ObjectSnapshot.h"
#include "YuEngine/Object/ObjectStatus.h"
#include "YuEngine/Object/ObjectTypeId.h"
#include "YuEngine/Object/ObjectSlot.h"

namespace yuengine::object {
class ObjectRegistry final {
public:
    /**
     * @comment 构造 ObjectRegistry 实例。
     */
    ObjectRegistry();
    /**
     * @comment 构造 ObjectRegistry 实例。
     * @param desc 输入 descriptor。
     */
    explicit ObjectRegistry(ObjectRegistryDesc desc);

    /**
     * @comment 创建 synthetic object。
     * @param descriptor 输入 descriptor。
     * @return 显式操作结果。
     */
    ObjectRegistrationResult CreateSyntheticObject(const ObjectDescriptor& descriptor);
    /**
     * @comment 校验 operation。
     * @param handle 输入 handle。
     * @return 显式操作状态。
     */
    ObjectStatus Validate(ObjectHandle handle);
    /**
     * @comment 校验 projected acquire 是否可以在不修改 registry state 时成功。
     * @param handle 输入 handle。
     * @param projected_acquire_count 输入 acquire count budget。
     * @return 显式操作状态。
     */
    ObjectStatus ValidateAcquire(ObjectHandle handle, std::uint32_t projected_acquire_count) const;
    /**
     * @comment 获取 operation。
     * @param handle 输入 handle。
     * @return 显式操作状态。
     */
    ObjectStatus Acquire(ObjectHandle handle);
    /**
     * @comment 释放 operation。
     * @param handle 输入 handle。
     * @return 显式操作状态。
     */
    ObjectStatus Release(ObjectHandle handle);
    /**
     * @comment 销毁 operation。
     * @param handle 输入 handle。
     * @return 显式操作状态。
     */
    ObjectStatus Destroy(ObjectHandle handle);
    /**
     * @comment 返回当前状态快照。
     * @return 快照值。
     */
    ObjectSnapshot Snapshot() const;

private:
    ObjectStatus RecordFailure(ObjectStatus status);
    ObjectRegistrationResult RecordCapacityFailure(
        const ObjectDescriptor &descriptor,
        std::uint32_t required_object_count,
        std::uint32_t required_type_count,
        bool record_type_capacity_entry);
    void RecordSuccess();
    void ClearCapacityFailureSnapshot();
    ObjectStatus ResolveHandle(ObjectHandle handle, std::size_t& out_index) const;
    ObjectStatus RegisterTypeIfNeeded(ObjectTypeId type);
    bool HasType(ObjectTypeId type) const;
    void AdvanceGeneration(ObjectSlot& slot);

    std::array<ObjectSlot, MAX_OBJECT_COUNT> slots_;
    std::array<ObjectTypeId, MAX_OBJECT_TYPE_COUNT> types_;
    ObjectSnapshot snapshot_;
};
}
