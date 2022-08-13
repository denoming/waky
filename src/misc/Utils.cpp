#include "Utils.hpp"

#include <esp_log.h>
#include <esp_system.h>

void
printHeapInfo(const char* tag, const char* prefix)
{
    const auto freeHeap = esp_get_free_heap_size();
    const auto minFreeHeap = esp_get_minimum_free_heap_size();
    ESP_LOGI(tag, "=== %s ===", prefix);
    ESP_LOGI(tag, "Heap memory info: free<%d>, min<%d>", freeHeap, minFreeHeap);
    ESP_LOGI(tag, "=== %s ===", prefix);
}