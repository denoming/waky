#pragma once

#include <esp_types.h>

class MemoryPool;

class AudioDataAccessor final {
public:
    explicit AudioDataAccessor(MemoryPool& memoryPool);

    void
    set(int16_t sample);

    [[nodiscard]] int16_t
    get() const;

    void
    put(int16_t sample);

    [[maybe_unused]] int16_t
    next();

    [[nodiscard]] size_t
    pos() const;

    void
    seek(long offset);

    [[nodiscard]] AudioDataAccessor
    clone();

private:
    MemoryPool& _memoryPool;
    size_t _index;
};