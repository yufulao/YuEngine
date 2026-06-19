// 模块: YuEngine Serialize
// 文件: Src/YuEngine/Serialize/Include/YuEngine/Serialize/RuntimeConfigStream.h

#pragma once

#include "YuEngine/Serialize/RuntimeConfigRecord.h"
#include "YuEngine/Serialize/RuntimeProfileBoundary.h"
#include "YuEngine/Serialize/SerializeStatus.h"

namespace yuengine::serialize {
class SerializeReader;
class SerializeWriter;

class RuntimeConfigStream final {
public:
    /**
     * @comment 将 runtime config 和 save/profile boundary 写入调用方持有的 value stream。
     * @param writer 调用方持有的 SerializeWriter。
     * @param config 输入 runtime config。
     * @param boundary 输入 save/profile boundary。
     * @return 显式序列化状态。
     */
    SerializeStatus WriteRuntimeConfig(
        SerializeWriter *writer,
        const RuntimeConfigRecord &config,
        const RuntimeProfileBoundary &boundary) const;

    /**
     * @comment 从调用方持有的 value stream 读取 runtime config 和 save/profile boundary。
     * @param reader 调用方持有的 SerializeReader。
     * @param out_config 成功时写入 runtime config。
     * @param out_boundary 成功时写入 save/profile boundary。
     * @return 显式序列化状态。
     */
    SerializeStatus ReadRuntimeConfig(
        SerializeReader *reader,
        RuntimeConfigRecord *out_config,
        RuntimeProfileBoundary *out_boundary) const;

private:
    SerializeStatus ValidateConfig(const RuntimeConfigRecord &config) const;
    SerializeStatus ValidateBoundary(const RuntimeProfileBoundary &boundary) const;
    SerializeStatus ValidateWriteBudget(const SerializeWriter &writer) const;
    bool IsBoundaryKindValid(RuntimeProfileBoundaryKind kind) const;
};
}
