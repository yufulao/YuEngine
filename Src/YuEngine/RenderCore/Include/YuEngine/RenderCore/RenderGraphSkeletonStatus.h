// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeletonStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment 定义 explicit RenderCore render graph skeleton 状态 值.
 */
enum class RenderGraphSkeletonStatus {
    Success,
    InvalidArgument,
    InvalidGraphId,
    EmptyGraph,
    InvalidDeclarationStorage,
    InvalidPreparedRequestStorage,
    InvalidResultStorage,
    InvalidPassId,
    DuplicateGraphId,
    DuplicatePassId,
    MissingDependency,
    SelfDependency,
    DependencyCycle,
    PassCapacityExceeded,
    DependencyCapacityExceeded,
    InvalidPassRequest,
    GraphRecordNotFound
};
}
