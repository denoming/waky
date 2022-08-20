#pragma once

#include <driver/i2s.h>

#include "audio-input/AudioDataAccessor.hpp"
#include "audio-input/AudioInputSampler.hpp"

class MemsMicrophone final : public AudioInputSampler {
public:
    MemsMicrophone(MemoryPool& memoryPool);

    MemsMicrophone(i2s_pin_config_t pins,
                   i2s_port_t port,
                   i2s_config_t config,
                   MemoryPool& memoryPool);

    bool
    start(TaskHandle_t waiter) override;

    AudioDataAccessor
    data();

private:
    std::size_t
    pullData(uint8_t* buffer, std::size_t size);

    void
    processData(const uint8_t* buffer, std::size_t size);

    void
    notify();

    static void
    pullDataTask(void* param);

private:
    i2s_pin_config_t _pins;
    i2s_port_t _port;
    i2s_config_t _config;
    AudioDataAccessor _accessor;
    QueueHandle_t _queue;
    TaskHandle_t _waiter;
};
