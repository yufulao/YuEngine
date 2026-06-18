// Module: YuEngine Audio
// File: Src/YuEngine/Audio/Src/AudioCallbackDeviceWindowsInternal.h

#pragma once

#include "YuEngine/Audio/AudioStatus.h"

namespace yuengine::audio {
struct AudioCallbackDeviceBackendTestConfig final {
    bool enabled;
    AudioStatus initialize_status;
    AudioStatus start_status;
    AudioStatus submit_status;
    bool complete_submitted_buffer;
    bool report_callback_error_on_submit;
};

void SetAudioCallbackDeviceBackendTestConfig(const AudioCallbackDeviceBackendTestConfig &config);
void ClearAudioCallbackDeviceBackendTestConfig();
}
