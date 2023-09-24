#pragma once

#include <esp_types.h>

class WiFi {
public:
    static constexpr uint32_t kMaxDelay = 0xFFFFFFFFUL;

    WiFi();

    ~WiFi();

    bool
    setup();

    bool
    connect(uint32_t timeout = kMaxDelay);

    void
    finalise();

private:
    class WiFiImpl* _impl;
};