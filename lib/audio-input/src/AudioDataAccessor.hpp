#pragma once

#include <memory>
#include <vector>

class MemoryPool;

class AudioDataAccessor final {
public:
    explicit AudioDataAccessor(MemoryPool& memoryPool);

    void
    set(std::int16_t sample);

    std::int16_t
    get() const;

    void
    put(std::int16_t sample);

    std::int16_t
    next();

    int
    pos() const;

    void
    seek(int index);

    AudioDataAccessor
    clone();

private:
    MemoryPool& _memoryPool;
    std::size_t _index;
};