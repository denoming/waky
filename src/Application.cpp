#include "Application.hpp"

#include "states/DetectWakeWordState.hpp"
#include "states/RecordingCommandState.hpp"
#include "misc/Utils.hpp"

#include <esp_err.h>
#include <esp_log.h>

static const char* TAG = "ESP32 JRVA - App";

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
    , _currentState{nullptr}
{
}

bool
Application::setup()
{
    ESP_LOGI(TAG, "Allocate memory for memory pool");
    if (!_memory.allocate()) {
        ESP_LOGE(TAG, "Failed to allocate memory for memory pool");
        return false;
    }
    printHeapInfo(TAG);

    _detectionState.reset(new DetectWakeWordState{_sampler});
    _recordingState.reset(new RecordingCommandState{_sampler});

    return true;
}

void
Application::start()
{
    static const auto kTaskStackDepth = 4096u;
    static const auto kTaskPriority = (tskIDLE_PRIORITY + 1) | portPRIVILEGE_BIT;

    assert(_detectionState);
    _currentState = _detectionState.get();

    auto rv = xTaskCreate(&run, "Application Task", kTaskStackDepth, this, kTaskPriority, &_task);
    if (rv != pdPASS) {
        ESP_LOGE(TAG, "Failed to create application task");
        return;
    }
}

void
Application::main()
{
    static const TickType_t kMaxBlockTime = pdMS_TO_TICKS(100);

    ESP_LOGI(TAG, "Start listening on MEMS microphone");
    if (!_sampler.start(_task)) {
        ESP_LOGE(TAG, "Failed to start microphone listening");
        return vTaskDelete(_task);
    }

    assert(_currentState != nullptr);
    _currentState->enterState();

    ESP_LOGI(TAG, "Start aplication lopping");
    while (true) {
        const uint32_t notificationValue = ulTaskNotifyTake(pdTRUE, kMaxBlockTime);
        if (notificationValue > 0) {
            assert(_currentState != nullptr);
            if (_currentState->run()) {
                _currentState->exitState();
                if (_currentState == _detectionState.get()) {
                    _currentState = _recordingState.get();
                } else {
                    _currentState = _detectionState.get();
                }
                assert(_currentState != nullptr);
                _currentState->enterState();
            }
        }
    }
}

void
Application::run(void* param)
{
    assert(param != nullptr);
    Application* application = static_cast<Application*>(param);
    application->main();
}