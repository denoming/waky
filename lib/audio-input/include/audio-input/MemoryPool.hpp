#pragma once

#include <cstdint>
#include <array>

#include "sdkconfig.h"

class MemoryPool final {
public:
    MemoryPool();

    ~MemoryPool();

    bool
    allocate();

    void
    deallocate();

    void
    set(std::size_t index, std::uint16_t value);

    std::uint16_t
    get(std::size_t index) const;

    static inline std::size_t
    capacity();

private:
    static const std::size_t ChunkCount = CONFIG_WAKY_MEMPOOL_BUFFER_COUNT;
    static const std::size_t ChunkSize = CONFIG_WAKY_MEMPOOL_BUFFER_SIZE;

    class Chunk {
    public:
        Chunk()
        {
            _payload.fill(0);
        }

        void
        set(std::size_t index, std::uint16_t value)
        {
            _payload[index] = value;
        }

        std::uint16_t
        get(std::size_t index) const
        {
            return _payload[index];
        }

    private:
        std::array<std::int16_t, ChunkSize> _payload;
    };

private:
    std::array<Chunk*, ChunkCount> _chunks;
};

// Inlines

std::size_t
MemoryPool::capacity()
{
    return (ChunkCount * ChunkSize);
}