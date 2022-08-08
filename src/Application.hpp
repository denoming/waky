#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "MemoryPool.hpp"
#include "MemsMicrophone.hpp"
#include "states/State.hpp"

#include <memory>

class Application {
public:
    static Application&
    create();

    bool
    setup();

    void
    start();

private:
    Application();

    void
    main();

    static void
    run(void* param);

private:
    MemoryPool _memory;
    MemsMicrophone _sampler;
    std::unique_ptr<State> _detectionState;
    std::unique_ptr<State> _recordingState;
    State* _currentState;
    TaskHandle_t _task;
};