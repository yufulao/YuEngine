#pragma once

namespace yuengine::package {
enum class PACKAGE_STATUS {
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
    DependencyCycle
};
}
