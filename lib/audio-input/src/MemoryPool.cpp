#include "audio-input/MemoryPool.hpp"

#include <esp_log.h>
#include <esp_system.h>

static const char* TAG = "ESP32 TFLITE WWD - MM";

MemoryPool::MemoryPool()
{
    for (int i = 0; i < ChunkCount; ++i) {
        _chunks[i] = nullptr;
    }
}

MemoryPool::~MemoryPool()
{
    deallocate();
}

bool
MemoryPool::allocate()
{
    for (int i = 0; i < ChunkCount; ++i) {
        const auto freeHeap = esp_get_free_heap_size();
        if (freeHeap < ChunkSize) {
            ESP_LOGE(TAG, "Free heap memory is exhausted");
            return false;
        }

        ESP_LOGD(TAG, "Allocate memory pool chunk: <%d>", i + 1);
        assert(_chunks[i] == nullptr);
        _chunks[i] = new Chunk;
    }

    return true;
}

void
MemoryPool::deallocate()
{
    for (int i = 0; i < ChunkCount; ++i) {
        ESP_LOGD(TAG, "Deallocate memory pool chunk: <%d>", i + 1);
        delete _chunks[i];
        _chunks[i] = nullptr;
    }
}

void
MemoryPool::set(std::size_t index, std::uint16_t value)
{
    assert(index < capacity());
    const long chunkIdx = (index / ChunkSize) % ChunkCount;
    const long chunkPos = (index % ChunkSize);
    assert(_chunks[chunkIdx] != nullptr);
    _chunks[chunkIdx]->set(chunkPos, value);
}

std::uint16_t
MemoryPool::get(std::size_t index) const
{
    assert(index < capacity());
    const long chunkIdx = (index / ChunkSize) % ChunkCount;
    const long chunkPos = (index % ChunkSize);
    assert(_chunks[chunkIdx] != nullptr);
    return _chunks[chunkIdx]->get(chunkPos);
}
