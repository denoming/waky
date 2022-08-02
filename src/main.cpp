#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "WiFi.hpp"

#include <esp_err.h>
#include <esp_log.h>

static const char* TAG = "ESP32 JRVA - Main";

static WiFi& wifi = WiFi::create();

void
setup()
{
    const TickType_t kDelay = 1000 / portTICK_PERIOD_MS;

    ESP_LOGI(TAG, "SetUp WiFi");
    wifi.setUp();

    ESP_LOGI(TAG, "Waiting...");
    vTaskDelay(kDelay * 10);

    ESP_LOGI(TAG, "TearDown WiFi");
    wifi.tearDown();
}

extern "C" void
app_main()
{
    setup();
}