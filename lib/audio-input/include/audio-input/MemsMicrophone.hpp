#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/i2s_std.h>

#include "audio-input/AudioDataAccessor.hpp"

class MemoryPool;

class MemsMicrophone final {
public:
    explicit MemsMicrophone(MemoryPool& memoryPool);

    [[nodiscard]] bool
    start(TaskHandle_t waiter);

    [[nodiscard]] AudioDataAccessor
    data();

private:
    [[nodiscard]] size_t
    pullAudioData(uint8_t* buffer, size_t size);

    void
    processAudioData(const uint8_t* buffer, size_t size);

    void
    notify();

    [[noreturn]] static void
    pullAudioDataTask(void* param);

private:
    i2s_chan_handle_t _channelHandle;
    AudioDataAccessor _accessor;
    TaskHandle_t _waiter;
};
