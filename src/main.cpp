#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "WiFi.hpp"
#include "Application.hpp"
#ifdef DEBUG
#include "misc/Utils.hpp"
#endif

#include <esp_err.h>
#include <esp_log.h>

static const char* TAG = "ESP32 JRVA - Main";

static WiFi& wifi = WiFi::create();
static Application& application = Application::create();

void
setup()
{
    ESP_LOGD(TAG, "Setup WiFi");
    wifi.setUp();
    ESP_LOGD(TAG, "Setup Application");
    application.setup();
}

void
run()
{
    ESP_LOGD(TAG, "Start Application");
    application.start();
}

extern "C" void
app_main()
{
#ifdef DEBUG
    printHeapInfo(TAG, "MAIN");
#endif
    setup();
    run();
}