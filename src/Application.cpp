#include "Application.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "state/DetectWakeWordState.hpp"
#include "state/RecordingCommandState.hpp"
#include "nn/NeuralNetwork.hpp"
#include "audio-processor/AudioProcessor.hpp"
#include "http/AgentUploader.hpp"
#include "LedIndicator.hpp"

#include <esp_err.h>
#include <esp_log.h>

static const char* TAG = "WAKY[Application]";

class ApplicationImpl {
public:
    ApplicationImpl() = default;

    bool
    setup();

    void
    start();

private:
    [[noreturn]] void
    main();

    static void
    run(void* param);

private:
    LedIndicator _indicator;
    std::unique_ptr<StateContext> _context;
    std::unique_ptr<MemoryPool> _memoryPool;
    std::unique_ptr<MemsMicrophone> _sampler;
};

bool
ApplicationImpl::setup()
{
    _context = std::make_unique<StateContext>(_indicator);
    _memoryPool = std::make_unique<MemoryPool>();
    _sampler = std::make_unique<MemsMicrophone>(*_memoryPool);

    if (_context->setup()) {
#ifdef DEBUG
        ESP_LOGD(TAG, "Enter into default state");
#endif
        _context->setState<DetectWakeWordState>(*_sampler);
    } else {
        ESP_LOGE(TAG, "Failed to setup context");
        return false;
    }

    return true;
}

void
ApplicationImpl::start()
{
    static const int32_t kTaskStackDepth = 4096u;
    static const int32_t kTaskPinnedCore = 0;

    const auto rv = xTaskCreatePinnedToCore(
        &run, "WAKY_RECO", kTaskStackDepth, this, tskIDLE_PRIORITY, nullptr, kTaskPinnedCore);
    if (rv != pdPASS) {
        ESP_LOGE(TAG, "Failed to create application task");
        return;
    }

#ifdef DEBUG
    ESP_LOGD(TAG, "Application task was created successfully");
#endif
}

void
ApplicationImpl::main()
{
    static const TickType_t kMaxBlockTime = pdMS_TO_TICKS(300);

#ifdef DEBUG
    printStackInfo(TAG, "Before listening");
#endif

#ifdef DEBUG
    ESP_LOGD(TAG, "Start audio listening");
#endif
    ESP_LOGI(TAG, "Start audio listening");
    if (!_sampler->start(xTaskGetCurrentTaskHandle())) {
        ESP_LOGE(TAG, "Unable to start microphone listening");
        vTaskSuspend(nullptr);
    }

#ifdef DEBUG
    ESP_LOGD(TAG, "Start application loop");
#endif
    ESP_LOGI(TAG, "Start application loop");
    while (true) {
        if (ulTaskNotifyTake(pdTRUE, kMaxBlockTime) > 0 /* notification value after reset */) {
            _context->proceed();
        }
    }
}

void
ApplicationImpl::run(void* param)
{
    auto* impl = static_cast<ApplicationImpl*>(param);
    assert(impl != nullptr);
    if (impl) {
        impl->main();
    }

    vTaskDelete(nullptr);
}

Application::Application()
{
    static ApplicationImpl impl;
    _impl = &impl;
}

Application::~Application()
{
    _impl = nullptr;
}

bool
Application::setup()
{
    assert(_impl != nullptr);
    return _impl != nullptr && _impl->setup();
}

void
Application::start()
{
    assert(_impl != nullptr);
    if (_impl) {
        _impl->start();
    }
}
