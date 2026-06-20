// 模块: YuEngine UiRuntime
// 文件: Src/YuEngine/UiRuntime/Src/BaseUiController.cpp

#include "YuEngine/UiRuntime/BaseUiController.h"

#include "YuEngine/UiRuntime/IBaseUiCloseRequestSink.h"

namespace yuengine::uiruntime {
BaseUiLifecycleStatus BaseUiController::Initialize() {
    if (snapshot_.destroyed) {
        return SetLastStatus(BaseUiLifecycleStatus::Destroyed);
    }

    if (snapshot_.initialized) {
        return SetLastStatus(BaseUiLifecycleStatus::Success);
    }

    BaseUiLifecycleStatus status = OnInitEvent();
    if (status != BaseUiLifecycleStatus::Success) {
        return SetLastStatus(status);
    }

    snapshot_.initialized = true;
    ++snapshot_.initialize_count;
    snapshot_.state = BaseUiLifecycleState::Initialized;

    status = OnBindEvent();
    if (status != BaseUiLifecycleStatus::Success) {
        return SetLastStatus(status);
    }

    snapshot_.events_bound = true;
    ++snapshot_.bind_event_count;
    return SetLastStatus(BaseUiLifecycleStatus::Success);
}

BaseUiLifecycleStatus BaseUiController::Open() {
    if (snapshot_.destroyed) {
        return SetLastStatus(BaseUiLifecycleStatus::Destroyed);
    }

    if (snapshot_.open) {
        return SetLastStatus(BaseUiLifecycleStatus::AlreadyOpen);
    }

    BaseUiLifecycleStatus status = Initialize();
    if (status != BaseUiLifecycleStatus::Success) {
        return status;
    }

    status = OnOpenEvent();
    if (status != BaseUiLifecycleStatus::Success) {
        return SetLastStatus(status);
    }

    snapshot_.open = true;
    ++snapshot_.open_count;
    snapshot_.state = BaseUiLifecycleState::Open;
    return SetLastStatus(BaseUiLifecycleStatus::Success);
}

BaseUiLifecycleStatus BaseUiController::Close() {
    if (snapshot_.destroyed) {
        return SetLastStatus(BaseUiLifecycleStatus::Destroyed);
    }

    if (!snapshot_.open) {
        return SetLastStatus(BaseUiLifecycleStatus::NotOpen);
    }

    const BaseUiLifecycleStatus status = OnCloseEvent();
    if (status != BaseUiLifecycleStatus::Success) {
        return SetLastStatus(status);
    }

    snapshot_.open = false;
    ++snapshot_.close_count;
    snapshot_.state = BaseUiLifecycleState::Closed;
    return SetLastStatus(BaseUiLifecycleStatus::Success);
}

BaseUiLifecycleStatus BaseUiController::Destroy() {
    if (snapshot_.destroyed) {
        return SetLastStatus(BaseUiLifecycleStatus::AlreadyDestroyed);
    }

    if (snapshot_.open) {
        const BaseUiLifecycleStatus close_status = Close();
        if (close_status != BaseUiLifecycleStatus::Success) {
            return close_status;
        }
    }

    const BaseUiLifecycleStatus status = OnClearEvent();
    if (status != BaseUiLifecycleStatus::Success) {
        return SetLastStatus(status);
    }

    snapshot_.open = false;
    snapshot_.destroyed = true;
    ++snapshot_.clear_count;
    snapshot_.state = BaseUiLifecycleState::Destroyed;
    return SetLastStatus(BaseUiLifecycleStatus::Success);
}

BaseUiLifecycleStatus BaseUiController::RequestCloseSelf(IBaseUiCloseRequestSink *close_request_sink) {
    if (close_request_sink == nullptr) {
        return SetLastStatus(BaseUiLifecycleStatus::InvalidCloseRequestSink);
    }

    if (snapshot_.destroyed) {
        return SetLastStatus(BaseUiLifecycleStatus::Destroyed);
    }

    if (!snapshot_.open) {
        return SetLastStatus(BaseUiLifecycleStatus::NotOpen);
    }

    const BaseUiLifecycleStatus status = close_request_sink->RequestCloseSelf(this);
    if (status != BaseUiLifecycleStatus::Success) {
        return SetLastStatus(status);
    }

    ++snapshot_.close_self_request_count;
    return SetLastStatus(BaseUiLifecycleStatus::Success);
}

BaseUiLifecycleSnapshot BaseUiController::Snapshot() const {
    return snapshot_;
}

BaseUiLifecycleStatus BaseUiController::OnInitEvent() {
    return BaseUiLifecycleStatus::Success;
}

BaseUiLifecycleStatus BaseUiController::OnBindEvent() {
    return BaseUiLifecycleStatus::Success;
}

BaseUiLifecycleStatus BaseUiController::OnOpenEvent() {
    return BaseUiLifecycleStatus::Success;
}

BaseUiLifecycleStatus BaseUiController::OnCloseEvent() {
    return BaseUiLifecycleStatus::Success;
}

BaseUiLifecycleStatus BaseUiController::OnClearEvent() {
    return BaseUiLifecycleStatus::Success;
}

BaseUiLifecycleStatus BaseUiController::SetLastStatus(BaseUiLifecycleStatus status) {
    snapshot_.last_status = status;
    return status;
}
}
