#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "WiFi.hpp"
#include "HttpClient.hpp"
#include "HttpChunkSender.hpp"

#include <esp_err.h>
#include <esp_log.h>

static const char* TAG = "ESP32 JRVA - Main";

static WiFi& wifi = WiFi::create();

void
setup()
{
    ESP_LOGI(TAG, "SetUp WiFi");
    wifi.setUp();

    HttpClient client("192.168.1.20", 8000, "/speech");
    client.setHeader(Http::Field::Expect, "100-continue");
    client.setHeader(Http::Field::UserAgent, "J.A.R.V.I.S Agent");
    client.setMethod(HTTP_METHOD_POST);
    client.setTimeout(8000);
    client.connect(HttpClient::kChunkConnection);

    if (client.connected()) {
        int statusCode{0};

        int attempt = 5;
        while (attempt-- > 0) {
            client.fetchHeaders();
            statusCode = client.getStatusCode();
            if (statusCode == 100) {
                break;
            }
            ESP_LOGE(TAG, "Attempt to fetch again: %d", attempt);
            vTaskDelay(pdMS_TO_TICKS(200));
        }

        if (statusCode == 100) {
            const char* text = "Some long long long text";
            HttpChunkSender sender{client};
            if (!sender.send(text)) {
                ESP_LOGE(TAG, "Failed to send chunk");
            }
            if (!sender.finalize()) {
                ESP_LOGE(TAG, "Failed to finalize");
            }
        } else {
            ESP_LOGE(TAG, "Invalid status code: %d", statusCode);
        }
    } else {
        ESP_LOGI(TAG, "Failed to connect");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
    client.disconnect();
    client.cleanup();

    ESP_LOGI(TAG, "Waiting...");
    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "TearDown WiFi");
    wifi.tearDown();
}

extern "C" void
app_main()
{
    setup();
}