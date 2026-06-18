// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDeviceFactory.h

#pragma once

#include <cstddef>
#include <span>

#include "YuEngine/Rhi/RhiBackendKind.h"
#include "YuEngine/Rhi/RhiDeviceCreateResult.h"
#include "YuEngine/Rhi/RhiDeviceDesc.h"
#include "YuEngine/Rhi/RhiNativeSurfaceDesc.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rhi {
class NullRhiDevice;

class RhiDeviceFactory final {
public:
    /**
     * @comment 创建 一个 backend-neutral device 视图 从 调用方持有 存储。
     * @param desc 输入描述。
     * @param null_device 调用方持有的 null backend 存储。
     * @return 显式 创建结果。
     */
    static RhiDeviceCreateResult CreateDevice(const RhiDeviceDesc &desc, NullRhiDevice *null_device);
    /**
     * @comment 创建 一个 backend-neutral device 视图 在 调用方持有 存储。
     * @param desc 输入描述。
     * @param device_storage 调用方持有的 存储 用于 选中的 backend。
     * @return 显式 创建结果。
     */
    static RhiDeviceCreateResult CreateDevice(const RhiDeviceDesc &desc, std::span<std::byte> device_storage);
    /**
     * @comment 销毁在调用方持有存储中创建的 device。
     * @param device CreateDevice 返回的 device 指针。
     * @return 显式操作状态。
     */
    static RhiStatus DestroyDevice(IRhiDevice *device);
    /**
     * @comment 返回 所需 存储 字节 用于 一个 backend。
     * @param backend_kind 输入 后端类型。
     * @return 所需 字节数。
     */
    static std::size_t RequiredDeviceStorageSize(RhiBackendKind backend_kind);
    /**
     * @comment 返回 所需 存储 对齐 用于 一个 backend。
     * @param backend_kind 输入 后端类型。
     * @return 所需 字节 对齐。
     */
    static std::size_t RequiredDeviceStorageAlignment(RhiBackendKind backend_kind);
    /**
     * @comment 验证 an RHI-owned native surface 描述。
     * @param surface_desc 输入描述。
     * @return 显式操作状态。
     */
    static RhiStatus ValidateNativeSurfaceDesc(const RhiNativeSurfaceDesc &surface_desc);

private:
    RhiDeviceFactory() = delete;

    static RhiStatus ValidateDeviceDesc(const RhiDeviceDesc &desc);
};
}
