#include "audio-input/MemoryPool.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <esp_system.h>

static const char* TAG = "WAKY[MemoryPool]";

MemoryPool::MemoryPool()
    : _chunks{}
{
    allocate();
}

MemoryPool::~MemoryPool()
{
    deallocate();
}

void
MemoryPool::allocate()
{
    for (int i = 0; i < ChunkCount; ++i) {
        const auto freeHeap = esp_get_free_heap_size();
        if (freeHeap < ChunkSize) {
            ESP_LOGE(TAG, "Free heap memory is exhausted");
            vTaskSuspend(nullptr);
            return;
        }
#ifdef DEBUG
        ESP_LOGD(TAG, "Allocate memory pool chunk: <%d>", i + 1);
#endif
        assert(_chunks[i] == nullptr);
        _chunks[i] = new Chunk;
    }
}

void
MemoryPool::deallocate()
{
    for (int i = 0; i < ChunkCount; ++i) {
#ifdef DEBUG
        ESP_LOGD(TAG, "Deallocate memory pool chunk: <%d>", i + 1);
#endif
        delete _chunks[i];
        _chunks[i] = nullptr;
    }
}

void
MemoryPool::set(size_t index, int16_t value)
{
    assert(index < capacity());
    const auto chunkIdx = (index / ChunkSize) % ChunkCount;
    const auto chunkPos = (index % ChunkSize);
    assert(_chunks[chunkIdx] != nullptr);
    _chunks[chunkIdx]->set(chunkPos, value);
}

int16_t
MemoryPool::get(std::size_t index) const
{
    assert(index < capacity());
    const auto chunkIdx = (index / ChunkSize) % ChunkCount;
    const auto chunkPos = (index % ChunkSize);
    assert(_chunks[chunkIdx] != nullptr);
    return _chunks[chunkIdx]->get(chunkPos);
}
