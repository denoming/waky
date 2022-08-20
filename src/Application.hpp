#pragma once

#include "MemoryPool.hpp"
#include "MemsMicrophone.hpp"
#include "state/StateContext.hpp"

#include <memory>

class Application {
public:
    static Application&
    create();

    ~Application();

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
    StateContext _context;
    MemoryPool _memory;
    MemsMicrophone _sampler;
};