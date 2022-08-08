#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "WiFi.hpp"
#include "Application.hpp"

#include <esp_err.h>
#include <esp_log.h>

static const char* TAG = "ESP32 JRVA - Main";

static WiFi& wifi = WiFi::create();
static Application& application = Application::create();

void
setup()
{
    ESP_LOGI(TAG, "Setup WiFi");
    wifi.setUp();
    ESP_LOGI(TAG, "Setup Application");
    application.setup();
}

void
run()
{
    ESP_LOGI(TAG, "Start Application");
    application.start();
}

extern "C" void
app_main()
{
    setup();
    run();
}