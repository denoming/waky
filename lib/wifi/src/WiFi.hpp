#pragma once

#include <memory>

class WiFi {
public:
    static constexpr uint32_t kMaxDelay = 0xFFFFFFFFUL;

    static WiFi&
    create();

    bool
    setUp(uint32_t timeout = kMaxDelay);

    void
    tearDown();

private:
    class Impl;
    WiFi(std::unique_ptr<Impl> impl);

private:
    std::unique_ptr<Impl> _impl;
};