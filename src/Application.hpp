#pragma once

#include "audio-input/MemoryPool.hpp"
#include "audio-input/MemsMicrophone.hpp"
#include "state/StateContext.hpp"

#include <memory>

class Application {
public:
    Application();

    ~Application();

    [[nodiscard]] bool
    setup();

    void
    start();

private:
    class ApplicationImpl* _impl;
};