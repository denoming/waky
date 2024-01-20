#pragma once

#include <esp_types.h>

enum class LedColor { Red, Green, Blue };

class LedIndicator {
public:
    LedIndicator();

    ~LedIndicator();

    void
    on(LedColor color);

    void
    blink(LedColor color, int32_t delayMs);

    void
    off();

private:
    class LedIndicatorImpl* _impl;
};