// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Src/AssetRuntimeFixture.cpp

#include "YuEngine/Asset/AssetRuntimeFixture.h"

#include <limits>

#include "YuEngine/Asset/AssetManager.h"

namespace yuengine::asset {
AssetRuntimeFixture::AssetRuntimeFixture()
    : snapshot_{} {
}

AssetRuntimeFixtureResult AssetRuntimeFixture::Execute(const AssetRuntimeFixtureRequest &request) {
    AssetRuntimeFixtureResult result{};
    result.status = ValidateRequest(request);
    if (result.status != AssetRuntimeFixtureStatus::Success) {
        return RecordRejected(result);
    }

    const std::uint32_t dependency_capacity =
        static_cast<std::uint32_t>(request.dependency_output.size());
    AssetStatus asset_status = request.manager->TraverseDependencies(
        request.root_asset,
        request.dependency_output.data(),
        dependency_capacity,
        &result.dependency_count);
    result.last_asset_status = asset_status;
    if (asset_status != AssetStatus::Success) {
        result.status = AssetRuntimeFixtureStatus::DependencyTraversalFailed;
        result.asset_snapshot = request.manager->Snapshot();
        return RecordRejected(result);
    }

    if (request.mark_loading) {
        asset_status = request.manager->MarkAssetLoading(request.root_asset);
        result.last_asset_status = asset_status;
        if (asset_status != AssetStatus::Success) {
            result.status = AssetRuntimeFixtureStatus::AssetLoadingFailed;
            result.asset_snapshot = request.manager->Snapshot();
            return RecordRejected(result);
        }
    }

    if (request.decoded_payload != nullptr) {
        asset_status = request.manager->MarkAssetDecoded(request.root_asset, *request.decoded_payload);
        result.last_asset_status = asset_status;
        if (asset_status != AssetStatus::Success) {
            result.status = AssetRuntimeFixtureStatus::AssetDecodedFailed;
            result.asset_snapshot = request.manager->Snapshot();
            return RecordRejected(result);
        }

        result.decoded_applied = true;
    }

    if (request.texture_result != nullptr) {
        asset_status = request.manager->MarkTextureReady(request.root_asset, *request.texture_result);
        result.last_asset_status = asset_status;
        if (asset_status != AssetStatus::Success) {
            result.status = AssetRuntimeFixtureStatus::TextureReadyFailed;
            result.asset_snapshot = request.manager->Snapshot();
            return RecordRejected(result);
        }

        result.texture_ready_applied = true;
    }

    if (request.audio_record != nullptr) {
        asset_status = request.manager->MarkAudioReady(request.root_asset, *request.audio_record);
        result.last_asset_status = asset_status;
        if (asset_status != AssetStatus::Success) {
            result.status = AssetRuntimeFixtureStatus::AudioReadyFailed;
            result.asset_snapshot = request.manager->Snapshot();
            return RecordRejected(result);
        }

        result.audio_ready_applied = true;
    }

    if (request.refresh_state_from_resource) {
        asset_status = request.manager->RefreshStateFromResource(
            request.resource_registry,
            request.root_asset);
        result.last_asset_status = asset_status;
        if (asset_status != AssetStatus::Success) {
            result.status = AssetRuntimeFixtureStatus::ResourceRefreshFailed;
            result.asset_snapshot = request.manager->Snapshot();
            return RecordRejected(result);
        }

        result.resource_state_refreshed = true;
    }

    asset_status = request.manager->QueryAsset(request.root_asset, &result.root_record);
    result.last_asset_status = asset_status;
    result.asset_snapshot = request.manager->Snapshot();
    if (asset_status != AssetStatus::Success) {
        result.status = AssetRuntimeFixtureStatus::AssetQueryFailed;
        return RecordRejected(result);
    }

    result.status = AssetRuntimeFixtureStatus::Success;
    return RecordCompleted(result);
}

AssetRuntimeFixtureSnapshot AssetRuntimeFixture::Snapshot() const {
    return snapshot_;
}

AssetRuntimeFixtureStatus AssetRuntimeFixture::ValidateRequest(
    const AssetRuntimeFixtureRequest &request) const {
    if (request.manager == nullptr) {
        return AssetRuntimeFixtureStatus::InvalidArgument;
    }

    if (!request.root_asset.IsValid()) {
        return AssetRuntimeFixtureStatus::InvalidArgument;
    }

    if (request.refresh_state_from_resource && request.resource_registry == nullptr) {
        return AssetRuntimeFixtureStatus::InvalidArgument;
    }

    if (request.dependency_output.size() > static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max())) {
        return AssetRuntimeFixtureStatus::InvalidArgument;
    }

    return AssetRuntimeFixtureStatus::Success;
}

AssetRuntimeFixtureResult AssetRuntimeFixture::RecordRejected(AssetRuntimeFixtureResult result) {
    ++snapshot_.rejected_count;
    snapshot_.last_status = result.status;
    snapshot_.last_asset_status = result.last_asset_status;
    snapshot_.last_root_state = result.root_record.state;
    return result;
}

AssetRuntimeFixtureResult AssetRuntimeFixture::RecordCompleted(AssetRuntimeFixtureResult result) {
    ++snapshot_.executed_count;
    snapshot_.dependency_traversal_count += result.dependency_count;
    snapshot_.last_status = result.status;
    snapshot_.last_asset_status = result.last_asset_status;
    snapshot_.last_root_state = result.root_record.state;
    return result;
}
}
