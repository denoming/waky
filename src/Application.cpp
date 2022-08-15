#include "Application.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "states/DetectWakeWordState.hpp"
#include "states/RecordingCommandState.hpp"
#ifdef DEBUG
#include "misc/Utils.hpp"
#endif

#include <esp_err.h>
#include <esp_log.h>

static const char* TAG = "ESP32 JRVA - Application";

Application&
Application::create()
{
    static std::unique_ptr<Application> instance;
    if (!instance) {
        instance.reset(new Application);
    }
    return *instance;
}

Application::Application()
    : _sampler{_memory}
{
}

bool
Application::setup()
{
    ESP_LOGD(TAG, "Allocate memory for memory pool");
    if (!_memory.allocate()) {
        ESP_LOGE(TAG, "Failed to allocate memory for memory pool");
        return false;
    }

#ifdef DEBUG
    printHeapInfo(TAG, "After memory pool allocation");
#endif

    return true;
}

void
Application::start()
{
    static const auto kTaskStackDepth = 4096u;
    static const auto kTaskPriority = (tskIDLE_PRIORITY + 1) | portPRIVILEGE_BIT;

    auto rv = xTaskCreate(&run, "Application Task", kTaskStackDepth, this, kTaskPriority, nullptr);
    if (rv != pdPASS) {
        ESP_LOGE(TAG, "Failed to create application task");
        return;
    }
}

void
Application::main()
{
    ESP_LOGD(TAG, "Start audio listening");
    if (!_sampler.start(xTaskGetCurrentTaskHandle())) {
        ESP_LOGE(TAG, "Failed to start microphone listening");
        return vTaskDelete(nullptr);
    }

    ESP_LOGD(TAG, "Start application loop");
    proceed(_sampler);

    vTaskDelete(nullptr);
}

void
Application::run(void* param)
{
    assert(param != nullptr);
    Application* application = static_cast<Application*>(param);
    application->main();
}