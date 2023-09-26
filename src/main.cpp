#include "wifi/Config.hpp"
#include "wifi/WiFi.hpp"
#ifdef DEBUG
#include "misc/Utils.hpp"
#endif
#include "Application.hpp"

#include <esp_log.h>

static const char* TAG = "WAKY[Main]";

static WiFi wifi;
static Application app;

void
setup()
{
    if (not wifi.setup()) {
        ESP_LOGE(TAG, "Unable to setup <%s> WiFi AP", CONFIG_WAKY_WIFI_SSID);
        return;
    }
    if (wifi.connect()) {
        ESP_LOGI(TAG, "Connecting to <%s> WiFi AP was successful", CONFIG_WAKY_WIFI_SSID);
    } else {
        ESP_LOGE(TAG, "Unable to connect to <%s> WiFi AP", CONFIG_WAKY_WIFI_SSID);
    }

    if (app.setup()) {
        ESP_LOGI(TAG, "Setup application was successful");
    } else {
        ESP_LOGE(TAG, "Unable to setup application");
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