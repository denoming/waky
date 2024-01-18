#include "wifi/Config.hpp"
#include "wifi/WiFi.hpp"
#include "Utils.hpp"
#include "Application.hpp"

#include <esp_log.h>

static const char* TAG = "WAKY[Main]";

static WiFi wifi;
static Application app;

void
setup()
{
    if (wifi.setup()) {
        ESP_LOGI(TAG, "Setup WiFi AP was successful");
    } else {
        ESP_LOGE(TAG, "Unable to setup WiFi AP");
        taskDelay(1000);
        return;
    }
    if (wifi.connect()) {
        ESP_LOGI(TAG, "Connecting to <%s> WiFi was successful", CONFIG_WAKY_WIFI_SSID);
    } else {
        ESP_LOGE(TAG, "Unable to connect to <%s> WiFi", CONFIG_WAKY_WIFI_SSID);
        taskDelay(1000);
        return;
    }

    if (app.setup()) {
        ESP_LOGI(TAG, "Setup application was successful");
    } else {
        ESP_LOGE(TAG, "Unable to setup application");
        taskDelay(1000);
    }
}

void
run()
{
    app.start();
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