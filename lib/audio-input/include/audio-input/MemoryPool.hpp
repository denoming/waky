#pragma once

#include <array>

#include <esp_types.h>

class MemoryPool final {
public:
    MemoryPool();

    ~MemoryPool();

    bool
    allocate();

    void
    deallocate();

    void
    set(size_t index, int16_t value);

    [[nodiscard]] int16_t
    get(size_t index) const;

    static inline size_t
    capacity();

private:
    static const std::size_t ChunkCount = 5;
    static const std::size_t ChunkSize = 1600;

    class Chunk {
    public:
        Chunk()
            : _payload{}
        {
            _payload.fill(0);
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
    std::array<Chunk*, ChunkCount> _chunks;
};

// Inlines

std::size_t
MemoryPool::capacity()
{
    return (ChunkCount * ChunkSize);
}