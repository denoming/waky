#include "Application.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "state/DetectWakeWordState.hpp"
#include "state/RecordingCommandState.hpp"
#include "nn/NeuralNetwork.hpp"
#include "audio-processor/AudioProcessor.hpp"
#include "http/AgentUploader.hpp"
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

Application::~Application() = default;

bool
Application::setup()
{
    if (_context.setup()) {
        ESP_LOGD(TAG, "Enter into default state");
        _context.setState<DetectWakeWordState>(_sampler);
    } else {
        ESP_LOGE(TAG, "Failed to setup context");
        return false;
    }

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
    static const auto kTaskStackDepth = 3072u;
    static const auto kTaskPriority = (tskIDLE_PRIORITY + 1) | portPRIVILEGE_BIT;

    const auto rv
        = xTaskCreate(&run, "Application Task", kTaskStackDepth, this, kTaskPriority, nullptr);
    if (rv != pdPASS) {
        ESP_LOGE(TAG, "Failed to create application task");
        return;
    }

    ESP_LOGD(TAG, "Application task was created successfully");
}

void
Application::main()
{
    static const TickType_t kMaxBlockTime = pdMS_TO_TICKS(100);

#ifdef DEBUG
    printStackInfo(TAG, "Before listening");
#endif

    ESP_LOGD(TAG, "Start audio listening");
    if (!_sampler.start(xTaskGetCurrentTaskHandle())) {
        ESP_LOGE(TAG, "Failed to start microphone listening");
        return;
    }

    ESP_LOGD(TAG, "Start application loop");
    while (true) {
        const uint32_t notificationValue = ulTaskNotifyTake(pdTRUE, kMaxBlockTime);
        if (notificationValue > 0) {
            _context.proceed();
        }
    }
}

void
Application::run(void* param)
{
    assert(param != nullptr);

    Application* application = static_cast<Application*>(param);
    application->main();

    vTaskDelete(nullptr);
}