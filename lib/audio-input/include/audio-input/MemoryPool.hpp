#pragma once

#include <array>

#include <esp_types.h>

#include "sdkconfig.h"

class MemoryPool final {
public:
    MemoryPool();

    ~MemoryPool();

    void
    set(size_t index, int16_t value);

    [[nodiscard]] int16_t
    get(size_t index) const;

    static inline size_t
    capacity();

private:
    static const std::size_t ChunkCount = CONFIG_WAKY_MEMPOOL_BUFFER_COUNT;
    static const std::size_t ChunkSize = CONFIG_WAKY_MEMPOOL_BUFFER_SIZE;

    class Chunk {
    public:
        Chunk()
            : _payload{}
        {
        }

        void
        set(size_t index, int16_t value)
        {
            _payload[index] = value;
        }

        [[nodiscard]] int16_t
        get(size_t index) const
        {
            return _payload[index];
        }

    private:
        std::array<int16_t, ChunkSize> _payload;
    };

private:
    void
    allocate();

    void
    deallocate();

private:
    std::array<Chunk*, ChunkCount> _chunks;
};

// Inlines

std::size_t
MemoryPool::capacity()
{
    return (ChunkCount * ChunkSize);
}