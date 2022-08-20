#include "misc/Utils.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <esp_system.h>

void
printHeapInfo(const char* tag, const char* prefix)
{
    const auto freeHeap = esp_get_free_heap_size();
    ESP_LOGI(tag, "=== %s ===", prefix);
    ESP_LOGI(tag, "Heap memory info: free <%d> bytes", freeHeap);
    ESP_LOGI(tag, "=== %s ===", prefix);
}

void
printStackInfo(const char* tag, const char* prefix)
{
    const auto waterMark = uxTaskGetStackHighWaterMark(nullptr);
    ESP_LOGI(tag, "=== %s ===", prefix);
    ESP_LOGI(tag, "Stack memory info: free <%u> words", waterMark * 4);
    ESP_LOGI(tag, "=== %s ===", prefix);
}