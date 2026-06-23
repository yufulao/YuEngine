// 模块: YuEngine Package
// 文件: Src/YuEngine/Package/Include/YuEngine/Package/PackageStatus.h

#pragma once

namespace yuengine::package {
enum class PackageStatus {
    Success,
    InvalidPackageId,
    InvalidEntryId,
    InvalidResourceType,
    InvalidLogicalKey,
    InvalidSourceKey,
    LogicalKeyTooLong,
    SourceKeyTooLong,
    DuplicateManifest,
    DuplicateEntry,
    DuplicateResourceKey,
    ManifestCapacityExceeded,
    EntryCapacityExceeded,
    DependencyCapacityExceeded,
    LoadPlanCapacityExceeded,
    ByteRangeOutOfBounds,
    NotFound,
    TypeMismatch,
    DependencyMissing,
    DependencyCycle,
    InvalidArtifact,
    ArtifactCapacityExceeded,
    FileReadFailed,
    FileWriteFailed
};
}
