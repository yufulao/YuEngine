// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderGraphSkeletonStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment Defines explicit RenderCore render graph skeleton status values.
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
