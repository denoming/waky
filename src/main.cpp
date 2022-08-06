#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "WiFi.hpp"
#include "Config.hpp"
#include "NeuralNetwork.hpp"
#include "AudioProcessor.hpp"
#include "AudioBuffer.hpp"
#include "MemsMicrophone.hpp"
#include "MemoryPool.hpp"
#include "states/DetectWakeWordState.hpp"
#include "states/RecordingCommandState.hpp"

#include <esp_err.h>
#include <esp_log.h>
#include <esp_check.h>

static const char* TAG = "ESP32 JRVA - Main";

static WiFi& wifi = WiFi::create();

void
setup()
{
    ESP_LOGI(TAG, "SetUp WiFi");
    wifi.setUp();
}

void
loop()
{
    static const TickType_t kMaxBlockTime = pdMS_TO_TICKS(100);

    MemoryPool memoryPool;
    MemsMicrophone mic{I2S_PIN_CONFIG, I2S_INMP441_PORT, I2S_CONFIG, memoryPool};
    ESP_LOGI(TAG, "Start listening on MEMS microphone");
    if (!mic.start(xTaskGetCurrentTaskHandle())) {
        ESP_LOGE(TAG, "Failed to start microphone");
        vTaskSuspend(NULL);
    }

    auto* detectWakeWordState = new DetectWakeWordState{mic};
    auto* recordingCommandState = new RecordingCommandState{mic};

    State* currentState = detectWakeWordState;
    ESP_LOGI(TAG, "Enter to default state");
    currentState->enterState();

    ESP_LOGI(TAG, "Start lopping");
    while (true) {
        const uint32_t notificationValue = ulTaskNotifyTake(pdTRUE, kMaxBlockTime);
        if (notificationValue > 0) {
            assert(currentState != nullptr);
            if (currentState->run()) {
                currentState->exitState();
                if (currentState == detectWakeWordState) {
                    currentState = recordingCommandState;
                } else {
                    currentState = detectWakeWordState;
                }
                currentState->enterState();
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

extern "C" void
app_main()
{
    setup();
    loop();
}