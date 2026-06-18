// 模块: YuEngine Audio
// 文件: Src/YuEngine/Audio/Include/YuEngine/Audio/AudioCallbackDevice.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/Audio/AudioCallbackDeviceDesc.h"
#include "YuEngine/Audio/AudioCallbackSnapshot.h"
#include "YuEngine/Audio/AudioStatus.h"

namespace yuengine::audio {
struct AudioCallbackDeviceState;

class AudioCallbackDevice final {
public:
    /**
     * @comment 构造空的 audio callback device owner。
     */
    AudioCallbackDevice();
    /**
     * @comment 释放 device 持有的 private backend objects。
     */
    ~AudioCallbackDevice();

    AudioCallbackDevice(const AudioCallbackDevice &) = delete;
    AudioCallbackDevice &operator=(const AudioCallbackDevice &) = delete;

    /**
     * @comment 根据 value descriptor 初始化 private callback backend。
     * @param desc 输入 callback backend descriptor。
     * @return 显式操作状态。
     */
    AudioStatus Initialize(const AudioCallbackDeviceDesc &desc);
    /**
     * @comment 为已初始化 backend 启动 callback processing。
     * @return 显式操作状态。
     */
    AudioStatus Start();
    /**
     * @comment 将 fixed S16 interleaved samples 提交到预分配 callback buffer。
     * @param interleaved_samples 输入 caller-owned S16 interleaved samples。
     * @param frame_count 输入 frame 数量。
     * @return 显式操作状态。
     */
    AudioStatus SubmitS16Buffer(std::span<const std::int16_t> interleaved_samples, std::size_t frame_count);
    /**
     * @comment 等待至少 target_completed_count 个 callbacks 完成。
     * @param target_completed_count Target completed callback 数量。
     * @param timeout_milliseconds 有界等待时长。
     * @return 显式操作状态。
     */
    AudioStatus WaitForCompletedCallbacks(std::uint64_t target_completed_count, std::uint32_t timeout_milliseconds);
    /**
     * @comment 将 callback completion records 排空到调用方持有存储。
     * @param completions 调用方持有的 completion output buffer。
     * @param completion_capacity completion records 可用数量。
     * @param out_completion_count 输出写入的 completion count。
     * @return 显式操作状态。
     */
    AudioStatus DrainCompletions(AudioCallbackCompletion *completions, std::size_t completion_capacity, std::size_t &out_completion_count);
    /**
     * @comment 停止 callback processing without destroying private backend objects。
     * @return 显式操作状态。
     */
    AudioStatus Stop();
    /**
     * @comment 关闭 private backend 并释放所有 platform objects。
     * @return 显式操作状态。
     */
    AudioStatus Shutdown();
    /**
     * @comment 返回 callback counters 和 lifecycle state。
     * @return 快照值。
     */
    AudioCallbackSnapshot Snapshot() const;

private:
    AudioCallbackDeviceState *state_;
};
}
