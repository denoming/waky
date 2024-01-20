#include "LedIndicator.hpp"

#define INCLUDE_vTaskDelete 1

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_err.h>

#include <driver/gpio.h>

#define BLINK_PIN_R gpio_num_t(25) // Red
#define BLINK_PIN_G gpio_num_t(33) // Green
#define BLINK_PIN_B gpio_num_t(32) // Blue

#define LEVEL1 (1)
#define LEVEL0 (0)

namespace {

gpio_num_t
toGpioNum(LedColor color)
{
    switch (color) {
        using enum LedColor;
    case Red:
        return BLINK_PIN_R;
    case Green:
        return BLINK_PIN_G;
    case Blue:
        return BLINK_PIN_B;
    default:
        break;
    }
    return gpio_num_t(2);
}

} // namespace

class LedIndicatorImpl {
public:
    LedIndicatorImpl()
    {
        gpio_reset_pin(BLINK_PIN_R);
        gpio_set_direction(BLINK_PIN_R, GPIO_MODE_OUTPUT);
        gpio_reset_pin(BLINK_PIN_G);
        gpio_set_direction(BLINK_PIN_G, GPIO_MODE_OUTPUT);
        gpio_reset_pin(BLINK_PIN_B);
        gpio_set_direction(BLINK_PIN_B, GPIO_MODE_OUTPUT);
    }

    ~LedIndicatorImpl()
    {
        gpio_reset_pin(BLINK_PIN_R);
        gpio_reset_pin(BLINK_PIN_G);
        gpio_reset_pin(BLINK_PIN_B);
    }

    void
    on(LedColor color)
    {
        if (isBlinking()) {
            stopBlinking();
        }

        const auto gpioNum = toGpioNum(color);
        if (const int gpioLevel = gpio_get_level(gpioNum); gpioLevel == LEVEL0) {
            offAll();
            gpio_set_level(gpioNum, LEVEL1);
        }
    }

    void
    blink(LedColor color, int32_t delayMs)
    {
        startBlinking(color, delayMs);
    }

    void
    off()
    {
        if (isBlinking()) {
            stopBlinking();
        }
        offAll();
    }

private:
    static void
    offAll()
    {
        gpio_set_level(BLINK_PIN_R, 0);
        gpio_set_level(BLINK_PIN_G, 0);
        gpio_set_level(BLINK_PIN_B, 0);
    }

    [[noreturn]] static void
    handleBlinking(void* param)
    {
        if (auto* self = static_cast<LedIndicatorImpl*>(param); self == nullptr) {
            std::abort();
        } else {
            while (true) {
                self->blinkWithDelay();
            }
        }
    }

    void
    blinkWithDelay()
    {
        static const uint32_t kLightOnIntervalMs{100};

        // Blink with particular color
        const auto gpioNum = toGpioNum(_blinkColor);
        gpio_set_level(gpioNum, LEVEL1);
        vTaskDelay(pdMS_TO_TICKS(kLightOnIntervalMs));
        gpio_set_level(gpioNum, LEVEL0);

        // Wait particular delay
        vTaskDelay(pdMS_TO_TICKS(_blinkDelay));
    }

    bool
    startBlinking(LedColor color, int32_t delayMs)
    {
        if (isBlinking()) {
            vTaskSuspend(_blinkTask);
        }

        _blinkColor = color;
        _blinkDelay = delayMs;

        if (isBlinking()) {
            vTaskResume(_blinkTask);
            return true;
        } else {
            return (xTaskCreate(&handleBlinking, "BLINK", 1024, this, tskIDLE_PRIORITY, &_blinkTask)
                    == pdPASS);
        }
    }

    void
    stopBlinking()
    {
        if (isBlinking()) {
            vTaskDelete(_blinkTask);
            _blinkTask = {};
        }
    }

    [[nodiscard]] bool
    isBlinking() const
    {
        return (_blinkTask != TaskHandle_t{});
    }

private:
    TaskHandle_t _blinkTask{};
    LedColor _blinkColor{LedColor::Red};
    uint32_t _blinkDelay{};
};

LedIndicator::LedIndicator()
{
    static LedIndicatorImpl impl;
    _impl = &impl;
}

LedIndicator::~LedIndicator()
{
    _impl = nullptr;
}

void
LedIndicator::on(LedColor color)
{
    assert(_impl);
    if (_impl) {
        _impl->on(color);
    }
}

void
LedIndicator::blink(LedColor color, int32_t delayMs)
{
    assert(_impl);
    if (_impl) {
        _impl->blink(color, delayMs);
    }
}

void
LedIndicator::off()
{
    assert(_impl);
    if (_impl) {
        _impl->off();
    }
}