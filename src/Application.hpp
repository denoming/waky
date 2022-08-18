#pragma once

#include "MemoryPool.hpp"
#include "MemsMicrophone.hpp"
#include "Context.hpp"

class Application : public Context {
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
};